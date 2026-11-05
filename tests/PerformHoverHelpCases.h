#pragma once
void Composition::verifyHoverHelp()
{
    stopTimer();setScene(0);auto& engine=processor->swellEngine();
    auto panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);panel->stopAutomaticRefreshForReview();panel->setSize(1100,850);panel->refresh();
    panel->addToDesktop(juce::ComponentPeer::windowIsTemporary);panel->setVisible(true);panel->toFront(true);
    auto& help=require<controls::EllipsisLabel>(*panel,"hoverHelp");const auto original=engine.savedSession();const auto renders=engine.snapshot().renderCount,capture=processor->getCaptureSnapshot().manifest.revision;
    auto stamp=juce::Time::currentTimeMillis();
    const auto event=[&](juce::Component& control,juce::Point<float> position)
    {const juce::Time time(++stamp);return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),position,{},0,0,0,0,0,&control,&control,time,position,time,0,false);};
    const auto hover=[&](juce::Component& control){panel->mouseEnter(event(control,control.getLocalBounds().getCentre().toFloat()));};
    REQUIRE(help.getBottom()==panel->getHeight() && help.getHeight()==textStyle(TextRole::helper).lineHeight);
    REQUIRE(help.getFont().getHeight()==textStyle(TextRole::helper).height && !help.getBounds().intersects(panel->timelineComponent().getBounds()));
    REQUIRE(require<juce::Component>(*panel,"hostInfo").getBottom()<=help.getY());
    struct Notifications final:juce::AudioProcessorListener
    {int changes=0;void audioProcessorParameterChanged(juce::AudioProcessor*,int,float)override{}
        void audioProcessorChanged(juce::AudioProcessor*,const ChangeDetails& detail)override{if(detail.nonParameterStateChanged)++changes;}} notifications;
    processor->flushHostStateChangesForTesting();processor->addListener(&notifications);
    std::vector<juce::Component*> providers;
    const auto collect=[&](auto&& self,juce::Component& component)->void
    {
        if(auto* client=dynamic_cast<juce::TooltipClient*>(&component);client && client->getTooltip().isNotEmpty() && component.isShowing() && !component.getProperties().contains("contextHelp"))providers.push_back(&component);
        for(auto* child:component.getChildren())self(self,*child);
    };collect(collect,*panel);REQUIRE(providers.size()>=20);
    for(auto* component:providers)
    {auto* client=dynamic_cast<juce::TooltipClient*>(component);const auto expected=client->getTooltip().replace("\r"," ").replace("\n"," ").trim();hover(*component);REQUIRE(help.getText()==expected && !help.getText().containsChar('\n'));}
    auto& list=require<controls::SwellList>(*panel,"swellList");hover(list);REQUIRE(help.getText().contains("Select a swell"));
    auto& timeline=panel->timelineComponent();const auto wet=timeline.wetLane().getCentre();const auto move=event(timeline,wet);timeline.mouseMove(move);panel->mouseMove(move);
    REQUIRE(help.getText().contains("wet") && help.getText().contains("Ctrl"));
    const auto kept=help.getText();panel->mouseMove(event(*panel,help.getBounds().getCentre().toFloat()));REQUIRE(help.getText()==kept);
    panel->mouseExit(event(*panel,{-100,-100}));REQUIRE(help.getText().isEmpty());
    auto transient=std::make_unique<juce::TextButton>("Transient");transient->setTooltip("Transient control help.");panel->addAndMakeVisible(*transient);transient->setBounds(10,10,50,30);hover(*transient);REQUIRE(help.getText()=="Transient control help.");
    transient.reset();panel->refresh();REQUIRE(help.getText().isEmpty());
    juce::MessageManager::getInstance()->runDispatchLoopUntil(20);processor->flushHostStateChangesForTesting();processor->removeListener(&notifications);
    REQUIRE(notifications.changes==0 && engine.snapshot().renderCount==renders && engine.savedSession().performUi==original.performUi);
    REQUIRE(engine.savedSession().swells[0].shape==original.swells[0].shape && processor->getCaptureSnapshot().manifest.revision==capture);
    auto& settings=require<WorkflowHeader>(*panel,"workflowHeader").settingsAction();settings.onClick();panel->refresh();
    auto& live=require<juce::Slider>(*panel->settingsPanel(),"liveWetLevel");hover(live);REQUIRE(help.getText().isNotEmpty());settings.onClick();panel->refresh();
    for(const auto size:{juce::Point<int>{1100,850},juce::Point<int>{1400,900},juce::Point<int>{1400,1000},juce::Point<int>{1600,1000}})
    {
        panel->setSize(size.x,size.y);
        for(const auto mode:{workflow::Mode::capture,workflow::Mode::live,workflow::Mode::finalise})
        {
            REQUIRE(processor->requestWorkflowMode(mode));panel->refresh();REQUIRE(help.isVisible() && help.getBottom()==size.y && panel->getLocalBounds().contains(help.getBounds()));
            if(mode!=workflow::Mode::live){const auto& workspace=require<juce::Component>(*panel,mode==workflow::Mode::capture?"learnWorkspace":"exportWorkspace");REQUIRE(workspace.getBottom()<=help.getY());}
        }
        REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));
    }
    panel->removeFromDesktop();engine.restore(original);wait([&]{return engine.snapshot().auditionReady;});
}
