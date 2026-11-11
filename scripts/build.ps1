[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Debug',
    [switch] $Test,
    [switch] $PluginOnly,
    [string] $ReleaseTag = '',
    [string] $Candidate = '',
    [switch] $PrRegression,
    [string] $TestTarget = '',
    [string] $TestFilter = ''
)

. (Join-Path $PSScriptRoot 'build-support.ps1')
$result = Invoke-ReverbBuild -Root (Split-Path $PSScriptRoot -Parent) -Configuration $Configuration -Test:$Test -PluginOnly:$PluginOnly -ReleaseTag $ReleaseTag -Candidate $Candidate -PrRegression:$PrRegression -TestTarget $TestTarget -TestFilter $TestFilter
exit $result
