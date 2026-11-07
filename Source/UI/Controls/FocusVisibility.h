#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace m3::ui::controls
{
// Input provenance only. Families opt into its visual policy independently;
// this never changes focus ownership, consumes a key or invokes a command.
class FocusVisibility final:private juce::MouseListener,private juce::KeyListener,private juce::FocusChangeListener
{
public:
    FocusVisibility()
    {
        auto& desktop=juce::Desktop::getInstance();
        desktop.addGlobalMouseListener(this);desktop.addFocusChangeListener(this);
        globalFocusChanged(juce::Component::getCurrentlyFocusedComponent());
    }
    ~FocusVisibility()override
    {
        if(focused!=nullptr)focused->removeKeyListener(this);
        auto& desktop=juce::Desktop::getInstance();
        desktop.removeFocusChangeListener(this);desktop.removeGlobalMouseListener(this);
    }
    FocusVisibility(const FocusVisibility&)=delete;
    FocusVisibility& operator=(const FocusVisibility&)=delete;
    bool isVisible()const noexcept{return keyboardOrAccessibility;}
    void keyboardInput(){setVisible(true);}
    void accessibilityInput(){setVisible(true);}
    void pointerInput(){setVisible(false);}
private:
    void setVisible(bool visible)
    {
        if(keyboardOrAccessibility==visible)return;
        keyboardOrAccessibility=visible;
        if(auto* target=juce::Component::getCurrentlyFocusedComponent())target->repaint();
    }
    void mouseDown(const juce::MouseEvent&)override{pointerInput();}
    bool keyPressed(const juce::KeyPress&,juce::Component*)override{keyboardInput();return false;}
    void globalFocusChanged(juce::Component* target)override
    {
        if(focused!=nullptr)focused->removeKeyListener(this);
        focused=target;
        if(focused!=nullptr)focused->addKeyListener(this);
    }
    bool keyboardOrAccessibility=true;
    juce::Component::SafePointer<juce::Component> focused;
};
}
