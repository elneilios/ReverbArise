#include "TestSupport.h"
#include <Analysis/AutomaticTargets.h>
#include <Core/TransportTime.h>
using namespace m3;
int main(){return runTest([]
{
    {Session tail;SongTime t{true,0,0,1000,120,4,4};juce::String error;
        REQUIRE(reconcileTargets(tail,{{990,1}},0,1000,t,error));auto& w=tail.swells[0];
        REQUIRE(w.sourceEnd==1490 && w.shape.endOffset==20);
        const CompletedPasses closed{{0,1000}};REQUIRE(updateAutomaticWindows(tail,t,error,closed));
        REQUIRE(w.sourceEnd==1000 && w.shape.endOffset==10 && w.shape.fadeOut==10);
        const auto generation=w.inputGeneration;REQUIRE(updateAutomaticWindows(tail,t,error,closed) && w.inputGeneration==generation);
        juce::String json;Session restored;REQUIRE(encode(tail,json,error) && decode(json,restored,error));
        REQUIRE(restored.swells[0].sourceEnd==1000 && restored.swells[0].shape.endOffset==10);
        REQUIRE(updateAutomaticWindows(tail,t,error) && w.sourceEnd==1490 && w.shape.endOffset==20);
        w.provenance=Provenance::manualEdited;w.shape.inTension=.3;
        REQUIRE(updateAutomaticWindows(tail,t,error,closed) && w.sourceEnd==1490 && w.shape.inTension==.3);}
    { // Actual fractional-tempo endpoint pair observed in FL Studio at 82 BPM.
        SongTime t{true,0,0,48000,82,4,4};int64_t end;REQUIRE(resolvePpqSample(t,36,end) && end==1264390);
        CaptureManifest map;map.sampleRate=48000;map.ranges={{0,static_cast<uint64_t>(end+1),CaptureCoverageKind::silence}};
        const CompletedPasses pass{{0,end}};REQUIRE(usableCompletedPasses(map,pass)==pass);
        Session tail;juce::String error;REQUIRE(reconcileTargets(tail,{{1229312,1}},0,end,t,error));
        REQUIRE(updateAutomaticWindows(tail,t,error,usableCompletedPasses(map,pass)) && tail.swells[0].sourceEnd==end);
        const auto id=tail.swells[0].id;tail.swells[0].provenance=Provenance::manualEdited;
        tail.swells[0].sourceEnd=end+100;tail.swells[0].frozen=false;
        REQUIRE(updateAutomaticWindows(tail,t,error,pass) && find(tail,id)->sourceEnd==end+100);
        map.ranges.back().frameCount=end+2;REQUIRE(usableCompletedPasses(map,pass).empty());
        CaptureManifest extreme;extreme.ranges={{INT64_MAX-10,10,CaptureCoverageKind::silence}};
        REQUIRE(usableCompletedPasses(extreme,{{INT64_MAX-10,INT64_MAX}}).size()==1);
    }
    Session s;SongTime time{true,0,0,48000,120,4,4};juce::String error;
    REQUIRE(reconcileTargets(s,{{1000,1},{1000,2},{30000,1}},0,48000,time,error));
    REQUIRE(s.swells.size()==2 && s.swells[0].sourceEnd==25000);
    const auto first=s.swells[0].id,second=s.swells[1].id;
    REQUIRE(select(s,{first},first));
    REQUIRE(reconcileTargets(s,{{1010,1},{30000,1}},0,48000,time,error));
    REQUIRE(s.swells.size()==2 && s.swells[0].id==first && s.primary==first && s.swells[0].arrival==1010);
    auto& edited=s.swells[0];edited.provenance=Provenance::manualEdited;
    edited.arrival=edited.sourceStart=7000;edited.sourceEnd=8000;edited.name="Protected";
    REQUIRE(reconcileTargets(s,{{1020,1},{30000,1}},0,48000,time,error));
    REQUIRE(s.swells.size()==2 && s.swells[0].arrival==7000 && s.swells[0].name=="Protected");
    REQUIRE(suppressAndErase(s,first,error) && s.suppressed.size()==1);
    REQUIRE(reconcileTargets(s,{{1030,1},{30000,1}},0,48000,time,error));
    REQUIRE(s.swells.size()==1 && s.swells[0].id==second);
    juce::String json;Session restored;REQUIRE(encode(s,json,error) && decode(json,restored,error));
    REQUIRE(reconcileTargets(restored,{{1030,1},{30000,1}},0,48000,time,error) && restored.swells.size()==1);
    REQUIRE(reconcileTargets(s,{},0,20000,time,error) && s.swells.size()==1);
    REQUIRE(reconcileTargets(s,{},20000,48000,time,error) && s.swells.empty());
    s.swells.emplace_back();s.swells.back().provenance=Provenance::automatic;
    s.suppressed.resize(maximumSwells);const auto retained=s.swells.back().id;
    REQUIRE(!suppressAndErase(s,retained,error) && find(s,retained));
    Session bounds;REQUIRE(reconcileTargets(bounds,{{0,1},{100,1}},0,1000,time,error));
    REQUIRE(bounds.swells[0].sourceEnd==100 && bounds.swells[1].sourceEnd==24100);
    REQUIRE(bounds.swells[0].sourceStart==0); // No coverage clamp or pre-roll requirement.
    const auto prior=bounds.swells[0].id;
    REQUIRE(!reconcileTargets(bounds,{},1000,0,time,error) && bounds.swells[0].id==prior);
    Session ties;REQUIRE(reconcileTargets(ties,{{80,1},{120,1}},0,1000,time,error));
    const auto winner=std::min(ties.swells[0].id,ties.swells[1].id);
    REQUIRE(reconcileTargets(ties,{{100,1}},0,1000,time,error));
    REQUIRE(ties.swells.size()==1 && ties.swells[0].id==winner);
});}
