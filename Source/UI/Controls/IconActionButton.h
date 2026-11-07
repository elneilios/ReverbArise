#pragma once
#include "ActionButton.h"

namespace m3::ui::controls
{
enum class ActionIcon { settings, openFile };

class IconActionButton final:public CommandButton
{
public:
    explicit IconActionButton(ActionIcon role):CommandButton(role==ActionIcon::settings?"Settings":"Open IR file",true),icon(role)
    {
        setTitle(getButtonText());setTooltip(role==ActionIcon::settings?"Open Settings":"Import WAV or WavPack IR");setHelpText(getTooltip());
        setWantsKeyboardFocus(true);setMouseClickGrabsKeyboardFocus(true);
    }
    juce::Rectangle<float> outlineBounds(bool focused)const noexcept
    {return controlGeometry::insideStroke(getLocalBounds(),focused?controlGeometry::focusStroke:rule);}
    void paintButton(juce::Graphics& g,bool over,bool down)override
    {
        ActionButton::paintSurface(g,*this,over,down);
        const auto c=getLocalBounds().toFloat().getCentre();
        g.setColour(commandInk(CommandVariant::neutral,{isEnabled(),over,down,isFocusVisible(),false}).glyph);
        switch(icon)
        {
            case ActionIcon::openFile:
            {
                juce::Path folder;folder.startNewSubPath(c.x-9,c.y+7);folder.lineTo(c.x-9,c.y-6);
                folder.lineTo(c.x-2,c.y-6);folder.lineTo(c.x+1,c.y-3);folder.lineTo(c.x+9,c.y-3);
                folder.lineTo(c.x+9,c.y+7);folder.closeSubPath();g.strokePath(folder,juce::PathStrokeType(1.5f));break;
            }
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
