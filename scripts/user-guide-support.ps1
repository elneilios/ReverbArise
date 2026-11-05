# Guide authoring is explicit. Import and currency checks are read-only.
function Get-UserGuideInputs([string] $Root) {
    $rootPath = [IO.Path]::GetFullPath($Root).TrimEnd('\','/')
    $paths = @()
    foreach ($directory in @('Source','openspec/specs','cmake')) {
        $folder = Join-Path $rootPath $directory
        if (-not (Test-Path -LiteralPath $folder)) {
            if ($directory -ne 'cmake') { throw "Guide review input directory missing: $directory" }
            continue
        }
        $items = @(Get-Item -LiteralPath $folder) + @(Get-ChildItem -LiteralPath $folder -Recurse -Force)
        if (@($items | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint }).Count) { throw "Reparse point in guide inputs: $directory" }
        $paths += @($items | Where-Object { -not $_.PSIsContainer } | ForEach-Object FullName)
    }
    foreach ($relative in @('CMakeLists.txt','CMakePresets.json','docs/development/alpha-guide.html',
        'docs/development/releases.md','docs/development/product-version.md','docs/development/fl-studio-test-install.md','docs/development/user-guide-maintenance.md',
        'scripts/build.ps1','scripts/build-support.ps1','scripts/release.ps1','scripts/release-support.ps1','scripts/release-guide.ps1','scripts/verify-version.cmake',
        'scripts/install-test-vst3.ps1','scripts/test-install-support.ps1','scripts/check-user-guide.ps1','scripts/review-user-guide.ps1','scripts/user-guide-support.ps1',
        'tests/learn_generation_ui_test.cpp','tests/LearnGenerationSupport.h','resources/logo/png/wordmark/reverbarise-wordmark-dark-640px.png',
        '.agents/skills/maintain-user-guide/SKILL.md')) {
        $path = Join-Path $rootPath $relative
        if (Test-Path -LiteralPath $path) { $paths += $path }
        elseif ($relative -eq 'docs/development/alpha-guide.html' -or $relative -eq 'CMakeLists.txt') { throw "Guide input missing: $relative" }
    }
    $unique = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($path in $paths) { $null=$unique.Add($path) }
    [string[]]$sortedPaths=@($unique)
    [Array]::Sort($sortedPaths,[StringComparer]::Ordinal)
    foreach ($path in $sortedPaths) {
        if ((Get-Item -LiteralPath $path).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Reparse point in guide input: $path" }
        # Text hashes survive Git's normal LF/CRLF checkout conversion; PNG bytes are exact.
        $bytes = if ([IO.Path]::GetExtension($path) -ieq '.png') { [IO.File]::ReadAllBytes($path) } else {
            [Text.Encoding]::UTF8.GetBytes([IO.File]::ReadAllText($path).Replace("`r`n","`n").Replace("`r","`n"))
        }
        if ($null -eq $bytes) { $bytes = @() }
        $hasher = [Security.Cryptography.SHA256]::Create()
        try { $hash = ([BitConverter]::ToString($hasher.ComputeHash([byte[]]$bytes))).Replace('-','').ToLowerInvariant() }
        finally { $hasher.Dispose() }
        [ordered]@{ path=$path.Substring($rootPath.Length+1).Replace('\','/'); bytes=$bytes.Length; sha256=$hash }
    }
}

function Assert-UserGuideCurrent([string] $Root) {
    $reviewPath = Join-Path $Root 'docs/development/user-guide-review.json'
    if (-not (Test-Path -LiteralPath $reviewPath)) { throw 'Guide review missing. Run maintain-user-guide before packaging.' }
    $review = Get-Content -LiteralPath $reviewPath -Raw -Encoding UTF8 | ConvertFrom-Json
    if ($review.schemaVersion -ne 1 -or $review.normalization -cne 'utf8-lf-v1' -or [string]::IsNullOrWhiteSpace($review.summary)) { throw 'Invalid guide review record; run maintain-user-guide.' }
    $current = @(Get-UserGuideInputs $Root)
    if (($current | ConvertTo-Json -Depth 5 -Compress) -cne ($review.files | ConvertTo-Json -Depth 5 -Compress)) {
        $expected = @{}; foreach ($file in $review.files) { $expected[$file.path] = $file.sha256 }
        $changed = @(); foreach ($file in $current) { if ($expected[$file.path] -cne $file.sha256) { $changed += $file.path }; $expected.Remove($file.path) }
        $changed += @($expected.Keys)
        throw "Guide review stale: $(($changed | Sort-Object | Select-Object -First 8) -join ', '). Run maintain-user-guide; packaging never rewrites documentation."
    }
}
