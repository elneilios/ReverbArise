#include "TestSupport.h"
#include <Capture/CaptureWorker.h>
#include <map>

template<class Predicate> void awaitCapture(Predicate predicate)
{
    const auto until=juce::Time::getMillisecondCounter()+10000;
    while(!predicate() && juce::Time::getMillisecondCounter()<until)juce::Thread::sleep(1);
    REQUIRE(predicate());
}
#include "CaptureFailureCases.h"
#include "CapturePressureCases.h"

static void failedQueuePass(bool prior)
{
    TestDirectory root;auto queue=std::make_unique<PluginCaptureQueue>();workflow::State flow;
    CaptureWorker worker(*queue,root.file);worker.useWorkflow(flow);worker.setHostRate(1000);worker.startThread();
    std::vector<float> audio(5000,.25f);
    auto start=[&]{if(flow.snapshot().mode!=workflow::Mode::capture)REQUIRE(flow.requestMode(workflow::Mode::capture,{true,true,false}));return flow.begin(true);};
    auto send=[&](uint64_t pass,int64_t at){REQUIRE(queue->tryPush(at,1000,audio.data(),audio.data(),5000,worker.captureGeneration(),pass));REQUIRE(worker.flush());};
    if(prior){auto pass=start();send(pass,0);REQUIRE(flow.stop(pass) && worker.flush());}
    const auto before=worker.snapshot();
    std::map<juce::String,juce::MemoryBlock> hashes;
    for(auto file:root.file.findChildFiles(juce::File::findFiles,true)){juce::MemoryBlock bytes;REQUIRE(file.loadFileAsData(bytes));hashes.emplace(file.getFullPathName(),std::move(bytes));}
    const auto pass=start();send(pass,10000);send(pass,15000);
    awaitCapture([&]{return !worker.progressSnapshot().ranges.empty();});
    REQUIRE(worker.provisionalSnapshot());
    REQUIRE(worker.snapshot().reference==before.reference);
    worker.holdWorkerForTesting(true);awaitCapture([&]{return worker.workerHeldForTesting();});
    unsigned queued=0;while(queue->tryPush(20000+queued*64,1000,audio.data(),audio.data(),64,worker.captureGeneration(),pass))++queued;
    REQUIRE(queued==32);worker.failPass(pass);
    REQUIRE(worker.progressSnapshot().ranges.empty());
    REQUIRE(!worker.provisionalSnapshot());
    worker.holdWorkerForTesting(false);
    awaitCapture([&]{return bool(worker.snapshot().failure);});
    REQUIRE(flow.snapshot().phase==workflow::Phase::recording && worker.snapshot().status.contains("retry"));
    REQUIRE(flow.stop(pass));REQUIRE(worker.flush());
    REQUIRE(flow.snapshot().phase==workflow::Phase::failed);
    REQUIRE(m3::ui::captureFailureText(worker.snapshot().failure,true).contains("queue 32/32"));
    REQUIRE(worker.snapshot().reference==before.reference && worker.snapshot().manifest.revision==before.manifest.revision);
    for(const auto& [path,bytes]:hashes){juce::MemoryBlock actual;REQUIRE(juce::File(path).loadFileAsData(actual) && actual==bytes);}
    if(prior)
    {
        CaptureSession restored(root.file);REQUIRE(restored.restore(before.manifest.projectId,before.manifest.instanceId,before.reference,before.manifest.revision));
        float l=0,r=0;REQUIRE(restored.readSample(0,l,r) && l==.25f && r==.25f);
        REQUIRE(restored.snapshot().manifest.contextSpans==before.manifest.contextSpans);
    }
    const auto retry=start();send(retry,30000);REQUIRE(flow.stop(retry) && worker.flush());
    REQUIRE(flow.snapshot().result==workflow::Result::accepted && worker.snapshot().reference!=before.reference);
    std::cout<<"queue rollback prior="<<prior<<" retained="<<hashes.size()<<" staged-files="<<root.file.findChildFiles(juce::File::findFiles,true).size()<<'\n';
    worker.stopThread(-1);
}
int main(int argc,char** argv){return runTest([&]{
    if(argc>1 && std::string(argv[1])=="--matrix"){capturePressureMatrix();return;}
    if(argc>1 && std::string(argv[1])=="--realtime"){capturePressureCase(180,48000,512,true,false);capturePressureCase(180,48000,512,true,true);return;}
    if(argc>1 && std::string(argv[1])=="--probe-small-blocks"){capturePressureCase(180,44100,64,true,false,false,true);capturePressureCase(180,48000,64,true,false,false,true);return;}
    failedQueuePass(false);failedQueuePass(true);workerFailureCases();captureFailureOwnershipCases();captureDiagnosticCapacityCase();lateWorkerFailureCase();
});}
