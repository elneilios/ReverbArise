$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
. (Join-Path $root 'scripts/release-support.ps1')
. (Join-Path $root 'scripts/test-install-support.ps1')
$fixture = Join-Path $root ('out/test-install-fixtures/' + [guid]::NewGuid().ToString('N'))
$source = Join-Path $fixture 'build with spaces/ReverbArise.vst3'
$destination = Join-Path $fixture 'user VST3'
$backups = Join-Path $fixture 'backups'
$null = New-Item -ItemType Directory -Path (Join-Path $source 'Contents/x86_64-win') -Force
$null = New-Item -ItemType Directory -Path (Join-Path $source 'Contents/Resources') -Force
Set-Content -LiteralPath (Join-Path $source 'Contents/x86_64-win/ReverbArise.vst3') -Value 'first binary'
Set-Content -LiteralPath (Join-Path $source 'Contents/Resources/notice.txt') -Value 'resource'
# Isolate fixtures from the owner's actual host; verify the process guard separately below.
function Assert-TestHostClosed { }
Install-TestBundle $source $destination $backups
$target = Join-Path $destination 'ReverbArise.vst3'
Assert-TestBundleMatch $source $target
Set-Content -LiteralPath (Join-Path $target 'stale.txt') -Value 'old extra resource'
$old = @(Get-ReleaseInventory $target) | ConvertTo-Json -Depth 4 -Compress
Set-Content -LiteralPath (Join-Path $source 'Contents/x86_64-win/ReverbArise.vst3') -Value 'second binary'
Install-TestBundle $source $destination $backups
Assert-TestBundleMatch $source $target
if (Test-Path -LiteralPath (Join-Path $target 'stale.txt')) { throw 'Stale files survived replacement.' }
$saved = @(Get-ChildItem -LiteralPath $backups -Directory)
if ($saved.Count -ne 1) { throw 'Expected one previous installation.' }
$actual = @(Get-ReleaseInventory (Join-Path $saved[0].FullName 'ReverbArise.vst3')) | ConvertTo-Json -Depth 4 -Compress
if ($old -cne $actual) { throw 'Previous installation was not preserved.' }
$held = [IO.File]::Open((Join-Path $destination '.reverbarise-install.lock'), 'Open', 'ReadWrite', 'None')
try {
    $failed = $false
    try { Install-TestBundle $source $destination $backups } catch { $failed = $true }
    if (-not $failed) { throw 'Concurrent install was allowed.' }
} finally { $held.Dispose() }
$failed = $false
try { Install-TestBundle (Join-Path $fixture 'missing') $destination $backups } catch { $failed = $true }
if (-not $failed) { throw 'Missing source was allowed.' }
Assert-TestBundleMatch $source $target
. (Join-Path $root 'scripts/test-install-support.ps1')
function Assert-TestHostClosed { }
$script:moveCount = 0
function Move-TestInstallDirectory {
    param($Source, $Destination)
    $script:moveCount++
    if ($script:moveCount -eq 2) { throw 'Simulated final rename failure' }
    [IO.Directory]::Move($Source, $Destination)
}
$failed = $false
try { Install-TestBundle $source $destination $backups } catch { $failed = $_.Exception.Message -eq 'Simulated final rename failure' }
if (-not $failed -or $script:moveCount -ne 3) { throw 'Failed install did not attempt rollback.' }
. (Join-Path $root 'scripts/test-install-support.ps1')
Assert-TestBundleMatch $source $target
# The entrypoint must propagate failed build/CTest without touching a destination.
$fake = Join-Path $fixture 'failed checkout/scripts'
$null = New-Item -ItemType Directory -Path $fake -Force
Copy-Item -LiteralPath (Join-Path $root 'scripts/install-test-vst3.ps1'), (Join-Path $root 'scripts/release-support.ps1'), (Join-Path $root 'scripts/test-install-support.ps1') -Destination $fake
Set-Content -LiteralPath (Join-Path $fake 'build-support.ps1') -Value 'function Invoke-ReverbBuild { param($Root, $Configuration, [switch] $Test) if ($Configuration -ne "Release" -or -not $Test) { throw "Expected Release CTest" }; return 29 }'
Add-Content -LiteralPath (Join-Path $fake 'test-install-support.ps1') -Value 'function Assert-TestHostClosed { }'
$untouched = Join-Path $fixture 'must not install'
$shell = if ($PSVersionTable.PSEdition -eq 'Core') { Join-Path $PSHOME 'pwsh.exe' } else { Join-Path $PSHOME 'powershell.exe' }
& $shell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $fake 'install-test-vst3.ps1') -Destination $untouched -Test
if ($LASTEXITCODE -ne 29 -or (Test-Path -LiteralPath $untouched)) { throw 'Failed build/test did not stop installation.' }
. (Join-Path $root 'scripts/test-install-support.ps1')
function Assert-TestHostClosed { }
function Assert-TestBundleMatch { param($Source, $Copy) throw 'Simulated staging verification failure' }
$failed = $false
try { Install-TestBundle $source $destination $backups } catch { $failed = $_.Exception.Message -eq 'Simulated staging verification failure' }
if (-not $failed) { throw 'Unverified staging was accepted.' }
. (Join-Path $root 'scripts/test-install-support.ps1')
function Assert-TestHostClosed { }
Assert-TestBundleMatch $source $target
. (Join-Path $root 'scripts/test-install-support.ps1')
function Get-Process { param($Name, $ErrorAction) [pscustomobject]@{ ProcessName = 'FL64' } }
$failed = $false
try { Install-TestBundle $source $destination $backups } catch { $failed = $_.Exception.Message -like 'Close FL Studio*' }
if (-not $failed) { throw 'Running FL Studio was not rejected.' }
Remove-Item Function:Get-Process
if (@(Get-ChildItem -LiteralPath $destination -Directory -Filter '.reverbarise-stage-*').Count -ne 0) { throw 'Staging directories leaked.' }
Write-Host 'PASS fixtures: first install, exact replacement, complete backup, destination lock, missing source, verification failure, rollback, failed build/test, host guard.'
