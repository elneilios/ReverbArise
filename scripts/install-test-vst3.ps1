[CmdletBinding(SupportsShouldProcess = $true)]
param(
    # USERPROFILE keeps the real user's install path stable when an agent's
    # sandbox redirects LOCALAPPDATA into a disposable package directory.
    [string] $Destination = (Join-Path $env:USERPROFILE 'AppData/Local/Programs/Common/VST3'),
    [switch] $Test
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'build-support.ps1')
. (Join-Path $PSScriptRoot 'release-support.ps1')
. (Join-Path $PSScriptRoot 'test-install-support.ps1')
$root = Split-Path $PSScriptRoot -Parent
try {
    $destinationPath = [IO.Path]::GetFullPath($Destination)
    if (-not $PSCmdlet.ShouldProcess((Join-Path $destinationPath 'ReverbArise.vst3'), 'Build Release and install for local FL Studio testing')) { exit 0 }
    Assert-TestHostClosed
    # Build through the supported wrapper; a failed build/test never installs stale output.
    $code = Invoke-ReverbBuild -Root $root -Configuration Release -Test:$Test
    if ($code -ne 0) { exit $code }
    $presets = Get-Content -LiteralPath (Join-Path $root 'CMakePresets.json') -Raw | ConvertFrom-Json
    $preset = @($presets.configurePresets | Where-Object { $_.name -eq 'windows-msvc' })[0]
    $binary = [IO.Path]::GetFullPath($preset.binaryDir.Replace('${sourceDir}', $root).Replace('${presetName}', 'windows-msvc'))
    $lock = Enter-BuildLock $binary
    try {
        $identity = Get-Content -LiteralPath (Join-Path $binary 'product-identity.json') -Raw | ConvertFrom-Json
        $bundle = Get-ReleaseBundle $binary $identity.version
        Install-TestBundle -Source $bundle -Destination $destinationPath -BackupRoot (Join-Path $root 'out/vst3-install-backups')
    } finally { $lock.Dispose() }
    Write-Host 'PASS: Release installed. Reopen FL Studio; rescan plugins if needed. FL Studio acceptance is still manual.'
    exit 0
} catch {
    Write-Host "FAIL test install: $($_.Exception.Message)"
    exit 1
}
