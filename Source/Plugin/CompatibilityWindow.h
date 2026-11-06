#pragma once
#include "PluginProcessor.h"
#include <UI/Controls/ActionButton.h>

#if PREVERB_COMPATIBILITY_TEST
class CompatibilityWindow final:public juce::DocumentWindow,private juce::Timer
{
public:
    explicit CompatibilityWindow(PreverbAudioProcessor& p):DocumentWindow("ReverbArise Compatibility Test",m3::ui::background,closeButton),processor(p)
    {
        setUsingNativeTitleBar(true);setResizable(true,false);setResizeLimits(620,460,1200,1000);
        setContentNonOwned(&content,false);content.addAndMakeVisible(status);content.addAndMakeVisible(copy);content.addAndMakeVisible(clear);content.addAndMakeVisible(feedback);
        status.setMultiLine(true);status.setReadOnly(true);status.setScrollbarsShown(true);status.setCaretVisible(false);
        copy.onClick=[this]{copyReport();};clear.onClick=[this]{clearReport();};
        setSize(760,540);refresh();startTimerHz(4);
    }
    ~CompatibilityWindow()override{stopTimer();clearContentComponent();}
    void closeButtonPressed()override{setVisible(false);}
    void show(){refresh();setVisible(true);toFront(false);}
    void resized()override
    {
        DocumentWindow::resized();auto a=content.getLocalBounds().reduced(m3::ui::spacing::group);
        auto buttons=a.removeFromBottom(36);copy.setBounds(buttons.removeFromLeft(160));buttons.removeFromLeft(12);clear.setBounds(buttons.removeFromLeft(160));
        feedback.setBounds(a.removeFromBottom(48));status.setBounds(a);
    }
    void refresh(){status.setText(processor.compatibilityReport(false),false);}
    void clearReport(){processor.clearCompatibilityReport();feedback.setText("Report cleared. Captured source and saved data retained.",juce::dontSendNotification);refresh();}
    void copyReport()
    {
        const auto report=processor.compatibilityReport();juce::SystemClipboard::copyTextToClipboard(report);
        feedback.setText(juce::SystemClipboard::getTextFromClipboard()==report?"Report copied. Send this text to Neil.":"Clipboard copy failed. Try Copy report again.",juce::dontSendNotification);
    }
    juce::String feedbackText()const{return feedback.getText();}
private:
    void timerCallback()override{refresh();}
    PreverbAudioProcessor& processor;juce::Component content;juce::TextEditor status;
    m3::ui::controls::ActionButton copy{"Copy report",m3::ui::controls::CommandIcon::none,true},clear{"Clear report"};
    juce::Label feedback;
};
#endif
