#pragma once
#include "InvocationSlider.h"
#include "NumericInputControl.h"

namespace m3::ui::controls
{
// Shared slider + numeric-stepper family. All input paths use the same Slider
// range/interval/formatter and existing invocation ownership/notification path.
class SliderControl final:public InvocationSlider
{
    class Theme final:public WorkspaceTheme
    {
    public:
        explicit Theme(SliderControl& s):owner(s){}
        juce::Label* createSliderTextBox(juce::Slider& slider)override
        {
            auto* field=new NumericInputControl;
            field->setColour(juce::Label::textColourId,slider.findColour(juce::Slider::textBoxTextColourId));
            field->setColour(juce::Label::backgroundColourId,slider.findColour(juce::Slider::textBoxBackgroundColourId));
            field->setColour(juce::Label::outlineColourId,slider.findColour(juce::Slider::textBoxOutlineColourId));
            const auto safe=juce::Component::SafePointer<SliderControl>(&owner);
            field->onBegin=[safe]{if(safe && safe->beforeInvocation)safe->beforeInvocation();};
            field->onStep=[safe](int direction)
            {
                if(!safe || !safe->isEnabled())return;
                safe->hideTextBox(false);
                const auto step=safe->getInterval()>0?safe->getInterval():(safe->getMaximum()-safe->getMinimum())*.01;
                safe->setValueWithinInvocation(safe->getValue()+direction*step);
            };
            return field;
        }
    private:SliderControl& owner;
    };
public:
    SliderControl():theme(std::make_unique<Theme>(*this))
    {setLookAndFeel(theme.get());setScrollWheelEnabled(true);setSliderStyle(juce::Slider::LinearHorizontal);setTextBoxStyle(juce::Slider::TextBoxRight,false,controlGeometry::valueWidth,controlGeometry::valueHeight);setValueFont(interFont(controlSize));}
    ~SliderControl()override{setLookAndFeel(nullptr);}
private:std::unique_ptr<Theme> theme;
};
}
