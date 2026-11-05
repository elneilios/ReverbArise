[CmdletBinding()]
param([string] $Root = (Split-Path $PSScriptRoot -Parent))
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'user-guide-support.ps1')
try { Assert-UserGuideCurrent $Root; Write-Host 'PASS user guide currency (read-only)'; exit 0 }
catch { Write-Host "FAIL user guide currency: $($_.Exception.Message)"; exit 1 }
