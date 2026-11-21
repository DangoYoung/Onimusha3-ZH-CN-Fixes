@echo off
setlocal DisableDelayedExpansion
if not exist "%~dp0oni3.exe" (
    echo Cannot find "%~dp0oni3.exe".
    echo Put this script next to oni3.exe.
    pause
    exit /b 2
)
for %%I in ("%~dp0.") do set "gameFolder=%%~fI"
set "gameKey=HKLM\SOFTWARE\CAPCOM\ONIMUSHA3 PC\1.00.000"
reg.exe add "%gameKey%" /v InstallPath /t REG_SZ /d "%gameFolder%" /f /reg:32
if errorlevel 1 goto failed
reg.exe add "%gameKey%" /v Language /t REG_SZ /d "Simplified Chinese" /f /reg:32
if errorlevel 1 goto failed
reg.exe query "%gameKey%" /reg:32
pause
exit /b 0
:failed
echo Registry configuration failed. Run this script as Administrator.
pause
exit /b 1

