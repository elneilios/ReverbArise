[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Debug',
    [switch] $Test,
    [switch] $PluginOnly,
    [switch] $CompatibilityTest,
    [string] $ReleaseTag = '',
    [string] $Candidate = ''
)

. (Join-Path $PSScriptRoot 'build-support.ps1')
$result = Invoke-ReverbBuild -Root (Split-Path $PSScriptRoot -Parent) -Configuration $Configuration -Test:$Test -PluginOnly:$PluginOnly -CompatibilityTest:$CompatibilityTest -ReleaseTag $ReleaseTag -Candidate $Candidate
exit $result
