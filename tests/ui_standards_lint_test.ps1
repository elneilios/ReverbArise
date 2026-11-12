$ErrorActionPreference = 'Stop'
$guard = Join-Path (Split-Path $PSScriptRoot -Parent) 'scripts/check-ui-standards.ps1'
$fixture = Join-Path (Split-Path $PSScriptRoot -Parent) ('out/ui standards fixtures/'+[guid]::NewGuid().ToString('N'))
$null = New-Item -ItemType Directory -Path (Join-Path $fixture 'Source/UI') -Force
$sourcePath = Join-Path $fixture 'Source/UI/Example.h'
$baselinePath = Join-Path $fixture 'baseline.json'
$script:assertions = 0
function Check([string] $source, [int] $expected, [string] $diagnostic, [object[]] $entries = @()) {
    [IO.File]::WriteAllText($sourcePath,$source)
    @{schema=1; entries=@($entries)} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $baselinePath -Encoding UTF8
    $output = & $guard -Root $fixture -BaselinePath $baselinePath
    $code = $LASTEXITCODE
    if ($code -ne $expected -or ($output -join "`n") -notmatch [regex]::Escape($diagnostic)) {
        throw "Expected $expected / $diagnostic; got $code : $output"
    }
    $script:assertions++
}
Check 'auto bounds = getLocalBounds().reduced(7); x += samples * 12; auto ink = aqua; auto f = interFont(bodySize);' 0 'UI standards passed'
Check "// juce::Font(14); 0xff123456`n/* setScrollWheelEnabled(false);`n0xffaabbcc */ auto x=4;" 0 'UI standards passed'
Check 'auto url = "https://example.com"; auto ink = juce::Colour(0xff123456);' 1 'raw-colour'
Check 'auto ink = juce::Colour{12, 20, 30};' 1 'raw-colour'
Check 'auto ink = Colour("#112233");' 1 'raw-colour'
Check 'auto ink = juce::Colour::fromRGB(12,20,30);' 1 'raw-colour'
Check 'auto ink = juce::Colour::fromFloatRGBA(.1f,.2f,.3f,1.f);' 1 'raw-colour'
Check 'auto f = juce::Font(14.0f);' 1 'font-construction'
Check 'juce::Font f;' 1 'font-construction'
Check 'juce::Font f(14.0f);' 1 'font-construction'
Check 'auto options = juce::FontOptions("Arial",14,0);' 1 'font-construction'
Check 'juce::FontOptions options("Arial",14,0);' 1 'font-construction'
Check 'juce::FontOptions options;' 1 'font-construction'
Check 'auto f = interFont(14);' 1 'literal-font'
Check 'auto f = fontFor(*this, 14.f);' 1 'literal-font'
Check 'label.setFont(14.0f);' 1 'literal-font'
Check 'auto f = textFont(TextRole::helper).withHeight(12);' 1 'literal-font'
Check 'auto f = interFont(bodySize).withHeight(12);' 1 'literal-font'
Check 'auto bounds = rect.withHeight(32).withTrimmedLeft(10);' 0 'UI standards passed'
Check 'control.setScrollWheelEnabled(false);' 1 'interaction-default'
Check 'control.setMouseDragSensitivity(150);' 1 'interaction-default'
Check 'control.setVelocityBasedMode(true);' 1 'interaction-default'
Check 'label.setColour(id, juce::Colours::white);' 1 'literal-style'
Check 'label.setMinimumHorizontalScale(1.f);' 1 'literal-style'
Check "auto x = 1;`nauto ink = 0xff123456;" 1 'Source/UI/Example.h:2: raw-colour'
$entry = @{id='fixture-1';path='Source/UI/Example.h';rule='interaction-default';occurrence='setScrollWheelEnabled(false)';count=1;reason='Preserved fixture';review='fixture approval';scope='fixture only'}
Check 'control.setScrollWheelEnabled(false);' 0 'UI standards passed' @($entry)
Check 'control.setScrollWheelEnabled(false); other.setScrollWheelEnabled(false);' 1 'interaction-default' @($entry)
Check 'auto x=1;' 1 'stale-exception' @($entry)
Check 'control.setScrollWheelEnabled(true);' 1 'interaction-default' @($entry)
Check 'control.setScrollWheelEnabled(false);' 1 'Duplicate UI baseline key' @($entry,$entry)
$bad = $entry.Clone(); $bad.Remove('review')
Check 'control.setScrollWheelEnabled(false);' 1 'Incomplete UI exception' @($bad)
# Function return declarations and comments are not font construction.
Check 'juce::Font fontFor(Component& c, float height) { return interFont(height); }' 0 'UI standards passed'
Check 'juce::Font getLabelFont(juce::Label& label) { return interFont(controlSize); }' 0 'UI standards passed'
Check "auto delay = 500;`njuce::TooltipWindow popupHost{nullptr, delay};" 1 'Source/UI/Example.h:2: popup-tooltip-host'
$footerMetadata = 'juce::TooltipClient* helpSource; juce::SettableTooltipClient help; help.setTooltip("Footer help"); control.getProperties().set("contextHelp", "Footer help");'
Check $footerMetadata 0 'UI standards passed'
Check '// juce::TooltipWindow popupHost;' 0 'UI standards passed'
Check 'auto typeName = "TooltipWindow";' 0 'UI standards passed'
$sourcePath = Join-Path $fixture 'Source/UI/UiGeometry.h'
Check 'inline constexpr int section = 24;' 1 'token-definition'
Check 'auto x = samples * 24; bounds.removeFromTop(12);' 0 'UI standards passed'
$null = New-Item -ItemType Directory -Path (Join-Path $fixture 'Source/Plugin') -Force
$sourcePath = Join-Path $fixture 'Source/Plugin/Editor.cpp'
Check 'juce::TooltipWindow popupHost;' 1 'Source/Plugin/Editor.cpp:1: popup-tooltip-host'
Check $footerMetadata 0 'UI standards passed'
$null = New-Item -ItemType Directory -Path (Join-Path $fixture 'tests/ui') -Force
$sourcePath = Join-Path $fixture 'tests/ui/audition_loop_component_test.cpp'
Check 'void runInteractiveReview() { using namespace juce; TooltipWindow popupHost; }' 1 'tests/ui/audition_loop_component_test.cpp:1: popup-tooltip-host'
Check "$footerMetadata auto testInk = 0xff123456;" 0 'UI standards passed'
Write-Output "UI lint command fixtures passed: $script:assertions assertions (PowerShell $($PSVersionTable.PSVersion))."
