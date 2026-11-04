#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace m3::ui
{
namespace spacing
{
inline constexpr int tight=4,labelHelp=6,related=12,group=16,section=24;
}
namespace controlGeometry
{
inline constexpr int height=34,labelHeight=24,headingHeight=30,helperLineHeight=18;
inline constexpr int valueWidth=74,valueHeight=28,rulerLabelWidth=40;
inline constexpr int numericStepWidth=18,feedbackHeight=20;
inline constexpr float focusStroke=2,focusOutset=3,focusPadding=6;
inline constexpr int switchTrackWidth=44,switchTrackHeight=24;
inline constexpr int switchWidth=switchTrackWidth+static_cast<int>(2*focusPadding);
inline constexpr int switchHeight=switchTrackHeight+static_cast<int>(2*focusPadding);
inline juce::Rectangle<float> insideStroke(juce::Rectangle<int> bounds,float stroke)
{return bounds.toFloat().reduced(stroke*.5f);}
}
namespace workspaceGeometry
{
inline constexpr int headerHeight=82,compactInset=16,largeInset=20;
inline constexpr int learnSettingsWidth=304,columnGap=28;
inline int outerInset(int width) noexcept{return width<1200?compactInset:largeInset;}
}
namespace learnTypography
{
// Workspace-only roles; the approved header and denser Perform stay unchanged.
inline constexpr float secondarySize=14;
inline constexpr int secondaryLineHeight=20;
inline const juce::Colour descriptionColour{0xffc4ccd4};
}
namespace learnGeometry
{
inline constexpr int metadataGap=6,rulerViewportGap=10;
inline constexpr int emptyIconHeight=38,emptyHeadingHeight=32,emptyTextHeight=40;
inline constexpr int emptyIconGap=12,emptyTextGap=6;
}
}
