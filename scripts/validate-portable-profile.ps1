[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Executable)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$administrator=[Security.Principal.WindowsPrincipal]::new([Security.Principal.WindowsIdentity]::GetCurrent())
if(!$administrator.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)){throw 'This isolated-profile test needs Windows elevation to create and remove its disposable standard account.'}
$id=[guid]::NewGuid().ToString('N')
$testName='RvRecovery'+$id.Substring(0,8)
$publicRoot=[Environment]::GetFolderPath('CommonDocuments')
$bridge=[IO.Path]::GetFullPath((Join-Path $publicRoot ('ReverbArise-Recovery-'+$id)))
if(!$bridge.StartsWith([IO.Path]::GetFullPath($publicRoot).TrimEnd('\')+'\',[StringComparison]::OrdinalIgnoreCase)){throw 'Bridge escaped the public test parent'}
$report=[ordered]@{bridge=$bridge;account=$testName;sourceExecutable=[IO.Path]::GetFullPath($Executable);testExit=$null;accountRemoved=$false;profileRemoved=$false}
$created=$null;$child=$null
New-Item -ItemType Directory -Path $bridge|Out-Null
try {
    $binary=Join-Path $bridge 'portable-recovery.exe'
    Copy-Item -LiteralPath $Executable -Destination $binary
    $report.binarySha256=(Get-FileHash -LiteralPath $binary -Algorithm SHA256).Hash
    & $binary '--prepare-profile' $bridge *> (Join-Path $bridge 'prepare.log')
    if($LASTEXITCODE -ne 0){throw 'Profile fixture preparation failed'}
    $random=[byte[]]::new(36);$rng=[Security.Cryptography.RandomNumberGenerator]::Create();$rng.GetBytes($random);$rng.Dispose()
    $password=ConvertTo-SecureString ([Convert]::ToBase64String($random)+'aA1!') -AsPlainText -Force
    $created=New-LocalUser -Name $testName -Password $password -AccountExpires (Get-Date).AddDays(1) -Description 'ReverbArise disposable test'
    $report.sid=$created.SID.Value
    $users=(Get-LocalGroup -SID 'S-1-5-32-545').Name
    Add-LocalGroupMember -Group $users -Member $created
    $acl=[Security.AccessControl.DirectorySecurity]::new();$acl.SetAccessRuleProtection($true,$false)
    $owner=[Security.Principal.WindowsIdentity]::GetCurrent().User
    foreach($sid in @($owner,[Security.Principal.SecurityIdentifier]::new('S-1-5-18'),[Security.Principal.SecurityIdentifier]::new('S-1-5-32-544'))){
        $acl.AddAccessRule([Security.AccessControl.FileSystemAccessRule]::new($sid,'FullControl','ContainerInherit,ObjectInherit','None','Allow'))
    }
    $acl.SetOwner($owner)
    $acl.AddAccessRule([Security.AccessControl.FileSystemAccessRule]::new($created.SID,'Modify','ContainerInherit,ObjectInherit','None','Allow'))
    Set-Acl -LiteralPath $bridge -AclObject $acl
    $credential=[PSCredential]::new(($env:COMPUTERNAME+'\'+$testName),$password)
    $child=Start-Process -FilePath $binary -WorkingDirectory $bridge -ArgumentList @('--fresh-profile',('"'+$bridge+'"'),$testName) -Credential $credential -LoadUserProfile -WindowStyle Hidden -RedirectStandardOutput (Join-Path $bridge 'profile.stdout.log') -RedirectStandardError (Join-Path $bridge 'profile.stderr.log') -PassThru
    $null=$child.Handle
    if(!$child.WaitForExit(120000)){Stop-Process -Id $child.Id;throw 'Owned profile-test child exceeded its two-minute limit'}
    $child.Refresh();$report.testExit=$child.ExitCode
    if($child.ExitCode -ne 0){throw ('Fresh-profile test failed: '+(Get-Content -LiteralPath (Join-Path $bridge 'profile.stderr.log') -Raw))}
    $profileResult=Get-Content -LiteralPath (Join-Path $bridge 'profile-results.json') -Raw|ConvertFrom-Json
    if($profileResult.sid -ne $created.SID.Value){throw 'Child token does not match the disposable test account'}
    $report.profile=$profileResult.profile
} catch {$report.error=$_.Exception.Message}
finally {
    if($created){
        try {
            $profile=Get-CimInstance Win32_UserProfile -Filter ("SID='"+$created.SID.Value+"'")
            if($profile){
                $profilesRoot=[Environment]::ExpandEnvironmentVariables((Get-ItemProperty 'HKLM:/SOFTWARE/Microsoft/Windows NT/CurrentVersion/ProfileList').ProfilesDirectory)
                $expected=[IO.Path]::GetFullPath((Join-Path $profilesRoot $testName))
                $actual=[IO.Path]::GetFullPath($profile.LocalPath)
                if($actual -ne $expected -or !$actual.StartsWith([IO.Path]::GetFullPath($profilesRoot).TrimEnd('\')+'\',[StringComparison]::OrdinalIgnoreCase)){throw 'Unexpected disposable profile path; retained for explicit inspection'}
                if($profile.Loaded){throw 'Disposable profile is still loaded; retained for explicit inspection'}
                # Only the newly created SID at the verified exact test profile path is removed.
                $profile|Remove-CimInstance
            }
            $report.profileRemoved=$true
        } catch {$report.cleanupError=$_.Exception.Message}
        try {Remove-LocalUser -SID $created.SID;$report.accountRemoved=$true} catch {$report.accountCleanupError=$_.Exception.Message}
    }
    $report|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $bridge 'launcher-results.json') -Encoding UTF8
    $report|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $repo 'out/recovery-profile-launcher-results.json') -Encoding UTF8
}
$report|ConvertTo-Json -Depth 6
if($report.error -or $report.cleanupError -or $report.accountCleanupError -or $report.testExit -ne 0){exit 1}
exit 0
