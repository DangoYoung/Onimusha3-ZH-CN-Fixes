# 鬼武者 3 简体中文版窗口与启动修复

适配《鬼武者 3：Demon Siege》2005 年简体中文 PC 版。宽屏补丁支持 4:3 和 16:9 窗口分辨率，并修复了 16:9 下过场 CG 和读档背景出现三角形缺口的问题。兼容启动脚本从游戏目录运行 `oni3.exe`，可绕过原启动器报出的 “Cannot Find the Game”。

## 安装

1. 退出游戏，下载 [Onimusha3-ZH-CN-Fixes-v1.0.0.zip](https://github.com/DangoYoung/Onimusha3-ZH-CN-Fixes/raw/HEAD/downloads/Onimusha3-ZH-CN-Fixes-v1.0.0.zip)。把压缩包里的 `scripts` 文件夹和两个 `.cmd` 文件放到 `oni3.exe` 所在目录。已装过原版宽屏补丁的用户可先备份旧的 `.asi` 和 `.ini`，再替换同名文件。
2. 下载 [32 位 dinput8.dll](https://github.com/DangoYoung/Onimusha3-ZH-CN-Fixes/raw/HEAD/downloads/dinput8.dll)，把它放到 `oni3.exe` 所在目录。这个 DLL 是运行宽屏补丁所需的 Ultimate ASI Loader。v1.0.0 补丁 ZIP 和加载器分别下载。
3. 双击 `Start-Onimusha3.cmd` 或 `oni3.exe`。兼容启动脚本以自身目录为工作目录，无需管理员权限。它也可以脱离宽屏补丁单独使用。

如果需要为免安装版补写注册表，右键以管理员身份运行 `Configure-Registry.cmd`。它只写入该游戏的 32 位 `InstallPath` 和 `Language`。原版 `Oni3Launcher.exe` 在部分系统上会额外请求注册表写权限，遇到该问题时请使用兼容启动脚本。

## 窗口与过场设置

在 `scripts/Onimusha3.WidescreenFix.ini` 中修改 `ResX` 和 `ResY` 后重启游戏。预设为 `1600 × 900`。可用的示例还有 `1280 × 720`、`1200 × 900`。两项都设为 `0` 时按主显示器的可用空间自动选择 4:3 尺寸。

`FMVFullscreenMode = 0` 保留过场影片完整画面，16:9 窗口中两侧可能有黑边。改为 `1` 时按比例铺满并裁掉上下部分。

## 适用版本与验证

已在 Windows 11 上用以下简体中文版执行文件核对 1600 × 900 的 Direct3D 9 后备缓冲区、开场 CG 和本能寺存档的读档界面。

```text
oni3.exe SHA-256
4f4da70b6155b8617f0d777e6c28338a70ab314e1c556e25e41151c37b34b5b2
```

补丁在写入内存前检查 PE 版本和目标指令。其他发行版请使用相应的补丁。若要停用宽屏修复，退出游戏后移走 `scripts/Onimusha3.WidescreenFix.asi`。存档在游戏自身的 `save` 目录。

## 编译

使用 Visual Studio 2022 的 x86 Native Tools Command Prompt，在仓库根目录运行 `build.cmd`。构建生成 `scripts/Onimusha3.WidescreenFix.asi`。

## 上游与许可证

宽屏布局参考 [ThirteenAG 的 Onimusha 3 Widescreen Fix](https://github.com/ThirteenAG/WidescreenFixesPack/releases/tag/onimusha3)。简中版移植和过场、读档顶点顺序修正于 2026-11-19 完成。本仓库按 GPL-3.0 发布，详见 [LICENSE](LICENSE) 和 [第三方说明](NOTICE.md)。发布包提供补丁和对应源码。加载器单独提供，游戏本体和游戏资源请从各自发行渠道获取。

