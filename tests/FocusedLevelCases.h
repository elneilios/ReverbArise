#pragma once
void Composition::verifyFocusedLevel()
{
    stopTimer();setScene(0);auto& engine=processor->swellEngine();
    auto panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);panel->stopAutomaticRefreshForReview();
    panel->setSize(1100,850);
    panel->addToDesktop(juce::ComponentPeer::windowIsTemporary);panel->setVisible(true);
    auto& cell=require<KnobValue>(*panel,"swellLevelCell");auto& dial=cell.valueKnob();auto& field=cell.valueField();
    const auto current=[&]{return *m3::find(engine.savedSession(),firstId);};
    const auto load=[&](float db)
    {auto state=baseline;m3::find(state,firstId)->shape.gainDb=db;engine.restore(state);
        wait([&]{return engine.snapshot().raw && engine.snapshot().auditionReady;});panel->refresh();};
    const auto enter=[&](const juce::String& text){field.setText(text,false);field.onReturnKey();};
    const auto key=[&](bool up,bool fine=false){REQUIRE(dial.keyPressed(juce::KeyPress(up?juce::KeyPress::upKey:juce::KeyPress::downKey,fine?juce::ModifierKeys::shiftModifier:0,0)));};
    for(const auto db:{-12.f,0.f,12.f})
    {
        load(db);REQUIRE(std::abs(cell.displayedFraction()-(db+12)/24.)<1.e-8);
        const auto before=current();const auto count=engine.snapshot().renderCount;panel->refresh();dial.grabKeyboardFocus();panel->refresh();
        REQUIRE(current().shape==before.shape && current().inputGeneration==before.inputGeneration
            && current().shapeGeneration==before.shapeGeneration && engine.snapshot().renderCount==count);
    }
    load(0);dial.onDragStart();dial.setValue(.75,juce::sendNotificationSync);dial.onDragEnd();REQUIRE(current().shape.gainDb==6);
    {
        load(0);const auto before=current();const auto defaults=engine.savedSession().defaultIR;
        const auto neighbour=*m3::find(engine.savedSession(),secondId);const auto renders=engine.snapshot().renderCount;
        TestDirectory otherRoot;auto other=std::make_unique<PreverbAudioProcessor>(otherRoot.file);
        wait([&]{return other->swellEngine().savedSession().defaultIR.present();});
        const auto otherDefault=other->swellEngine().savedSession().defaultIR;
        enter("-30");wait([&]{return engine.snapshot().auditionReady;});
        REQUIRE(current().shapeGeneration>before.shapeGeneration && current().inputGeneration==before.inputGeneration
            && current().render==before.render && current().calibration==before.calibration && engine.snapshot().renderCount==renders);
        REQUIRE(engine.savedSession().defaultIR==defaults && m3::sameSavedSwell(*m3::find(engine.savedSession(),secondId),neighbour));
        REQUIRE(other->swellEngine().savedSession().swells.empty() && other->swellEngine().savedSession().defaultIR==otherDefault);
        REQUIRE(engine.setFrozen(firstId,true));panel->refresh();enter("-24");
        REQUIRE(current().frozen && current().shape.gainDb==-24 && current().render==before.render);
        // Creating a later swell still snapshots the IR default with neutral Level.
        REQUIRE(engine.addTarget(96000));const auto created=engine.savedSession();
        REQUIRE(m3::find(created,created.primary)->shape.gainDb==0 && created.defaultIR==defaults);
    }
    load(-12);key(false);REQUIRE(current().shape.gainDb==-12);load(12);key(true);REQUIRE(current().shape.gainDb==12);
    load(0);key(true);key(true,true);REQUIRE(std::abs(current().shape.gainDb-1.1f)<1.e-6);
    load(0);juce::MouseWheelDetails wheel{};wheel.deltaY=.1f;
    const auto point=dial.getLocalBounds().getCentre().toFloat();const auto now=juce::Time::getCurrentTime();
    juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(),point,{},0,0,0,0,0,&dial,&dial,now,point,now,0,false);
    dial.mouseWheelMove(event,wheel);REQUIRE(std::abs(current().shape.gainDb-1)<1.e-5);
    load(-30);REQUIRE(cell.displayedFraction()==0 && field.getText()=="-30");key(true);REQUIRE(current().shape.gainDb==-29);
    load(-30);key(true,true);REQUIRE(std::abs(current().shape.gainDb+29.9f)<1.e-6);
    load(-60);key(false);REQUIRE(current().shape.gainDb==-60);
    load(-30);dial.onDragStart();dial.setValue(.5,juce::sendNotificationSync);dial.onDragEnd();REQUIRE(current().shape.gainDb==-18);
    load(-30);auto* accessible=dial.getAccessibilityHandler()->getValueInterface();REQUIRE(accessible);
    REQUIRE(accessible->getCurrentValue()==-30 && accessible->getCurrentValueAsString()=="-30");
    const auto range=accessible->getRange();REQUIRE(range.isValid() && range.getMinimumValue()==-60 && range.getMaximumValue()==12);
    accessible->setValue(-24);REQUIRE(current().shape.gainDb==-24);load(0);accessible->setValue(-30);REQUIRE(current().shape.gainDb==-30);
    accessible->setValueAsString("-30.12345");REQUIRE(current().shape.gainDb==-30.12345f);
    const auto exact=current().shape.gainDb;field.setText(m3::ui::preciseValue(exact),false);field.onReturnKey();REQUIRE(current().shape.gainDb==exact);
    for(const auto* invalid:{"-60.001","12.001","nan","inf","--3",""}){enter(invalid);REQUIRE(current().shape.gainDb==exact);}
    accessible->setValue(std::numeric_limits<double>::infinity());accessible->setValue(-61);REQUIRE(current().shape.gainDb==exact);
    for(const auto db:{-60.f,-30.f,-12.f,0.f,12.f}){enter(juce::String(db));REQUIRE(current().shape.gainDb==db);}
    load(-30);auto before=current();auto reset=cell.prepareResetAction();REQUIRE(reset.apply);reset.apply();
    auto expected=before.shape;expected.gainDb=0;REQUIRE(current().shape==expected && current().ir==before.ir && current().render==before.render);
    // Escape leaves the accepted value intact; selection/session/source changes reject delayed input.
    load(-30);field.setText("-24",false);REQUIRE(field.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey)));REQUIRE(current().shape.gainDb==-30);
    for(int stale=0;stale<3;++stale)
    {
        load(-30);dial.onDragStart();
        if(stale==0)engine.restore(engine.savedSession());
        if(stale==1)REQUIRE(engine.selectTargets({secondId},secondId));
        if(stale==2)REQUIRE(engine.edit(firstId,[](auto& w,auto& error){return m3::setSource(w,w.sourceStart-1,w.sourceEnd,error);}));
        before=current();dial.setValue(.2,juce::sendNotificationSync);dial.onDragEnd();REQUIRE(current().shape==before.shape);
    }
    panel->removeFromDesktop();
}
