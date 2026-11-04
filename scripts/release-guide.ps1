# Loaded by release-support.ps1. Capture uses the existing production-editor evidence test.
function Get-ReleaseGuideStates { @('alpha-empty-learn', 'alpha-captured-markers', 'alpha-ready-perform', 'alpha-ready-export') }

function Invoke-ReleaseGuideCapture([string] $Binary, [string] $Directory, [string] $Sha) {
    $executable = Join-Path $Binary 'Release/preverb_learn_generation_ui_test.exe'
    $previous = $env:PREVERB_UI_M3_OUTPUT
    try {
        $env:PREVERB_UI_M3_OUTPUT = $Directory
        $null = Invoke-ReleaseNative $executable @('--guide-only')
    } finally { $env:PREVERB_UI_M3_OUTPUT = $previous }
    [ordered]@{
        gitSha = $Sha; command = 'Release/preverb_learn_generation_ui_test.exe --guide-only'
        executableSha256 = (Get-FileHash -LiteralPath $executable -Algorithm SHA256).Hash.ToLowerInvariant()
        fixture = 'Deterministic captured audio; ordinary production editor; readiness assertions passed'
    } | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $Directory 'capture.json') -Encoding UTF8
}

function Write-ReleaseGuide([string] $Root, [string] $Directory, [string] $Payload, [string] $Sha, [string] $Identity) {
    $receipt = Get-Content -LiteralPath (Join-Path $Directory 'capture.json') -Raw | ConvertFrom-Json
    if ($receipt.gitSha -cne $Sha -or $receipt.command -cne 'Release/preverb_learn_generation_ui_test.exe --guide-only' -or $receipt.executableSha256 -notmatch '^[a-f0-9]{64}$') { throw 'Guide capture provenance mismatch.' }
    $html = Get-Content -LiteralPath (Join-Path $Root 'docs/development/alpha-guide.html') -Raw -Encoding UTF8
    $html = $html.Replace('{{IDENTITY}}', [Net.WebUtility]::HtmlEncode($Identity)).Replace('{{SHA}}', $Sha)
    Add-Type -AssemblyName System.Drawing
    $images = @()
    foreach ($state in (Get-ReleaseGuideStates)) {
        $path = Join-Path $Directory ($state + '.png')
        $bytes = [IO.File]::ReadAllBytes($path)
        $stream = New-Object IO.MemoryStream(,$bytes)
        $image = $null
        try {
            $image = [Drawing.Image]::FromStream($stream)
            if ($image.RawFormat.Guid -ne [Drawing.Imaging.ImageFormat]::Png.Guid -or $image.Width -ne 1400 -or $image.Height -ne 1000) { throw "Invalid guide PNG: $state" }
            $images += [ordered]@{ state = $state; width = $image.Width; height = $image.Height; sha256 = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() }
        } finally { if ($image) { $image.Dispose() }; $stream.Dispose() }
        if (-not $html.Contains('{{' + $state + '}}')) { throw "Missing guide image slot: $state" }
        $html = $html.Replace('{{' + $state + '}}', 'data:image/png;base64,' + [Convert]::ToBase64String($bytes))
    }
    # The committed template is static. Fail closed on active content or resource references.
    if ($html -match '{{|https?://|<\s*(script|iframe|object|embed|link|base|form|meta\s+http-equiv)\b|\bon\w+\s*=|@import|url\s*\(|\b(srcset|background|poster)\s*=' ) { throw 'Guide contains unresolved slots, active content or external resources.' }
    foreach ($reference in [regex]::Matches($html, '(?i)\b(src|href)\s*=\s*(["''])(.*?)\2')) {
        $value = $reference.Groups[3].Value
        if (($reference.Groups[1].Value -ieq 'src' -and $value -notmatch '^data:image/png;base64,[A-Za-z0-9+/=]+$') -or ($reference.Groups[1].Value -ieq 'href' -and $value -notmatch '^#[a-z0-9-]+$')) { throw 'Guide resource must be embedded or an internal anchor.' }
    }
    if ([regex]::Matches($html, '<img\s').Count -ne 4) { throw 'Guide requires four embedded images.' }
    $html | Set-Content -LiteralPath (Join-Path $Payload 'User-Guide.html') -Encoding UTF8
    [ordered]@{ gitSha = $Sha; releaseIdentity = $Identity; capture = $receipt; images = $images } | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $Payload 'guide-provenance.json') -Encoding UTF8
}
