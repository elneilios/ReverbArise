#include "AudioTestFiles.h"
#include "AllocationGuard.h"
#include <DSP/SwellEngine.h>
#include <thread>
#include <barrier>
#include <source_location>
using namespace m3;
template<class F>static void waitFor(F test,const std::source_location line=std::source_location::current())
{const auto end=juce::Time::getMillisecondCounter()+30000;while(!test()){if(juce::Time::getMillisecondCounter()>end)throw std::runtime_error("Configuration timeout "+std::to_string(line.line()));juce::Thread::sleep(2);}}
int main(){return runTest([]
{
    TestDirectory root;CaptureSession capture(root.file);const PreviewContext context{capture.snapshot(),{true,0,0,1000,120,4,4},0};
    std::atomic<unsigned> changes=0;
    SwellEngine engine(root.file,[&]{return context;},[&]{++changes;});
    waitFor([&]{return engine.savedSession().defaultIR.present();});workflow::State flow;engine.useWorkflow(flow);
    REQUIRE(engine.setGainPolicy(GainPolicy::legacy));const auto compatibilityPass=flow.begin(true);
    REQUIRE(compatibilityPass && engine.passConfiguration(compatibilityPass) && engine.passConfiguration(compatibilityPass)->session==engine.assignmentEpoch());
    REQUIRE(!engine.setGainPolicy(GainPolicy::musical));
    REQUIRE(flow.stop(compatibilityPass) && flow.validating(compatibilityPass) && flow.complete(compatibilityPass,true,false,false));
    REQUIRE(engine.setGainPolicy(GainPolicy::musical));
    auto oldToken=flow.snapshot().pass;engine.setPreferences(0,.25f,.2,false,oldToken);
    const auto accepted=engine.savedSession();const auto before=changes.load();uint64_t pass;
    {AudioAllocationGuard guard;pass=flow.begin(true);}
    REQUIRE(pass && watchedAllocations==0 && watchedDeallocations==0);
    auto pinned=engine.passConfiguration(pass);REQUIRE(pinned && pinned->automatic && pinned->sensitivity==.25f && pinned->separationSeconds==.2 && pinned->ir==accepted.defaultIR);
    for(int phase=0;phase<3;++phase)
    {
        engine.setPreferences(0,.8f,.3,false);engine.setAutomaticProcessing(false);engine.setAutoMarkers(false);engine.setLearningAutomatic(false);
        REQUIRE(!engine.assignDefaultBundledIR("monumental"));
        REQUIRE(engine.savedSession().sensitivity==accepted.sensitivity && engine.savedSession().automaticProcessing && engine.savedSession().autoMarkersEnabled);
        REQUIRE(engine.passConfiguration(pass)->revision==pinned->revision && changes.load()==before);
        if(phase==0)REQUIRE(flow.stop(pass));if(phase==1)REQUIRE(flow.validating(pass));
    }
    REQUIRE(flow.complete(pass,true,false,false));REQUIRE(!engine.passConfiguration(pass));
    // Typed/knob/chooser completions retain their start token, even after unlock.
    engine.setPreferences(0,.8f,.3,false,oldToken);engine.setAutoMarkers(false,oldToken);engine.setAutomaticProcessing(false,oldToken);
    REQUIRE(!engine.assignDefaultBundledIR("monumental",engine.assignmentEpoch(),oldToken));
    REQUIRE(engine.savedSession().sensitivity==.25f && changes.load()==before);
    engine.setPreferences(0,.6f,.15,false,flow.snapshot().pass);REQUIRE(engine.savedSession().sensitivity==.6f && changes.load()==before+1);
    // In-flight import intent cannot become the accepted pass configuration.
    engine.holdDefaultImportsForTesting(true);REQUIRE(engine.assignDefaultBundledIR("monumental"));
    waitFor([&]{return engine.defaultImportWaitingForTesting();});pass=flow.begin(true);REQUIRE(pass);
    const auto priorIR=engine.passConfiguration(pass)->ir;engine.holdDefaultImportsForTesting(false);
    waitFor([&]{return !engine.snapshot().defaultIRPending;});
    REQUIRE(engine.savedSession().defaultIR==priorIR && engine.passConfiguration(pass)->ir==priorIR);
    REQUIRE(flow.stop(pass) && flow.validating(pass) && flow.complete(pass,true,false,false));
    // Effective Learn policy respects all legacy master/marker combinations.
    const auto original=engine.savedSession();
    for(int bits=0;bits<4;++bits)
    {
        flow.reset();auto legacy=original;legacy.automaticProcessing=(bits&1)!=0;legacy.autoMarkersEnabled=(bits&2)!=0;
        engine.restore(legacy);pass=flow.begin(true);REQUIRE(pass && engine.passConfiguration(pass)->automatic==(bits==3));
        REQUIRE(flow.stop(pass) && flow.validating(pass) && flow.complete(pass,true,false,false));
        engine.setLearningAutomatic(true);const auto enabled=engine.savedSession();
        REQUIRE(enabled.automaticProcessing && enabled.autoMarkersEnabled && enabled.defaultIR==legacy.defaultIR && enabled.sensitivity==legacy.sensitivity);
        engine.setLearningAutomatic(false);const auto off=engine.savedSession();
        REQUIRE(!off.automaticProcessing && off.autoMarkersEnabled && off.defaultIR==enabled.defaultIR && off.sensitivity==enabled.sensitivity);
        const auto notified=changes.load();engine.setLearningAutomatic(false);REQUIRE(changes.load()==notified);
        juce::String json,error;Session reopened;REQUIRE(encode(off,json,error) && decode(json,reopened,error));
        REQUIRE(!reopened.automaticProcessing && reopened.autoMarkersEnabled && reopened.defaultIR==off.defaultIR && reopened.sensitivity==off.sensitivity);
    }
    engine.restore(original);
    // A single racing edit must resolve as a whole pair, never mixed controls.
    for(int iteration=0;iteration<200;++iteration)
    {
        flow.reset();engine.setPreferences(0,.25f,.2,false);const auto token=flow.snapshot().pass;
        std::barrier start(2);std::thread edit([&]{start.arrive_and_wait();engine.setPreferences(0,.75f,.4,false,token);});
        start.arrive_and_wait();{AudioAllocationGuard guard;pass=flow.begin(true);}
        edit.join();REQUIRE(pass && watchedAllocations==0 && watchedDeallocations==0);
        const auto config=engine.passConfiguration(pass);const auto saved=engine.savedSession();
        REQUIRE(config && ((config->sensitivity==.25f && config->separationSeconds==.2) || (config->sensitivity==.75f && config->separationSeconds==.4)));
        REQUIRE(config->sensitivity==saved.sensitivity && config->separationSeconds==saved.separationSeconds);
        REQUIRE(flow.stop(pass) && flow.validating(pass) && flow.complete(pass,true,false,false));
    }
    // Configuration lifetime also survives saved-mode restore and source Clear.
    flow.reset(workflow::Mode::finalise);auto restored=engine.savedSession();restored.sensitivity=.3f;engine.restore(restored);
    REQUIRE(flow.requestMode(workflow::Mode::capture,{}));pass=flow.begin(true);REQUIRE(pass && engine.passConfiguration(pass)->sensitivity==.3f);
    REQUIRE(flow.stop(pass) && flow.validating(pass) && flow.complete(pass,true,false,false));
    engine.clear();pass=flow.begin(true);REQUIRE(pass && engine.passConfiguration(pass));
    {
        // Factory loading is asynchronous too: no accepted IR at Play remains
        // an honestly missing dependency for this pass, not a mutable default.
        std::atomic<bool> loading=true;workflow::State earlyFlow;
        SwellEngine early(root.file,[&]{auto c=context;if(loading.load())c.capture.status="Loading";return c;});early.useWorkflow(earlyFlow);
        const auto token=earlyFlow.begin(true);REQUIRE(token && !early.passConfiguration(token)->ir.present());loading=false;
        REQUIRE(earlyFlow.stop(token) && earlyFlow.validating(token));
        REQUIRE(!early.savedSession().defaultIR.present() && !early.passConfiguration(token)->ir.present());
        REQUIRE(earlyFlow.complete(token,true,false,false));waitFor([&]{return early.savedSession().defaultIR.present();});
        const auto next=earlyFlow.begin(true);REQUIRE(next && early.passConfiguration(next)->ir.present());
    }
});}
