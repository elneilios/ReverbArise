#pragma once
#include <Capture/CaptureWorker.h>
#include <UI/CaptureFailureText.h>
#include <limits>

struct CaptureFailureFixture
{
    TestDirectory root;
    std::unique_ptr<PluginCaptureQueue> queue=std::make_unique<PluginCaptureQueue>();
    workflow::State flow;
    CaptureWorker worker{*queue,root.file};
    std::vector<float> audio=std::vector<float>(5000,.25f);
    uint64_t pass=0;
    explicit CaptureFailureFixture(bool prior)
    {
        worker.useWorkflow(flow);worker.setHostRate(1000);worker.startThread();
        if(prior){begin();send(0);stop();}
    }
    ~CaptureFailureFixture(){worker.holdValidationForTesting(false);worker.holdWorkerForTesting(false);worker.stopThread(-1);}
    void begin(bool replace=false)
    {
        if(flow.snapshot().mode!=workflow::Mode::capture)REQUIRE(flow.requestMode(workflow::Mode::capture,{true,true,false}));
        REQUIRE(flow.arm(replace));pass=flow.begin(true);REQUIRE(pass);
    }
    void send(int64_t at,int frames=5000,double rate=1000)
    {REQUIRE(queue->tryPush(at,rate,audio.data(),audio.data(),frames,worker.captureGeneration(),pass));REQUIRE(worker.flush());}
    void stop(){REQUIRE(flow.stop(pass));REQUIRE(worker.flush());}
    void retry()
    {begin();send(90000);stop();REQUIRE(flow.snapshot().result==workflow::Result::accepted);}
};

inline void workerFailureCases()
{
    for(bool prior:{false,true})for(int kind=0;kind<7;++kind)
    {
        CaptureFailureFixture f(prior);const auto before=f.worker.snapshot();f.begin(prior);f.send(10000);f.send(15000);
        const auto directory=f.root.file.getChildFile(before.manifest.projectId).getChildFile(before.manifest.instanceId);
        juce::File blocked,backup;
        CaptureFailureReason expected=CaptureFailureReason::none;
        if(kind==0)
        {
            f.audio[0]=std::numeric_limits<float>::quiet_NaN();f.send(20000);
            f.audio[0]=.25f;expected=CaptureFailureReason::invalidData;
        }
        if(kind==1){f.send(20000,100,44100);expected=CaptureFailureReason::sampleRate;}
        if(kind==2)
        {
            blocked=directory.getChildFile("segments");backup=directory.getChildFile("held-segments");
            REQUIRE(blocked.moveFileTo(backup) && blocked.replaceWithText("blocked"));
            f.send(20000);expected=CaptureFailureReason::assetWrite;
        }
        if(kind==3)
        {
            blocked=directory.getChildFile("manifests");backup=directory.getChildFile("held-manifests");
            if(blocked.isDirectory())REQUIRE(blocked.moveFileTo(backup));
            REQUIRE(blocked.replaceWithText("blocked"));expected=CaptureFailureReason::publication;
        }
        if(kind==4)
        {f.worker.throwNextStageForTesting();f.send(20000);expected=CaptureFailureReason::workerException;}
        if(kind==5)
        {f.send(20000,100);f.worker.throwNextStageForTesting();expected=CaptureFailureReason::workerException;}
        if(kind==6)
        {
            bool corrupted=false;
            for(auto file:directory.getChildFile("segments").findChildFiles(juce::File::findFiles,false))
            {
                const auto protectedAsset=std::any_of(before.manifest.ranges.begin(),before.manifest.ranges.end(),[&](const auto& range){return directory.getChildFile(range.assetPath)==file;});
                if(!protectedAsset){REQUIRE(file.replaceWithText("corrupted staged asset"));corrupted=true;break;}
            }
            REQUIRE(corrupted);expected=CaptureFailureReason::assetValidation;
        }
        if(kind==0 || kind==1 || kind==2 || kind==4)
        {
            CaptureFailureEvent later;later.pass=f.pass;later.epoch=f.worker.captureGeneration();later.reason=CaptureFailureReason::queueFull;
            later.dropped=7;f.worker.failPass(later);REQUIRE(f.worker.flush());
        }
        f.stop();const auto rejected=f.worker.snapshot();
        REQUIRE(f.flow.snapshot().phase==workflow::Phase::failed && rejected.reference==before.reference);
        REQUIRE(rejected.failure && rejected.failure.event.reason==expected);
        REQUIRE(rejected.failure.event.pass==f.pass && rejected.failure.reference==before.reference);
        REQUIRE(rejected.failure.hadCommittedSource==prior && rejected.failure.detail.isNotEmpty());
        if(kind==0 || kind==1 || kind==2 || kind==4)REQUIRE(rejected.failure.finalDropped==7 && rejected.failure.event.dropped==0);
        REQUIRE(rejected.status.contains("retry") && f.worker.progressSnapshot().ranges.empty());
        const auto evidence=m3::ui::captureFailureText(rejected.failure,true);
        REQUIRE(evidence.contains("unavailable") && evidence.contains("Current failed pass"));
        if(blocked!=juce::File{}){REQUIRE(blocked.deleteFile());if(backup.exists())REQUIRE(backup.moveFileTo(blocked));}
        if(prior)
        {
            CaptureSession restored(f.root.file);REQUIRE(restored.restore(before.manifest.projectId,before.manifest.instanceId,before.reference,before.manifest.revision));
            float l=0,r=0;REQUIRE(restored.readSample(0,l,r) && l==.25f);
        }
        f.retry();REQUIRE(f.worker.snapshot().failure.event.pass==rejected.failure.event.pass);
        REQUIRE(m3::ui::captureFailureText(f.worker.snapshot().failure,false).contains("historical"));
        f.worker.requestRestore(before.manifest.projectId,before.manifest.instanceId,before.reference,before.manifest.revision);
        REQUIRE(!f.worker.snapshot().failure);REQUIRE(f.worker.flush());REQUIRE(!f.worker.snapshot().failure);
        std::cout<<"worker failure prior="<<prior<<" kind="<<kind<<" retry=passed\n";
    }
}

inline void captureFailureOwnershipCases()
{
    for(bool validation:{false,true})for(bool restore:{false,true})
    {
        CaptureFailureFixture f(true);const auto before=f.worker.snapshot();f.begin(true);f.send(0);
        if(validation)
        {f.worker.holdValidationForTesting(true);REQUIRE(f.flow.stop(f.pass));awaitCapture([&]{return f.flow.snapshot().phase==workflow::Phase::validating;});}
        else
        {
            f.worker.holdWorkerForTesting(true);awaitCapture([&]{return f.worker.workerHeldForTesting();});
            REQUIRE(f.queue->tryPush(5000,1000,f.audio.data(),f.audio.data(),5000,f.worker.captureGeneration(),f.pass));
        }
        const auto obsolete=f.pass;
        if(restore)f.worker.requestRestore(before.manifest.projectId,before.manifest.instanceId,before.reference,before.manifest.revision);
        else f.worker.cancelWorkflowPass();
        CaptureFailureEvent stale;stale.pass=obsolete;stale.epoch=0;stale.reason=CaptureFailureReason::queueFull;f.worker.failPass(stale);
        f.worker.holdValidationForTesting(false);f.worker.holdWorkerForTesting(false);REQUIRE(f.worker.flush());
        REQUIRE(f.worker.snapshot().reference==before.reference && !f.worker.snapshot().failure);
        f.retry();REQUIRE(!f.worker.snapshot().failure);
        std::cout<<"obsolete capture completion validation="<<validation<<" restore="<<restore<<" fenced; retry=passed\n";
    }
    CaptureFailureFixture f(true);const auto before=f.worker.snapshot();f.begin(true);f.send(0);
    f.worker.holdValidationForTesting(true);REQUIRE(f.flow.stop(f.pass));
    REQUIRE(!f.flow.stop(f.pass));awaitCapture([&]{return f.flow.snapshot().phase==workflow::Phase::validating;});
    CaptureFailureEvent event;event.pass=f.pass;event.epoch=f.worker.captureGeneration();event.reason=CaptureFailureReason::queueFull;f.worker.failPass(event);
    f.worker.holdValidationForTesting(false);REQUIRE(f.worker.flush());
    REQUIRE(f.flow.snapshot().phase==workflow::Phase::failed && f.worker.snapshot().reference==before.reference);
    REQUIRE(f.worker.snapshot().failure.event.pass==f.pass);f.retry();
    for(bool throwing:{false,true})
    {
        CaptureFailureFixture pin(false);
        pin.worker.setPassContextProvider([&](uint64_t)->std::optional<CaptureGenerationContext>
        {if(throwing)throw std::runtime_error(std::string(2048,'x'));return {};});
        pin.begin();pin.send(0);pin.stop();const auto failed=pin.worker.snapshot();
        REQUIRE(failed.failure.event.reason==(throwing?CaptureFailureReason::workerException:CaptureFailureReason::context));
        REQUIRE(failed.failure.detail.length()<=512);
        if(throwing)REQUIRE(failed.failure.detail.length()==512);
        pin.worker.setPassContextProvider({});pin.retry();
    }
}

inline void captureDiagnosticCapacityCase()
{
    CaptureFailureFixture f(false);f.worker.holdWorkerForTesting(true);awaitCapture([&]{return f.worker.workerHeldForTesting();});
    for(int n=0;n<65;++n)
    {
        f.begin();CaptureFailureEvent event;event.pass=f.pass;event.epoch=f.worker.captureGeneration();event.reason=CaptureFailureReason::timingUnavailable;f.worker.failPass(event);
        if(n!=64)f.worker.cancelWorkflowPass();
    }
    REQUIRE(f.worker.omittedFailureEvents()==1);REQUIRE(f.flow.stop(f.pass));f.worker.holdWorkerForTesting(false);REQUIRE(f.worker.flush());
    REQUIRE(f.flow.snapshot().phase==workflow::Phase::failed && f.worker.snapshot().manifest.ranges.empty());
    REQUIRE(!f.worker.snapshot().failure && f.worker.snapshot().status.contains("details unavailable"));
    f.retry();
}

inline void lateWorkerFailureCase()
{
    CaptureFailureFixture f(true);const auto before=f.worker.snapshot();std::atomic<bool> pinned{false},release{false};
    f.worker.setPassContextProvider([&](uint64_t)->std::optional<CaptureGenerationContext>
    {
        pinned=true;const auto until=juce::Time::getMillisecondCounter()+10000;
        while(!release && juce::Time::getMillisecondCounter()<until)juce::Thread::sleep(1);
        throw std::runtime_error("Obsolete pass pin failed");
    });
    f.begin(true);REQUIRE(f.queue->tryPush(0,1000,f.audio.data(),f.audio.data(),64,f.worker.captureGeneration(),f.pass));
    awaitCapture([&]{return pinned.load();});f.worker.cancelWorkflowPass();f.begin();
    CaptureFailureEvent current;current.pass=f.pass;current.epoch=f.worker.captureGeneration();current.reason=CaptureFailureReason::timingUnavailable;f.worker.failPass(current);
    REQUIRE(f.flow.stop(f.pass));release=true;REQUIRE(f.worker.flush());
    REQUIRE(f.flow.snapshot().phase==workflow::Phase::failed && f.worker.snapshot().reference==before.reference);
    REQUIRE(f.worker.snapshot().failure.event.pass==f.pass && f.worker.snapshot().failure.event.reason==CaptureFailureReason::timingUnavailable);
    f.worker.setPassContextProvider({});f.retry();
}
