#pragma once
#include <cstdlib>

void Composition::verifyResponsiveScale()
{
    stopTimer();setScene(0);auto& engine=processor->swellEngine();
    TestDirectory profile;auto storage=std::make_shared<FilePresentationStorage>(profile.file.getChildFile("presentation.json"));
    const PresentationFitContext large{{4000,3000},{0,0}};
   #if JUCE_WINDOWS
    // Exercise the real host factory without reading/writing the user's profile.
    {
        struct LocalProfile
        {
            juce::String previous=juce::SystemStats::getEnvironmentVariable("LOCALAPPDATA",{});
            explicit LocalProfile(const juce::File& directory){_putenv_s("LOCALAPPDATA",directory.getFullPathName().toRawUTF8());}
            ~LocalProfile(){_putenv_s("LOCALAPPDATA",previous.toRawUTF8());}
        } isolatedProfile(profile.file);
        std::unique_ptr<juce::AudioProcessorEditor> hosted(processor->createEditor());
        auto* actual=dynamic_cast<PreverbAudioProcessorEditor*>(hosted.get());REQUIRE(actual && actual->presentationState());
        actual->stopAutomaticRefreshForReview();actual->setPresentationDisplayForReview(large);
        REQUIRE(actual->getWidth()==900 && actual->getHeight()==600);
        REQUIRE(actual->setPresentationSize(1000,700));hosted.reset();
        REQUIRE(FilePresentationStorage::userFile().isAChildOf(profile.file));
        REQUIRE(FilePresentationStorage::userFile().existsAsFile());
        hosted.reset(processor->createEditor());actual=dynamic_cast<PreverbAudioProcessorEditor*>(hosted.get());
        REQUIRE(actual && actual->presentationState()->desired().width==1000);
    }
   #endif
    const auto open=[&]
    {
        auto editor=productionEditor();editor->stopAutomaticRefreshForReview();editor->enableResponsivePresentation(storage,large);
        editor->refresh();return editor;
    };
    auto panel=open(),other=open();
    REQUIRE(juce::Point<int>(panel->getWidth(),panel->getHeight())==juce::Point<int>(900,600));
    auto& root=require<juce::Component>(*panel,"presentationContent");
    auto& reset=require<m3::ui::controls::ActionButton>(*panel,"resetWetShape");
    auto& footer=require<controls::TransportReadout>(*panel,"hostInfo");
    auto& cell=require<KnobValue>(*panel,"endOffsetCell");auto& field=cell.valueField();
    using Page=m3::ui::SettingsPage;auto& tabs=*panel->settingsPanel();
    REQUIRE(tabs.selector().getNumChildComponents()==8);
    auto& appearance=*tabs.page(Page::appearance);
    auto& scaleChoice=require<controls::DeliberateSelector>(appearance,"presentationScale");
    auto& sizeChoice=require<controls::DeliberateSelector>(appearance,"presentationSize");
    const auto saved=engine.savedSession();const auto capture=processor->getCaptureSnapshot().manifest.revision;
    const auto renderCount=engine.snapshot().renderCount;const auto view=panel->timelineComponent().viewport();
    juce::String before,error;REQUIRE(m3::encode(saved,before,error));
    struct Notices final:juce::AudioProcessorListener
    {
        PreverbAudioProcessor& processor;int dirty=0;
        explicit Notices(PreverbAudioProcessor& p):processor(p){p.addListener(this);}
        ~Notices()override{processor.removeListener(this);}
        void audioProcessorParameterChanged(juce::AudioProcessor*,int,float)override{}
        void audioProcessorChanged(juce::AudioProcessor*,const ChangeDetails& details)override{if(details.nonParameterStateChanged)++dirty;}
    } notices(*processor);
    processor->flushHostStateChangesForTesting();notices.dirty=0;
    uint64_t baseBytes=0;
    for(int percent:{100,125,100,125})
    {
        REQUIRE(panel->setPresentationSize(900,600));scaleChoice.setSelectedId(percent,juce::sendNotificationSync);panel->refresh();
        REQUIRE(panel->presentationState()->effective().effective.scalePercent==percent);
        REQUIRE(root.getLocalBounds()==juce::Rectangle<int>(0,0,900,600));
        const auto extent=presentationExtent({900,600,percent});REQUIRE(juce::Point<int>(panel->getWidth(),panel->getHeight())==juce::Point<int>(extent.width,extent.height));
        REQUIRE(scaleChoice.getSelectedId()==percent && sizeChoice.getSelectedId()==1);
        const auto physical=panel->getLocalArea(&reset,reset.getLocalBounds());
        REQUIRE(require<juce::Component>(*panel,"presentationSurface").getComponentAt(physical.getCentre())==&reset);REQUIRE(reset.getParentComponent()==&root);
        REQUIRE(root.getLocalBounds().contains(footer.getBounds()));
        REQUIRE(footer.geometry().group.getX()>=0 && footer.geometry().group.getRight()<=footer.getWidth());
        const auto point=field.getLocalBounds().getCentre().toFloat();
        for(float hostScale:{1.f,1.5f,2.f})
        {
            panel->setTransform(juce::AffineTransform::scale(hostScale));
            const auto screen=field.localPointToGlobal(point),roundtrip=field.getLocalPoint(nullptr,screen);
            REQUIRE(roundtrip.getDistanceFrom(point)<.01f);
            const auto now=juce::Time::getCurrentTime();
            const juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(),point,{},0,0,0,0,0,&field,&field,now,point,now,1,false);
            REQUIRE(panel->swellMenuAnchor(event).getPosition()==screen.roundToInt());
        }
        panel->setTransform({});
        const auto image=panel->createComponentSnapshot(panel->getLocalBounds());REQUIRE(image.getWidth()==extent.width && image.getHeight()==extent.height);
        const auto bytes=panel->timelineComponent().waveformPaintMetrics().bytes;REQUIRE(bytes>0);
        if(percent==100)baseBytes=bytes;else REQUIRE(bytes>baseBytes*1.4);
        REQUIRE(panel->timelineComponent().viewport().first==view.first && panel->timelineComponent().viewport().last==view.last);
        REQUIRE(juce::Point<int>(other->getWidth(),other->getHeight())==juce::Point<int>(900,600));
    }
    // Exercise the shared editor's exact-entry lifecycle without a desktop peer
    // or OS focus changes. Scaling/refresh must retain active text; Escape cancels.
    require<controls::SegmentedSelector>(*panel,"inspectorSections").onRequest(2);
    const auto point=field.getLocalBounds().getCentre().toFloat();const auto now=juce::Time::getCurrentTime();
    const juce::MouseEvent entryEvent(juce::Desktop::getInstance().getMainMouseSource(),point,juce::ModifierKeys::leftButtonModifier,0,0,0,0,0,&field,&field,now,point,now,1,false);
    field.mouseDown(entryEvent);field.setText("0.123456789",false);
    REQUIRE(panel->setPresentationScale(100));panel->refresh();REQUIRE(field.getText()=="0.123456789");
    REQUIRE(panel->setPresentationScale(125));panel->refresh();REQUIRE(field.getText()=="0.123456789");field.onEscapeKey();
    REQUIRE(!panel->setPresentationScale(90));REQUIRE(!panel->setPresentationScale(75));
    REQUIRE(panel->setPresentationSize(1100,850));const auto desired=panel->presentationState()->desired();
    panel->setPresentationDisplayForReview({{1000,750},{16,80}});
    REQUIRE(juce::Point<int>(panel->getWidth(),panel->getHeight())==juce::Point<int>(984,670));REQUIRE(panel->presentationState()->desired()==desired);
    REQUIRE(!scaleChoice.isItemEnabled(125));REQUIRE(!sizeChoice.isItemEnabled(2));
    scaleChoice.setSelectedId(125,juce::sendNotificationSync);REQUIRE(scaleChoice.getSelectedId()==100);
    REQUIRE(panel->presentationState()->desired()==desired);
    panel->setPresentationDisplayForReview(large);REQUIRE(juce::Point<int>(panel->getWidth(),panel->getHeight())==juce::Point<int>(1375,1063));
    panel->setPresentationDisplayForReview({{800,500},{16,80}});
    REQUIRE(!panel->presentationState()->effective().supported);
    REQUIRE(require<juce::Label>(appearance,"presentationHelp").getDescription().contains("cannot fit"));
    panel->setPresentationDisplayForReview(large);
    processor->flushHostStateChangesForTesting();REQUIRE(notices.dirty==0);
    juce::String after;REQUIRE(m3::encode(engine.savedSession(),after,error));REQUIRE(before==after);
    REQUIRE(engine.snapshot().renderCount==renderCount && processor->getCaptureSnapshot().manifest.revision==capture);
    // Real editor close/reopen persistence, including a second unedited editor
    // closing later, must preserve the explicitly changed presentation only.
    panel.reset();other.reset();panel=open();
    REQUIRE(panel->presentationState()->desired()==desired);REQUIRE(juce::Point<int>(panel->getWidth(),panel->getHeight())==juce::Point<int>(1375,1063));
    processor->flushHostStateChangesForTesting();REQUIRE(notices.dirty==0);
    REQUIRE(m3::encode(engine.savedSession(),after,error));REQUIRE(before==after);
    // Owned windows use the same magnification without resizing the editor.
    NativeEditorPeer nativeOwner(*panel,"ReverbArise - P8 auxiliary lifecycle native validation");
    const bool nativeFocusRequired=juce::SystemStats::getEnvironmentVariable("PREVERB_REQUIRE_NATIVE_AUX_FOCUS",{})=="1";
    if(nativeFocusRequired)
    {
        const auto deadline=juce::Time::getMillisecondCounterHiRes()+30000;
        while((!juce::Process::isForegroundProcess() || !nativeOwner.getPeer()->isFocused()) && juce::Time::getMillisecondCounterHiRes()<deadline)
            juce::MessageManager::getInstance()->runDispatchLoopUntil(10);
        REQUIRE(juce::Process::isForegroundProcess() && nativeOwner.getPeer()->isFocused());
    }
    auto& header=require<WorkflowHeader>(*panel,"workflowHeader");header.onSettings();
    auto* settingsWindow=dynamic_cast<juce::DocumentWindow*>(panel->settingsPanel()->getTopLevelComponent());REQUIRE(settingsWindow);
    {
        auto& actualTabs=*panel->settingsPanel();actualTabs.selectPage(Page::about);
        juce::SharedResourcePointer<controls::FocusVisibility> input;input->keyboardInput();
        REQUIRE(settingsWindow->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey)));
        juce::MessageManager::getInstance()->runDispatchLoopUntil(40);
        REQUIRE(!settingsWindow->isVisible() && actualTabs.selectedPage()==Page::about);
        if(nativeFocusRequired || juce::Process::isForegroundProcess())REQUIRE(juce::Process::isForegroundProcess() && header.settingsAction().hasKeyboardFocus(false));
        header.onSettings();REQUIRE(actualTabs.getTopLevelComponent()==settingsWindow && actualTabs.selectedPage()==Page::about);
    }
    settingsWindow->setVisible(false);
    auto& ownedTabs=*panel->settingsPanel();
    ownedTabs.selectPage(Page::about);
    auto& auxiliary=require<controls::PresentationSurface>(*settingsWindow,"presentationSurface");
    for(int scale:{100,125,100})
    {
        REQUIRE(panel->setPresentationScale(scale));
        REQUIRE(auxiliary.contentRoot().getWidth()==640 && auxiliary.contentRoot().getHeight()==570);
        REQUIRE(std::abs(auxiliary.contentRoot().getTransform().mat00-scale/100.f)<.001f);
        REQUIRE(ownedTabs.selectedPage()==Page::about);
        const auto& strip=ownedTabs.selector();
        for(auto* child:strip.getChildren())
        {
            const auto& button=dynamic_cast<const juce::Button&>(*child);
            const auto width=juce::GlyphArrangement::getStringWidth(interFont(bodySize,InterFace::semibold),button.getButtonText());
            REQUIRE(width+10<=button.getWidth());
            REQUIRE(strip.getLocalBounds().contains(button.getBounds().expanded(static_cast<int>(controlGeometry::focusOutset+controlGeometry::focusStroke))));
        }
        auto& viewport=require<controls::FocusViewport>(*settingsWindow,"auxiliaryPresentationViewport");
        auto& copy=require<juce::Component>(*panel->settingsPanel(),"copyBuildInformation");viewport.reveal(copy);
        REQUIRE(viewport.getViewArea().intersects(auxiliary.getLocalArea(&copy,copy.getLocalBounds())));
        REQUIRE(panel->getWidth()==presentationExtent(panel->presentationState()->effective().effective).width);
    }
    settingsWindow->setSize(settingsWindow->getWidth(),250);settingsWindow->setVisible(true);settingsWindow->toFront(true);
    auto& copyAction=require<juce::Component>(*panel->settingsPanel(),"copyBuildInformation");copyAction.grabKeyboardFocus();
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    auto& focusViewport=require<controls::FocusViewport>(*settingsWindow,"auxiliaryPresentationViewport");
    REQUIRE(copyAction.hasKeyboardFocus(true));
    REQUIRE(focusViewport.getViewArea().contains(auxiliary.getLocalArea(&copyAction,copyAction.getLocalBounds()).expanded(static_cast<int>(controlGeometry::focusPadding))));
    ownedTabs.selector().grabKeyboardFocus();juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(focusViewport.getViewArea().intersects(auxiliary.getLocalArea(&ownedTabs.selector(),ownedTabs.selector().getLocalBounds())));
    copyAction.grabKeyboardFocus();juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(copyAction.hasKeyboardFocus(true));
    const auto revealVertically=[](controls::FocusViewport& viewport,juce::Component& target)
    {
        auto* viewed=viewport.getViewedComponent();
        REQUIRE(viewed && viewed->isParentOf(&target));
        viewport.setViewPosition(0,0);
        viewport.reveal(target);
        const auto targetBounds=viewed->getLocalArea(&target,target.getLocalBounds());
        const auto visible=viewport.getViewArea();
        REQUIRE(targetBounds.getY()>=visible.getY() && targetBounds.getBottom()<=visible.getBottom());
    };
    for (const auto percent : {100,125})
    {
        REQUIRE(panel->setPresentationScale(percent));
        ownedTabs.selectPage(Page::storage);
        panel->refresh();
        auto& nested=require<controls::FocusViewport>(*panel->settingsPanel(),"storageContentViewport");
        auto& destination=require<juce::Label>(*panel->settingsPanel(),"portableRestoreDestination");
        const auto before=destination.getText();
        revealVertically(nested,destination);
        REQUIRE(destination.getText()==before);
        auto& outer=require<controls::FocusViewport>(*panel->settingsPanel(),"storageViewport");
        revealVertically(outer,require<juce::Component>(*panel->settingsPanel(),"locateAssets"));

        for (const auto size : {juce::Point<int>{900,600},juce::Point<int>{1100,850}})
        {
            REQUIRE(panel->setPresentationSize(size.x,size.y));
            require<controls::SegmentedSelector>(*panel,"inspectorSections").onRequest(2);
            panel->refresh();
            auto& inspector=require<controls::FocusViewport>(*panel,"inspectorViewport");
            auto& currentField=require<KnobValue>(*panel,"endOffsetCell").valueField();
            auto& pinnedReset=require<juce::Component>(*panel,"resetWetShape");
            const auto value=currentField.getText();
            const auto resetBounds=pinnedReset.getBounds();
            revealVertically(inspector,currentField);
            REQUIRE(currentField.getText()==value && pinnedReset.getBounds()==resetBounds
                && !inspector.isParentOf(&pinnedReset));
        }
    }
    settingsWindow->setVisible(false);
    ownedTabs.selectPage(Page::recovery);
    settingsWindow->setVisible(true);
    require<m3::ui::controls::ActionButton>(*panel->settingsPanel(),"openDiagnostics").onClick();
    auto* diagnosticsWindow=dynamic_cast<juce::DocumentWindow*>(panel->diagnosticsPanel()->getTopLevelComponent());REQUIRE(diagnosticsWindow);
    for(const auto percent : {100,125})
    {
        REQUIRE(panel->setPresentationScale(percent));
        auto& diagnostics=require<controls::FocusViewport>(*diagnosticsWindow,"diagnosticsViewport");
        revealVertically(diagnostics,require<juce::Component>(*diagnosticsWindow,"copyDiagnostics"));
    }
    REQUIRE(diagnosticsWindow->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey)));REQUIRE(!diagnosticsWindow->isVisible());
    juce::MessageManager::getInstance()->runDispatchLoopUntil(40);
    if(nativeFocusRequired || juce::Process::isForegroundProcess())REQUIRE(juce::Process::isForegroundProcess() && require<controls::ActionButton>(*panel->settingsPanel(),"openDiagnostics").hasKeyboardFocus(false));
    require<controls::ActionButton>(*panel->settingsPanel(),"openDiagnostics").onClick();settingsWindow->setVisible(false);
    diagnosticsWindow->closeButtonPressed();juce::MessageManager::getInstance()->runDispatchLoopUntil(40);
    REQUIRE(!settingsWindow->isVisible() && !diagnosticsWindow->isVisible());
    if(nativeFocusRequired || juce::Process::isForegroundProcess())REQUIRE(juce::Process::isForegroundProcess() && header.settingsAction().hasKeyboardFocus(false));
    REQUIRE(panel->setPresentationScale(125));panel->refresh();
    REQUIRE(!diagnosticsWindow->isVisible());
    REQUIRE(panel->setPresentationScale(100));REQUIRE(panel->setPresentationSize(900,600));
    auto& zoom=require<controls::TimelineZoomControls>(*panel,"performZoom");
    auto& list=require<controls::SwellList>(*panel,"swellList");
    REQUIRE(panel->setPresentationScale(125));
    zoom.beforeInvocation();zoom.onZoomFraction(.85);
    auto* row=list.getComponentForRowNumber(list.rowFor(firstId));REQUIRE(row);
    const auto physicalRow=panel->getLocalArea(row,row->getLocalBounds());
    auto* hit=require<juce::Component>(*panel,"presentationSurface").getComponentAt(physicalRow.getCentre());REQUIRE(hit==row || row->isParentOf(hit));
    const auto rowPoint=row->getLocalBounds().getCentre().toFloat();const auto clicked=juce::Time::getCurrentTime();
    const juce::MouseEvent rowClick(juce::Desktop::getInstance().getMainMouseSource(),rowPoint,juce::ModifierKeys::leftButtonModifier,0,0,0,0,0,row,row,clicked,rowPoint,clicked,1,false);
    const juce::MouseEvent rowRelease(juce::Desktop::getInstance().getMainMouseSource(),rowPoint,{},0,0,0,0,0,row,row,clicked,rowPoint,clicked,1,false);
    row->mouseDown(rowClick);row->mouseUp(rowRelease);
    const auto selected=panel->timelineComponent().viewport();
    zoom.beforeInvocation();zoom.onZoomFraction(.65);zoom.beforeInvocation();zoom.onFit();
    REQUIRE(panel->timelineComponent().viewport().first==selected.first && panel->timelineComponent().viewport().last==selected.last);
    REQUIRE(panel->setPresentationSize(1000,700));panel->refresh();
    REQUIRE(panel->timelineComponent().viewport().first==selected.first && panel->timelineComponent().viewport().last==selected.last);
    for(const auto mode:{workflow::Mode::capture,workflow::Mode::finalise,workflow::Mode::live})
    {
        REQUIRE(processor->requestWorkflowMode(mode));panel->refresh();
        REQUIRE(find(*panel,"presentationSurface")->isVisible());
        REQUIRE(panel->setPresentationScale(125));REQUIRE(panel->createComponentSnapshot(panel->getLocalBounds()).isValid());
        REQUIRE(panel->setPresentationScale(100));
    }
    // Reusable focus reveal includes a transformed content root and its margin.
    controls::PresentationSurface surface;controls::FocusViewport viewport;juce::TextEditor target;
    surface.contentRoot().addAndMakeVisible(target);surface.present(640,800,125);surface.setSize(800,1000);
    target.setBounds(40,700,300,32);viewport.setBounds(0,0,600,300);viewport.setViewedComponent(&surface,false);
    viewport.reveal(target);const auto targetBounds=surface.getLocalArea(&target,target.getLocalBounds());
    REQUIRE(viewport.getViewArea().contains(targetBounds.expanded(static_cast<int>(controlGeometry::focusPadding))));
}
