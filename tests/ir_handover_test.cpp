#include "AudioTestFiles.h"
#include "AllocationGuard.h"
#include <DSP/SwellEngine.h>
#include <DSP/DefaultImpulse.h>
#include <Core/MonitoringSafety.h>
#include <source_location>
using namespace m3;
template<class F>static void waitFor(F test,const std::source_location line=std::source_location::current())
{const auto end=juce::Time::getMillisecondCounter()+30000;while(!test()){if(juce::Time::getMillisecondCounter()>end)throw std::runtime_error("IR handover timeout "+std::to_string(line.line()));juce::Thread::sleep(2);}}
static float sample(const Swell& w,const WetRender& raw,int64_t p)
{
    int64_t a,b;juce::String error;REQUIRE(shapeBounds(w,raw.start,raw.sourceEnd,a,b,error));
    if(p<a || p>=b)return 0;const auto at=p-w.placement-raw.start;
    return static_cast<float>(raw.samples[at*2]*envelope(w.shape,p,a,b)*monitorEdgeGain(static_cast<size_t>(p-a),static_cast<size_t>(b-a),1000,.002));
}
int main(){return runTest([]
{
    TestDirectory root;CaptureSession capture(root.file);std::vector<float> source(10000,.125f);REQUIRE(capture.commit(0,1000,source.data(),5000));
    juce::CriticalSection mutex;PreviewContext context{capture.snapshot(),{true,950,3.8,1000,240,4,4},0};
    SwellEngine engine(root.file,[&]{const juce::ScopedLock guard(mutex);return context;});engine.setAutomaticProcessing(false);
    waitFor([&]{return engine.savedSession().defaultIR.present();});
    REQUIRE(engine.addTarget(1000));const auto a=engine.savedSession().primary;REQUIRE(engine.addTarget(1100));const auto b=engine.savedSession().primary;
    REQUIRE(engine.selectTargets({a,b},a));const auto oldFile=root.file.getChildFile("accepted.wav");writeTestWav(oldFile,1,1000,std::vector<float>(201,.001f));
    engine.loadIR(oldFile);waitFor([&]{return find(engine.savedSession(),a)->irName=="accepted.wav" && find(engine.savedSession(),b)->irName=="accepted.wav";});
    engine.generate(true);waitFor([&]{const auto s=engine.savedSession();return find(s,a)->phase==Phase::ready && find(s,b)->phase==Phase::ready;});
    REQUIRE(engine.selectTargets({a},a));
    const auto original=engine.savedSession();SwellAssetStore assets(engine.assetDirectory());WetRender rawA,rawB;juce::String error;
    REQUIRE(assets.readRender(find(original,a)->render,previewMemoryBudget,rawA,error));REQUIRE(assets.readRender(find(original,b)->render,previewMemoryBudget,rawB,error));
    const auto position=[&](int64_t p,bool playing){const juce::ScopedLock guard(mutex);context.time.sample=p;context.time.ppq=(p-(playing?16:0))/250.;return TransportTime{context.time,playing};};
    float l[80]{},r[80]{};auto time=position(950,false);engine.processTransportAudio(l,r,16,time);waitFor([&]{return engine.snapshot().liveReady;});
    time=position(950,true);engine.processTransportAudio(l,r,16,time);
    REQUIRE(std::abs(l[15]-sample(*find(original,a),rawA,965)-sample(*find(original,b),rawB,965))<1e-6);
    const auto invalid=root.file.getChildFile("invalid.wav");REQUIRE(invalid.replaceWithText("not audio"));
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::error;});
    REQUIRE(engine.snapshot().retainedIRName=="accepted.wav" && engine.snapshot().requestedIRName=="invalid.wav");
    waitFor([&]{return engine.snapshot().liveReady;});time=position(966,true);std::fill_n(l,80,0.f);std::fill_n(r,80,0.f);
    {AudioAllocationGuard guard;engine.processTransportAudio(l,r,16,time);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    for(int i=0;i<16;++i)REQUIRE(std::abs(l[i]-sample(*find(original,a),rawA,966+i)-sample(*find(original,b),rawB,966+i))<1e-6);
    REQUIRE(engine.assignBundledIR(a,find(engine.savedSession(),a)->inputGeneration,"monumental"));
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::error;});
    REQUIRE(engine.snapshot().retainedIRName=="accepted.wav" && find(engine.savedSession(),b)->render==find(original,b)->render);
    // A bounce intentionally uses the accepted version despite the requested error.
    time=position(950,true);time.playing=false;std::fill_n(l,80,0.f);std::fill_n(r,80,0.f);engine.processOfflineAudio(l,r,80,time);
    for(int i=5;i<80;++i)REQUIRE(std::abs(l[i]-sample(*find(original,a),rawA,950+i)-sample(*find(original,b),rawB,950+i))<1e-6);
    REQUIRE(engine.snapshot().offlineStatus.contains("accepted IRs") && !engine.snapshot().offlineStatus.contains("incomplete"));engine.endOffline();
    const auto next=root.file.getChildFile("next.wav");writeTestWav(next,1,1000,std::vector<float>(201,.002f));
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,next));waitFor([&]{return find(engine.savedSession(),a)->irName=="next.wav";});
    engine.generate();waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
    time=position(950,false);engine.processTransportAudio(l,r,16,time);waitFor([&]{return engine.snapshot().liveReady && engine.snapshot().retainedIRName.isEmpty();});
    const auto replacement=engine.savedSession();WetRender newRaw;REQUIRE(assets.readRender(find(replacement,a)->render,previewMemoryBudget,newRaw,error));
    time=position(950,true);std::fill_n(l,80,0.f);std::fill_n(r,80,0.f);engine.processTransportAudio(l,r,16,time);
    REQUIRE(std::abs(l[15]-sample(*find(replacement,a),newRaw,965)-sample(*find(original,b),rawB,965))<1e-6);
    REQUIRE(find(replacement,a)->shape==find(original,a)->shape && find(replacement,a)->placement==find(original,a)->placement);
    {const auto saved=engine.savedSession();const auto neighbour=*find(saved,b);
        for(const auto& entry:bundledImpulses()){
            REQUIRE(engine.assignBundledIR(a,find(engine.savedSession(),a)->inputGeneration,entry.id));
            waitFor([&]{return find(engine.savedSession(),a)->irName==entry.name;});const auto assigned=*find(engine.savedSession(),a);
            REQUIRE(assigned.ir.content==entry.decodedHash && assigned.shape==find(saved,a)->shape && assigned.arrival==find(saved,a)->arrival);
            REQUIRE(find(engine.savedSession(),b)->ir==neighbour.ir && find(engine.savedSession(),b)->render==neighbour.render && engine.savedSession().defaultIR==saved.defaultIR);}
        const auto assigned=engine.savedSession();engine.restore(assigned);REQUIRE(find(engine.savedSession(),a)->ir==find(assigned,a)->ir);
        engine.restore(saved);waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});}
    {const auto saved=engine.savedSession();
        REQUIRE(engine.assignBundledIR(a,find(saved,a)->inputGeneration,"monumental"));waitFor([&]{return find(engine.savedSession(),a)->irName=="Monumental";});
        engine.generate();waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
        REQUIRE(find(engine.savedSession(),a)->render.start<0 && find(engine.savedSession(),a)->sourceStart==1000);
        REQUIRE(engine.edit(a,[](auto& w,auto&){w.shape.automaticStart=false;w.shape.startOffset=-2000;return true;}));
        waitFor([&]{return engine.snapshot().auditionReady;});const auto crop=engine.savedSession();
        const auto shorten=[&]{REQUIRE(engine.assignBundledIR(a,find(engine.savedSession(),a)->inputGeneration,"whisper"));
            waitFor([&]{return find(engine.savedSession(),a)->irName=="Whisper";});engine.generate();
            waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::error;});
            REQUIRE(find(engine.savedSession(),a)->shape.startOffset==-2000 && engine.snapshot().retainedIRName=="Monumental");};
        shorten();REQUIRE(engine.resetSwell(a,false));waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
        REQUIRE(find(engine.savedSession(),a)->shape.automaticStart && find(engine.savedSession(),a)->render.irContent==find(engine.savedSession(),a)->ir.content);
        engine.restore(crop);waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});shorten();
        REQUIRE(engine.edit(a,[](auto& w,auto& error){return fullShape(w,w.sourceStart-499,renderInputEnd(w),error);}));
        waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
        REQUIRE(find(engine.savedSession(),a)->render.start==501 && find(engine.savedSession(),a)->shape.startOffset==-499);
        engine.restore(saved);waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});}
    // A decoded but computationally oversized request cannot consume the old voice.
    const auto huge=root.file.getChildFile("oversized.wav");writeTestWav(huge,1,10,std::vector<float>(200000,.001f));
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,huge));waitFor([&]{return find(engine.savedSession(),a)->irName=="oversized.wav";});
    engine.generate();waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::error;});
    REQUIRE(find(engine.savedSession(),a)->status.containsIgnoreCase("budget") && engine.snapshot().retainedIRName=="next.wav");
    REQUIRE(engine.residentBytes()<previewMemoryBudget);
    engine.restore(replacement);waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
    for(int kind=0;kind<4;++kind){
        REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::error;});
        REQUIRE(engine.snapshot().retainedIRName=="next.wav");
        REQUIRE(engine.edit(a,[&](auto& w,auto&){if(kind==0)w.enabled=false;if(kind==1)w.muted=true;if(kind==2)w.solo=true;if(kind==3)w.placement=5;return true;}));
        REQUIRE(engine.snapshot().retainedIRName.isEmpty());
        REQUIRE(engine.edit(a,[](auto& w,auto&){w.enabled=true;w.muted=false;w.solo=false;w.placement=0;return true;}));
        engine.generate();waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});}
    const auto beforeDelete=engine.savedSession();
    REQUIRE(engine.assignIR(a,find(beforeDelete,a)->inputGeneration,invalid));waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::error;});
    engine.removeSelected();REQUIRE(!find(engine.savedSession(),a) && engine.snapshot().retainedIRName.isEmpty());
    engine.restore(beforeDelete);waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::error;});
    REQUIRE(engine.edit(a,[](auto& w,auto&){w.shape.gainDb=-3;return true;}));REQUIRE(engine.snapshot().retainedIRName.isEmpty());
    engine.generate();waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::error;});
    float changed[]{.25f,.25f};REQUIRE(capture.commit(1050,1000,changed,1));{const juce::ScopedLock guard(mutex);context.capture=capture.snapshot();}
    waitFor([&]{return engine.snapshot().retainedIRName.isEmpty();});
    engine.generate();waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
    changed[0]=changed[1]=.3f;REQUIRE(capture.commit(1050,1000,changed,1));{const juce::ScopedLock guard(mutex);context.capture=capture.snapshot();}
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));
    REQUIRE(engine.snapshot().retainedIRName.isEmpty()); // Never relabel an old render as matching a fresh source commit.
    waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::error;});engine.generate();
    waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::error;});
    REQUIRE(assets.directory().getChildFile(find(engine.savedSession(),a)->ir.path).deleteFile());
    time=position(950,false);engine.processTransportAudio(l,r,16,time);waitFor([&]{return engine.snapshot().retainedIRName.isEmpty();});
    engine.restore(original);waitFor([&]{return find(engine.savedSession(),b)->phase==Phase::ready;});REQUIRE(engine.snapshot().retainedIRName.isEmpty());
    REQUIRE(engine.selectTargets({b},b));REQUIRE(engine.setFrozen(b,true));REQUIRE(!engine.assignIR(b,find(engine.savedSession(),b)->inputGeneration,next));
    REQUIRE(engine.snapshot().retainedIRName.isEmpty() && engine.residentBytes()<previewMemoryBudget);
});}
