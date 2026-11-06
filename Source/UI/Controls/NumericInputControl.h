#pragma once
#include <UI/WorkspaceTheme.h>

namespace m3::ui::controls
{
// A reusable formatted numeric editor. The binding owns range, formatting and
// validation; this control owns typed editing and accessible increment buttons.
class NumericInputControl final:public juce::Label
{
    class StepButton final:public juce::Button
    {
    public:
        explicit StepButton(bool increase):juce::Button(increase?"Increase value":"Decrease value"),up(increase)
        {setTitle(getButtonText());setTooltip(getButtonText());setWantsKeyboardFocus(true);setRepeatSpeed(400,80,20);}
        std::function<void()> onBegin;
        void mouseDown(const juce::MouseEvent& e)override{if(isEnabled() && onBegin)onBegin();juce::Button::mouseDown(e);}
        bool keyPressed(const juce::KeyPress& key)override
        {
            if(key.isKeyCode(juce::KeyPress::returnKey) || key.isKeyCode(juce::KeyPress::spaceKey))
            {if(isEnabled()){if(onBegin)onBegin();triggerClick();}return true;}
            return juce::Button::keyPressed(key);
        }
        void focusGained(FocusChangeType)override{repaint();}
        void focusLost(FocusChangeType)override{repaint();}
        void paintButton(juce::Graphics& g,bool over,bool down)override
        {
            if(over || down || hasKeyboardFocus(false)){g.setColour(aqua.withAlpha(.12f));g.fillRect(getLocalBounds());}
            g.setColour((over || down || hasKeyboardFocus(false)?aqua:m3::ui::text).withMultipliedAlpha(isEnabled()?1.f:.35f));
            const auto c=getLocalBounds().toFloat().getCentre();juce::Path p;
            p.startNewSubPath(c.x-3,c.y+(up?1.5f:-1.5f));p.lineTo(c.x,c.y+(up?-1.5f:1.5f));p.lineTo(c.x+3,c.y+(up?1.5f:-1.5f));
            g.strokePath(p,juce::PathStrokeType(1.2f));
            if(hasKeyboardFocus(false))g.drawRect(getLocalBounds().toFloat().reduced(1),1.f);
        }
    private:bool up;
    };
public:
    NumericInputControl()
    {
        setFont(interFont(controlSize));setJustificationType(juce::Justification::centred);setKeyboardType(juce::TextInputTarget::decimalKeyboard);
        setBorderSize({0,spacing::tight,0,controlGeometry::numericStepWidth+spacing::tight});
        addAndMakeVisible(increment);addAndMakeVisible(decrement);
        increment.onBegin=decrement.onBegin=[this]{if(onBegin)onBegin();};
        increment.onClick=[this]{if(isEnabled() && onStep)onStep(1);};decrement.onClick=[this]{if(isEnabled() && onStep)onStep(-1);};
    }
    std::function<void()> onBegin;
    std::function<void(int)> onStep;
    juce::Button& incrementButton()noexcept{return increment;}
    juce::Button& decrementButton()noexcept{return decrement;}
    void resized()override
    {
        juce::Label::resized();auto buttons=getLocalBounds().removeFromRight(controlGeometry::numericStepWidth);
        increment.setBounds(buttons.removeFromTop(buttons.getHeight()/2));decrement.setBounds(buttons);
        if(auto* input=getCurrentTextEditor())input->setBounds(getLocalBounds().withTrimmedRight(controlGeometry::numericStepWidth));
    }
    void paint(juce::Graphics& g)override
    {juce::Label::paint(g);g.setColour(outline);g.drawVerticalLine(getWidth()-controlGeometry::numericStepWidth,1.f,static_cast<float>(getHeight()-1));}
private:
    StepButton increment{true},decrement{false};
};
}
