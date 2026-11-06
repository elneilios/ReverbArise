[CmdletBinding()]
param([string]$Executable='', [string]$OutputParent='')
$ErrorActionPreference='Stop'
$taskRepo=Split-Path $PSScriptRoot -Parent
if(-not $Executable){$Executable=Join-Path $taskRepo 'out/build/windows-msvc/Release/preverb_storage_audit.exe'}
if(-not $OutputParent){$OutputParent=Join-Path ([IO.Path]::GetTempPath()) 'Pv8'}
$Executable=(Resolve-Path -LiteralPath $Executable).Path
$taskParent=[IO.Path]::GetFullPath($OutputParent)
$null=New-Item -ItemType Directory -Path $taskParent -Force
if((Get-Item -LiteralPath $taskParent).Attributes -band [IO.FileAttributes]::ReparsePoint){throw 'Output parent must not be a reparse point'}
$taskRun=Join-Path $taskParent ('Issue8Audit-'+[guid]::NewGuid().ToString('N').Substring(0,8))
if(Test-Path -LiteralPath $taskRun){throw 'Audit requires a fresh disposable root'}
& $Executable --growth $taskRun
if($LASTEXITCODE -ne 0){throw "Growth fixture failed; retained $taskRun"}
$taskProcesses=@()
try {
    foreach($taskIdentity in 1,2){
        $taskArgs=@('--writer',('"'+$taskRun+'"'),$taskIdentity)
        $taskProcesses+=Start-Process -FilePath $Executable -ArgumentList $taskArgs -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $taskRun "writer-$taskIdentity.stdout.log") -RedirectStandardError (Join-Path $taskRun "writer-$taskIdentity.stderr.log")
    }
    $taskDeadline=[DateTime]::UtcNow.AddSeconds(30)
    while(-not ((Test-Path -LiteralPath (Join-Path $taskRun 'writer-ready-1')) -and (Test-Path -LiteralPath (Join-Path $taskRun 'writer-ready-2')))){
        if([DateTime]::UtcNow -gt $taskDeadline){throw 'Concurrent-writer barrier timed out'}
        foreach($taskProcess in $taskProcesses){if($taskProcess.HasExited){throw 'Writer exited before barrier'}}
        Start-Sleep -Milliseconds 20
    }
    [IO.File]::WriteAllText((Join-Path $taskRun 'writer-go'),'Both processes loaded the same accepted snapshot')
    foreach($taskProcess in $taskProcesses){
        if(-not $taskProcess.WaitForExit(30000)){throw 'Concurrent writer timed out'}
        if($taskProcess.ExitCode -ne 0){throw "Writer $($taskProcess.Id) failed: $($taskProcess.ExitCode)"}
    }
} finally { foreach($taskProcess in $taskProcesses){$taskProcess.Dispose()} }
& $Executable --verify-writers $taskRun
if($LASTEXITCODE -ne 0){throw "Writer verification failed; retained $taskRun"}
$taskReportPath=Join-Path $taskRun 'audit.json'
$taskReport=Get-Content -LiteralPath $taskReportPath -Raw | ConvertFrom-Json
$taskReport | Add-Member -NotePropertyName environment -NotePropertyValue ([ordered]@{
    os=[Environment]::OSVersion.VersionString
    architecture=$env:PROCESSOR_ARCHITECTURE
    processor=$env:PROCESSOR_IDENTIFIER
    powershell=$PSVersionTable.PSVersion.ToString()
    revision=(& git -C $taskRepo rev-parse HEAD)
    dirty=[bool](& git -C $taskRepo status --porcelain)
    executableSha256=(Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash.ToLowerInvariant()
    runnerSha256=(Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash.ToLowerInvariant()
    fileSystem=([IO.DriveInfo]::new([IO.Path]::GetPathRoot($taskRun))).DriveFormat
    harnessSha256=(Get-FileHash -LiteralPath (Join-Path $taskRepo 'tests/storage_audit.cpp') -Algorithm SHA256).Hash.ToLowerInvariant()
    sourceBaseline='df765d95170caaaab10a646e43088ea3be66ce53'
    cache='Fresh native build tree; reused pinned dependency sources. Runtime filesystem cache uncontrolled.'
})
[IO.File]::WriteAllText($taskReportPath,($taskReport | ConvertTo-Json -Depth 32),[Text.UTF8Encoding]::new($false))
Write-Output "PASS: $taskReportPath"
Write-Output 'Synthetic assets retained. This command never scans, migrates or cleans user storage.'
