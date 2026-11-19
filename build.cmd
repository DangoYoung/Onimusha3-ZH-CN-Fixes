@echo off
setlocal DisableDelayedExpansion
pushd "%~dp0" || exit /b 1
if /i not "%VSCMD_ARG_TGT_ARCH%"=="x86" (
    >&2 echo Run this script in the Visual Studio x86 Native Tools Command Prompt.
    popd
    exit /b 1
)
if not exist build mkdir build
cl /nologo /W4 /WX /O2 /MT /LD /EHsc /std:c++17 /arch:IA32 /Fo"build\\" Onimusha3.WidescreenFix.cpp /link /OUT:scripts\Onimusha3.WidescreenFix.asi /IMPLIB:build\Onimusha3.WidescreenFix.lib user32.lib
set "buildResult=%errorlevel%"
popd
exit /b %buildResult%

