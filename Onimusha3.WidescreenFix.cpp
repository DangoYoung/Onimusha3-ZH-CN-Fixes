// Simplified Chinese retail edition adapter for Onimusha 3.
// Widescreen layout adapted from ThirteenAG's Onimusha 3 Widescreen Fix (GPL-3.0).
// https://github.com/ThirteenAG/WidescreenFixesPack
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <array>
#include <cstdint>
#include <cstring>

namespace
{
    struct Resolution
    {
        std::uint32_t width;
        std::uint32_t height;
    };

    struct Settings
    {
        Resolution resolution;
        bool cropMovies;
    };

    constexpr DWORD ChineseRetailTimestamp = 0x4364f2d3;
    constexpr DWORD ChineseRetailImageSize = 0x03f00000;

    // RVAs for this edition, with expected instruction bytes checked before patching.
    constexpr DWORD StartupResolution = 0x0001fc00;
    constexpr BYTE OriginalStartup[] = {0x68, 0x58, 0x02, 0, 0, 0x68, 0x20, 0x03, 0, 0};

    constexpr DWORD WindowedFallback = 0x003d3593;
    constexpr BYTE OriginalFallback[] = {0xb9, 0x20, 0x03, 0, 0, 0xba, 0x58, 0x02, 0, 0};

    constexpr DWORD CameraProjection = 0x00033bca;
    constexpr BYTE OriginalCamera[] = {0xf3, 0x0f, 0x10, 0x0d, 0x28, 0x8e, 0x90, 0};

    constexpr DWORD SpriteProjection = 0x00033f0c;
    constexpr BYTE OriginalSprite[] = {0xf3, 0x0f, 0x10, 0x2d, 0x28, 0x8e, 0x90, 0};

    constexpr DWORD BackgroundQuad = 0x00036987;
    constexpr BYTE OriginalBackground[] = {0x8b, 0x08, 0x8b, 0x91, 0x0c, 0x01, 0, 0};

    constexpr DWORD MovieQuad = 0x000367df;
    constexpr BYTE OriginalMovie[] = {0xf3, 0x0f, 0x11, 0x64, 0x24, 0x70, 0xf3, 0x0f, 0x11, 0x44, 0x24, 0x74};

    constexpr DWORD HorizontalClip = 0x000ca6c2;
    constexpr BYTE OriginalHorizontalClip[] = {0xf3, 0x0f, 0x10, 0x05, 0x88, 0x5f, 0x91, 0};

    constexpr DWORD VerticalClip = 0x000ca74c;
    constexpr BYTE OriginalVerticalClip[] = {0xf3, 0x0f, 0x10, 0x05, 0x84, 0x5f, 0x91, 0};

    // DXUT stores the active device settings and the actual D3D9 back-buffer dimensions.
    constexpr DWORD DeviceSettingsPointer = 0x03e60a0c;
    constexpr DWORD BackBufferDimensions = 0x03e60a28;
    constexpr DWORD CurrentSprite = 0x02dbae30;
    constexpr int PresentParametersWindowed = 12;

    HMODULE pluginModule;
    Resolution wideResolution;
    const volatile std::uint32_t* const volatile* activeDeviceSettings;
    const volatile Resolution* activeBackBuffer;
    const volatile std::uint32_t* currentSprite;

    struct Layout
    {
        float horizontalScale;
        float left;
        float right;
        float movieLeft;
        float movieRight;
        float movieTop;
        float movieBottom;
        float clipX;
        float clipY;
    };

    Layout layout;
    const float originalScale = 2.0f;
    const float originalClipX = -320.0f;
    const float originalClipY = -240.0f;
    const float* wideScale = &layout.horizontalScale;

    const BYTE* cameraReturn;
    const BYTE* spriteReturn;
    const BYTE* backgroundReturn;
    const BYTE* movieReturn;
    const BYTE* clipXReturn;
    const BYTE* clipYReturn;

    // The inline bridges call integer-only helpers while the game's SIMD and x87
    // registers are live. Build this file for x86 with /arch:IA32.
    __declspec(noinline) bool __cdecl UseWidescreenLayout()
    {
        const auto* settings = *activeDeviceSettings;
        return settings && settings[PresentParametersWindowed] != 0 &&
               activeBackBuffer->width == wideResolution.width &&
               activeBackBuffer->height == wideResolution.height;
    }

    __declspec(noinline) const float* __cdecl GetSpriteScale()
    {
        if (!UseWidescreenLayout())
            return &originalScale;

        const auto x = currentSprite[0x00 / 4];
        const auto y = currentSprite[0x08 / 4];
        const auto flags = currentSprite[0x38 / 4];
        const auto type = currentSprite[0xb8 / 4];

        // Preserve the horizontal coverage of full-screen overlays and shadows.
        const bool screenQuad = flags == 0x80000 || flags == 0x80008;
        const bool screenFade = (x == 100 && y == 70 && (type == 1 || type == 0x10000)) ||
                                (x == 0 && currentSprite[1] == 0 && y == 6 && type == 1);
        const bool playerShadow = x == 68 && currentSprite[1] == 0 && y == 6 &&
                                  flags == 0x80000 && type == 1;
        return (screenQuad && screenFade) || playerShadow ? &originalScale : wideScale;
    }

    __declspec(naked) void SetCameraProjection()
    {
        __asm {
            pushfd
            pushad
            call UseWidescreenLayout
            test al, al
            jz original
            movss xmm1, dword ptr [layout]
            jmp done
        original:
            movss xmm1, dword ptr [originalScale]
        done:
            popad
            popfd
            jmp dword ptr [cameraReturn]
        }
    }

    __declspec(naked) void SetSpriteProjection()
    {
        __asm {
            pushfd
            pushad
            call GetSpriteScale
            movss xmm5, dword ptr [eax]
            popad
            popfd
            jmp dword ptr [spriteReturn]
        }
    }

    __declspec(naked) void SetBackgroundQuad()
    {
        __asm {
            pushfd
            pushad
            call UseWidescreenLayout
            test al, al
            jz done
            // pushfd + pushad move the game's quad 36 bytes up the stack.
            mov eax, dword ptr [layout.left]
            mov edx, dword ptr [layout.right]
            mov dword ptr [esp+30h], eax
            mov dword ptr [esp+4ch], edx
            // Vertex order is top-left, top-right, bottom-right, bottom-left.
            mov dword ptr [esp+68h], edx
            mov dword ptr [esp+84h], eax
        done:
            popad
            popfd
            mov ecx, dword ptr [eax]
            mov edx, dword ptr [ecx+10ch]
            jmp dword ptr [backgroundReturn]
        }
    }

    __declspec(naked) void SetMovieQuad()
    {
        __asm {
            movss dword ptr [esp+70h], xmm4
            movss dword ptr [esp+74h], xmm0
            pushfd
            pushad
            call UseWidescreenLayout
            test al, al
            jz done
            mov eax, dword ptr [layout.movieLeft]
            mov edx, dword ptr [layout.movieRight]
            mov dword ptr [esp+2ch], eax
            mov dword ptr [esp+48h], edx
            // Vertex order is top-left, top-right, bottom-right, bottom-left.
            mov dword ptr [esp+64h], edx
            mov dword ptr [esp+80h], eax
            mov eax, dword ptr [layout.movieTop]
            mov edx, dword ptr [layout.movieBottom]
            mov dword ptr [esp+30h], eax
            mov dword ptr [esp+4ch], eax
            mov dword ptr [esp+68h], edx
            mov dword ptr [esp+84h], edx
        done:
            popad
            popfd
            jmp dword ptr [movieReturn]
        }
    }

    __declspec(naked) void SetHorizontalClip()
    {
        __asm {
            pushfd
            pushad
            call UseWidescreenLayout
            test al, al
            jz original
            movss xmm0, dword ptr [layout.clipX]
            jmp done
        original:
            movss xmm0, dword ptr [originalClipX]
        done:
            popad
            popfd
            jmp dword ptr [clipXReturn]
        }
    }

    __declspec(naked) void SetVerticalClip()
    {
        __asm {
            pushfd
            pushad
            call UseWidescreenLayout
            test al, al
            jz original
            movss xmm0, dword ptr [layout.clipY]
            jmp done
        original:
            movss xmm0, dword ptr [originalClipY]
        done:
            popad
            popfd
            jmp dword ptr [clipYReturn]
        }
    }

    bool IsWidescreen(Resolution resolution)
    {
        return resolution.width * 9 == resolution.height * 16;
    }

    bool ReadSettings(Settings& settings)
    {
        wchar_t path[MAX_PATH];
        const DWORD length = GetModuleFileNameW(pluginModule, path, MAX_PATH);
        if (length < 4 || length >= MAX_PATH)
            return false;

        std::memcpy(path + length - 4, L".ini", sizeof(L".ini"));
        Resolution& resolution = settings.resolution;
        resolution = {
            GetPrivateProfileIntW(L"MAIN", L"ResX", 0, path),
            GetPrivateProfileIntW(L"MAIN", L"ResY", 0, path),
        };
        settings.cropMovies = GetPrivateProfileIntW(L"MAIN", L"FMVFullscreenMode", 0, path) != 0;

        if (!resolution.width && !resolution.height)
        {
            RECT work = {};
            RECT frame = {};
            if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0) ||
                !AdjustWindowRect(&frame, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE))
                return false;

            const LONG fitWidth = (work.right - work.left - (frame.right - frame.left)) / 4;
            const LONG fitHeight = (work.bottom - work.top - (frame.bottom - frame.top)) / 3;
            const LONG aspectUnits = fitWidth < fitHeight ? fitWidth : fitHeight;
            if (aspectUnits <= 0)
                return false;
            resolution = {static_cast<std::uint32_t>(4 * aspectUnits),
                          static_cast<std::uint32_t>(3 * aspectUnits)};
        }

        return resolution.width >= 640 && resolution.height >= 480 &&
               resolution.width <= 16384 && resolution.height <= 16384 &&
               (resolution.width * 3 == resolution.height * 4 || IsWidescreen(resolution));
    }

    bool IsSupportedEdition(const BYTE* image)
    {
        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(image);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE)
            return false;

        const auto* pe = reinterpret_cast<const IMAGE_NT_HEADERS32*>(image + dos->e_lfanew);
        return pe->Signature == IMAGE_NT_SIGNATURE &&
               pe->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 &&
               pe->FileHeader.TimeDateStamp == ChineseRetailTimestamp &&
               pe->OptionalHeader.SizeOfImage == ChineseRetailImageSize;
    }

    struct Patch
    {
        DWORD rva;
        const BYTE* expected;
        const BYTE* replacement;
        SIZE_T length;
        DWORD protection = 0;
    };

    bool InstallPatches(BYTE* image, Patch* patches, std::size_t count)
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            const Patch& patch = patches[i];
            if (std::memcmp(image + patch.rva, patch.expected, patch.length) != 0)
                return false;
        }

        std::size_t protectedCount = 0;
        while (protectedCount < count)
        {
            Patch& patch = patches[protectedCount];
            if (!VirtualProtect(image + patch.rva, patch.length, PAGE_EXECUTE_READWRITE, &patch.protection))
                break;
            ++protectedCount;
        }

        const bool ready = protectedCount == count;
        if (ready)
        {
            for (std::size_t i = 0; i < count; ++i)
            {
                const Patch& patch = patches[i];
                BYTE* destination = image + patch.rva;
                std::memcpy(destination, patch.replacement, patch.length);
                FlushInstructionCache(GetCurrentProcess(), destination, patch.length);
            }
        }

        // Some patches share a page. Restore protection in reverse order.
        while (protectedCount)
        {
            Patch& patch = patches[--protectedCount];
            DWORD unused = 0;
            VirtualProtect(image + patch.rva, patch.length, patch.protection, &unused);
        }
        return ready;
    }

    template<std::size_t Size>
    std::array<BYTE, Size> JumpTo(const BYTE* from, void (*target)())
    {
        static_assert(Size >= 5);
        std::array<BYTE, Size> code;
        code.fill(0x90);
        code[0] = 0xe9;
        const auto displacement = reinterpret_cast<std::uintptr_t>(target) -
                                  reinterpret_cast<std::uintptr_t>(from + 5);
        std::memcpy(code.data() + 1, &displacement, sizeof(displacement));
        return code;
    }

    void Install(BYTE* image, Settings settings)
    {
        const Resolution resolution = settings.resolution;
        BYTE startup[sizeof(OriginalStartup)];
        BYTE fallback[sizeof(OriginalFallback)];
        std::memcpy(startup, OriginalStartup, sizeof(startup));
        std::memcpy(fallback, OriginalFallback, sizeof(fallback));
        std::memcpy(startup + 1, &resolution.height, sizeof(resolution.height));
        std::memcpy(startup + 6, &resolution.width, sizeof(resolution.width));
        std::memcpy(fallback + 1, &resolution.width, sizeof(resolution.width));
        std::memcpy(fallback + 6, &resolution.height, sizeof(resolution.height));

        if (!IsWidescreen(resolution))
        {
            Patch patches[] = {
                {StartupResolution, OriginalStartup, startup, sizeof(startup)},
                {WindowedFallback, OriginalFallback, fallback, sizeof(fallback)},
            };
            InstallPatches(image, patches, ARRAYSIZE(patches));
            return;
        }

        wideResolution = resolution;
        activeDeviceSettings = reinterpret_cast<decltype(activeDeviceSettings)>(image + DeviceSettingsPointer);
        activeBackBuffer = reinterpret_cast<decltype(activeBackBuffer)>(image + BackBufferDimensions);
        currentSprite = reinterpret_cast<decltype(currentSprite)>(image + CurrentSprite);

        const float width = static_cast<float>(resolution.width);
        const float height = static_cast<float>(resolution.height);
        const float width43 = height * (4.0f / 3.0f);
        const float margin = (width - width43) * 0.5f;
        const float movieMarginY = (width * (3.0f / 4.0f) - height) * 0.5f;
        layout.horizontalScale = 2.0f * width43 / width;
        layout.left = margin;
        layout.right = margin + width43;
        layout.movieLeft = settings.cropMovies ? 0.0f : layout.left;
        layout.movieRight = settings.cropMovies ? width : layout.right;
        layout.movieTop = settings.cropMovies ? -movieMarginY : 0.0f;
        layout.movieBottom = settings.cropMovies ? height + movieMarginY : height;
        layout.clipX = -width;
        layout.clipY = -height;

        cameraReturn = image + CameraProjection + sizeof(OriginalCamera);
        spriteReturn = image + SpriteProjection + sizeof(OriginalSprite);
        backgroundReturn = image + BackgroundQuad + sizeof(OriginalBackground);
        movieReturn = image + MovieQuad + sizeof(OriginalMovie);
        clipXReturn = image + HorizontalClip + sizeof(OriginalHorizontalClip);
        clipYReturn = image + VerticalClip + sizeof(OriginalVerticalClip);

        const auto camera = JumpTo<sizeof(OriginalCamera)>(image + CameraProjection, SetCameraProjection);
        const auto sprite = JumpTo<sizeof(OriginalSprite)>(image + SpriteProjection, SetSpriteProjection);
        const auto background = JumpTo<sizeof(OriginalBackground)>(image + BackgroundQuad, SetBackgroundQuad);
        const auto movie = JumpTo<sizeof(OriginalMovie)>(image + MovieQuad, SetMovieQuad);
        const auto clipX = JumpTo<sizeof(OriginalHorizontalClip)>(image + HorizontalClip, SetHorizontalClip);
        const auto clipY = JumpTo<sizeof(OriginalVerticalClip)>(image + VerticalClip, SetVerticalClip);
        Patch patches[] = {
            {StartupResolution, OriginalStartup, startup, sizeof(startup)},
            {WindowedFallback, OriginalFallback, fallback, sizeof(fallback)},
            {CameraProjection, OriginalCamera, camera.data(), camera.size()},
            {SpriteProjection, OriginalSprite, sprite.data(), sprite.size()},
            {BackgroundQuad, OriginalBackground, background.data(), background.size()},
            {MovieQuad, OriginalMovie, movie.data(), movie.size()},
            {HorizontalClip, OriginalHorizontalClip, clipX.data(), clipX.size()},
            {VerticalClip, OriginalVerticalClip, clipY.data(), clipY.size()},
        };
        InstallPatches(image, patches, ARRAYSIZE(patches));
    }
}

extern "C" __declspec(dllexport) void InitializeASI()
{
    BYTE* image = reinterpret_cast<BYTE*>(GetModuleHandleW(nullptr));
    Settings settings = {};
    if (image && IsSupportedEdition(image) && ReadSettings(settings))
        Install(image, settings);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
        pluginModule = instance;
    return TRUE;
}

