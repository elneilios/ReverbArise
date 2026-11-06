# Focused notes validation/rendering only; no native build, host, installation or publication.
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'scripts/release-support.ps1')
$fixture = Join-Path ([IO.Path]::GetTempPath()) ('reverbarise-notes-' + [guid]::NewGuid().ToString('N'))
$source = Join-Path $fixture 'docs/development/release-notes/v0.1.0-alpha.2.json'
$payload = Join-Path $fixture 'payload'
$null = New-Item -ItemType Directory -Path (Split-Path $source) -Force
$null = New-Item -ItemType Directory -Path $payload
$script:checks = 0
function Assert($Condition, [string] $Message) {
    if (-not $Condition) { throw $Message }
    $script:checks++
}
function Expect-Failure([scriptblock] $Action, [string] $Pattern) {
    $caught = ''
    try { & $Action | Out-Null } catch { $caught = $_.Exception.Message }
    Assert ($caught -match $Pattern) "Expected '$Pattern', got '$caught'."
}
function Save-Notes($Value) {
    $Value | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $source -Encoding UTF8
}
try {
    Expect-Failure { Get-ReleaseNotes $fixture 'v0.1.0-alpha.2' } 'Release notes missing'
    Expect-Failure { Get-ReleaseNotes $fixture '../bad' } 'Invalid release-notes identity'
    '{bad json' | Set-Content -LiteralPath $source -Encoding UTF8
    Expect-Failure { Get-ReleaseNotes $fixture 'v0.1.0-alpha.2' } 'Invalid release notes JSON'
    $notes = [ordered]@{
        schemaVersion = 1; releaseIdentity = 'v0.1.0-alpha.2'
        previousRelease = 'v0.1.0-alpha.1'; previousGitSha = ('a' * 40)
        changes = @('Independent Dry & Wet levels')
        fixes = @('Display <script>alert("test")</script> as text')
        upgradeSteps = @('Regenerate eligible older swells when required')
        knownLimitations = @('Only the recorded host coverage is verified')
    }
    Save-Notes $notes
    $before = [IO.File]::ReadAllBytes($source)
    $accepted = Get-ReleaseNotes $fixture 'v0.1.0-alpha.2'
    Write-ReleaseNotes $accepted $payload ('b' * 40)
    $html = Get-Content -LiteralPath (Join-Path $payload 'Release-Notes.html') -Raw -Encoding UTF8
    Assert ($html.Contains('v0.1.0-alpha.2') -and $html.Contains('v0.1.0-alpha.1') -and $html.Contains(('a' * 40)) -and $html.Contains(('b' * 40))) 'Candidate/source/baseline missing.'
    Assert ($html.Contains('Dry &amp; Wet') -and $html.Contains('&lt;script&gt;') -and $html -notmatch '<script\b') 'Notes were not HTML-escaped.'
    Assert ($html.Contains('Upgrading') -and $html.Contains('Known limitations') -and $html.Contains('Regenerate eligible')) 'Recipient guidance missing.'
    Assert ($html -notmatch '\b(src|href)\s*=|<\s*(script|iframe|link|object|embed)\b|url\s*\(') 'Offline notes contain active/external resources.'
    Assert ([Convert]::ToBase64String($before) -ceq [Convert]::ToBase64String([IO.File]::ReadAllBytes($source))) 'Packaging changed authored notes.'
    $inventory = @(Get-ReleaseInventory $payload)
    $zipPath = Join-Path $fixture 'notes.zip'
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [IO.Compression.ZipFile]::CreateFromDirectory($payload, $zipPath)
    Assert-ReleaseArchive $zipPath $inventory
    Assert ($inventory.Count -eq 1 -and $inventory[0].path -ceq 'Release-Notes.html') 'Notes missing from ZIP inventory.'
    $inventory[0].sha256 = ('0' * 64)
    Expect-Failure { Assert-ReleaseArchive $zipPath $inventory } 'checksum mismatch'
    $notes.releaseIdentity = 'v0.1.0-alpha.1'; Save-Notes $notes
    Expect-Failure { Get-ReleaseNotes $fixture 'v0.1.0-alpha.2' } 'schema/identity mismatch'
    $notes.releaseIdentity = 'v0.1.0-alpha.2'; $notes.schemaVersion = 99; Save-Notes $notes
    Expect-Failure { Get-ReleaseNotes $fixture 'v0.1.0-alpha.2' } 'schema/identity mismatch'
    $notes.schemaVersion = 1; $notes.previousGitSha = 'short'; Save-Notes $notes
    Expect-Failure { Get-ReleaseNotes $fixture 'v0.1.0-alpha.2' } 'full source SHA'
    $notes.previousRelease = $null; Save-Notes $notes
    Expect-Failure { Get-ReleaseNotes $fixture 'v0.1.0-alpha.2' } 'First-release notes'
    $notes.previousGitSha = $null; $notes.fixes = @(); $notes.upgradeSteps = @(); $notes.knownLimitations = @(); Save-Notes $notes
    $first = Get-ReleaseNotes $fixture 'v0.1.0-alpha.2'
    Write-ReleaseNotes $first $payload ('c' * 40)
    $html = Get-Content -LiteralPath (Join-Path $payload 'Release-Notes.html') -Raw -Encoding UTF8
    Assert ($html.Contains('Initial release.') -and -not $html.Contains('<h2>Upgrading')) 'First release/empty sections incorrect.'
    $notes.Remove('previousRelease'); Save-Notes $notes
    Expect-Failure { Get-ReleaseNotes $fixture 'v0.1.0-alpha.2' } 'field missing'
    $notes.previousRelease = $null; $notes.changes = 'not an array'; Save-Notes $notes
    Expect-Failure { Get-ReleaseNotes $fixture 'v0.1.0-alpha.2' } 'require an array'
    $notes.changes = @(''); Save-Notes $notes
    Expect-Failure { Get-ReleaseNotes $fixture 'v0.1.0-alpha.2' } 'empty/non-text'
    $notes.changes = @(); Save-Notes $notes
    Expect-Failure { Get-ReleaseNotes $fixture 'v0.1.0-alpha.2' } 'at least one delivered'
    Expect-Failure { Write-ReleaseNotes $first $payload 'short' } 'Invalid release-notes source SHA'
    Write-Host "PASS $script:checks focused release-notes checks. No native build or publication performed."
} finally {
    Remove-Item -LiteralPath $fixture -Recurse -Force
}
