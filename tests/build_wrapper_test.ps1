# Native command doubles only: this does not validate C++, MSVC or plugin tests.
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$repository = Split-Path $PSScriptRoot -Parent
$fixture = Join-Path $repository ('out/build-wrapper-fixtures/' + [guid]::NewGuid().ToString('N') + '/checkout with spaces')
$null = New-Item -ItemType Directory -Path (Join-Path $fixture 'scripts') -Force
Copy-Item -LiteralPath (Join-Path $repository 'scripts/build.ps1'), (Join-Path $repository 'scripts/build-support.ps1') -Destination (Join-Path $fixture 'scripts')
Copy-Item -LiteralPath (Join-Path $repository 'CMakePresets.json') -Destination $fixture
$tools = Join-Path $fixture 'fake tools'
$null = New-Item -ItemType Directory -Path $tools -Force
$source = Join-Path $tools 'Double.cs'
@'
using System;
using System.IO;
using System.Reflection;
using System.Threading;
class Double {
  static int Main(string[] args) {
    string tool = Path.GetFileNameWithoutExtension(Assembly.GetExecutingAssembly().Location).ToLowerInvariant();
    string all = String.Join(" ", args);
    if (tool == "vswhere") {
      if (Environment.GetEnvironmentVariable("FIXTURE_MISSING_VS") != "1") Console.WriteLine(Environment.GetEnvironmentVariable("FIXTURE_VS"));
      return 0;
    }
    if (tool == "git") {
      if (Environment.GetEnvironmentVariable("FIXTURE_NO_REVISION") == "1") { Console.Error.WriteLine("revision unavailable"); return 1; }
      if (all.Contains("rev-parse")) Console.WriteLine("abc1234");
      if (all.Contains("status")) Console.WriteLine(" M fixture");
      return 0;
    }
    if (all == "-E capabilities") {
      Console.WriteLine(Environment.GetEnvironmentVariable("FIXTURE_BAD_CMAKE") == "1" && !Assembly.GetExecutingAssembly().Location.Contains("Common7")
        ? "{\"generators\":[]}" : "{\"generators\":[{\"name\":\"Visual Studio 18 2026\"}]}");
      return 0;
    }
    string stage = tool == "ctest" ? "Test" : all.Contains("--build") ? "Build" : "Configure";
    File.AppendAllText(Environment.GetEnvironmentVariable("FIXTURE_TRACE"), stage + "|" + all + "|" + Environment.CurrentDirectory + "\n");
    Console.WriteLine("fixture stdout " + stage);
    Console.Error.WriteLine("fixture stderr " + stage);
    if (Environment.GetEnvironmentVariable("FIXTURE_FAIL") == stage) return 23;
    if (stage == "Test" && Environment.GetEnvironmentVariable("FIXTURE_EMPTY") == "1") { Console.WriteLine("No tests were found"); return 8; }
    return 0;
  }
}
'@ | Set-Content -LiteralPath $source
# Windows PowerShell's framework compiler produces a standalone .NET Framework EXE
# usable by both shells. No compiler packages or third-party test modules needed.
$compiler = Join-Path $tools 'compile.ps1'
@'
param($Source, $Output)
$ErrorActionPreference = 'Stop'
Add-Type -Path $Source -OutputAssembly $Output -OutputType ConsoleApplication
'@ | Set-Content -LiteralPath $compiler
$frameworkShell = Join-Path $env:SystemRoot 'System32/WindowsPowerShell/v1.0/powershell.exe'
& $frameworkShell -NoProfile -ExecutionPolicy Bypass -File $compiler $source (Join-Path $tools 'double.exe')
if ($LASTEXITCODE -ne 0) { throw 'Could not compile fixture command double.' }
foreach ($name in @('cmake', 'ctest', 'git')) { Copy-Item -LiteralPath (Join-Path $tools 'double.exe') -Destination (Join-Path $tools "$name.exe") }
$installer = Join-Path $tools 'Microsoft Visual Studio/Installer'
$null = New-Item -ItemType Directory -Path $installer -Force
Copy-Item -LiteralPath (Join-Path $tools 'double.exe') -Destination (Join-Path $installer 'vswhere.exe')
$installation = Join-Path $tools 'VS installation'
$bundled = Join-Path $installation 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin'
$null = New-Item -ItemType Directory -Path $bundled -Force
foreach ($name in @('cmake', 'ctest')) { Copy-Item -LiteralPath (Join-Path $tools 'double.exe') -Destination (Join-Path $bundled "$name.exe") }
$names = @('PATH', 'ProgramFiles', 'ProgramFiles(x86)', 'FIXTURE_VS', 'FIXTURE_TRACE', 'FIXTURE_FAIL', 'FIXTURE_EMPTY', 'FIXTURE_MISSING_VS', 'FIXTURE_NO_REVISION', 'FIXTURE_BAD_CMAKE')
$saved = @{}
foreach ($name in $names) { $saved[$name] = [Environment]::GetEnvironmentVariable($name) }
$shell = (Get-Process -Id $PID).Path
$trace = Join-Path $tools 'trace.txt'
$script:checks = 0
function Assert($Condition, [string] $Message) {
    if (-not $Condition) { throw $Message }
    $script:checks++
}
function Run-Case([string[]] $Arguments = @(), [int] $Expected = 0) {
    [IO.File]::WriteAllText($trace, '')
    $savedPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue' # Expected parameter-binding errors use stderr on 5.1.
    try {
        $output = @(& $shell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $fixture 'scripts/build.ps1') @Arguments 2>&1)
        $actual = $LASTEXITCODE
    } finally { $ErrorActionPreference = $savedPreference }
    Assert ($actual -eq $Expected) "Expected exit $Expected, got $actual : $output"
    return ($output -join "`n")
}
try {
    $env:PATH = "$tools;$tools" # Duplicate executable matches must select one tool.
    $env:ProgramFiles = $tools
    ${env:ProgramFiles(x86)} = $tools
    $env:FIXTURE_VS = $installation; $env:FIXTURE_TRACE = $trace
    foreach ($name in @('FIXTURE_FAIL', 'FIXTURE_EMPTY', 'FIXTURE_MISSING_VS', 'FIXTURE_NO_REVISION', 'FIXTURE_BAD_CMAKE')) { [Environment]::SetEnvironmentVariable($name, '') }
    $start = (Get-Location).Path
    $output = Run-Case
    Assert ($output -match 'Test: not requested' -and $output -match 'abc1234 \(dirty\)') 'Build-only summary/revision incorrect.'
    Assert ((Get-Content -LiteralPath $trace -Raw) -match 'windows-msvc-debug') 'Default did not select Debug.'
    foreach ($configuration in @('Debug', 'Release')) {
        $output = Run-Case @('-Configuration', $configuration, '-Test')
        $calls = Get-Content -LiteralPath $trace
        Assert ($calls.Count -eq 3 -and $calls[0].StartsWith('Configure|') -and $calls[1].StartsWith('Build|') -and $calls[2].StartsWith('Test|')) 'Stages not sequential.'
        Assert ($calls[2].Contains("windows-msvc-$($configuration.ToLowerInvariant()) --no-tests=error")) 'CTest config/empty-suite handling incorrect.'
        Assert ($calls[0].Contains('-DFETCHCONTENT_UPDATES_DISCONNECTED=ON')) 'Dependency update suppression absent.'
        Assert ($calls[0].EndsWith($fixture)) 'Arbitrary caller/spaced root was not respected.'
    }
    $null = Run-Case @('-Configuration', 'Other') 1
    Assert ((Get-Item -LiteralPath $trace).Length -eq 0) 'Invalid input launched a native stage.'
    foreach ($failure in @('Configure', 'Build', 'Test')) {
        $env:FIXTURE_FAIL = $failure
        $output = Run-Case @('-Test') 23
        Assert ($output -match "$failure failed" -and $output -match 'fixture stderr' -and $output -match 'fixture stdout') 'Failure diagnostics lost.'
        $calls = @(Get-Content -LiteralPath $trace)
        Assert ($calls[-1].StartsWith("$failure|")) 'A later stage ran after failure.'
    }
    $env:FIXTURE_FAIL = ''; $env:FIXTURE_EMPTY = '1'
    $null = Run-Case @('-Test') 8
    $env:FIXTURE_EMPTY = ''; $env:FIXTURE_MISSING_VS = '1'
    $output = Run-Case @() 1
    Assert ($output -match 'Install Visual Studio 2026') 'Missing toolchain diagnostic absent.'
    $env:FIXTURE_MISSING_VS = ''; $env:FIXTURE_NO_REVISION = '1'
    $output = Run-Case
    Assert ($output -match 'Git unavailable') 'Unavailable Git revision fabricated.'
    $env:FIXTURE_NO_REVISION = ''; $env:FIXTURE_BAD_CMAKE = '1'
    $output = Run-Case
    Assert ($output.Contains($bundled)) 'Incompatible PATH CMake did not fall back to bundled tools.'
    $env:FIXTURE_BAD_CMAKE = ''
    Move-Item -LiteralPath (Join-Path $tools 'git.exe') -Destination (Join-Path $tools 'git.hidden')
    $output = Run-Case @() 1
    Assert ($output -match 'Git is required') 'Missing Git diagnostic absent.'
    Move-Item -LiteralPath (Join-Path $tools 'git.hidden') -Destination (Join-Path $tools 'git.exe')
    Move-Item -LiteralPath (Join-Path $tools 'ctest.exe') -Destination (Join-Path $tools 'ctest.hidden')
    Move-Item -LiteralPath (Join-Path $bundled 'ctest.exe') -Destination (Join-Path $bundled 'ctest.hidden')
    $output = Run-Case @() 1
    Assert ($output -match 'Compatible CMake and adjacent CTest not found') 'Missing CTest diagnostic absent.'
    Move-Item -LiteralPath (Join-Path $tools 'ctest.hidden') -Destination (Join-Path $tools 'ctest.exe')
    Move-Item -LiteralPath (Join-Path $bundled 'ctest.hidden') -Destination (Join-Path $bundled 'ctest.exe')

    . (Join-Path $fixture 'scripts/build-support.ps1')
    $binary = Join-Path $fixture 'out/build/windows-msvc'
    $cache = Join-Path $binary 'CMakeCache.txt'
    [IO.File]::WriteAllText($cache, "CMAKE_HOME_DIRECTORY:INTERNAL=C:/other-checkout`n")
    $output = Run-Case @() 1
    Assert ($output -match 'Incompatible CMake cache') 'Foreign cache accepted.'
    Assert ((Get-Content -LiteralPath $cache -Raw).Contains('C:/other-checkout')) 'Foreign cache modified.'
    [IO.File]::WriteAllText($cache, "CMAKE_HOME_DIRECTORY:INTERNAL=$($fixture.Replace('\','/'))`nCMAKE_GENERATOR:INTERNAL=Visual Studio 18 2026`nCMAKE_GENERATOR_PLATFORM:INTERNAL=x64`n")
    $marker = Join-Path $binary 'cached-dependency.marker'
    [IO.File]::WriteAllText($marker, 'keep')
    $lock = Enter-BuildLock $binary
    try {
        $output = Run-Case @('-Configuration', 'Release') 1
        Assert ($output -match 'Build directory is busy') 'Concurrent subprocess did not reject the owned tree.'
        Assert ((Get-Item -LiteralPath $trace).Length -eq 0) 'Contender launched a build stage.'
        $otherLock = Enter-BuildLock (Join-Path $fixture 'independent-worktree/out/build/windows-msvc')
        $otherLock.Dispose()
        Assert $true 'Independent directory lock failed.'
        $otherRoot = Join-Path $fixture 'independent-worktree'
        Copy-Item -LiteralPath (Join-Path $fixture 'CMakePresets.json') -Destination $otherRoot
        $otherResult = Invoke-ReverbBuild -Root $otherRoot -Configuration Release
        Assert ($otherResult -eq 0) 'Independent worktree could not build while first directory was locked.'
    } finally { $lock.Dispose() }
    $null = Run-Case @('-Configuration', 'Release')
    Assert ((Get-Content -LiteralPath $marker -Raw) -eq 'keep') 'Warm run deleted cached state.'
    # Same-process checks prove the function restores location on both outcomes.
    $null = Invoke-ReverbBuild -Root $fixture
    Assert ((Get-Location).Path -eq $start) 'Success did not restore caller location.'
    $env:FIXTURE_FAIL = 'Build'
    $result = Invoke-ReverbBuild -Root $fixture -Test
    Assert ($result -eq 23 -and (Get-Location).Path -eq $start) 'Failure did not preserve exit/location.'
    $released = Enter-BuildLock $binary
    $released.Dispose()
    Assert $true 'Failure did not release directory lock.'
    Write-Host "PASS: $script:checks fixture assertions on PowerShell $($PSVersionTable.PSVersion). No native MSVC builds/tests performed."
    Write-Host "Fixture logs retained: $fixture"
} finally {
    foreach ($name in $names) { [Environment]::SetEnvironmentVariable($name, $saved[$name]) }
}
