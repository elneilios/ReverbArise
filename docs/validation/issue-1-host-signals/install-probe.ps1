[CmdletBinding(SupportsShouldProcess=$true)]
param([switch]$Remove)
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../..'))
. (Join-Path $repo 'scripts/build-support.ps1')
. (Join-Path $repo 'scripts/release-support.ps1')
. (Join-Path $repo 'scripts/test-install-support.ps1')
$name='ReverbArise Host Signal Probe.vst3'
$destination=[IO.Path]::GetFullPath((Join-Path $env:ProgramFiles 'Common Files/VST3'))
$target=[IO.Path]::GetFullPath((Join-Path $destination $name))
$root=Join-Path $repo 'out/issue-1-host-signals'
$source=Join-Path $root "build/ra_host_probe_artefacts/Release/VST3/$name"
$production=Join-Path $destination 'ReverbArise.vst3/Contents/x86_64-win/ReverbArise.vst3'
if((Split-Path $target -Leaf) -cne $name -or (Split-Path $target -Parent) -ine $destination){throw 'Unexpected target; refusing mutation'}
if(-not $PSCmdlet.ShouldProcess($target,$(if($Remove){'Retain/remove ONLY host signal probe'}else{'Build/test/install separate host signal probe; no script or controller changes'}))){return}
Assert-TestHostClosed
$productionHash=if(Test-Path -LiteralPath $production){(Get-FileHash -LiteralPath $production).Hash}else{$null}
if(-not $Remove){& (Join-Path $PSScriptRoot 'build-probe.ps1');if(-not $?){throw 'Build/test failed; nothing installed'}}
$backupRoot=Join-Path $root 'retained-installs';$null=[IO.Directory]::CreateDirectory($backupRoot)
$run=(Get-Date -Format yyyyMMdd-HHmmss)+'-'+[guid]::NewGuid().ToString('N')
$lock=[IO.File]::Open((Join-Path $destination '.reverbarise-install.lock'),'OpenOrCreate','ReadWrite','None')
try {
    Assert-TestHostClosed
    if($Remove){if(Test-Path -LiteralPath $target){$retained=Join-Path $backupRoot $run;$null=[IO.Directory]::CreateDirectory($retained);[IO.Directory]::Move($target,(Join-Path $retained $name));Write-Host "Retained probe: $retained"};return}
    $buildLock=Enter-BuildLock (Join-Path $root 'build')
    try {
        $null=Get-ReleaseInventory $source
        $stage=Join-Path $destination ('.ra-host-signal-stage-'+$run)
        Copy-Item -LiteralPath $source -Destination $stage -Recurse
        Assert-TestBundleMatch $source $stage
        Assert-TestHostClosed
        $previous=$null
        if(Test-Path -LiteralPath $target){$previous=Join-Path $backupRoot $run;$null=[IO.Directory]::CreateDirectory($previous);[IO.Directory]::Move($target,(Join-Path $previous $name))}
        try{[IO.Directory]::Move($stage,$target)}catch{if($previous){[IO.Directory]::Move((Join-Path $previous $name),$target)};throw}
        Assert-TestBundleMatch $source $target
        Write-Host "Installed: $target"
        Write-Host "Retained previous: $(if($previous){$previous}else{'none (first install)'})"
        Write-Host "Probe SHA256: $((Get-FileHash -LiteralPath (Join-Path $target ('Contents/x86_64-win/'+$name))).Hash)"
    }finally{$buildLock.Dispose()}
}finally{
    $lock.Dispose()
    if($productionHash -and (Get-FileHash -LiteralPath $production).Hash -cne $productionHash){throw 'Production hash unexpectedly changed; investigate before host tests'}
    Write-Host "Production SHA256 unchanged: $productionHash"
}
