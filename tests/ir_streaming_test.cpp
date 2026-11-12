#include "AudioTestFiles.h"
#include "AllocationGuard.h"
#include <DSP/SwellEngine.h>
#include <Core/MonitoringSafety.h>
#define NOMINMAX
#include <Windows.h>
#include <psapi.h>
#include <source_location>
using namespace m3;
template<class F>static void waitFor(F f,const std::source_location loc=std::source_location::current())
{const auto until=juce::Time::getMillisecondCounter()+60000;while(!f()){if(juce::Time::getMillisecondCounter()>until)throw std::runtime_error("IR streaming timeout "+std::to_string(loc.line()));juce::Thread::sleep(2);}}
static float reference(const Swell& w,const WetRender& raw,int64_t p)
{int64_t a,b;juce::String error;REQUIRE(shapeBounds(w,raw.start,raw.sourceEnd,a,b,error));if(p<a || p>=b)return 0;
 return static_cast<float>(raw.samples[(p-w.placement-raw.start)*2]*envelope(w.shape,p,a,b)*monitorEdgeGain(static_cast<size_t>(p-a),static_cast<size_t>(b-a),96000,.002));}
int main(int argc,char** argv){return runTest([&]
{
    constexpr double rate=96000;constexpr int64_t target=12*96000;
    TestDirectory root;CaptureSession capture(root.file);std::vector<float> source(96000*2,0);source[0]=.2f;source[1]=-.1f;source[48000]=.15f;source[48001]=.08f;
    REQUIRE(capture.commit(target,rate,source.data(),96000));juce::CriticalSection mutex;
    PreviewContext context{capture.snapshot(),{true,target,24,rate,120,4,4},0};
    SwellEngine engine(root.file,[&]{const juce::ScopedLock guard(mutex);return context;});engine.setAutomaticProcessing(false);
    waitFor([&]{return engine.savedSession().defaultIR.present();});REQUIRE(engine.addTarget(target));const auto a=engine.savedSession().primary;
    REQUIRE(engine.addTarget(target+24000));const auto b=engine.savedSession().primary;
    REQUIRE(engine.assignBundledIR(a,find(engine.savedSession(),a)->inputGeneration,"shimmer-cloud"));
    REQUIRE(engine.assignBundledIR(b,find(engine.savedSession(),b)->inputGeneration,"shimmer-cloud"));
    waitFor([&]{const auto s=engine.savedSession();return find(s,a)->irName=="Shimmer Cloud" && find(s,b)->irName=="Shimmer Cloud";});
    engine.generate(true);waitFor([&]{const auto s=engine.savedSession();return find(s,a)->phase==Phase::ready && find(s,b)->phase==Phase::ready;});
    REQUIRE(engine.selectTargets({a},a));const auto baseline=engine.savedSession();const auto initialRenders=engine.snapshot().renderCount;
    const auto cacheBytes=[&]{int64_t bytes=0;for(const auto& f:engine.assetDirectory().findChildFiles(juce::File::findFiles,true))bytes+=f.getSize();return bytes;};const auto baselineCache=cacheBytes();
    float left[512]{},right[512]{};
    const auto time=[&](int64_t p,bool playing){const juce::ScopedLock guard(mutex);context.time.sample=p;context.time.ppq=p/48000.;return TransportTime{context.time,playing};};
    auto host=time(target-5*96000,false);engine.processTransportAudio(left,right,512,host);waitFor([&]{return engine.snapshot().liveReady;});
    const auto start=juce::Time::getMillisecondCounterHiRes();double nextBlock=start;int64_t p=host.song.sample;uint64_t peakAccounted=0;int blocks=0;
    engine.holdRenderPublicationForTesting(true);
    REQUIRE(engine.assignBundledIR(a,find(engine.savedSession(),a)->inputGeneration,"monumental"));
    while(true){
        const auto state=engine.snapshot();REQUIRE(state.irReplacementError.isEmpty());
        if(find(state.session,a)->irName=="Monumental" && state.liveReady)break;
        REQUIRE(juce::Time::getMillisecondCounterHiRes()-start<30000);
        while(juce::Time::getMillisecondCounterHiRes()<nextBlock)juce::Thread::sleep(1);
        host=time(p,true);std::fill_n(left,512,0.f);std::fill_n(right,512,0.f);
        {AudioAllocationGuard guard;engine.processTransportAudio(left,right,512,host);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        // Both old and new intervals cover this test window; page starvation is a failure.
        if(engine.snapshot().liveMissing){const auto failure=engine.snapshot();std::cerr<<"Missing at "<<p<<" after "<<(juce::Time::getMillisecondCounterHiRes()-start)<<" ms | "<<failure.liveStatus<<" | "<<find(failure.session,a)->status<<"\n";}
        REQUIRE(!engine.snapshot().liveMissing);p+=512;nextBlock+=512000./rate;++blocks;
        if(blocks==11)engine.holdRenderPublicationForTesting(false);
        peakAccounted=std::max(peakAccounted,engine.residentBytes());REQUIRE(peakAccounted<previewMemoryBudget);
    }
    const auto firstMs=juce::Time::getMillisecondCounterHiRes()-start;REQUIRE(blocks>10 && find(engine.savedSession(),b)->render==find(baseline,b)->render);
    const auto readyRenders=engine.snapshot().renderCount;REQUIRE(readyRenders==initialRenders+1);
    const auto repeat=juce::Time::getMillisecondCounterHiRes();
    REQUIRE(engine.assignBundledIR(a,find(engine.savedSession(),a)->inputGeneration,"monumental"));
    waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready && engine.snapshot().retainedIRName.isEmpty();});
    const auto repeatMs=juce::Time::getMillisecondCounterHiRes()-repeat;REQUIRE(engine.snapshot().renderCount==readyRenders);
    // A cold bounce starts during actual long work, then must keep the entry snapshot.
    engine.restore(baseline);waitFor([&]{const auto s=engine.savedSession();return find(s,a)->phase==Phase::ready && find(s,b)->phase==Phase::ready;});
    host=time(target-2*96000,false);engine.processTransportAudio(left,right,512,host);waitFor([&]{return engine.snapshot().liveReady;});
    REQUIRE(engine.assignBundledIR(a,find(engine.savedSession(),a)->inputGeneration,"monumental"));
    REQUIRE(engine.snapshot().irReplacementPending);
    std::fill_n(left,512,0.f);std::fill_n(right,512,0.f); // Export receives zero dry, not the previous live output.
    engine.processOfflineAudio(left,right,512,host);
    SwellAssetStore assets(engine.assetDirectory());WetRender oldA,oldB;juce::String error;
    REQUIRE(assets.readRender(find(baseline,a)->render,previewMemoryBudget,oldA,error));REQUIRE(assets.readRender(find(baseline,b)->render,previewMemoryBudget,oldB,error));
    // Independent reference includes the conservative aggregate wet peak guard.
    double bound=0;for(const auto* w:{find(baseline,a),find(baseline,b)}){const auto& raw=w->id==a?oldA:oldB;float peak=0;
        for(int64_t at=raw.start;at<raw.sourceEnd;++at)peak=std::max(peak,std::abs(reference(*w,raw,at)));bound+=peak;}
    const auto guard=std::min(1.,std::pow(10.,-6./20)/bound);
    for(int i=480;i<512;++i)REQUIRE(std::abs(left[i]-(reference(*find(baseline,a),oldA,host.song.sample+i)+reference(*find(baseline,b),oldB,host.song.sample+i))*guard)<2e-6);
    REQUIRE(engine.snapshot().offlineStatus.contains("accepted IRs") && !engine.snapshot().offlineStatus.contains("incomplete"));engine.endOffline();
    PROCESS_MEMORY_COUNTERS memory{};memory.cb=sizeof(memory);REQUIRE(GetProcessMemoryInfo(GetCurrentProcess(),&memory,sizeof(memory)));
    auto report=juce::var(new juce::DynamicObject);auto* fields=report.getDynamicObject();fields->setProperty("rate",rate);
    fields->setProperty("first_assignment_to_playable_ms",firstMs);fields->setProperty("repeat_assignment_to_playable_ms",repeatMs);
    fields->setProperty("paced_blocks_without_missing_pages",blocks);fields->setProperty("sampled_accounted_peak_bytes",static_cast<juce::int64>(peakAccounted));
    fields->setProperty("process_peak_working_set_bytes",static_cast<juce::int64>(memory.PeakWorkingSetSize));
    fields->setProperty("baseline_cache_bytes",static_cast<juce::int64>(baselineCache));fields->setProperty("final_cache_bytes",static_cast<juce::int64>(cacheBytes()));
    if(argc>1)REQUIRE(juce::File(juce::String::fromUTF8(argv[1])).replaceWithText(juce::JSON::toString(report)));
});}
