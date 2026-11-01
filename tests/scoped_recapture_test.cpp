#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Capture/CaptureWorker.h>
using namespace m3;
int main() { return runTest([]
{
    ScopedRecapture arm;
    REQUIRE(!arm.begin(10,10,48000)); REQUIRE(arm.begin(10,20,48000));
    CaptureAdmission a;
    {AudioAllocationGuard guard;a=arm.admit(0,16,48000);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    REQUIRE(a.armed && a.offset==10 && a.count==6 && a.token!=0);
    REQUIRE(!arm.committed(10,3,a.token)); REQUIRE(arm.snapshot().covered==3);
    REQUIRE(!arm.committed(16,4,a.token)); REQUIRE(arm.snapshot().covered==7);
    arm.failed(a.token); REQUIRE(arm.snapshot().active && arm.snapshot().covered==7);
    REQUIRE(arm.committed(13,3,a.token)); REQUIRE(!arm.snapshot().active && arm.snapshot().covered==10);
    REQUIRE(arm.begin(-5,5,48000)); const auto next=arm.admit(-10,30,48000);
    REQUIRE(next.offset==5 && next.count==10 && !arm.committed(-5,10,a.token));
    REQUIRE(arm.admit(-5,10,44100).count==0); arm.cancel(); REQUIRE(!arm.committed(-5,10,next.token));
    TestDirectory root; auto queue=std::make_unique<PluginCaptureQueue>(); CaptureWorker worker(*queue,root.file);
    worker.setHostRate(1000); worker.startThread();
    std::atomic<int> completions{0}; worker.setRecaptureHandler([&](int64_t begin,int64_t end){REQUIRE(begin==10 && end==20);++completions;});
    REQUIRE(worker.requestRecapture(10,20,1000)); auto admitted=worker.recaptureAdmission(0,15,1000);
    float left[30],right[30]; for(int n=0;n<30;++n){left[n]=static_cast<float>(n);right[n]=-left[n];}
    {AudioAllocationGuard guard; REQUIRE(queue->tryPush(10,1000,left+10,right+10,5,worker.captureGeneration(),admitted.token));}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    REQUIRE(worker.flush()); REQUIRE(completions==0 && worker.recaptureSnapshot().covered==5);
    admitted=worker.recaptureAdmission(15,15,1000);
    REQUIRE(queue->tryPush(15,1000,left+15,right+15,admitted.count,worker.captureGeneration(),admitted.token));
    REQUIRE(worker.flush()); REQUIRE(completions==1 && !worker.recaptureSnapshot().active);
    const auto snapshot=worker.snapshot(); REQUIRE(snapshot.manifest.ranges.front().startSample==10);
    REQUIRE(snapshot.manifest.ranges.back().startSample+static_cast<int64_t>(snapshot.manifest.ranges.back().frameCount)==20);
    // Existing committed coverage cannot complete a newly armed fresh capture.
    REQUIRE(worker.requestRecapture(10,20,1000)); REQUIRE(worker.recaptureSnapshot().covered==0);
    REQUIRE(worker.flush()); REQUIRE(completions==1 && worker.recaptureSnapshot().active);
    worker.cancelRecapture(); worker.stopThread(-1);
}); }
