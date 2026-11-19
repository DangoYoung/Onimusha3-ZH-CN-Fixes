@echo off
setlocal DisableDelayedExpansion
if not exist "%~dp0oni3.exe" (
    echo Cannot find "%~dp0oni3.exe".
    echo Put this launcher next to oni3.exe.
    pause
    exit /b 2
)
start "" /d "%~dp0." "%~dp0oni3.exe"
exit /b %errorlevel%

