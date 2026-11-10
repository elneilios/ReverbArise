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
void PreverbAudioProcessorEditor::showRegenerationResult(const m3::RegenerationResult& result)
{
    regenerationFeedbackSession=result.sessionEpoch;
    juce::String text;
    if(!result.accepted)text="Not queued: regeneration requires Perform mode.";
    else
    {
        text=result.queued ? "Queued "+juce::String(static_cast<int>(result.queued))+" for regeneration." : "No eligible swells to queue.";
        if(result.frozen)text+=" Skipped "+juce::String(static_cast<int>(result.frozen))+" frozen.";
        if(result.importing)text+=" Skipped "+juce::String(static_cast<int>(result.importing))+" importing IR.";
    }
    regenerationFeedback.setText(text,juce::dontSendNotification);
    regenerationFeedback.setDescription(text);
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
    storageInventory=std::make_unique<m3::ui::PortableStoragePanel>([this](auto& input,auto& error){return processor.pinPortableAssets(input,error,true);},recoveryRoot);
    auto* storageOwner=&processor;
    storageInventory->enableWorkingFolder(processor.assetRegistryRoot(),[storageOwner](const auto& folder,const auto& input,auto& error,const auto& hooks){return storageOwner->useWorkingFolder(folder,input,error,hooks);});
    storageInventory->onRestored=[this](const auto& input,auto& error){return processor.reloadRecoveredAssets(input,error);};
    storageViewport.setName("storageViewport");storageViewport.setFocusRevealEnabled(true);
    storageViewport.setViewedComponent(storageInventory.get(),false);
    settingsTabs.addTab("Storage",m3::ui::background,&storageViewport,false);
    const auto move=[&](int page,std::initializer_list<juce::Component*> controls)
    {for(auto* c:controls)settingsPages[page].addAndMakeVisible(c);};
    move(0,{&gainPolicyLabel,&gainPolicyBox,&liveButton,&liveRoutingBox,&liveLevel,&liveLevelLabel});
    move(1,{&mappingButton,&automaticButton,&autoButton,&separationLabel,&separationSlider,&defaultIRButton});
    move(2,{&muteButton,&fullButton,&inCurveBox,&outCurveBox,&sampleUnitsButton,&barStartLabel,&barStartEditor,&barEndLabel,&barEndEditor,&setBarsButton,&removeButton,&sourceRangeFeedback});
    sourceRangeFeedback.setName("sourceRangeFeedback");
    for(auto* caption:{&barStartLabel,&barEndLabel})
    {
        m3::ui::applyTextRole(*caption,m3::ui::TextRole::controlLabel);
        caption->setBorderSize({});
        caption->setAccessible(false);
    }
    move(3,{&isolateButton,&isolateDryButton,&tailLabel,&tailSlider,&auditionLabel,&auditionSlider});
    move(4,{&generateButton,&allButton,&analyzeButton,&confirmButton,&candidateBox,&recaptureButton,&cancelButton,&recaptureStatus,&clearButton,&suppressionButton,&toneButton,&toneStartLabel,&toneStartSlider,&toneEndLabel,&toneEndSlider,&openDiagnosticsButton});
    allButton.setName("regenerateAll");
    allButton.setTooltip("Force fresh renders for all eligible swells, including current, disabled and muted swells. Frozen and importing-IR swells are skipped.");
    regenerationFeedback.setName("regenerationFeedback");
    m3::ui::applyTextRole(regenerationFeedback,m3::ui::TextRole::helper);
    settingsPages[4].addAndMakeVisible(regenerationFeedback);
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
    {label.setBounds(left,y,labelWidth,32);const juce::Rectangle<int> body{left+labelWidth,y,width-labelWidth,32};
        if(auto* selector=dynamic_cast<m3::ui::controls::ChoiceSelector*>(&value))selector->setRowBounds(body);else value.setBounds(body);};
    const auto switchHeight=m3::ui::controlGeometry::switchHeight;
    row(gainPolicyLabel,gainPolicyBox,24);liveButton.setBounds(left,68,width,switchHeight);row(settingsLabels[0],liveRoutingBox,108);row(liveLevelLabel,liveLevel,150);
    mappingButton.setBounds(left,24,width,switchHeight);automaticButton.setBounds(left,64,width,switchHeight);autoButton.setBounds(left,104,width,switchHeight);
    row(separationLabel,separationSlider,150);row(settingsLabels[1],defaultIRButton,196);
    muteButton.setBounds(left,20,150,switchHeight);sampleUnitsButton.setBounds({});sampleUnitsButton.setVisible(false); // Retained internal exact-coordinate diagnostic seam.
    row(settingsLabels[2],inCurveBox,60);row(settingsLabels[3],outCurveBox,100);
    settingsLabels[4].setBounds(left,146,width,24);
    layoutSourceRange();
    settingsLabels[5].setBounds(left,20,width,24);isolateButton.setBounds(left,56,250,switchHeight);isolateDryButton.setBounds(left+265,56,300,switchHeight);
    row(tailLabel,tailSlider,106);row(auditionLabel,auditionSlider,152);
    settingsLabels[6].setBounds(left,12,160,26);generateButton.setBounds(left+175,12,200,30);allButton.setBounds(left+390,12,200,30);
    const auto feedbackBand=m3::ui::controlGeometry::feedbackHeight+m3::ui::spacing::related;
    regenerationFeedback.setBounds(left,allButton.getBottom()+m3::ui::spacing::tight,width,m3::ui::controlGeometry::feedbackHeight);
    settingsLabels[7].setBounds(left,52+feedbackBand,160,26);analyzeButton.setBounds(left+175,52+feedbackBand,200,30);confirmButton.setBounds(left+390,52+feedbackBand,200,30);
    candidateBox.setBounds(left+175,90+feedbackBand,415,30);
    settingsLabels[8].setBounds(left,132+feedbackBand,160,26);recaptureButton.setBounds(left+175,132+feedbackBand,200,30);cancelButton.setBounds(left+390,132+feedbackBand,200,30);
    recaptureStatus.setBounds(left,170+feedbackBand,width,38);
    clearButton.setBounds(left,214+feedbackBand,220,30);suppressionButton.setBounds(left+240,214+feedbackBand,350,30);
    toneButton.setBounds(left,258+feedbackBand,width,switchHeight);
    toneStartLabel.setBounds(left,300+feedbackBand,90,30);toneStartSlider.setBounds(left+95,300+feedbackBand,190,30);toneEndLabel.setBounds(left+300,300+feedbackBand,100,30);toneEndSlider.setBounds(left+405,300+feedbackBand,185,30);
    openDiagnosticsButton.setBounds(left,346+feedbackBand,220,32);
    settingsLabels[9].setBounds({});settingsLabels[10].setBounds({});settingsLabels[11].setBounds({});
    for(int i:{0,1,3,4})settingsHelp[i].setBounds(left,i==4?394+feedbackBand:310,width,70);
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

void PreverbAudioProcessorEditor::refreshSourceRangeFeedback(const m3::Swell* swell)
{
    const PreverbAudioProcessor::SourceBarTarget target{state.sessionEpoch,state.session.primary};
    if(sourceRangeDraftTarget!=target)
    {
        // The pair belongs to the context in which it was displayed, even
        // when input and a selection change arrive before the next refresh.
        if(sourceRangeDraftTarget)
        {
            barStartEditor.setText({},false);
            barEndEditor.setText({},false);
        }
        sourceRangeDraftTarget=target;
        sourceRangeResult={};
    }

    const auto hasError=sourceRangeResult.rejection.isNotEmpty();
    const auto affected=sourceRangeResult.affected;
    barStartEditor.setInvalid(affected==SourceBarFields::start || affected==SourceBarFields::both);
    barEndEditor.setInvalid(affected==SourceBarFields::end || affected==SourceBarFields::both);
    juce::String message=hasError ? sourceRangeResult.rejection
        : !swell ? "Select a swell to apply its source range."
        : swell->frozen ? "Unfreeze the selected swell to change its source range."
        : barStartEditor.getText().isEmpty() && barEndEditor.getText().isEmpty()
            ? "Enter both bars for the selected swell. The end bar is excluded."
            : "The end bar is excluded.";
    if(affected==SourceBarFields::end)
        message+=". Enter a later bar, then choose Set source from bars to apply the corrected range.";
    if(sourceRangeMessage==message && sourceRangeHasError==hasError)return;

    sourceRangeMessage=message;
    sourceRangeHasError=hasError;
    barStartEditor.setFieldDescription(message);
    barEndEditor.setFieldDescription(message);
    if(!settingsLabels[4].getBounds().isEmpty())layoutSourceRange();
}

void PreverbAudioProcessorEditor::layoutSourceRange()
{
    using namespace m3::ui;
    using controls::commandBodyBounds;
    const auto heading=settingsLabels[4].getBounds();
    const auto margin=controls::SelectorPresentation::focusMargin;
    const auto fieldWidth=(heading.getWidth()/2-spacing::related)/2;
    const auto frameHeight=controlGeometry::height+2*margin;
    const auto top=heading.getBottom()+controlGeometry::labelHeight;
    barStartEditor.setBounds(heading.getX()-margin,top,fieldWidth,frameHeight);
    barEndEditor.setBounds(barStartEditor.getRight()+spacing::related,top,fieldWidth,frameHeight);

    const auto startBody=commandBodyBounds(barStartEditor.getBounds());
    const auto endBody=commandBodyBounds(barEndEditor.getBounds());
    const auto caption=[top](juce::Rectangle<int> body)
    {return body.withHeight(controlGeometry::labelHeight).withY(top-controlGeometry::labelHeight);};
    barStartLabel.setBounds(caption(startBody));
    barEndLabel.setBounds(caption(endBody));
    const auto pair=startBody.getUnion(endBody);
    sourceRangeFeedback.arrange(sourceRangeMessage,sourceRangeHasError,pair);

    const auto feedbackBottom=std::max(pair.getBottom(),sourceRangeFeedback.getBottom());
    setBarsButton.setBounds(barStartEditor.getBounds().getUnion(barEndEditor.getBounds())
                            .withY(feedbackBottom+spacing::related));
    const auto actionY=setBarsButton.getBottom()+spacing::related;
    fullButton.setBounds(heading.getX(),actionY,220,32);
    removeButton.setBounds(heading.getX()+240,actionY,160,32);
    settingsHelp[2].setBounds(heading.getX(),fullButton.getBottom()+spacing::section,heading.getWidth(),70);
}

void PreverbAudioProcessorEditor::refreshStorageScope(const CaptureSessionSnapshot& capture,uint64_t epoch)
{
    const auto identity=(capture.location?capture.location->projectsRoot.getFullPathName():juce::String{})+"/"+juce::String(capture.manifest.projectId)+"/"+juce::String(capture.manifest.instanceId)+"/"+juce::String(static_cast<juce::int64>(epoch));
    if(storageInventory)
    {
        if(identity!=storageIdentity){storageIdentity=identity;if(!storageInventory->reloadPending() && !storageInventory->consumeReloadEpoch(epoch))storageInventory->bind([this](auto& input,auto& error){return processor.pinPortableAssets(input,error,true);});}
        storageInventory->setSize(std::max(1,storageViewport.getWidth()-storageViewport.getScrollBarThickness()),
            std::max(1,storageViewport.getHeight()));
        storageInventory->poll();
    }
}
void PreverbAudioProcessorEditor::serviceAssetRecovery()
{
    if(!isShowing() || destroying)return;
    m3::storage::portable::Input input;juce::String error;
    if(!processor.pinPortableAssets(input,error,true)){
        if(error.contains("could not be decoded") && !metadataRecoveryWarned){
            metadataRecoveryWarned=true;
            // A failed restore must replace any earlier "edits preserved"
            // warning, including one still modal for a previous valid state.
            if(recoveryWarning){recoveryWarning->exitModalState(0);recoveryWarning->setVisible(false);}
            showAssetRecoveryWarning(input,true);
        }return;
    }
    metadataRecoveryWarned=false;
    const auto xml=juce::XmlDocument::parse(input.processorXml);const auto reference=xml?xml->getStringAttribute("manifestReference"):juce::String{};
    if(input.controlEpoch!=recoveryInspectionEpoch || input.instanceRoot!=recoveryInspectionRoot){
        recoveryInspection.cancel();recoveryInspectionEpoch=input.controlEpoch;recoveryInspectionRoot=input.instanceRoot;recoveryInspected=false;recoveryWarned=false;
    }
    if(recoveryWarned)return;
    if(input.sourceUnavailable){recoveryWarned=true;showAssetRecoveryWarning(input);return;}
    if(!recoveryInspected){
        m3::storage::ObservedSnapshot observed;if(!m3::storage::readLegacySnapshot(input.processorXml,input.manifestJson,observed,error))return;
        m3::storage::Request request;request.roots={input.instanceRoot};request.snapshots.push_back(std::move(observed));
        recoveryInspectionReference=reference;recoveryInspection.request(std::move(request));recoveryInspected=true;return;
    }
    if(reference!=recoveryInspectionReference){recoveryInspection.cancel();recoveryInspected=false;return;}
    if(!recoveryInspection.busy())if(const auto report=recoveryInspection.snapshot();report && !report->cancelled && report->missingReferences>0){recoveryWarned=true;showAssetRecoveryWarning(input);}
}
void PreverbAudioProcessorEditor::showAssetRecoveryWarning(const m3::storage::portable::Input& expected,bool metadataFailure)
{
    if(recoveryWarning && recoveryWarning->isCurrentlyModal())return;
    recoveryWarning=std::make_unique<juce::AlertWindow>(metadataFailure?"Project metadata unavailable":"Project assets unavailable",
        metadataFailure?"Saved plugin metadata could not be decoded. The arrangement cannot be confirmed intact. Keep the project and original assets; restore an earlier DAW project copy if available.":m3::ui::PortableStoragePanel::recoveryGuidance(),juce::MessageBoxIconType::NoIcon,this);
    recoveryWarning->setDescription(metadataFailure?"Saved plugin metadata could not be decoded; the arrangement cannot be confirmed intact.":m3::ui::PortableStoragePanel::recoveryGuidance());
    recoveryWarning->setComponentID("missingAssetRecoveryDialog");recoveryWarning->setLookAndFeel(&workspaceTheme);
    if(!metadataFailure){recoveryWarning->addButton("Locate assets...",1);recoveryWarning->addButton("Restore backup...",2);}
    recoveryWarning->addButton("Continue",0,juce::KeyPress(juce::KeyPress::escapeKey));
    recoveryWarning->setAlwaysOnTop(false);addAndMakeVisible(*recoveryWarning);recoveryWarning->setCentrePosition(getLocalBounds().getCentre());recoveryWarning->toFront(true);
    recoveryWarning->enterModalState(true,juce::ModalCallbackFunction::create([safe=juce::Component::SafePointer<PreverbAudioProcessorEditor>(this),expected](int action){
        if(!safe || safe->destroying || action==0)return;
        m3::storage::portable::Input current;juce::String error;
        if(!safe->processor.pinPortableAssets(current,error,true) || current.instanceRoot!=expected.instanceRoot || current.controlEpoch!=expected.controlEpoch)return;
        safe->setSettingsOpen(true);for(int i=0;i<safe->settingsTabs.getNumTabs();++i)if(safe->settingsTabs.getTabNames()[i]=="Storage")safe->settingsTabs.setCurrentTabIndex(i);
        if(action==1)safe->storageInventory->locateOriginalAssets();else if(action==2)safe->storageInventory->restoreBackup();
    }),false);
}
