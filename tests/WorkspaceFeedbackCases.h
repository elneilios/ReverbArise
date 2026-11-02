#pragma once
#include <Core/MonitoringSafety.h>
inline void workspaceFeedbackCases()
{
    TestDirectory root;CaptureSession capture(root.file);juce::CriticalSection mutex;
    std::vector<float> pcm(8000);for(int n=0;n<4000;++n){pcm[n*2]=.02f+std::sin(n*.03f)*.005f;pcm[n*2+1]=-.01f;}
    REQUIRE(capture.commit(0,1000,pcm.data(),4000));PreviewContext context{capture.snapshot(),{true,0,0,1000,240,4,4},0};
    SwellEngine engine(root.file,[&]{const juce::ScopedLock guard(mutex);return context;});engine.setAutomaticProcessing(false);
    std::vector<juce::String> ids;for(int at:{100,200,300}){REQUIRE(engine.addTarget(at));ids.push_back(engine.savedSession().primary);}
    REQUIRE(engine.selectTargets(ids,ids[0]));writeTestWav(root.file.getChildFile("ir.wav"),1,1000,std::vector<float>(101,.01f));engine.loadIR(root.file.getChildFile("ir.wav"));
    await([&]{auto s=engine.savedSession();return std::all_of(s.swells.begin(),s.swells.end(),[](auto& w){return w.irName=="ir.wav";});});engine.generate(true);
    await([&]{return engine.snapshot().auditionReady && engine.savedSession().swells[2].phase==Phase::ready;});
    const auto original=engine.savedSession();const auto count=engine.snapshot().renderCount;
    REQUIRE(engine.snapshot().auditionMembers.size()==2); // Third only overlaps the neighbour, not the anchor window.
    SwellAssetStore assets(engine.assetDirectory());juce::String error;std::vector<WetRender> shaped;
    for(const auto& w:original.swells){WetRender raw,out;REQUIRE(assets.readRender(w.render,previewMemoryBudget,raw,error));REQUIRE(prepareShape(w,raw,previewMemoryBudget,out,error));shaped.push_back(std::move(out));}
    const auto oracle=[&](int64_t at){double sum=0;for(const auto& w:shaped){const auto i=at-w.start;if(i>=0 && i<static_cast<int64_t>(w.frames())){
        const auto ramp=std::min(int64_t{2},static_cast<int64_t>((w.frames()-1)/2));const auto g=std::min({1.0,static_cast<double>(i)/ramp,static_cast<double>(w.frames()-1-i)/ramp});sum+=w.samples[i*2]*g;}}return sum;};
    REQUIRE(engine.audition());float left[130]{},right[130]{};{AudioAllocationGuard guard;engine.processAudio(left,right,130,context.time);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    for(int i=0;i<120;++i){const auto edge=std::min({1.0,i/5.0,(119-i)/5.0});REQUIRE(std::abs(left[i]-oracle(i)*edge)<1e-6);}
    REQUIRE(left[0]==0 && left[119]==0 && left[120]==0);
    for(float blend:{0.0f,.5f,1.0f})
    {
        engine.setMonitoring(AuditionMode::dryWet,blend);await([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.snapshot().auditionMembers.size()==3);
        REQUIRE(engine.audition());std::vector<float> l(2200,.125f),r(2200,-.25f);engine.processAudio(l.data(),r.data(),2200,context.time);
        for(int i=10;i<2110;++i){const auto dry=i>=100?pcm[i*2]:0;REQUIRE(std::abs(l[i]-(.125+oracle(i)*blend+dry*(1-blend)))<1e-6);}
        REQUIRE(l[0]==.125f && l[2119]==.125f);
    }
    engine.setIsolateDry(true);engine.setMonitoring(AuditionMode::dryWet,0);await([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.snapshot().auditionDryStart==original.swells[0].sourceStart && engine.snapshot().auditionDryEnd==original.swells[0].sourceEnd);
    REQUIRE(engine.audition());std::vector<float> isolatedL(2200,.125f),isolatedR(2200,-.25f);
    {AudioAllocationGuard guard;engine.processAudio(isolatedL.data(),isolatedR.data(),2200,context.time);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    for(int i=0;i<2200;++i){const auto inside=i>=original.swells[0].sourceStart && i<original.swells[0].sourceEnd;
        REQUIRE(std::abs(isolatedL[i]-(.125f+(inside?pcm[i*2]:0)))<1e-6);}
    REQUIRE(engine.snapshot().renderCount==count);juce::String dryJson,dryError;Session drySaved;
    REQUIRE(encode(engine.savedSession(),dryJson,dryError) && decode(dryJson,drySaved,dryError) && drySaved.isolateDry);
    auto legacyDry=juce::JSON::parse(dryJson);legacyDry.getDynamicObject()->setProperty("version",6);legacyDry.getDynamicObject()->removeProperty("isolateDry");
    REQUIRE(decode(juce::JSON::toString(legacyDry),drySaved,dryError) && !drySaved.isolateDry);
    auto badDry=juce::JSON::parse(dryJson);badDry.getDynamicObject()->setProperty("isolateDry","yes");REQUIRE(!decode(juce::JSON::toString(badDry),drySaved,dryError));
    engine.setIsolateDry(false);
    engine.setMonitoring(AuditionMode::wetOnly,.5f);await([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.edit(ids[1],[](auto& w,auto&){w.phase=Phase::error;w.status="missing fixture";return true;}));
    await([&]{return !engine.snapshot().auditionReady && engine.snapshot().dryStatus.contains(find(engine.savedSession(),ids[1])->name);});
    // An excluded bad member cannot block isolation; a required incompatible member must block the group.
    engine.setIsolateSelected(true);await([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.snapshot().auditionMembers.size()==1);
    engine.restore(original);await([&]{return engine.snapshot().auditionReady && engine.savedSession().swells[2].phase==Phase::ready;});
    REQUIRE(engine.edit(ids[1],[](auto& w,auto&){w.render.rate=2000;return true;}));
    await([&]{return !engine.snapshot().auditionReady && engine.snapshot().dryStatus.contains(find(engine.savedSession(),ids[1])->name);});
    engine.setIsolateSelected(true);await([&]{return engine.snapshot().auditionReady;});
    engine.restore(original);await([&]{return engine.snapshot().auditionReady && engine.savedSession().swells[2].phase==Phase::ready;});
    // Exact boundary touch is excluded, and rapid membership edits cannot publish an old group.
    for(int i=0;i<20;++i)REQUIRE(engine.edit(ids[1],[&](auto& w,auto& e){return moveWet(w,(i%2)?20:19,e);}));
    REQUIRE(engine.edit(ids[1],[](auto& w,auto& e){return moveWet(w,20,e);}));
    await([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.snapshot().auditionMembers.size()==1 && engine.snapshot().renderCount==count);
    REQUIRE(engine.residentBytes()<previewMemoryBudget);
    engine.restore(original);await([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.edit(ids[0],[](auto& w,auto&){w.muted=true;return true;}));await([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.snapshot().auditionMembers.size()==1 && engine.snapshot().auditionMembers[0]==ids[1]);
    REQUIRE(engine.edit(ids[2],[](auto& w,auto&){w.solo=true;return true;}));await([&]{return !engine.snapshot().auditionReady && engine.snapshot().dryStatus.contains("No audible");});
    engine.restore(original);await([&]{return engine.snapshot().auditionReady;});
    const auto samples=shaped[0].samples;REQUIRE(engine.edit(ids[0],[](auto& w,auto& e){return moveWet(w,-1000,e);}));
    await([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.snapshot().shaped->start==shaped[0].start-1000);
    REQUIRE(engine.snapshot().renderCount==count && find(engine.savedSession(),ids[0])->render.checksum==original.swells[0].render.checksum);
    REQUIRE(find(engine.savedSession(),ids[0])->arrival==100 && find(engine.savedSession(),ids[0])->shape==original.swells[0].shape);
    // Positioned endpoint extension is inverted back into logical captured-input coordinates.
    REQUIRE(engine.edit(ids[0],[](auto& w,auto& e){auto s=w.shape;s.endOffset=400;return setShape(w,s,w.render.start,w.render.sourceEnd,e);}));
    await([&]{return engine.snapshot().auditionReady && engine.snapshot().renderCount==count+1;});
    const auto extended=*find(engine.savedSession(),ids[0]);
    REQUIRE(extended.render.sourceEnd==500 && extended.sourceEnd==350 && extended.placement==-1000 && extended.arrival==100);
    REQUIRE(engine.snapshot().shaped->end==-500 && engine.snapshot().shaped->start==-1000);
    engine.setAutoMarkers(false);engine.setIsolateSelected(true);auto saved=engine.savedSession();engine.restore(saved);await([&]{return engine.snapshot().auditionReady;});
    REQUIRE(!engine.savedSession().autoMarkersEnabled && engine.savedSession().isolateSelected && !engine.snapshot().playing);
}
