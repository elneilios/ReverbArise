#pragma once
#include <UI/LearnTimelineModel.h>
static void learnTimelineCases()
{
    using namespace m3::ui;
    CaptureSessionSnapshot source;source.manifest.sampleRate=48000;
    source.manifest.ranges={{0,48000,CaptureCoverageKind::audio},{48000,48000,CaptureCoverageKind::silence},{144000,48000,CaptureCoverageKind::audio}};
    source.overview={{source.manifest.ranges[0],true,{{0,512,-1.e-8f,1.e-8f}}},{source.manifest.ranges[1],true,{}},{source.manifest.ranges[2],false,{}}};
    SongTime time{true,0,0,48000,120,4,4};CaptureProgress pending;
    REQUIRE(learnCoverageAt(source,pending,100,0,300000).kind=="Audio"); // Quiet is not silence.
    REQUIRE(learnCoverageAt(source,pending,50000,0,300000).kind=="Silence");
    const auto gap=learnCoverageAt(source,pending,100000,0,300000);
    REQUIRE(gap.kind=="Gap (uncaptured)" && gap.start==96000 && gap.end==144000);
    REQUIRE(learnCoverageAt(source,pending,150000,0,300000).kind=="Unavailable");
    float samples[128];std::fill_n(samples,128,.2f);pending.add(50000,64,samples,true,source.manifest);
    const auto provisional=learnCoverageAt(source,pending,50010,0,300000);
    REQUIRE(provisional.provisional && provisional.kind=="Audio" && provisional.description().contains("Recording Audio"));
    REQUIRE(learnMappedFrames(source.manifest)==144000 && learnProvisionalFrames(pending)==64 && learnExtentFrames(source,pending,time)==192000);
    const auto position=learnInspectionReadout(108000,gap,time,48000);
    REQUIRE(position.timingAvailable && position.text.contains("Bar 2 Beat 1.5") && position.text.contains("0:02.250"));
    REQUIRE(position.text.contains("Sample 108000") && position.text.contains("Gap (uncaptured)") && position.details.contains("samples [96000, 144000)"));
    auto shifted=time;shifted.sample=196000;shifted.ppq=4;
    REQUIRE(learnInspectionReadout(208000,gap,shifted,48000).text.contains("Bar 2 Beat 1.5 | 0:02.250"));
    auto compound=time;compound.numerator=6;compound.denominator=8;
    REQUIRE(learnInspectionReadout(90000,gap,compound,48000).text.contains("Bar 2 Beat 2.5 | 0:01.875"));
    REQUIRE(learnInspectionReadout(-24000,gap,time,48000).text.contains("Bar 0 Beat 4 | -0:00.500"));
    REQUIRE(learnInspectionReadout(95999,gap,time,48000).text.contains("Bar 2 Beat 1")); // Rounded beat carries at the boundary.
    REQUIRE(learnInspectionReadout(172800000,gap,time,48000).text.contains("1:00:00.000"));
    auto noTiming=time;noTiming.valid=false;
    for(const auto rate:{48000.,44100.,0.}){
        const auto unavailable=learnInspectionReadout(108000,gap,noTiming,rate);
        REQUIRE(!unavailable.timingAvailable && unavailable.text.startsWith("Timing unavailable | Sample 108000") && !unavailable.text.contains("Bar "));}
    REQUIRE(!learnInspectionReadout(108000,gap,time,44100).timingAvailable);
    REQUIRE(learnInspectionReadout(108000,gap,time,0).timingAvailable); // Empty accepted map; reliable current host timing.
    REQUIRE(!learnInspectionReadout(INT64_MAX,gap,time,48000).timingAvailable);
    for(const auto kind:{"Audio","Silence","Unavailable","Gap (uncaptured)"}){
        auto info=gap;info.kind=kind;const auto text=learnInspectionReadout(108000,info,time,48000).text;
        REQUIRE(text.contains(kind) && text.contains("Sample 108000"));}
    REQUIRE(learnInspectionReadout(50010,provisional,time,48000).text.contains("Recording Audio"));
    LearnDryView view;view.update(source,pending,time);const auto original=std::pair{view.first,view.last};
    view.update(source,pending,time);REQUIRE(std::pair(view.first,view.last)==original);
    auto jitter=time;jitter.sample=1;view.update(source,pending,jitter);REQUIRE(std::pair(view.first,view.last)==original);
    auto unavailable=time;unavailable.valid=false;view.update(source,pending,unavailable);REQUIRE(std::pair(view.first,view.last)==original);
    source.manifest.ranges={{85*96000,5*96000,CaptureCoverageKind::audio}};
    view.update(source,{},time);REQUIRE(view.first==0 && view.last>=90*96000);
    const auto distant=std::pair{view.first,view.last};source.manifest.ranges.push_back({0,4*96000,CaptureCoverageKind::silence});
    view.update(source,{},time);REQUIRE(std::pair(view.first,view.last)==distant); // 86-90 then 1-4 stays stable.
    source.manifest.ranges={{0,48000,CaptureCoverageKind::audio}};view.update(source,{},time);
    REQUIRE(std::pair(view.first,view.last)==distant); // No shrinking at loops/cancel.
    view.reset();view.update(source,{},time);REQUIRE(view.last<distant.second);
    source.manifest.ranges={{-48000,48000,CaptureCoverageKind::audio},{1000000000000LL,48000,CaptureCoverageKind::audio}};
    view.update(source,{},time);REQUIRE(view.first<=-48000 && view.last>1000000000000LL);
    REQUIRE(learnMappedFrames(source.manifest)==96000 && learnExtentFrames(source,{},time)==1000000096000.L);
    view.reset();view.update(source,{},unavailable);REQUIRE(view.first<=-48000 && view.last>1000000000000LL && std::isfinite(view.last));
}
