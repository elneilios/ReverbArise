#pragma once
#include "DialTrackPresentation.h"
#include "FocusVisibility.h"
#include "InvocationSlider.h"

namespace m3::ui::controls
{
class TrackSlider : public InvocationSlider
{
public:
    TrackSlider()
    {
        setWantsKeyboardFocus(true);
        setMouseClickGrabsKeyboardFocus(true);
        setVelocityModeParameters(1.0,1,0,false);
    }

    void setWheelSteps(double coarse,double fine)
    {
        jassert(coarse>=fine && fine>0);
        wheelSteps=WheelSteps{coarse,fine};
    }

    void paint(juce::Graphics& g) override
    {
        if(getSliderStyle()!=LinearHorizontal){InvocationSlider::paint(g);return;}
        const auto frame=trackFrame();
        DialTrackPresentation::track(g,frame,{isEnabled(),isMouseOver(false),isMouseButtonDown(),isFocusVisible()},
                                     getPositionOfValue(getMinimum()),getPositionOfValue(getMaximum()),getPositionOfValue(getValue()));
    }

    bool isFocusVisible() const {return hasKeyboardFocus(false) && visibility->isVisible();}
    void focusGained(FocusChangeType cause) override
    {
        if(cause==focusChangedByMouseClick)visibility->pointerInput();
        else visibility->keyboardInput();
        repaint();
    }
    void focusLost(FocusChangeType) override {repaint();}
    void mouseEnter(const juce::MouseEvent&) override {repaint();}
    void mouseExit(const juce::MouseEvent&) override {repaint();}
    void mouseDown(const juce::MouseEvent& event) override
    {
        visibility->pointerInput();
        InvocationSlider::mouseDown(event);
        repaint();
    }
    void mouseUp(const juce::MouseEvent& event) override {InvocationSlider::mouseUp(event);repaint();}
    bool keyPressed(const juce::KeyPress& key) override
    {
        visibility->keyboardInput();
        return InvocationSlider::keyPressed(key);
    }

    void mouseWheelMove(const juce::MouseEvent& event,const juce::MouseWheelDetails& wheel) override
    {
        const auto point=event.getEventRelativeTo(this).position.toInt();
        if(!isEnabled() || !isScrollWheelEnabled() || !getLocalBounds().contains(point))
        {
            juce::Component::mouseWheelMove(event,wheel);
            return;
        }
        if(!wheelSteps || !trackFrame().contains(point))
        {
            InvocationSlider::mouseWheelMove(event,wheel);
            return;
        }
        const auto amount=DialTrackPresentation::wheelAmount(wheel);
        if(amount==0 || event.mods.isAnyMouseButtonDown() || event.eventTime==lastWheel)return;
        lastWheel=event.eventTime;
        const auto step=event.mods.isCtrlDown()?wheelSteps->fine:wheelSteps->coarse;
        const juce::Component::SafePointer<TrackSlider> safe(this);
        hideTextBox(false);
        if(safe)safe->invokeValue(safe->getValue()+std::copysign(step,amount));
    }

    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<AccessibleTrack>(*this);
    }

private:
    struct WheelSteps {double coarse,fine;};
    class AccessibleTrack final : public juce::AccessibilityHandler
    {
        class Value final : public juce::AccessibilityValueInterface
        {
        public:
            explicit Value(TrackSlider& owner):slider(owner){}
            bool isReadOnly() const override {return !slider.isEnabled() || slider.getMinimum()>=slider.getMaximum();}
            double getCurrentValue() const override {return slider.getValue();}
            juce::String getCurrentValueAsString() const override {return slider.getTextFromValue(slider.getValue());}
            void setValue(double value) override {if(!isReadOnly())slider.invokeValue(value);}
            void setValueAsString(const juce::String& value) override {setValue(slider.getValueFromText(value));}
            AccessibleValueRange getRange() const override
            {
                if(slider.getMinimum()>=slider.getMaximum())return {};
                const auto step=slider.getInterval()>0?slider.getInterval():(slider.getMaximum()-slider.getMinimum())*.01;
                return {{slider.getMinimum(),slider.getMaximum()},step};
            }
        private:
            TrackSlider& slider;
        };
    public:
        explicit AccessibleTrack(TrackSlider& slider)
            : AccessibilityHandler(slider,juce::AccessibilityRole::slider,juce::AccessibilityActions{}
                .addAction(juce::AccessibilityActionType::focus,[&slider]
                {
                    if(!slider.isEnabled())return;
                    slider.visibility->accessibilityInput();
                    slider.grabKeyboardFocus();
                }),Interfaces{std::make_unique<Value>(slider)}),
              owner(slider){}
        juce::String getHelp() const override {return owner.getTooltip();}
    private:
        TrackSlider& owner;
    };

    juce::Rectangle<int> trackFrame()
    {
        auto frame=getLocalBounds();
        const auto field=getLookAndFeel().getSliderLayout(*this).textBoxBounds;
        if(!field.isEmpty() && getTextBoxPosition()==TextBoxRight)frame.setRight(field.getX());
        return frame;
    }

    void invokeValue(double value)
    {
        if(!isEnabled() || !std::isfinite(value))return;
        const juce::Component::SafePointer<TrackSlider> safe(this);
        if(beforeInvocation)beforeInvocation();
        if(safe && safe->isEnabled())safe->setValueWithinInvocation(value);
    }

    juce::SharedResourcePointer<FocusVisibility> visibility;
    std::optional<WheelSteps> wheelSteps;
    juce::Time lastWheel;
};
}
