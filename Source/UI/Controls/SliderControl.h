#pragma once
#include "InvocationSlider.h"
#include "FieldNumericInput.h"

namespace m3::ui::controls
{
// Shared slider + numeric-stepper family. All input paths use the same Slider
// range/interval/formatter and existing invocation ownership/notification path.
class SliderControl:public InvocationSlider
{
public:
    struct FieldMetadata
    {
        juce::String caption;
        juce::String description;
        bool steppers = true;
    };

private:
    class Theme final:public WorkspaceTheme
    {
    public:
        Theme(SliderControl& slider, FieldMetadata metadata)
            : owner(slider), fieldMetadata(std::move(metadata)) {}
        juce::Label* createSliderTextBox(juce::Slider& slider)override
        {
            auto* field=fieldMetadata.caption.isEmpty()
                ? new NumericInputControl
                : new FieldNumericInput(fieldMetadata.caption,fieldMetadata.description,fieldMetadata.steppers);
            field->setColour(juce::Label::textColourId,slider.findColour(juce::Slider::textBoxTextColourId));
            field->setColour(juce::Label::backgroundColourId,slider.findColour(juce::Slider::textBoxBackgroundColourId));
            field->setColour(juce::Label::outlineColourId,slider.findColour(juce::Slider::textBoxOutlineColourId));
            const auto safe=juce::Component::SafePointer<SliderControl>(&owner);
            field->onBegin=[safe]{if(safe && safe->beforeInvocation)safe->beforeInvocation();};
            if (fieldMetadata.steppers) field->onStep=[safe](int direction)
            {
                if(!safe || !safe->isEnabled())return;
                safe->hideTextBox(false);
                const auto step=safe->getInterval()>0?safe->getInterval():(safe->getMaximum()-safe->getMinimum())*.01;
                safe->setValueWithinInvocation(safe->getValue()+direction*step);
            };
            return field;
        }
    private:
        SliderControl& owner;
        FieldMetadata fieldMetadata;
    };
public:
    SliderControl():SliderControl(FieldMetadata{}) {}
    explicit SliderControl(FieldMetadata metadata):theme(std::make_unique<Theme>(*this,std::move(metadata)))
    {setLookAndFeel(theme.get());setScrollWheelEnabled(true);setSliderStyle(juce::Slider::LinearHorizontal);setTextBoxStyle(juce::Slider::TextBoxRight,false,controlGeometry::valueWidth,controlGeometry::valueHeight);setValueFont(interFont(controlSize));}
    ~SliderControl()override{setLookAndFeel(nullptr);}
private:std::unique_ptr<Theme> theme;
};
}
