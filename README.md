# 鬼武者 3 简体中文版修复

适用于 2005 年简体中文 PC 版 1.0.0.1。安装包包含窗口和宽屏修正、游戏注册表配置，以及动物园破魔镜报错修复。宽屏补丁也修正了 16:9 下过场 CG 和读档背景折叠的问题。

## 下载与安装

1. 退出游戏和启动器。[下载一键安装包](https://raw.githubusercontent.com/DangoYoung/Onimusha3-ZH-CN-Fixes/main/downloads/Onimusha3-ZH-CN-Fixes.zip)
2. 把 ZIP 里的全部内容解压到 `oni3.exe` 所在的游戏根目录。确认 `Install-Fixes.cmd` 和 `Onimusha3-ZH-CN-Fixes` 文件夹都与 `oni3.exe` 并列。
3. 双击 `Install-Fixes.cmd`，允许管理员权限以写入注册表。看到 `Installation complete` 后，双击 `oni3.exe` 启动游戏。

安装脚本会把 ASI 放进 `scripts`，把加载器 `dinput8.dll` 放到游戏根目录，并设置安装路径和简体中文。若需替换已有的加载器或宽屏 ASI，脚本会先为对应文件保留 `*.before-fixes-*.bak` 备份。

## 动物园破魔镜

使用动物园研究所幻魔科学家 Boss 房门前的破魔镜打开「保存」或「强化」时，游戏可能弹出 `Error .\class\c_moji.cpp:3045`。安装脚本会校验本机游戏资源，并修正 `Image\Area\area809.ldt` 的简中字形索引。

如需还原破魔镜资源，退出游戏后，将同目录的 `area809.ldt.before-mirror-fix.bak` 复制为 `area809.ldt` 并覆盖当前文件。

## 设置窗口分辨率

默认窗口为 **1200 × 900**。如需自定义尺寸，在游戏的 `scripts` 目录中编辑或创建 `Onimusha3.WidescreenFix.ini`，保存后重启游戏。

```ini
[MAIN]
ResX = 1600
ResY = 900
FMVFullscreenMode = 0
```

支持精确的 4:3 和 16:9，例如 1024 × 768、1280 × 720 和 1600 × 900。将 `ResX` 和 `ResY` 都设为 `0` 可按主显示器可用空间自动选择 4:3 窗口。过场参数为 `0` 时保留完整画面，为 `1` 时裁掉上下边缘以填满宽屏窗口。

## 编译与许可

在 Visual Studio 2022 的 **x86 Native Tools Command Prompt** 中进入仓库目录，运行 `build.cmd`。编译结果为 `scripts\Onimusha3.WidescreenFix.asi`。

简中版宽屏适配和画面折叠修正完成于 2026-11-19。本适配版按 [GPL-3.0](LICENSE) 发布，参考了 [ThirteenAG 的宽屏补丁](https://github.com/ThirteenAG/WidescreenFixesPack/releases/tag/onimusha3)。安装包内的 `dinput8.dll` 是原补丁包所附的 Win32 [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)。 [上游宽屏补丁许可声明](licenses/WidescreenFix-MIT.txt) · [加载器 MIT 许可](licenses/Ultimate-ASI-Loader-MIT.txt)
