#include "TestSupport.h"
#include <Capture/CaptureSession.h>
#include <Capture/CaptureWorker.h>
#include <Core/ContentHash.h>
#include <limits>

int main(){return runTest([]{
    TestDirectory root;CaptureSession s(root.file);
    float original[]{1,-1,2,-2,3,-3,4,-4};float silence[4]{};
    REQUIRE(s.commit(10,48000,original,4));REQUIRE(s.commit(20,48000,silence,2));
    const auto old=s.snapshot();float incoming[24];for(int i=0;i<12;++i){incoming[i*2]=float(i+100);incoming[i*2+1]=-float(i+100);}
    REQUIRE(s.beginPass(1,0,false));REQUIRE(s.stagePass(1,9,48000,incoming,12));
    REQUIRE(s.snapshot().reference==old.reference);float l,r;
    REQUIRE(s.readSample(10,l,r) && l==1 && !s.readSample(9,l,r));
    // Earlier staged positions and mapped silence are both protected on loops.
    REQUIRE(s.stagePass(1,9,48000,original,4));
    auto result=s.finishPass(1,0);REQUIRE(result.success && result.useful && result.changed && !result.replaced);
    REQUIRE(s.snapshot().manifest.revision==old.manifest.revision+1);
    for(int p=9;p<21;++p){REQUIRE(s.readSample(p,l,r));const auto expected=p>=10 && p<14?float(p-9):p>=20?0.0f:float(p-9+100);REQUIRE(l==expected && r==-expected);}
    REQUIRE(s.readSample(21,l,r) && l==0);
    REQUIRE(!s.finishPass(1,0).success);
    const auto additive=s.snapshot();
    REQUIRE(s.beginPass(2,0,false));REQUIRE(s.stagePass(2,10,48000,original,4));
    result=s.finishPass(2,0);REQUIRE(result.success && !result.changed && !result.useful && s.snapshot().reference==additive.reference);
    REQUIRE(s.beginPass(3,0,true));REQUIRE(s.stagePass(3,10,48000,original,4));
    result=s.finishPass(3,0);REQUIRE(result.success && result.useful && result.replaced && !result.changed);
    REQUIRE(s.beginPass(4,0,true));REQUIRE(s.stagePass(4,10,48000,incoming,4));
    REQUIRE(s.stagePass(4,12,48000,original,2));
    REQUIRE(s.readSample(10,l,r) && l==1);
    result=s.finishPass(4,0);REQUIRE(result.success && result.replaced);
    REQUIRE(s.readSample(10,l,r) && l==100);REQUIRE(s.readSample(12,l,r) && l==1);
    // Old immutable source snapshots remain exact after accepted replacements.
    CaptureSession earlier(root.file);REQUIRE(earlier.restore(old.manifest.projectId,old.manifest.instanceId,old.reference,old.manifest.revision));
    REQUIRE(earlier.readSample(10,l,r) && l==1);
    auto before=s.snapshot();REQUIRE(s.beginPass(5,0,true));REQUIRE(s.stagePass(5,10,48000,incoming,4));
    float bad[]{std::numeric_limits<float>::quiet_NaN(),0};REQUIRE(!s.stagePass(5,30,48000,bad,1));
    REQUIRE(!s.finishPass(5,0).success && s.snapshot().reference==before.reference);
    REQUIRE(s.readSample(10,l,r) && l==100);
    REQUIRE(s.beginPass(6,0,true));REQUIRE(s.stagePass(6,10,48000,original,4));s.abortPass();
    REQUIRE(!s.finishPass(6,0).success && s.snapshot().reference==before.reference);
    REQUIRE(s.beginPass(7,0,true));REQUIRE(!s.stagePass(7,10,44100,original,4));REQUIRE(!s.finishPass(7,0).success);
    REQUIRE(s.beginPass(8,0,true));REQUIRE(!s.stagePass(8,INT64_MAX,48000,original,4));REQUIRE(!s.finishPass(8,0).success);
    // Stale completion cannot commit; a later correct owner may settle.
    REQUIRE(s.beginPass(9,5,true));REQUIRE(s.stagePass(9,-10,48000,original,4));
    REQUIRE(!s.finishPass(9,4).success && s.snapshot().reference==before.reference);
    REQUIRE(s.finishPass(9,5).success && s.readSample(-10,l,r) && l==1);
    // Publication failure after a valid chunk leaves the old reference intact.
    before=s.snapshot();const auto cache=root.file.getChildFile(before.manifest.projectId).getChildFile(before.manifest.instanceId);
    REQUIRE(s.beginPass(10,0,true));REQUIRE(s.stagePass(10,10,48000,original,4));
    const auto manifests=cache.getChildFile("manifests"), backup=cache.getChildFile("held");
    REQUIRE(manifests.moveFileTo(backup) && manifests.replaceWithText("blocked"));
    REQUIRE(!s.finishPass(10,0).success && s.snapshot().reference==before.reference);
    REQUIRE(manifests.deleteFile() && backup.moveFileTo(manifests));
    CaptureSession clone(root.file);REQUIRE(clone.restore(before.manifest.projectId,before.manifest.instanceId,before.reference,before.manifest.revision));
    REQUIRE(clone.beginPass(1,0,true) && clone.stagePass(1,10,48000,original,4) && clone.finishPass(1,0).success);
    REQUIRE(s.snapshot().reference==before.reference && s.readSample(10,l,r) && l==100);
    // Prepared disk publication is not accepted source until ownership rechecks.
    const auto stable=s.snapshot();REQUIRE(s.beginPass(11,5,true));REQUIRE(s.stagePass(11,10,48000,original,4));
    REQUIRE(s.preparePass(11,5).success && s.snapshot().reference==stable.reference);
    REQUIRE(s.restore(stable.manifest.projectId,stable.manifest.instanceId,stable.reference,stable.manifest.revision));
    REQUIRE(!s.acceptPreparedPass(11,5) && s.snapshot().reference==stable.reference);
    // Multi-chunk and sparse spans remain private until one final revision.
    std::vector<float> chunk(48000*2,.1f);REQUIRE(s.beginPass(12,0,false));
    REQUIRE(s.stagePass(12,48000,48000,chunk.data(),48000) && s.stagePass(12,144000,48000,chunk.data(),48000));
    REQUIRE(s.snapshot().reference==stable.reference && !s.readSample(48000,l,r));
    REQUIRE(s.finishPass(12,0).success && s.snapshot().manifest.revision==stable.manifest.revision+1);
    REQUIRE(s.readSample(48000,l,r) && l==.1f && !s.readSample(96000,l,r));
    const auto checked=s.snapshot();const auto segments=cache.getChildFile("segments");const auto priorFiles=segments.findChildFiles(juce::File::findFiles,false);
    REQUIRE(s.beginPass(13,0,true) && s.stagePass(13,48000,48000,original,4));
    bool corrupted=false;for(const auto& file:segments.findChildFiles(juce::File::findFiles,false))if(!priorFiles.contains(file)){REQUIRE(file.replaceWithText("corrupt staged audio"));corrupted=true;break;}
    REQUIRE(corrupted && !s.finishPass(13,0).success && s.snapshot().reference==checked.reference);
    REQUIRE(m3::sampleHash({1,-1,2,-2,3,-3,4,-4,5,-5,6,-6,7,-7,8,-8},48000,2)=="27cef11381100414153548a069ad170379d701cce556ccdafef253005016837a");
    {
        auto queue=std::make_unique<PluginCaptureQueue>();workflow::State mode;
        std::atomic<unsigned> acceptedUpdates{0};
        CaptureWorker worker(*queue,root.file.getChildFile("silence-review"));worker.useWorkflow(mode);worker.setHostRate(48000);worker.startThread();
        worker.setAcceptedStateHandler([&]{++acceptedUpdates;});
        float note[]{.1f,.2f,.3f,.4f},zero[4]{};
        auto pass=mode.begin(true);
        REQUIRE(queue->tryPush(0,48000,note,note,4,worker.captureGeneration(),pass));
        REQUIRE(queue->tryPush(100,48000,note,note,4,worker.captureGeneration(),pass));
        REQUIRE(mode.stop(pass) && worker.flush() && mode.snapshot().mode==workflow::Mode::live);
        REQUIRE(mode.requestMode(workflow::Mode::capture,{true,true,false}) && mode.arm(true));pass=mode.begin(true);
        REQUIRE(queue->tryPush(0,48000,zero,zero,4,worker.captureGeneration(),pass));
        REQUIRE(mode.stop(pass) && worker.flush() && worker.hasUsefulSource());
        REQUIRE(worker.snapshot().manifest.ranges.front().kind==CaptureCoverageKind::silence);
        REQUIRE(mode.snapshot().mode==workflow::Mode::live && !mode.snapshot().recapture);
        REQUIRE(acceptedUpdates==2);
        const auto acceptedSilence=worker.snapshot().reference;
        REQUIRE(mode.requestMode(workflow::Mode::capture,{true,true,false}) && mode.arm(true));pass=mode.begin(true);
        REQUIRE(queue->tryPush(0,48000,zero,zero,4,worker.captureGeneration(),pass));
        REQUIRE(mode.stop(pass) && worker.flush() && worker.snapshot().reference==acceptedSilence);
        REQUIRE(mode.snapshot().mode==workflow::Mode::live && !mode.snapshot().recapture && acceptedUpdates==3);
        REQUIRE(mode.requestMode(workflow::Mode::capture,{true,true,false}) && mode.arm(true));pass=mode.begin(true);
        REQUIRE(queue->tryPush(100,48000,zero,zero,4,worker.captureGeneration(),pass));
        REQUIRE(mode.stop(pass) && worker.flush() && !worker.hasUsefulSource());
        REQUIRE(mode.snapshot().mode==workflow::Mode::capture && !mode.snapshot().recapture && acceptedUpdates==4);
        worker.stopThread(-1);
    }
    {
        auto queue=std::make_unique<PluginCaptureQueue>();workflow::State mode;
        CaptureWorker worker(*queue,root.file.getChildFile("worker"));worker.useWorkflow(mode);worker.setHostRate(48000);worker.startThread();
        float left[]{.1f,.2f,.3f,.4f},right[]{-.1f,-.2f,-.3f,-.4f};
        auto pass=mode.begin(true);REQUIRE(pass && queue->tryPush(0,48000,left,right,4,worker.captureGeneration(),pass));
        REQUIRE(worker.flush());REQUIRE(worker.snapshot().manifest.ranges.empty());
        REQUIRE(mode.stop(pass));REQUIRE(worker.flush());
        REQUIRE(mode.snapshot().mode==workflow::Mode::live && !worker.snapshot().manifest.ranges.empty());
        const auto accepted=worker.snapshot();
        REQUIRE(mode.requestMode(workflow::Mode::capture,{true,true,true}));pass=mode.begin(true);
        REQUIRE(queue->tryPush(0,48000,right,left,4,worker.captureGeneration(),pass));REQUIRE(mode.stop(pass));REQUIRE(worker.flush());
        REQUIRE(mode.snapshot().mode==workflow::Mode::capture && worker.snapshot().reference==accepted.reference);
        REQUIRE(mode.arm(true));pass=mode.begin(true);
        REQUIRE(queue->tryPush(0,48000,right,left,4,worker.captureGeneration(),pass));worker.failPass(pass);
        REQUIRE(mode.stop(pass) && worker.flush());REQUIRE(mode.snapshot().phase==workflow::Phase::failed && worker.snapshot().reference==accepted.reference);
        worker.cancelWorkflowPass();REQUIRE(!mode.snapshot().recapture);worker.stopThread(-1);
    }
});}
