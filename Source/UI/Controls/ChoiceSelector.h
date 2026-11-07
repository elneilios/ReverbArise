#pragma once
#include "DeliberateSelector.h"
#include "ChoicePopup.h"
#include <map>

namespace m3::ui::controls
{
// Explicit C2 adoption seam. Existing DeliberateSelector consumers stay unchanged
// until the native family checkpoint; the existing ComboBox binding API survives.
class ChoiceSelector:public DeliberateSelector
{
public:
    ChoiceSelector(){setWantsKeyboardFocus(true);setMouseClickGrabsKeyboardFocus(true);hideNativeLabel();}
    ~ChoiceSelector()override{hidePopup();}
    void setChoiceReason(int id,juce::String reason){if(reasons[id]!=reason){hidePopup();reasons[id]=std::move(reason);}}
    void addItem(const juce::String& text,int id){hidePopup();juce::ComboBox::addItem(text,id);}
    void clear(juce::NotificationType notification=juce::sendNotificationAsync)
    {hidePopup();reasons.clear();juce::ComboBox::clear(notification);}
    void setItemEnabled(int id,bool enabled){if(isItemEnabled(id)!=enabled)hidePopup();juce::ComboBox::setItemEnabled(id,enabled);}
    void setSelectedId(int id,juce::NotificationType notification=juce::sendNotificationAsync)
    {if(getSelectedId()!=id)hidePopup();const juce::Component::SafePointer<ChoiceSelector> safe(this);juce::ComboBox::setSelectedId(id,notification);if(safe)repaint();}
    void setSelectedItemIndex(int index,juce::NotificationType notification=juce::sendNotificationAsync)
    {setSelectedId(getItemId(index),notification);}
    bool isPopupActive()const noexcept{return popup!=nullptr && !popup->state().done();}
    ChoicePopup* activePopup()const noexcept{return popup.get();}
    bool isFocusVisible()const noexcept{return visibility->isVisible() && (hasKeyboardFocus(true) || isPopupActive());}
    juce::String getTooltip()override{return juce::ComboBox::getTooltip()+"\n"+getText();}
    void hidePopup(){++generation;if(popup)popup->dismiss();popup.reset();juce::ComboBox::hidePopup();repaint();}
    void showPopup()override
    {
        if(!isEnabled() || !isShowing() || popup!=nullptr)return;
        const juce::Component::SafePointer<ChoiceSelector> safe(this);
        const auto prepare=beforeInvocation;if(prepare)prepare();
        if(safe==nullptr || !isEnabled() || !isShowing())return;
        std::vector<PopupChoice> snapshot;
        for(int index=0;index<getNumItems();++index)
        {
            const auto id=getItemId(index);if(id==0)continue;
            const auto eligible=isItemEnabled(id);
            const auto reason=reasons.contains(id)?reasons.at(id):eligible?juce::String{}:"Unavailable for the current state";
            snapshot.push_back({id,getItemText(index),reason,eligible,id==getSelectedId()});
        }
        const auto ticket=++generation;
        const auto accepted=getSelectedId();
        popup=std::make_unique<ChoicePopup>(*this,snapshot,[safe,ticket,accepted,snapshot](int id,PopupDismissal why)
        {
            if(safe==nullptr || safe->generation!=ticket)return;
            bool eligible=safe->isEnabled() && safe->isShowing() && safe->getSelectedId()==accepted;
            for(const auto& row:snapshot)
                if(safe->getItemText(safe->indexOfItemId(row.id))!=row.label || safe->isItemEnabled(row.id)!=row.eligible)eligible=false;
            safe->popup.reset();if(safe==nullptr)return;safe->repaint();
            if(why==PopupDismissal::tabForward || why==PopupDismissal::tabBackward)
            {safe->visibility->keyboardInput();safe->grabKeyboardFocus();if(safe)safe->moveKeyboardFocusToSibling(why==PopupDismissal::tabForward);}
            else if(why!=PopupDismissal::outside && why!=PopupDismissal::ownerGone && safe->isShowing() && safe->isEnabled())safe->grabKeyboardFocus();
            if(id!=0 && eligible && safe!=nullptr && safe->isItemEnabled(id))safe->setSelectedId(id,juce::sendNotificationSync);
        });
        popup->show(localAreaToGlobal(commandBodyBounds(getLocalBounds())),visibility->isVisible());repaint();
    }
    void paint(juce::Graphics& g)override
    {
        SelectorPresentation::surface(g,getLocalBounds(),{isEnabled(),over,held && over,isFocusVisible(),false});
        const auto alpha=isEnabled()?1.f:.35f;auto area=commandBodyBounds(getLocalBounds()).reduced(spacing::labelHelp,0);
        const auto arrow=area.removeFromRight(controlGeometry::numericStepWidth).toFloat().getCentre();
        juce::Path chevron;chevron.startNewSubPath(arrow.x-5,arrow.y-2);chevron.lineTo(arrow.x,arrow.y+2);chevron.lineTo(arrow.x+5,arrow.y-2);
        g.setColour(text.withMultipliedAlpha(alpha));g.strokePath(chevron,juce::PathStrokeType(rule*2));
        area.removeFromRight(spacing::tight);g.setFont(componentTextFont(*this,TextRole::controlValue));
        g.drawText(getText().isNotEmpty()?getText():getTextWhenNothingSelected(),area,juce::Justification::centredLeft,true);
    }
    void resized()override{juce::ComboBox::resized();hideNativeLabel();}
    void lookAndFeelChanged()override{juce::ComboBox::lookAndFeelChanged();hideNativeLabel();}
    void enablementChanged()override{if(!isEnabled())hidePopup();juce::ComboBox::enablementChanged();repaint();}
    void focusGained(FocusChangeType cause)override
    {if(cause==focusChangedByMouseClick)visibility->pointerInput();else if(cause==focusChangedByTabKey)visibility->keyboardInput();repaint();}
    void focusLost(FocusChangeType)override{held=false;repaint();}
    void mouseMove(const juce::MouseEvent& event)override{over=commandBodyBounds(getLocalBounds()).contains(event.getPosition());repaint();}
    void mouseEnter(const juce::MouseEvent& event)override{mouseMove(event);}
    void mouseExit(const juce::MouseEvent&)override{over=false;repaint();}
    void mouseDown(const juce::MouseEvent& event)override
    {if(!isEnabled() || !event.mods.isLeftButtonDown())return;visibility->pointerInput();held=true;over=commandBodyBounds(getLocalBounds()).contains(event.getPosition());grabKeyboardFocus();repaint();}
    void mouseDrag(const juce::MouseEvent& event)override{mouseMove(event);}
    void mouseUp(const juce::MouseEvent& event)override
    {const bool open=held && isEnabled() && commandBodyBounds(getLocalBounds()).contains(event.getPosition());held=false;repaint();if(open){if(isPopupActive())hidePopup();else showPopup();}}
    bool keyPressed(const juce::KeyPress& key)override
    {
        visibility->keyboardInput();repaint();if(!isEnabled())return false;
        if(popup!=nullptr){if(!popup->state().done())return popup->keyPressed(key);return true;}
        const auto code=key.getKeyCode();
        if(code==juce::KeyPress::returnKey || code==juce::KeyPress::spaceKey || code==juce::KeyPress::downKey || code==juce::KeyPress::upKey || code==juce::KeyPress::homeKey || code==juce::KeyPress::endKey)
        {showPopup();if(popup && (code==juce::KeyPress::homeKey || code==juce::KeyPress::endKey))popup->keyPressed(key);return true;}
        return juce::Component::keyPressed(key);
    }
    bool keyStateChanged(bool)override{return false;}
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override
    {
        class Value final:public juce::AccessibilityTextValueInterface
        {
        public:explicit Value(ChoiceSelector& selector):owner(selector){}
            bool isReadOnly()const override{return true;}
            juce::String getCurrentValueAsString()const override{return owner.getText();}
            void setValueAsString(const juce::String&)override{}
        private:ChoiceSelector& owner;
        };
        class Handler final:public juce::AccessibilityHandler
        {
        public:
            explicit Handler(ChoiceSelector& selector):juce::AccessibilityHandler(selector,juce::AccessibilityRole::comboBox,
                juce::AccessibilityActions{}
                    .addAction(juce::AccessibilityActionType::focus,[&selector]{selector.visibility->accessibilityInput();selector.grabKeyboardFocus();selector.repaint();})
                    .addAction(juce::AccessibilityActionType::showMenu,[&selector]{selector.visibility->accessibilityInput();selector.showPopup();})
                    .addAction(juce::AccessibilityActionType::press,[&selector]{selector.visibility->accessibilityInput();selector.showPopup();}),
                Interfaces{std::make_unique<Value>(selector)}),owner(selector){}
            juce::AccessibleState getCurrentState()const override
            {auto state=juce::AccessibilityHandler::getCurrentState().withExpandable();return owner.isPopupActive()?state.withExpanded():state.withCollapsed();}
            juce::String getDescription()const override{return owner.getDescription()+"\n"+owner.getText();}
            juce::String getHelp()const override{return owner.getTooltip();}
        private:ChoiceSelector& owner;
        };
        return std::make_unique<Handler>(*this);
    }
private:
    void hideNativeLabel(){for(auto* child:getChildren())if(dynamic_cast<juce::Label*>(child))child->setVisible(false);}
    std::map<int,juce::String> reasons;
    std::unique_ptr<ChoicePopup> popup;
    juce::SharedResourcePointer<FocusVisibility> visibility;
    uint64_t generation=0;
    bool over=false,held=false;
};
}
