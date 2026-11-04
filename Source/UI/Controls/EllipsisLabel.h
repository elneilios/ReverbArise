#pragma once
#include <UI/WorkspaceTheme.h>
namespace m3::ui::controls
{
// A single production line retains its font metrics when content overflows.
class EllipsisLabel final:public juce::Label
{
public:void paint(juce::Graphics& g)override
    {g.setFont(getLookAndFeel().getLabelFont(*this));g.setColour(findColour(juce::Label::textColourId).withMultipliedAlpha(isEnabled()?1.f:.35f));
        g.drawText(getText(),getBorderSize().subtractedFrom(getLocalBounds()),getJustificationType(),true);}
};
}
