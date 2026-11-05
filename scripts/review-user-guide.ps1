[CmdletBinding()]
param([string] $Root = (Split-Path $PSScriptRoot -Parent), [Parameter(Mandatory=$true)][ValidateNotNullOrEmpty()][string] $Summary)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'user-guide-support.ps1')
if ([string]::IsNullOrWhiteSpace($Summary)) { throw 'Describe the completed code/spec/user-facing guide review.' }
$review = [ordered]@{ schemaVersion=1; normalization='utf8-lf-v1'; summary=$Summary.Trim(); files=@(Get-UserGuideInputs $Root) }
$review | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $Root 'docs/development/user-guide-review.json') -Encoding UTF8
Assert-UserGuideCurrent $Root
Write-Host 'Recorded explicit user-guide review. Commit the guide and review record before packaging.'
