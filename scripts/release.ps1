[CmdletBinding(DefaultParameterSetName = 'Package')]
param(
    [Parameter(Mandatory = $true)][string] $ReleaseTag,
    [string] $Candidate = '',
    [Parameter(Mandatory = $true, ParameterSetName = 'Publish')][switch] $Publish,
    [Parameter(ParameterSetName = 'Publish')][switch] $RedistributionReviewed,
    [Parameter(ParameterSetName = 'Publish')][string] $LicenseReview = '',
    [Parameter(ParameterSetName = 'Publish')][string] $ConfirmPublish = ''
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'build-support.ps1')
. (Join-Path $PSScriptRoot 'release-support.ps1')
try {
    Invoke-ReverbRelease -Root (Split-Path $PSScriptRoot -Parent) @PSBoundParameters
    exit 0
} catch {
    Write-Host "FAIL release: $($_.Exception.Message)"
    exit 1
}
