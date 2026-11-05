[CmdletBinding()]
param([string]$ScriptEvidence='D:/Users/neila/Image-Line/FL Studio/Settings/Hardware/ReverbAriseNativeContextProbe/evidence')
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../../..'))
$root=Join-Path $repo 'out/issue-1-native'
$collection=Join-Path $root ('session-'+(Get-Date -Format yyyyMMdd-HHmmss))
if(Test-Path -LiteralPath $collection){throw 'Collection already exists; retry with a new timestamp'}
$null=[IO.Directory]::CreateDirectory($collection)
foreach($entry in @(@('receiver',(Join-Path $root 'evidence')),@('script',$ScriptEvidence))){
    $target=Join-Path $collection $entry[0];$null=[IO.Directory]::CreateDirectory($target)
    if(Test-Path -LiteralPath $entry[1]){
        Get-ChildItem -LiteralPath $entry[1] -File | Where-Object { $_.Name -like 'receiver-*.csv' -or $_.Name -like 'script-*.csv' -or $_.Extension -eq '.wav' } | ForEach-Object {Copy-Item -LiteralPath $_.FullName -Destination $target}
    }
}
& python -B (Join-Path $PSScriptRoot '../analyse_native.py') (Join-Path $collection receiver) --script-directory (Join-Path $collection script) > (Join-Path $collection 'summary.json')
if($LASTEXITCODE -ne 0){Write-Warning 'No receiver evidence or analysis failure: do not claim delivery. Inspect summary/logs.'}
Compress-Archive -LiteralPath $collection -DestinationPath ($collection+'.zip')
Write-Host "Evidence: $collection"
Write-Host "ZIP: $collection.zip"
