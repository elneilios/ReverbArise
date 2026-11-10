#pragma once
#include <UI/WorkspaceTheme.h>
#include "FocusVisibility.h"
namespace m3::ui::controls
{
enum class CommandIcon { none, play, stop, save, returnBack, add };
// Shared command input, including accessibility paths that JUCE otherwise sends
// straight to triggerClick/toggle without capturing the invocation owner.
class CommandButton:public juce::Button
{
public:
    explicit CommandButton(const juce::String& label,bool glyphOnly=false):juce::Button(label),keepGlyphTitle(glyphOnly)
    {setWantsKeyboardFocus(true);setMouseClickGrabsKeyboardFocus(true);setTooltip(label);setTitle(label);}
    void setButtonText(const juce::String& label)
    {
        const bool follow=!keepGlyphTitle && getTitle()==getButtonText();
        juce::Button::setButtonText(label);if(follow)setTitle(label);
    }
    std::function<void()> beforeInvocation;
    bool isFocusVisible()const noexcept{return hasKeyboardFocus(true) && focusVisibility->isVisible();}
    virtual void invokeCommand()
    {
        if(!isEnabled())return;
        const juce::Component::SafePointer<CommandButton> safe(this);
        const auto prepare=beforeInvocation;if(prepare)prepare();
        if(safe && safe->isEnabled())safe->triggerClick();
    }
    void mouseDown(const juce::MouseEvent& e)override
    {
        if(!isEnabled())return;
        focusVisibility->pointerInput();
        const juce::Component::SafePointer<CommandButton> safe(this);
        const auto prepare=beforeInvocation;if(prepare)prepare();
        if(safe && safe->isEnabled())safe->juce::Button::mouseDown(e);
    }
    bool keyPressed(const juce::KeyPress& key)override
    {
        focusVisibility->keyboardInput();
        if(key.isKeyCode(juce::KeyPress::spaceKey) || key.isKeyCode(juce::KeyPress::returnKey))
        {invokeCommand();return true;}
        return juce::Button::keyPressed(key);
    }
    void focusGained(FocusChangeType cause)override
    {
        if(cause==focusChangedByMouseClick)focusVisibility->pointerInput();
        else if(cause==focusChangedByTabKey)focusVisibility->keyboardInput();
        repaint();
    }
    void focusLost(FocusChangeType)override{repaint();}
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override
    {
        class Handler final:public juce::AccessibilityHandler
        {
            class Value final:public juce::AccessibilityTextValueInterface
            {
            public:
                explicit Value(CommandButton& command):owner(command){}
                bool isReadOnly()const override{return true;}
                juce::String getCurrentValueAsString()const override{return owner.getToggleState()?"On":"Off";}
                void setValueAsString(const juce::String&)override{}
            private:CommandButton& owner;
            };
        public:
            explicit Handler(CommandButton& command):juce::AccessibilityHandler(command,juce::AccessibilityRole::button,
                actions(command),interfaces(command)),owner(command){}
            juce::AccessibleState getCurrentState()const override
            {
                auto state=juce::AccessibilityHandler::getCurrentState();
                if(owner.isToggleable()){state=state.withCheckable();if(owner.getToggleState())state=state.withChecked();}
                return state;
            }
            juce::String getTitle()const override{return owner.getTitle().isNotEmpty()?owner.getTitle():owner.getButtonText();}
            juce::String getHelp()const override{return owner.getTooltip();}
        private:
            static Interfaces interfaces(CommandButton& command)
            {return command.isToggleable()?Interfaces{std::make_unique<Value>(command)}:Interfaces{};}
            static juce::AccessibilityActions actions(CommandButton& command)
            {
                auto result=juce::AccessibilityActions{}
                    .addAction(juce::AccessibilityActionType::focus,[&command]{command.focusVisibility->accessibilityInput();command.grabKeyboardFocus();})
                    .addAction(juce::AccessibilityActionType::press,[&command]{command.focusVisibility->accessibilityInput();command.invokeCommand();});
                if(command.isToggleable())result=result.addAction(juce::AccessibilityActionType::toggle,[&command]{command.focusVisibility->accessibilityInput();command.invokeCommand();});
                return result;
            }
            CommandButton& owner;
        };
        return std::make_unique<Handler>(*this);
    }
private:
    bool keepGlyphTitle;
    juce::SharedResourcePointer<FocusVisibility> focusVisibility;
};
class ActionButton:public CommandButton
{
public:
    ActionButton(const juce::String& label,CommandIcon role=CommandIcon::none,bool primary=false)
        :ActionButton(label,role,primary?CommandVariant::primary:CommandVariant::neutral){}
    ActionButton(const juce::String& label,CommandIcon role,CommandVariant presentation)
        :CommandButton(label,role==CommandIcon::play || role==CommandIcon::stop || role==CommandIcon::add),icon(role),variant(presentation){}
    juce::Rectangle<float> focusInkBounds()const noexcept{return controlGeometry::insideStroke(getLocalBounds(),controlGeometry::focusStroke);}
    // Shared surface for text and glyph-only command buttons. Content colour
    // and focus are resolved independently from hover and held-press feedback.
    static void paintSurface(juce::Graphics& g,juce::Button& button,bool over,bool down,bool emphasized=false)
    {
        const auto* command=dynamic_cast<CommandButton*>(&button);
        paintCommandSurface(g,button.getLocalBounds(),emphasized?CommandVariant::primary:CommandVariant::neutral,
                            {button.isEnabled(),over,down,command?command->isFocusVisible():button.hasKeyboardFocus(true),false});
    }
    void paintButton(juce::Graphics& g,bool over,bool down)override
    {
        const CommandState state{isEnabled(),over,down,isFocusVisible(),effectiveLatchState()};
        paintCommandSurface(g,getLocalBounds(),variant,state);
        g.setColour(commandInk(variant,state).content);
        auto area=getLocalBounds().toFloat().reduced(12,4);
        const bool iconOnly=icon==CommandIcon::play || icon==CommandIcon::stop || icon==CommandIcon::add;
        if(icon!=CommandIcon::none){g.setColour(commandInk(variant,state).glyph);const auto centre=iconOnly?getLocalBounds().toFloat().getCentre():juce::Point<float>(area.getX()+10,area.getCentreY());drawIcon(g,icon,centre);if(!iconOnly)area.removeFromLeft(30);g.setColour(commandInk(variant,state).content);}
        if(!iconOnly){g.setFont(componentTextFont(*this,TextRole::controlLabel));g.drawText(getButtonText(),area,juce::Justification::centred);}
    }
    static void drawIcon(juce::Graphics& g,CommandIcon icon,juce::Point<float> c)
    {
        juce::Path p;
        if(icon==CommandIcon::play){p.addTriangle(c.x-5,c.y-8,c.x+8,c.y,c.x-5,c.y+8);g.fillPath(p);}
        else if(icon==CommandIcon::stop)g.fillRect(juce::Rectangle<float>(13,13).withCentre(c));
        else if(icon==CommandIcon::add){g.drawLine(c.x-5,c.y,c.x+5,c.y,rule);g.drawLine(c.x,c.y-5,c.x,c.y+5,rule);}
        else if(icon==CommandIcon::save){p.startNewSubPath(c.x,c.y-9);p.lineTo(c.x,c.y+3);p.startNewSubPath(c.x-5,c.y-2);p.lineTo(c.x,c.y+3);p.lineTo(c.x+5,c.y-2);
            p.startNewSubPath(c.x-9,c.y+2);p.lineTo(c.x-9,c.y+8);p.lineTo(c.x+9,c.y+8);p.lineTo(c.x+9,c.y+2);g.strokePath(p,juce::PathStrokeType(1.5f));}
        else if(icon==CommandIcon::returnBack){p.startNewSubPath(c.x-8,c.y-1);p.lineTo(c.x+3,c.y-1);p.cubicTo(c.x+12,c.y-1,c.x+12,c.y+8,c.x+2,c.y+8);
            p.startNewSubPath(c.x-2,c.y-7);p.lineTo(c.x-8,c.y-1);p.lineTo(c.x-2,c.y+5);g.strokePath(p,juce::PathStrokeType(1.5f));}
    }
protected:
    virtual bool effectiveLatchState()const noexcept{return false;}
private:
    CommandIcon icon;CommandVariant variant;
};
}
