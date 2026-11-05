[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Debug',
    [switch] $Test,
    [switch] $PluginOnly
)

. (Join-Path $PSScriptRoot 'build-support.ps1')
$result = Invoke-ReverbBuild -Root (Split-Path $PSScriptRoot -Parent) -Configuration $Configuration -Test:$Test -PluginOnly:$PluginOnly
exit $result
