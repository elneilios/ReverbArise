#include "AudioTestFiles.h"
#include "AllocationGuard.h"
#include <DSP/SwellEngine.h>
#include <source_location>
using namespace m3;
template<class F>static void await(F f,const std::source_location loc=std::source_location::current())
{const auto deadline=juce::Time::getMillisecondCounter()+30000;while(!f()){if(juce::Time::getMillisecondCounter()>deadline)throw std::runtime_error("Automatic timeout "+std::to_string(loc.line()));juce::Thread::sleep(3);}}

#include "PassOwnedGenerationCases.h"
int main(){return runTest([]
{
    passOwnedGenerationCases();
    TestDirectory root;CaptureSession capture(root.file);juce::CriticalSection mutex;
    PreviewContext context{capture.snapshot(),{true,0,0,48000,120,4,4},0};
    SwellEngine engine(root.file,[&]{const juce::ScopedLock guard(mutex);return context;});
    await([&]{return engine.savedSession().defaultIR.present();});
    std::vector<float> source(48000*4*2);
    for(int start:{12000,60000,120000})for(int i=0;i<12000;++i)
    {const auto v=static_cast<float>(0.25*std::exp(-i/2000.0)*std::sin(i*.13));source[(start+i)*2]=v;source[(start+i)*2+1]=v*.7f;}
    const auto commit=[&](int64_t a,const float* data,uint32_t frames)
    {REQUIRE(capture.commit(a,48000,data,frames));{const juce::ScopedLock guard(mutex);context.capture=capture.snapshot();}engine.captureCommitted(a,a+frames,0);};
    engine.transportPlaying(true);commit(0,source.data(),192000);
    await([&]{const auto s=engine.savedSession();return s.swells.size()>=2 && std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==Phase::ready;});});
    const auto initial=engine.savedSession();const auto count=engine.snapshot().renderCount;const auto analyses=engine.snapshot().analysisCount;
    REQUIRE(count>=2 && initial.swells[0].ir.content==initial.defaultIR.content);
    REQUIRE(engine.selectTargets({initial.swells[0].id},initial.swells[0].id));await([&]{return engine.snapshot().auditionReady;});
    float left[64]{},right[64]{};
    {AudioAllocationGuard guard;engine.processAudio(left,right,64,context.time);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && std::all_of(left,left+64,[](float v){return v==0;}));
    commit(0,source.data(),192000);juce::Thread::sleep(200);
    await([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.snapshot().renderCount==count && engine.savedSession().swells[0].id==initial.swells[0].id);
    REQUIRE(engine.snapshot().analysisCount==analyses);
    // A failed/non-coherent worker timing snapshot while stopped is not an
    // asset change. Repeated availability gaps must not requeue settled swells.
    engine.processTransportAudio(left,right,64,TransportTime{context.time,false});
    const auto settled=engine.savedSession();
    for(int pass=0;pass<3;++pass){
        {const juce::ScopedLock guard(mutex);context.time.valid=false;}
        juce::Thread::sleep(60);
        const auto state=engine.savedSession();
        for(const auto& original:settled.swells){const auto w=find(state,original.id);REQUIRE(w && w->phase==Phase::ready && w->inputGeneration==original.inputGeneration);}
        {const juce::ScopedLock guard(mutex);context.time.valid=true;}
        juce::Thread::sleep(30);
    }
    engine.transportPlaying(true);
    {const juce::ScopedLock guard(mutex);context.time.bpm=0;}
    juce::Thread::sleep(60);
    for(const auto& w:engine.savedSession().swells)REQUIRE(w.phase==Phase::ready);
    {const juce::ScopedLock guard(mutex);context.time.bpm=120;}
    REQUIRE(engine.edit(initial.swells[0].id,[](auto& w,auto&){w.name="Keep this";w.shape.gainDb=-3;return true;}));
    await([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.snapshot().renderCount==count);
    REQUIRE(engine.savedSession().swells[0].provenance==Provenance::manualEdited);
    const auto movedAutomatic=initial.swells.back().id;const auto movedAnchor=find(engine.savedSession(),movedAutomatic)->detectorAnchor;
    REQUIRE(engine.edit(movedAutomatic,[](auto& w,auto& e){return moveWet(w,-13,e);}));
    REQUIRE(find(engine.savedSession(),movedAutomatic)->provenance==Provenance::manualEdited);
    commit(0,source.data(),192000);juce::Thread::sleep(100);
    REQUIRE(find(engine.savedSession(),movedAutomatic)->placement==-13 && find(engine.savedSession(),movedAutomatic)->detectorAnchor==movedAnchor);
    const auto beforePreferences=engine.snapshot().analysisCount;
    engine.setPreferences(-3,.6f,.08,false);
    await([&]{return engine.snapshot().analysisCount>beforePreferences && !engine.snapshot().analyzing && engine.snapshot().pendingIntervals==0;});
    REQUIRE(engine.savedSession().swells[0].id==initial.swells[0].id && engine.savedSession().swells[0].name=="Keep this");
    engine.setPreferences(0,.5f,.08,false);
    await([&]{return !engine.snapshot().analyzing && engine.snapshot().pendingIntervals==0;});
    // Superseded setting/enable generations must not publish after the final Off.
    for(int i=0;i<10;++i){engine.setAutoMarkers(true);engine.setPreferences(0,.4f+i*.01f,.08,false);engine.setAutoMarkers(false);}
    const auto disabledState=engine.savedSession();juce::Thread::sleep(250);
    REQUIRE(engine.savedSession().swells.size()==disabledState.swells.size());
    for(size_t i=0;i<disabledState.swells.size();++i)REQUIRE(engine.savedSession().swells[i].id==disabledState.swells[i].id);
    engine.setAutoMarkers(false);const auto offAnalyses=engine.snapshot().analysisCount;
    commit(0,source.data(),192000);engine.setPreferences(0,.1f,.08,false);juce::Thread::sleep(250);
    REQUIRE(engine.snapshot().analysisCount==offAnalyses && engine.savedSession().swells[0].name=="Keep this");
    engine.setAutoMarkers(true);await([&]{return engine.snapshot().analysisCount>offAnalyses && !engine.snapshot().analyzing && engine.snapshot().pendingIntervals==0;});
    REQUIRE(engine.savedSession().swells[0].id==initial.swells[0].id);engine.setPreferences(0,.5f,.08,false);
    await([&]{return !engine.snapshot().analyzing && engine.snapshot().pendingIntervals==0;});
    const auto second=initial.swells[1].id;REQUIRE(engine.selectTargets({second},second));engine.removeSelected();
    commit(0,source.data(),192000);juce::Thread::sleep(200);
    REQUIRE(!find(engine.savedSession(),second));REQUIRE(engine.savedSession().swells.size()==initial.swells.size()-1);
    // Future-default changes never overwrite an existing assignment.
    const auto replacement=root.file.getChildFile("future.wav");writeTestWav(replacement,1,48000,{1,.5f,.25f});
    engine.loadDefaultIR(replacement);await([&]{return engine.savedSession().defaultIRName=="future.wav";});
    REQUIRE(engine.savedSession().swells[0].ir.content==initial.defaultIR.content);
    REQUIRE(engine.addTarget(170000));auto future=engine.savedSession().swells.back();
    REQUIRE(future.ir.content==engine.savedSession().defaultIR.content && future.irName=="future.wav");
    await([&]{return find(engine.savedSession(),future.id)->phase==Phase::ready;});
    // Incomplete extraction windows remain waiting until actual source is committed.
    std::vector<float> partial(4000*2);for(int i=0;i<4000;++i)partial[i*2]=static_cast<float>(.2*std::sin(i*.2)*std::exp(-i/1000.0));
    engine.transportPlaying(false);commit(500000,partial.data(),4000);
    await([&]{const auto s=engine.savedSession();return std::any_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.arrival>=500000 && w.status=="Waiting for source";});});
    REQUIRE(engine.savedSession().swells[0].name=="Keep this");
    std::vector<float> tail(24000*2,0);commit(504000,tail.data(),24000);
    await([&]{const auto state=engine.savedSession();return std::any_of(state.swells.begin(),state.swells.end(),[](const auto& w){return w.arrival>=500000 && w.phase==Phase::ready;});});
    const auto store=engine.assetDirectory();const auto missing=engine.savedSession().defaultIR;
    REQUIRE(store.getChildFile(missing.path).deleteFile());
    REQUIRE(engine.addTarget(180000));const auto blocked=engine.savedSession().primary;
    await([&]{return find(engine.savedSession(),blocked)->phase==Phase::error;});
    REQUIRE(engine.savedSession().swells[0].ir.content==initial.defaultIR.content);
    engine.loadIR(replacement);await([&]{return find(engine.savedSession(),blocked)->phase==Phase::ready;});
    REQUIRE(find(engine.savedSession(),blocked)->ir.content==missing.content);
    // Rapid selected changes coalesce and leave unrelated per-swell assets intact.
    const auto retainedId=initial.swells[0].id;
    REQUIRE(engine.selectTargets({retainedId},retainedId));
    const auto beforeEdits=engine.savedSession();
    const auto alternate=root.file.getChildFile("alternate.wav");writeTestWav(alternate,1,48000,{.4f,.2f,.1f});
    engine.loadIR(replacement,true);engine.loadIR(alternate,true);
    await([&]{const auto state=engine.savedSession();return find(state,retainedId)->phase==Phase::ready && find(state,retainedId)->irName=="alternate.wav";});
    for(const auto& w:beforeEdits.swells)if(w.id!=retainedId)REQUIRE(find(engine.savedSession(),w.id)->render.path==w.render.path);
    const auto beforeSource=engine.snapshot().renderCount;
    for(int n=0;n<3;++n)REQUIRE(engine.edit(retainedId,[](auto& w,auto&){--w.sourceEnd;return true;}));
    await([&]{return find(engine.savedSession(),retainedId)->phase==Phase::ready;});
    REQUIRE(engine.snapshot().renderCount==beforeSource+1);
    auto changedSource=source;const auto changedAt=find(engine.savedSession(),retainedId)->sourceStart+128;
    changedSource[static_cast<size_t>(changedAt)*2]=.23f;commit(0,changedSource.data(),192000);
    await([&]{return find(engine.savedSession(),retainedId)->phase==Phase::ready && engine.snapshot().renderCount>beforeSource+1;});
    // Pressure on the bounded dirty summary is drained without losing manual definitions.
    for(int n=0;n<600;++n)engine.captureCommitted(n*400,n*400+100,0);
    await([&]{const auto state=engine.snapshot();return state.pendingIntervals==0 && !state.analyzing;});
    REQUIRE(find(engine.savedSession(),retainedId)->name=="Keep this" && engine.residentBytes()<previewMemoryBudget);
    // The same audio committed across onset boundaries converges to whole-run targets.
    TestDirectory incrementalRoot;CaptureSession incrementalCapture(incrementalRoot.file);juce::CriticalSection incrementalLock;
    PreviewContext incrementalContext{incrementalCapture.snapshot(),context.time,0};
    SwellEngine incremental(incrementalRoot.file,[&]{const juce::ScopedLock guard(incrementalLock);return incrementalContext;});
    incremental.transportPlaying(true);
    for(int at=0;at<192000;at+=10000)
    {
        const auto frames=std::min(10000,192000-at);REQUIRE(incrementalCapture.commit(at,48000,source.data()+at*2,frames));
        {const juce::ScopedLock guard(incrementalLock);incrementalContext.capture=incrementalCapture.snapshot();}
        incremental.captureCommitted(at,at+frames,0);juce::Thread::sleep(10);
    }
    incremental.transportPlaying(false);
    try {await([&]{const auto state=incremental.savedSession();return state.swells.size()==initial.swells.size() && std::all_of(state.swells.begin(),state.swells.end(),[](const auto& w){return w.phase==Phase::ready;});});}
    catch(...){const auto state=incremental.snapshot();std::cerr<<"Expected "<<initial.swells.size()<<" got "<<state.session.swells.size()<<" "<<state.message<<"\n";for(const auto& w:state.session.swells)std::cerr<<w.arrival<<" "<<w.status<<"\n";throw;}
    auto chunked=incremental.savedSession();
    for(const auto& target:initial.swells)
        REQUIRE(std::any_of(chunked.swells.begin(),chunked.swells.end(),[&](const auto& w){return std::abs(w.arrival-target.arrival)<=analysisWindow(48000);}));
    REQUIRE(incremental.residentBytes()<previewMemoryBudget);
    TestDirectory failedRoot;CaptureSession failedCapture(failedRoot.file);
    const auto failedMap=failedCapture.snapshot().manifest;
    const auto failedStore=failedRoot.file.getChildFile(failedMap.projectId).getChildFile(failedMap.instanceId);
    REQUIRE(failedStore.createDirectory().wasOk());REQUIRE(failedStore.getChildFile("irs").replaceWithText("blocked directory"));
    PreviewContext failedContext{failedCapture.snapshot(),context.time,0};
    SwellEngine failedDefault(failedRoot.file,[&]{return failedContext;});
    await([&]{return failedDefault.snapshot().defaultIRError.isNotEmpty();});
    REQUIRE(!failedDefault.savedSession().defaultIR.present() && !failedDefault.snapshot().auditionReady);
});}
