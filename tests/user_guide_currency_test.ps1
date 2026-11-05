# Content currency only; no native build or semantic-review claim.
$ErrorActionPreference = 'Stop'
function File-Hash([string] $Path) {
    $hasher=[Security.Cryptography.SHA256]::Create()
    try { return [BitConverter]::ToString($hasher.ComputeHash([IO.File]::ReadAllBytes($Path))) }
    finally { $hasher.Dispose() }
}
$root = Split-Path $PSScriptRoot -Parent
. (Join-Path $root 'scripts/user-guide-support.ps1')
$fixture = Join-Path $root ('out/guide-currency-fixtures/' + [guid]::NewGuid().ToString('N'))
foreach ($folder in @('Source','openspec/specs/fixture','docs/development')) { $null=New-Item -ItemType Directory -Path (Join-Path $fixture $folder) -Force }
[IO.File]::WriteAllText((Join-Path $fixture 'Source/feature.cpp'),"first`nsecond`n")
'fixture specification' | Set-Content (Join-Path $fixture 'openspec/specs/fixture/spec.md')
'project(Preverb VERSION 0.1.0 LANGUAGES C CXX)' | Set-Content (Join-Path $fixture 'CMakeLists.txt')
'<html>fixture guide</html>' | Set-Content (Join-Path $fixture 'docs/development/alpha-guide.html')
function Expect-Stale([string] $Reason) {
    $failed=$false
    try { Assert-UserGuideCurrent $fixture } catch { $failed=$true }
    if (-not $failed) { throw "Guide currency accepted $Reason" }
}
function Record-Review { & (Join-Path $root 'scripts/review-user-guide.ps1') -Root $fixture -Summary 'Fixture review only' }
Expect-Stale 'missing review'
Record-Review
$reviewPath=Join-Path $fixture 'docs/development/user-guide-review.json'
$originalHash=File-Hash $reviewPath
Assert-UserGuideCurrent $fixture
if ((File-Hash $reviewPath) -cne $originalHash) { throw 'Read-only check modified review' }
[IO.File]::WriteAllText((Join-Path $fixture 'Source/feature.cpp'),"first`r`nsecond`r`n")
Assert-UserGuideCurrent $fixture
foreach ($relative in @('Source/feature.cpp','openspec/specs/fixture/spec.md','docs/development/alpha-guide.html')) {
    Add-Content (Join-Path $fixture $relative) 'changed'
    Expect-Stale $relative
    if ((File-Hash $reviewPath) -cne $originalHash) { throw 'Failed check rewrote review' }
    Record-Review; $originalHash=File-Hash $reviewPath
}
'new behavior' | Set-Content (Join-Path $fixture 'Source/new.cpp')
Expect-Stale 'added code'
Record-Review
Remove-Item -LiteralPath (Join-Path $fixture 'Source/new.cpp')
Expect-Stale 'deleted code'
Record-Review
Assert-UserGuideCurrent $fixture
[IO.File]::WriteAllText((Join-Path $fixture 'Source/empty.cpp'),'')
Record-Review
Assert-UserGuideCurrent $fixture
Write-Host 'PASS guide currency: missing/stale/added/deleted inputs, canonical specs, guide, LF/CRLF stability and read-only checks'
