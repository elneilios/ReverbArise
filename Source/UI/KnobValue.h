#pragma once
#include "WorkspaceTheme.h"

namespace m3::ui
{
// A value cell owns the edit lifetime, not the selected object. The controller
// pins an exact model value when begin is called and applies relative deltas.
class KnobValue final:public juce::Component
{
    class Dial final:public juce::Slider
    {
    public:
        explicit Dial(KnobValue& cell):owner(cell){setWantsKeyboardFocus(true);}
        void paint(juce::Graphics& g)override
        {
            auto& look=getLookAndFeel();const auto box=look.getSliderLayout(*this).sliderBounds;const auto angles=getRotaryParameters();
            look.drawRotarySlider(g,box.getX(),box.getY(),box.getWidth(),box.getHeight(),static_cast<float>(owner.canonicalFraction),angles.startAngleRadians,angles.endAngleRadians,*this);
            if(hasKeyboardFocus(false)){g.setColour(aqua);g.drawRoundedRectangle(controlGeometry::insideStroke(getLocalBounds(),controlGeometry::focusStroke),radius,controlGeometry::focusStroke);}
        }
        void mouseDown(const juce::MouseEvent& e)override{owner.inputModifiers(e.mods);juce::Slider::mouseDown(e);}
        void mouseDrag(const juce::MouseEvent& e)override{owner.inputModifiers(e.mods);juce::Slider::mouseDrag(e);}
        void mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel)override
        {owner.inputModifiers(e.mods);if(owner.onStep && isEnabled() && !e.mods.isAnyMouseButtonDown())
            {if(e.eventTime!=lastWheel){lastWheel=e.eventTime;const auto amount=std::abs(wheel.deltaX)>std::abs(wheel.deltaY)?-wheel.deltaX:wheel.deltaY;owner.step(amount*10*(wheel.isReversed?-1:1));}}
            else juce::Slider::mouseWheelMove(e,wheel);}
        void modifierKeysChanged(const juce::ModifierKeys& mods)override{owner.inputModifiers(mods);}
        bool keyPressed(const juce::KeyPress& key)override
        {
            const auto code=key.getKeyCode();const auto direction=code==juce::KeyPress::rightKey || code==juce::KeyPress::upKey?1:code==juce::KeyPress::leftKey || code==juce::KeyPress::downKey?-1:0;
            if(!direction)return juce::Slider::keyPressed(key);
            if(isEnabled()){owner.inputModifiers(key.getModifiers());if(owner.onStep)owner.step(direction);else {juce::Slider::ScopedDragNotification gesture(*this);setValue(getValue()+direction*.01*(key.getModifiers().isShiftDown()?.1:1.),juce::sendNotificationSync);}}return true;
        }
        void focusGained(FocusChangeType)override{repaint();}
        void focusLost(FocusChangeType)override{owner.cancel();owner.inputModifiers({});repaint();}
        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override{return owner.accessibility(*this);}
    private:KnobValue& owner;juce::Time lastWheel;
    };
    class Caption final:public juce::Label
    {
    public:void paint(juce::Graphics& g)override
        {applyTextRole(g,TextRole::controlLabel);g.drawText(getText(),getLocalBounds(),juce::Justification::centredLeft);}
    };
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
    KnobValue():knob(*this)
    {
        addAndMakeVisible(label);addAndMakeVisible(knob);addAndMakeVisible(entry);
        label.setBorderSize({});
        knob.setRange(0,1,.000001);knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);knob.setScrollWheelEnabled(true);
        knob.setVelocityModeParameters(.2,1,0,true,juce::ModifierKeys::shiftModifier);
        entry.setFont(fontFor(*this,controlSize));entry.setJustification(juce::Justification::centredLeft);
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
        applyTextRole(label,TextRole::controlLabel);
    }
    std::function<void()> onBegin,onEnd;
    std::function<void()> onRebase;
    std::function<void(const juce::ModifierKeys&)> onModifierInput;
    std::function<void(double)> onDelta;
    std::function<void(double)> onStep;
    std::function<bool(const juce::String&)> onCommit;
    void configure(const juce::String& title,const juce::String& name,const juce::String& tooltip)
    {
        label.setText(title,juce::dontSendNotification);setName(name+"Cell");
        label.setTooltip(title);
        entry.setName(name);knob.setName(name+"Knob");
        setTitle(title);entry.setTitle(title+" exact value");knob.setTitle(title);
        entry.setTooltip(tooltip);knob.setTooltip(tooltip);
    }
    void display(const juce::String& owner,const juce::String& valueText,double fraction,bool enabled)
    {
        if(owner!=identity){cancel();identity=owner;}
        canonical=valueText;canonicalFraction=std::clamp(fraction,0.0,1.0);
        if(!domainSupplied)domain={canonicalFraction,0,1,.01,1,canonical};
        setEnabled(enabled);
        if(!typing){refreshing=true;entry.setText(canonical,false);if(!active)knob.setValue(canonicalFraction,juce::dontSendNotification);refreshing=false;}knob.repaint();
    }
    void cancel(){if(active && !typing)cancelledGesture=true;active=false;typing=false;restore();if(onEnd)onEnd();}
    bool editing()const noexcept{return active;}
    void setCaption(const juce::String& caption,const juce::String& fullTitle={})
    {const auto title=fullTitle.isEmpty()?caption:fullTitle;label.setText(caption,juce::dontSendNotification);label.setTooltip(title);setTitle(title);knob.setTitle(title);entry.setTitle(title+" exact value");}
    void setGestureModifiers(const juce::ModifierKeys& mods)
    {
        const auto next=mods.getRawFlags()&(juce::ModifierKeys::ctrlModifier|juce::ModifierKeys::shiftModifier);
        if(next==modifiers)return;modifiers=next;
        rebaseGesture();
    }
    void rebaseGesture(){if(active && !typing){origin=knob.getValue();if(onRebase)onRebase();}}
    struct DomainValue {double value=0,minimum=0,maximum=1,step=.01,unitsPerFraction=1;juce::String text;};
    void displayDomain(DomainValue value){domain=std::move(value);domainSupplied=true;}
    double displayedFraction()const noexcept{return canonicalFraction;}
    juce::TextEditor& valueField()noexcept{return entry;}
    juce::Slider& valueKnob()noexcept{return knob;}
    void lookAndFeelChanged()override{entry.setFont(textFont(TextRole::controlValue));applyTextRole(label,TextRole::controlLabel);}
    void resized()override
    {
        auto bounds=getLocalBounds();
        knob.setBounds(bounds.removeFromLeft(42).withSizeKeepingCentre(42,42));
        bounds.removeFromLeft(spacing::labelHelp);
        label.setBounds(bounds.removeFromTop(controlGeometry::labelHeight));
        entry.setBounds(bounds.removeFromTop(controlGeometry::valueHeight));
    }
private:
    void step(double amount){if(!isEnabled() || !std::isfinite(amount) || amount==0)return;begin(false);if(onStep)onStep(amount);finish(false);}
    void inputModifiers(const juce::ModifierKeys& mods){if(onModifierInput)onModifierInput(mods);else setGestureModifiers(mods);}
    std::unique_ptr<juce::AccessibilityHandler> accessibility(juce::Component& component)
    {
        class Value final:public juce::AccessibilityValueInterface
        {
        public:explicit Value(KnobValue& cell):owner(cell){}
            bool isReadOnly()const override{return !owner.isEnabled();}
            double getCurrentValue()const override{return owner.domain.value;}
            juce::String getCurrentValueAsString()const override{return owner.domain.text.isNotEmpty()?owner.domain.text:owner.canonical;}
            void setValue(double value)override
            {if(!owner.isEnabled() || !std::isfinite(value) || owner.domain.unitsPerFraction<=0)return;const auto delta=(value-owner.domain.value)/owner.domain.unitsPerFraction;owner.begin(false);if(owner.onDelta)owner.onDelta(delta);owner.finish(false);}
            void setValueAsString(const juce::String& value)override
            {if(!owner.isEnabled())return;owner.begin(true);if(owner.onCommit)owner.onCommit(value);owner.finish(false);}
            AccessibleValueRange getRange()const override{return {{owner.domain.minimum,owner.domain.maximum},owner.domain.step};}
        private:KnobValue& owner;
        };
        return std::make_unique<juce::AccessibilityHandler>(component,juce::AccessibilityRole::slider,juce::AccessibilityActions{},juce::AccessibilityHandler::Interfaces{std::make_unique<Value>(*this)});
    }
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
    Caption label;Dial knob;Entry entry;
    DomainValue domain;int modifiers=0;bool domainSupplied=false;
    juce::String identity,canonical;
    double origin=0,canonicalFraction=0;
    bool refreshing=false,active=false,typing=false,cancelledGesture=false;
};
}
