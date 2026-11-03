function Get-ReleaseNoticePaths {
    # Notices from the pinned dependency checkout, including supported codecs.
    ,@('LICENSE.md', 'JUCE-LICENSE.md')
    ,@('JUCE.spdx.json', 'JUCE.spdx.json')
    ,@('modules/juce_audio_processors_headless/format_types/VST3_SDK/LICENSE.txt', 'VST3-SDK-LICENSE.txt')
    ,@('modules/juce_core/zip/zlib/LICENSE', 'zlib-LICENSE.txt')
    ,@('modules/juce_graphics/fonts/harfbuzz/COPYING', 'HarfBuzz-LICENSE.txt')
    ,@('modules/juce_graphics/image_formats/pnglib/LICENSE', 'libpng-LICENSE.txt')
    ,@('modules/juce_graphics/image_formats/jpglib/README', 'JPEG-README.txt')
    ,@('modules/juce_audio_formats/codecs/ogg/COPYING', 'Ogg-LICENSE.txt')
    ,@('modules/juce_audio_formats/codecs/vorbis/COPYING', 'Vorbis-LICENSE.txt')
    ,@('modules/juce_audio_formats/codecs/opus/opus/COPYING', 'Opus-LICENSE.txt')
    ,@('modules/juce_audio_formats/codecs/opus/opusfile/COPYING', 'Opusfile-LICENSE.txt')
    ,@('modules/juce_audio_formats/codecs/opus/libopusenc/COPYING', 'Opusenc-LICENSE.txt')
}
# Import has no side effects. Publication is reachable only through explicit Publish mode.
function Invoke-ReleaseNative([string] $Executable, [string[]] $Arguments) {
    $ErrorActionPreference = 'Continue'
    $PSNativeCommandUseErrorActionPreference = $false
    $output = @(& $Executable @Arguments 2>&1)
    if ($LASTEXITCODE -ne 0) { throw "Command failed ($LASTEXITCODE): $Executable $($Arguments -join ' ')`n$($output -join "`n")" }
    return ($output -join "`n")
}

function Get-ReleaseSource([string] $Git, [string] $Root, [string] $Identity = '') {
    $sha = (Invoke-ReleaseNative $Git @('-C', $Root, 'rev-parse', '--verify', 'HEAD^{commit}')).Trim()
    if ($sha -notmatch '^[a-f0-9]{40}$') { throw 'Cannot determine full Git HEAD revision.' }
    $status = Invoke-ReleaseNative $Git @('-C', $Root, 'status', '--porcelain=v1', '--untracked-files=all', '--ignore-submodules=none')
    if (-not [string]::IsNullOrWhiteSpace($status)) { throw "Release requires a clean worktree, including untracked files: $status" }
    if ($Identity) {
        $tags = @( (Invoke-ReleaseNative $Git @('-C', $Root, 'tag', '--list', $Identity)) -split "`r?`n" )
        if ($tags -contains $Identity) {
            $tagSha = (Invoke-ReleaseNative $Git @('-C', $Root, 'rev-parse', '--verify', "refs/tags/$Identity^{commit}")).Trim()
            if ($tagSha -cne $sha) { throw "Release tag $Identity points to $tagSha, not HEAD $sha." }
        }
    }
    return $sha
}

function Assert-ReleaseSource([string] $Git, [string] $Root, [string] $Identity, [string] $Sha) {
    if ((Get-ReleaseSource $Git $Root $Identity) -cne $Sha) { throw 'Git HEAD changed during release validation.' }
}

function Get-ReleaseIdentity([string] $CMake, [string] $Root, [string] $Tag, [string] $Candidate, [string] $BinaryDirectory = '') {
    $temporary = Join-Path $Root ('out/release-checks/' + [guid]::NewGuid().ToString('N') + '.json')
    $null = New-Item -ItemType Directory -Path (Split-Path $temporary) -Force
    $arguments = @("-DRELEASE_TAG=$Tag", "-DOUTPUT_FILE=$temporary")
    if ($Candidate) { $arguments += "-DCANDIDATE=$Candidate" }
    if ($BinaryDirectory) { $arguments += @("-DBINARY_DIR=$BinaryDirectory", '-DCONFIGURATION=Release') }
    $arguments += @('-P', (Join-Path $Root 'scripts/verify-version.cmake'))
    $null = Invoke-ReleaseNative $CMake $arguments
    # Unique per-call output, read only on successful exit: stale success cannot be consumed.
    return (Get-Content -LiteralPath $temporary -Raw | ConvertFrom-Json)
}

function Get-ReleaseInventory([string] $Directory) {
    $root = [IO.Path]::GetFullPath($Directory).TrimEnd('\', '/')
    if ((Get-Item -LiteralPath $root).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Reparse point forbidden: $root" }
    $items = @(Get-ChildItem -LiteralPath $root -Force -Recurse)
    foreach ($item in $items) {
        if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Reparse point forbidden: $($item.FullName)" }
    }
    return @($items | Where-Object { -not $_.PSIsContainer } | Sort-Object FullName | ForEach-Object {
        [ordered]@{ path = $_.FullName.Substring($root.Length + 1).Replace('\', '/'); bytes = $_.Length; sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant() }
    })
}

function Assert-ReleaseBinary([string] $Path, [string] $Version) {
    $stream = [IO.File]::OpenRead($Path)
    $reader = New-Object IO.BinaryReader($stream)
    try {
        if ($reader.ReadUInt16() -ne 0x5a4d) { throw 'VST3 binary is not a Windows PE file.' }
        $stream.Position = 0x3c; $offset = $reader.ReadUInt32()
        $stream.Position = $offset
        if ($reader.ReadUInt32() -ne 0x4550 -or $reader.ReadUInt16() -ne 0x8664) { throw 'VST3 binary must be Windows x64 PE.' }
    } finally { $reader.Dispose(); $stream.Dispose() }
    $info = [Diagnostics.FileVersionInfo]::GetVersionInfo($Path)
    $actual = "$($info.FileMajorPart).$($info.FileMinorPart).$($info.FileBuildPart)"
    if ($actual -cne $Version) { throw "Binary file version $actual does not match $Version." }
}

function Get-ReleaseBundle([string] $BinaryDirectory, [string] $Version) {
    $bundle = Join-Path $BinaryDirectory 'preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3'
    $required = @('Contents/x86_64-win/ReverbArise.vst3', 'Contents/Resources/moduleinfo.json', 'Contents/Resources/WavPack-LICENSE.txt', 'Contents/Resources/Inter-OFL.txt', 'Contents/Resources/IR-LICENSE.md', 'Contents/Resources/IR-manifest.json')
    foreach ($relative in $required) {
        $file = Get-Item -LiteralPath (Join-Path $bundle $relative) -ErrorAction Stop
        if ($file.PSIsContainer -or $file.Length -eq 0) { throw "Missing or empty required bundle file: $relative" }
    }
    $null = Get-ReleaseInventory $bundle
    Assert-ReleaseBinary (Join-Path $bundle $required[0]) $Version
    return $bundle
}

function Get-ReleaseToolchain([string] $CMake, [string] $BinaryDirectory) {
    $cache = Get-Content -LiteralPath (Join-Path $BinaryDirectory 'CMakeCache.txt') -Raw
    if ($cache -notmatch '(?m)^CMAKE_GENERATOR:INTERNAL=Visual Studio 18 2026\r?$' -or $cache -notmatch '(?m)^CMAKE_GENERATOR_PLATFORM:INTERNAL=x64\r?$') { throw 'Release requires the supported Visual Studio x64 cache.' }
    $compiler = @(Get-ChildItem -Path (Join-Path $BinaryDirectory 'CMakeFiles/*/CMakeCXXCompiler.cmake'))
    if ($compiler.Count -ne 1) { throw 'Expected one configured C++ compiler identity.' }
    $description = Get-Content -LiteralPath $compiler[0].FullName -Raw
    if ($description -notmatch 'set\(CMAKE_CXX_COMPILER_ID "MSVC"\)' -or $description -notmatch 'set\(CMAKE_CXX_COMPILER_VERSION "([^"]+)"\)') { throw 'Missing MSVC compiler version.' }
    return [ordered]@{ generator = 'Visual Studio 18 2026'; compiler = 'MSVC'; compilerVersion = $matches[1]; cmake = (Invoke-ReleaseNative $CMake @('--version')).Split("`n")[0].Trim() }
}

function Assert-ReleaseArchive([string] $ZipPath, $Files) {
    Add-Type -AssemblyName System.IO.Compression
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [IO.Compression.ZipFile]::OpenRead($ZipPath)
    try {
        $entries = @($zip.Entries | Where-Object { -not $_.FullName.EndsWith('/') })
        if ($entries.Count -ne @($Files).Count) { throw 'ZIP file inventory count mismatch.' }
        foreach ($file in $Files) {
            $entry = @($entries | Where-Object { $_.FullName -ceq $file.path })
            if ($entry.Count -ne 1 -or $entry[0].Length -ne $file.bytes) { throw "ZIP missing or invalid entry: $($file.path)" }
            $stream = $entry[0].Open(); $hasher = [Security.Cryptography.SHA256]::Create()
            try { $hash = ([BitConverter]::ToString($hasher.ComputeHash($stream))).Replace('-', '').ToLowerInvariant() }
            finally { $hasher.Dispose(); $stream.Dispose() }
            if ($hash -cne $file.sha256) { throw "ZIP entry checksum mismatch: $($file.path)" }
        }
    } finally { $zip.Dispose() }
}

function Invoke-ReleaseBuild([string] $Root) {
    $shell = (Get-Process -Id $PID).Path
    & $shell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Root 'scripts/build.ps1') -Configuration Release -Test
    if ($LASTEXITCODE -ne 0) { throw "Release build/CTest failed ($LASTEXITCODE); no package produced." }
}

function Invoke-ReleasePublication([string] $Root, [string] $Directory, $Identity, [string] $Sha, [string] $Git, [switch] $RedistributionReviewed, [string] $LicenseReview, [string] $ConfirmPublish) {
    if (-not $RedistributionReviewed -or $LicenseReview.Trim().Length -lt 20) { throw 'Redistribution gate: explicitly review JUCE, project and third-party licensing; supply -RedistributionReviewed and a substantive -LicenseReview record.' }
    $confirmation = "$($Identity.releaseIdentity)@$Sha"
    if ($ConfirmPublish -cne $confirmation) { throw "Separate publication confirmation required: -ConfirmPublish '$confirmation'" }
    $manifest = Get-Content -LiteralPath (Join-Path $Directory 'build-manifest.json') -Raw | ConvertFrom-Json
    $artifact = "ReverbArise-$($Identity.releaseIdentity)-win-x64.zip"
    if ($manifest.gitSha -cne $Sha -or $manifest.releaseIdentity -cne $Identity.releaseIdentity -or $manifest.version -cne $Identity.version -or $manifest.candidate -cne $Identity.candidate -or $manifest.artifact -cne $artifact -or $manifest.configuration -cne 'Release' -or $manifest.target -cne 'windows-x64' -or $manifest.validation -cne 'build.ps1 Release -Test; verify-version.cmake built checks; x64 PE/file version') { throw 'Package source/release/validation identity mismatch.' }
    $zip = Join-Path $Directory $artifact
    $hash = (Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant()
    $checksum = (Get-Content -LiteralPath ($zip + '.sha256') -Raw).Trim()
    if ($manifest.sha256 -cne $hash -or $checksum -cne "$hash  $artifact") { throw 'Package SHA-256 mismatch.' }
    Assert-ReleaseArchive $zip $manifest.files
    Assert-ReleaseSource $Git $Root $Identity.releaseIdentity $Sha
    $gh = Get-Command gh.exe -CommandType Application -ErrorAction Stop | Select-Object -First 1
    $null = Invoke-ReleaseNative $gh.Source @('auth', 'status')
    $repository = 'elneilios/ReverbArise'
    $ref = Invoke-ReleaseNative $gh.Source @('api', "repos/$repository/git/ref/tags/$($Identity.releaseIdentity)")
    $object = ($ref | ConvertFrom-Json).object
    for ($depth = 0; $object.type -eq 'tag' -and $depth -lt 16; $depth++) {
        $object = ((Invoke-ReleaseNative $gh.Source @('api', "repos/$repository/git/tags/$($object.sha)")) | ConvertFrom-Json).object
    }
    if ($object.type -cne 'commit' -or $object.sha -cne $Sha) { throw 'Remote release tag must already exist and identify the packaged commit. Push it deliberately after review.' }
    Assert-ReleaseSource $Git $Root $Identity.releaseIdentity $Sha
    $arguments = @('release', 'create', $Identity.releaseIdentity, $zip, ($zip + '.sha256'), (Join-Path $Directory 'build-manifest.json'), '--repo', $repository, '--verify-tag', '--title', "ReverbArise $($Identity.releaseIdentity)", '--latest=false', '--notes', "Source: $Sha. Redistribution reviewed: $LicenseReview")
    if ($Identity.candidate) { $arguments += '--prerelease' }
    $null = Invoke-ReleaseNative $gh.Source $arguments
    Write-Host "Published $($Identity.releaseIdentity) from $Sha after explicit redistribution review."
}

function Invoke-ReverbRelease {
    [CmdletBinding()]
    param([string] $Root, [string] $ReleaseTag, [string] $Candidate = '', [switch] $Publish, [switch] $RedistributionReviewed, [string] $LicenseReview = '', [string] $ConfirmPublish = '')
    $ErrorActionPreference = 'Stop'
    $lock = $null
    Push-Location -LiteralPath $Root
    try {
        $tools = Resolve-BuildTools
        $sha = Get-ReleaseSource $tools.Git $Root
        $identity = Get-ReleaseIdentity $tools.CMake $Root $ReleaseTag $Candidate
        Assert-ReleaseSource $tools.Git $Root $identity.releaseIdentity $sha
        $releases = Join-Path $Root 'out/releases'
        $destination = Join-Path $releases $identity.releaseIdentity
        if ($Publish) {
            Invoke-ReleasePublication -Root $Root -Directory $destination -Identity $identity -Sha $sha -Git $tools.Git -RedistributionReviewed:$RedistributionReviewed -LicenseReview $LicenseReview -ConfirmPublish $ConfirmPublish
            return
        }
        if (Test-Path -LiteralPath $destination) { throw "Release output already exists: $destination. Preserve it; use a new explicit candidate or deliberately relocate it while idle." }
        Invoke-ReleaseBuild $Root
        $presets = Get-Content -LiteralPath (Join-Path $Root 'CMakePresets.json') -Raw | ConvertFrom-Json
        $preset = @($presets.configurePresets | Where-Object { $_.name -eq 'windows-msvc' })[0]
        $binary = [IO.Path]::GetFullPath($preset.binaryDir.Replace('${sourceDir}', $Root).Replace('${presetName}', 'windows-msvc'))
        $lock = Enter-BuildLock $binary
        Assert-BuildCache $binary $Root
        Assert-ReleaseSource $tools.Git $Root $identity.releaseIdentity $sha
        $built = Get-ReleaseIdentity $tools.CMake $Root $ReleaseTag $Candidate $binary
        if (-not $built.builtVerified -or -not $built.tagVerified) { throw 'Built product identity was not verified.' }
        $bundle = Get-ReleaseBundle $binary $identity.version
        $toolchain = Get-ReleaseToolchain $tools.CMake $binary
        $staging = Join-Path $releases ('.staging-' + [guid]::NewGuid().ToString('N'))
        $payload = Join-Path $staging 'payload'
        $null = New-Item -ItemType Directory -Path $payload -Force
        $inventory = @(Get-ReleaseInventory $bundle)
        Copy-Item -LiteralPath $bundle -Destination $payload -Recurse
        $copied = @(Get-ReleaseInventory (Join-Path $payload 'ReverbArise.vst3'))
        if (($inventory | ConvertTo-Json -Depth 5 -Compress) -cne ($copied | ConvertTo-Json -Depth 5 -Compress)) { throw 'Bundle changed while copying.' }
        $notices = Join-Path $payload 'licenses'
        $null = New-Item -ItemType Directory -Path $notices
        $juce = Join-Path $binary '_deps/juce-src'
        foreach ($notice in (Get-ReleaseNoticePaths)) {
            $source = Join-Path $juce $notice[0]
            if ((Get-Item -LiteralPath $source).Length -eq 0) { throw "Missing dependency license: $source" }
            Copy-Item -LiteralPath $source -Destination (Join-Path $notices $notice[1])
        }
        Copy-Item -LiteralPath (Join-Path $Root 'resources/README.md') -Destination (Join-Path $notices 'Original-Hall-provenance.md')
        # The pinned FLAC distribution keeps its complete BSD notice in this header.
        $flac = Get-Content -LiteralPath (Join-Path $juce 'modules/juce_audio_formats/codecs/flac/all.h') -Raw
        if ($flac -notmatch '(?s)\A(/\*.*?\*/)') { throw 'Missing FLAC notice header.' }
        $matches[1] | Set-Content -LiteralPath (Join-Path $notices 'FLAC-LICENSE.txt') -Encoding UTF8
        @"
ReverbArise $($identity.releaseIdentity) - Windows x64 Release
Source: $sha
INTERNAL EVALUATION ONLY. This local package grants no redistribution permission.
Before external distribution the owner must review JUCE eligibility/terms, project rights,
WavPack, Inter, VST3 SDK, bundled IRs and any other applicable third-party conditions.
Notices are in licenses/ and ReverbArise.vst3/Contents/Resources/.
Unsigned build. Extract the complete ReverbArise.vst3 directory; do not copy only its binary.
With the DAW closed, preserve any prior installation and copy the bundle to your user VST3
folder (%LOCALAPPDATA%\Programs\Common\VST3), then rescan in FL Studio.
Keep saved FL Studio projects and captured/cache assets; verify an existing project copy.
See docs/development/releases.md in the source checkout for the manual acceptance checklist.
"@ | Set-Content -LiteralPath (Join-Path $payload 'README.txt') -Encoding UTF8
        $files = @(Get-ReleaseInventory $payload)
        $artifact = "ReverbArise-$($identity.releaseIdentity)-win-x64.zip"
        $zip = Join-Path $staging $artifact
        Add-Type -AssemblyName System.IO.Compression
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        $archive = [IO.Compression.ZipFile]::Open($zip, [IO.Compression.ZipArchiveMode]::Create)
        try {
            foreach ($directory in (Get-ChildItem -LiteralPath $payload -Directory -Recurse -Force)) {
                $relative = $directory.FullName.Substring($payload.Length + 1).Replace('\', '/') + '/'
                $null = $archive.CreateEntry($relative)
            }
            foreach ($file in $files) {
                $null = [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive, (Join-Path $payload $file.path), $file.path, [IO.Compression.CompressionLevel]::Optimal)
            }
        } finally { $archive.Dispose() }
        Assert-ReleaseArchive $zip $files
        $hash = (Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant()
        "$hash  $artifact" | Set-Content -LiteralPath ($zip + '.sha256') -Encoding ASCII
        $manifest = [ordered]@{
            schemaVersion = 1; version = $identity.version; releaseTag = $ReleaseTag; candidate = $identity.candidate; releaseIdentity = $identity.releaseIdentity
            gitSha = $sha; worktreeClean = $true; builtAtUtc = [DateTime]::UtcNow.ToString('o'); configuration = 'Release'; target = 'windows-x64'
            toolchain = $toolchain; artifact = $artifact; sha256 = $hash
            validation = 'build.ps1 Release -Test; verify-version.cmake built checks; x64 PE/file version'
            redistribution = 'internal-evaluation-only; separate owner licensing review required'; files = $files
        }
        $manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $staging 'build-manifest.json') -Encoding UTF8
        Assert-ReleaseSource $tools.Git $Root $identity.releaseIdentity $sha
        # Directory.Move refuses an existing destination, including a racing packager.
        [IO.Directory]::Move($staging, $destination)
        Write-Host "PASS internal package: $(Join-Path $destination $artifact)"
        Write-Host "SHA-256: $hash | Git: $sha | Redistribution NOT approved."
    } finally {
        if ($lock) { $lock.Dispose() }
        Pop-Location
    }
}
