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
    auto replacement=original;replacement.sourceEnd+=480;replacement.render={};replacement.ir={};replacement.soundPreset={};
    auto sourceOnly=baseline;*m3::find(sourceOnly,firstId)=replacement;engine.restore(sourceOnly);panel->refresh(); // Source-only epoch oracle; no wet readiness is implied.
    REQUIRE(engine.edit(firstId,[](auto& w,auto& error){return m3::setSource(w,w.sourceStart,w.sourceEnd+2400,error);}));panel->refresh();reset("sourceSampleEnd");REQUIRE(current().sourceEnd==replacement.sourceEnd);
    // The real asynchronous popup action, including focus loss to the menu.
    load(original);NativeEditorPeer nativePeer(*panel,"ReverbArise - PerformKnobResetCases native validation");panel->setVisible(true);panel->toFront(true);
    // JUCE keyboard focus alone does not make this an active native window.
    // The popup dismisses when its owning process is in the background.
    auto* peer=panel->getPeer();REQUIRE(peer);
   #if JUCE_WINDOWS
    // JUCE's Windows toFront/grabFocus use BringWindowToTop/SetFocus, which
    // do not request foreground activation for a background CTest process.
    ::SetForegroundWindow(static_cast<HWND>(peer->getNativeHandle()));
   #endif
    peer->grabFocus();
    const auto focusDeadline=juce::Time::getMillisecondCounter()+30000; // Allow the interactive native runner to grant real foreground focus.
    while((!peer->isFocused() || !juce::Process::isForegroundProcess()) && juce::Time::getMillisecondCounter()<focusDeadline)
        juce::MessageManager::getInstance()->runDispatchLoopUntil(10);
    if(!peer->isFocused() || !juce::Process::isForegroundProcess())
        std::cerr<<"Reset fixture native focus unavailable: peer="<<peer->isFocused()<<" foreground="<<juce::Process::isForegroundProcess()<<"\n";
    REQUIRE(peer->isFocused() && juce::Process::isForegroundProcess());
    using controls::ChoicePopup;
    const auto pump = [] { juce::MessageManager::getInstance()->runDispatchLoopUntil(50); };
    const auto openMenu = [&](const char* name)
    {
        auto& dial = cell(name).valueKnob();
        auto& inspector = require<juce::Viewport>(*panel, "inspectorViewport");
        const auto bounds = inspector.getViewedComponent()->getLocalArea(&dial, dial.getLocalBounds());
        inspector.setViewPosition(0, bounds.getCentreY() - inspector.getHeight() / 2);
        const auto click = event(dial, juce::ModifierKeys::rightButtonModifier, 1);
        const auto point = click.getScreenPosition();
        dial.mouseDown(click);
        dial.mouseUp(event(dial, {}, 1));

        auto* popup = dynamic_cast<ChoicePopup*>(juce::Component::getCurrentlyModalComponent());
        REQUIRE(popup && popup->getName() == "knobResetMenu" && popup->hasKeyboardFocus(true));
        const auto& rows = popup->state().items();
        REQUIRE(rows.size() == 1 && rows[0].role == controls::PopupRowRole::action && !rows[0].hasSelectionMarker());
        REQUIRE(popup->state().inspected() == -1);
        const juce::Rectangle<int> anchor{point.x, point.y, 1, 1};
        const auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect(anchor);
        REQUIRE(display && popup->getScreenBounds() == ChoicePopup::placedBounds(anchor, display->userArea, popup->getWidth(), popup->getHeight()));
        return popup;
    };
    const auto pressKey = [](ChoicePopup& popup, int key)
    {
        popup.keyPressed(juce::KeyPress(juce::KeyPress::downKey));
        popup.keyPressed(juce::KeyPress(key));
    };
    const auto cornerInk = [](juce::Slider& dial)
    {
        juce::Image image(juce::Image::RGB, dial.getWidth(), dial.getHeight(), true, juce::SoftwareImageType{});
        juce::Graphics g(image);
        g.fillAll(background);
        dial.paint(g);
        return image.getPixelAt(1, 1);
    };
    auto& level = cell("swellLevel").valueKnob();
    auto* popup = openMenu("swellLevel");
    REQUIRE(current().shape.gainDb == -7 && popup->state().items()[0].label == "Reset to default (0 dB)");
    const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_PERFORM_KNOB_OUTPUT",{});
    if(output.isNotEmpty()){juce::File folder(output);REQUIRE(folder.createDirectory().wasOk());auto stream=folder.getChildFile("knob-reset-menu.png").createOutputStream();REQUIRE(stream && juce::PNGImageFormat().writeImageToStream(popup->createComponentSnapshot(popup->getLocalBounds()),*stream));}
    pressKey(*popup, juce::KeyPress::returnKey);
    pump();
    REQUIRE(current().shape.gainDb == 0 && level.hasKeyboardFocus(false) && cornerInk(level) == aqua);

    load(original);
    popup = openMenu("swellLevel");
    popup->mouseDown(event(*popup, juce::ModifierKeys::leftButtonModifier, 1));
    popup->mouseUp(event(*popup, {}, 1));
    REQUIRE(popup->state().done());
    popup->activate(0);
    pump();
    REQUIRE(current().shape.gainDb == 0);
    REQUIRE(level.hasKeyboardFocus(false));
    REQUIRE(cornerInk(level) != aqua);

    load(original);
    popup = openMenu("preRoll");
    auto* actionHandler = popup->rowComponent(0).getAccessibilityHandler();
    REQUIRE(actionHandler && actionHandler->getActions().invoke(juce::AccessibilityActionType::press));
    pump();
    REQUIRE(current().shape.automaticStart && current().shape.startOffset == 0);

    load(original);
    REQUIRE(engine.edit(firstId, [](auto& w, auto& error) { return m3::moveArrival(w, w.arrival + 6000, error); }));
    panel->refresh();
    popup = openMenu("arrivalSample");
    REQUIRE(popup->state().items()[0].label == "Reset to initial position");
    pressKey(*popup, juce::KeyPress::spaceKey);
    pump();
    REQUIRE(current().arrival == original.arrival);

    load(original);
    popup = openMenu("swellLevel");
    popup->inputAttemptWhenModal();
    pump();
    REQUIRE(current().shape.gainDb == -7 && juce::Component::getCurrentlyModalComponent() == nullptr);

    REQUIRE(engine.selectTargets({secondId}, secondId)); // The editor still displays the old selection.
    popup = openMenu("swellLevel");
    REQUIRE(!popup->state().items()[0].eligible && popup->state().items()[0].label.contains("selection changed"));
    pressKey(*popup, juce::KeyPress::returnKey);
    REQUIRE(!popup->state().done());
    popup->dismiss();
    pump();
    REQUIRE(current().shape.gainDb == -7);

    auto legacy = original;
    legacy.shape.gainDb = -33;
    load(legacy);
    popup = openMenu("swellLevel");
    popup->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
    pump();
    REQUIRE(current().shape.gainDb == -33);

    load(original);
    reset("swellLevel");
    struct Notifications final:juce::AudioProcessorListener
    {int changes=0;void audioProcessorParameterChanged(juce::AudioProcessor*,int,float)override{}
        void audioProcessorChanged(juce::AudioProcessor*,const ChangeDetails& detail)override{if(detail.nonParameterStateChanged)++changes;}} notifications;
    processor->flushHostStateChangesForTesting();processor->addListener(&notifications);const auto generation=current().shapeGeneration,renders=engine.snapshot().renderCount;
    reset("swellLevel");processor->flushHostStateChangesForTesting();processor->removeListener(&notifications);
    REQUIRE(notifications.changes==0 && current().shapeGeneration==generation && engine.snapshot().renderCount==renders);
    for(int stale=0;stale<8;++stale)
    {
        load(original);
        popup = openMenu("swellLevel");
        REQUIRE(popup->state().items()[0].eligible);
        popup->activate(0); // Queue the real popup result before the context changes.
        if(stale==0)REQUIRE(engine.selectTargets({secondId},secondId));
        if(stale==1){auto state=engine.savedSession();engine.restore(state);}
        if(stale==2)REQUIRE(processor->requestWorkflowMode(workflow::Mode::finalise));
        if(stale==3)REQUIRE(engine.edit(firstId,[](auto& w,auto& error){return m3::setSource(w,w.sourceStart-1,w.sourceEnd,error);}));
        if(stale==4)panel.reset();
        const auto position=head.position;
        if(stale==5){const auto ppq=head.position.getPpqPosition();head.position.setPpqPosition((ppq?*ppq:0)+1);juce::AudioBuffer<float> quiet(2,1);quiet.clear();juce::MidiBuffer events;processor->processBlock(quiet,events);}
        if(stale==6){auto& other=cell("curveInTension").valueKnob();other.onDragStart();other.onDragEnd();}
        if(stale==7)REQUIRE(engine.setFrozen(firstId,true));
        const auto safe=current();pump();REQUIRE(current().shape==safe.shape && current().placement==safe.placement);
        REQUIRE(juce::Component::getCurrentlyModalComponent() == nullptr);
        if(stale==2)REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));
        if(stale==4){panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);panel->stopAutomaticRefreshForReview();panel->setSize(1100,850);nativePeer.setContentNonOwned(panel.get(),true);}
        if(stale==5){head.position=position;juce::AudioBuffer<float> quiet(2,1);quiet.clear();juce::MidiBuffer events;processor->processBlock(quiet,events);}
    }
    load(original);cell("swellLevel").setEnabled(false);REQUIRE(!cell("swellLevel").prepareResetAction().apply);
    cell("swellLevel").setEnabled(true);
    openMenu("swellLevel");
    panel.reset();
    pump();
    REQUIRE(juce::Component::getCurrentlyModalComponent() == nullptr && current().shape.gainDb == -7);
    REQUIRE(processor->getCaptureSnapshot().manifest.revision==captureRevision);
}
