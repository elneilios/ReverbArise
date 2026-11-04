#pragma once
void Composition::verifyGestures()
{
    setScene(0);auto& engine=processor->swellEngine();bool available=true;std::optional<SongTime> shownTime;
    const juce::ScopedValueSetter<std::function<void()>> isolated(timeline().onEnvelopeEdited,
        [this,&engine,&available,&shownTime]{timeline().setData(processor->getCaptureSnapshot(),engine.snapshot(),shownTime.value_or(processor->getSongTime()),available);});
    auto original=*m3::find(baseline,firstId);original.placement=2300;original.shape.automaticStart=false;
    original.shape.startOffset=-18000;original.shape.endOffset=18000;original.shape.fadeIn=original.shape.fadeOut=6000;
    const auto load=[&](m3::Swell chosen)
    {
        available=true;shownTime.reset();auto state=baseline;*m3::find(state,firstId)=chosen;
        engine.restore(state);wait([&]{const auto s=engine.snapshot();return s.raw && s.auditionReady
            && std::all_of(s.session.swells.begin(),s.session.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
        editor->refresh();timeline().setData(processor->getCaptureSnapshot(),engine.snapshot(),processor->getSongTime());
        timeline().setEditingEnabled(true);timeline().setViewport({0,96000});timeline().setSnap(true);
        timeline().setGestureModifiers(false,false);timeline().setGridDivision(m3::GridDivision::quarterBeat);
    };
    const auto grab=[&](m3::EnvelopePoint kind){const auto points=timeline().grabPoints();const auto at=std::find_if(points.begin(),points.end(),[&](const auto& p){return p.kind==kind;});REQUIRE(at!=points.end());return *at;};
    const auto move=[&](juce::Point<float> point,double samples){point.x+=static_cast<float>(samples*timeline().sourceLane().getWidth()/timeline().viewport().span());return point;};
    const auto swell=[&]{return *m3::find(engine.savedSession(),firstId);};
    const auto captured=processor->getCaptureSnapshot().manifest.revision;
    using P=m3::EnvelopePoint;
    struct TimingCase {P kind;int delta;int64_t expected;};
    for(const auto test:{TimingCase{P::start,4200,36000},TimingCase{P::end,-4000,66000},TimingCase{P::fadeIn,4200,42000},TimingCase{P::fadeOut,-4000,60000}})
    {
        load(original);const auto before=swell();const auto renders=engine.snapshot().renderCount;const auto point=grab(test.kind);
        REQUIRE(timeline().beginDrag(point.point));timeline().dragTo(move(point.point,test.delta));
        REQUIRE(grab(test.kind).sample==test.expected);
        const auto after=swell();REQUIRE(after.id==before.id && after.render==before.render && after.ir==before.ir
            && after.sourceStart==before.sourceStart && after.sourceEnd==before.sourceEnd && after.arrival==before.arrival
            && after.placement==before.placement && after.inputGeneration==before.inputGeneration);
        REQUIRE(engine.snapshot().renderCount==renders);
    }
    load(original);auto before=swell();auto lane=timeline().wetLane();
    auto body=juce::Point<float>{static_cast<float>(timeline().viewport().pixel(42300,lane.getX(),lane.getWidth())),lane.getBottom()-30};
    REQUIRE(timeline().beginDrag(body));timeline().dragTo(move(body,4200));auto after=swell();
    REQUIRE(after.placement==6000 && after.shape==before.shape && after.sourceStart==before.sourceStart && after.sourceEnd==before.sourceEnd && after.arrival==before.arrival && after.render==before.render);
    REQUIRE(grab(P::start).sample==36000);
    for(int handle=0;handle<3;++handle)
    {
        load(original);const auto beforeSource=swell();const auto source=timeline().sourceLane();
        const auto at=handle==2?beforeSource.sourceEnd:beforeSource.sourceStart;
        auto point=juce::Point<float>{static_cast<float>(timeline().viewport().pixel(at,source.getX(),source.getWidth())),handle==0?source.getY()+15:source.getBottom()-9};
        REQUIRE(timeline().beginDrag(point));timeline().dragTo(move(point,handle==1?-4500:4500));const auto changed=swell();
        if(handle==0)REQUIRE(changed.arrival==54000 && changed.sourceStart==54000 && changed.sourceEnd==78000);
        if(handle==1)REQUIRE(changed.arrival==48000 && changed.sourceStart==42000 && changed.sourceEnd==72000);
        if(handle==2)REQUIRE(changed.arrival==48000 && changed.sourceStart==48000 && changed.sourceEnd==78000);
    }
    auto shortShape=original;shortShape.shape.startOffset=-100;shortShape.shape.endOffset=100;shortShape.shape.fadeIn=shortShape.shape.fadeOut=0;
    load(shortShape);timeline().setViewport({48000,52000});auto zero=grab(P::fadeIn);
    REQUIRE(timeline().beginDrag(zero.point));auto pointer=move(zero.point,10);timeline().dragTo(pointer);
    REQUIRE(swell().shape.fadeIn==0 && timeline().interactionStatus().contains("No valid grid point"));
    const auto generation=swell().shapeGeneration;timeline().setGestureModifiers(false,true);timeline().dragTo(pointer);
    REQUIRE(swell().shapeGeneration==generation && timeline().requestedSnap() && !timeline().effectiveSnap());
    timeline().dragTo(move(pointer,10));REQUIRE(swell().shape.fadeIn==10);
    load(original);timeline().setGestureModifiers(false,true);auto end=grab(P::end);
    REQUIRE(timeline().beginDrag(end.point));pointer=move(end.point,200);timeline().dragTo(pointer);const auto unsnapped=grab(P::end).sample;
    REQUIRE(unsnapped==68500);
    const auto accepted=swell();timeline().setGestureModifiers(true,true);timeline().dragTo(pointer);
    REQUIRE(swell().shape==accepted.shape && swell().shapeGeneration==accepted.shapeGeneration);
    pointer=move(pointer,1000);timeline().dragTo(pointer);REQUIRE(grab(P::end).sample==unsnapped+100);
    const auto fine=swell();timeline().setGestureModifiers(true,false);timeline().dragTo(pointer);
    REQUIRE(swell().shape==fine.shape && swell().shapeGeneration==fine.shapeGeneration && timeline().effectiveSnap());
    pointer=move(pointer,10);timeline().dragTo(pointer);REQUIRE(grab(P::end).sample%6000==0);
    load(original);available=false;timeline().setData(processor->getCaptureSnapshot(),engine.snapshot(),processor->getSongTime(),false);
    end=grab(P::end);REQUIRE(timeline().beginDrag(end.point));pointer=move(end.point,4200);timeline().dragTo(pointer);
    REQUIRE(swell().shape==original.shape && timeline().interactionStatus().contains("Snap unavailable"));
    timeline().setGestureModifiers(false,true);timeline().dragTo(move(pointer,100));REQUIRE(grab(P::end).sample==68400);
    load(original);available=false;timeline().setData(processor->getCaptureSnapshot(),engine.snapshot(),processor->getSongTime(),false);
    auto tension=grab(P::inTension);REQUIRE(timeline().beginDrag(tension.point));timeline().dragTo(tension.point+juce::Point<float>{0,16});
    REQUIRE(std::abs(swell().shape.inTension-.2)<1.e-6 && swell().shape.fadeIn==original.shape.fadeIn && swell().render==original.render);
    load(original);end=grab(P::end);REQUIRE(timeline().beginDrag(end.point));shownTime=processor->getSongTime();shownTime->bpm+=1;
    timeline().setData(processor->getCaptureSnapshot(),engine.snapshot(),*shownTime);timeline().dragTo(move(end.point,4200));REQUIRE(swell().shape==original.shape);
    load(original);end=grab(P::end);REQUIRE(timeline().beginDrag(end.point));REQUIRE(engine.selectTargets({secondId},secondId));
    timeline().dragTo(move(end.point,4200));REQUIRE(swell().shape==original.shape && timeline().interactionStatus().contains("Selection changed"));
    load(original);end=grab(P::end);REQUIRE(timeline().beginDrag(end.point));auto replacement=engine.savedSession();engine.restore(replacement);
    timeline().dragTo(move(end.point,4200));REQUIRE(swell().shape==original.shape);
    load(original);end=grab(P::end);REQUIRE(timeline().beginDrag(end.point));timeline().setEditingEnabled(false);timeline().dragTo(move(end.point,4200));
    REQUIRE(swell().shape==original.shape);
    load(original);end=grab(P::end);REQUIRE(timeline().beginDrag(end.point));REQUIRE(processor->requestWorkflowMode(workflow::Mode::finalise));
    timeline().dragTo(move(end.point,4200));REQUIRE(swell().shape==original.shape);REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));
    timeline().dragTo(move(end.point,8400));REQUIRE(swell().shape==original.shape);
    load(original);end=grab(P::end);REQUIRE(timeline().beginDrag(end.point));timeline().setGestureModifiers(true,true);
    timeline().focusLost(juce::Component::focusChangedDirectly);timeline().dragTo(move(end.point,4200));
    REQUIRE(swell().shape==original.shape && timeline().requestedSnap() && timeline().effectiveSnap());
    REQUIRE(processor->getCaptureSnapshot().manifest.revision==captured);
}
