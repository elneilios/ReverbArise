#pragma once
void Composition::verifyKnobs()
{
    stopTimer();setScene(0);auto& engine=processor->swellEngine();
    {
        KnobValue empty;empty.setSize(180,64);empty.configure("Unavailable timing (s)","emptyTiming","Timing unavailable");empty.display("empty","",0,false);
        empty.displayDomain({0,-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity(),0,0,{}});
        empty.addToDesktop(juce::ComponentPeer::windowIsTemporary);empty.setVisible(true);auto* handler=empty.valueKnob().getAccessibilityHandler();REQUIRE(handler);
        auto* value=handler->getValueInterface();REQUIRE(value && value->isReadOnly() && std::isfinite(value->getCurrentValue()) && !value->getRange().isValid());
        empty.displayDomain({0,0,0,1,1,"0.000"});REQUIRE(value->isReadOnly() && value->getCurrentValueAsString()=="0.000" && !value->getRange().isValid());empty.removeFromDesktop();
    }
    auto original=*m3::find(baseline,firstId);original.placement=2300;original.shape.automaticStart=false;
    original.shape.startOffset=-18000;original.shape.endOffset=18000;original.shape.fadeIn=original.shape.fadeOut=6000;
    auto panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);panel->stopAutomaticRefreshForReview();panel->setSize(1100,850);
    const auto load=[&](m3::Swell swell)
    {auto state=baseline;*m3::find(state,firstId)=swell;engine.restore(state);wait([&]{const auto s=engine.snapshot();return s.raw && s.auditionReady && m3::find(s.session,firstId)->phase==m3::Phase::ready;});panel->refresh();
        panel->timelineComponent().setViewport({0,96000});panel->timelineComponent().setSnap(true);panel->modifierKeysChanged({});panel->refresh();};
    const auto cell=[&](const char* name)->KnobValue&{return require<KnobValue>(*panel,juce::String(name)+"Cell");};
    const auto current=[&]{return *m3::find(engine.savedSession(),firstId);};
    const auto at=[&](m3::TimingAnchor kind){return *m3::timingAnchorSample(current(),kind,engine.snapshot().raw->start);};
    const auto change=[&](KnobValue& value,double delta){auto& dial=value.valueKnob();dial.onDragStart();dial.setValue(dial.getValue()+delta,juce::sendNotificationSync);dial.onDragEnd();};
    struct Case{const char* name;m3::TimingAnchor kind;double delta;int64_t expected;};
    using A=m3::TimingAnchor;
    // Independent 120-BPM quarter-beat oracle: grid interval is 6000 samples.
    for(const auto test:{Case{"preRoll",A::start,-.04375,36000},Case{"endOffset",A::end,-.0416666666667,66000},Case{"fadeInMs",A::fadeIn,.1166699075,42000},Case{"fadeOutMs",A::fadeOut,.1111141976,60000},Case{"wetPlacement",A::move,.04375,36000}})
    {load(original);change(cell(test.name),test.delta);REQUIRE(at(test.kind)==test.expected);REQUIRE(current().render==original.render && current().ir==original.ir);}
    for(const auto test:{Case{"arrivalSample",A::arrival,.046875,54000},Case{"sourceSampleStart",A::sourceStart,-.046875,42000},Case{"sourceSampleEnd",A::sourceEnd,.046875,78000}})
    {load(original);change(cell(test.name),test.delta);REQUIRE(at(test.kind)==test.expected);}
    load(original);auto& end=cell("endOffset");auto& dial=end.valueKnob();dial.onDragStart();const auto raw=dial.getValue();
    dial.setValue(raw+.04375,juce::sendNotificationSync);REQUIRE(at(A::end)==72000);const auto accepted=current();
    dial.modifierKeysChanged(juce::ModifierKeys::ctrlModifier|juce::ModifierKeys::shiftModifier);REQUIRE(current().shape==accepted.shape);
    dial.setValue(dial.getValue()+.001,juce::sendNotificationSync);REQUIRE(at(A::end)==72096);
    const auto beforeKeys=current();panel->modifierKeysChanged({});REQUIRE(current().shape==beforeKeys.shape);dial.onDragEnd();
    // A no-grid domain retains the accepted dial pixels, not the raw pointer value.
    auto tiny=original;tiny.shape.startOffset=-100;tiny.shape.endOffset=100;tiny.shape.fadeIn=tiny.shape.fadeOut=0;
    load(tiny);auto& fade=cell("fadeInMs");auto& fadeDial=fade.valueKnob();const auto before=fadeDial.createComponentSnapshot(fadeDial.getLocalBounds());
    fadeDial.onDragStart();fadeDial.setValue(.05,juce::sendNotificationSync);REQUIRE(current().shape.fadeIn==0 && fade.displayedFraction()==0);
    const auto after=fadeDial.createComponentSnapshot(fadeDial.getLocalBounds());
    for(int y=0;y<before.getHeight();++y)for(int x=0;x<before.getWidth();++x)REQUIRE(before.getPixelAt(x,y)==after.getPixelAt(x,y));
    panel->modifierKeysChanged(juce::ModifierKeys::ctrlModifier);fadeDial.setValue(.1,juce::sendNotificationSync);REQUIRE(current().shape.fadeIn==10);fadeDial.onDragEnd();
    load(original);auto& value=cell("endOffset");auto& input=value.valueField();input.setText("0.123",false);input.onReturnKey();
    REQUIRE(at(A::end)==53904); // Exact entry ignores Snap and placement is inverted.
    // Real pointer/wheel/keyboard input uses the actual shared component.
    load(original);auto& actual=cell("endOffset").valueKnob();const auto start=actual.getLocalBounds().getCentre().toFloat();auto stamp=juce::Time::currentTimeMillis();
    const auto event=[&](juce::Point<float> point,juce::ModifierKeys mods){const juce::Time now(++stamp);return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,mods,0,0,0,0,0,&actual,&actual,now,start,now,1,point!=start);};
    actual.mouseDown(event(start,juce::ModifierKeys::leftButtonModifier));actual.mouseDrag(event(start.translated(0,-20),juce::ModifierKeys::leftButtonModifier));
    REQUIRE(at(A::end)%6000==0);actual.mouseUp(event(start.translated(0,-20),{}));
    juce::MouseWheelDetails wheel{};wheel.deltaY=.1f;actual.mouseWheelMove(event(start,{}),wheel);REQUIRE(at(A::end)%6000==0);
    REQUIRE(actual.keyPressed(juce::KeyPress(juce::KeyPress::rightKey,juce::ModifierKeys::ctrlModifier,0)));REQUIRE(actual.getWantsKeyboardFocus());
    std::array<int64_t,2> pointerChanges{};
    for(int fine=0;fine<2;++fine)
    {load(original);const auto flags=juce::ModifierKeys::leftButtonModifier|juce::ModifierKeys::ctrlModifier|(fine?juce::ModifierKeys::shiftModifier:0);const auto beforePointer=at(A::end);
        actual.mouseDown(event(start,flags));actual.mouseDrag(event(start.translated(0,-20),flags));pointerChanges[static_cast<size_t>(fine)]=at(A::end)-beforePointer;actual.mouseUp(event(start.translated(0,-20),{}));}
    REQUIRE(pointerChanges[0]>pointerChanges[1] && pointerChanges[1]>0);
    // Accessibility reports domain values and friendly names, never the dial fraction.
    panel->addToDesktop(juce::ComponentPeer::windowIsTemporary);panel->setVisible(true);actual.grabKeyboardFocus();
    REQUIRE(actual.hasKeyboardFocus(false));auto* handler=actual.getAccessibilityHandler();REQUIRE(handler && handler->getTitle().contains("End Offset"));
    auto* accessible=handler->getValueInterface();REQUIRE(accessible && accessible->getCurrentValueAsString()==m3::ui::preciseValue(accessible->getCurrentValue()));
    const auto position=at(A::end);REQUIRE(std::abs(accessible->getCurrentValue()-(position-current().arrival)/48000.)<1.e-8);
    panel->modifierKeysChanged(juce::ModifierKeys::ctrlModifier);accessible->setValue(.321);REQUIRE(at(A::end)==current().arrival+15408);
    const auto focus=actual.createComponentSnapshot(actual.getLocalBounds());REQUIRE(focus.getPixelAt(1,actual.getHeight()/2)!=aqua);
    for(const auto point:{juce::Point<int>{1,1},{actual.getWidth()-2,1},{1,actual.getHeight()-2},{actual.getWidth()-2,actual.getHeight()-2}})REQUIRE(focus.getPixelAt(point.x,point.y)==aqua);
    const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_PERFORM_KNOB_OUTPUT",{});
    if(output.isNotEmpty())
    {juce::File folder(output);REQUIRE(folder.createDirectory().wasOk());auto stream=folder.getChildFile("knob-focus.png").createOutputStream();
        REQUIRE(stream && juce::PNGImageFormat().writeImageToStream(actual.createComponentSnapshot(actual.getLocalBounds(),true,3.f),*stream));
        stream=folder.getChildFile("knob-context.png").createOutputStream();REQUIRE(stream && juce::PNGImageFormat().writeImageToStream(panel->createComponentSnapshot(panel->getLocalBounds()),*stream));}
    panel->removeFromDesktop();
    // Fixed unsnapped wheel/arrow increments do not depend on the current zoom.
    for(const auto span:{96000.,4800.})
    {
        load(original);panel->timelineComponent().setViewport({0,span});panel->timelineComponent().setSnap(false);panel->refresh();const auto beforeStep=at(A::end);
        actual.mouseWheelMove(event(start,{}),wheel);REQUIRE(at(A::end)==beforeStep+48);
        REQUIRE(actual.keyPressed(juce::KeyPress(juce::KeyPress::rightKey,juce::ModifierKeys::shiftModifier,0)));REQUIRE(at(A::end)==beforeStep+53);
    }
    load(original);actual.mouseWheelMove(event(start,{}),wheel);REQUIRE(at(A::end)==72000);
    actual.mouseWheelMove(event(start,{}),wheel);REQUIRE(at(A::end)==78000);
    actual.mouseWheelMove(event(start,juce::ModifierKeys::ctrlModifier|juce::ModifierKeys::shiftModifier),wheel);REQUIRE(at(A::end)==78005);
    load(original);panel->timelineComponent().setSnap(false);const auto beforeLevel=current().shape.gainDb;
    auto& level=cell("swellLevel").valueKnob();REQUIRE(level.keyPressed(juce::KeyPress(juce::KeyPress::rightKey,{},0)));
    REQUIRE(std::abs(current().shape.gainDb-beforeLevel-1)<1.e-5 && cell("swellLevel").valueField().getText()==m3::ui::compactValue(current().shape.gainDb,1));
    const auto lp=level.getLocalBounds().getCentre().toFloat();
    const auto levelWheel=[&](juce::ModifierKeys mods){const juce::Time time(++stamp);juce::MouseEvent input(juce::Desktop::getInstance().getMainMouseSource(),lp,mods,0,0,0,0,0,&level,&level,time,lp,time,0,false);level.mouseWheelMove(input,wheel);};
    levelWheel({});REQUIRE(std::abs(current().shape.gainDb-beforeLevel-2)<1.e-5);
    levelWheel(juce::ModifierKeys::shiftModifier);REQUIRE(std::abs(current().shape.gainDb-beforeLevel-2.1)<1.e-5);
    REQUIRE(level.keyPressed(juce::KeyPress(juce::KeyPress::leftKey,juce::ModifierKeys::shiftModifier,0)));REQUIRE(std::abs(current().shape.gainDb-beforeLevel-2)<1.e-5);
    auto& curve=cell("curveInTension").valueKnob();const auto beforeCurve=current().shape.inTension;
    REQUIRE(curve.keyPressed(juce::KeyPress(juce::KeyPress::rightKey,juce::ModifierKeys::shiftModifier,0)));REQUIRE(std::abs(current().shape.inTension-beforeCurve-.001)<1.e-8);
    REQUIRE(cell("preRoll").valueField().getText()==m3::ui::compactValue((current().arrival-at(A::start))/48000.,3));
    REQUIRE(cell("arrivalSample").getTitle().contains("bar.beat"));
    for(const auto* name:{"arrivalSample","sourceSampleStart","sourceSampleEnd","preRoll","endOffset","fadeInMs","fadeOutMs","wetPlacement","swellLevel","curveInTension","curveOutTension"})
        for(auto* child:cell(name).getChildren())if(auto* caption=dynamic_cast<juce::Label*>(child))
            REQUIRE(juce::GlyphArrangement::getStringWidth(caption->getFont(),caption->getText())<=caption->getWidth());
    // Complete labels keep Inter glyph proportions when names overflow.
    auto& status=require<controls::EllipsisLabel>(*panel,"selectedStatus");const auto savedText=status.getText();
    status.setText("Stale | A very long selected swell name that must retain its normal Inter proportions",juce::dontSendNotification);const auto longImage=status.createComponentSnapshot(status.getLocalBounds());
    status.setText("Stale | A very long selected swell name that",juce::dontSendNotification);const auto shortImage=status.createComponentSnapshot(status.getLocalBounds());
    for(int y=0;y<longImage.getHeight();++y)for(int x=0;x<40;++x)REQUIRE(longImage.getPixelAt(x,y)==shortImage.getPixelAt(x,y));
    REQUIRE(status.getFont().getHeight()==14 && status.getFont().getTypefacePtr()==interTypeface(InterFace::regular));status.setText(savedText,juce::dontSendNotification);
    // Unavailable accepted mapping blocks snapped timing, but allows Ctrl sample-coordinate editing.
    load(original);const auto savedPosition=head.position;const auto ppq=head.position.getPpqPosition();head.position.setPpqPosition((ppq?*ppq:0)+1);
    juce::AudioBuffer<float> quiet(2,1);quiet.clear();juce::MidiBuffer events;processor->processBlock(quiet,events);panel->refresh();
    auto& unavailable=cell("endOffset");const auto beforeUnavailable=current();change(unavailable,.01);REQUIRE(current().shape==beforeUnavailable.shape);
    panel->modifierKeysChanged(juce::ModifierKeys::ctrlModifier);change(unavailable,.01);REQUIRE(current().shape!=beforeUnavailable.shape);
    head.position=savedPosition;processor->processBlock(quiet,events);
    // Same-ID restore, secondary selection, mode, source and fixed-origin changes cancel stale input.
    for(int stale=0;stale<5;++stale)
    {
        load(original);auto& knob=cell("endOffset").valueKnob();knob.onDragStart();const auto initial=knob.getValue();
        if(stale==0){auto state=engine.savedSession();engine.restore(state);}
        if(stale==1)REQUIRE(engine.selectTargets({firstId,secondId},firstId));
        if(stale==2)REQUIRE(processor->requestWorkflowMode(workflow::Mode::finalise));
        if(stale==3)REQUIRE(engine.edit(firstId,[](auto& w,auto& error){return m3::setSource(w,w.sourceStart-1,w.sourceEnd,error);}));
        if(stale==4){const auto ppq=head.position.getPpqPosition();head.position.setPpqPosition((ppq?*ppq:0)+1);juce::AudioBuffer<float> stopped(2,1);stopped.clear();juce::MidiBuffer midi;processor->processBlock(stopped,midi);}
        const auto safe=current();knob.setValue(initial+.05,juce::sendNotificationSync);knob.onDragEnd();
        if(current().shape!=safe.shape || current().placement!=safe.placement)std::cerr<<"Stale knob case "<<stale<<" accepted an edit\n";
        REQUIRE(current().shape==safe.shape && current().placement==safe.placement);
        if(stale==2)REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));
    }
}
