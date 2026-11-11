[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Debug',
    [switch] $Test,
    [switch] $PluginOnly,
    [string] $ReleaseTag = '',
    [string] $Candidate = '',
    [switch] $PrRegression
)

. (Join-Path $PSScriptRoot 'build-support.ps1')
$result = Invoke-ReverbBuild -Root (Split-Path $PSScriptRoot -Parent) -Configuration $Configuration -Test:$Test -PluginOnly:$PluginOnly -ReleaseTag $ReleaseTag -Candidate $Candidate -PrRegression:$PrRegression
exit $result
