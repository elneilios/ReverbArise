#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Real, discoverable native owner for interactive component fixtures. Untitled
// borderless peers can receive JUCE focus while Windows keeps another process
// active, and cannot be selected reliably by the desktop validation runner.
class NativeEditorPeer final:public juce::DocumentWindow
{
public:
    NativeEditorPeer(juce::Component& content,const juce::String& title)
        :DocumentWindow(title,juce::Colour{},juce::DocumentWindow::closeButton)
    {
        setUsingNativeTitleBar(true);setContentNonOwned(&content,true);
        centreWithSize(getWidth(),getHeight());setVisible(true);toFront(true);
    }
    ~NativeEditorPeer()override{setVisible(false);clearContentComponent();}
    void closeButtonPressed()override{} // Assertions/RAII own fixture lifetime.
};
