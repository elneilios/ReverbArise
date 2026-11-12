#include "AudioTestFiles.h"
#include "AllocationGuard.h"
#include <DSP/SwellEngine.h>
#include <DSP/DefaultImpulse.h>
#include <DSP/SongMix.h>
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
    const auto failed=[&]
    {
        const auto state=engine.snapshot();
        const auto request=state.irRequests.find(a);
        return request!=state.irRequests.end() && request->second.error.isNotEmpty();
    };
    const auto original=engine.savedSession();SwellAssetStore assets(engine.assetDirectory());WetRender rawA,rawB;juce::String error;
    REQUIRE(assets.readRender(find(original,a)->render,previewMemoryBudget,rawA,error));REQUIRE(assets.readRender(find(original,b)->render,previewMemoryBudget,rawB,error));
    const auto position=[&](int64_t p,bool playing){const juce::ScopedLock guard(mutex);context.time.sample=p;context.time.ppq=(p-(playing?16:0))/250.;return TransportTime{context.time,playing};};
    float l[80]{},r[80]{};auto time=position(950,false);engine.processTransportAudio(l,r,16,time);waitFor([&]{return engine.snapshot().liveReady;});
    const auto resumePlaying=[&]{const auto resumed=position(950,true);std::fill_n(l,80,0.f);std::fill_n(r,80,0.f);
        engine.processTransportAudio(l,r,16,resumed);REQUIRE(engine.transportClockSnapshot() && engine.transportClockSnapshot()->gate==LiveGate::admitted);};
    time=position(950,true);engine.processTransportAudio(l,r,16,time);
    REQUIRE(std::abs(l[15]-sample(*find(original,a),rawA,965)-sample(*find(original,b),rawB,965))<1e-6);
    const auto invalid=root.file.getChildFile("invalid.wav");REQUIRE(invalid.replaceWithText("not audio"));
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));waitFor(failed);
    REQUIRE(engine.snapshot().retainedIRName=="accepted.wav" && engine.snapshot().requestedIRName=="invalid.wav");
    waitFor([&]{return engine.snapshot().liveReady;});time=position(966,true);std::fill_n(l,80,0.f);std::fill_n(r,80,0.f);
    {AudioAllocationGuard guard;engine.processTransportAudio(l,r,16,time);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    for(int i=0;i<16;++i)REQUIRE(std::abs(l[i]-sample(*find(original,a),rawA,966+i)-sample(*find(original,b),rawB,966+i))<1e-6);
    REQUIRE(engine.assignBundledIR(a,find(engine.savedSession(),a)->inputGeneration,"monumental"));
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));waitFor(failed);
    REQUIRE(engine.snapshot().retainedIRName=="accepted.wav" && find(engine.savedSession(),b)->render==find(original,b)->render);
    // A failed row still contributes its accepted wet audio to the mix.
    // Generate a hot, overlapping neighbour while that fallback is audible.
    REQUIRE(engine.addTarget(1000));const auto c=engine.savedSession().primary;
    const auto hotFile=root.file.getChildFile("hot.wav");
    writeTestWav(hotFile,1,1000,std::vector<float>(201,.125f));
    engine.loadIR(hotFile);waitFor([&]{return find(engine.savedSession(),c)->irName=="hot.wav";});
    engine.generate();waitFor([&]{return find(engine.savedSession(),c)->phase==Phase::ready;});
    const auto withNeighbour=engine.savedSession();
    const auto& neighbour=*find(withNeighbour,c);
    REQUIRE(failed() && neighbour.generatedGain && *neighbour.generatedGain<1);
    WetRender rawC;REQUIRE(assets.readRender(neighbour.render,previewMemoryBudget,rawC,error));
    float combinedPeak=0;
    for (int64_t at=800;at<1150;++at)
        combinedPeak=std::max(combinedPeak,std::abs(sample(*find(original,a),rawA,at)+sample(*find(original,b),rawB,at)
            +sample(neighbour,rawC,at) * *neighbour.generatedGain));
    REQUIRE(combinedPeak>0.85f && combinedPeak<=juce::Decibels::decibelsToGain(candidateWetCeilingDb));
    engine.removeSelected();
    REQUIRE(engine.selectTargets({a},a) && engine.snapshot().retainedIRName=="accepted.wav");
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
        engine.restore(saved);waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
        // Restore retires the old clock. Model the next real host callback,
        // rather than relying on render completion to overwrite saved origin.
        REQUIRE(!engine.transportClockSnapshot());resumePlaying();}
    {const auto saved=engine.savedSession();
        REQUIRE(engine.assignBundledIR(a,find(saved,a)->inputGeneration,"monumental"));waitFor([&]{return find(engine.savedSession(),a)->irName=="Monumental";});
        engine.generate();waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
        REQUIRE(find(engine.savedSession(),a)->render.start<0 && find(engine.savedSession(),a)->sourceStart==1000);
        REQUIRE(engine.edit(a,[](auto& w,auto&){w.shape.automaticStart=false;w.shape.startOffset=-2000;return true;}));
        waitFor([&]{return engine.snapshot().auditionReady;});const auto crop=engine.savedSession();
        time=position(950,false);engine.processTransportAudio(l,r,16,time);
        engine.setMonitoring(AuditionMode::wetOnly,1);
        waitFor([&]{return engine.snapshot().auditionReady;});
        REQUIRE(engine.audition());
        engine.processAudio(l,r,16,time.song);
        REQUIRE(engine.snapshot().playing);
        engine.holdRenderPublicationForTesting(true);
        REQUIRE(engine.assignBundledIR(a,find(engine.savedSession(),a)->inputGeneration,"whisper"));
        waitFor([&]{return engine.renderPublicationWaitingForTesting();});
        const auto pending=engine.savedSession();
        REQUIRE(sameSavedSwell(*find(crop,a),*find(pending,a)) && engine.snapshot().irReplacementPending);
        engine.processAudio(l,r,16,time.song);
        REQUIRE(engine.snapshot().playing);
        engine.holdRenderPublicationForTesting(false);
        waitFor([&]{return find(engine.savedSession(),a)->irName=="Whisper";});
        const auto shorter=engine.savedSession();
        REQUIRE(find(shorter,a)->render.start==501 && find(shorter,a)->shape.startOffset==-499);
        REQUIRE(find(shorter,a)->arrival==find(crop,a)->arrival && sameSavedSwell(*find(shorter,b),*find(crop,b)));
        REQUIRE(find(shorter,a)->wetTimingIntent && find(shorter,a)->wetTimingIntent->startOffset==-2000);
        {AudioAllocationGuard guard;engine.processAudio(l,r,16,time.song);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && engine.snapshot().playing);
        engine.stop();
        engine.processAudio(l,r,16,time.song);
        const auto custom=root.file.getChildFile("short-custom.wav");
        writeTestWav(custom,1,1000,std::vector<float>(51,.002f));
        REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,custom));
        waitFor([&]{return find(engine.savedSession(),a)->irName==custom.getFileName();});
        REQUIRE(find(engine.savedSession(),a)->shape.startOffset==-50);
        REQUIRE(engine.assignIR(b,find(engine.savedSession(),b)->inputGeneration,custom));
        waitFor([&]{return find(engine.savedSession(),b)->irName==custom.getFileName();});
        REQUIRE(find(engine.savedSession(),b)->shape.automaticStart);
        const auto fittedSession=engine.savedSession();
        juce::String encoded;
        Session reopened;
        REQUIRE(encode(fittedSession,encoded,error) && decode(encoded,reopened,error));
        engine.restore(reopened);
        waitFor([&]{return engine.snapshot().auditionReady;});
        REQUIRE(sameSavedSwell(*find(engine.savedSession(),a),*find(fittedSession,a)));

        // Compare interior wet samples after reopening a fitted replacement.
        // Export has independent mix controls, so compare both at unity wet.
        {
            REQUIRE(reopened.liveWetDb==0);
            time=position(950,false);
            engine.processTransportAudio(l,r,16,time);
            waitFor([&]{return engine.snapshot().liveReady;});
            const auto source=capture.snapshot();
            std::shared_ptr<const SongExportPlan> plan;
            REQUIRE(prepareSongExportPlan(engine.savedSession(),source.manifest,source.reference,time.song,0,1,plan,error));
            SongMix mix(engine.assetDirectory());
            REQUIRE(mix.prepare(plan,previewMemoryBudget,error) && mix.measure(error));
            std::array<float,64> exported{};
            REQUIRE(mix.read(955,exported,error));
            REQUIRE(std::any_of(exported.begin(),exported.end(),[](float value){return std::abs(value)>1e-4f;}));
            std::fill_n(l,80,0.f);std::fill_n(r,80,0.f);
            time=position(950,true);
            engine.processTransportAudio(l,r,37,time);
            for(int i=0;i<32;++i)
            {
                REQUIRE(std::abs(l[i+5]-exported[2*i])<1e-6f);
                REQUIRE(std::abs(r[i+5]-exported[2*i+1])<1e-6f);
            }
            time=position(950,false);
            engine.processTransportAudio(l,r,16,time);
        }

        engine.setMonitoring(AuditionMode::dryWet,.5f);
        waitFor([&]{const auto state=engine.snapshot();return state.auditionReady && state.dryAvailable;});
        REQUIRE(engine.audition());
        engine.processAudio(l,r,16,time.song);
        REQUIRE(engine.snapshot().playing);
        REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));
        waitFor(failed);
        engine.processAudio(l,r,16,time.song);
        REQUIRE(engine.snapshot().playing && find(engine.savedSession(),a)->irName==custom.getFileName());
        REQUIRE(engine.assignBundledIR(a,find(engine.savedSession(),a)->inputGeneration,"monumental"));
        waitFor([&]{return find(engine.savedSession(),a)->irName=="Monumental";});
        REQUIRE(find(engine.savedSession(),a)->shape==find(crop,a)->shape);
        {AudioAllocationGuard guard;engine.processAudio(l,r,16,time.song);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && engine.snapshot().playing);
        waitFor([&]{const auto state=engine.snapshot();return state.auditionReady && state.dryAvailable;});
        const auto mixed=engine.snapshot();
        REQUIRE(mixed.playing);
        REQUIRE(std::find(mixed.auditionMembers.begin(),mixed.auditionMembers.end(),b)!=mixed.auditionMembers.end());
        engine.stop();
        engine.processAudio(l,r,16,time.song);
        REQUIRE(engine.edit(a,[](auto& w,auto&){w.sourceEnd=6000;return true;}));
        REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,custom));
        waitFor(failed);
        REQUIRE(engine.snapshot().irReplacementError.contains("Required source audio is missing"));
        REQUIRE(find(engine.savedSession(),a)->irName=="Monumental");
        engine.restore(saved);waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});resumePlaying();}
    // A decoded but computationally oversized request cannot consume the old voice.
    const auto huge=root.file.getChildFile("oversized.wav");writeTestWav(huge,1,10,std::vector<float>(200000,.001f));
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,huge));waitFor(failed);
    REQUIRE(engine.snapshot().irReplacementError.containsIgnoreCase("budget") && engine.snapshot().retainedIRName=="next.wav");
    REQUIRE(engine.residentBytes()<previewMemoryBudget);
    engine.restore(replacement);waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
    resumePlaying();
    for(int kind=0;kind<4;++kind){
        REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));waitFor(failed);
        REQUIRE(engine.snapshot().retainedIRName=="next.wav");
        REQUIRE(engine.edit(a,[&](auto& w,auto&){if(kind==0)w.enabled=false;if(kind==1)w.muted=true;if(kind==2)w.solo=true;if(kind==3)w.placement=5;return true;}));
        REQUIRE(engine.snapshot().retainedIRName.isEmpty());
        REQUIRE(engine.edit(a,[](auto& w,auto&){w.enabled=true;w.muted=false;w.solo=false;w.placement=0;return true;}));
        engine.generate();waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});}
    const auto beforeDelete=engine.savedSession();
    REQUIRE(engine.assignIR(a,find(beforeDelete,a)->inputGeneration,invalid));waitFor(failed);
    engine.removeSelected();REQUIRE(!find(engine.savedSession(),a) && engine.snapshot().retainedIRName.isEmpty());
    engine.restore(beforeDelete);waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
    resumePlaying();
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));waitFor(failed);
    REQUIRE(engine.edit(a,[](auto& w,auto&){w.shape.gainDb=-3;return true;}));REQUIRE(engine.snapshot().retainedIRName.isEmpty());
    engine.generate();waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));waitFor(failed);
    float changed[]{.25f,.25f};REQUIRE(capture.commit(1050,1000,changed,1));{const juce::ScopedLock guard(mutex);context.capture=capture.snapshot();}
    waitFor([&]{return engine.snapshot().retainedIRName.isEmpty();});
    engine.generate();waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
    changed[0]=changed[1]=.3f;REQUIRE(capture.commit(1050,1000,changed,1));{const juce::ScopedLock guard(mutex);context.capture=capture.snapshot();}
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));
    waitFor([&]{return !engine.snapshot().irReplacementPending;});
    engine.generate();
    waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,invalid));waitFor(failed);
    REQUIRE(assets.directory().getChildFile(find(engine.savedSession(),a)->ir.path).deleteFile());
    // A warm verified window need not reopen its assets just because Play
    // stops. Seek beyond that window to require fresh dependency validation.
    time=position(4500,false);engine.processTransportAudio(l,r,16,time);
    REQUIRE(engine.transportClockSnapshot()->gate==LiveGate::stopped);
    waitFor([&]{return engine.snapshot().retainedIRName.isEmpty();});
    engine.restore(original);waitFor([&]{return find(engine.savedSession(),b)->phase==Phase::ready;});REQUIRE(engine.snapshot().retainedIRName.isEmpty());
    REQUIRE(engine.selectTargets({b},b));REQUIRE(engine.setFrozen(b,true));REQUIRE(!engine.assignIR(b,find(engine.savedSession(),b)->inputGeneration,next));
    REQUIRE(engine.snapshot().retainedIRName.isEmpty() && engine.residentBytes()<previewMemoryBudget);

    const auto quiet=root.file.getChildFile("quiet.wav");
    writeTestWav(quiet,1,1000,{.00001f,.00001f});
    REQUIRE(engine.selectTargets({a},a));
    waitFor([&]{return find(engine.savedSession(),a)->phase==Phase::ready;});
    engine.setLivePlayback(false,LiveRouting::dryWet,0);
    engine.setLiveStatusForTesting("256 MiB live publication budget exceeded");
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,quiet));
    waitFor([&]{return find(engine.savedSession(),a)->irName==quiet.getFileName() || failed();});
    REQUIRE(find(engine.savedSession(),a)->irName==quiet.getFileName());

    // A quieter candidate can reach headroom in the same batch as a new
    // neighbour, then fail at publication because the IR collection is full.
    auto full=original;
    full.liveEnabled=false;
    full.defaultIR=neighbour.ir;
    full.defaultIRName=neighbour.irName;
    while(full.importedIRs.size()<maximumImportedIRs)
    {
        auto entry=full.importedIRs.front();
        entry.asset.content=juce::String::toHexString(static_cast<int>(full.importedIRs.size())).paddedLeft('0',64);
        full.importedIRs.push_back(std::move(entry));
    }
    engine.restore(full);
    waitFor([&]{const auto state=engine.savedSession();return find(state,a)->phase==Phase::ready && find(state,b)->phase==Phase::ready;});
    engine.holdRenderPublicationForTesting(true);
    REQUIRE(engine.assignIR(a,find(engine.savedSession(),a)->inputGeneration,quiet));
    waitFor([&]{return engine.renderPublicationWaitingForTesting();});
    REQUIRE(engine.addTarget(1000));
    const auto added=engine.savedSession().primary;
    REQUIRE(engine.generate().queued==1);
    engine.holdRenderPublicationForTesting(false);
    waitFor([&]{return failed() && find(engine.savedSession(),added)->phase==Phase::ready;});
    const auto accepted=engine.savedSession();
    REQUIRE(find(accepted,a)->ir==find(original,a)->ir);
    REQUIRE(engine.snapshot().irRequests.at(a).error.contains("collection is full"));
    REQUIRE(assets.readRender(find(accepted,a)->render,previewMemoryBudget,rawA,error));
    REQUIRE(assets.readRender(find(accepted,b)->render,previewMemoryBudget,rawB,error));
    REQUIRE(assets.readRender(find(accepted,added)->render,previewMemoryBudget,rawC,error));
    combinedPeak=0;
    for(int64_t at=800;at<1150;++at)
    {
        const auto contribution=[&](const juce::String& id,const WetRender& raw)
        {
            const auto& swell=*find(accepted,id);
            return sample(swell,raw,at)*swell.generatedGain.value_or(1.0f);
        };
        combinedPeak=std::max(combinedPeak,std::abs(contribution(a,rawA)+contribution(b,rawB)+contribution(added,rawC)));
    }
    REQUIRE(combinedPeak>.85f && combinedPeak<=juce::Decibels::decibelsToGain(candidateWetCeilingDb));
});}
