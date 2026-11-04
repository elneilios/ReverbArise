#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <optional>

namespace m3::ui::controls
{
// Pins controller ownership before every human input path, including the
// Slider's embedded numeric editor. Keep the token through asynchronous change
// delivery; the next invocation replaces it. Styling remains the shared theme.
class InvocationSlider:public juce::Slider,private juce::Label::Listener
{
public:
    InvocationSlider(){onDragStart=[this]{begin();};}
    ~InvocationSlider()override{for(auto* c:getChildren())if(auto* label=dynamic_cast<juce::Label*>(c))label->removeListener(this);}
    std::function<void()> beforeInvocation;
    // Explicit value typography survives LookAndFeel/text-box recreation.
    void setValueFont(juce::Font font){valueFont=std::move(font);updateLabels();}
    void mouseDown(const juce::MouseEvent& e)override{begin();juce::Slider::mouseDown(e);}
    void mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& w)override{begin();juce::Slider::mouseWheelMove(e,w);}
    bool keyPressed(const juce::KeyPress& key)override{begin();return juce::Slider::keyPressed(key);}
    void childrenChanged()override
    {juce::Slider::childrenChanged();updateLabels();}
protected:
    void setValueWithinInvocation(double value)
    {
        const juce::ScopedValueSetter<bool> retain(retainInvocation,true);
        const juce::Slider::ScopedDragNotification gesture(*this);
        setValue(value,juce::sendNotificationSync);
    }
private:
    bool retainInvocation=false;
    std::optional<juce::Font> valueFont;
    void updateLabels(){for(auto* c:getChildren())if(auto* label=dynamic_cast<juce::Label*>(c)){label->addListener(this);if(valueFont)label->setFont(*valueFont);}}
    void begin(){if(!retainInvocation && beforeInvocation)beforeInvocation();}
    void editorShown(juce::Label*,juce::TextEditor&)override{begin();}
    void labelTextChanged(juce::Label*)override{}
};
}
