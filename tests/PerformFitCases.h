#pragma once
void Composition::verifyFit()
{
    stopTimer();setScene(0);auto& engine=processor->swellEngine();const auto original=engine.savedSession();
    const auto capture=processor->getCaptureSnapshot();const auto time=processor->getSongTime();const auto renders=engine.snapshot().renderCount;
    auto shown=engine.snapshot();const auto first=m3::find(shown.session,firstId);REQUIRE(first && first->render.present());
    const auto sourceStart=first->sourceStart,sourceEnd=first->sourceEnd;
    const auto rawStart=first->render.start,rawEnd=first->render.start+first->render.frames;
    timeline().setData(capture,shown,time);timeline().fitSelection();
    REQUIRE(timeline().viewport().first<std::min(sourceStart,rawStart) && timeline().viewport().last>std::max(sourceEnd,rawEnd));
    // Pending wet uses real source/request metadata and never invents a raw lane.
    auto pending=shown;pending.raw.reset();pending.shaped.reset();auto selected=m3::find(pending.session,firstId);selected->render={};selected->phase=m3::Phase::rendering;
    timeline().setData(capture,pending,time);timeline().fitSelection();
    REQUIRE(timeline().viewport().first<sourceStart && timeline().viewport().last>sourceEnd && timeline().grabPoints().empty());
    selected=m3::find(shown.session,firstId);selected->placement=33000;
    timeline().setData(capture,shown,time);timeline().fitSelection();
    REQUIRE(timeline().viewport().first<sourceStart && timeline().viewport().last>rawEnd+33000);
    selected->placement=0;auto neighbor=m3::find(shown.session,secondId);REQUIRE(neighbor && neighbor->render.present());neighbor->placement=-190000;
    const auto overlappingEnd=neighbor->render.start+neighbor->render.frames+neighbor->placement;
    timeline().setData(capture,shown,time);timeline().fitSelection();
    REQUIRE(timeline().viewport().last>overlappingEnd && timeline().viewport().last<200000);
    shown.session.primary.clear();shown.session.selected.clear();shown.raw.reset();shown.shaped.reset();
    timeline().setData(capture,shown,time);timeline().fitSelection();
    const auto& last=capture.manifest.ranges.back();
    REQUIRE(timeline().viewport().first<capture.manifest.ranges.front().startSample && timeline().viewport().last>last.startSample+last.frameCount);
    // Synthetic coordinate oracle only; no PCM or model is published for these ranges.
    CaptureSessionSnapshot sparse;sparse.manifest.ranges={{-96000,48000,CaptureCoverageKind::audio},{192000,48000,CaptureCoverageKind::silence}};
    timeline().setData(sparse,{},time);timeline().fitSelection();
    REQUIRE(timeline().viewport().first< -96000 && timeline().viewport().last>240000);
    timeline().setData({}, {},{});timeline().fitSelection();
    REQUIRE(std::isfinite(timeline().viewport().first) && std::isfinite(timeline().viewport().last) && timeline().viewport().span()>0);
    const auto accepted=timeline().viewport();timeline().setViewport({std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity()});
    REQUIRE(timeline().viewport().first==accepted.first && timeline().viewport().last==accepted.last);
    const auto after=engine.savedSession();REQUIRE(after.primary==original.primary && after.selected==original.selected && after.swells.size()==original.swells.size());
    for(size_t n=0;n<original.swells.size();++n)
    {const auto& a=original.swells[n];const auto& b=after.swells[n];REQUIRE(a.id==b.id && a.render==b.render && a.ir==b.ir && a.shape==b.shape && a.placement==b.placement && a.inputGeneration==b.inputGeneration && a.shapeGeneration==b.shapeGeneration);}
    REQUIRE(engine.snapshot().renderCount==renders && processor->getCaptureSnapshot().manifest.revision==capture.manifest.revision);
}
