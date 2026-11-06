#pragma once
#include "ActionButton.h"

namespace m3::ui::controls
{
// The owner displays accepted effective state (for example a temporary bypass).
// This family has no preference storage, modifier polling or model mutation.
class LatchingActionButton final:public ActionButton
{
public:
    explicit LatchingActionButton(const juce::String& label):ActionButton(label)
    {setClickingTogglesState(true);}
    void display(bool effective){setToggleState(effective,juce::dontSendNotification);}
    void paintButton(juce::Graphics& g,bool over,bool down)override
    {
        if(!getToggleState()){ActionButton::paintButton(g,over,down);return;}
        const auto alpha=isEnabled()?1.f:.35f;
        const auto box=controlGeometry::insideStroke(getLocalBounds(),rule);
        g.setColour((over || down?hover:aqua).withMultipliedAlpha(alpha));g.fillRoundedRectangle(box,radius);
        g.setColour((hasKeyboardFocus(true) || over || down?aqua:outline).withMultipliedAlpha(alpha));
        g.drawRoundedRectangle(hasKeyboardFocus(true)?focusInkBounds():box,radius,
                               hasKeyboardFocus(true)?controlGeometry::focusStroke:rule);
        g.setColour(background.withMultipliedAlpha(alpha));g.setFont(componentTextFont(*this,TextRole::controlLabel));
        g.drawText(getButtonText(),getLocalBounds().reduced(spacing::related,spacing::tight),juce::Justification::centred);
    }
};
}
