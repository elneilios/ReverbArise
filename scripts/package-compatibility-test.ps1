[CmdletBinding()]
param()
& {
    $ErrorActionPreference = 'Stop'
    $root = Split-Path $PSScriptRoot -Parent
    . (Join-Path $PSScriptRoot 'build-support.ps1')
    . (Join-Path $PSScriptRoot 'release-support.ps1')
    $tools = Resolve-BuildTools
    $sha = (Invoke-ReleaseNative $tools.Git @('-C', $root, 'rev-parse', 'HEAD')).Trim()
    # This isolated private packager intentionally does not invoke standard release
    # gates waived for #111. Standard release/publication tooling stays unchanged.
    $null = Invoke-ReleaseNative $tools.Git @('-C', $root, 'diff', '--exit-code', 'HEAD', '--')
    $binary = Join-Path $root 'out/build/windows-msvc'
    $lock = Enter-BuildLock $binary
    try {
        Assert-BuildCache $binary $root
        $cache = Get-Content (Join-Path $binary 'CMakeCache.txt') -Raw
        if ($cache -notmatch '(?m)^PREVERB_COMPATIBILITY_TEST:BOOL=(ON|TRUE)\r?$') { throw 'Diagnostic variant is not configured' }
        $meterExperiment = $cache -match '(?m)^PREVERB_COMPATIBILITY_METER_4_4:BOOL=(ON|TRUE)\r?$'
        $variant = 'PREVERB_COMPATIBILITY_TEST=ON; PREVERB_COMPATIBILITY_METER_4_4=' + $meterExperiment.ToString().ToUpperInvariant()
        $variantLabel = if ($meterExperiment) { 'compatibility-111-EXPERIMENTAL-4-4' } else { 'compatibility-111' }
        $meterNote = if ($meterExperiment) { 'EXPERIMENT: this build supplies fixed 4/4 ONLY when host meter is absent. A real host meter wins. Real host sample/PPQ/tempo and mapping-consistency guards remain in force. Other guards may still block/reject capture. This is not proof of VCV compatibility.' } else { 'No meter fallback is enabled. Missing host time signature blocks capture.' }
        $identity = Get-Content (Join-Path $binary 'build-information.json') -Raw | ConvertFrom-Json
        if ($identity.gitSha -cne $sha) { throw 'Build SHA differs from source; rebuild this commit' }
        $version = (Get-Content (Join-Path $binary 'product-identity.json') -Raw | ConvertFrom-Json).version
        $null = Invoke-ReleaseNative $tools.CMake @("-DSOURCE_DIR=$root", "-DBINARY_DIR=$binary", '-DCONFIGURATION=Release', '-P', (Join-Path $root 'scripts/verify-version.cmake'))
        $bundle = Get-ReleaseBundle $binary $version
        $null = Invoke-ReleaseNative $tools.CTest @('--test-dir', $binary, '-C', 'Release', '--output-on-failure', '--no-tests=error', '-R', '^preverb_(compatibility_test|workflow_processor_test|capture_failure_test|ui_standards_check)$')
        $destination = Join-Path $root "out/diagnostic-releases/$variantLabel-$sha"
        if (Test-Path -LiteralPath $destination) { throw "Immutable diagnostic output already exists: $destination" }
        $null = New-Item -ItemType Directory -Path $destination
        $payload = Join-Path $destination 'payload'
        $null = New-Item -ItemType Directory -Path $payload
        $before = @(Get-ReleaseInventory $bundle)
        Copy-Item -LiteralPath $bundle -Destination $payload -Recurse
        $after = @(Get-ReleaseInventory (Join-Path $payload 'ReverbArise.vst3'))
        if (($before | ConvertTo-Json -Depth 5 -Compress) -cne ($after | ConvertTo-Json -Depth 5 -Compress)) { throw 'Bundle changed during copying' }
        $notices = Join-Path $payload 'licenses'
        $null = New-Item -ItemType Directory -Path $notices
        $juce = Join-Path $binary '_deps/juce-src'
        foreach ($notice in (Get-ReleaseNoticePaths)) {
            $source = Join-Path $juce $notice[0]
            if ((Get-Item -LiteralPath $source).Length -eq 0) { throw "Missing notice: $source" }
            Copy-Item -LiteralPath $source -Destination (Join-Path $notices $notice[1])
        }
        Copy-Item -LiteralPath (Join-Path $root 'resources/README.md') -Destination (Join-Path $notices 'Original-Hall-provenance.md')
        $flac = Get-Content (Join-Path $juce 'modules/juce_audio_formats/codecs/flac/all.h') -Raw
        if ($flac -notmatch '(?s)\A(/\*.*?\*/)') { throw 'Missing FLAC notice' }
        $matches[1] | Set-Content (Join-Path $notices 'FLAC-LICENSE.txt') -Encoding UTF8
        @"
ReverbArise $version - PRIVATE EXPERIMENTAL COMPATIBILITY TEST #111
For Stuart. Windows x64 Release, unsigned. Not a public release.
Source SHA: $sha
Variant: $variant. Same plugin and saved-session identities.
$meterNote

INSTALL / ROLLBACK
Close every DAW before changing the installation. Back up your project first.
Keep the existing entire ReverbArise.vst3 bundle in a safe folder outside VST3
scan paths, then extract and copy this entire ReverbArise.vst3 bundle into
%PROGRAMFILES%\Common Files\VST3 (administrator permission may be needed).
Replace the prior bundle; do not install a second copy under another name.
Rescan/reopen the DAW. The editor automatically opens Compatibility Test.
Rollback: close every DAW, remove this diagnostic bundle and restore the entire
previous bundle to the same location. Preserve all project and captured/cache data.

TESTER PROCEDURE
Open the plugin, select Learn, start the patch/playback, attempt capture normally,
stop, then click Copy report and send the text to Neil. Close/reopen the companion
using Compatibility report in the plugin header. Clear report resets only diagnostic
history. No audio or private project paths are included in the report.

RELEASE NOTES
Adds independent raw input measurements, exact pre-start gate observations,
per-field host timing evidence, capture outcomes and bounded processor-instance
history retained across editor reopen. The only optional timing-policy exception is
the explicitly labelled missing-meter 4/4 experiment described above.
Silence and absent callbacks are distinct; observed seconds never drive capture.
Repeats are coalesced and history/queue omissions are disclosed.

VALIDATION / LIMITS
Native MSVC Release build; focused compatibility, workflow processor, capture failure
and UI standards checks. ZIP entries verified against payload SHA-256 inventory.
Full regression, OpenSpec approval gates, separate UI approvals and user-guide
refresh were waived by the owner for #111. Standard release tooling is unchanged.
VCV Host-FX, Audacity, FL Studio, clean-machine and alternate-DPI behaviour untested.
This is evidence gathering, not a claimed compatibility fix. Audacity investigation
remains #60. Do not distribute publicly. Third-party notices included in licenses
and bundle Contents/Resources. No new redistribution rights are granted.
"@ | Set-Content (Join-Path $payload 'README.txt') -Encoding UTF8
        $manifest = [ordered]@{
            sourceSha=$sha; variant=$variant; configuration='Release'; target='windows-x64'; version=$version
            buildInformation=$identity; toolchain=(Get-ReleaseToolchain $tools.CMake $binary)
            trackedSourceClean=$true; untrackedFiles=(Invoke-ReleaseNative $tools.Git @('-C', $root, 'ls-files', '--others', '--exclude-standard'))
            validation='native Release build; focused compatibility/workflow/capture-failure/UI checks; PE/version/plugin identity; per-entry ZIP SHA256'
            waived='OpenSpec proposal/Apply approvals; separate UI approvals; full regression; user-guide refresh'
            untested='VCV Host-FX; Audacity; FL Studio; alternate DPI; clean machine'
        }
        $manifest | ConvertTo-Json -Depth 8 | Set-Content (Join-Path $payload 'BUILD-IDENTITY.json') -Encoding UTF8
        $files = @(Get-ReleaseInventory $payload)
        $zip = Join-Path $destination "ReverbArise-PRIVATE-$variantLabel-$($sha.Substring(0,12))-win-x64.zip"
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        [IO.Compression.ZipFile]::CreateFromDirectory($payload, $zip, [IO.Compression.CompressionLevel]::Optimal, $false)
        Assert-ReleaseArchive $zip $files
        $hash = (Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant()
        "$hash  $([IO.Path]::GetFileName($zip))" | Set-Content ($zip + '.sha256') -Encoding ASCII
        $manifest['files']=$files; $manifest['sha256']=$hash; $manifest['zip']=$zip
        $manifest | ConvertTo-Json -Depth 8 | Set-Content (Join-Path $destination 'build-manifest.json') -Encoding UTF8
        if ((Invoke-ReleaseNative $tools.Git @('-C', $root, 'rev-parse', 'HEAD')).Trim() -cne $sha) { throw 'HEAD changed during packaging' }
        $null = Invoke-ReleaseNative $tools.Git @('-C', $root, 'diff', '--exit-code', 'HEAD', '--')
        Write-Host "Verified private ZIP: $zip"
        Write-Host "SHA256: $hash"
    } finally { $lock.Dispose() }
}
