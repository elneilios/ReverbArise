#pragma once
void Composition::verifyAudition()
{
    stopTimer();setScene(0);auto& engine=processor->swellEngine();
    auto panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);panel->stopAutomaticRefreshForReview();panel->setSize(1100,850);panel->refresh();
    auto& toggle=require<controls::LatchingActionButton>(*panel,"auditionWetOnly");auto& status=require<controls::EllipsisLabel>(*panel,"auditionStatus");
    REQUIRE(!find(*panel,"auditionBlend") && !find(*panel,"resetAuditionTrim"));
    const auto saved=engine.savedSession();const auto capture=processor->getCaptureSnapshot().manifest.revision;const auto renders=engine.snapshot().renderCount;
    REQUIRE(saved.auditionFollowsLive && saved.auditionMode==m3::AuditionMode::dryWet && !toggle.getToggleState());
    wait([&]{return engine.snapshot().auditionReady;});panel->refresh();REQUIRE(status.getText().startsWith("Ready") && toggle.getButtonText()=="Dry + wet");
    REQUIRE(toggle.keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    wait([&]{return engine.snapshot().auditionReady;});panel->refresh();REQUIRE(toggle.getToggleState() && engine.savedSession().auditionMode==m3::AuditionMode::wetOnly);
    auto& play=require<controls::ActionButton>(*panel,"audition");
    REQUIRE(play.isEnabled() && play.keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(35);float left[4096]{},right[4096]{};
    for(int n=0;n<80;++n){std::fill_n(left,4096,0);std::fill_n(right,4096,0);engine.processAudio(left,right,4096,processor->getSongTime());}
    panel->refresh();REQUIRE(engine.snapshot().playing && status.getText().startsWith("Looping") && toggle.getButtonText()=="Wet only");
    auto& stop=require<controls::ActionButton>(*panel,"stopAudition");
    REQUIRE(stop.isEnabled() && stop.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(35);
    engine.processAudio(left,right,4096,processor->getSongTime());panel->refresh();REQUIRE(!engine.snapshot().playing && status.getText().startsWith("Ready"));
    auto legacy=saved;legacy.auditionFollowsLive=false;legacy.wetBlend=.65f;legacy.auditionDb=-24;engine.restore(legacy);
    wait([&]{return engine.snapshot().auditionReady;});panel->refresh();
    REQUIRE(engine.savedSession().auditionFollowsLive && status.getTooltip().contains("Live Wet") && !toggle.getToggleState());
    toggle.beforeInvocation();toggle.setToggleState(true,juce::dontSendNotification);toggle.onClick();wait([&]{return engine.snapshot().auditionReady;});panel->refresh();
    REQUIRE(engine.savedSession().auditionFollowsLive && engine.savedSession().wetBlend==.65f && engine.savedSession().auditionDb==-24);
    const auto unityPeak=engine.snapshot().monitorPeak;engine.setPreferences(-60,saved.sensitivity,saved.separationSeconds,saved.autoRegenerate);
    REQUIRE(engine.snapshot().monitorPeak==unityPeak);engine.setPreferences(-24,saved.sensitivity,saved.separationSeconds,saved.autoRegenerate);
    const auto oldPeak=engine.snapshot().monitorPeak;engine.setLivePlayback(saved.liveEnabled,saved.liveRouting,-12);wait([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.snapshot().monitorPeak<oldPeak && engine.snapshot().renderCount==renders);
    juce::MemoryBlock encoded;processor->getStateInformation(encoded);auto reopened=std::make_unique<PreverbAudioProcessor>(directory.file);
    reopened->setStateInformation(encoded.getData(),static_cast<int>(encoded.getSize()));const auto restored=reopened->swellEngine().savedSession();
    REQUIRE(restored.auditionFollowsLive && restored.auditionMode==m3::AuditionMode::wetOnly && restored.liveWetDb==-12 && restored.auditionDb==-24 && restored.wetBlend==.65f && !reopened->swellEngine().snapshot().playing);
    reopened.reset();
    for(int stale=0;stale<3;++stale)
    {
        wait([&]{return engine.snapshot().auditionReady;});panel->refresh();toggle.beforeInvocation();toggle.setToggleState(false,juce::dontSendNotification);
        if(stale==0){auto replacement=engine.savedSession();engine.restore(replacement);}
        if(stale==1)REQUIRE(processor->requestWorkflowMode(workflow::Mode::finalise));
        if(stale==2)toggle.setEnabled(false);
        toggle.onClick();REQUIRE(engine.savedSession().auditionMode==m3::AuditionMode::wetOnly);
        if(stale==1)REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));
    }
    engine.restore(saved);wait([&]{return engine.snapshot().auditionReady;});panel->refresh();
    engine.setPostRoll(5);wait([&]{const auto state=engine.snapshot();return !state.auditionReady && state.dryStatus.contains("Dry source not captured");});panel->refresh();
    REQUIRE(status.getText().startsWith("Unavailable: capture") && status.getTooltip().contains("need samples"));
    REQUIRE(toggle.keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    wait([&]{return engine.snapshot().auditionReady;});panel->refresh();REQUIRE(status.getText().startsWith("Ready") && toggle.getToggleState());
    engine.restore(saved);wait([&]{return engine.snapshot().auditionReady;});panel->refresh();
    REQUIRE(engine.selectTargets({},{}));panel->refresh();REQUIRE(status.getText()=="Select a swell to audition");
    REQUIRE(engine.selectTargets(saved.selected,saved.primary));wait([&]{return engine.snapshot().auditionReady;});
    REQUIRE(processor->getCaptureSnapshot().manifest.revision==capture && engine.snapshot().renderCount==renders);
    const auto after=engine.savedSession();REQUIRE(after.exportState==saved.exportState && after.liveWetDb==saved.liveWetDb && !engine.snapshot().playing);
    for(size_t n=0;n<saved.swells.size();++n){const auto& a=saved.swells[n];const auto& b=after.swells[n];REQUIRE(a.id==b.id && a.ir==b.ir && a.render==b.render && a.shape==b.shape);}
}
