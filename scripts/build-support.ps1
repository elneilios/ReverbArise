# Shared with the dependency-free orchestration fixtures; no work occurs on import.
function Get-BuildVisualStudio {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $paths = @(& $vswhere -version '[18.0,19.0)' -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath)
        if ($LASTEXITCODE -ne 0) { throw 'Visual Studio discovery failed. Repair the Visual Studio Installer.' }
        return $paths
    }
    # Documented default locations if the installer discovery utility is absent.
    foreach ($edition in @('Community', 'Professional', 'Enterprise', 'BuildTools')) {
        $path = Join-Path $env:ProgramFiles "Microsoft Visual Studio/18/$edition"
        if (Test-Path -LiteralPath (Join-Path $path 'VC/Tools/MSVC')) { $path }
    }
}

function Resolve-BuildTools {
    if ($env:OS -ne 'Windows_NT') { throw 'Windows x64 and Visual Studio 2026 C++ tools are required.' }
    $git = Get-Command git.exe -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $git) { throw 'Git is required. Install Git and add it to PATH.' }
    $installations = @(Get-BuildVisualStudio)
    if ($installations.Count -eq 0) { throw 'Install Visual Studio 2026 Desktop development with C++, MSVC x64/x86 tools and a Windows SDK.' }
    $candidates = @()
    $onPath = Get-Command cmake.exe -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($onPath) { $candidates += $onPath.Source }
    foreach ($installation in $installations) {
        $candidates += Join-Path $installation 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
    }
    foreach ($candidate in ($candidates | Select-Object -Unique)) {
        $ctest = Join-Path (Split-Path $candidate) 'ctest.exe'
        if (-not (Test-Path -LiteralPath $candidate) -or -not (Test-Path -LiteralPath $ctest)) { continue }
        $capabilities = & $candidate -E capabilities 2>$null
        if ($LASTEXITCODE -ne 0) { continue }
        try { $info = ($capabilities -join "`n") | ConvertFrom-Json } catch { continue }
        if (@($info.generators | Where-Object { $_.name -eq 'Visual Studio 18 2026' }).Count -gt 0) {
            return @{ CMake = $candidate; CTest = $ctest; Git = $git.Source }
        }
    }
    throw 'Compatible CMake and adjacent CTest not found. Install VS 2026 C++ CMake tools for Windows (generator Visual Studio 18 2026 required).'
}

function Enter-BuildLock([string] $BinaryDirectory) {
    # A Windows file handle, rather than file existence, owns the lock. Process
    # termination releases it; never delete the file (which would introduce races).
    $null = [IO.Directory]::CreateDirectory($BinaryDirectory)
    $path = Join-Path $BinaryDirectory '.reverbarise-build.lock'
    try {
        return [IO.File]::Open($path, [IO.FileMode]::OpenOrCreate, [IO.FileAccess]::ReadWrite, [IO.FileShare]::None)
    } catch [IO.IOException] {
        throw "Build directory is busy or inaccessible: $BinaryDirectory. Another build.ps1 may own it. Retry after it finishes; do not delete the lock file."
    }
}

function Assert-BuildCache([string] $BinaryDirectory, [string] $Root) {
    $cache = Join-Path $BinaryDirectory 'CMakeCache.txt'
    if (-not (Test-Path -LiteralPath $cache)) { return }
    $expected = @{
        CMAKE_HOME_DIRECTORY = $Root.Replace('\', '/').TrimEnd('/')
        CMAKE_GENERATOR = 'Visual Studio 18 2026'
        CMAKE_GENERATOR_PLATFORM = 'x64'
    }
    $seen = @{}
    foreach ($line in Get-Content -LiteralPath $cache) {
        if ($line -match '^(CMAKE_HOME_DIRECTORY|CMAKE_GENERATOR|CMAKE_GENERATOR_PLATFORM):[^=]+=(.*)$') {
            $key = $matches[1]; $value = $matches[2].Replace('\', '/').TrimEnd('/')
            $seen[$key] = $true
            if ($value -ine $expected[$key]) {
                throw "Incompatible CMake cache ($key=$value) in $BinaryDirectory. Use the original checkout/toolchain or explicitly relocate the old build tree while idle; no cache was deleted."
            }
        }
    }
    $missing = @($expected.Keys | Where-Object { -not $seen.ContainsKey($_) } | Sort-Object)
    if ($missing.Count -gt 0) {
        throw "Incomplete CMake cache (missing required entries: $($missing -join ', ')) in $BinaryDirectory. Inspect the failed configure logs and explicitly recover or relocate the old build tree while idle; no cache was deleted."
    }
}

function Invoke-BuildNative([string] $Executable, [string[]] $Arguments, [string] $Log) {
    # Redirection writes incrementally; no full successful build log is retained in memory.
    # Windows PowerShell reports native stderr as ErrorRecords, not native failure.
    $savedPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    $PSNativeCommandUseErrorActionPreference = $false
    try {
        & $Executable @Arguments > $Log 2>&1
        $nativeCode = $LASTEXITCODE
        if ($null -eq $nativeCode) { return 1 }
        return $nativeCode
    } finally { $ErrorActionPreference = $savedPreference }
}

function Get-BuildRevision([string] $Git, [string] $Root) {
    # Revision failures are metadata failures, including native stderr on PS 5.1.
    $ErrorActionPreference = 'Continue'
    $PSNativeCommandUseErrorActionPreference = $false
    $head = @(& $Git -C $Root rev-parse --short HEAD 2>$null)
    if ($LASTEXITCODE -ne 0 -or $head.Count -eq 0) { return 'unavailable' }
    $revision = $head[0]
    $dirty = @(& $Git -C $Root status --porcelain 2>$null)
    if ($LASTEXITCODE -ne 0) { return "$revision (worktree status unavailable)" }
    if ($dirty.Count -gt 0) { return "$revision (dirty)" }
    return $revision
}

function Invoke-ReverbBuild {
    [CmdletBinding()]
    param([string] $Root, [ValidateSet('Debug', 'Release')][string] $Configuration = 'Debug', [switch] $Test)
    $ErrorActionPreference = 'Stop'
    $Configuration = if ($Configuration -ieq 'Release') { 'Release' } else { 'Debug' }
    $preset = 'windows-msvc-' + $Configuration.ToLowerInvariant()
    $stages = [ordered]@{ Preflight = 'not run'; Configure = 'not run'; Build = 'not run'; Test = 'not requested' }
    if ($Test) { $stages.Test = 'not run' }
    $revision = 'unavailable'; $logDirectory = 'unavailable'; $lock = $null; $code = 1; $stage = 'Preflight'
    Push-Location -LiteralPath $Root
    try {
        $tools = Resolve-BuildTools
        $revision = Get-BuildRevision $tools.Git $Root
        # Resolve binaryDir from the unchanged configure preset rather than using a global lock.
        $presets = Get-Content -LiteralPath (Join-Path $Root 'CMakePresets.json') -Raw | ConvertFrom-Json
        $configure = @($presets.configurePresets | Where-Object { $_.name -eq 'windows-msvc' })[0]
        $binaryDirectory = [IO.Path]::GetFullPath($configure.binaryDir.Replace('${sourceDir}', $Root).Replace('${presetName}', 'windows-msvc'))
        $lock = Enter-BuildLock $binaryDirectory
        Assert-BuildCache $binaryDirectory $Root
        $run = (Get-Date -Format 'yyyyMMdd-HHmmss') + '-' + [guid]::NewGuid().ToString('N').Substring(0, 8)
        $logDirectory = Join-Path $Root "out/build-logs/$Configuration/$run"
        $null = [IO.Directory]::CreateDirectory($logDirectory)
        $stages.Preflight = 'pass'
        Write-Host "Tools: $($tools.CMake); $($tools.CTest)"
        foreach ($stage in @('Configure', 'Build', 'Test')) {
            if ($stage -eq 'Test' -and -not $Test) { continue }
            $executable = $tools.CMake
            $arguments = switch ($stage) {
                'Configure' { @('--preset', 'windows-msvc', '-DFETCHCONTENT_UPDATES_DISCONNECTED=ON') }
                'Build' { @('--build', '--preset', $preset) }
                'Test' { @('--preset', $preset, '--no-tests=error') }
            }
            if ($stage -eq 'Test') { $executable = $tools.CTest }
            $log = Join-Path $logDirectory ($stage.ToLowerInvariant() + '.log')
            Write-Host "$stage $Configuration... ($log)"
            $code = Invoke-BuildNative $executable $arguments $log
            if ($code -ne 0) {
                $stages[$stage] = "fail ($code)"
                Get-Content -LiteralPath $log -Tail 40 | ForEach-Object { Write-Host $_ }
                throw "$stage failed. Full output: $log"
            }
            $stages[$stage] = 'pass'
            if ($stage -eq 'Test') {
                Select-String -LiteralPath $log -Pattern 'tests passed|Total Test time' | ForEach-Object { Write-Host $_.Line }
            }
        }
        $code = 0
    } catch {
        if ($code -eq 0) { $code = 1 }
        if ($stages[$stage] -notlike 'fail*') { $stages[$stage] = 'fail' }
        Write-Host "ERROR: $($_.Exception.Message)"
    } finally {
        if ($lock) { $lock.Dispose() }
        Pop-Location
        $outcome = if ($code -eq 0) { 'PASS' } else { 'FAIL' }
        Write-Host "$outcome | $Configuration | Git $revision | Exit $code"
        Write-Host (($stages.GetEnumerator() | ForEach-Object { "$($_.Key): $($_.Value)" }) -join '; ')
        Write-Host "Logs: $logDirectory"
    }
    return $code
}
