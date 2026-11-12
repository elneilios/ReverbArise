#pragma once
#include "WorkspaceTheme.h"
#include "Controls/ChoicePopup.h"
#include "Controls/DialTrackPresentation.h"
#include "Controls/FieldTextEditor.h"
#include "Controls/FieldPresentation.h"

namespace m3::ui
{
// A value cell owns the edit lifetime, not the selected object. The controller
// pins an exact model value when begin is called and applies relative deltas.
class KnobValue final:public juce::Component
{
public:
    struct ResetAction {juce::String label;std::function<void()> apply;};
    struct HomeDetent {double value,band;};
private:
    class Dial final:public juce::Slider
    {
    public:
        explicit Dial(KnobValue& cell):owner(cell){setWantsKeyboardFocus(true);}
        void paint(juce::Graphics& g)override
        {
            const auto fraction=static_cast<float>(owner.canonicalFraction);
            const auto position=[&](double value)
            {return static_cast<float>(std::clamp(owner.canonicalFraction+(value-owner.domain.value)/owner.domain.unitsPerFraction,0.,1.));};
            const auto origin=owner.domain.minimum<0 && owner.domain.maximum>0?position(0):0.f;
            const auto home=owner.homeDetent?std::optional<float>(position(owner.homeDetent->value)):std::nullopt;
            controls::DialTrackPresentation::dial(g,getLocalBounds(),
                {isEnabled(),isMouseOver(false),isMouseButtonDown(),hasKeyboardFocus(false) && visibility->isVisible() && !pointerResetFocus},
                fraction,getRotaryParameters(),origin,home);
        }
        void mouseDown(const juce::MouseEvent& e)override
        {
            pointerResetFocus=false;
            visibility->pointerInput();
            owner.inputModifiers(e.mods);
            if(e.mods.isPopupMenu()){if(isEnabled())restoreResetMenuFocus(false);owner.showResetMenu(e);return;}
            juce::Slider::mouseDown(e);
            if(isEnabled() && e.mods.isLeftButtonDown() && owner.active && !owner.typing && owner.onDialPress)
                owner.onDialPress();
            repaint();
        }
        void mouseDoubleClick(const juce::MouseEvent& e)override{if(e.mods.isLeftButtonDown() && !e.mods.isPopupMenu()){auto reset=owner.prepareResetAction();if(reset.apply)reset.apply();}}
        void mouseDrag(const juce::MouseEvent& e)override
        {
            owner.inputModifiers(e.mods);
            const juce::ScopedValueSetter<bool> dragging(pointerDrag,true);
            const juce::ScopedValueSetter<bool> snapped(atHome,false);
            const auto before=getValue();
            juce::Slider::mouseDrag(e);
            // An exact typed value very close to home can already display at
            // the dial's rounded home position without being neutral.
            if(atHome && e.mouseWasDraggedSinceMouseDown() && getValue()==before && owner.active
                && owner.onAbsolute && owner.domain.value!=owner.homeDetent->value)
                owner.onAbsolute(owner.homeDetent->value);
        }
        bool isDraggingAtHome()const noexcept{return pointerDrag && atHome;}
        double snapValue(double candidate,DragMode mode)override
        {
            if(!pointerDrag || mode==notDragging || !owner.homeDetent || !owner.active || !owner.domainValid)
                return candidate;
            const auto& home=*owner.homeDetent;
            const auto span=owner.domain.unitsPerFraction;
            const auto distance=owner.dragDomainOrigin+(candidate-owner.origin)*span-home.value;
            atHome=std::abs(distance)<=home.band;
            return atHome?owner.origin+(home.value-owner.dragDomainOrigin)/span:candidate;
        }
        void mouseUp(const juce::MouseEvent& e)override{juce::Slider::mouseUp(e);repaint();}
        void mouseEnter(const juce::MouseEvent&)override{repaint();}
        void mouseExit(const juce::MouseEvent&)override{repaint();}
        void mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel)override
        {
            if(!owner.onStep || !isEnabled()){juce::Slider::mouseWheelMove(e,wheel);return;}
            const auto amount=controls::DialTrackPresentation::wheelAmount(wheel);
            if(amount==0 || e.mods.isAnyMouseButtonDown() || e.eventTime==lastWheel)return;
            lastWheel=e.eventTime;
            owner.inputModifiers(e.mods);
            owner.step(amount*10,e.mods.isCtrlDown());
        }
        void modifierKeysChanged(const juce::ModifierKeys& mods)override{owner.inputModifiers(mods);juce::Slider::modifierKeysChanged(mods);}
        bool keyPressed(const juce::KeyPress& key)override
        {
            pointerResetFocus=false;
            visibility->keyboardInput();
            const auto code=key.getKeyCode();const auto direction=code==juce::KeyPress::rightKey || code==juce::KeyPress::upKey?1:code==juce::KeyPress::leftKey || code==juce::KeyPress::downKey?-1:0;
            if(!direction)return juce::Slider::keyPressed(key);
            if(isEnabled()){owner.inputModifiers(key.getModifiers());if(owner.onStep)owner.step(direction);else {juce::Slider::ScopedDragNotification gesture(*this);setValue(getValue()+direction*.01*(key.getModifiers().isShiftDown()?.1:1.),juce::sendNotificationSync);}}return true;
        }
        void focusGained(FocusChangeType cause)override
        {
            if(cause==focusChangedByTabKey)pointerResetFocus=false;
            if(cause==focusChangedByMouseClick)visibility->pointerInput();
            else if(!owner.resetMenu && !pointerResetFocus)visibility->keyboardInput();
            repaint();
        }
        void focusLost(FocusChangeType)override{owner.cancel();owner.inputModifiers({});repaint();}
        void restoreResetMenuFocus(bool showBrackets)
        {
            pointerResetFocus=!showBrackets;
            grabKeyboardFocus();
            if(showBrackets)visibility->keyboardInput();
            else visibility->pointerInput();
            repaint();
        }
        void focusFromAccessibility()
        {
            pointerResetFocus=false;
            visibility->accessibilityInput();
            grabKeyboardFocus();
            repaint();
        }
        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override{return owner.accessibility(*this);}
    private:
        KnobValue& owner;
        juce::SharedResourcePointer<controls::FocusVisibility> visibility;
        juce::Time lastWheel;
        bool pointerDrag=false,atHome=false,pointerResetFocus=false;
    };
    class Caption final:public juce::Label
    {
    public:void paint(juce::Graphics& g)override
        {g.setColour(text);g.setFont(getLookAndFeel().getLabelFont(*this));g.drawText(getText(),getLocalBounds(),juce::Justification::centredLeft);}
    };
    class Entry final:public controls::FieldTextEditor
    {
    public:
        std::function<void()> gained;
        void focusGained(FocusChangeType type)override{controls::FieldTextEditor::focusGained(type);if(gained)gained();}
        void mouseDown(const juce::MouseEvent& event)override{if(gained)gained();controls::FieldTextEditor::mouseDown(event);}
        bool keyPressed(const juce::KeyPress& key)override
        {if(key!=juce::KeyPress::escapeKey && key!=juce::KeyPress::returnKey && gained)gained();return juce::TextEditor::keyPressed(key);}
    };
public:
    KnobValue():knob(*this)
    {
        addAndMakeVisible(label);addAndMakeVisible(knob);addAndMakeVisible(entry);addAndMakeVisible(feedback);
        label.setBorderSize({});
        knob.setRange(0,1,.000001);knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);knob.setScrollWheelEnabled(true);
        knob.setVelocityModeParameters(.2,1,0,true,juce::ModifierKeys::shiftModifier);
        entry.setFont(fontFor(*this,controlSize));entry.setJustification(juce::Justification::centredLeft);
        entry.setIndents(spacing::tight,0);
        entry.setNumericCharacters("0123456789+.-");
        entry.setSelectAllWhenFocused(true);
        entry.gained=[this]{begin(true);};
        entry.onReturnKey=[this]{finish(true);};entry.onFocusLost=[this]{if(typing)finish(true);};
        entry.onEscapeKey=[this]{cancel();};
        knob.onDragStart=[this]{cancelledGesture=false;begin(false);};
        knob.onValueChange=[this]
        {
            if(refreshing)return;if(cancelledGesture){restore();return;}
            const auto oneShot=!active;begin(false);
            if(knob.isDraggingAtHome() && onAbsolute)onAbsolute(homeDetent->value);
            else if(onDelta)onDelta(knob.getValue()-origin);
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
    std::function<void(double,bool)> onStep;
    std::function<bool(const juce::String&)> onCommit;
    void configure(const juce::String& title,const juce::String& name,const juce::String& tooltip)
    {
        label.setText(title,juce::dontSendNotification);setName(name+"Cell");
        label.setTooltip(title);
        entry.setName(name);knob.setName(name+"Knob");
        setTitle(title);entry.setTitle(title+" exact value");knob.setTitle(title);
        setValueHelp(tooltip);
        feedback.setName(name+"Feedback");
    }
    void setValueHelp(const juce::String& tooltip)
    {
        entry.setTooltip(tooltip);
        if(validation.isEmpty())entry.setFieldDescription(tooltip);
        knob.setTooltip(onResetInvocation?tooltip+"\nDouble-click to reset. Right-click for reset options.":tooltip);
    }
    void setValueDescription(const juce::String& description){knob.setDescription(description);}
    void setSteppedReadout(bool readout)
    {
        entry.setReadOnly(readout);
        entry.setWantsKeyboardFocus(!readout);
        entry.setInterceptsMouseClicks(!readout,!readout);
    }
    void display(const juce::String& owner,const juce::String& valueText,double fraction,bool enabled,const juce::String& exactText={})
    {
        if(owner!=identity){++resetTicket;cancel();identity=owner;setValidation({});}
        if(!enabled)setValidation({});
        const auto exact=exactText.isEmpty()?valueText:exactText;
        if(exact!=canonicalExact)setValidation({});
        canonical=valueText;canonicalExact=exact;canonicalFraction=std::clamp(fraction,0.0,1.0);
        if(!domainSupplied)domain={canonicalFraction,0,1,.01,1,canonical};
        setEnabled(enabled);
        if(!typing){refreshing=true;entry.setText(canonical,false);if(!active)knob.setValue(canonicalFraction,juce::dontSendNotification);refreshing=false;}knob.repaint();
    }
    void cancel(){if(active && !typing)cancelledGesture=true;active=false;typing=false;restore();if(onEnd)onEnd();}
    bool editing()const noexcept{return active;}
    void setCaption(const juce::String& caption,const juce::String& fullTitle={})
    {
        const auto title=fullTitle.isEmpty()?caption:fullTitle;
        if(getTitle()!=title)setValidation({});
        label.setText(caption,juce::dontSendNotification);label.setTooltip(title);setTitle(title);knob.setTitle(title);entry.setTitle(title+" exact value");
    }
    void setGestureModifiers(const juce::ModifierKeys& mods)
    {
        const auto next=mods.getRawFlags()&(juce::ModifierKeys::ctrlModifier|juce::ModifierKeys::shiftModifier);
        if(next==modifiers)return;modifiers=next;
        rebaseGesture();
    }
    void rebaseGesture(){if(active && !typing){origin=knob.getValue();dragDomainOrigin=domain.value;if(onRebase)onRebase();}}
    void setHomeDetent(HomeDetent detent){homeDetent=detent;knob.repaint();}
    struct DomainValue {double value=0,minimum=0,maximum=1,step=.01,unitsPerFraction=1;juce::String text;};
    void displayDomain(DomainValue value)
    {
        domainValid=std::isfinite(value.value) && std::isfinite(value.minimum) && std::isfinite(value.maximum) && value.minimum<=value.maximum
            && std::isfinite(value.step) && value.step>0 && std::isfinite(value.unitsPerFraction) && value.unitsPerFraction>0;
        domain=domainValid?std::move(value):DomainValue{0,0,1,.01,1,"unavailable"};domainSupplied=true;
    }
    double displayedFraction()const noexcept{return canonicalFraction;}
    void setResetAction(std::function<ResetAction()> prepare)
    {onResetInvocation=std::move(prepare);setValueHelp(entry.getTooltip());}
    ResetAction prepareResetAction()
    {
        if(!isEnabled() || !onResetInvocation)return {"Reset unavailable",{}};
        cancel();auto reset=onResetInvocation();const auto ticket=++resetTicket;const juce::Component::SafePointer<KnobValue> safe(this);
        if(reset.apply){auto action=std::move(reset.apply);reset.apply=[safe,ticket,action=std::move(action)]{if(safe && safe->isEnabled() && safe->resetTicket==ticket)action();};}
        return reset;
    }
    controls::FieldTextEditor& valueField()noexcept{return entry;}
    juce::Slider& valueKnob()noexcept{return knob;}
    std::function<void()> onFeedbackChanged;
    void setInlineHelp(const juce::String& message)
    {
        if(inlineHelp==message)return;
        inlineHelp=message;
        if(onFeedbackChanged)onFeedbackChanged();
    }
    void setValidation(const juce::String& message)
    {
        if(validation==message)return;
        validation=message;
        entry.setInvalid(validation.isNotEmpty());
        entry.setFieldDescription(validation.isNotEmpty()?validation:entry.getTooltip());
        if(onFeedbackChanged)onFeedbackChanged();
    }
    int setValueBounds(juce::Rectangle<int> bounds)
    {
        baseHeight=bounds.getHeight();
        arrangeEntry(bounds.withZeroOrigin());
        bounds.setHeight(std::max(baseHeight,feedback.getBottom()));
        setBounds(bounds);
        return bounds.getHeight();
    }
    // Optional domain intent: absolute accessible writes must not inherit a
    // focused relative gesture clamp. Other consumers keep their delta path.
    std::function<void(double)> onAbsolute;
    void setCompactPresentation(bool enabled)
    {compactPresentation=enabled;if(enabled)entry.setIndents(responsiveGeometry::entryInset,0);lookAndFeelChanged();resized();}
    void lookAndFeelChanged()override
    {entry.setFont(componentTextFont(*this,TextRole::controlValue));applyTextRole(label,TextRole::controlLabel);if(compactPresentation)label.setFont(interFont(controlSize,InterFace::medium));}
    void resized()override
    {
        arrangeEntry(getLocalBounds().withHeight(baseHeight>0?baseHeight:getHeight()));
    }
private:
    void arrangeValueColumn(juce::Rectangle<int> column)
    {
        entry.setBounds(column.withWidth(std::min(column.getWidth(),controlGeometry::valueWidth)));
        const bool invalid=validation.isNotEmpty();
        feedback.arrange(invalid?validation:inlineHelp,invalid,controls::commandBodyBounds(column));
    }
    void arrangeEntry(juce::Rectangle<int> bounds)
    {
        if(compactPresentation)
        {
            label.setBounds(bounds.removeFromTop(responsiveGeometry::captionHeight));
            auto row=bounds.removeFromTop(responsiveGeometry::dialSize);
            knob.setBounds(row.removeFromLeft(responsiveGeometry::dialSize));row.removeFromLeft(spacing::tight);
            arrangeValueColumn(row.withSizeKeepingCentre(row.getWidth(),responsiveGeometry::entryHeight));
            return;
        }
        knob.setBounds(bounds.removeFromLeft(42).withSizeKeepingCentre(42,42));
        bounds.removeFromLeft(spacing::labelHelp);
        label.setBounds(bounds.removeFromTop(controlGeometry::labelHeight));
        arrangeValueColumn(bounds.removeFromTop(controlGeometry::valueHeight));
    }
    bool compactPresentation=false;
    int baseHeight=0;
    void showResetMenu(const juce::MouseEvent& event)
    {
        using namespace controls;
        if (resetMenu || !isEnabled() || !knob.isShowing())
            return;

        const auto point = event.getScreenPosition();
        const juce::Component::SafePointer<KnobValue> safe(this);
        auto reset = prepareResetAction();
        if (!safe || !isEnabled() || !knob.isShowing())
            return;

        std::vector<PopupChoice> rows{{1, reset.label, {}, static_cast<bool>(reset.apply), false, PopupRowRole::action}};
        resetMenu = std::make_unique<ChoicePopup>(knob, std::move(rows),
            [safe, action = std::move(reset.apply)](int result, PopupDismissal why)
            {
                if (!safe || !safe->resetMenu) return;
                const bool showBrackets = safe->resetMenu->showFocusOnReturn();
                safe->resetMenu.reset();
                if (!safe || why == PopupDismissal::outside || why == PopupDismissal::ownerGone
                    || !safe->knob.isShowing() || !safe->isEnabled())
                    return;

                safe->knob.restoreResetMenuFocus(showBrackets);
                if (!safe) return;
                if (why == PopupDismissal::tabForward || why == PopupDismissal::tabBackward)
                    safe->knob.moveKeyboardFocusToSibling(why == PopupDismissal::tabForward);
                if (safe && result == 1 && action) action();
            });
        resetMenu->setName("knobResetMenu");
        juce::SharedResourcePointer<FocusVisibility> visibility;
        visibility->pointerInput();
        resetMenu->show({point.x, point.y, 1, 1}, false);
    }
    void step(double amount,bool fineWheel=false){if(!isEnabled() || !std::isfinite(amount) || amount==0)return;begin(false);if(onStep)onStep(amount,fineWheel);finish(false);}
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
        return std::make_unique<juce::AccessibilityHandler>(component,juce::AccessibilityRole::slider,juce::AccessibilityActions{}
            .addAction(juce::AccessibilityActionType::focus,[this]
            {
                if(isEnabled())knob.focusFromAccessibility();
            }),juce::AccessibilityHandler::Interfaces{std::make_unique<Value>(*this)});
    }
    void begin(bool textEntry)
    {
        if(active && typing==textEntry)return;
        if(active)finish(typing);
        ++resetTicket;
        if(textEntry){typingBaseline=canonicalExact;entry.setText(canonicalExact,false);entry.selectAll();}
        active=true;typing=textEntry;origin=canonicalFraction;dragDomainOrigin=domain.value;if(onBegin)onBegin();
    }
    void restore(){refreshing=true;entry.setText(canonical,false);knob.setValue(canonicalFraction,juce::dontSendNotification);refreshing=false;}
    void finish(bool commit)
    {
        if(committing)return;
        if(!active){if(commit && entry.getText()!=canonical){const auto proposed=entry.getText();begin(true);entry.setText(proposed,false);}else return;}
        if(commit && typing && entry.getText()!=typingBaseline && onCommit)
        {
            const juce::ScopedValueSetter<bool> guard(committing,true);
            onCommit(entry.getText());
        }
        active=false;typing=false;restore();if(onEnd)onEnd();
    }
    Caption label;Dial knob;Entry entry;
    controls::FieldFeedbackLabel feedback;
    juce::String validation,inlineHelp;
    DomainValue domain;int modifiers=0;bool domainSupplied=false,domainValid=true;
    std::function<ResetAction()> onResetInvocation;uint64_t resetTicket=0;
    juce::String identity,canonical,canonicalExact,typingBaseline;
    std::optional<HomeDetent> homeDetent;
    double origin=0,canonicalFraction=0,dragDomainOrigin=0;
    bool refreshing=false,active=false,typing=false,cancelledGesture=false,committing=false;
    std::unique_ptr<controls::ChoicePopup> resetMenu;
};
}
