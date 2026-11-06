#pragma once
#include <UI/WorkspaceTheme.h>
namespace m3::ui::controls
{
enum class CommandIcon { none, play, stop, save, returnBack };
class ActionButton:public juce::Button
{
public:
    ActionButton(const juce::String& label,CommandIcon role=CommandIcon::none,bool primary=false)
        :juce::Button(label),icon(role),emphasized(primary)
    {setWantsKeyboardFocus(true);setMouseClickGrabsKeyboardFocus(true);setTooltip(label);setTitle(label);}
    std::function<void()> beforeInvocation;
    void mouseDown(const juce::MouseEvent& e)override
    {if(isEnabled() && beforeInvocation)beforeInvocation();juce::Button::mouseDown(e);}
    bool keyPressed(const juce::KeyPress& key)override
    {
        if(key.isKeyCode(juce::KeyPress::spaceKey) || key.isKeyCode(juce::KeyPress::returnKey))
        {if(isEnabled()){if(beforeInvocation)beforeInvocation();triggerClick();}return true;}
        return juce::Button::keyPressed(key);
    }
    void focusGained(FocusChangeType)override{repaint();}
    void focusLost(FocusChangeType)override{repaint();}
    juce::Rectangle<float> focusInkBounds()const noexcept{return controlGeometry::insideStroke(getLocalBounds(),controlGeometry::focusStroke);}
    void paintButton(juce::Graphics& g,bool over,bool down)override
    {
        const auto alpha=isEnabled()?1.f:.35f;const auto box=controlGeometry::insideStroke(getLocalBounds(),rule);
        const auto active=over || down;g.setColour((emphasized?(active?hover:aqua):background).withMultipliedAlpha(alpha));g.fillRoundedRectangle(box,radius);
        g.setColour((hasKeyboardFocus(true) || active?aqua:outline).withMultipliedAlpha(alpha));
        g.drawRoundedRectangle(hasKeyboardFocus(true)?focusInkBounds():box,radius,hasKeyboardFocus(true)?controlGeometry::focusStroke:rule);
        g.setColour((emphasized?background:m3::ui::text).withMultipliedAlpha(alpha));
        auto area=getLocalBounds().toFloat().reduced(12,4);
        const bool iconOnly=icon==CommandIcon::play || icon==CommandIcon::stop;
        if(icon!=CommandIcon::none){const auto centre=iconOnly?box.getCentre():juce::Point<float>(area.getX()+10,area.getCentreY());drawIcon(g,icon,centre);if(!iconOnly)area.removeFromLeft(30);}
        if(!iconOnly){g.setFont(componentTextFont(*this,TextRole::controlLabel));g.drawText(getButtonText(),area,juce::Justification::centred);}
    }
    static void drawIcon(juce::Graphics& g,CommandIcon icon,juce::Point<float> c)
    {
        juce::Path p;
        if(icon==CommandIcon::play){p.addTriangle(c.x-5,c.y-8,c.x+8,c.y,c.x-5,c.y+8);g.fillPath(p);}
        else if(icon==CommandIcon::stop)g.fillRect(juce::Rectangle<float>(13,13).withCentre(c));
        else if(icon==CommandIcon::save){p.startNewSubPath(c.x,c.y-9);p.lineTo(c.x,c.y+3);p.startNewSubPath(c.x-5,c.y-2);p.lineTo(c.x,c.y+3);p.lineTo(c.x+5,c.y-2);
            p.startNewSubPath(c.x-9,c.y+2);p.lineTo(c.x-9,c.y+8);p.lineTo(c.x+9,c.y+8);p.lineTo(c.x+9,c.y+2);g.strokePath(p,juce::PathStrokeType(1.5f));}
        else if(icon==CommandIcon::returnBack){p.startNewSubPath(c.x-8,c.y-1);p.lineTo(c.x+3,c.y-1);p.cubicTo(c.x+12,c.y-1,c.x+12,c.y+8,c.x+2,c.y+8);
            p.startNewSubPath(c.x-2,c.y-7);p.lineTo(c.x-8,c.y-1);p.lineTo(c.x-2,c.y+5);g.strokePath(p,juce::PathStrokeType(1.5f));}
    }
private:
    CommandIcon icon;bool emphasized;
};
}
