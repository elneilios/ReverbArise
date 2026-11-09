#pragma once
#include <UI/WorkspaceTheme.h>

namespace m3::ui::controls
{
// A reusable formatted numeric editor. The binding owns range, formatting and
// validation; this control owns typed editing and accessible increment buttons.
class NumericInputControl:public juce::Label
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
            {invokeStep();return true;}
            return juce::Button::keyPressed(key);
        }
        void focusGained(FocusChangeType)override{repaint();}
        void focusLost(FocusChangeType)override{repaint();}
        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override
        {
            return std::make_unique<juce::AccessibilityHandler>(*this, juce::AccessibilityRole::button,
                juce::AccessibilityActions{}.addAction(juce::AccessibilityActionType::press, [this]
                {
                    invokeStep();
                }));
        }
        void paintButton(juce::Graphics& g,bool over,bool down)override
        {
            if(over || down || hasKeyboardFocus(false)){g.setColour(aqua.withAlpha(.12f));g.fillRect(getLocalBounds());}
            g.setColour((over || down || hasKeyboardFocus(false)?aqua:m3::ui::text).withMultipliedAlpha(isEnabled()?1.f:.35f));
            const auto c=getLocalBounds().toFloat().getCentre();juce::Path p;
            p.startNewSubPath(c.x-3,c.y+(up?1.5f:-1.5f));p.lineTo(c.x,c.y+(up?-1.5f:1.5f));p.lineTo(c.x+3,c.y+(up?1.5f:-1.5f));
            g.strokePath(p,juce::PathStrokeType(1.2f));
            if(hasKeyboardFocus(false))g.drawRect(getLocalBounds().toFloat().reduced(1),1.f);
        }
    private:
        void invokeStep()
        {
            if(!isEnabled())return;
            const juce::Component::SafePointer<StepButton> safe(this);
            if(onBegin)onBegin();
            if(safe && safe->isEnabled())safe->triggerClick();
        }
        bool up;
    };
public:
    explicit NumericInputControl(bool steppers = true) : hasSteppers(steppers)
    {
        setFont(interFont(controlSize));setJustificationType(juce::Justification::centred);setKeyboardType(juce::TextInputTarget::decimalKeyboard);
        setBorderSize({0,spacing::tight,0,spacing::tight+(hasSteppers?controlGeometry::numericStepWidth:0)});
        if (!hasSteppers) return;
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
        juce::Label::resized();
        if (hasSteppers)
        {
            auto buttons=getLocalBounds().removeFromRight(controlGeometry::numericStepWidth);
            increment.setBounds(buttons.removeFromTop(buttons.getHeight()/2));decrement.setBounds(buttons);
        }
        if(auto* input=getCurrentTextEditor())input->setBounds(valueBounds());
    }
    void paint(juce::Graphics& g)override
    {juce::Label::paint(g);paintStepDivider(g);}
protected:
    juce::Rectangle<int> valueBounds() const
    {return getLocalBounds().withTrimmedRight(hasSteppers?controlGeometry::numericStepWidth:0);}
    void paintStepDivider(juce::Graphics& graphics) const
    {
        if (!hasSteppers) return;
        graphics.setColour(outline);
        graphics.drawVerticalLine(valueBounds().getRight(),1.f,static_cast<float>(getHeight()-1));
    }
private:
    const bool hasSteppers;
    StepButton increment{true},decrement{false};
};
}
