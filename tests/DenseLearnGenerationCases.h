#pragma once
#include "LearnGenerationSupport.h"
// Deterministic layered arrangement fixture, not a detector-quality listening
// oracle: two 16-bar sections at 120 BPM, separated by an uncaptured 16-bar gap.
inline void denseLearnGenerationCases()
{
    GenerationFixture fixture;auto& processor=*fixture.processor;auto& engine=fixture.engine();
    const auto started=juce::Time::getMillisecondCounterHiRes();
    for(int section:{0,2})
    {
        if(section==0)processor.holdCaptureValidationForTesting(true);
        const int64_t start=section*32LL*48000,frames=32LL*48000;
        for(int64_t local=0;local<frames;local+=4096)
        {
            const auto count=static_cast<int>(std::min<int64_t>(4096,frames-local));fixture.audio.setSize(2,count);fixture.audio.clear();
            fixture.host.position.setIsPlaying(true);fixture.host.position.setTimeInSamples(start+local);fixture.host.position.setPpqPosition((start+local)/24000.0);
            for(int n=0;n<count;++n)
            {
                const auto at=local+n,beat=at%24000,offbeat=(at+12000)%24000;
                const auto accent=(at/24000)%4==0?.23:.16;
                const auto hit=beat>=2400 && beat<8400?accent*std::exp(-(beat-2400)/1100.0)*std::sin(beat*.17):0;
                const auto ghost=offbeat>=2400 && offbeat<5400?.012*std::exp(-(offbeat-2400)/450.0)*std::sin(offbeat*.61):0;
                const auto bed=.002*(std::sin(at*.0144)+std::sin(at*.0181));
                fixture.audio.setSample(0,n,static_cast<float>(hit+ghost+bed));fixture.audio.setSample(1,n,static_cast<float>(hit*.7-ghost+bed));
            }
            {AudioAllocationGuard guard;processor.processBlock(fixture.audio,fixture.midi);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0);juce::Thread::sleep(1);
            // The fixture streams much faster than wall-clock audio. Drain the
            // private writer periodically so Debug/compiler contention does not
            // turn this detector test into an unrelated capture-overflow test.
            if((local/4096)%16==15)REQUIRE(processor.flushCaptureForTesting());
        }
        fixture.tick(start+frames,false);
        if(section==0)
        {
            await([&]{return processor.getWorkflowSnapshot().phase==workflow::Phase::validating;});
            juce::Thread::sleep(30);processor.holdCaptureValidationForTesting(false);
        }
        REQUIRE(processor.flushCaptureForTesting());
        await([&]{return !processor.getWorkflowSnapshot().locked();});
        REQUIRE(processor.getWorkflowSnapshot().result==workflow::Result::accepted);fixture.ready();
    }
    const auto captured=processor.getCaptureSnapshot();
    REQUIRE(std::none_of(captured.manifest.ranges.begin(),captured.manifest.ranges.end(),[](const auto& r)
        {return r.startSample>=32LL*48000 && r.startSample<64LL*48000;}));
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));REQUIRE(engine.addTarget(120000));
    const auto manualId=engine.savedSession().primary;
    await([&]{return find(engine.savedSession(),manualId)->phase==Phase::ready;});
    REQUIRE(engine.edit(manualId,[](auto& swell,auto&){swell.name="Protected full-song edit";swell.shape.gainDb=-4;swell.enabled=false;return true;}));
    const auto protectedWork=*find(engine.savedSession(),manualId);
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));fixture.ready();engine.holdLearnRendersForTesting(true);
    const auto fileCount=[&]{return engine.assetDirectory().findChildFiles(juce::File::findFiles,true).size();};
    size_t lowCount=0,highCount=0;
    for(float sensitivity:{.2f,.5f,.8f,.2f})
    {
        const auto before=engine.snapshot();const auto files=fileCount();const auto began=juce::Time::getMillisecondCounterHiRes();
        engine.setPreferences(0,sensitivity,.08,false,processor.getWorkflowSnapshot().pass);const auto preview=fixture.ready();
        const auto count=preview.learnGeneration.displayed->positions.size();REQUIRE(count>=61 && count<=maximumSwells);
        if(fileCount()!=files || engine.snapshot().renderCount!=before.renderCount)
        {
            std::cerr<<"Preview side effects at sensitivity "<<sensitivity<<": files "<<files<<" -> "<<fileCount()<<", renders "<<before.renderCount<<" -> "<<engine.snapshot().renderCount<<'\n';
            for(const auto& swell:engine.savedSession().swells)std::cerr<<swell.name<<" phase="<<static_cast<int>(swell.phase)<<" "<<swell.status<<'\n';
        }
        REQUIRE(fileCount()==files && engine.snapshot().renderCount==before.renderCount);
        const bool reducing=sensitivity==.2f && lowCount;
        if(reducing)
        {
            // Keep an earlier high-density job in flight at publication. Generate
            // must interrupt it and commit without waiting for Ready or this gate.
            engine.holdRenderPublicationForTesting(true);engine.holdLearnRendersForTesting(false);
            await([&]{return engine.renderPublicationWaitingForTesting();});
        }
        REQUIRE(fixture.generate());await([&]{return !engine.snapshot().learnGeneration.generating;});
        const auto generated=engine.savedSession();REQUIRE(generated.swells.size()>=61 && generated.swells.size()<=count+1);
        if(generated.swells.size()!=before.session.swells.size())REQUIRE(processor.getWorkflowSnapshot().mode==workflow::Mode::live);
        if(sensitivity==.2f && !lowCount)lowCount=generated.swells.size();
        else if(sensitivity==.2f){REQUIRE(highCount>lowCount && generated.swells.size()==lowCount && engine.snapshot().renderCount==before.renderCount);}
        if(sensitivity==.8f)highCount=generated.swells.size();
        REQUIRE(sameSavedSwell(*find(generated,manualId),protectedWork));
        std::set<juce::String> ids;for(const auto& swell:generated.swells)REQUIRE(ids.insert(swell.id).second);
        std::cout<<"Dense Learn sensitivity="<<sensitivity<<" candidates="<<count<<" swells="<<generated.swells.size()
            <<" preview+commit-ms="<<(juce::Time::getMillisecondCounterHiRes()-began)<<" resident-bytes="<<engine.residentBytes()<<'\n';
        if(processor.getWorkflowSnapshot().mode==workflow::Mode::live)REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));fixture.ready();
        const auto retained=engine.savedSession();REQUIRE(fixture.generate());
        await([&]{const auto view=engine.snapshot().learnGeneration;return !view.generating && view.notice.contains("No changes");});
        REQUIRE(engine.savedSession().swells.size()==retained.swells.size() && processor.getWorkflowSnapshot().mode==workflow::Mode::capture);
    }
    REQUIRE(engine.residentBytes()<=previewMemoryBudget);
    engine.holdLearnRendersForTesting(true);engine.holdRenderPublicationForTesting(false);
    std::cout<<"Dense Learn total-ms="<<(juce::Time::getMillisecondCounterHiRes()-started)<<" mapped-seconds=64 extent-seconds=96\n";
}
