#Requires -Version 5.1
[CmdletBinding()]
param([switch]$PauseOnExit)

$ErrorActionPreference = 'Stop'

function Get-FileSha256([string]$Path) {
    $algorithm = [Security.Cryptography.SHA256]::Create()
    try {
        $stream = [IO.File]::OpenRead($Path)
        try {
            [BitConverter]::ToString($algorithm.ComputeHash($stream)).Replace('-', '')
        } finally {
            $stream.Dispose()
        }
    } finally {
        $algorithm.Dispose()
    }
}

function Install-GameFile([string]$Source, [string]$Destination) {
    $backup = $null
    $existingHash = $null
    if ([IO.File]::Exists($Destination)) {
        $existingHash = Get-FileSha256 $Destination
        if ($existingHash -eq (Get-FileSha256 $Source)) {
            return
        }
        $backup = "$Destination.before-fixes-$($existingHash.Substring(0, 12)).bak"
        if ([IO.File]::Exists($backup)) {
            if ((Get-FileSha256 $backup) -ne $existingHash) {
                throw "A different backup already exists at $backup"
            }
            $backup = $null
        }
    }

    $staged = "$Destination.$([guid]::NewGuid().ToString('N')).tmp"
    try {
        [IO.File]::Copy($Source, $staged)
        if ($existingHash) {
            $backupPath = if ($backup) { $backup } else { [NullString]::Value }
            [IO.File]::Replace($staged, $Destination, $backupPath)
        } else {
            [IO.File]::Move($staged, $Destination)
        }
    } finally {
        if ([IO.File]::Exists($staged)) {
            [IO.File]::Delete($staged)
        }
    }
}

function Set-GameRegistry([string]$GamePath) {
    $root = [Microsoft.Win32.RegistryKey]::OpenBaseKey(
        [Microsoft.Win32.RegistryHive]::LocalMachine, [Microsoft.Win32.RegistryView]::Registry32)
    try {
        $key = $root.CreateSubKey('SOFTWARE\CAPCOM\ONIMUSHA3 PC\1.00.000')
        try {
            $key.SetValue('InstallPath', $GamePath, [Microsoft.Win32.RegistryValueKind]::String)
            $key.SetValue('Language', 'Simplified Chinese', [Microsoft.Win32.RegistryValueKind]::String)
        } finally {
            $key.Dispose()
        }
    } finally {
        $root.Dispose()
    }
}

$exitCode = 0
try {
    $payload = $PSScriptRoot
    $gamePath = Split-Path -Parent $payload
    if (-not [IO.File]::Exists((Join-Path $gamePath 'oni3.exe'))) {
        throw 'Extract the entire ZIP into the folder containing oni3.exe.'
    }

    $asiName = 'Onimusha3.WidescreenFix.asi'
    $loader = Join-Path $payload 'dinput8.dll'
    $plugin = Join-Path $payload $asiName
    $mirrorFix = Join-Path $payload 'Fix-Mirror.ps1'
    foreach ($file in @($loader, $plugin, $mirrorFix)) {
        if (-not [IO.File]::Exists($file)) {
            throw "Missing installer file: $file"
        }
    }
    if (Get-Process -Name oni3,Oni3Launcher -ErrorAction SilentlyContinue) {
        throw 'Close the game and launcher, then run the installer again.'
    }

    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        $arguments = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', ('"{0}"' -f $PSCommandPath))
        if ($PauseOnExit) {
            $arguments += '-PauseOnExit'
        }
        $child = Start-Process -FilePath (Join-Path $PSHOME 'powershell.exe') -ArgumentList $arguments -Verb RunAs -Wait -PassThru
        exit $child.ExitCode
    }

    $mutex = [Threading.Mutex]::new($false, 'Local\Onimusha3ZhCnFixesInstaller')
    $hasLock = $false
    try {
        try {
            $hasLock = $mutex.WaitOne(0)
        } catch [Threading.AbandonedMutexException] {
            $hasLock = $true
        }
        if (-not $hasLock) {
            throw 'The installer is already running.'
        }

        & $mirrorFix -GamePath $gamePath
        $scriptsPath = Join-Path $gamePath 'scripts'
        [void][IO.Directory]::CreateDirectory($scriptsPath)
        Install-GameFile $loader (Join-Path $gamePath 'dinput8.dll')
        Install-GameFile $plugin (Join-Path $scriptsPath $asiName)
        Set-GameRegistry $gamePath
    } finally {
        if ($hasLock) {
            $mutex.ReleaseMutex()
        }
        $mutex.Dispose()
    }

    Write-Host 'Installation complete. Start oni3.exe.'
} catch {
    Write-Host "Installation failed: $($_.Exception.Message)" -ForegroundColor Red
    $exitCode = 1
}
if ($PauseOnExit) {
    [void](Read-Host 'Press Enter to close')
}
exit $exitCode
