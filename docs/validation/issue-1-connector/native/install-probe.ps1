[CmdletBinding(SupportsShouldProcess=$true)]
param([switch]$Remove,[string]$ScriptHardwareRoot='D:/Users/neila/Image-Line/FL Studio/Settings/Hardware')
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../../..'))
. (Join-Path $repo 'scripts/build-support.ps1')
. (Join-Path $repo 'scripts/release-support.ps1')
. (Join-Path $repo 'scripts/test-install-support.ps1')
$name='ReverbArise FL Context Probe.vst3'
$destination=Join-Path $env:ProgramFiles 'Common Files/VST3'
$target=[IO.Path]::GetFullPath((Join-Path $destination $name))
$source=Join-Path $repo "out/issue-1-native/build/ra_context_probe_artefacts/Release/VST3/$name"
$backupRoot=Join-Path $repo 'out/issue-1-native/retained-installs'
$scriptTarget=Join-Path $ScriptHardwareRoot 'ReverbAriseNativeContextProbe'
if((Split-Path $target -Leaf) -cne $name){throw 'Unexpected target; refusing installation'}
if(-not $PSCmdlet.ShouldProcess($target, $(if($Remove){'Retain/remove only diagnostic probe bundle'}else{'Install separate diagnostic probe bundle and owned script folder'}))){return}
Assert-TestHostClosed
if(-not $Remove){
    & (Join-Path $PSScriptRoot 'build-probe.ps1')
    if(-not $?){throw 'Probe build/test failed; nothing installed'}
}
$null=[IO.Directory]::CreateDirectory($backupRoot)
$run=(Get-Date -Format yyyyMMdd-HHmmss)+'-'+[guid]::NewGuid().ToString('N')
$lock=[IO.File]::Open((Join-Path $destination '.ra-context-probe-install.lock'),'OpenOrCreate','ReadWrite','None')
try {
    if($Remove){
        if(Test-Path -LiteralPath $target){
            $null=Get-ReleaseInventory $target
            $retained=Join-Path $backupRoot $run;$null=[IO.Directory]::CreateDirectory($retained)
            [IO.Directory]::Move($target,(Join-Path $retained $name))
            Write-Host "Probe bundle retained at $retained"
        }
        Write-Host "Script left intact with evidence: $scriptTarget. First detach its test input in FL; then retain/remove ONLY this owned folder."
        return
    }
    if(-not(Test-Path -LiteralPath $source)){throw 'Build the probe successfully before installation'}
    if(-not(Test-Path -LiteralPath $ScriptHardwareRoot)){throw 'Pass the existing configured FL Settings/Hardware path; no controller settings are edited'}
    $buildLock=Enter-BuildLock (Join-Path $repo 'out/issue-1-native/build')
    try {
        $stage=Join-Path $destination ('.ra-context-probe-stage-'+$run)
        Copy-Item -LiteralPath $source -Destination $stage -Recurse
        Assert-TestBundleMatch $source $stage
        $previous=$null
        if(Test-Path -LiteralPath $target){
            $previous=Join-Path $backupRoot $run;$null=[IO.Directory]::CreateDirectory($previous)
            [IO.Directory]::Move($target,(Join-Path $previous $name))
        }
        try {[IO.Directory]::Move($stage,$target)} catch {
            if($previous){[IO.Directory]::Move((Join-Path $previous $name),$target)}
            throw
        }
    } finally {$buildLock.Dispose()}
    # Refuse overwrite: installed custom scripts/evidence are retained for explicit review.
    if(Test-Path -LiteralPath $scriptTarget){Write-Host "Existing probe script not overwritten: $scriptTarget"} else {
        $null=[IO.Directory]::CreateDirectory($scriptTarget)
        Copy-Item -LiteralPath (Join-Path $PSScriptRoot '../device_ra_context_native.py') -Destination (Join-Path $scriptTarget 'device_ra_context_native.py')
    }
    Write-Host "Installed separate probe: $target"
    Write-Host "Script staged (NOT activated): $scriptTarget"
    Write-Host "Binary SHA256: $((Get-FileHash -LiteralPath (Join-Path $target 'Contents/x86_64-win/ReverbArise FL Context Probe.vst3')).Hash)"
} finally {$lock.Dispose()}
