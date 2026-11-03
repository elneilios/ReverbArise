#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
namespace m3::ui::controls
{
class InvocationToggle:public juce::ToggleButton
{
public:
    using juce::ToggleButton::ToggleButton;
    std::function<void()> beforeInvocation;
    void mouseDown(const juce::MouseEvent& e)override{if(beforeInvocation)beforeInvocation();juce::ToggleButton::mouseDown(e);}
    bool keyPressed(const juce::KeyPress& key)override
    {if(beforeInvocation)beforeInvocation();if(key.isKeyCode(juce::KeyPress::spaceKey)){if(isEnabled())triggerClick();return true;}return juce::ToggleButton::keyPressed(key);}
};
}
