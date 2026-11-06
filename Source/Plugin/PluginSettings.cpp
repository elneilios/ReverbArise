#include "PluginEditor.h"
#include <Core/BuildInformation.h>

namespace
{
class WorkspaceWindow final:public juce::DocumentWindow
{
public:
    WorkspaceWindow(const juce::String& title,juce::Component& content,int width,int height,std::function<void()> closed,int scale=0)
        :DocumentWindow(title,m3::ui::background,DocumentWindow::closeButton),onClose(std::move(closed)),logicalWidth(width),logicalHeight(height),scalable(scale!=0)
    {
        setUsingNativeTitleBar(true);setResizable(false,false);
        if(scalable)
        {
            setLookAndFeel(&comfortableTheme);
            surface.contentRoot().addAndMakeVisible(content);content.setBounds(0,0,width,height);
            viewport.setName("auxiliaryPresentationViewport");viewport.setFocusRevealEnabled(true);viewport.setViewedComponent(&surface,false);
            setContentNonOwned(&viewport,false);setPresentationScale(scale);
        }
        else{setContentNonOwned(&content,false);setSize(width,height);}
    }
    ~WorkspaceWindow()override{setLookAndFeel(nullptr);}
    void closeButtonPressed()override{setVisible(false);if(onClose)onClose();}
    void setPresentationScale(int scale)
    {
        if(!scalable)return;
        const auto* display=juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());
        if(!display)return;
        const auto work=display->userBounds.toNearestInt();juce::BorderSize<int> frame;
        if(const auto* peer=getPeer())if(const auto measured=peer->getFrameSizeIfPresent())frame=*measured;
        if(scale==lastScale && work==lastWork && frame==lastFrame)return;
        lastScale=scale;lastWork=work;lastFrame=frame;
        const auto width=(logicalWidth*scale+99)/100,height=(logicalHeight*scale+99)/100;
        surface.present(logicalWidth,logicalHeight,scale);surface.setSize(width,height);
        const auto border=getContentComponentBorder();
        const juce::Rectangle<int> available{0,0,std::max(1,work.getWidth()-frame.getLeftAndRight()-border.getLeftAndRight()),std::max(1,work.getHeight()-frame.getTopAndBottom()-border.getTopAndBottom())};
        const auto scroll=viewport.getScrollBarThickness();
        setSize(std::min(width+(height>available.getHeight()?scroll:0),available.getWidth())+border.getLeftAndRight(),std::min(height+(width>available.getWidth()?scroll:0),available.getHeight())+border.getTopAndBottom());
        if(isVisible())setTopLeftPosition(juce::jlimit(work.getX()+frame.getLeft(),std::max(work.getX()+frame.getLeft(),work.getRight()-getWidth()-frame.getRight()),getX()),
            juce::jlimit(work.getY()+frame.getTop(),std::max(work.getY()+frame.getTop(),work.getBottom()-getHeight()-frame.getBottom()),getY()));
    }
private:
    std::function<void()> onClose;
    int logicalWidth,logicalHeight,lastScale=0;bool scalable;
    juce::Rectangle<int> lastWork;juce::BorderSize<int> lastFrame;
    m3::ui::controls::PresentationSurface surface;
    m3::ui::controls::FocusViewport viewport;
    m3::ui::WorkspaceTheme comfortableTheme;
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
        settingsWindow=std::make_unique<WorkspaceWindow>("ReverbArise - Settings",settingsTabs,640,570,[]{},responsivePresentation?presentation->effective().effective.scalePercent:0);
        settingsWindow->setName("settingsWindow");if(!responsivePresentation)settingsWindow->setLookAndFeel(&workspaceTheme);
        settingsWindow->addMouseListener(this,true);
        settingsWindow->centreAroundComponent(this,settingsWindow->getWidth(),settingsWindow->getHeight());
    }
    if(settingsWindow){settingsWindow->setVisible(open);if(open){refresh();updateAuxiliaryPresentation();settingsWindow->toFront(true);}}
}
void PreverbAudioProcessorEditor::setDiagnosticsOpen(bool open)
{
    if(destroying)return;
    diagnosticsOpen=open;
    if(open)diagnosticsRefreshedAt=-1;
    if(!diagnosticsWindow && open)
    {
        diagnosticsWindow=std::make_unique<WorkspaceWindow>("ReverbArise - Diagnostics",diagnosticsViewport,1000,650,
            [safe=juce::Component::SafePointer<PreverbAudioProcessorEditor>(this)]{if(safe)safe->diagnosticsOpen=false;},responsivePresentation?presentation->effective().effective.scalePercent:0);
        diagnosticsWindow->setName("diagnosticsWindow");if(!responsivePresentation)diagnosticsWindow->setLookAndFeel(&workspaceTheme);
        diagnosticsWindow->addMouseListener(this,true);
        diagnosticsWindow->centreAroundComponent(this,diagnosticsWindow->getWidth(),diagnosticsWindow->getHeight());
    }
    if(diagnosticsWindow){diagnosticsWindow->setVisible(open);if(open){refresh();updateAuxiliaryPresentation();diagnosticsWindow->toFront(true);}}
}
void PreverbAudioProcessorEditor::updateAuxiliaryPresentation()
{
    if(!presentation)return;const auto scale=presentation->effective().effective.scalePercent;
    for(auto* window:{settingsWindow.get(),diagnosticsWindow.get()})
        if(auto* workspace=dynamic_cast<WorkspaceWindow*>(window))workspace->setPresentationScale(scale);
}
void PreverbAudioProcessorEditor::initialiseSettings()
{
    settingsTabs.getProperties().set("contextHelp","Choose a settings tab. These settings apply to this plugin instance.");
    settingsTabs.setName("settingsTabs");settingsTabs.setTabBarDepth(36);
    settingsTabs.setColour(juce::TabbedComponent::backgroundColourId,m3::ui::background);
    settingsTabs.setColour(juce::TabbedComponent::outlineColourId,juce::Colours::transparentBlack);
    const juce::StringArray titles{"Playback","Processing","Swell","Audition","Recovery","About"};
    for(int i=0;i<6;++i)
    {
        settingsPages[i].setName("settings"+titles[i]);
        settingsTabs.addTab(titles[i],m3::ui::background,&settingsPages[i],false);
        settingsPages[i].addAndMakeVisible(settingsHelp[i]);
        settingsHelp[i].setColour(juce::Label::textColourId,m3::ui::secondary);settingsHelp[i].setFont(juce::FontOptions(m3::ui::bodySize));
        settingsHelp[i].setJustificationType(juce::Justification::topLeft);
    }
    const auto recoveryRoot=processor.swellEngine().assetDirectory().getParentDirectory().getParentDirectory();
    storageInventory=std::make_unique<m3::ui::PortableStoragePanel>([this](auto& input,auto& error){return processor.pinPortableAssets(input,error);},recoveryRoot);
    storageViewport.setName("storageViewport");storageViewport.setFocusRevealEnabled(true);
    storageViewport.setViewedComponent(storageInventory.get(),false);
    settingsTabs.addTab("Storage",m3::ui::background,&storageViewport,false);
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
    settingsHelp[5].setText("Include this information when reporting a problem.\nIt identifies the running plugin; no project or audio data is copied.",juce::dontSendNotification);
    m3::ui::applyTextRole(settingsHelp[5],m3::ui::TextRole::helper,juce::Justification::topLeft);
    aboutHeading.setName("aboutReverbArise");aboutHeading.setText("About ReverbArise",juce::dontSendNotification);
    m3::ui::applyTextRole(aboutHeading,m3::ui::TextRole::sectionHeading);settingsPages[5].addAndMakeVisible(aboutHeading);
    const juce::StringArray buildCaptions{"Product version","Build identifier","Git commit"};
    const juce::StringArray buildValues{juce::String(m3::build::version.data()),juce::String(m3::build::identifier.data()),
        juce::String(m3::build::gitSha.data())+(m3::build::worktreeDirty?" (modified)":"")};
    for(size_t i=0;i<buildInformationLabels.size();++i)
    {
        auto& label=buildInformationLabels[i];auto& value=buildInformationValues[i];
        label.setText(buildCaptions[static_cast<int>(i)],juce::dontSendNotification);m3::ui::applyTextRole(label,m3::ui::TextRole::controlLabel);
        value.setName("aboutBuildValue"+juce::String(static_cast<int>(i)));value.setText(buildValues[static_cast<int>(i)],juce::dontSendNotification);
        value.setTitle(buildCaptions[static_cast<int>(i)]);value.setDescription(value.getText());value.setTooltip(value.getText());
        m3::ui::applyTextRole(value,m3::ui::TextRole::helper);
        settingsPages[5].addAndMakeVisible(label);settingsPages[5].addAndMakeVisible(value);
    }
    copyBuildInformation.setName("copyBuildInformation");copyBuildInformation.setTooltip("Copy product version, build identifier, full Git commit and source status for a bug report");
    settingsHelp[5].setName("buildInformationCopyStatus");
    copyBuildInformation.onClick=[this]{
        buildInformationWriter(juce::String(m3::build::report()));
        settingsHelp[5].setText("Build information copied to clipboard.\nPaste it into your bug report; no project or audio data was copied.",juce::dontSendNotification);
        settingsHelp[5].setDescription(settingsHelp[5].getText());
        if(auto* handler=settingsHelp[5].getAccessibilityHandler())handler->notifyAccessibilityEvent(juce::AccessibilityEvent::valueChanged);
    };
    settingsPages[5].addAndMakeVisible(copyBuildInformation);
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
    using namespace m3::ui;
    aboutHeading.setBounds(left,spacing::section,width,controlGeometry::headingHeight);
    auto aboutY=aboutHeading.getBottom()+spacing::section;
    for(size_t i=0;i<buildInformationLabels.size();++i)
    {
        buildInformationLabels[i].setBounds(left,aboutY,labelWidth,controlGeometry::labelHeight);
        buildInformationValues[i].setBounds(left+labelWidth,aboutY,width-labelWidth,controlGeometry::labelHeight);
        aboutY+=controlGeometry::labelHeight+spacing::related;
    }
    copyBuildInformation.setBounds(left,aboutY+spacing::related,width,controlGeometry::height);
    settingsHelp[5].setBounds(left,copyBuildInformation.getBottom()+spacing::section,width,2*learnTypography::secondaryLineHeight);
}

void PreverbAudioProcessorEditor::refreshStorageScope(const CaptureSessionSnapshot& capture,uint64_t epoch)
{
    const auto identity=juce::String(capture.manifest.projectId)+"/"+juce::String(capture.manifest.instanceId)+"/"+juce::String(static_cast<juce::int64>(epoch));
    if(storageInventory)
    {
        if(identity!=storageIdentity){storageIdentity=identity;storageInventory->bind([this](auto& input,auto& error){return processor.pinPortableAssets(input,error);});}
        storageInventory->setSize(std::max(1,storageViewport.getWidth()-storageViewport.getScrollBarThickness()),
            std::max(1,storageViewport.getHeight()));
        storageInventory->poll();
    }
}