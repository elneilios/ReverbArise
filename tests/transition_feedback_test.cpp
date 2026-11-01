#include "AudioTestFiles.h"
#include "AllocationGuard.h"
#include <DSP/SwellEngine.h>
#include <source_location>
using namespace m3;
template<class F>static void await(F f,const std::source_location loc=std::source_location::current())
{const auto deadline=juce::Time::getMillisecondCounter()+30000;while(!f()){if(juce::Time::getMillisecondCounter()>deadline)throw std::runtime_error("Transition timeout "+std::to_string(loc.line()));juce::Thread::sleep(2);}}
#include "WorkspaceFeedbackCases.h"
int main(){return runTest([]{
    TestDirectory root;CaptureSession capture(root.file);juce::CriticalSection lock;
    std::vector<float> pcm(3000*2);for(int i=0;i<3000;++i){pcm[i*2]=.1f*std::sin(i*.1f);pcm[i*2+1]=.07f*std::cos(i*.13f);}
    REQUIRE(capture.commit(0,1000,pcm.data(),600));
    PreviewContext context{capture.snapshot(),{true,0,0,1000,240,4,4},0};
    SwellEngine engine(root.file,[&]{const juce::ScopedLock g(lock);return context;});engine.setAutomaticProcessing(false);
    REQUIRE(engine.addTarget(100));const auto id=engine.savedSession().primary;
    std::vector<float> ir(101);ir[0]=1;ir[1]=.5f;ir[2]=.25f;writeTestWav(root.file.getChildFile("ir.wav"),1,1000,ir);engine.loadIR(root.file.getChildFile("ir.wav"));
    await([&]{return find(engine.savedSession(),id)->irName=="ir.wav";});engine.generate();await([&]{return engine.snapshot().auditionReady;});
    const auto baseline=engine.snapshot();const auto original=*find(baseline.session,id);
    const auto editEnd=[&](int64_t end){REQUIRE(engine.edit(id,[&](auto& w,auto& error){auto s=w.shape;s.endOffset=end-w.arrival;s.fadeOut=40;s.outTension=.4;return setShape(w,s,baseline.raw->start,baseline.raw->end,error);}));};
    editEnd(800);await([&]{return find(engine.savedSession(),id)->status=="Waiting for source";});
    REQUIRE(find(engine.savedSession(),id)->shape.endOffset==700 && !engine.snapshot().auditionReady);
    // A pending unavailable extension can be cancelled by returning inside old support.
    editEnd(300);await([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.snapshot().renderCount==baseline.renderCount);
    editEnd(800);await([&]{return find(engine.savedSession(),id)->status=="Waiting for source";});
    REQUIRE(capture.commit(600,1000,pcm.data()+1200,2400));{const juce::ScopedLock g(lock);context.capture=capture.snapshot();}
    await([&]{return engine.snapshot().auditionReady;});const auto extended=engine.snapshot();const auto w=*find(extended.session,id);
    REQUIRE(w.arrival==original.arrival && w.sourceStart==original.sourceStart && w.sourceEnd==original.sourceEnd);
    REQUIRE(w.render.sourceEnd==800 && w.render.start==0 && w.render.frames==800 && extended.renderCount==baseline.renderCount+1);
    SwellAssetStore assets(engine.assetDirectory());WetRender raw;juce::String error;REQUIRE(assets.readRender(w.render,previewMemoryBudget,raw,error));
    for(int n=0;n<800;++n)for(int c=0;c<2;++c){double expected=0;for(int k=0;k<3;++k){const auto input=n-98-k;if(input>=0 && input<700)expected+=pcm[(100+input)*2+c]*(k==0?.25:k==1?.5:1);}
        REQUIRE(std::abs(raw.samples[n*2+c]-expected)<1e-5);}
    const auto checksum=w.render.checksum;editEnd(90);await([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.snapshot().renderCount==extended.renderCount && find(engine.savedSession(),id)->render.checksum==checksum);
    REQUIRE(find(engine.savedSession(),id)->shape.fadeOut==40);
    engine.setMonitoring(AuditionMode::dryWet,0);await([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.audition());
    float gapLeft[2200]{},gapRight[2200]{};engine.processAudio(gapLeft,gapRight,2200,context.time);
    for(int i=110;i<2100;++i){REQUIRE(std::abs(gapLeft[i]-pcm[i*2]*std::min(1.0f,(2099-i)/5.0f))<1e-6);REQUIRE(std::abs(gapRight[i]-pcm[i*2+1]*std::min(1.0f,(2099-i)/5.0f))<1e-6);}
    REQUIRE(gapLeft[2100]==0 && gapRight[2100]==0);
    editEnd(800);await([&]{return engine.snapshot().auditionReady;});
    engine.setMonitoring(AuditionMode::dryWet,0);await([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.savedSession().postRollSeconds==2 && engine.audition());
    float left[2900]{},right[2900]{};{AudioAllocationGuard guard;engine.processAudio(left,right,2900,context.time);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    // Dry begins at sample 100 and continues through max(350,800+2000)=2800.
    for(int i=110;i<2800;++i){REQUIRE(std::abs(left[i]-pcm[i*2]*std::min(1.0f,(2799-i)/5.0f))<1e-6);REQUIRE(std::abs(right[i]-pcm[i*2+1]*std::min(1.0f,(2799-i)/5.0f))<1e-6);}
    REQUIRE(left[2800]==0 && right[2800]==0);
    const auto count=engine.snapshot().renderCount;engine.setPostRoll(1);await([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.snapshot().renderCount==count);engine.setPostRoll(5);await([&]{return !engine.snapshot().auditionReady && engine.snapshot().dryStatus.contains("5800");});
    engine.setMonitoring(AuditionMode::wetOnly,.5f);await([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.snapshot().renderCount==count);
    auto saved=engine.savedSession();juce::String json;REQUIRE(encode(saved,json,error));Session decoded;REQUIRE(decode(json,decoded,error));
    REQUIRE(decoded.postRollSeconds==5 && decoded.swells[0].shape.outTension==.4 && decoded.swells[0].supportOffset==450);
    engine.restore(decoded);await([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.snapshot().renderCount==count && !engine.snapshot().playing);
    REQUIRE(find(engine.savedSession(),id)->shape.outTension==.4 && engine.savedSession().postRollSeconds==5);
    engine.setMonitoring(AuditionMode::dryWet,.5f);await([&]{return !engine.snapshot().auditionReady && engine.snapshot().dryStatus.contains("5800");});
    // Capturing only the missing listening context must retry monitoring, without FFT.
    std::vector<float> silence(2800*2);REQUIRE(capture.commit(3000,1000,silence.data(),2800));
    {const juce::ScopedLock g(lock);context.capture=capture.snapshot();}
    await([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.snapshot().renderCount==count);
    engine.setMonitoring(AuditionMode::wetOnly,.5f);await([&]{return engine.snapshot().auditionReady;});
    // A changed sample exclusively in extended input invalidates the raw dependency.
    float replacement[]{.2f,-.1f};REQUIRE(capture.commit(700,1000,replacement,1));{const juce::ScopedLock g(lock);context.capture=capture.snapshot();}
    engine.recaptureCompleted(700,701);await([&]{return engine.snapshot().renderCount==count+1 && engine.snapshot().auditionReady;});
    workspaceFeedbackCases();
});}
