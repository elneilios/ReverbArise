#pragma once
void Composition::verifyReadability()
{
    SongTime musical{true,0,0,48000,120,4,4};
    REQUIRE(m3::ui::musicalReadout(95999,musical)=="2.1" && m3::ui::musicalReadout(-1,musical)=="1.1");
    REQUIRE(m3::ui::musicalReadout(-96001,musical)=="0.1");
    REQUIRE(m3::ui::footerPosition(48000,musical)=="Bar 1  |  Beat 3");
    REQUIRE(m3::ui::compactValue(-.000001,3)=="0" && m3::ui::compactValue(1.250,3)=="1.25");
    REQUIRE(m3::ui::preciseValue(.123456789).getDoubleValue()==.123456789);
    musical.denominator=8;musical.numerator=6;REQUIRE(m3::ui::musicalReadout(71999,musical)=="2.1");
    musical.valid=false;REQUIRE(m3::ui::footerPosition(0,musical)=="Position unavailable");
    stopTimer();setScene(0);auto& engine=processor->swellEngine();
    auto panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);panel->stopAutomaticRefreshForReview();
    panel->enableResponsiveReview({},PresentationFitContext{{4000,3000},{0,0}});
    panel->setPresentationSize(1100,850);panel->refresh();
    const auto saved=engine.savedSession();const auto renders=engine.snapshot().renderCount;const auto capture=processor->getCaptureSnapshot().manifest.revision;
    auto& viewport=require<juce::Viewport>(*panel,"inspectorViewport");auto* content=viewport.getViewedComponent();REQUIRE(content);
    auto& tabs=require<controls::SegmentedSelector>(*panel,"inspectorSections");
    auto& reset=require<juce::Button>(*panel,"resetWetShape");
    const auto resetBounds=reset.getBounds();
    REQUIRE(reset.getParentComponent()==find(*panel,"presentationContent"));
    const auto checkFields=[&](int section,std::initializer_list<const char*> names)
    {
        tabs.onRequest(section);panel->refresh();
        REQUIRE(viewport.isVisible() && reset.isVisible() && reset.getBounds()==resetBounds);
        for(const auto* name:names)
        {
            auto& cell=require<KnobValue>(*panel,juce::String(name)+"Cell");
            REQUIRE(cell.isVisible() && content->getLocalBounds().contains(cell.getBounds()));
            REQUIRE(cell.getLocalBounds().contains(cell.valueField().getBounds()) && cell.valueField().getWidth()==controlGeometry::valueWidth);
            viewport.setViewPosition(0,std::clamp(cell.getY(),0,std::max(0,content->getHeight()-viewport.getHeight())));
            REQUIRE(viewport.getViewArea().intersects(cell.getBounds()) && reset.getBounds()==resetBounds);
        }
    };
    checkFields(1,{"arrivalSample","sourceSampleStart","sourceSampleEnd","wetPlacement"});
    for(const auto* name:{"sourceSampleStart","sourceSampleEnd"})
    {
        auto& cell=require<KnobValue>(*panel,juce::String(name)+"Cell");REQUIRE(cell.getWidth()>content->getWidth()-spacing::related);
        juce::Label* caption=nullptr;for(auto* child:cell.getChildren())if(auto* label=dynamic_cast<juce::Label*>(child))caption=label;
        REQUIRE(caption && caption->getText().startsWith("Source ") && !caption->getText().contains("Src"));
        REQUIRE(juce::GlyphArrangement::getStringWidth(caption->getFont(),caption->getText())<=caption->getWidth());
    }
    tabs.onRequest(3);panel->refresh();
    auto& sound=require<controls::SoundPresetEditor>(*panel,"selectedIrEditor");
    auto& level=require<KnobValue>(*panel,"swellLevelCell");
    REQUIRE(sound.isVisible() && sound.isParentOf(&level) && level.isVisible() && !viewport.isVisible());
    REQUIRE(level.getLocalBounds().contains(level.valueField().getBounds()));
    REQUIRE(reset.isVisible() && reset.getBounds()==resetBounds);
    checkFields(2,{"preRoll","endOffset","fadeInMs","fadeOutMs","curveInTension","curveOutTension"});
    auto& timeline=panel->timelineComponent();
    for(const auto view:{m3::SwellViewport{-24000,96000},m3::SwellViewport{40000,70000},m3::SwellViewport{230000,290000}})
    {
        timeline.setViewport(view);juce::MessageManager::getInstance()->runDispatchLoopUntil(10);
        REQUIRE(timeline.viewport().first==view.first && timeline.viewport().last==view.last);
        const auto ruler=timeline.musicalRulerBounds();const auto dry=timeline.sourceLane(),wet=timeline.wetLane();
        REQUIRE(ruler.getBottom()<dry.getY() && ruler.getX()==dry.getX() && ruler.getWidth()==dry.getWidth());
        REQUIRE(dry.getX()==wet.getX() && dry.getWidth()==wet.getWidth());
        const auto& rulerTime=timeline.acceptedGridTiming();
        const auto ticks=controls::musicalTimelineTicks(timeline.viewport(),ruler.getWidth(),rulerTime);REQUIRE(!ticks.empty());
        for(const auto& tick:ticks)REQUIRE(std::abs(timeline.viewport().pixel(tick.sample,dry.getX(),dry.getWidth())-timeline.viewport().pixel(tick.sample,wet.getX(),wet.getWidth()))<1e-8);
        const auto actual=timeline.createComponentSnapshot(timeline.getLocalBounds()).getClippedImage(ruler);
        juce::Image expected(juce::Image::ARGB,ruler.getWidth(),ruler.getHeight(),true);
        {juce::Graphics graphics(expected);graphics.fillAll(background);controls::paintMusicalTimelineRuler(graphics,expected.getBounds(),timeline.viewport(),rulerTime,&timeline);}
        for(int y=0;y<ruler.getHeight();++y)for(int x=0;x<ruler.getWidth();++x)REQUIRE(actual.getPixelAt(x,y)==expected.getPixelAt(x,y));
    }
    viewport.setViewPosition(0,0);NativeEditorPeer nativePeer(*panel,"ReverbArise - PerformReadabilityCases native validation");panel->setVisible(true);panel->toFront(true);
    // Exact-value editing is a real native focus test. A temporary/no-activate
    // peer can lose focus after its delayed activation, restoring rounded text.
    auto* peer=panel->getPeer();REQUIRE(peer);
   #if JUCE_WINDOWS
    ::SetForegroundWindow(static_cast<HWND>(peer->getNativeHandle()));
   #endif
    peer->grabFocus();wait([&]{return peer->isFocused() && juce::Process::isForegroundProcess();});
    auto& preRoll=require<KnobValue>(*panel,"preRollCell");const auto compact=preRoll.valueField().getText();REQUIRE(compact=="0.5");
    const auto focusBefore=engine.savedSession();int commits=0;const auto commit=preRoll.onCommit;preRoll.onCommit=[&](const auto& value){++commits;return commit(value);};
    struct Notifications final:juce::AudioProcessorListener
    {int changes=0;void audioProcessorParameterChanged(juce::AudioProcessor*,int,float)override{}
        void audioProcessorChanged(juce::AudioProcessor*,const ChangeDetails& detail)override{if(detail.nonParameterStateChanged)++changes;}} notifications;
    processor->flushHostStateChangesForTesting();processor->addListener(&notifications);
    preRoll.valueField().grabKeyboardFocus();juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    REQUIRE(preRoll.valueField().getText().length()>compact.length());preRoll.valueField().onReturnKey();
    processor->flushHostStateChangesForTesting();processor->removeListener(&notifications);
    REQUIRE(notifications.changes==0 && commits==0 && focusBefore.performUi==engine.savedSession().performUi && preRoll.valueField().getText()==compact && m3::find(engine.savedSession(),saved.primary)->shape==m3::find(saved,saved.primary)->shape);
    auto& curve=require<KnobValue>(*panel,"curveInTensionCell");curve.valueField().setText("0.123456789",false);curve.valueField().onReturnKey();panel->refresh();
    REQUIRE(m3::find(engine.savedSession(),saved.primary)->shape.inTension==.123456789 && curve.valueField().getText()=="0.12");
    viewport.setViewPosition(0,curve.getY());curve.valueField().grabKeyboardFocus();juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    wait([&]{return curve.valueField().hasKeyboardFocus(false);});
    REQUIRE(curve.valueField().getText().getDoubleValue()==.123456789);curve.valueField().onEscapeKey();nativePeer.setVisible(false);
    const auto point=timeline.wetLane().getCentre();const auto now=juce::Time::getCurrentTime();
    timeline.mouseMove(juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,{},0,0,0,0,0,&timeline,&timeline,now,point,now,0,false));
    REQUIRE(timeline.getTooltip().contains("raw reference") && timeline.getTooltip().contains("Ctrl"));
    auto& footer=require<controls::TransportReadout>(*panel,"hostInfo");REQUIRE(footer.getTitle().startsWith("Bar ") && footer.getTitle().contains("Beat "));
    engine.restore(saved);wait([&]{return engine.snapshot().auditionReady;});panel->refresh();
    REQUIRE(engine.snapshot().renderCount==renders && processor->getCaptureSnapshot().manifest.revision==capture);
    const auto after=engine.savedSession();for(size_t n=0;n<saved.swells.size();++n)REQUIRE(after.swells[n].id==saved.swells[n].id && after.swells[n].ir==saved.swells[n].ir && after.swells[n].render==saved.swells[n].render && after.swells[n].shape==saved.swells[n].shape);

    // Actual processor snapshots -> editor refresh, independent of the component's
    // synthetic presentation tests. Consistent host PPQ keeps Song origin at zero.
    const auto originalHead=head.position;
    // This is a footer geometry/readout oracle, not a material-tempo render
    // request. Freeze its accepted sounds before exercising many real clocks.
    wait([&]{const auto s=engine.savedSession();return std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
    for(const auto& w:engine.savedSession().swells)REQUIRE(engine.setFrozen(w.id,true));
    juce::String beforeReadout,error;REQUIRE(m3::encode(engine.savedSession(),beforeReadout,error));
    const auto renderRevision=engine.snapshot().renderCount,captureRevision=processor->getCaptureSnapshot().manifest.revision;
    juce::AudioBuffer<float> transportAudio(2,64);juce::MidiBuffer transportMidi;
    for(const auto size:{juce::Point<int>{1100,850},{1400,900},{1400,1000},{1600,1000}})
    {
        panel->setPresentationSize(size.x,size.y);panel->refresh();const auto geometry=footer.geometry();const auto bounds=footer.getBounds();
        for(const auto bar:{9,10,99,100,99,9})for(const auto bpm:{82.,123.45})for(const auto numerator:{4,6,12})
        {
            const auto ppq=static_cast<double>(bar-1)*numerator*4/8;
            head.position.setBpm(bpm);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{numerator,8});
            head.position.setTimeInSamples(static_cast<int64_t>(std::llround(ppq*48000*60/bpm)));head.position.setPpqPosition(ppq);
            transportAudio.clear();processor->processBlock(transportAudio,transportMidi);panel->refresh();
            REQUIRE(footer.geometry()==geometry && footer.getBounds()==bounds);
            REQUIRE(footer.presentation().values[0]==juce::String(bar) && footer.presentation().values[1]=="1");
            REQUIRE(footer.presentation().values[2]==m3::ui::compactValue(bpm,2) && footer.presentation().values[3]==juce::String(numerator)+" / 8");
            REQUIRE(footer.getTooltip().contains("Exact: "+m3::musicalPosition(processor->getSongTime().sample,processor->getSongTime())));
            REQUIRE(!timeline.transportPlayhead());
        }
        head.position=juce::AudioPlayHead::PositionInfo{};transportAudio.clear();processor->processBlock(transportAudio,transportMidi);panel->refresh();
        for(const auto& value:footer.presentation().values)REQUIRE(value=="unavailable");
        REQUIRE(footer.geometry()==geometry && footer.getBounds()==bounds);
        const auto help=find(*panel,"hoverHelp");REQUIRE(help && footer.getBottom()<=help->getY());
    }
    head.position=originalHead;transportAudio.clear();processor->processBlock(transportAudio,transportMidi);panel->refresh();
    REQUIRE(timeline.transportPlayhead()==processor->getSongTime().sample);
    wait([&]{const auto s=engine.savedSession();return std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
    const auto returnGeometry=footer.geometry();
    REQUIRE(processor->requestWorkflowMode(workflow::Mode::capture));panel->refresh();REQUIRE(!footer.isVisible());
    REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));panel->refresh();REQUIRE(footer.isVisible() && footer.geometry()==returnGeometry);
    juce::String afterReadout;REQUIRE(m3::encode(engine.savedSession(),afterReadout,error));REQUIRE(beforeReadout==afterReadout);
    REQUIRE(engine.snapshot().renderCount==renderRevision && processor->getCaptureSnapshot().manifest.revision==captureRevision);
}
