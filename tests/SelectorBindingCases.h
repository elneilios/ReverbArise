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
    juce::Component settingsHost;auto& tabs=dynamic_cast<juce::TabbedComponent&>(*panel->settingsPanel());
    settingsHost.addAndMakeVisible(tabs);settingsHost.setBounds(120,120,760,600);tabs.setBounds(settingsHost.getLocalBounds());
    settingsHost.addToDesktop(0);settingsHost.setVisible(true);settingsHost.toFront(true);tabs.setCurrentTabIndex(2);pump();
    auto& in=require<ChoiceSelector>(*tabs.getTabContentComponent(2),"fadeInCurve");
    auto& out=require<ChoiceSelector>(*tabs.getTabContentComponent(2),"fadeOutCurve");
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
    tabs.setCurrentTabIndex(0);pump();
    auto& routing=require<ChoiceSelector>(*tabs.getTabContentComponent(0),"liveRouting");
    auto& policy=require<ChoiceSelector>(*tabs.getTabContentComponent(0),"gainPolicy");
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
}
