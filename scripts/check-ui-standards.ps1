[CmdletBinding()]
param(
    [string] $Root = '',
    [string] $BaselinePath = '',
    [switch] $ListFindings
)

# A targeted source guard, not a C++ parser. Keep rule fixtures with rule changes.
$ErrorActionPreference = 'Stop'
try {
    if (!$Root) { $Root = Split-Path $PSScriptRoot -Parent }
    if (!$BaselinePath) { $BaselinePath = Join-Path $PSScriptRoot 'ui-standards-baseline.json' }
    $Root = (Resolve-Path -LiteralPath $Root).Path
    $rules = @(
        @{ id='token-definition'; path='Source/UI/(WorkspaceTheme|UiGeometry)\.h$'; pattern='\binline\s+constexpr\s+(?:int|float)\s+[^;]+;'; remedy='Changed/new token definitions require owner review and a standards revision.' },
        @{ id='raw-colour'; pattern='(?i)\b0x[0-9a-f]{6}(?:[0-9a-f]{2})?\b|\b(?:juce::)?Colour\s*(?:\(|\{)\s*(?:[0-9]+\s*,|"#[0-9a-f]+")|\b(?:juce::)?Colour::(?:fromRGB|fromRGBA|fromFloatRGBA|fromString)\s*\([^;]*?\)'; remedy='Use a named semantic colour in WorkspaceTheme/UiGeometry.' },
        @{ id='font-construction'; pattern='\bjuce::Font(?:Options)?\s*[({]|\bjuce::Font(?:Options)?\s+\w+\s*(?:;|[({]\s*[0-9"])|\b(?:getDefaultSansSerifFontName|setDefaultSansSerifTypefaceName|setDefaultLookAndFeel)\s*\('; remedy='Use embedded text roles; font construction belongs to the shared theme policy.' },
        @{ id='literal-font'; pattern='\b(?:interFont|fontFor)\s*\(\s*(?:\*this\s*,\s*)?[0-9]+(?:\.[0-9]*)?f?\b|\bsetFont\s*\(\s*[0-9]+(?:\.[0-9]*)?f?\b|\b(?:interFont|fontFor|textFont)\s*\([^;]*?\)\s*\.withHeight\s*\(\s*[0-9]+(?:\.[0-9]*)?f?\b'; remedy='Use a named typography role or approved shared size.' },
        @{ id='interaction-default'; pattern='\b(?:setScrollWheelEnabled|setMouseDragSensitivity|setVelocityBasedMode|setVelocityModeParameters)\s*\([^;]*?\)'; remedy='Reuse the owning control family or named interaction policy.' },
        @{ id='literal-style'; pattern='\b(?:setColour|setColor)\s*\([^;]*?\b(?:juce::Colours::\w+|Colours::\w+)|\b(?:setMinimumHorizontalScale|setAlpha)\s*\(\s*(?:[0-9]+(?:\.[0-9]*)?f?|\.[0-9]+f?)\s*\)'; remedy='Use a named presentation policy or a reviewed scoped exception.' }
    )
    $paths = @()
    foreach ($dir in @('Source/UI','Source/Plugin')) {
        $path = Join-Path $Root $dir
        if (Test-Path -LiteralPath $path) {
            $paths += @(Get-ChildItem -LiteralPath $path -Recurse -File | Where-Object { $_.Extension -in @('.h','.cpp') })
        }
    }
    $findings = @(
        foreach ($file in ($paths | Sort-Object FullName)) {
            $relative = $file.FullName.Substring($Root.Length).TrimStart('\','/').Replace('\','/')
            $source = [IO.File]::ReadAllText($file.FullName)
            # Match quoted strings first, so a URL or // inside text is not a comment.
            # Preserve newlines/offsets for useful diagnostics, including block comments.
            $source = [regex]::Replace($source, '(?s)"(?:\\.|[^"\\])*"|''(?:\\.|[^''\\])*''|//[^\r\n]*|/\*.*?\*/', {
                param($m)
                if ($m.Value.StartsWith('//') -or $m.Value.StartsWith('/*')) { return [regex]::Replace($m.Value,'[^\r\n]',' ') }
                return $m.Value
            })
            foreach ($rule in $rules) {
                if ($rule.path -and $relative -notmatch $rule.path) { continue }
                foreach ($match in [regex]::Matches($source,$rule.pattern)) {
                    [pscustomobject]@{
                        path=$relative; line=1+([regex]::Matches($source.Substring(0,$match.Index),"`n")).Count
                        rule=$rule.id; occurrence=([regex]::Replace($match.Value,'\s+','')).Trim()
                        remedy=$rule.remedy
                    }
                }
            }
        }
    )
    if ($ListFindings) { ConvertTo-Json -InputObject @($findings) -Depth 5; exit 0 }
    $baseline = Get-Content -LiteralPath $BaselinePath -Raw | ConvertFrom-Json
    if ($baseline.schema -ne 1) { throw 'Unsupported UI baseline schema.' }
    $allowed = @{}
    foreach ($entry in $baseline.entries) {
        if (!$entry.id -or !$entry.reason -or !$entry.review -or !$entry.scope -or $entry.count -lt 1) { throw 'Incomplete UI exception: id, reason, review, scope and positive count are required.' }
        $key = $entry.path+'|'+$entry.rule+'|'+$entry.occurrence
        if ($allowed.ContainsKey($key)) { throw "Duplicate UI baseline key: $key" }
        $allowed[$key] = $entry
    }
    $seen = @{}; $failures = 0
    foreach ($finding in $findings) {
        $key = $finding.path+'|'+$finding.rule+'|'+$finding.occurrence
        if (!$seen.ContainsKey($key)) { $seen[$key] = 0 }; $seen[$key]++
        if (!$allowed.ContainsKey($key) -or $seen[$key] -gt $allowed[$key].count) {
            Write-Output ('{0}:{1}: {2}: {3} {4}' -f $finding.path,$finding.line,$finding.rule,$finding.occurrence,$finding.remedy)
            $failures++
        }
    }
    foreach ($key in $allowed.Keys) {
        if (!$seen.ContainsKey($key) -or $seen[$key] -lt $allowed[$key].count) {
            Write-Output ('{0}:1: stale-exception: {1}; remove or review the changed occurrence.' -f $allowed[$key].path,$allowed[$key].id)
            $failures++
        }
    }
    if ($paths.Count -eq 0) { throw 'No governed UI sources found; refusing an empty scan.' }
    if ($failures) { Write-Output "UI standards: $failures violation(s)."; exit 1 }
    Write-Output "UI standards passed: $($paths.Count) files; $($findings.Count) explicitly bounded definition/debt occurrences."
    exit 0
} catch {
    Write-Output "UI standards error: $($_.Exception.Message)"
    exit 1
}
