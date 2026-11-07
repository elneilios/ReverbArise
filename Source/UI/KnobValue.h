#pragma once
#include "WorkspaceTheme.h"

namespace m3::ui
{
// A value cell owns the edit lifetime, not the selected object. The controller
// pins an exact model value when begin is called and applies relative deltas.
class KnobValue final:public juce::Component
{
public:
    struct ResetAction {juce::String label;std::function<void()> apply;};
private:
    class Dial final:public juce::Slider
    {
    public:
        explicit Dial(KnobValue& cell):owner(cell){setWantsKeyboardFocus(true);}
        void paint(juce::Graphics& g)override
        {
            auto& look=getLookAndFeel();const auto box=look.getSliderLayout(*this).sliderBounds;const auto angles=getRotaryParameters();
            look.drawRotarySlider(g,box.getX(),box.getY(),box.getWidth(),box.getHeight(),static_cast<float>(owner.canonicalFraction),angles.startAngleRadians,angles.endAngleRadians,*this);
            if(hasKeyboardFocus(false))
            {
                const auto bounds=controlGeometry::insideStroke(getLocalBounds(),controlGeometry::focusStroke);
                const auto leg=std::min(bounds.getWidth(),bounds.getHeight())*.2f;juce::Path brackets;
                for(const auto corner:std::array<juce::Point<float>,4>{{bounds.getTopLeft(),bounds.getTopRight(),bounds.getBottomLeft(),bounds.getBottomRight()}})
                {const auto dx=corner.x==bounds.getX()?1.f:-1.f,dy=corner.y==bounds.getY()?1.f:-1.f;
                    brackets.startNewSubPath(corner.x+dx*leg,corner.y);brackets.lineTo(corner);brackets.lineTo(corner.x,corner.y+dy*leg);}
                g.setColour(aqua);g.strokePath(brackets,juce::PathStrokeType(controlGeometry::focusStroke,juce::PathStrokeType::mitered,juce::PathStrokeType::butt));
            }
        }
        void mouseDown(const juce::MouseEvent& e)override
        {
            owner.inputModifiers(e.mods);
            if(e.mods.isPopupMenu()){if(isEnabled())grabKeyboardFocus();owner.showResetMenu(e);return;}
            juce::Slider::mouseDown(e);
            if(isEnabled() && e.mods.isLeftButtonDown() && owner.active && !owner.typing && owner.onDialPress)
                owner.onDialPress();
        }
        void mouseDoubleClick(const juce::MouseEvent& e)override{if(e.mods.isLeftButtonDown() && !e.mods.isPopupMenu()){auto reset=owner.prepareResetAction();if(reset.apply)reset.apply();}}
        void mouseDrag(const juce::MouseEvent& e)override{owner.inputModifiers(e.mods);juce::Slider::mouseDrag(e);}
        void mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel)override
        {owner.inputModifiers(e.mods);if(owner.onStep && isEnabled() && !e.mods.isAnyMouseButtonDown())
            {if(e.eventTime!=lastWheel){lastWheel=e.eventTime;const auto amount=std::abs(wheel.deltaX)>std::abs(wheel.deltaY)?-wheel.deltaX:wheel.deltaY;owner.step(amount*10*(wheel.isReversed?-1:1));}}
            else juce::Slider::mouseWheelMove(e,wheel);}
        void modifierKeysChanged(const juce::ModifierKeys& mods)override{owner.inputModifiers(mods);juce::Slider::modifierKeysChanged(mods);}
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
        {g.setColour(text);g.setFont(getLookAndFeel().getLabelFont(*this));g.drawText(getText(),getLocalBounds(),juce::Justification::centredLeft);}
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
    std::function<void()> onDialPress;
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
    void display(const juce::String& owner,const juce::String& valueText,double fraction,bool enabled,const juce::String& exactText={})
    {
        if(owner!=identity){++resetTicket;cancel();identity=owner;}
        canonical=valueText;canonicalExact=exactText.isEmpty()?valueText:exactText;canonicalFraction=std::clamp(fraction,0.0,1.0);
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
    void displayDomain(DomainValue value)
    {
        domainValid=std::isfinite(value.value) && std::isfinite(value.minimum) && std::isfinite(value.maximum) && value.minimum<=value.maximum
            && std::isfinite(value.step) && value.step>0 && std::isfinite(value.unitsPerFraction) && value.unitsPerFraction>0;
        domain=domainValid?std::move(value):DomainValue{0,0,1,.01,1,"unavailable"};domainSupplied=true;
    }
    double displayedFraction()const noexcept{return canonicalFraction;}
    void setResetAction(std::function<ResetAction()> prepare)
    {onResetInvocation=std::move(prepare);knob.setTooltip(knob.getTooltip()+"\nDouble-click to reset. Right-click for reset options.");}
    ResetAction prepareResetAction()
    {
        if(!isEnabled() || !onResetInvocation)return {"Reset unavailable",{}};
        cancel();auto reset=onResetInvocation();const auto ticket=++resetTicket;const juce::Component::SafePointer<KnobValue> safe(this);
        if(reset.apply){auto action=std::move(reset.apply);reset.apply=[safe,ticket,action=std::move(action)]{if(safe && safe->isEnabled() && safe->resetTicket==ticket)action();};}
        return reset;
    }
    juce::TextEditor& valueField()noexcept{return entry;}
    juce::Slider& valueKnob()noexcept{return knob;}
    // Optional domain intent: absolute accessible writes must not inherit a
    // focused relative gesture clamp. Other consumers keep their delta path.
    std::function<void(double)> onAbsolute;
    void setCompactPresentation(bool enabled)
    {compactPresentation=enabled;if(enabled)entry.setIndents(responsiveGeometry::entryInset,0);lookAndFeelChanged();resized();}
    void lookAndFeelChanged()override
    {entry.setFont(componentTextFont(*this,TextRole::controlValue));applyTextRole(label,TextRole::controlLabel);if(compactPresentation)label.setFont(interFont(controlSize,InterFace::medium));}
    void resized()override
    {
        auto bounds=getLocalBounds();
        if(compactPresentation)
        {
            label.setBounds(bounds.removeFromTop(responsiveGeometry::captionHeight));
            auto row=bounds.removeFromTop(responsiveGeometry::dialSize);
            knob.setBounds(row.removeFromLeft(responsiveGeometry::dialSize));row.removeFromLeft(spacing::tight);
            entry.setBounds(row.withSizeKeepingCentre(row.getWidth(),responsiveGeometry::entryHeight));return;
        }
        knob.setBounds(bounds.removeFromLeft(42).withSizeKeepingCentre(42,42));
        bounds.removeFromLeft(spacing::labelHelp);
        label.setBounds(bounds.removeFromTop(controlGeometry::labelHeight));
        entry.setBounds(bounds.removeFromTop(controlGeometry::valueHeight));
    }
private:
    bool compactPresentation=false;
    void showResetMenu(const juce::MouseEvent& event)
    {
        if(!isEnabled())return;auto reset=prepareResetAction();juce::PopupMenu menu;menu.setLookAndFeel(&getLookAndFeel());
        menu.addItem(1,reset.label,static_cast<bool>(reset.apply));
        const auto point=event.getScreenPosition();
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&knob).withTargetScreenArea({point.x,point.y,1,1}).withDeletionCheck(*this),
            [action=std::move(reset.apply)](int result){if(result==1 && action)action();});
    }
    void step(double amount){if(!isEnabled() || !std::isfinite(amount) || amount==0)return;begin(false);if(onStep)onStep(amount);finish(false);}
    void inputModifiers(const juce::ModifierKeys& mods){if(onModifierInput)onModifierInput(mods);else setGestureModifiers(mods);}
    std::unique_ptr<juce::AccessibilityHandler> accessibility(juce::Component& component)
    {
        class Value final:public juce::AccessibilityValueInterface
        {
        public:explicit Value(KnobValue& cell):owner(cell){}
            bool isReadOnly()const override{return !owner.isEnabled() || !owner.domainValid || owner.domain.minimum==owner.domain.maximum;}
            double getCurrentValue()const override{return owner.domain.value;}
            juce::String getCurrentValueAsString()const override{return owner.domain.text.isNotEmpty()?owner.domain.text:owner.canonical;}
            void setValue(double value)override
            {if(isReadOnly() || !std::isfinite(value))return;const auto delta=(value-owner.domain.value)/owner.domain.unitsPerFraction;owner.begin(false);
                if(owner.onAbsolute)owner.onAbsolute(value);else if(owner.onDelta)owner.onDelta(delta);owner.finish(false);}
            void setValueAsString(const juce::String& value)override
            {if(isReadOnly())return;owner.begin(true);if(owner.onCommit)owner.onCommit(value);owner.finish(false);}
            AccessibleValueRange getRange()const override{return owner.domainValid && owner.domain.minimum<owner.domain.maximum?AccessibleValueRange{{owner.domain.minimum,owner.domain.maximum},owner.domain.step}:AccessibleValueRange{};}
        private:KnobValue& owner;
        };
        return std::make_unique<juce::AccessibilityHandler>(component,juce::AccessibilityRole::slider,juce::AccessibilityActions{},juce::AccessibilityHandler::Interfaces{std::make_unique<Value>(*this)});
    }
    void begin(bool textEntry)
    {
        if(active && typing==textEntry)return;
        if(active)finish(typing);
        ++resetTicket;
        if(textEntry){typingBaseline=canonicalExact;entry.setText(canonicalExact,false);entry.selectAll();}
        active=true;typing=textEntry;origin=canonicalFraction;if(onBegin)onBegin();
    }
    void restore(){refreshing=true;entry.setText(canonical,false);knob.setValue(canonicalFraction,juce::dontSendNotification);refreshing=false;}
    void finish(bool commit)
    {
        if(!active){if(commit && entry.getText()!=canonical){const auto proposed=entry.getText();begin(true);entry.setText(proposed,false);}else return;}
        if(commit && typing && entry.getText()!=typingBaseline && onCommit)onCommit(entry.getText());
        active=false;typing=false;restore();if(onEnd)onEnd();
    }
    Caption label;Dial knob;Entry entry;
    DomainValue domain;int modifiers=0;bool domainSupplied=false,domainValid=true;
    std::function<ResetAction()> onResetInvocation;uint64_t resetTicket=0;
    juce::String identity,canonical,canonicalExact,typingBaseline;
    double origin=0,canonicalFraction=0;
    bool refreshing=false,active=false,typing=false,cancelledGesture=false;
};
}
