[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string] $Project,
    [Parameter(Mandatory=$true)][string] $TraceDirectory,
    [string] $HostExecutable = 'C:\Program Files\Image-Line\FL Studio 2026\FL64.exe'
)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'test-install-support.ps1')
Assert-TestHostClosed
$projectPath=(Resolve-Path -LiteralPath $Project).Path
$hostPath=(Resolve-Path -LiteralPath $HostExecutable).Path
if($projectPath.Contains('"')) { throw 'Project path cannot contain quotation marks.' }
$tracePath=[IO.Path]::GetFullPath($TraceDirectory)
$null=[IO.Directory]::CreateDirectory($tracePath)
$oldTrace=$env:REVERBARISE_TIMING_TRACE_DIR
try {
    $env:REVERBARISE_TIMING_TRACE_DIR=$tracePath
    $hostProcess=Start-Process -FilePath $hostPath -ArgumentList ('"'+$projectPath+'"') -WindowStyle Hidden -PassThru
    [pscustomobject]@{ ProcessId=$hostProcess.Id; Project=$projectPath; TraceDirectory=$tracePath;
        HostVersion=(Get-Item -LiteralPath $hostPath).VersionInfo.FileVersion }
} finally {
    if($null -eq $oldTrace){Remove-Item Env:REVERBARISE_TIMING_TRACE_DIR -ErrorAction SilentlyContinue}
    else{$env:REVERBARISE_TIMING_TRACE_DIR=$oldTrace}
}
