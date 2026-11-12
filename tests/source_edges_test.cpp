#include "AudioTestFiles.h"
#include "AllocationGuard.h"
#include <Capture/CaptureSession.h>
#include <Capture/SourceVisitor.h>
#include <Core/ContentHash.h>
#include <DSP/SourceEdgeConditioning.h>
#include <DSP/SwellEngine.h>
#include <numbers>
#include <source_location>
using namespace m3;

template<class F> static void waitFor(F condition,const std::source_location location=std::source_location::current())
{
    const auto deadline=juce::Time::getMillisecondCounter()+30000;
    while(!condition()){if(juce::Time::getMillisecondCounter()>deadline)throw std::runtime_error("Timeout at "+std::to_string(location.line()));juce::Thread::sleep(2);}
}
static void checkDsp()
{
    juce::String error;
    for(const auto rate:{44100.,48000.,96000.})
    {
        const auto ramp=static_cast<size_t>(std::llround(rate*.001));
        std::vector<float> input(2000);
        for(size_t i=0;i<input.size()/2;++i){input[2*i]=.25f+static_cast<float>(i)*.00001f;input[2*i+1]=-2*input[2*i];}
        auto wet=input;const auto* pointer=wet.data();const auto capacity=wet.capacity();
        {AudioAllocationGuard guard;REQUIRE(conditionWetSourceEdges(wet,rate,error));}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && wet.data()==pointer && wet.capacity()==capacity);
        for(size_t i=0;i<wet.size()/2;++i)
        {
            const auto edge=std::min(i,wet.size()/2-1-i);
            const auto gain=edge<ramp?.5-.5*std::cos(std::numbers::pi*static_cast<double>(edge)/static_cast<double>(ramp-1)):1.;
            REQUIRE(std::abs(wet[2*i]-input[2*i]*gain)<3e-8);
            REQUIRE(wet[2*i+1]==-2*wet[2*i]);
            if(edge>=ramp)REQUIRE(wet[2*i]==input[2*i]);
        }
        REQUIRE(wet.front()==0 && wet.back()==0 && input.front()!=0 && input.back()!=0);
        // Independent double-precision direct convolution; complete length/negative origin.
        ImpulseResponse ir;ir.rate=rate;ir.channels=2;ir.samples={1,-.5f,.2f,.3f,-.1f,.7f};
        WetRender raw;REQUIRE(renderReverseConvolution(wet,0,rate,ir,0,raw,error));
        REQUIRE(raw.start==-2 && raw.frames()==1002 && raw.sourceStart==0 && raw.sourceEnd==1000);
        for(size_t i=0;i<raw.frames();++i)for(size_t c=0;c<2;++c)
        {
            double expected=0;
            for(size_t k=0;k<3;++k)if(i>=k && i-k<1000)expected+=static_cast<double>(wet[2*(i-k)+c])*ir.samples[2*(2-k)+c];
            REQUIRE(std::abs(raw.samples[2*i+c]-expected)<2e-6);
        }
    }
    for(size_t frames=1;frames<=101;++frames)
    {
        std::vector<float> shortInput(frames*2,1);REQUIRE(conditionWetSourceEdges(shortInput,48000,error));
        REQUIRE(shortInput.size()==frames*2);
        REQUIRE(*std::max_element(shortInput.begin(),shortInput.end())==1);
        for(auto v:shortInput)REQUIRE(std::isfinite(v) && v>=0 && v<=1);
        if(frames<4)REQUIRE(shortInput==std::vector<float>(frames*2,1));
        else REQUIRE(shortInput.front()==0 && shortInput.back()==0);
    }
    std::vector<float> silence(400,0);REQUIRE(conditionWetSourceEdges(silence,48000,error));
    REQUIRE(std::all_of(silence.begin(),silence.end(),[](float v){return v==0;}));
    std::vector<float> cancelled(200,1);const auto original=cancelled;
    REQUIRE(!conditionWetSourceEdges(cancelled,48000,error,[]{return true;}));REQUIRE(cancelled==original);
    int calls=0;cancelled.assign(20000,1);
    REQUIRE(!conditionWetSourceEdges(cancelled,4000000,error,[&]{return ++calls==3;}));REQUIRE(calls==3);
    std::vector<float> invalid;REQUIRE(!conditionWetSourceEdges(invalid,48000,error));
    invalid={1,1,1};REQUIRE(!conditionWetSourceEdges(invalid,48000,error));
    invalid={1,1};REQUIRE(!conditionWetSourceEdges(invalid,0,error));
    REQUIRE(!conditionWetSourceEdges(invalid,std::numeric_limits<double>::infinity(),error));
}
static void checkEngine()
{
    TestDirectory root;CaptureSession capture(root.file);juce::String error;
    std::vector<float> input(8192);
    for(size_t i=0;i<input.size()/2;++i){input[2*i]=.2f+.05f*std::sin(static_cast<float>(i)*.013f);input[2*i+1]=-.7f*input[2*i];}
    REQUIRE(capture.commit(0,48000,input.data(),4096));
    PreviewContext context{capture.snapshot(),{true,0,0,48000,122,4,4},0};
    const auto cache=root.file.getChildFile(context.capture.manifest.projectId).getChildFile(context.capture.manifest.instanceId);
    CaptureAssetStore sourceStore(cache);SwellAssetStore assets(cache);
    SourceInterval before;REQUIRE(readSourceInterval(sourceStore,context.capture.manifest,0,4096,4096,before,error));
    REQUIRE(before.samples==input);const auto captureIdentity=sampleHash(before.samples,48000,2);
    ImpulseResponse ir;ir.rate=48000;ir.channels=1;ir.samples={1,.5f,-.25f};Asset irAsset;REQUIRE(assets.writeIR(ir,irAsset,error));
    Session saved;saved.timing=context.time;saved.automaticProcessing=false;saved.defaultIR=irAsset;saved.defaultIRName="fixture";
    for(int i=0;i<4;++i)
    {
        Swell w;w.name="Preserved "+juce::String(i);w.sourceStart=w.arrival=100+i*700;w.sourceEnd=w.sourceStart+512;
        w.ir=irAsset;w.irName="fixture";w.placement=7;w.shape.gainDb=-3;w.frozen=i==2;w.enabled=i!=3;w.muted=i==1;
        SourceInterval source;REQUIRE(readSourceInterval(sourceStore,context.capture.manifest,w.sourceStart,w.sourceEnd,512,source,error));
        juce::String dependency;REQUIRE(sourceFingerprint(sourceStore,context.capture.manifest,w.sourceStart,w.sourceEnd,dependency,error));
        if(i==1){REQUIRE(conditionWetSourceEdges(source.samples,48000,error));dependency=juce::String(sourceEdgeRevision)+dependency;}
        WetRender raw;REQUIRE(renderReverseConvolution(source.samples,w.sourceStart,48000,ir,0,raw,error));
        REQUIRE(assets.writeRender(raw,irAsset,dependency,context.time,w.render,error));saved.swells.push_back(w);
    }
    saved.primary=saved.swells[0].id;saved.selected={saved.primary};
    SwellEngine engine(root.file,[&]{return context;});engine.restore(saved);
    const auto ready=[&]{const auto s=engine.savedSession();return s.swells.size()==4 && std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==Phase::ready;});};
    waitFor(ready);REQUIRE(engine.snapshot().renderCount==0);
    for(size_t i=0;i<4;++i)REQUIRE(engine.savedSession().swells[i].render==saved.swells[i].render);
    const auto first=engine.generate(true);
    REQUIRE(first.accepted && first.queued==3 && first.frozen==1 && first.importing==0 && first.sessionEpoch==engine.assignmentEpoch());
    waitFor([&]{return ready() && engine.snapshot().renderCount==3;});
    const auto upgraded=engine.savedSession();
    for(size_t i=0;i<4;++i)
    {
        const auto& old=saved.swells[i];const auto& now=upgraded.swells[i];
        REQUIRE(now.id==old.id && now.name==old.name && now.sourceStart==old.sourceStart && now.sourceEnd==old.sourceEnd);
        REQUIRE(now.shape==old.shape && now.placement==old.placement && now.enabled==old.enabled && now.muted==old.muted && now.frozen==old.frozen);
        REQUIRE(assets.verify(old.render,error)); // Previous complete audio retained.
        if(i==2){REQUIRE(now.render==old.render);continue;}
        REQUIRE(now.inputGeneration>old.inputGeneration && now.render.dependency.startsWith(sourceEdgeRevision));
        if(i!=1)REQUIRE(now.render!=old.render); // Identical current content may reuse its content-addressed asset.
        SourceInterval source;REQUIRE(readSourceInterval(sourceStore,context.capture.manifest,now.sourceStart,now.sourceEnd,512,source,error));
        REQUIRE(conditionWetSourceEdges(source.samples,48000,error));
        WetRender expected,actual;REQUIRE(renderReverseConvolution(source.samples,now.sourceStart,48000,ir,0,expected,error));
        REQUIRE(assets.readRender(now.render,previewMemoryBudget,actual,error));
        REQUIRE(actual.start==expected.start && actual.samples==expected.samples);
    }
    const auto repeated=engine.generate(true);REQUIRE(repeated.accepted && repeated.queued==3 && repeated.frozen==1);
    waitFor([&]{return ready() && engine.snapshot().renderCount==6;});
    const auto beforeSelected=engine.savedSession();
    REQUIRE(engine.selectTargets({saved.swells[1].id},saved.swells[1].id));engine.generate();
    waitFor([&]{return ready() && engine.snapshot().renderCount==7;});
    for(size_t i=0;i<4;++i)if(i!=1)REQUIRE(engine.savedSession().swells[i].inputGeneration==beforeSelected.swells[i].inputGeneration);
    // New processing identity survives the existing session codec and restore without rerender.
    const auto beforeRestore=engine.savedSession();
    juce::String json;REQUIRE(encode(beforeRestore,json,error));Session decoded;REQUIRE(decode(json,decoded,error));
    engine.restore(decoded);waitFor(ready);REQUIRE(engine.snapshot().renderCount==7);
    REQUIRE(engine.savedSession().swells[0].render==beforeRestore.swells[0].render);
    // An import in flight retains ownership while All queues the other eligible targets.
    engine.holdRenderPublicationForTesting(true);engine.generate();
    waitFor([&]{return engine.renderPublicationWaitingForTesting();});
    const auto importing=engine.savedSession().swells[1];
    REQUIRE(engine.assignManagedIR(importing.id,importing.inputGeneration,irAsset,"pending fixture",engine.assignmentEpoch()));
    const auto importGeneration=engine.savedSession().swells[1].inputGeneration;
    const auto duringImport=engine.generate(true);
    REQUIRE(duringImport.queued==2 && duringImport.frozen==1 && duringImport.importing==1);
    REQUIRE(engine.savedSession().swells[1].inputGeneration==importGeneration && engine.snapshot().irReplacementPending);
    engine.holdRenderPublicationForTesting(false);
    waitFor([&]{return ready() && !engine.snapshot().irReplacementPending;});
    // A failed explicit request keeps both old audio and immutable capture.
    const auto retained=engine.savedSession().swells[0].render;const auto failureCount=engine.snapshot().renderCount;
    REQUIRE(engine.edit(saved.swells[0].id,[](auto& w,auto&){w.sourceEnd=5000;return true;}));engine.generate(true);
    waitFor([&]{const auto session=engine.savedSession();return session.swells[0].phase==Phase::error && session.swells[1].phase==Phase::ready && session.swells[3].phase==Phase::ready;});
    REQUIRE(engine.snapshot().renderCount==failureCount+2);
    REQUIRE(engine.savedSession().swells[0].render==retained && assets.verify(retained,error));
    SourceInterval after;REQUIRE(readSourceInterval(sourceStore,capture.snapshot().manifest,0,4096,4096,after,error));
    REQUIRE(after.samples==before.samples && sampleHash(after.samples,48000,2)==captureIdentity);
    REQUIRE(capture.snapshot().manifest.revision==context.capture.manifest.revision);
}
int main(int argc,char** argv)
{
    return runTest([&]
    {
        if(argc==4)
        {
            // Optional local review utility; never writes the supplied source or IR.
            juce::String error;ImpulseResponse source,ir,prepared;WetRender wet;
            REQUIRE(loadImpulseResponse(juce::File(argv[1]),previewMemoryBudget,source,error) && source.channels==2);
            REQUIRE(loadImpulseResponse(juce::File(argv[2]),previewMemoryBudget,ir,error));
            REQUIRE(conditionWetSourceEdges(source.samples,source.rate,error));
            REQUIRE(prepareImpulseResponse(ir,source.rate,previewMemoryBudget,prepared,error));
            REQUIRE(renderReverseConvolution(source.samples,0,source.rate,prepared,0,wet,error));
            const juce::File destination(argv[3]);REQUIRE(!destination.exists());
            writeTestWav(destination,2,source.rate,wet.samples);return;
        }
        REQUIRE(argc==1);checkDsp();checkEngine();
    });
}
