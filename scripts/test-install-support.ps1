# Import is side-effect free. Uses inventory/hash helpers from release-support.ps1.
function Assert-TestHostClosed {
    $hosts = @(Get-Process -Name 'FL', 'FL64', 'FL64 (scaled)' -ErrorAction SilentlyContinue)
    if ($hosts.Count -gt 0) { throw 'Close FL Studio before installing. No host process was stopped.' }
}

function Assert-TestBundleMatch([string] $Source, [string] $Copy) {
    $expected = @(Get-ReleaseInventory $Source) | ConvertTo-Json -Depth 4 -Compress
    $actual = @(Get-ReleaseInventory $Copy) | ConvertTo-Json -Depth 4 -Compress
    if ($actual -cne $expected) { throw "Bundle verification failed: $Copy" }
}

function Move-TestInstallDirectory([string] $Source, [string] $Destination) {
    [IO.Directory]::Move($Source, $Destination)
}

function Install-TestBundle([string] $Source, [string] $Destination, [string] $BackupRoot) {
    $ErrorActionPreference = 'Stop'
    $destinationPath = [IO.Path]::GetFullPath($Destination)
    $backupPath = [IO.Path]::GetFullPath($BackupRoot)
    $sourcePath = [IO.Path]::GetFullPath($Source)
    $target = Join-Path $destinationPath 'ReverbArise.vst3'
    # Never move/delete the build source or put backups inside a scanned bundle.
    foreach ($path in @($sourcePath, $backupPath)) {
        if ($path.TrimEnd('\', '/') -ieq $target -or $path.StartsWith($target + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
            throw 'Source and backups must be outside the installed bundle.'
        }
    }
    if ($target.StartsWith($sourcePath.TrimEnd('\', '/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'Destination cannot be inside the source bundle.' }
    Assert-TestHostClosed
    $null = Get-ReleaseInventory $sourcePath
    $null = [IO.Directory]::CreateDirectory($destinationPath)
    $installLock = $null; $stage = $null; $backup = $null
    try {
        # Serializes installs from separate worktrees to this destination.
        $installLock = [IO.File]::Open((Join-Path $destinationPath '.reverbarise-install.lock'), 'OpenOrCreate', 'ReadWrite', 'None')
        $run = (Get-Date -Format 'yyyyMMdd-HHmmss') + '-' + [guid]::NewGuid().ToString('N')
        $stage = Join-Path $destinationPath ('.reverbarise-stage-' + $run)
        Copy-Item -LiteralPath $sourcePath -Destination $stage -Recurse
        Assert-TestBundleMatch $sourcePath $stage
        Assert-TestHostClosed
        if (Test-Path -LiteralPath $target) {
            $null = Get-ReleaseInventory $target
            $backupDirectory = Join-Path $backupPath $run
            $null = [IO.Directory]::CreateDirectory($backupDirectory)
            $backupCandidate = Join-Path $backupDirectory 'ReverbArise.vst3'
            # Directory.Move fails safely on loaded files or across volumes.
            Move-TestInstallDirectory $target $backupCandidate
            $backup = $backupCandidate
            Write-Host "Previous installation: $backup"
        }
        try {
            Move-TestInstallDirectory $stage $target
            $stage = $null
        } catch {
            if ($backup) {
                try { Move-TestInstallDirectory $backup $target }
                catch { throw "Install and rollback failed. Previous bundle is preserved at $backup. $($_.Exception.Message)" }
            }
            throw
        }
        Write-Host "Installed: $target"
        Write-Host "SHA256: $((Get-FileHash -LiteralPath (Join-Path $target 'Contents/x86_64-win/ReverbArise.vst3') -Algorithm SHA256).Hash)"
    } finally {
        # Only the unique staging directory owned by this invocation is removed.
        if ($stage -and (Test-Path -LiteralPath $stage)) {
            $resolvedStage = [IO.Path]::GetFullPath($stage)
            if ((Split-Path $resolvedStage -Parent) -ine $destinationPath.TrimEnd('\', '/') -or (Split-Path $resolvedStage -Leaf) -notlike '.reverbarise-stage-*') { throw 'Unsafe staging cleanup path.' }
            try { Remove-Item -LiteralPath $resolvedStage -Recurse -Force }
            finally { if ($installLock) { $installLock.Dispose(); $installLock = $null } }
        }
        if ($installLock) { $installLock.Dispose() }
    }
}
