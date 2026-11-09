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
    const auto pointer=[&](float travel,int flags,bool moved)
    {
        const auto start=dial.getLocalBounds().getCentre().toFloat();
        const auto now=juce::Time::getCurrentTime();
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),start.translated(0,-travel),
            flags,0,0,0,0,0,&dial,&dial,now,start,now,1,moved);
    };
    const auto drag=[&](float travel,bool fine=false)
    {
        const auto flags=juce::ModifierKeys::leftButtonModifier|(fine?juce::ModifierKeys::shiftModifier:0);
        dial.mouseDown(pointer(0,flags,false));
        if(travel!=0)dial.mouseDrag(pointer(travel,flags,true));
        dial.mouseUp(pointer(travel,0,travel!=0));
    };
    const auto scroll=[&](bool up,bool fine=false)
    {
        juce::MouseWheelDetails wheel{};wheel.deltaY=up?.1f:-.1f;
        const auto nextTime=juce::Time::getMillisecondCounterHiRes()+2;
        while(juce::Time::getMillisecondCounterHiRes()<nextTime)juce::Thread::sleep(1);
        dial.mouseWheelMove(pointer(0,fine?juce::ModifierKeys::shiftModifier:0,false),wheel);
    };
    const auto beginTyping=[&]
    {
        const auto point=field.getLocalBounds().getCentre().toFloat();
        const auto now=juce::Time::getCurrentTime();
        field.mouseDown(juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,
            juce::ModifierKeys::leftButtonModifier,0,0,0,0,0,&field,&field,now,point,now,1,false));
    };
    for(const auto db:{-12.f,0.f,12.f})
    {
        load(db);REQUIRE(std::abs(cell.displayedFraction()-(db+12)/24.)<1.e-8);
        const auto before=current();const auto count=engine.snapshot().renderCount;panel->refresh();dial.grabKeyboardFocus();panel->refresh();
        REQUIRE(current().shape==before.shape && current().inputGeneration==before.inputGeneration
            && current().shapeGeneration==before.shapeGeneration && engine.snapshot().renderCount==count);
    }
    const auto quarter=.25f*dial.getMouseDragSensitivity();
    load(0);drag(quarter);REQUIRE(current().shape.gainDb==6);
    load(0);drag(-quarter);REQUIRE(current().shape.gainDb==-6);
    load(0);drag(quarter,true);const auto fine=current().shape.gainDb;
    REQUIRE(fine>0 && fine<6);
    load(0);drag(-quarter,true);REQUIRE(std::abs(current().shape.gainDb+fine)<1.e-6);
    load(0);drag(3*quarter);REQUIRE(current().shape.gainDb==12);
    load(0);drag(-3*quarter);REQUIRE(current().shape.gainDb==-12);
    load(0);dial.mouseDown(pointer(0,juce::ModifierKeys::leftButtonModifier,false));
    dial.mouseDrag(pointer(quarter,juce::ModifierKeys::leftButtonModifier,true));REQUIRE(current().shape.gainDb==6);
    dial.mouseDrag(pointer(0,juce::ModifierKeys::leftButtonModifier,true));
    dial.mouseUp(pointer(0,0,true));REQUIRE(current().shape.gainDb==0);
    {
        load(0);const auto before=current();const auto defaults=engine.savedSession().defaultIR;
        const auto neighbour=*m3::find(engine.savedSession(),secondId);const auto renders=engine.snapshot().renderCount;
        TestDirectory otherRoot;auto other=std::make_unique<PreverbAudioProcessor>(otherRoot.file);
        wait([&]{return other->swellEngine().savedSession().defaultIR.present();});
        const auto otherDefault=other->swellEngine().savedSession().defaultIR;
        enter("-6");wait([&]{return engine.snapshot().auditionReady;});
        REQUIRE(current().shapeGeneration>before.shapeGeneration && current().inputGeneration==before.inputGeneration
            && current().render==before.render && current().calibration==before.calibration && engine.snapshot().renderCount==renders);
        REQUIRE(engine.savedSession().defaultIR==defaults && m3::sameSavedSwell(*m3::find(engine.savedSession(),secondId),neighbour));
        REQUIRE(other->swellEngine().savedSession().swells.empty() && other->swellEngine().savedSession().defaultIR==otherDefault);
        REQUIRE(engine.setFrozen(firstId,true));panel->refresh();enter("-12");
        REQUIRE(current().frozen && current().shape.gainDb==-12 && current().render==before.render);
        // Creating a later swell still snapshots the IR default with neutral Level.
        REQUIRE(engine.addTarget(96000));const auto created=engine.savedSession();
        REQUIRE(m3::find(created,created.primary)->shape.gainDb==0 && created.defaultIR==defaults);
    }
    load(-12);key(false);REQUIRE(current().shape.gainDb==-12);load(12);key(true);REQUIRE(current().shape.gainDb==12);
    load(0);key(true);key(true,true);REQUIRE(std::abs(current().shape.gainDb-1.1f)<1.e-6);
    load(0);scroll(true);scroll(true,true);REQUIRE(std::abs(current().shape.gainDb-1.1f)<1.e-6);
    load(-12);scroll(false);REQUIRE(current().shape.gainDb==-12);
    load(12);scroll(true);REQUIRE(current().shape.gainDb==12);
    load(-30);REQUIRE(cell.displayedFraction()==0 && field.getText()=="-30");
    const auto saved=current();
    dial.grabKeyboardFocus();dial.mouseMove(pointer(0,0,false));panel->refresh();
    REQUIRE(current().shape==saved.shape && current().shapeGeneration==saved.shapeGeneration);
    dial.mouseDown(pointer(0,juce::ModifierKeys::leftButtonModifier,false));
    REQUIRE(current().shape.gainDb==-12 && field.getText()=="-12");
    dial.mouseUp(pointer(0,0,false));
    load(-30);drag(-quarter);REQUIRE(current().shape.gainDb==-12);
    load(-30);key(false);REQUIRE(current().shape.gainDb==-12);
    load(-30);scroll(false);REQUIRE(current().shape.gainDb==-12);
    load(-30);
    key(true);REQUIRE(current().shape.gainDb==-11);
    load(-30);key(true,true);REQUIRE(std::abs(current().shape.gainDb+11.9f)<1.e-6);
    load(-30);scroll(true);REQUIRE(current().shape.gainDb==-11);
    load(-30);drag(dial.getMouseDragSensitivity()/24.f);REQUIRE(std::abs(current().shape.gainDb+11)<.0001f);
    load(-60);key(false);REQUIRE(current().shape.gainDb==-12);
    load(-30);auto* accessible=dial.getAccessibilityHandler()->getValueInterface();REQUIRE(accessible);
    REQUIRE(accessible->getCurrentValue()==-12 && accessible->getCurrentValueAsString()=="-30 dB (saved)");
    REQUIRE(dial.getAccessibilityHandler()->getHelp().contains("Saved Level: -30 dB"));
    REQUIRE(dial.getTooltip().contains("Saved Level: -30 dB") && dial.getTooltip().contains("-12 to +12 dB"));
    const auto range=accessible->getRange();REQUIRE(range.isValid() && range.getMinimumValue()==-12 && range.getMaximumValue()==12);
    accessible->setValue(-24);accessible->setValueAsString("-24");REQUIRE(current().shape.gainDb==-30);
    accessible->setValue(-12);REQUIRE(current().shape.gainDb==-12);
    accessible->setValueAsString("-6.12345");REQUIRE(current().shape.gainDb==-6.12345f);
    load(-30.12345f);
    const auto exact=current().shape.gainDb;beginTyping();
    REQUIRE(field.getText()==m3::ui::preciseValue(exact));field.onReturnKey();REQUIRE(current().shape.gainDb==exact);
    for(const auto* invalid:{"-24","-12.001","12.001","nan","inf","--3",""}){enter(invalid);REQUIRE(current().shape.gainDb==exact);}
    for(const auto invalid:{std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN(),-12.001,12.001})
    {accessible->setValue(invalid);REQUIRE(current().shape.gainDb==exact);}
    for(const auto db:{-12.f,-6.12345f,0.f,12.f}){enter(m3::ui::preciseValue(db));REQUIRE(current().shape.gainDb==db);}
    load(-30);auto before=current();auto reset=cell.prepareResetAction();REQUIRE(reset.apply);reset.apply();
    auto expected=before.shape;expected.gainDb=0;REQUIRE(current().shape==expected && current().ir==before.ir && current().render==before.render);
    // Escape leaves the accepted value intact; selection/session/source changes reject delayed input.
    load(-30);beginTyping();field.setText("-6",false);
    REQUIRE(field.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey)));
    wait([&]{return field.getText()=="-30";});
    REQUIRE(current().shape.gainDb==-30);
    for(int stale=0;stale<3;++stale)
    {
        load(-12);dial.mouseDown(pointer(0,juce::ModifierKeys::leftButtonModifier,false));
        if(stale==0)engine.restore(engine.savedSession());
        if(stale==1)REQUIRE(engine.selectTargets({secondId},secondId));
        if(stale==2)REQUIRE(engine.edit(firstId,[](auto& w,auto& error){return m3::setSource(w,w.sourceStart-1,w.sourceEnd,error);}));
        before=current();dial.mouseDrag(pointer(quarter,juce::ModifierKeys::leftButtonModifier,true));
        dial.mouseUp(pointer(quarter,0,true));REQUIRE(current().shape==before.shape);
    }
    load(-30);
    REQUIRE(engine.selectTargets({secondId},secondId));
    drag(quarter);REQUIRE(current().shape.gainDb==-30);
    REQUIRE(m3::find(engine.savedSession(),secondId)->shape==m3::find(baseline,secondId)->shape);
    panel->enableResponsiveReview({},PresentationFitContext{{4000,3000},{0,0}});
    require<controls::SegmentedSelector>(*panel,"inspectorSections").onRequest(3);
    auto& standard=require<KnobValue>(*panel,"fadeInMsCell");
    for(const auto [width,height,scale]:std::array<std::array<int,3>,3>{{{900,600,100},{1400,900,100},{900,600,125}}})
    {
        REQUIRE(panel->setPresentationSize(width,height));
        REQUIRE(panel->setPresentationScale(scale));
        load(0);
        const auto local=cell.getLocalBounds();
        const auto* viewport=cell.findParentComponentOfClass<juce::Viewport>();REQUIRE(viewport && cell.isShowing());
        REQUIRE(viewport->getViewArea().contains(cell.getBounds()));
        REQUIRE(cell.getNumChildComponents()==standard.getNumChildComponents());
        auto& feedback=require<controls::FieldFeedbackLabel>(cell,"swellLevelFeedback");
        REQUIRE(!feedback.getBounds().isEmpty() && local.contains(feedback.getBounds()));
        REQUIRE(dial.getBounds()==standard.valueKnob().getBounds() && local.contains(field.getBounds()));
        const auto angles=dial.getRotaryParameters();
        const auto neutral=(angles.startAngleRadians+angles.endAngleRadians)/2;
        REQUIRE(std::abs(neutral-juce::MathConstants<float>::twoPi)<1.e-5);
        const auto box=dial.getLookAndFeel().getSliderLayout(dial).sliderBounds;
        const auto image=dial.createComponentSnapshot(dial.getLocalBounds());
        // The bright marker in the production paint must land above the centre.
        REQUIRE(image.getPixelAt(box.getCentreX(),box.getY()+5)==m3::ui::text);
        load(-30);
        bool hasStandardCaption=false;
        for(auto* child:cell.getChildren())
            if(auto* label=dynamic_cast<juce::Label*>(child);label && label->getText()=="Level (dB)")
                hasStandardCaption=label->isVisible() && local.contains(label->getBounds());
        REQUIRE(hasStandardCaption && field.getText()=="-30");
    }
    load(-30.12345f);
    REQUIRE(engine.setFrozen(firstId,true));
    panel->refresh();
    const auto beforeTouch=current();
    drag(0);
    const auto edited=current();
    auto expectedShape=beforeTouch.shape;expectedShape.gainDb=-12;
    REQUIRE(edited.frozen && edited.shape==expectedShape && edited.render==beforeTouch.render
        && edited.calibration==beforeTouch.calibration);
    panel->removeFromDesktop();
    juce::MemoryBlock project;
    processor->getStateInformation(project);
    processor->setStateInformation(project.getData(),static_cast<int>(project.getSize()));
    REQUIRE(processor->flushCaptureForTesting());
    REQUIRE(m3::sameSavedSwell(current(),edited) && !engine.snapshot().playing);
}
