#include "PluginEditor.h"

namespace
{
class WorkspaceWindow final:public juce::DocumentWindow
{
public:
    WorkspaceWindow(const juce::String& title,juce::Component& content,int width,int height,std::function<void()> closed)
        :DocumentWindow(title,m3::ui::background,DocumentWindow::closeButton),onClose(std::move(closed))
    {
        setUsingNativeTitleBar(true);setResizable(false,false);
        setContentNonOwned(&content,false);setSize(width,height);
    }
    void closeButtonPressed()override{setVisible(false);if(onClose)onClose();}
private:std::function<void()> onClose;
};
}

juce::Rectangle<int> PreverbAudioProcessorEditor::swellMenuAnchor(const juce::MouseEvent& event)const
{
    // A row event's screen position already includes list scrolling, editor
    // embedding and the peer's desktop scale. Never anchor to the whole list.
    return m3::ui::controls::SwellList::menuAnchor(event);
}
void PreverbAudioProcessorEditor::setSettingsOpen(bool open)
{
    if(destroying)return;
    if(!settingsWindow && open)
    {
        settingsWindow=std::make_unique<WorkspaceWindow>("ReverbArise - Settings",settingsTabs,640,570,[]{});
        settingsWindow->setName("settingsWindow");settingsWindow->setLookAndFeel(&workspaceTheme);
        settingsWindow->centreAroundComponent(this,640,570);
    }
    if(settingsWindow){settingsWindow->setVisible(open);if(open){refresh();settingsWindow->toFront(true);}}
}
void PreverbAudioProcessorEditor::setDiagnosticsOpen(bool open)
{
    if(destroying)return;
    diagnosticsOpen=open;
    if(open)diagnosticsRefreshedAt=-1;
    if(!diagnosticsWindow && open)
    {
        diagnosticsWindow=std::make_unique<WorkspaceWindow>("ReverbArise - Diagnostics",diagnosticsViewport,1000,650,
            [safe=juce::Component::SafePointer<PreverbAudioProcessorEditor>(this)]{if(safe)safe->diagnosticsOpen=false;});
        diagnosticsWindow->setName("diagnosticsWindow");diagnosticsWindow->setLookAndFeel(&workspaceTheme);
        diagnosticsWindow->centreAroundComponent(this,1000,650);
    }
    if(diagnosticsWindow){diagnosticsWindow->setVisible(open);if(open){refresh();diagnosticsWindow->toFront(true);}}
}
void PreverbAudioProcessorEditor::initialiseSettings()
{
    settingsTabs.setName("settingsTabs");settingsTabs.setTabBarDepth(36);
    settingsTabs.setColour(juce::TabbedComponent::backgroundColourId,m3::ui::background);
    settingsTabs.setColour(juce::TabbedComponent::outlineColourId,juce::Colours::transparentBlack);
    const juce::StringArray titles{"Playback","Processing","Swell","Audition","Recovery"};
    for(int i=0;i<5;++i)
    {
        settingsPages[i].setName("settings"+titles[i]);
        settingsTabs.addTab(titles[i],m3::ui::background,&settingsPages[i],false);
        settingsPages[i].addAndMakeVisible(settingsHelp[i]);
        settingsHelp[i].setColour(juce::Label::textColourId,m3::ui::secondary);settingsHelp[i].setFont(juce::FontOptions(m3::ui::bodySize));
        settingsHelp[i].setJustificationType(juce::Justification::topLeft);
    }
    const auto move=[&](int page,std::initializer_list<juce::Component*> controls)
    {for(auto* c:controls)settingsPages[page].addAndMakeVisible(c);};
    move(0,{&gainPolicyLabel,&gainPolicyBox,&liveButton,&liveRoutingBox,&liveLevel,&liveLevelLabel});
    move(1,{&mappingButton,&automaticButton,&autoButton,&separationLabel,&separationSlider,&defaultIRButton});
    move(2,{&muteButton,&fullButton,&inCurveBox,&outCurveBox,&sampleUnitsButton,&barStartLabel,&barStartEditor,&barEndLabel,&barEndEditor,&setBarsButton,&removeButton});
    move(3,{&isolateButton,&isolateDryButton,&tailLabel,&tailSlider,&auditionLabel,&auditionSlider});
    move(4,{&generateButton,&allButton,&analyzeButton,&confirmButton,&candidateBox,&recaptureButton,&cancelButton,&recaptureStatus,&clearButton,&suppressionButton,&toneButton,&toneStartLabel,&toneStartSlider,&toneEndLabel,&toneEndSlider,&openDiagnosticsButton});
    const juce::StringArray captions{"Routing","Default IR","Fade-in curve","Fade-out curve","Source range (bars)","Audition scope","Manual preparation","Onset suggestions","Fresh capture","Test tone range","Details",""};
    const int labelPages[]{0,1,2,2,2,3,4,4,4,4,4,4};
    for(int i=0;i<12;++i){settingsLabels[i].setText(captions[i],juce::dontSendNotification);settingsLabels[i].setFont(juce::FontOptions(m3::ui::bodySize));settingsPages[labelPages[i]].addAndMakeVisible(settingsLabels[i]);}
    settingsHelp[0].setText("Musical gain matches source intensity, then adds the approved +6 dB target.\nLevel 0 dB is that starting balance. Legacy preserves older projects.\nLive Wet also sets new audition wet gain; legacy blend/trim stay separate.",juce::dontSendNotification);
    settingsHelp[1].setText("Both sensitivities stay in the main workspace.\nThe default IR applies to future swells; existing swells keep their own IR.",juce::dontSendNotification);
    settingsHelp[2].setText("Applies to the selected swell. Rename from its three-dot list menu.\nSource, envelope, Enabled, Solo and Freeze stay in the inspector.",juce::dontSendNotification);
    settingsHelp[3].setText("Audition loops captured audio. Isolation/tail affect its listening window.\nWet only uses Live Wet gain; stored legacy trim is inactive in live-level mode.",juce::dontSendNotification);
    settingsHelp[4].setText("Recovery actions run only when clicked. Clear map keeps swell assets.\nReset deleted targets allows automatic rediscovery.",juce::dontSendNotification);
    openDiagnosticsButton.setName("openDiagnostics");openDiagnosticsButton.onClick=[this]{setDiagnosticsOpen(true);};
    toneButton.setTooltip("Explicit scheduled 440 Hz test tone. Turn off here after testing.");
}
void PreverbAudioProcessorEditor::layoutSettings()
{
    // Fixed logical dialog width keeps every field visible without horizontal
    // scrolling, independent of the plugin editor's current dimensions.
    const int left=24,width=590,labelWidth=175;
    const auto row=[&](juce::Component& label,juce::Component& value,int y)
    {label.setBounds(left,y,labelWidth,32);value.setBounds(left+labelWidth,y,width-labelWidth,32);};
    row(gainPolicyLabel,gainPolicyBox,24);liveButton.setBounds(left,68,width,30);row(settingsLabels[0],liveRoutingBox,108);row(liveLevelLabel,liveLevel,150);
    mappingButton.setBounds(left,24,width,30);automaticButton.setBounds(left,64,width,30);autoButton.setBounds(left,104,width,30);
    row(separationLabel,separationSlider,150);row(settingsLabels[1],defaultIRButton,196);
    muteButton.setBounds(left,20,150,30);sampleUnitsButton.setBounds({});sampleUnitsButton.setVisible(false); // Retained internal exact-coordinate diagnostic seam.
    row(settingsLabels[2],inCurveBox,60);row(settingsLabels[3],outCurveBox,100);
    settingsLabels[4].setBounds(left,146,width,24);barStartLabel.setBounds(left,178,70,30);barStartEditor.setBounds(left+75,178,90,30);
    barEndLabel.setBounds(left+185,178,70,30);barEndEditor.setBounds(left+260,178,90,30);setBarsButton.setBounds(left+365,178,225,30);
    fullButton.setBounds(left,230,220,32);removeButton.setBounds(left+240,230,160,32);
    settingsLabels[5].setBounds(left,20,width,24);isolateButton.setBounds(left,56,250,30);isolateDryButton.setBounds(left+265,56,300,30);
    row(tailLabel,tailSlider,106);row(auditionLabel,auditionSlider,152);
    settingsLabels[6].setBounds(left,12,160,26);generateButton.setBounds(left+175,12,200,30);allButton.setBounds(left+390,12,200,30);
    settingsLabels[7].setBounds(left,52,160,26);analyzeButton.setBounds(left+175,52,200,30);confirmButton.setBounds(left+390,52,200,30);
    candidateBox.setBounds(left+175,90,415,30);
    settingsLabels[8].setBounds(left,132,160,26);recaptureButton.setBounds(left+175,132,200,30);cancelButton.setBounds(left+390,132,200,30);
    recaptureStatus.setBounds(left,170,width,38);
    clearButton.setBounds(left,214,220,30);suppressionButton.setBounds(left+240,214,350,30);
    toneButton.setBounds(left,258,width,30);
    toneStartLabel.setBounds(left,300,90,30);toneStartSlider.setBounds(left+95,300,190,30);toneEndLabel.setBounds(left+300,300,100,30);toneEndSlider.setBounds(left+405,300,185,30);
    openDiagnosticsButton.setBounds(left,346,220,32);
    settingsLabels[9].setBounds({});settingsLabels[10].setBounds({});settingsLabels[11].setBounds({});
    for(int i=0;i<5;++i)settingsHelp[i].setBounds(left,i==4?394:310,width,70);
}
