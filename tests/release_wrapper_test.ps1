# Simulated packaging orchestration only: real Git/CMake verifier, dummy bundle/build/gh.
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$repository = Split-Path $PSScriptRoot -Parent
. (Join-Path $repository 'scripts/build-support.ps1')
. (Join-Path $repository 'scripts/release-support.ps1')
$actualNative = ${function:Invoke-ReleaseNative}
$tools = Resolve-BuildTools
$fixture = Join-Path $repository ('out/release-wrapper-fixtures/' + [guid]::NewGuid().ToString('N') + '/checkout with spaces')
$null = New-Item -ItemType Directory -Path (Join-Path $fixture 'scripts') -Force
$null = New-Item -ItemType Directory -Path (Join-Path $fixture 'cmake')
Copy-Item -LiteralPath (Join-Path $repository 'scripts/verify-version.cmake') -Destination (Join-Path $fixture 'scripts')
Copy-Item -LiteralPath (Join-Path $repository 'cmake/ProductVersion.cmake') -Destination (Join-Path $fixture 'cmake')
foreach ($scriptFile in @('release.ps1','release-support.ps1','build-support.ps1','release-guide.ps1','user-guide-support.ps1','check-user-guide.ps1','review-user-guide.ps1')) { Copy-Item -LiteralPath (Join-Path $repository ('scripts/' + $scriptFile)) -Destination (Join-Path $fixture 'scripts') }
Copy-Item -LiteralPath (Join-Path $repository 'CMakePresets.json') -Destination $fixture
$null = New-Item -ItemType Directory -Path (Join-Path $fixture 'docs/development') -Force
Copy-Item -LiteralPath (Join-Path $repository 'docs/development/alpha-guide.html') -Destination (Join-Path $fixture 'docs/development')
$logoRelative = 'resources/logo/png/wordmark/reverbarise-wordmark-dark-640px.png'
$null = New-Item -ItemType Directory -Path (Split-Path (Join-Path $fixture $logoRelative)) -Force
Copy-Item -LiteralPath (Join-Path $repository $logoRelative) -Destination (Join-Path $fixture $logoRelative)
'project(Preverb VERSION 0.1.0 LANGUAGES C CXX)' | Set-Content (Join-Path $fixture 'CMakeLists.txt')
'/out/' | Set-Content (Join-Path $fixture '.gitignore')
'fixture' | Set-Content (Join-Path $fixture 'tracked.txt')
$null = New-Item -ItemType Directory -Path (Join-Path $fixture 'resources') -Force
'fixture original Hall provenance' | Set-Content (Join-Path $fixture 'resources/README.md')
$null=New-Item -ItemType Directory -Path (Join-Path $fixture 'Source')
$null=New-Item -ItemType Directory -Path (Join-Path $fixture 'openspec/specs/fixture') -Force
'fixture production code' | Set-Content (Join-Path $fixture 'Source/fixture.cpp')
'fixture canonical specification' | Set-Content (Join-Path $fixture 'openspec/specs/fixture/spec.md')
& (Join-Path $repository 'scripts/review-user-guide.ps1') -Root $fixture -Summary 'Simulated release fixture only'
$null = & $actualNative $tools.Git @('-C', $fixture, 'init', '--quiet')
$null = & $actualNative $tools.Git @('-C', $fixture, 'config', 'user.name', 'Release Fixture')
$null = & $actualNative $tools.Git @('-C', $fixture, 'config', 'user.email', 'fixture@example.invalid')
$null = & $actualNative $tools.Git @('-C', $fixture, 'add', '.')
$null = & $actualNative $tools.Git @('-C', $fixture, 'commit', '-qm', 'fixture source')
$sha = Get-ReleaseSource $tools.Git $fixture
$binary = Join-Path $fixture 'out/build/windows-msvc'
$bundle = Join-Path $binary 'preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3'
$resources = Join-Path $bundle 'Contents/Resources'
$script:checks = 0; $script:buildCalls = 0; $script:ghCalls = @(); $script:case = ''; $script:heldLock = $null
function Assert($Condition, [string] $Message) { if (-not $Condition) { throw $Message }; $script:checks++ }
function Invoke-ReleaseNative([string] $Executable, [string[]] $Arguments) {
    if ([IO.Path]::GetFileName($Executable) -ieq 'gh.exe') {
        $script:ghCalls += ,$Arguments
        if ($Arguments[0] -eq 'api') {
            $remoteSha = if ($script:case -eq 'remote-wrong') { '0000000000000000000000000000000000000000' } else { $sha }
            if ($script:case -eq 'annotated' -and $Arguments[1] -match '/ref/') { return '{"object":{"type":"tag","sha":"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}}' }
            return ('{"object":{"type":"commit","sha":"' + $remoteSha + '"}}')
        }
        return ''
    }
    return (& $actualNative $Executable $Arguments)
}
function Assert-ReleaseBinary([string] $Path, [string] $Version) {
    # Dummy bytes deliberately bypass PE validation. Actual PE validation is native evidence only.
    if ($script:case -eq 'bad-pe') { throw 'fixture invalid PE' }
}
function Invoke-ReleaseGuideCapture([string] $Binary, [string] $Directory, [string] $Sha) {
    if ($script:case -eq 'guide-readiness-failed') { throw 'fixture guide readiness failed' }
    Add-Type -AssemblyName System.Drawing
    foreach ($state in (Get-ReleaseGuideStates)) {
        $image = New-Object Drawing.Bitmap(1400,1000)
        try { $image.Save((Join-Path $Directory ($state + '.png')), [Drawing.Imaging.ImageFormat]::Png) } finally { $image.Dispose() }
    }
    $captureSha = if ($script:case -eq 'guide-stale') { '0000000000000000000000000000000000000000' } else { $Sha }
    @{ gitSha=$captureSha; command='Release/preverb_learn_generation_ui_test.exe --guide-only'; executableSha256=('a'*64) } | ConvertTo-Json | Set-Content (Join-Path $Directory 'capture.json')
    if ($script:case -eq 'guide-missing') { Remove-Item -LiteralPath (Join-Path $Directory 'alpha-ready-export.png') }
    if ($script:case -eq 'guide-corrupt') { 'broken image' | Set-Content (Join-Path $Directory 'alpha-ready-export.png') }
}
function Invoke-ReleaseBuild([string] $Root, [string] $ReleaseTag, [string] $Candidate) {
    $script:buildCalls++
    if ($script:case -in @('build-fail', 'ctest-fail')) { throw "fixture $script:case" }
    $null = New-Item -ItemType Directory -Path $resources -Force
    $null = New-Item -ItemType Directory -Path (Join-Path $bundle 'Contents/x86_64-win') -Force
    'dummy binary - NOT a native build' | Set-Content (Join-Path $bundle 'Contents/x86_64-win/ReverbArise.vst3')
    foreach ($name in @('WavPack-LICENSE.txt', 'Inter-OFL.txt', 'IR-LICENSE.md', 'IR-manifest.json', 'extra-resource.txt')) { 'fixture notice' | Set-Content (Join-Path $resources $name) }
    '{"Name":"ReverbArise","Version":"0.1.0","Classes":[{"CID":"ABCDEF019182FAEB4B374D6150727631","Category":"Audio Module Class","Name":"ReverbArise","Vendor":"K7MA","Version":"0.1.0"},{"CID":"ABCDEF011234ABCD4B374D6150727631","Category":"Component Controller Class","Name":"ReverbArise","Vendor":"K7MA","Version":"0.1.0"}]}' | Set-Content (Join-Path $resources 'moduleinfo.json')
    '{"version":"0.1.0","manufacturerCode":"K7Ma","pluginCode":"Prv1","bundleId":"com.k7ma.preverb","productName":"ReverbArise"}' | Set-Content (Join-Path $binary 'product-identity.json')
    $buildIdentifier=$ReleaseTag;if($Candidate){$buildIdentifier+='-'+$Candidate}
    @{productName='ReverbArise';version='0.1.0';candidate=$Candidate;releaseTag=$ReleaseTag;buildIdentifier=$buildIdentifier;gitSha=$sha;worktreeDirty=$false} | ConvertTo-Json | Set-Content (Join-Path $binary 'build-information.json')
    if($script:case -in @('stale-build-info','stale-build-sha','dirty-build-info')) {
        $metadata=Get-Content (Join-Path $binary 'build-information.json') -Raw | ConvertFrom-Json
        if($script:case -eq 'stale-build-info'){$metadata.candidate='old'}
        if($script:case -eq 'stale-build-sha'){$metadata.gitSha='0000000000000000000000000000000000000000'}
        if($script:case -eq 'dirty-build-info'){$metadata.worktreeDirty=$true}
        $metadata | ConvertTo-Json | Set-Content (Join-Path $binary 'build-information.json')
    }
    "CMAKE_HOME_DIRECTORY:INTERNAL=$($Root.Replace('\','/'))`nCMAKE_GENERATOR:INTERNAL=Visual Studio 18 2026`nCMAKE_GENERATOR_PLATFORM:INTERNAL=x64" | Set-Content (Join-Path $binary 'CMakeCache.txt')
    $compiler = Join-Path $binary 'CMakeFiles/fixture/CMakeCXXCompiler.cmake'
    $null = New-Item -ItemType Directory -Path (Split-Path $compiler) -Force
    'set(CMAKE_CXX_COMPILER_ID "MSVC")', 'set(CMAKE_CXX_COMPILER_VERSION "fixture-0")' | Set-Content $compiler
    foreach ($notice in (Get-ReleaseNoticePaths)) {
        $relative = '_deps/juce-src/' + $notice[0]
        $path = Join-Path $binary $relative
        $null = New-Item -ItemType Directory -Path (Split-Path $path) -Force
        'fixture dependency license' | Set-Content $path
    }
    $flac = Join-Path $binary '_deps/juce-src/modules/juce_audio_formats/codecs/flac/all.h'
    $null = New-Item -ItemType Directory -Path (Split-Path $flac) -Force
    '/* fixture FLAC license */' | Set-Content $flac
    $null = New-Item -ItemType Directory -Path (Join-Path $Root 'resources') -Force
    # Out-of-tree provenance fixture created initially before source commit below.
    if ($script:case -like 'missing:*') { Remove-Item -LiteralPath (Join-Path $bundle $script:case.Substring(8)) }
    if ($script:case -eq 'missing-juce-notice') { Remove-Item -LiteralPath (Join-Path $binary '_deps/juce-src/LICENSE.md') }
    if ($script:case -eq 'empty-notice') { '' | Set-Content (Join-Path $resources 'Inter-OFL.txt') -NoNewline }
    if ($script:case -eq 'bad-metadata') { '{"Name":"Wrong","Version":"0.2.0"}' | Set-Content (Join-Path $resources 'moduleinfo.json') }
    if ($script:case -eq 'dirty-after-build') { 'changed' | Set-Content (Join-Path $Root 'tracked.txt') }
    if ($script:case -eq 'head-after-build') { $null = & $actualNative $tools.Git @('-C', $Root, 'commit', '--allow-empty', '-qm', 'head changed') }
    if ($script:case -eq 'busy') { $script:heldLock = Enter-BuildLock $binary }
}
function Fail-Case([string] $Name, [string] $Pattern, [hashtable] $Parameters = @{}) {
    $script:case = $Name
    $caught = ''
    $arguments = @{ Root = $fixture; ReleaseTag = 'v0.1.0' }
    foreach ($key in $Parameters.Keys) { $arguments[$key] = $Parameters[$key] }
    try { Invoke-ReverbRelease @arguments } catch { $caught = $_.Exception.Message }
    Assert ($caught -match $Pattern) "Expected $Name failure /$Pattern/, got: $caught"
    if ($script:heldLock) { $script:heldLock.Dispose(); $script:heldLock = $null }
    $script:case = ''
}
Fail-Case 'version-mismatch' 'does not match' @{ ReleaseTag = 'v0.2.0' }
Fail-Case 'bad-candidate' 'Invalid CANDIDATE' @{ Candidate = '../oops' }
'changed' | Set-Content (Join-Path $fixture 'tracked.txt')
Fail-Case 'dirty' 'clean worktree'
$null = & $actualNative $tools.Git @('-C', $fixture, 'restore', 'tracked.txt')
'new' | Set-Content (Join-Path $fixture 'untracked.txt')
Fail-Case 'untracked' 'clean worktree'
Remove-Item -LiteralPath (Join-Path $fixture 'untracked.txt')
$null = & $actualNative $tools.Git @('-C', $fixture, 'tag', 'v0.1.0')
$null = & $actualNative $tools.Git @('-C', $fixture, 'commit', '--allow-empty', '-qm', 'different commit')
Fail-Case 'wrong-tag' 'not HEAD'
$null = & $actualNative $tools.Git @('-C', $fixture, 'tag', '-d', 'v0.1.0')
$sha = Get-ReleaseSource $tools.Git $fixture
foreach ($name in @('build-fail', 'ctest-fail', 'bad-pe', 'missing-juce-notice', 'empty-notice', 'bad-metadata', 'busy', 'dirty-after-build', 'head-after-build', 'stale-build-info', 'stale-build-sha', 'dirty-build-info')) {
    $pattern = switch ($name) { {$_ -in @('stale-build-info','stale-build-sha','dirty-build-info')} { 'Built About identity mismatch' }; 'busy' { 'busy or inaccessible' }; 'dirty-after-build' { 'clean worktree' }; 'head-after-build' { 'HEAD changed' }; 'empty-notice' { 'empty required' }; 'bad-metadata' { 'version mismatch' }; 'missing-juce-notice' { 'not exist|cannot find' }; default { 'fixture' } }
    Fail-Case $name $pattern
    if ($name -eq 'dirty-after-build') { $null = & $actualNative $tools.Git @('-C', $fixture, 'restore', 'tracked.txt') }
}
$sha = Get-ReleaseSource $tools.Git $fixture
foreach ($relative in @('Contents/x86_64-win/ReverbArise.vst3', 'Contents/Resources/moduleinfo.json', 'Contents/Resources/WavPack-LICENSE.txt', 'Contents/Resources/Inter-OFL.txt', 'Contents/Resources/IR-LICENSE.md', 'Contents/Resources/IR-manifest.json')) {
    Fail-Case ('missing:' + $relative) 'missing|not exist|cannot find|failed'
}
$start = (Get-Location).Path
$reviewFile=Join-Path $fixture 'docs/development/user-guide-review.json'
$savedReview=[IO.File]::ReadAllText($reviewFile)
Remove-Item -LiteralPath $reviewFile
$null=& $actualNative $tools.Git @('-C',$fixture,'add','docs/development/user-guide-review.json')
$null=& $actualNative $tools.Git @('-C',$fixture,'commit','-qm','fixture missing guide review')
$sha=Get-ReleaseSource $tools.Git $fixture
$beforeGateBuilds=$script:buildCalls
Fail-Case 'missing-guide-review' 'Guide review missing'
Assert ($script:buildCalls -eq $beforeGateBuilds) 'Stale guide gate ran a build.'
[IO.File]::WriteAllText($reviewFile,$savedReview)
$null=& $actualNative $tools.Git @('-C',$fixture,'add','docs/development/user-guide-review.json')
$null=& $actualNative $tools.Git @('-C',$fixture,'commit','-qm','fixture restored guide review')
$sha=Get-ReleaseSource $tools.Git $fixture
foreach ($entry in @(@('guide-readiness-failed','readiness failed'),@('guide-stale','provenance mismatch'),@('guide-missing','not find|does not exist|cannot find'),@('guide-corrupt','Parameter is not valid|invalid|FromStream'))) {
    Fail-Case $entry[0] $entry[1]
}
Invoke-ReverbRelease -Root $fixture -ReleaseTag v0.1.0
Assert ((Get-Location).Path -eq $start) 'Caller location was not restored.'
$stable = Join-Path $fixture 'out/releases/v0.1.0'
$manifest = Get-Content (Join-Path $stable 'build-manifest.json') -Raw | ConvertFrom-Json
Assert ($manifest.gitSha -ceq $sha -and $manifest.candidate -ceq '' -and $manifest.toolchain.compiler -ceq 'MSVC') 'Stable manifest mismatch.'
Assert (@($manifest.files | Where-Object { $_.path -like '*extra-resource.txt' }).Count -eq 1) 'Whole bundle not preserved.'
Assert (@($manifest.files | Where-Object { $_.path -ceq 'User-Guide.html' }).Count -eq 1) 'Guide not inventoried.'
$guide = Get-Content (Join-Path $stable 'payload/User-Guide.html') -Raw -Encoding UTF8
Assert ([regex]::Matches($guide, 'src="data:image/png;base64,').Count -eq 5 -and $guide.Contains($sha) -and $guide.Contains('v0.1.0') -and $guide -notmatch '{{|https?://|<script') 'Guide identity/media mismatch.'
Assert ($guide.Contains([string][char]0x00d7) -and $guide -notmatch ([string][char]0x00c3)) 'Guide UTF-8 text was corrupted.'
$provenance = Get-Content (Join-Path $stable 'payload/guide-provenance.json') -Raw | ConvertFrom-Json
Assert ($provenance.gitSha -ceq $sha -and $provenance.images.Count -eq 4) 'Guide provenance missing.'
Assert ([IO.File]::ReadAllText($reviewFile) -ceq $savedReview) 'Packaging rewrote guide review.'
Assert ($manifest.buildInformation.gitSha -ceq $sha -and $manifest.buildInformation.buildIdentifier -ceq 'v0.1.0') 'Packaged About information mismatch.'
Assert ($provenance.logo.sha256 -ceq (Get-FileHash -LiteralPath (Join-Path $fixture $logoRelative) -Algorithm SHA256).Hash.ToLowerInvariant() -and $provenance.logo.width -eq 640 -and $guide -match '<main>\s*<img class="brand" src="data:image/png;base64,') 'Production logo is missing or not at the top.'
Assert ((Get-Content (Join-Path $stable 'payload/README.txt') -Raw).Contains('play the DAW to capture')) 'Quick start missing.'
$templatePath = Join-Path $fixture 'docs/development/alpha-guide.html'
$templateBytes = [IO.File]::ReadAllBytes($templatePath)
try {
    foreach ($bad in @('<script>alert(1)</script>', '<img src="external.png">', '<style>body{background:url(remote.png)}</style>', '<iframe src="remote.html"></iframe>', '{{unresolved}}')) {
        [IO.File]::WriteAllText($templatePath, [Text.Encoding]::UTF8.GetString($templateBytes) + $bad)
        $caught = ''
        try { Write-ReleaseGuide $fixture (Join-Path $stable 'native-guide-evidence') (Join-Path $fixture 'out') $sha 'v0.1.0' } catch { $caught = $_.Exception.Message }
        Assert ($caught -match 'Guide contains|Guide resource|four embedded') "Unsafe guide accepted: $bad | $caught"
    }
} finally { [IO.File]::WriteAllBytes($templatePath,$templateBytes) }
Assert (@($manifest.files | Where-Object { $_.path -like 'licenses/*' }).Count -eq 14) 'Dependency licenses missing.'
$beforeHash = (Get-FileHash (Join-Path $stable $manifest.artifact)).Hash
$beforeBuilds = $script:buildCalls
Fail-Case 'repeat' 'already exists'
Assert ($script:buildCalls -eq $beforeBuilds -and (Get-FileHash (Join-Path $stable $manifest.artifact)).Hash -ceq $beforeHash) 'Repeat modified package or started build.'
Invoke-ReverbRelease -Root $fixture -ReleaseTag v0.1.0 -Candidate rc.1
$candidateDir = Join-Path $fixture 'out/releases/v0.1.0-rc.1'
$candidate = Get-Content (Join-Path $candidateDir 'build-manifest.json') -Raw | ConvertFrom-Json
Assert ($candidate.releaseIdentity -ceq 'v0.1.0-rc.1' -and $candidate.version -ceq '0.1.0' -and $candidate.artifact -match '-rc.1-win-x64') 'Candidate lost its label.'
Assert ($script:ghCalls.Count -eq 0) 'Packaging reached GitHub.'
$publish = @{ Publish = $true; RedistributionReviewed = $true; LicenseReview = 'SIMULATED license review only - no permission to distribute'; ConfirmPublish = "v0.1.0@$sha" }
Fail-Case 'no-gate' 'Redistribution gate' @{ Publish = $true }
Fail-Case 'no-confirmation' 'Separate publication confirmation' @{ Publish = $true; RedistributionReviewed = $true; LicenseReview = $publish.LicenseReview }
Assert ($script:ghCalls.Count -eq 0) 'Unconfirmed publication reached GitHub.'
Fail-Case 'remote-wrong' 'Remote release tag' $publish
$script:case = 'annotated'
Invoke-ReverbRelease -Root $fixture -ReleaseTag v0.1.0 @publish
$script:case = ''
$create = @($script:ghCalls | Where-Object { $_[0] -eq 'release' })
Assert ($create.Count -eq 1 -and $create[0] -contains '--verify-tag' -and $create[0] -notcontains '--prerelease' -and $create[0] -contains '--latest=false') 'Stable publication arguments incorrect.'
$publish.ConfirmPublish = "v0.1.0-rc.1@$sha"
Invoke-ReverbRelease -Root $fixture -ReleaseTag v0.1.0 -Candidate rc.1 @publish
$create = @($script:ghCalls | Where-Object { $_[0] -eq 'release' })
Assert ($create.Count -eq 2 -and $create[1] -contains '--prerelease') 'Candidate publication not marked prerelease.'
$publish.ConfirmPublish = "v0.1.0@$sha"
$zip = Join-Path $stable $manifest.artifact
[IO.File]::AppendAllText($zip, 'tampered')
$beforeCalls = $script:ghCalls.Count
Fail-Case 'tampered' 'SHA-256 mismatch' $publish
Assert ($script:ghCalls.Count -eq $beforeCalls) 'Tampered package reached GitHub.'
# Exercise the public entry point in a real child shell; all cases fail before build/network.
$shell = (Get-Process -Id $PID).Path
$savedPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
try {
    $output = @(& $shell -NoProfile -NonInteractive -ExecutionPolicy Bypass -File (Join-Path $fixture 'scripts/release.ps1') -ReleaseTag v0.2.0 2>&1)
    Assert ($LASTEXITCODE -eq 1 -and ($output -join "`n") -match 'does not match') 'Public version mismatch exit incorrect.'
    'dirty' | Set-Content (Join-Path $fixture 'tracked.txt')
    $output = @(& $shell -NoProfile -NonInteractive -ExecutionPolicy Bypass -File (Join-Path $fixture 'scripts/release.ps1') -ReleaseTag v0.1.0 2>&1)
    Assert ($LASTEXITCODE -eq 1 -and ($output -join "`n") -match 'clean worktree') 'Public dirty-tree exit incorrect.'
    $null = & $actualNative $tools.Git @('-C', $fixture, 'restore', 'tracked.txt')
    $output = @(& $shell -NoProfile -NonInteractive -ExecutionPolicy Bypass -File (Join-Path $fixture 'scripts/release.ps1') -ReleaseTag v0.1.0 -Publish 2>&1)
    Assert ($LASTEXITCODE -eq 1 -and ($output -join "`n") -match 'Redistribution gate') 'Public unconfirmed publication exit incorrect.'
} finally { $ErrorActionPreference = $savedPreference }
Write-Host "PASS $script:checks simulated release checks. No native build, CTest, DAW or real publication performed. Fixtures: $fixture"
