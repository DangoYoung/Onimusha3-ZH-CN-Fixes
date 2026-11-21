# 鬼武者 3 简体中文版修复

适用于 2005 年简体中文 PC 版。`Configure-Registry.cmd` 写入游戏安装路径和简体中文设置。`Onimusha3.WidescreenFix.cpp` 是宽屏补丁源码，修复了 16:9 下过场 CG 与读档背景折叠的问题。

## 下载与安装

退出游戏后下载所需文件，按下表放置。

| 文件 | 放置位置 |
| --- | --- |
| [Configure-Registry.cmd](https://github.com/DangoYoung/Onimusha3-ZH-CN-Fixes/raw/HEAD/downloads/Configure-Registry.cmd) | `oni3.exe` 同目录 |
| [Onimusha3.WidescreenFix.asi](https://github.com/DangoYoung/Onimusha3-ZH-CN-Fixes/raw/HEAD/downloads/Onimusha3.WidescreenFix.asi) | 游戏目录下的 `scripts` 文件夹 |
| [dinput8.dll](https://github.com/DangoYoung/Onimusha3-ZH-CN-Fixes/raw/HEAD/downloads/dinput8.dll) | `oni3.exe` 同目录 |

如需补写注册表，右键以管理员身份运行 `Configure-Registry.cmd`。双击 `oni3.exe` 即可直接启动游戏，绕过原启动器的 “Cannot Find the Game” 报错。

## 修复动物园破魔镜报错

简体中文 PC 版 1.0.0.1 中，使用动物园研究所幻魔科学家 Boss 房门前的破魔镜打开「保存」或「强化」界面时，可能弹出 `Error .\class\c_moji.cpp:3045`。补丁修正该房间的简中字形索引，可单独使用。

退出游戏后，[下载 Fix-Mirror.ps1](https://raw.githubusercontent.com/DangoYoung/Onimusha3-ZH-CN-Fixes/main/Fix-Mirror.ps1)，把脚本放到 `oni3.exe` 同目录。在游戏目录打开命令提示符或 PowerShell，运行以下命令。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Fix-Mirror.ps1
```

脚本会校验游戏资源，使用 `Image\Area\area801.ldt` 修正 `Image\Area\area809.ldt`，并在同目录备份原文件为 `area809.ldt.before-mirror-fix.bak`。如果需要还原，退出游戏后把备份复制为 `area809.ldt` 并覆盖当前文件。

## 设置窗口分辨率

需要指定尺寸时，在 `scripts` 中创建 `Onimusha3.WidescreenFix.ini`，写入以下内容后重启游戏。

```ini
[MAIN]
ResX = 1600
ResY = 900
FMVFullscreenMode = 0
```

此版本支持精确的 4:3 和 16:9，例如 `1024 × 768`、`1280 × 720` 和 `1600 × 900`。两项分辨率均为 `0` 或省略配置文件时，会自动选择适合主显示器的 4:3 尺寸。过场参数为 `0` 时保留完整画面，为 `1` 时裁掉上下边缘以填满宽屏窗口。

## 编译与许可

在 Visual Studio 2022 的 **x86 Native Tools Command Prompt** 中进入仓库目录，执行以下命令。

```bat
cl /nologo /W4 /WX /O2 /MT /LD /EHsc /std:c++17 /arch:IA32 Onimusha3.WidescreenFix.cpp /link /OUT:Onimusha3.WidescreenFix.asi user32.lib
```

简中版适配和画面折叠修正完成于 2026-11-19。本适配版按 [GPL-3.0](LICENSE) 发布，参考了 [ThirteenAG 的宽屏补丁](https://github.com/ThirteenAG/WidescreenFixesPack/releases/tag/onimusha3)。下载区的 `dinput8.dll` 是原补丁包所附的 Win32 [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)。 [上游宽屏补丁许可声明](licenses/WidescreenFix-MIT.txt) · [加载器 MIT 许可](licenses/Ultimate-ASI-Loader-MIT.txt)
