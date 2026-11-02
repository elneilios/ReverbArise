#pragma once
#include "WorkspaceTheme.h"

namespace m3::ui
{
// A value cell owns the edit lifetime, not the selected object. The controller
// pins an exact model value when begin is called and applies relative deltas.
class KnobValue final:public juce::Component
{
    class Entry final:public juce::TextEditor
    {
    public:
        std::function<void()> gained;
        void focusGained(FocusChangeType type)override{juce::TextEditor::focusGained(type);if(gained)gained();}
        void mouseDown(const juce::MouseEvent& event)override{if(gained)gained();juce::TextEditor::mouseDown(event);}
        bool keyPressed(const juce::KeyPress& key)override
        {if(key!=juce::KeyPress::escapeKey && key!=juce::KeyPress::returnKey && gained)gained();return juce::TextEditor::keyPressed(key);}
    };
public:
    KnobValue()
    {
        addAndMakeVisible(label);addAndMakeVisible(knob);addAndMakeVisible(entry);
        label.setBorderSize({});
        knob.setRange(0,1,.000001);knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);knob.setScrollWheelEnabled(false);
        knob.setVelocityModeParameters(.2,1,0,true,juce::ModifierKeys::shiftModifier);
        entry.setFont(juce::FontOptions(controlSize));entry.setJustification(juce::Justification::centredLeft);
        entry.setSelectAllWhenFocused(true);
        entry.gained=[this]{begin(true);};
        entry.onReturnKey=[this]{finish(true);};entry.onFocusLost=[this]{if(typing)finish(true);};
        entry.onEscapeKey=[this]{cancel();};
        knob.onDragStart=[this]{cancelledGesture=false;begin(false);};
        knob.onValueChange=[this]
        {
            if(refreshing)return;if(cancelledGesture){restore();return;}
            const auto oneShot=!active;begin(false);
            if(onDelta)onDelta(knob.getValue()-origin);
            if(oneShot)finish(false);
        };
        knob.onDragEnd=[this]{cancelledGesture=false;finish(false);};
        label.setFont(juce::FontOptions(smallSize));
    }
    std::function<void()> onBegin,onEnd;
    std::function<void(double)> onDelta;
    std::function<bool(const juce::String&)> onCommit;
    void configure(const juce::String& title,const juce::String& name,const juce::String& tooltip)
    {
        label.setText(title,juce::dontSendNotification);setName(name+"Cell");
        entry.setName(name);knob.setName(name+"Knob");
        entry.setTooltip(tooltip);knob.setTooltip(tooltip);
    }
    void display(const juce::String& owner,const juce::String& valueText,double fraction,bool enabled)
    {
        if(owner!=identity){cancel();identity=owner;}
        canonical=valueText;canonicalFraction=std::clamp(fraction,0.0,1.0);
        setEnabled(enabled);
        if(!typing){refreshing=true;entry.setText(canonical,false);if(!active)knob.setValue(canonicalFraction,juce::dontSendNotification);refreshing=false;}
    }
    void cancel(){if(active && !typing)cancelledGesture=true;active=false;typing=false;restore();if(onEnd)onEnd();}
    bool editing()const noexcept{return active;}
    void setCaption(const juce::String& caption){label.setText(caption,juce::dontSendNotification);}
    juce::TextEditor& valueField()noexcept{return entry;}
    juce::Slider& valueKnob()noexcept{return knob;}
    void resized()override
    {
        auto bounds=getLocalBounds().reduced(0,2);
        knob.setBounds(bounds.removeFromLeft(46).withSizeKeepingCentre(42,42));
        bounds.removeFromLeft(8);
        label.setBounds(bounds.removeFromTop(16));
        entry.setBounds(bounds.reduced(0,3));
    }
private:
    void begin(bool textEntry)
    {
        if(active && typing==textEntry)return;
        if(active)finish(typing);
        active=true;typing=textEntry;origin=canonicalFraction;if(onBegin)onBegin();
    }
    void restore(){refreshing=true;entry.setText(canonical,false);knob.setValue(canonicalFraction,juce::dontSendNotification);refreshing=false;}
    void finish(bool commit)
    {
        if(!active){if(commit && entry.getText()!=canonical)begin(true);else return;}
        if(commit && typing && entry.getText()!=canonical && onCommit)onCommit(entry.getText());
        active=false;typing=false;restore();if(onEnd)onEnd();
    }
    juce::Label label;juce::Slider knob;Entry entry;
    juce::String identity,canonical;
    double origin=0,canonicalFraction=0;
    bool refreshing=false,active=false,typing=false,cancelledGesture=false;
};
}
