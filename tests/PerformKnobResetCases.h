#pragma once
void Composition::verifyKnobResets()
{
    stopTimer();setScene(0);auto& engine=processor->swellEngine();
    auto original=*m3::find(baseline,firstId);original.placement=2300;original.shape.automaticStart=false;
    original.shape.startOffset=-18000;original.shape.endOffset=18000;original.shape.fadeIn=original.shape.fadeOut=6000;
    original.shape.gainDb=-7;original.shape.inTension=.4;original.shape.outTension=-.3;
    auto panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);panel->stopAutomaticRefreshForReview();panel->setSize(1100,850);
    const auto load=[&](m3::Swell chosen)
    {auto state=baseline;*m3::find(state,firstId)=chosen;engine.restore(state);wait([&]{return engine.snapshot().raw && engine.snapshot().auditionReady;});panel->refresh();panel->timelineComponent().setSnap(true);};
    const auto cell=[&](const char* name)->KnobValue&{return require<KnobValue>(*panel,juce::String(name)+"Cell");};
    const auto current=[&]{return *m3::find(engine.savedSession(),firstId);};
    auto stamp=juce::Time::currentTimeMillis();
    const auto event=[&](juce::Component& dial,juce::ModifierKeys mods,int clicks)
    {const auto point=dial.getLocalBounds().getCentre().toFloat();const juce::Time time(++stamp);return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,mods,0,0,0,0,0,&dial,&dial,time,point,time,clicks,false);};
    const auto reset=[&](const char* name)
    {auto& dial=cell(name).valueKnob();dial.mouseDown(event(dial,juce::ModifierKeys::leftButtonModifier,1));dial.mouseUp(event(dial,{},1));
        dial.mouseDown(event(dial,juce::ModifierKeys::leftButtonModifier,2));dial.mouseDoubleClick(event(dial,juce::ModifierKeys::leftButtonModifier,2));dial.mouseUp(event(dial,{},2));};
    const auto captureRevision=processor->getCaptureSnapshot().manifest.revision;
    for(const auto* name:{"swellLevel","curveInTension","curveOutTension","wetPlacement","fadeInMs","fadeOutMs","endOffset","preRoll"})
    {
        load(original);const auto before=current();reset(name);const auto after=current();
        if(juce::String(name)=="swellLevel")REQUIRE(after.shape.gainDb==0);
        if(juce::String(name)=="curveInTension")REQUIRE(after.shape.inTension==0 && after.shape.inCurve==before.shape.inCurve);
        if(juce::String(name)=="curveOutTension")REQUIRE(after.shape.outTension==0 && after.shape.outCurve==before.shape.outCurve);
        if(juce::String(name)=="wetPlacement")REQUIRE(after.placement==0 && after.shape==before.shape);
        if(juce::String(name)=="fadeInMs")REQUIRE(after.shape.fadeIn==0);
        if(juce::String(name)=="fadeOutMs")REQUIRE(after.shape.fadeOut==960);
        if(juce::String(name)=="endOffset")REQUIRE(after.shape.endOffset+after.placement==960); // 20 ms, unsnapped.
        if(juce::String(name)=="preRoll")REQUIRE(after.shape.automaticStart && after.shape.startOffset==0);
        REQUIRE(after.id==before.id && after.ir==before.ir && after.render==before.render && after.arrival==before.arrival && after.sourceStart==before.sourceStart && after.sourceEnd==before.sourceEnd);
        REQUIRE(panel->timelineComponent().requestedSnap());
    }
    load(original);REQUIRE(engine.edit(firstId,[](auto& w,auto& error){return m3::setSource(w,w.sourceStart-2400,w.sourceEnd+2400,error);}));panel->refresh();
    reset("sourceSampleStart");REQUIRE(current().sourceStart==original.sourceStart && current().sourceEnd==original.sourceEnd+2400);
    reset("sourceSampleEnd");REQUIRE(current().sourceEnd==original.sourceEnd);
    REQUIRE(engine.edit(firstId,[](auto& w,auto& error){return m3::moveArrival(w,w.arrival+6000,error);}));panel->refresh();reset("arrivalSample");REQUIRE(current().arrival==original.arrival);
    load(original);REQUIRE(engine.edit(firstId,[](auto& w,auto& error){return m3::moveArrival(w,w.arrival-24000,error);}));panel->refresh();
    const auto constrained=current();reset("sourceSampleStart");REQUIRE(current().arrival==constrained.arrival && current().sourceStart==constrained.sourceStart && current().sourceEnd==constrained.sourceEnd);
    auto replacement=original;replacement.sourceEnd+=480;replacement.render={};replacement.ir={};
    auto sourceOnly=baseline;*m3::find(sourceOnly,firstId)=replacement;engine.restore(sourceOnly);panel->refresh(); // Source-only epoch oracle; no wet readiness is implied.
    REQUIRE(engine.edit(firstId,[](auto& w,auto& error){return m3::setSource(w,w.sourceStart,w.sourceEnd+2400,error);}));panel->refresh();reset("sourceSampleEnd");REQUIRE(current().sourceEnd==replacement.sourceEnd);
    // The real asynchronous popup action, including focus loss to the menu.
    load(original);panel->addToDesktop(juce::ComponentPeer::windowIsTemporary);panel->setVisible(true);
    auto& inspector=require<juce::Viewport>(*panel,"inspectorViewport");inspector.setViewPosition(0,inspector.getViewedComponent()->getHeight()-inspector.getHeight());
    auto& level=cell("swellLevel").valueKnob();panel->toFront(true);level.grabKeyboardFocus();juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    REQUIRE(level.hasKeyboardFocus(false));const auto menuClick=event(level,juce::ModifierKeys::rightButtonModifier,1);level.mouseDown(menuClick);level.mouseUp(event(level,{},1));
    juce::Component* popup=nullptr;auto& desktop=juce::Desktop::getInstance();
    for(int attempt=0;attempt<50 && !popup;++attempt)
    {juce::MessageManager::getInstance()->runDispatchLoopUntil(10);for(int n=0;n<desktop.getNumComponents();++n)if(desktop.getComponent(n)->getName()=="menu")popup=desktop.getComponent(n);}
    if(!popup){std::cerr<<"Reset popup missing; Level enabled="<<level.isEnabled()<<" showing="<<level.isShowing()<<" gain="<<current().shape.gainDb<<" dial="<<level.getScreenBounds().toString()<<" viewport="<<inspector.getScreenBounds().toString()<<"\n";
        for(int n=0;n<desktop.getNumComponents();++n)std::cerr<<"Desktop: "<<desktop.getComponent(n)->getName()<<" "<<desktop.getComponent(n)->getScreenBounds().toString()<<"\n";}
    REQUIRE(popup && current().shape.gainDb==-7);
    REQUIRE(popup->getScreenBounds().expanded(spacing::related).contains(menuClick.getScreenPosition()));
    const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_PERFORM_KNOB_OUTPUT",{});
    if(output.isNotEmpty()){juce::File folder(output);REQUIRE(folder.createDirectory().wasOk());auto stream=folder.getChildFile("knob-reset-menu.png").createOutputStream();REQUIRE(stream && juce::PNGImageFormat().writeImageToStream(popup->createComponentSnapshot(popup->getLocalBounds()),*stream));}
    popup->keyPressed(juce::KeyPress(juce::KeyPress::downKey));popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(50);
    REQUIRE(current().shape.gainDb==0);juce::PopupMenu::dismissAllActiveMenus();
    load(original);level.mouseDown(event(level,juce::ModifierKeys::rightButtonModifier,1));level.mouseUp(event(level,{},1));
    juce::PopupMenu::dismissAllActiveMenus();juce::MessageManager::getInstance()->runDispatchLoopUntil(20);REQUIRE(current().shape.gainDb==-7);
    reset("swellLevel");panel->removeFromDesktop();
    struct Notifications final:juce::AudioProcessorListener
    {int changes=0;void audioProcessorParameterChanged(juce::AudioProcessor*,int,float)override{}
        void audioProcessorChanged(juce::AudioProcessor*,const ChangeDetails& detail)override{if(detail.nonParameterStateChanged)++changes;}} notifications;
    processor->flushHostStateChangesForTesting();processor->addListener(&notifications);const auto generation=current().shapeGeneration,renders=engine.snapshot().renderCount;
    reset("swellLevel");processor->flushHostStateChangesForTesting();processor->removeListener(&notifications);
    REQUIRE(notifications.changes==0 && current().shapeGeneration==generation && engine.snapshot().renderCount==renders);
    for(int stale=0;stale<8;++stale)
    {
        load(original);auto action=cell("swellLevel").prepareResetAction();REQUIRE(action.apply);
        if(stale==0)REQUIRE(engine.selectTargets({secondId},secondId));
        if(stale==1){auto state=engine.savedSession();engine.restore(state);}
        if(stale==2)REQUIRE(processor->requestWorkflowMode(workflow::Mode::finalise));
        if(stale==3)REQUIRE(engine.edit(firstId,[](auto& w,auto& error){return m3::setSource(w,w.sourceStart-1,w.sourceEnd,error);}));
        if(stale==4)panel.reset();
        const auto position=head.position;
        if(stale==5){const auto ppq=head.position.getPpqPosition();head.position.setPpqPosition((ppq?*ppq:0)+1);juce::AudioBuffer<float> quiet(2,1);quiet.clear();juce::MidiBuffer events;processor->processBlock(quiet,events);}
        if(stale==6){auto& other=cell("curveInTension").valueKnob();other.onDragStart();other.onDragEnd();}
        if(stale==7)REQUIRE(engine.setFrozen(firstId,true));
        const auto safe=current();action.apply();REQUIRE(current().shape==safe.shape && current().placement==safe.placement);
        if(stale==2)REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));
        if(stale==4){panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);panel->stopAutomaticRefreshForReview();}
        if(stale==5){head.position=position;juce::AudioBuffer<float> quiet(2,1);quiet.clear();juce::MidiBuffer events;processor->processBlock(quiet,events);}
    }
    load(original);cell("swellLevel").setEnabled(false);REQUIRE(!cell("swellLevel").prepareResetAction().apply);
    cell("swellLevel").setEnabled(true);panel->addToDesktop(juce::ComponentPeer::windowIsTemporary);panel->setVisible(true);
    auto& visible=require<juce::Viewport>(*panel,"inspectorViewport");visible.setViewPosition(0,10000);
    auto& closing=cell("swellLevel").valueKnob();closing.mouseDown(event(closing,juce::ModifierKeys::rightButtonModifier,1));closing.mouseUp(event(closing,{},1));
    panel.reset();juce::PopupMenu::dismissAllActiveMenus();juce::MessageManager::getInstance()->runDispatchLoopUntil(20);REQUIRE(current().shape.gainDb==-7);
    REQUIRE(processor->getCaptureSnapshot().manifest.revision==captureRevision);
}
