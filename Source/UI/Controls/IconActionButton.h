#pragma once
#include <UI/WorkspaceTheme.h>

namespace m3::ui::controls
{
enum class ActionIcon { settings };

class IconActionButton final:public juce::Button
{
public:
    explicit IconActionButton(ActionIcon role):juce::Button("Settings"),icon(role)
    {
        setTitle("Settings");setTooltip("Open Settings");setHelpText("Open Settings");
        setWantsKeyboardFocus(true);setMouseClickGrabsKeyboardFocus(true);
    }
    // Space and Return have the same activation semantics. No mode toggling.
    bool keyPressed(const juce::KeyPress& key)override
    {
        if(key.isKeyCode(juce::KeyPress::returnKey) || key.isKeyCode(juce::KeyPress::spaceKey))
        {if(isEnabled())triggerClick();return true;}
        return juce::Button::keyPressed(key);
    }
    void focusGained(FocusChangeType)override{repaint();}
    void focusLost(FocusChangeType)override{repaint();}
    void paintButton(juce::Graphics& g,bool over,bool down)override
    {
        const auto box=getLocalBounds().toFloat().reduced(.5f);
        const auto enabled=isEnabled();const auto ink=(over || down?aqua:m3::ui::text).withMultipliedAlpha(enabled?1.0f:.35f);
        g.setColour(background);g.fillRoundedRectangle(box,radius);
        g.setColour((hasKeyboardFocus(true)?aqua:outline).withMultipliedAlpha(enabled?1.0f:.35f));
        g.drawRoundedRectangle(box,radius,hasKeyboardFocus(true)?2.0f:rule);
        const auto c=box.getCentre();g.setColour(ink);
        switch(icon)
        {
            case ActionIcon::settings:
                g.drawEllipse(juce::Rectangle<float>(14,14).withCentre(c),1.5f);
                g.drawEllipse(juce::Rectangle<float>(5,5).withCentre(c),1.5f);
                for(int n=0;n<8;++n){const auto angle=juce::MathConstants<float>::twoPi*n/8;
                    g.drawLine(c.x+std::cos(angle)*7,c.y+std::sin(angle)*7,c.x+std::cos(angle)*10,c.y+std::sin(angle)*10,2);}
                break;
        }
    }
private:
    ActionIcon icon;
};
}
