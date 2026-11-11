#pragma once
void Composition::verifySelectors()
{
    using controls::ChoiceSelector;
    stopTimer();setScene(0);auto& engine=processor->swellEngine();
    auto panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);
    panel->stopAutomaticRefreshForReview();panel->setSize(1100,850);panel->refresh();
    panel->addToDesktop(0);panel->setVisible(true);panel->toFront(true);
    const auto pump=[]{juce::MessageManager::getInstance()->runDispatchLoopUntil(40);};
    const auto inspect=[&](ChoiceSelector& selector,int index)
    {selector.showPopup();auto* popup=selector.activePopup();REQUIRE(popup);popup->inspect(index,true);return popup;};
    auto& grid=require<ChoiceSelector>(*panel,"performGrid");
    const auto original=grid.getSelectedId();const auto division=panel->timelineComponent().selectedGrid();
    auto* popup=inspect(grid,0);REQUIRE(grid.getSelectedId()==original && panel->timelineComponent().selectedGrid()==division);
    popup->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));pump();REQUIRE(grid.getSelectedId()==original);
    popup=inspect(grid,original==1?1:0);popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();
    REQUIRE(grid.getSelectedId()==(original==1?2:1));
    REQUIRE(panel->timelineComponent().selectedGrid()==m3::gridChoices[static_cast<size_t>(grid.getSelectedId()-1)].division);
    // Present the actual Settings pages on a peer; their production callbacks
    // and editor lifetime stay intact, without opening a second review window.
    juce::Component settingsHost;using Page=m3::ui::SettingsPage;auto& tabs=*panel->settingsPanel();
    settingsHost.addAndMakeVisible(tabs);settingsHost.setBounds(120,120,760,600);tabs.setBounds(settingsHost.getLocalBounds());
    settingsHost.addToDesktop(0);settingsHost.setVisible(true);settingsHost.toFront(true);tabs.selectPage(Page::swell);pump();
    auto& in=require<ChoiceSelector>(*tabs.page(Page::swell),"fadeInCurve");
    auto& out=require<ChoiceSelector>(*tabs.page(Page::swell),"fadeOutCurve");
    const auto current=[&](const juce::String& id){return *m3::find(engine.savedSession(),id);};
    const auto before=current(firstId).shape;
    popup=inspect(in,2);REQUIRE(current(firstId).shape==before);
    popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();panel->refresh();
    REQUIRE(current(firstId).shape.inCurve==m3::Curve::exponential);
    popup=inspect(out,1);popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();panel->refresh();
    REQUIRE(static_cast<int>(current(firstId).shape.outCurve)==1);
    // A replacement primary with the same displayed preset must not receive a
    // queued command, even when the editor has not refreshed yet.
    const auto secondBefore=current(secondId).shape;
    popup=inspect(in,0);REQUIRE(engine.selectTargets({secondId},secondId));
    popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();panel->refresh();
    REQUIRE(current(secondId).shape==secondBefore && current(firstId).shape.inCurve==m3::Curve::exponential);
    tabs.selectPage(Page::playback);pump();
    auto& routing=require<ChoiceSelector>(*tabs.page(Page::playback),"liveRouting");
    auto& policy=require<ChoiceSelector>(*tabs.page(Page::playback),"gainPolicy");
    const auto routingBefore=engine.savedSession().liveRouting;
    popup=inspect(routing,routingBefore==m3::LiveRouting::dryWet?1:0);
    popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();panel->refresh();
    REQUIRE(engine.savedSession().liveRouting!=routingBefore);
    const auto policyBefore=engine.savedSession().gainPolicy;
    popup=inspect(policy,2);REQUIRE(!popup->state().items()[2].eligible);
    REQUIRE(popup->rowComponent(2).getAccessibilityHandler()->getDescription().contains("saved-project compatibility"));
    popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));REQUIRE(policy.isPopupActive() && engine.savedSession().gainPolicy==policyBefore);
    popup->inspect(policyBefore==m3::GainPolicy::legacy?1:0,true);
    popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();panel->refresh();
    REQUIRE(engine.savedSession().gainPolicy!=policyBefore);
    const auto acceptedRouting=engine.savedSession().liveRouting;
    popup=inspect(routing,acceptedRouting==m3::LiveRouting::dryWet?1:0);
    engine.restore(engine.savedSession());popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();panel->refresh();
    REQUIRE(engine.savedSession().liveRouting==acceptedRouting);

    tabs.selectPage(Page::recovery);pump();
    auto& integrated=require<RecoveryOnsetSelector>(*tabs.page(Page::recovery),"onsetSuggestions");
    REQUIRE(integrated.getAccessibilityHandler()->getTitle()=="Onset suggestions");
    REQUIRE(integrated.getTextWhenNothingSelected()=="Click a suggestion in the source lane");
    REQUIRE(controls::commandBodyBounds(integrated.getBounds()).getHeight()==30);

    // Exercise the same control with deterministic sample data and a native peer.
    RecoveryOnsetSelector suggestions;
    settingsHost.addAndMakeVisible(suggestions);
    suggestions.setRowBounds({200,140,415,30});
    REQUIRE(!suggestions.isEnabled() && !suggestions.selectedSample());
    REQUIRE(suggestions.getAccessibilityHandler()->getDescription().contains("source lane"));
    const int64_t wideSample=int64_t{1}<<40;
    std::vector<m3::Candidate> candidates{{10,1.f},{20,1.f},{wideSample,1.f}};
    auto currentCandidates=candidates;
    suggestions.canCommit=[&]{return suggestions.matches(currentCandidates,0,30);};
    int changes=0;
    suggestions.onChange=[&]{++changes;};
    suggestions.display(candidates,0,30,std::nullopt);
    REQUIRE(suggestions.getNumItems()==2 && suggestions.selectedSample()==10 && changes==0);
    popup=inspect(suggestions,1);
    suggestions.display(candidates,0,30,suggestions.selectedSample());
    REQUIRE(suggestions.activePopup()==popup && suggestions.selectedSample()==10);
    popup->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));pump();
    REQUIRE(!suggestions.isPopupActive() && suggestions.selectedSample()==10 && changes==0);
    popup=inspect(suggestions,1);
    popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();
    REQUIRE(suggestions.selectedSample()==20 && changes==1);

    std::reverse(candidates.begin(),candidates.end());
    suggestions.display(candidates,0,30,suggestions.selectedSample());
    REQUIRE(suggestions.selectedSample()==20 && changes==1);
    suggestions.display(candidates,0,30,wideSample);
    REQUIRE(suggestions.selectedSample()==wideSample && suggestions.getNumItems()==3 && changes==1);
    currentCandidates=candidates;
    popup=inspect(suggestions,1);
    currentCandidates[1].sample=25;
    popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();
    REQUIRE(suggestions.selectedSample()==wideSample && changes==1);
    candidates.erase(candidates.begin());
    suggestions.display(candidates,0,30,suggestions.selectedSample());
    REQUIRE(suggestions.selectedSample()==20 && changes==1);
    suggestions.display({},0,30,suggestions.selectedSample());
    REQUIRE(!suggestions.isEnabled() && !suggestions.selectedSample() && changes==1);
}
