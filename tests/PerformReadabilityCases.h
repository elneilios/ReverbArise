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
    auto panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);panel->stopAutomaticRefreshForReview();panel->setSize(1100,850);panel->refresh();
    const auto saved=engine.savedSession();const auto renders=engine.snapshot().renderCount;const auto capture=processor->getCaptureSnapshot().manifest.revision;
    auto& viewport=require<juce::Viewport>(*panel,"inspectorViewport");auto* content=viewport.getViewedComponent();REQUIRE(content);
    auto& timing=require<juce::Label>(*panel,"inspectorTimingGroup");auto& envelope=require<juce::Label>(*panel,"inspectorEnvelopeGroup");auto& level=require<juce::Label>(*panel,"inspectorLevelGroup");
    REQUIRE(timing.isVisible() && timing.getY()<envelope.getY() && envelope.getY()<level.getY());
    REQUIRE(content->getHeight()>viewport.getHeight());
    for(const auto* name:{"arrivalSample","sourceSampleStart","sourceSampleEnd","wetPlacement","preRoll","endOffset","fadeInMs","fadeOutMs","curveInTension","curveOutTension","swellLevel"})
    {
        auto& cell=require<KnobValue>(*panel,juce::String(name)+"Cell");REQUIRE(content->getLocalBounds().contains(cell.getBounds()));
        REQUIRE(cell.getLocalBounds().contains(cell.valueField().getBounds()) && cell.valueField().getWidth()>=controlGeometry::valueWidth);
        viewport.setViewPosition(0,std::clamp(cell.getY(),0,std::max(0,content->getHeight()-viewport.getHeight())));
        REQUIRE(viewport.getViewArea().intersects(cell.getBounds()));
    }
    for(const auto* name:{"sourceSampleStart","sourceSampleEnd"})
    {
        auto& cell=require<KnobValue>(*panel,juce::String(name)+"Cell");REQUIRE(cell.getWidth()>content->getWidth()-spacing::related);
        juce::Label* caption=nullptr;for(auto* child:cell.getChildren())if(auto* label=dynamic_cast<juce::Label*>(child))caption=label;
        REQUIRE(caption && caption->getText().startsWith("Source ") && !caption->getText().contains("Src"));
        REQUIRE(juce::GlyphArrangement::getStringWidth(caption->getFont(),caption->getText())<=caption->getWidth());
    }
    auto& reset=require<juce::Button>(*panel,"resetWetShape");viewport.setViewPosition(0,10000);REQUIRE(viewport.getViewArea().contains(reset.getBounds()));
    auto& timeline=panel->timelineComponent();
    for(const auto view:{m3::SwellViewport{-24000,96000},m3::SwellViewport{40000,70000},m3::SwellViewport{230000,290000}})
    {
        timeline.setViewport(view);juce::MessageManager::getInstance()->runDispatchLoopUntil(10);
        REQUIRE(timeline.viewport().first==view.first && timeline.viewport().last==view.last);
        const auto ruler=timeline.musicalRulerBounds();const auto dry=timeline.sourceLane(),wet=timeline.wetLane();
        REQUIRE(ruler.getBottom()<dry.getY() && ruler.getX()==dry.getX() && ruler.getWidth()==dry.getWidth());
        REQUIRE(dry.getX()==wet.getX() && dry.getWidth()==wet.getWidth());
        const auto ticks=controls::musicalTimelineTicks(timeline.viewport(),ruler.getWidth(),processor->getSongTime());REQUIRE(!ticks.empty());
        for(const auto& tick:ticks)REQUIRE(std::abs(timeline.viewport().pixel(tick.sample,dry.getX(),dry.getWidth())-timeline.viewport().pixel(tick.sample,wet.getX(),wet.getWidth()))<1e-8);
        const auto actual=timeline.createComponentSnapshot(timeline.getLocalBounds()).getClippedImage(ruler);
        juce::Image expected(juce::Image::ARGB,ruler.getWidth(),ruler.getHeight(),true);
        {juce::Graphics graphics(expected);graphics.fillAll(background);controls::paintMusicalTimelineRuler(graphics,expected.getBounds(),timeline.viewport(),processor->getSongTime());}
        for(int y=0;y<ruler.getHeight();++y)for(int x=0;x<ruler.getWidth();++x)REQUIRE(actual.getPixelAt(x,y)==expected.getPixelAt(x,y));
    }
    viewport.setViewPosition(0,0);panel->addToDesktop(juce::ComponentPeer::windowIsTemporary);panel->setVisible(true);panel->toFront(true);
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
    REQUIRE(curve.valueField().getText().getDoubleValue()==.123456789);curve.valueField().onEscapeKey();panel->removeFromDesktop();
    const auto point=timeline.wetLane().getCentre();const auto now=juce::Time::getCurrentTime();
    timeline.mouseMove(juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,{},0,0,0,0,0,&timeline,&timeline,now,point,now,0,false));
    REQUIRE(timeline.getTooltip().contains("raw reference") && timeline.getTooltip().contains("Ctrl"));
    auto& footer=require<juce::Label>(*panel,"hostInfo");REQUIRE(footer.getText().startsWith("Bar ") && footer.getText().contains("Beat "));
    engine.restore(saved);wait([&]{return engine.snapshot().auditionReady;});panel->refresh();
    REQUIRE(engine.snapshot().renderCount==renders && processor->getCaptureSnapshot().manifest.revision==capture);
    const auto after=engine.savedSession();for(size_t n=0;n<saved.swells.size();++n)REQUIRE(after.swells[n].id==saved.swells[n].id && after.swells[n].ir==saved.swells[n].ir && after.swells[n].render==saved.swells[n].render && after.swells[n].shape==saved.swells[n].shape);
}
