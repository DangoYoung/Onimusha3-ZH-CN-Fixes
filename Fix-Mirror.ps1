#Requires -Version 5.1
[CmdletBinding()]
param(
    [string]$GamePath
)

$ErrorActionPreference = 'Stop'

function Get-Sha256([byte[]]$Bytes) {
    $algorithm = [Security.Cryptography.SHA256]::Create()
    try {
        [BitConverter]::ToString($algorithm.ComputeHash($Bytes)).Replace('-', '')
    } finally {
        $algorithm.Dispose()
    }
}

if (-not $GamePath) {
    $GamePath = $PSScriptRoot
}
if (-not (Test-Path -LiteralPath (Join-Path $GamePath 'oni3.exe') -PathType Leaf)) {
    throw 'Place Fix-Mirror.ps1 next to oni3.exe, or specify the game folder with -GamePath.'
}

$areaPath = (Resolve-Path -LiteralPath (Join-Path $GamePath 'Image\Area')).ProviderPath
$roomPath = Join-Path $areaPath 'area809.ldt'
$referencePath = Join-Path $areaPath 'area801.ldt'
$backupPath = "$roomPath.before-mirror-fix.bak"

$originalHash = '80437AC373F843AE3A41ECEFEE5D86AE806BF49F89AAE218EC06A426DF8DC72B'
$referenceHash = '2F015E0C83DB0A43482DB861951A17A263496F2A5D377D4FC115FCF688069FAC'
$fixedHash = '02C70A5FA56F1A6111EA408338F0E5C3168D86FF8D7422795FA9C410FCE10B82'

$pathHash = Get-Sha256 ([Text.Encoding]::UTF8.GetBytes($roomPath.ToUpperInvariant()))
$mutex = [Threading.Mutex]::new($false, "Local\Onimusha3MirrorFix_$pathHash")
$hasLock = $false
$stagedPath = $null
try {
    try {
        $hasLock = $mutex.WaitOne(0)
    } catch [Threading.AbandonedMutexException] {
        $hasLock = $true
    }
    if (-not $hasLock) {
        throw 'The mirror fix is already running for this game folder.'
    }

    $room = [IO.File]::ReadAllBytes($roomPath)
    $roomHash = Get-Sha256 $room
    if ($roomHash -eq $fixedHash) {
        Write-Host 'The mirror fix is already installed.'
        return
    }
    if ($roomHash -ne $originalHash) {
        throw 'Unrecognized area809.ldt. This fix requires the matching Simplified Chinese PC resources.'
    }

    $reference = [IO.File]::ReadAllBytes($referencePath)
    if ((Get-Sha256 $reference) -ne $referenceHash) {
        throw 'Unrecognized area801.ldt. Restore the matching Simplified Chinese PC resource before retrying.'
    }

    # The rooms share the same glyph texture. Copy only the Simplified Chinese
    # text block, where area801 uses the matching glyph bank selectors.
    [Array]::Copy($reference, 0x147CA0, $room, 0x112670, 0xB00)
    if ((Get-Sha256 $room) -ne $fixedHash) {
        throw 'The patched resource failed verification.'
    }

    $backupForReplace = [NullString]::Value
    if (Test-Path -LiteralPath $backupPath) {
        if ((Get-Sha256 ([IO.File]::ReadAllBytes($backupPath))) -ne $originalHash) {
            throw "The existing backup does not match the original resource. Move it aside before retrying: $backupPath"
        }
    } else {
        $backupForReplace = $backupPath
    }

    $stagedPath = Join-Path $areaPath ("area809.{0}.tmp" -f [guid]::NewGuid().ToString('N'))
    [IO.File]::WriteAllBytes($stagedPath, $room)
    if (Get-Process -Name oni3,Oni3Launcher -ErrorAction SilentlyContinue) {
        throw 'Close the game and launcher before installing the fix.'
    }
    if ((Get-Sha256 ([IO.File]::ReadAllBytes($roomPath))) -ne $originalHash) {
        throw 'area809.ldt changed during installation. Retry after closing other game tools.'
    }

    [IO.File]::Replace($stagedPath, $roomPath, $backupForReplace)
    Write-Host 'The mirror fix is installed.'
    Write-Host "Original resource: $backupPath"
} finally {
    try {
        if ($stagedPath -and [IO.File]::Exists($stagedPath)) {
            [IO.File]::Delete($stagedPath)
        }
    } finally {
        if ($hasLock) {
            $mutex.ReleaseMutex()
        }
        $mutex.Dispose()
    }
}
