#include "AudioTestFiles.h"
#include "AllocationGuard.h"
#include <DSP/SwellEngine.h>
#include <source_location>
#include <wavpack.h>
#include <bit>
using namespace m3;
static void writePackedIR(const juce::File& file)
{
    std::vector<char> bytes;const auto output=[](void* destination,void* data,int32_t count)->int
    {auto& sink=*static_cast<std::vector<char>*>(destination);auto begin=static_cast<char*>(data);sink.insert(sink.end(),begin,begin+count);return 1;};
    std::unique_ptr<WavpackContext,decltype(&WavpackCloseFile)> writer(WavpackOpenFileOutput(output,&bytes,nullptr),&WavpackCloseFile);REQUIRE(writer);
    WavpackConfig config{};config.sample_rate=48000;config.num_channels=2;config.channel_mask=3;config.bits_per_sample=32;config.bytes_per_sample=4;config.float_norm_exp=127;
    REQUIRE(WavpackSetConfiguration64(writer.get(),&config,4800,nullptr) && WavpackPackInit(writer.get()));
    std::vector<int32_t> samples(9600,0);samples[0]=samples[1]=std::bit_cast<int32_t>(.125f);
    REQUIRE(WavpackPackSamples(writer.get(),samples.data(),4800) && WavpackFlushSamples(writer.get()));
    REQUIRE(file.replaceWithData(bytes.data(),bytes.size()));
}
template<class F> static void waitFor(F condition,const std::source_location location=std::source_location::current())
{
    const auto deadline=juce::Time::getMillisecondCounter()+30000;
    while(!condition()) { if(juce::Time::getMillisecondCounter()>deadline) throw std::runtime_error("Timeout at "+std::to_string(location.line())); juce::Thread::sleep(2); }
}
int main() { return runTest([]
{
    {
        TestDirectory directory;PreviewContext empty;
        SwellEngine editor(directory.file,[&]{return empty;});
        Session base;base.automaticProcessing=false;base.swells.emplace_back();
        auto& automatic=base.swells.front();automatic.provenance=Provenance::automatic;
        automatic.arrival=automatic.sourceStart=automatic.detectorAnchor=100;automatic.sourceEnd=500;
        const auto id=automatic.id;
        for(int change=0;change<9;++change)
        {
            editor.restore(base);
            REQUIRE(editor.edit(id,[&](auto& w,auto&)
            {
                if(change==0)w.name="User name";
                if(change==1)w.enabled=false;
                if(change==2)w.muted=true;
                if(change==3)w.solo=true;
                if(change==4)w.shape.gainDb=-6;
                if(change==5)w.placement=50;
                if(change==6)w.sourceEnd=450;
                if(change==7)w.arrival=101;
                if(change==8)w.irName="User IR label";
                return true;
            }));
            const auto saved=editor.savedSession();
            REQUIRE(saved.swells.front().id==id && saved.swells.front().detectorAnchor==100);
            // An IR label alone is metadata; all actual musical/activity/name edits protect origin.
            REQUIRE(saved.swells.front().provenance==(change==8?Provenance::automatic:Provenance::manualEdited));
        }
        editor.restore(base);REQUIRE(editor.assignBundledIR(id,0,"monumental",editor.assignmentEpoch()));
        REQUIRE(editor.savedSession().swells.front().provenance==Provenance::manualEdited);
    }
    TestDirectory root; CaptureSession capture(root.file); juce::String error;
    std::vector<float> source(6000,0); source[0]=1; source[1]=-0.5f; source[400]=0.8f; source[1000]=0.5f;
    REQUIRE(capture.commit(0,1000,source.data(),3000));
    juce::CriticalSection contextLock; PreviewContext context{capture.snapshot(),{true,0,0,1000,240,4,4},0};
    SwellEngine engine(root.file,[&]{const juce::ScopedLock guard(contextLock);return context;});
    engine.setAutomaticProcessing(false);
    const auto update=[&]{const juce::ScopedLock guard(contextLock);context.capture=capture.snapshot();};
    REQUIRE(engine.addTarget(0)); const auto first=engine.savedSession().primary;
    REQUIRE(engine.addTarget(500)); const auto second=engine.savedSession().primary;
    REQUIRE(engine.selectTargets({first,second},first));
    writeTestWav(root.file.getChildFile("ir.wav"),1,1000,{1,0.5f,0.25f});
    engine.loadIR(root.file.getChildFile("ir.wav"));
    waitFor([&]{auto s=engine.savedSession();return s.swells[0].irName=="ir.wav" && s.swells[1].irName=="ir.wav";});
    engine.generate(true); waitFor([&]{auto s=engine.snapshot();return s.auditionReady && s.session.swells[1].phase==Phase::ready;});
    const auto initial=engine.snapshot(); REQUIRE(initial.renderCount==2 && initial.raw && initial.shaped);
    REQUIRE(engine.edit(first,[&](auto& w,auto& text){return fullShape(w,initial.raw->start,initial.raw->end,text);}));
    waitFor([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.audition());
    float checkLeft[64]{},checkRight[64]{};engine.processAudio(checkLeft,checkRight,64,context.time);REQUIRE(engine.snapshot().playing);
    engine.recaptureCompleted(0,250);std::fill_n(checkLeft,64,0.0f);std::fill_n(checkRight,64,0.0f);
    {AudioAllocationGuard guard;engine.processAudio(checkLeft,checkRight,64,context.time);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && std::all_of(checkLeft,checkLeft+64,[](float v){return v==0;}));
    REQUIRE(engine.edit(first,[](auto& w,auto&){resetShape(w);return true;}));waitFor([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.snapshot().renderCount==2);
    REQUIRE(engine.audition()); float left[64]{},right[64]{};
    {AudioAllocationGuard guard;engine.processAudio(left,right,64,context.time);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    REQUIRE(left[0]==0 && left[2]==0 && std::abs(left[1]-.5f)<1e-5);
    REQUIRE(engine.edit(first,[](auto& w,auto&){w.shape.gainDb=6;return true;}));
    waitFor([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.snapshot().renderCount==2 && engine.snapshot().raw->peak==initial.raw->peak);
    REQUIRE(engine.snapshot().shaped->peak>initial.shaped->peak);
    REQUIRE(engine.edit(second,[](auto& w,auto&){w.solo=true;return true;})); REQUIRE(!engine.audition());
    REQUIRE(engine.edit(second,[](auto& w,auto&){w.solo=false;return true;})); waitFor([&]{return engine.snapshot().auditionReady;}); REQUIRE(engine.audition());
    auto changed=context.time; changed.bpm=241;
    std::fill_n(left,64,0); engine.processAudio(left,right,64,changed); REQUIRE(std::all_of(left,left+64,[](float x){return x==0;}));
    waitFor([&]{return engine.savedSession().swells[0].phase==Phase::stale;});
    REQUIRE(engine.selectTargets({first},first)); engine.generate(); waitFor([&]{return engine.snapshot().auditionReady;});
    const auto count=engine.snapshot().renderCount;
    REQUIRE(capture.commit(0,1000,source.data(),250)); update(); juce::Thread::sleep(100);
    waitFor([&]{return engine.snapshot().auditionReady;}); REQUIRE(engine.snapshot().renderCount==count);
    float replacement[]{0.25f,-0.25f}; REQUIRE(capture.commit(0,1000,replacement,1)); update();
    waitFor([&]{return engine.savedSession().swells[0].phase==Phase::stale;});
    engine.setPreferences(-24,0.7f,0.1,true);
    replacement[0]=0.3f; REQUIRE(capture.commit(0,1000,replacement,1)); update();
    waitFor([&]{return engine.snapshot().auditionReady && engine.snapshot().renderCount==count+1;});
    // A bad batch member cannot prevent independent valid results.
    REQUIRE(engine.edit(second,[](auto& w,auto&){w.sourceEnd=4000;return true;}));
    engine.generate(true); waitFor([&]{return engine.savedSession().swells[1].phase==Phase::error;});
    waitFor([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.edit(second,[](auto& w,auto&){w.sourceEnd=750;return true;}));
    engine.generate(true);waitFor([&]{return engine.savedSession().swells[1].phase==Phase::ready;});
    const auto saved=engine.savedSession(); const auto savedRender=saved.swells[0].render;
    engine.restore(saved); waitFor([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.savedSession().auditionDb==-24 && !engine.snapshot().playing);
    REQUIRE(engine.savedSession().swells[0].render.path==savedRender.path);
    const auto cache=root.file.getChildFile(context.capture.manifest.projectId).getChildFile(context.capture.manifest.instanceId);
    REQUIRE(cache.getChildFile(savedRender.path).deleteFile());
    engine.restore(saved); waitFor([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.savedSession().swells[0].render.path!=savedRender.path);
    const auto ids=engine.savedSession(); engine.analyze(0,1000);
    waitFor([&]{return !engine.snapshot().analyzing;}); REQUIRE(!engine.snapshot().candidates.empty());
    engine.analyze(0,1000); engine.analyze(1000,2000);
    waitFor([&]{return !engine.snapshot().analyzing;}); REQUIRE(engine.savedSession().swells.size()==ids.swells.size());
    REQUIRE(engine.snapshot().candidates.empty());
    REQUIRE(engine.savedSession().swells[0].id==first && engine.savedSession().swells[0].shape.gainDb==6);
    engine.generate(); REQUIRE(engine.edit(first,[](auto& w,auto&){++w.sourceEnd;return true;}));
    juce::Thread::sleep(100); REQUIRE(engine.savedSession().swells[0].phase!=Phase::ready);
    engine.generate(); engine.removeSelected(); juce::Thread::sleep(100); REQUIRE(!find(engine.savedSession(),first));
    REQUIRE(engine.residentBytes()<previewMemoryBudget);
    REQUIRE(engine.selectTargets({second},second));
    REQUIRE(engine.edit(second,[](auto& w,auto&){w.sourceEnd=40000000;return true;}));
    engine.generate();waitFor([&]{return engine.savedSession().swells[0].phase==Phase::error;});
    REQUIRE(engine.savedSession().swells[0].status.contains("MiB"));
    engine.generate(); engine.restore({}); juce::Thread::sleep(100); REQUIRE(engine.savedSession().swells.empty());
    // More stored render audio than the RAM budget must not hydrate every full buffer.
    SwellAssetStore assets(cache); ImpulseResponse longIR;longIR.rate=1000;longIR.channels=1;longIR.samples.assign(400000,0);longIR.samples[0]=1;
    Asset longRef; REQUIRE(assets.writeIR(longIR,longRef,error));
    WetRender longWet;longWet.rate=1000;longWet.start=500-399999;longWet.sourceStart=500;longWet.sourceEnd=750;
    longWet.samples.assign(400249*2,0);longWet.samples[399999*2]=0.5f;
    CaptureAssetStore sourceStore(cache);juce::String dependency;
    REQUIRE(sourceFingerprint(sourceStore,context.capture.manifest,500,750,dependency,error));
    Asset longRender;REQUIRE(assets.writeRender(longWet,longRef,dependency,context.time,longRender,error));
    Session collection;collection.timing=context.time;
    for(int i=0;i<90;++i)
    { Swell w;w.name="Stored "+juce::String(i);w.arrival=w.sourceStart=500;w.sourceEnd=750;w.ir=longRef;w.render=longRender;collection.swells.push_back(w); }
    auto independent=saved.swells[1];independent.id=juce::Uuid().toDashedString();collection.swells.push_back(independent);
    collection.primary=collection.swells[0].id;collection.selected={collection.primary};
    REQUIRE(90ULL*static_cast<uint64_t>(longRender.frames)*8>previewMemoryBudget);
    engine.restore(collection);
    waitFor([&]{const auto s=engine.savedSession();return std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==Phase::ready;});});
    REQUIRE(engine.residentBytes()<32ULL*1024*1024);
    REQUIRE(cache.getChildFile(longRef.path).deleteFile());engine.restore(collection);
    waitFor([&]{const auto s=engine.savedSession();return s.swells[0].phase==Phase::error && s.swells.back().phase==Phase::ready;});
    REQUIRE(engine.savedSession().swells.size()==91 && !engine.snapshot().playing);
    {
        TestDirectory editsRoot;CaptureSession editsCapture(editsRoot.file);
        std::vector<float> signal(4000,0);signal[0]=signal[1000]=.25f;
        REQUIRE(editsCapture.commit(0,1000,signal.data(),2000));
        PreviewContext editsContext{editsCapture.snapshot(),{true,0,0,1000,120,4,4},0};
        SwellEngine edits(editsRoot.file,[&]{return editsContext;});edits.setAutomaticProcessing(false);
        waitFor([&]{return edits.savedSession().defaultIR.present();});
        REQUIRE(edits.addTarget(0));const auto a=edits.savedSession().primary;
        REQUIRE(edits.addTarget(500));const auto b=edits.savedSession().primary;
        const auto custom=editsRoot.file.getChildFile("own.wav");writeTestWav(custom,1,1000,{1,.5f,0});
        const auto token=find(edits.savedSession(),a)->inputGeneration;
        REQUIRE(edits.assignIR(a,token,custom));REQUIRE(edits.selectTargets({b},b));
        waitFor([&]{return find(edits.savedSession(),a)->irName=="own.wav";});
        REQUIRE(find(edits.savedSession(),b)->irName=="Preverb Hall");
        REQUIRE(!edits.assignIR(a,token,custom));
        edits.loadDefaultIR(custom);waitFor([&]{return edits.savedSession().defaultIRName=="own.wav";});
        const auto future=edits.savedSession().defaultIR;
        auto imported=*find(edits.savedSession(),a);REQUIRE(!edits.assignIR(a,imported.inputGeneration,{}));
        REQUIRE(find(edits.savedSession(),a)->inputGeneration==imported.inputGeneration);
        const auto packed=editsRoot.file.getChildFile("own.wv");writePackedIR(packed);
        REQUIRE(edits.assignIR(a,imported.inputGeneration,packed));waitFor([&]{return find(edits.savedSession(),a)->irName=="own.wv";});
        REQUIRE(find(edits.savedSession(),a)->ir.channels==2 && edits.savedSession().defaultIR==future && find(edits.savedSession(),b)->irName=="Preverb Hall");
        const auto bad=editsRoot.file.getChildFile("invalid.wav");REQUIRE(bad.replaceWithText("invalid IR fixture"));imported=*find(edits.savedSession(),a);
        REQUIRE(edits.assignIR(a,imported.inputGeneration,bad));waitFor([&]{return find(edits.savedSession(),a)->phase==Phase::error;});
        REQUIRE(find(edits.savedSession(),a)->ir==imported.ir && edits.savedSession().defaultIR==future);
        const auto oldEpoch=edits.assignmentEpoch();const auto restored=edits.savedSession();edits.restore(restored);
        REQUIRE(edits.snapshot().sessionEpoch!=oldEpoch);
        REQUIRE(!edits.selectTargets({b},b,oldEpoch));
        REQUIRE(!edits.removeTarget(a,oldEpoch));
        REQUIRE(!edits.edit(a,[](auto& w,auto&){w.name="Stale row command";return true;},oldEpoch));
        REQUIRE(edits.savedSession().selected==restored.selected && edits.savedSession().primary==restored.primary);
        REQUIRE(find(edits.savedSession(),a)->name==find(restored,a)->name && edits.savedSession().swells.size()==restored.swells.size());
        REQUIRE(!edits.assignIR(a,find(restored,a)->inputGeneration,custom,false,oldEpoch));
        waitFor([&]{return find(edits.savedSession(),a)->phase==Phase::ready;});
        auto request=*find(edits.savedSession(),a);
        REQUIRE(edits.assignBundledIR(a,request.inputGeneration,"preverb-hall",edits.assignmentEpoch()));
        waitFor([&]{return find(edits.savedSession(),a)->irName=="Preverb Hall";});
        REQUIRE(edits.savedSession().defaultIR==future && find(edits.savedSession(),a)->ir!=future);
        edits.generate(true);waitFor([&]{const auto state=edits.savedSession();return state.swells[0].phase==Phase::ready && state.swells[1].phase==Phase::ready;});
        const auto calibrationTarget=b;
        REQUIRE(edits.assignIR(b,find(edits.savedSession(),b)->inputGeneration,custom));
        waitFor([&]{return find(edits.savedSession(),b)->irName=="own.wav";});edits.generate(true);
        waitFor([&]{return find(edits.savedSession(),b)->phase==Phase::ready;});
        const auto rendersBeforeAdoption=edits.snapshot().renderCount;
        REQUIRE(edits.setGainPolicy(GainPolicy::musical));
        waitFor([&]{const auto state=edits.savedSession();const auto* w=find(state,calibrationTarget);return w && w->phase!=Phase::checking && w->phase!=Phase::rendering;});
        const auto afterAdoption=edits.savedSession();const auto* calibrated=find(afterAdoption,calibrationTarget);
        if(calibrated->calibration.state==CalibrationState::unavailable)throw std::runtime_error(calibrated->status.toStdString());
        const auto adopted=*calibrated;REQUIRE(edits.snapshot().renderCount==rendersBeforeAdoption);
        REQUIRE(edits.addTarget(550));
        waitFor([&]{const auto state=edits.savedSession();const auto* w=find(state,b);return w->phase==Phase::ready && w->calibration.referenceEnd==550;});
        REQUIRE(edits.snapshot().renderCount==rendersBeforeAdoption);edits.removeSelected();REQUIRE(edits.selectTargets({b},b));
        waitFor([&]{const auto state=edits.savedSession();const auto* w=find(state,b);return w->phase==Phase::ready && w->calibration.referenceEnd==adopted.calibration.referenceEnd;});
        REQUIRE(find(edits.savedSession(),b)->calibration==adopted.calibration);
        REQUIRE(edits.edit(calibrationTarget,[](auto& w,auto&){w.shape.gainDb=-3;w.shape.inTension=.5;w.placement=7;return true;}));
        if(const auto actual=find(edits.savedSession(),calibrationTarget)->calibration;actual!=adopted.calibration)
            throw std::runtime_error(("Calibration changed: "+juce::String(actual.baselineDb)+"/"+actual.renderIdentity+" vs "+juce::String(adopted.calibration.baselineDb)+"/"+adopted.calibration.renderIdentity).toStdString());
        REQUIRE(edits.resetSwell(calibrationTarget,false));waitFor([&]{return find(edits.savedSession(),calibrationTarget)->phase==Phase::ready;});
        if(const auto actual=find(edits.savedSession(),calibrationTarget)->calibration;actual!=adopted.calibration)
            throw std::runtime_error(("Reset calibration changed: "+juce::String(actual.baselineDb)+"/"+actual.renderIdentity+" vs "+juce::String(adopted.calibration.baselineDb)+"/"+adopted.calibration.renderIdentity).toStdString());
        edits.setLivePlayback(true,LiveRouting::dryWet,4);REQUIRE(!edits.setGainPolicy(GainPolicy::legacy));
        REQUIRE(edits.savedSession().gainPolicy==GainPolicy::musical && edits.savedSession().liveWetDb==4);
        edits.setLivePlayback(true,LiveRouting::dryWet,0);REQUIRE(edits.setGainPolicy(GainPolicy::legacy));
        REQUIRE(edits.setGainPolicy(GainPolicy::musical));waitFor([&]{const auto state=edits.savedSession();return std::all_of(state.swells.begin(),state.swells.end(),[](const auto& w){return w.phase!=Phase::checking && w.phase!=Phase::rendering;});});
        REQUIRE(find(edits.savedSession(),calibrationTarget)->calibration==adopted.calibration);
        REQUIRE(edits.setFrozen(calibrationTarget,true));const auto energySaved=edits.savedSession();
        REQUIRE(find(energySaved,calibrationTarget)->provenance==Provenance::manualEdited);
        juce::String energyJson,energyError;Session energyDecoded;
        REQUIRE(encode(energySaved,energyJson,energyError) && decode(energyJson,energyDecoded,energyError));
        edits.restore(energyDecoded);waitFor([&]{return find(edits.savedSession(),calibrationTarget)->phase==Phase::ready;});
        REQUIRE(find(edits.savedSession(),calibrationTarget)->calibration==adopted.calibration);
        REQUIRE(edits.setFrozen(calibrationTarget,false));waitFor([&]{return find(edits.savedSession(),calibrationTarget)->phase==Phase::ready;});
        REQUIRE(find(edits.savedSession(),calibrationTarget)->provenance==Provenance::manualEdited);
        REQUIRE(edits.setFrozen(a,true));request=*find(edits.savedSession(),a);
        REQUIRE(!edits.assignIR(a,request.inputGeneration,custom) && find(edits.savedSession(),a)->ir==request.ir);
        REQUIRE(edits.edit(a,[](auto& w,auto&){w.shape.gainDb=-8;w.shape.fadeIn=5;w.placement=10;w.enabled=false;w.solo=true;w.name="Keep name";return true;}));
        REQUIRE(edits.resetSwell(a,false));request=*find(edits.savedSession(),a);
        REQUIRE(request.frozen && !request.enabled && request.solo && request.shape==initialShape(1000) && request.placement==0 && request.name=="Keep name");
        const auto neighbour=*find(edits.savedSession(),b);const auto preferences=edits.savedSession();
        REQUIRE(edits.resetSwell(a,true));waitFor([&]{return !find(edits.savedSession(),a)->frozen;});
        request=*find(edits.savedSession(),a);
        REQUIRE(request.id==a && request.name=="Keep name" && request.arrival==0 && request.enabled && !request.solo && !request.muted);
        REQUIRE(request.shape==initialShape(1000) && request.ir==preferences.defaultIR && request.sourceEnd==500);
        REQUIRE(find(edits.savedSession(),b)->shape==neighbour.shape && find(edits.savedSession(),b)->ir==neighbour.ir);
        REQUIRE(edits.savedSession().primary==b && edits.savedSession().sensitivity==preferences.sensitivity);
        waitFor([&]{return find(edits.savedSession(),a)->phase==Phase::ready;});
        REQUIRE(edits.edit(a,[](auto& w,auto&){w.shape.gainDb=-7;w.enabled=false;return true;}));
        const auto defaultFile=edits.assetDirectory().getChildFile(edits.savedSession().defaultIR.path);REQUIRE(defaultFile.deleteFile());
        const auto prior=*find(edits.savedSession(),a);REQUIRE(edits.resetSwell(a,true));
        waitFor([&]{return edits.snapshot().message.startsWith("Reset all:");});
        request=*find(edits.savedSession(),a);REQUIRE(request.shape==prior.shape && request.ir==prior.ir && !request.enabled && request.inputGeneration==prior.inputGeneration);
        REQUIRE(!edits.assignBundledIR(a,request.inputGeneration,"unknown",edits.assignmentEpoch()));
        REQUIRE(find(edits.savedSession(),a)->inputGeneration==request.inputGeneration);
        REQUIRE(edits.assignIR(a,request.inputGeneration,custom));edits.removeSelected(); // b was selected; assignment remains pinned to a.
        waitFor([&]{return find(edits.savedSession(),a)->irName=="own.wav";});
        REQUIRE(edits.selectTargets({a},a));request=*find(edits.savedSession(),a);
        REQUIRE(edits.assignIR(a,request.inputGeneration,{},true));edits.removeSelected();
        juce::Thread::sleep(30);REQUIRE(edits.savedSession().swells.empty());
    }
}); }
