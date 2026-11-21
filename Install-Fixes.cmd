@echo off
setlocal DisableDelayedExpansion
set "installer=%~dp0Onimusha3-ZH-CN-Fixes\Install-Fixes.ps1"
if not exist "%installer%" (
    echo Extract the entire ZIP into the folder containing oni3.exe.
    pause
    exit /b 1
)
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy Bypass -File "%installer%" -PauseOnExit
exit /b %errorlevel%
