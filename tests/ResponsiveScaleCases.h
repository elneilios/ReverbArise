#pragma once

void Composition::verifyResponsiveScale()
{
    stopTimer();setScene(0);auto& engine=processor->swellEngine();
    TestDirectory profile;auto storage=std::make_shared<FilePresentationStorage>(profile.file.getChildFile("presentation.json"));
    const PresentationFitContext large{{4000,3000},{0,0}};
    const auto open=[&]
    {
        auto editor=productionEditor();editor->stopAutomaticRefreshForReview();editor->enableResponsiveReview(storage,large);
        editor->refresh();return editor;
    };
    auto panel=open(),other=open();
    REQUIRE(juce::Point<int>(panel->getWidth(),panel->getHeight())==juce::Point<int>(900,600));
    auto& root=require<juce::Component>(*panel,"presentationContent");
    auto& reset=require<juce::TextButton>(*panel,"resetWetShape");
    auto& footer=require<controls::TransportReadout>(*panel,"hostInfo");
    auto& cell=require<KnobValue>(*panel,"endOffsetCell");auto& field=cell.valueField();
    auto& tabs=dynamic_cast<juce::TabbedComponent&>(*panel->settingsPanel());
    REQUIRE(tabs.getNumTabs()==7);
    auto& appearance=*tabs.getTabContentComponent(6);
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
