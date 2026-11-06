#include "TestSupport.h"
#include <Analysis/AutomaticTargets.h>
#include <Core/TransportTime.h>
#include <UI/Controls/SwellListOrder.h>
using namespace m3;
int main(){return runTest([]
{
    {
        Session s;SongTime t{true,0,0,48000,120,4,4};juce::String error;
        REQUIRE(reconcileTargets(s,{{200000,1},{230000,1}},192000,250000,t,error));
        const auto late=s.swells.front().id;
        s.swells.front().name="AAA user name";s.swells.front().enabled=false;
        s.swells.front().provenance=Provenance::manualEdited;s.swells.front().placement=37;s.swells.front().shape.gainDb=-8;
        const auto pinned=s.swells.front();
        REQUIRE(reconcileTargets(s,{{12000,1},{40000,1}},0,48000,t,error));
        REQUIRE(s.swells.size()==4 && s.swells.front().id==late);
        const auto early=s.swells[2].id;s.swells[2].name="ZZZ user name";
        REQUIRE(select(s,{late,early},early));
        const auto check=[&](const Session& state)
        {
            const auto order=m3::ui::controls::chronologicalSwellOrder(std::span<const Swell>(state.swells));
            REQUIRE(state.swells[order.front()].id==early && state.swells[order.back()].arrival==230000);
            REQUIRE(state.primary==early && state.selected==std::vector<juce::String>({late,early}));
            REQUIRE(sameSavedSwell(pinned,*find(state,late)));
        };
        check(s);
        REQUIRE(reconcileTargets(s,{{12010,1},{40000,1}},0,48000,t,error));check(s);
        REQUIRE(reconcileTargets(s,{{12010,1},{40000,1}},0,48000,t,error));check(s);
        juce::String json;Session restored;REQUIRE(encode(s,json,error) && decode(json,restored,error));check(restored);
        REQUIRE(restored.swells.front().id==late);
    }
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
    { // Ownership follows the arrival, even when extraction crosses into B.
        SongTime t{true,0,0,48000,120,4,4};juce::String error;
        CaptureManifest map;map.schemaVersion=2;
        CaptureGenerationContext a,b;a.automatic=b.automatic=true;
        for(auto* c:{&a,&b}){const auto hash=juce::String::repeatedString(c==&a?"a":"b",64);
            c->ir.path="irs/"+hash+".wav";c->ir.content=c->ir.checksum=hash;c->ir.rate=48000;c->ir.channels=1;c->ir.frames=32;
            c->irName=c==&a?"Pass A":"Pass B";c->id=captureGenerationContextId(*c);}
        map.generationContexts={a,b};map.contextSpans={{0,24000,a.id},{24000,96000,b.id}};
        Session session;session.defaultIR=b.ir;session.defaultIRName=b.irName;
        REQUIRE(reconcileTargets(session,{{23900,1},{60000,1}},0,96000,t,error,{},&map));
        REQUIRE(session.swells.size()==2 && session.swells[0].sourceEnd>24000);
        REQUIRE(session.swells[0].ir==a.ir && session.swells[1].ir==b.ir);
        const auto first=session.swells[0].id,second=session.swells[1].id;
        session.swells[0].render=a.ir;session.swells[0].render.path="renders/"+a.ir.content+".wav";
        session.swells[0].frozen=true;session.swells[0].shape.gainDb=-7;
        const auto protectedSwell=session.swells[0];
        REQUIRE(reconcileTargets(session,{{23910,1},{60000,1}},0,96000,t,error,{},&map));
        REQUIRE(find(session,first)->arrival==23900 && find(session,first)->shape.gainDb==-7 && find(session,first)->ir==a.ir);
        REQUIRE(sameSavedSwell(*find(session,first),protectedSwell));
        REQUIRE(reconcileTargets(session,{},0,24000,t,error,{},&map) && find(session,first));
        REQUIRE(suppressAndErase(session,second,error));
        REQUIRE(reconcileTargets(session,{{60000,1}},24000,96000,t,error,{},&map) && session.swells.size()==1);
        // Absent provenance never borrows the currently selected default.
        REQUIRE(reconcileTargets(session,{{120000,1}},100000,140000,t,error,{},&map));
        REQUIRE(session.swells.back().phase==Phase::error && !session.swells.back().ir.present());
    }
    Session s;SongTime time{true,0,0,48000,120,4,4};juce::String error;
    {
        Session named;REQUIRE(reconcileTargets(named,{{1632000,1},{1638000,1}},1600000,1700000,time,error));
        REQUIRE(named.swells.size()==2 && named.swells[0].name=="Swell Bar 18.1" && named.swells[1].name=="Swell Bar 18.1.25" && named.swells[0].id!=named.swells[1].id);
        const auto id=named.swells[0].id;named.swells[0].name="A saved custom name";
        REQUIRE(reconcileTargets(named,{{1632010,1},{1638000,1}},1600000,1700000,time,error));
        REQUIRE(find(named,id) && find(named,id)->name=="A saved custom name");
        REQUIRE(moveWet(*find(named,id),1000,error) && find(named,id)->name=="A saved custom name");
        juce::String text;Session reopen;REQUIRE(encode(named,text,error) && decode(text,reopen,error));
        REQUIRE(find(reopen,id)->name=="A saved custom name");
    }
    REQUIRE(reconcileTargets(s,{{1000,1},{1000,2},{30000,1}},0,48000,time,error));
    REQUIRE(s.swells.size()==2 && s.swells[0].sourceEnd==25000);
    const auto first=s.swells[0].id,second=s.swells[1].id;
    REQUIRE(select(s,{first},first));
    REQUIRE(reconcileTargets(s,{{1010,1},{30000,1}},0,48000,time,error));
    REQUIRE(s.swells.size()==2 && s.swells[0].id==first && s.primary==first && s.swells[0].arrival==1010);
    auto& edited=s.swells[0];edited.provenance=Provenance::manualEdited;
    edited.arrival=edited.sourceStart=7000;edited.sourceEnd=8000;edited.name="Protected";
    const auto protectedEdit=edited;
    REQUIRE(reconcileTargets(s,{{1020,1},{30000,1}},0,48000,time,error));
    REQUIRE(s.swells.size()==2 && s.swells[0].arrival==7000 && s.swells[0].name=="Protected");
    REQUIRE(sameSavedSwell(*find(s,first),protectedEdit));
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
