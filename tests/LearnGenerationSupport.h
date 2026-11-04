#pragma once
#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Plugin/PluginProcessor.h>
#include <source_location>
using namespace m3;
struct GenerationHost:juce::AudioPlayHead
{PositionInfo position;juce::Optional<PositionInfo> getPosition()const override{return position;}};
template<class Condition> static void await(Condition condition,const std::source_location location=std::source_location::current())
{
    const auto deadline=juce::Time::getMillisecondCounter()+30000;
    while(!condition())
    {if(juce::Time::getMillisecondCounter()>deadline)throw std::runtime_error("Generation timeout at "+std::string(location.file_name())+":"+std::to_string(location.line()));juce::Thread::sleep(2);}
}
struct GenerationFixture
{
    TestDirectory root;GenerationHost host;std::unique_ptr<PreverbAudioProcessor> processor;
    juce::AudioBuffer<float> audio{2,4096};juce::MidiBuffer midi;
    GenerationFixture():processor(std::make_unique<PreverbAudioProcessor>(root.file))
    {
        processor->prepareToPlay(48000,4096);processor->setPlayHead(&host);
        host.position.setBpm(120);host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
        engine().setLivePlayback(false,LiveRouting::dryWet,0);
        await([&]{return engine().savedSession().defaultIR.present();});tick(0,false);
    }
    SwellEngine& engine(){return processor->swellEngine();}
    void tick(int64_t at,bool playing,float scale=1)
    {
        host.position.setIsPlaying(playing);host.position.setTimeInSamples(at);host.position.setPpqPosition(at/24000.0);audio.clear();
        if(playing)for(int i=0;i<audio.getNumSamples();++i)
        {
            const auto local=(at+i)%28800;
            if(local>=4800 && local<9600)
            {const auto value=static_cast<float>(scale*.2*std::exp(-(local-4800)/1000.0)*std::sin(local*.17));audio.setSample(0,i,value);audio.setSample(1,i,value*.7f);}
        }
        {AudioAllocationGuard guard;processor->processBlock(audio,midi);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    }
    void record(bool inspectProvisional=false,bool flush=true)
    {
        tick(0,false);
        for(int block=0;block<72;++block){tick(block*4096,true);juce::Thread::sleep(2);}
        if(inspectProvisional)
        {
            await([&]{const auto view=engine().snapshot().learnGeneration;return view.displayed && view.displayed->identity.provisional && !view.displayed->positions.empty();});
            const auto snapshot=engine().snapshot();REQUIRE(!snapshot.learnGeneration.canGenerate && snapshot.session.swells.empty());
            REQUIRE(!engine().generateLearn(snapshot.sessionEpoch,processor->getWorkflowSnapshot().pass,snapshot.learnGeneration.displayed->serial));
        }
        tick(72*4096,false);if(flush)REQUIRE(processor->flushCaptureForTesting());
    }
    EditorSnapshot ready(const std::source_location location=std::source_location::current())
    {
        try{await([&]{return engine().snapshot().learnGeneration.canGenerate;},location);}
        catch(...){const auto view=engine().snapshot().learnGeneration;std::cerr<<"Readiness: "<<view.reason<<" | "<<view.notice
            <<" | source="<<processor->getCaptureSnapshot().status<<" phase="<<static_cast<int>(processor->getWorkflowSnapshot().phase)<<'\n';throw;}
        return engine().snapshot();
    }
    bool generate(const std::source_location location=std::source_location::current())
    {
        const auto snapshot=ready(location);return engine().generateLearn(snapshot.sessionEpoch,processor->getWorkflowSnapshot().pass,snapshot.learnGeneration.displayed->serial);
    }
};
