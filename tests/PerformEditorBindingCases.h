#pragma once
void Composition::verifyEditorBindings()
{
    stopTimer();setScene(0);auto& engine=processor->swellEngine();
    auto panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);panel->stopAutomaticRefreshForReview();panel->setSize(1100,850);panel->refresh();
    const auto saved=engine.savedSession();const auto captureRevision=processor->getCaptureSnapshot().manifest.revision;
    auto& zoom=require<controls::TimelineZoomControls>(*panel,"performZoom");
    auto& snap=require<controls::LatchingActionButton>(*panel,"performSnap");
    auto& grid=require<controls::DeliberateSelector>(*panel,"performGrid");
    REQUIRE(!find(*panel,"detailBars"));
    const auto checkLayout=[&]
    {
        const PerformLayout bounds(panel->getLocalBounds());
        REQUIRE(panel->timelineComponent().getBounds()==bounds.detail.withTrimmedTop(performGeometry::columnInset));
        REQUIRE(bounds.toolbar.contains(zoom.getBounds()) && zoom.getWidth()>=controls::TimelineZoomControls::minimumWidth());
        REQUIRE(bounds.toolbar.contains(snap.getBounds()) && bounds.toolbar.contains(grid.getBounds()));
        REQUIRE(!require<juce::Component>(*panel,"autoMarkers").isVisible());
        const auto picture=panel->createComponentSnapshot(panel->getLocalBounds());
        for(const auto y:{bounds.header.getBottom(),bounds.work.getBottom(),bounds.audition.getBottom(),bounds.overview.getBottom()})
            for(int x=bounds.header.getX();x<bounds.header.getRight();++x)REQUIRE(picture.getPixelAt(x,y)==separator);
    };
    for(const auto size:{juce::Point<int>{1100,850},{1199,900},{1200,900},{1400,900},{1400,1000},{1600,1000}})
    {panel->setSize(size.x,size.y);checkLayout();}
    panel->setSize(1100,850);panel->refresh();
    zoom.beforeInvocation();zoom.onZoomFraction(.65);const auto userView=panel->timelineComponent().viewport();
    REQUIRE(engine.savedSession().performUi.hasView);
    REQUIRE(engine.selectTargets({secondId},secondId));panel->refresh();
    REQUIRE(panel->timelineComponent().viewport().first==userView.first && panel->timelineComponent().viewport().last==userView.last);
    REQUIRE(processor->requestWorkflowMode(workflow::Mode::capture));panel->refresh();
    REQUIRE(require<LearnWorkspace>(*panel,"learnWorkspace").isVisible() && !zoom.isVisible());
    REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));panel->refresh();
    REQUIRE(panel->timelineComponent().viewport().first==userView.first && panel->timelineComponent().viewport().last==userView.last);
    // Real list row input, including a re-click that does not change selection.
    auto& list=require<controls::SwellList>(*panel,"swellList");
    auto& timeline=panel->timelineComponent();
    const auto clickRow=[&](const juce::String& id,juce::ModifierKeys mods=juce::ModifierKeys{},int x=80)
    {
        const int row=list.rowFor(id);REQUIRE(row>=0);list.scrollToEnsureRowIsOnscreen(row);
        const auto p=juce::Point<int>{x,list.getRowPosition(row,true).getCentreY()};
        auto* component=list.getComponentAt(p);REQUIRE(component);
        const auto local=component->getLocalPoint(&list,p).toFloat();const auto now=juce::Time::getCurrentTime();
        juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(),local,mods,0,0,0,0,0,component,component,now,local,now,1,false);
        component->mouseDown(event);component->mouseUp(event);
    };
    const auto reveals=[&](int64_t arrival)
    {
        const auto view=timeline.viewport();REQUIRE(view.first<arrival && view.last>arrival);
        timeline.fitSelection();
        REQUIRE(timeline.viewport().first==view.first && timeline.viewport().last==view.last);
        const auto preferences=engine.savedSession().performUi;REQUIRE(preferences.hasView && preferences.first==view.first && preferences.last==view.last);
        auto& overview=require<m3::SwellOverview>(*panel,"songOverview");
        REQUIRE(overview.waveformBounds().contains(overview.detailBounds()));
        REQUIRE(timeline.sourceLane().getX()==timeline.wetLane().getX() && timeline.sourceLane().getWidth()==timeline.wetLane().getWidth());
        REQUIRE(timeline.musicalRulerBounds().getX()==juce::roundToInt(timeline.sourceLane().getX()));
    };
    const auto selectionJobs=engine.snapshot().renderCount;const auto transport=processor->getSongTime().sample;
    timeline.setViewport({100000,140000});clickRow(firstId);reveals(48000);
    REQUIRE(timeline.viewport().span()!=40000); // Selection automatically adopts Fit's zoom too.
    auto revealed=timeline.viewport();clickRow(firstId);REQUIRE(timeline.viewport().first==revealed.first);
    timeline.setViewport({100000,140000});clickRow(firstId);reveals(48000);
    timeline.zoom(.5);clickRow(firstId);reveals(48000); // Same row after manual zoom.
    clickRow(secondId);reveals(245760);
    clickRow(firstId,juce::ModifierKeys::ctrlModifier);reveals(48000);REQUIRE(engine.savedSession().selected.size()==2);
    clickRow(secondId,juce::ModifierKeys::shiftModifier);reveals(245760);REQUIRE(engine.savedSession().selected.size()==2);
    // Checkbox enablement remains a separate action and does not request reveal.
    timeline.setViewport({100000,140000});clickRow(secondId,{},12);
    REQUIRE(timeline.viewport().first==100000);clickRow(secondId,{},12);
    REQUIRE(engine.edit(firstId,[](auto& swell,auto&){swell.name="Renamed navigation target";return true;}));panel->refresh();
    clickRow(firstId);reveals(48000);
    revealed=timeline.viewport();panel->refresh();panel->refresh();REQUIRE(timeline.viewport().first==revealed.first);
    const auto currentEpoch=engine.assignmentEpoch();
    list.onReveal(secondId,currentEpoch-1);REQUIRE(timeline.viewport().first==revealed.first);
    list.onReveal({},currentEpoch);REQUIRE(timeline.viewport().first==revealed.first);
    REQUIRE(!engine.snapshot().playing && engine.snapshot().renderCount==selectionJobs && processor->getSongTime().sample==transport);
    // Return to the same preferred view; subsequent save/reopen assertions below
    // now cover the view published by a real row reveal.
    REQUIRE(processor->requestWorkflowMode(workflow::Mode::finalise));panel->refresh();
    REQUIRE(require<ExportWorkspace>(*panel,"exportWorkspace").isVisible() && !zoom.isVisible());
    REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));panel->refresh();
    REQUIRE(panel->timelineComponent().viewport().first==revealed.first && panel->timelineComponent().viewport().last==revealed.last);
    REQUIRE(snap.keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(!panel->timelineComponent().requestedSnap() && !engine.savedSession().performUi.snap && !snap.getToggleState());
    panel->modifierKeysChanged(juce::ModifierKeys::ctrlModifier);panel->modifierKeysChanged({});
    REQUIRE(!snap.getToggleState() && !engine.savedSession().performUi.snap);
    REQUIRE(snap.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(snap.getToggleState() && engine.savedSession().performUi.snap);
    struct Notifications final:juce::AudioProcessorListener
    {
        PreverbAudioProcessor& owner;int changes=0;
        explicit Notifications(PreverbAudioProcessor& p):owner(p){owner.addListener(this);}
        ~Notifications()override{owner.removeListener(this);}
        void audioProcessorParameterChanged(juce::AudioProcessor*,int,float)override{}
        void audioProcessorChanged(juce::AudioProcessor*,const ChangeDetails& detail)override{if(detail.nonParameterStateChanged)++changes;}
    };
    processor->flushHostStateChangesForTesting();Notifications notifications(*processor);const auto beforeJobs=engine.snapshot().renderCount;
    const auto beforeBypass=engine.savedSession().performUi;
    panel->modifierKeysChanged(juce::ModifierKeys::ctrlModifier|juce::ModifierKeys::shiftModifier);
    REQUIRE(!snap.getToggleState() && panel->timelineComponent().requestedSnap() && engine.savedSession().performUi==beforeBypass);
    processor->flushHostStateChangesForTesting();REQUIRE(notifications.changes==0 && engine.snapshot().renderCount==beforeJobs);
    panel->addToDesktop(juce::ComponentPeer::windowIsTemporary);panel->setVisible(true);
    REQUIRE(snap.getAccessibilityHandler() && !snap.getAccessibilityHandler()->getCurrentState().isChecked());
    juce::MemoryBlock state;processor->getStateInformation(state);const auto heldState=state;panel->modifierKeysChanged({});
    REQUIRE(snap.getToggleState() && engine.savedSession().performUi==beforeBypass);panel->removeFromDesktop();
    processor->flushHostStateChangesForTesting();REQUIRE(notifications.changes==0);
    const auto beforeGrid=engine.savedSession().performUi;const auto point=grid.getLocalBounds().getCentre().toFloat();const auto now=juce::Time::getCurrentTime();
    juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(),point,{},0,0,0,0,0,&grid,&grid,now,point,now,1,false);juce::MouseWheelDetails wheel{};wheel.deltaY=1;
    grid.mouseWheelMove(event,wheel);REQUIRE(engine.savedSession().performUi==beforeGrid);
    grid.beforeInvocation();grid.setSelectedId(6,juce::sendNotificationSync);
    REQUIRE(panel->timelineComponent().selectedGrid()==m3::GridDivision::twoBars && engine.savedSession().performUi.grid==m3::GridDivision::twoBars);
    // Save the latest requested preference, then reopen while no bypass is restored.
    processor->getStateInformation(state);const auto preference=engine.savedSession().performUi;const auto exportPreference=engine.savedSession().exportState;
    panel.reset();panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);panel->stopAutomaticRefreshForReview();panel->refresh();
    REQUIRE(panel->timelineComponent().requestedSnap() && panel->timelineComponent().selectedGrid()==m3::GridDivision::twoBars);
    REQUIRE(std::abs(panel->timelineComponent().viewport().first-preference.first)<.001 && std::abs(panel->timelineComponent().viewport().last-preference.last)<.001);
    auto copy=std::make_unique<PreverbAudioProcessor>(directory.file);copy->setStateInformation(heldState.getData(),static_cast<int>(heldState.getSize()));
    REQUIRE(copy->swellEngine().savedSession().performUi.snap);
    copy->setStateInformation(state.getData(),static_cast<int>(state.getSize()));
    REQUIRE(copy->swellEngine().savedSession().performUi==preference && copy->swellEngine().savedSession().exportState==exportPreference && !copy->swellEngine().snapshot().playing);
    auto separate=preference;separate.snap=false;REQUIRE(copy->swellEngine().setPerformNavigation(separate,copy->swellEngine().assignmentEpoch()));
    REQUIRE(engine.savedSession().performUi.snap);
    auto& pending=require<controls::TimelineZoomControls>(*panel,"performZoom");pending.beforeInvocation();pending.fitButton().triggerClick();
    const auto oldEpoch=engine.assignmentEpoch();auto replacement=engine.savedSession();engine.restore(replacement);panel->refresh();panel->timelineComponent().setViewport({100000,140000});
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(panel->timelineComponent().viewport().first==100000 && panel->timelineComponent().viewport().last==140000);
    REQUIRE(!engine.setPerformNavigation({},oldEpoch));
    REQUIRE(processor->getCaptureSnapshot().manifest.revision==captureRevision);
    const auto after=engine.savedSession();REQUIRE(after.swells.size()==saved.swells.size());
    for(size_t n=0;n<saved.swells.size();++n)
    {const auto& a=saved.swells[n];const auto& b=after.swells[n];REQUIRE(a.id==b.id && a.render==b.render && a.ir==b.ir && a.shape==b.shape && a.arrival==b.arrival && a.sourceStart==b.sourceStart && a.sourceEnd==b.sourceEnd);}
    // Production reset buttons retain their model boundary through keyboard input.
    const auto id=after.primary;REQUIRE(engine.edit(id,[](auto& swell,auto&){swell.shape.gainDb=-7;return true;}));panel->refresh();
    const auto edited=*m3::find(engine.savedSession(),id);
    auto& reset=require<controls::ActionButton>(*panel,"resetWetShape");
    REQUIRE(reset.isEnabled() && reset.keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(35);
    const auto resetShape=*m3::find(engine.savedSession(),id);
    REQUIRE(resetShape.shape.gainDb==0 && resetShape.id==edited.id && resetShape.ir==edited.ir && resetShape.render==edited.render);
    REQUIRE(resetShape.sourceStart==edited.sourceStart && resetShape.sourceEnd==edited.sourceEnd);
    auto& resetAll=require<controls::ActionButton>(*panel,"resetSwellMenu");
    REQUIRE(resetAll.getTitle()=="Reset All");
    REQUIRE(resetAll.isEnabled() && resetAll.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(35);
    REQUIRE(m3::find(engine.savedSession(),id)->id==id && processor->getCaptureSnapshot().manifest.revision==captureRevision);
}
