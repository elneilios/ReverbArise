#pragma once
#include "SurfaceLifecycle.h"
#include <UI/WorkspaceTheme.h>
#include "ActionButton.h"

namespace m3::ui::controls
{
// Retain AlertWindow's measured content/custom controls with native close chrome.
class OwnedAlertWindow:public juce::AlertWindow
{
public:
    OwnedAlertWindow(const juce::String& title,const juce::String& message,juce::MessageBoxIconType icon,juce::Component* owner)
        :juce::AlertWindow(title,message,icon,owner),openingOwner(owner){setLookAndFeel(&theme);}
    ~OwnedAlertWindow()override{if(actionsAdded)removeCustomComponent(getNumCustomComponents()-1);setLookAndFeel(nullptr);}
    void finish(int result)
    {if(!isCurrentlyModal())return;exitModalState(result);setVisible(false);}
    void userTriedToCloseWindow()override{finish(0);}
    bool keyPressed(const juce::KeyPress& key)override
    {if(key.isKeyCode(juce::KeyPress::escapeKey)){finish(0);return true;}return juce::AlertWindow::keyPressed(key);}
    void showNative(juce::Component& owner)
    {
        setUsingNativeTitleBar(true);addToDesktop(getDesktopWindowStyleFlags());
        centreAroundComponent(&owner,getWidth(),getHeight());setVisible(true);toFront(true);
    }
    void showNative(){if(openingOwner)showNative(*openingOwner);}
    void addAction(const juce::String& label,int result,bool initial=false)
    {
        auto* button=actionButtons.add(new ActionButton(label));actionRow.addAndMakeVisible(button);
        button->onClick=[this,result]{finish(result);};
        button->setBounds((actionButtons.size()-1)*(164+spacing::related),0,164,controlGeometry::height+2*static_cast<int>(controlGeometry::focusPadding));
        actionRow.setSize(actionButtons.size()*164+(actionButtons.size()-1)*spacing::related,button->getHeight());
        if(initial)initialAction=button;
    }
    void showActions(juce::Component& owner)
    {
        if(!actionsAdded){addCustomComponent(&actionRow);actionsAdded=true;}
        showNative(owner);
    }
    void focusInitialAction(){if(initialAction)initialAction->grabKeyboardFocus();}
private:
    struct Theme:WorkspaceTheme
    {
        Theme()
        {
            setColour(juce::AlertWindow::backgroundColourId,m3::ui::background);
            setColour(juce::AlertWindow::textColourId,m3::ui::text);
            setColour(juce::AlertWindow::outlineColourId,m3::ui::outline);
        }
        juce::Font getAlertWindowTitleFont()override{return textFont(TextRole::sectionHeading);}
        juce::Font getAlertWindowMessageFont()override{return textFont(TextRole::helper);}
        int getAlertBoxWindowFlags()override{return juce::ComponentPeer::windowHasTitleBar|juce::ComponentPeer::windowHasCloseButton|juce::ComponentPeer::windowHasDropShadow;}
    } theme;
    int getDesktopWindowStyleFlags()const override
    {return juce::ComponentPeer::windowHasTitleBar|juce::ComponentPeer::windowHasCloseButton|juce::ComponentPeer::windowHasDropShadow;}
    juce::Component::SafePointer<juce::Component> openingOwner;
    juce::Component actionRow;
    juce::OwnedArray<ActionButton> actionButtons;
    ActionButton* initialAction=nullptr;
    bool actionsAdded=false;
};
}
