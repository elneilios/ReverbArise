#pragma once
#include <UI/WorkspaceTheme.h>
#include "InvocationToggle.h"
namespace m3::ui::controls
{
class Switch final:public InvocationToggle
{
public:
    explicit Switch(const juce::String& label):InvocationToggle(label)
    {setWantsKeyboardFocus(true);setMouseClickGrabsKeyboardFocus(true);setTooltip(label);}
    juce::Rectangle<float> trackBounds()const noexcept
    {
        const auto available=getLocalBounds().toFloat().reduced(controlGeometry::focusPadding);
        return juce::Rectangle<float>(std::min(static_cast<float>(controlGeometry::switchTrackWidth),std::max(0.0f,available.getWidth())),
            std::min(static_cast<float>(controlGeometry::switchTrackHeight),std::max(0.0f,available.getHeight()))).withCentre(getLocalBounds().toFloat().getCentre());
    }
    juce::Rectangle<float> focusInkBounds()const noexcept
    {return trackBounds().expanded(controlGeometry::focusOutset+controlGeometry::focusStroke*.5f);}
    void paintButton(juce::Graphics& g,bool over,bool down)override
    {
        const auto box=trackBounds();if(box.isEmpty())return;
        const auto alpha=isEnabled()?1.0f:.4f;const auto on=getToggleState();
        const auto radius=box.getHeight()*.5f,thumb=std::max(0.0f,box.getHeight()-6);
        g.setColour((on?aqua:outline).withAlpha(alpha*(over || down?1.0f:.8f)));g.fillRoundedRectangle(box,radius);
        g.setColour((on?background:m3::ui::text).withAlpha(alpha));g.fillEllipse(juce::Rectangle<float>(thumb,thumb).withCentre({on?box.getRight()-radius:box.getX()+radius,box.getCentreY()}));
        if(hasKeyboardFocus(true)){g.setColour(aqua);g.drawRoundedRectangle(box.expanded(controlGeometry::focusOutset),radius+controlGeometry::focusOutset,controlGeometry::focusStroke);}
    }
};
}
