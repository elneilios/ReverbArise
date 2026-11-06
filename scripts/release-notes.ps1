# Release notes are authored/reviewed before packaging; this helper only validates and renders them.
function Get-ReleaseNotes([string] $Root, [string] $Identity) {
    if ($Identity -cnotmatch '^v[0-9]+\.[0-9]+\.[0-9]+(?:-[A-Za-z0-9][A-Za-z0-9.-]*)?$') { throw 'Invalid release-notes identity.' }
    $path = Join-Path $Root ("docs/development/release-notes/$Identity.json")
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Release notes missing: $path. Run build-release to author and commit notes before packaging." }
    try { $notes = Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json }
    catch { throw "Invalid release notes JSON: $path. $($_.Exception.Message)" }
    if ($notes.schemaVersion -ne 1 -or $notes.releaseIdentity -cne $Identity) { throw 'Release notes schema/identity mismatch.' }
    foreach ($required in @('previousRelease','previousGitSha')) {
        if ($notes.PSObject.Properties.Name -cnotcontains $required) { throw "Release notes field missing: $required" }
    }
    $hasPrevious = -not [string]::IsNullOrWhiteSpace($notes.previousRelease)
    if ($hasPrevious) {
        if ($notes.previousRelease -isnot [string] -or $notes.previousGitSha -isnot [string] -or $notes.previousGitSha -cnotmatch '^[a-f0-9]{40}$') { throw 'Release notes require the previous release label and full source SHA.' }
    } elseif ($null -ne $notes.previousRelease -or $null -ne $notes.previousGitSha) { throw 'First-release notes must use null for both previousRelease and previousGitSha.' }
    foreach ($section in @('changes','fixes','upgradeSteps','knownLimitations')) {
        if ($notes.$section -isnot [array]) { throw "Release notes require an array: $section" }
        foreach ($entry in $notes.$section) {
            if ($entry -isnot [string] -or [string]::IsNullOrWhiteSpace($entry)) { throw "Release notes contain an empty/non-text item: $section" }
        }
    }
    if (($notes.changes.Count + $notes.fixes.Count) -eq 0) { throw 'Release notes require at least one delivered change or fix.' }
    return $notes
}

function Write-ReleaseNotes($Notes, [string] $Payload, [string] $Sha) {
    if ($Sha -cnotmatch '^[a-f0-9]{40}$') { throw 'Invalid release-notes source SHA.' }
    $identity = [Net.WebUtility]::HtmlEncode($Notes.releaseIdentity)
    $baseline = if ($null -eq $Notes.previousRelease) { 'Initial release.' } else {
        'Changes since ' + [Net.WebUtility]::HtmlEncode($Notes.previousRelease) + ' (' + [Net.WebUtility]::HtmlEncode($Notes.previousGitSha) + ').'
    }
    $sections = ''
    foreach ($section in @(@('changes','What is new'), @('fixes','Fixes'), @('upgradeSteps','Upgrading'), @('knownLimitations','Known limitations'))) {
        $items = @($Notes.($section[0]))
        if ($items.Count -eq 0) { continue }
        $sections += '<h2>' + $section[1] + '</h2><ul>'
        foreach ($item in $items) { $sections += '<li>' + [Net.WebUtility]::HtmlEncode($item) + '</li>' }
        $sections += '</ul>'
    }
    $html = @"
<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>ReverbArise $identity release notes</title>
<style>body{font:16px/1.6 system-ui,sans-serif;max-width:52rem;margin:3rem auto;padding:0 1.25rem;color:#17232b;background:#fff}h1,h2{line-height:1.25}li{margin:.5rem 0}code{overflow-wrap:anywhere}</style></head>
<body><h1>ReverbArise $identity</h1><p>$baseline</p><p>Packaged source: <code>$Sha</code></p>
$sections
<p>See User-Guide.html for installation and full instructions.</p></body></html>
"@
    $html | Set-Content -LiteralPath (Join-Path $Payload 'Release-Notes.html') -Encoding UTF8
}
