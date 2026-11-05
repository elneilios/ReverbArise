[CmdletBinding()]
param([string]$JuceRoot='C:/git/k7ma-preverb/out/build/windows-msvc/_deps/juce-src')
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../../..'))
. (Join-Path $repo 'scripts/build-support.ps1')
$tools=Resolve-BuildTools
$binary=Join-Path $repo 'out/issue-1-native/build'
$evidence=Join-Path $repo 'out/issue-1-native/evidence'
$lock=Enter-BuildLock $binary
try {
    Assert-BuildCache $binary $PSScriptRoot
    & $tools.CMake -S $PSScriptRoot -B $binary -G 'Visual Studio 18 2026' -A x64 "-DJUCE_ROOT=$($JuceRoot.Replace('\','/'))" "-DEVIDENCE_DIR=$($evidence.Replace('\','/'))"
    if($LASTEXITCODE -ne 0){throw 'Probe configure failed'}
    & $tools.CMake --build $binary --config Release --target ra_context_probe_VST3 probe_native_test --parallel 4
    if($LASTEXITCODE -ne 0){throw 'Probe build failed'}
    & $tools.CTest --test-dir $binary -C Release --output-on-failure --no-tests=error
    if($LASTEXITCODE -ne 0){throw 'Probe native test failed'}
    Write-Host "PASS separately identified Release probe; evidence: $evidence"
} finally {$lock.Dispose()}
