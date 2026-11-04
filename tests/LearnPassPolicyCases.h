#pragma once
#include "M3ProcessorCases.h"
inline void learnPassPolicyCases()
{
    for(const bool replace:{false,true})for(const bool automatic:{false,true})
    {
        TestDirectory root;auto processor=std::make_unique<PreverbAudioProcessor>(root.file);
        auto& engine=processor->swellEngine();processor->prepareToPlay(48000,1024);
        engine.setLivePlayback(false,m3::LiveRouting::dryWet,0);
        TestPlayHead head;processor->setPlayHead(&head);juce::MidiBuffer midi;juce::AudioBuffer<float> buffer(2,1024);
        head.position.setBpm(120);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
        const auto ir=root.file.getChildFile("policy.wav");writeTestWav(ir,1,48000,{1,.1f});
        engine.assignDefaultIR(ir);waitM3([&]{return engine.savedSession().defaultIRName=="policy.wav";});
        const auto tick=[&](int64_t at,bool playing,float scale){
            head.position.setTimeInSamples(at);head.position.setPpqPosition(at/24000.);head.position.setIsPlaying(playing);buffer.clear();
            if(playing)for(int n=0;n<1024;++n){const auto local=(at+n)%65536;
                if(local>=4000 && local<10000){const auto v=static_cast<float>(scale*.2*std::exp(-(local-4000)/1500.)*std::sin(local*.17));buffer.setSample(0,n,v);buffer.setSample(1,n,v*.7f);}}
            const auto dry=buffer;
            {AudioAllocationGuard guard;processor->processBlock(buffer,midi);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
            for(int c=0;c<2;++c)for(int n=0;n<1024;++n)REQUIRE(buffer.getSample(c,n)==dry.getSample(c,n));
        };
        const auto pass=[&](bool sparse,float scale){
            tick(0,false,0);for(int n=0;n<32;++n){tick(n*1024,true,scale);juce::Thread::sleep(1);}
            if(sparse)for(int n=0;n<32;++n){tick(65536+n*1024,true,scale);juce::Thread::sleep(1);}
            tick(sparse?98304:32768,false,0);REQUIRE(processor->flushCaptureForTesting());
        };
        pass(false,1);
        waitM3([&]{const auto s=engine.savedSession();return !s.swells.empty() && std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
        const auto initial=engine.savedSession();const auto before=engine.snapshot();
        REQUIRE(processor->requestWorkflowMode(workflow::Mode::capture));
        engine.setPreferences(0,.5f,.08,true); // Legacy refresh must not defeat Off.
        engine.setLearningAutomatic(automatic);
        REQUIRE(processor->setRecaptureExisting(replace));
        pass(true,.4f);
        // Capture flush completes storage; the accepted-mode handoff publishes
        // asynchronously. Still require the actual Live transition, not a sleep.
        waitM3([&]{return processor->getWorkflowSnapshot().mode==workflow::Mode::live;});
        REQUIRE(processor->getWorkflowSnapshot().mode==workflow::Mode::live);
        if(automatic)waitM3([&]{const auto s=engine.savedSession();return s.swells.size()>initial.swells.size() && std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
        else{
            juce::Thread::sleep(200);const auto after=engine.snapshot();
            REQUIRE(after.analysisCount==before.analysisCount && after.renderCount==before.renderCount);
            REQUIRE(after.session.swells.size()==initial.swells.size());
            const auto* w=m3::find(after.session,initial.swells[0].id);REQUIRE(w && w->render==initial.swells[0].render);
            if(replace)REQUIRE(w->phase==m3::Phase::stale);
            else REQUIRE(w->phase==m3::Phase::ready);
            if(replace){
                engine.recaptureCompleted(0,32768);juce::Thread::sleep(80);
                REQUIRE(engine.snapshot().renderCount==before.renderCount);
                auto saved=engine.savedSession();engine.restore(saved,processor->getCaptureSnapshot().reference);
                waitM3([&]{return m3::find(engine.savedSession(),initial.swells[0].id)->phase==m3::Phase::stale;});
                REQUIRE(engine.snapshot().renderCount==before.renderCount);
            }
            REQUIRE(processor->requestWorkflowMode(workflow::Mode::capture));
            engine.setLearningAutomatic(true); // Future defaults do not backfill Off.
            REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));juce::Thread::sleep(100);
            REQUIRE(engine.snapshot().analysisCount==before.analysisCount && engine.snapshot().renderCount==before.renderCount);
            // Merely entering Perform does no disabled-pass work; an explicit
            // Generate is still allowed, including changed dry-only source.
            REQUIRE(engine.selectTargets({w->id},w->id));engine.generate(false);
            waitM3([&]{return m3::find(engine.savedSession(),initial.swells[0].id)->phase==m3::Phase::ready;});
            REQUIRE(engine.snapshot().analysisCount==before.analysisCount);
            REQUIRE(engine.snapshot().renderCount==before.renderCount+1);
            REQUIRE(engine.addTarget(65536+14000));const auto manual=engine.savedSession().primary;
            waitM3([&]{return m3::find(engine.savedSession(),manual)->phase==m3::Phase::ready;});
            const auto ready=engine.snapshot().renderCount;
            REQUIRE(engine.edit(manual,[](auto& w,auto&){--w.sourceEnd;return true;}));
            waitM3([&]{return m3::find(engine.savedSession(),manual)->phase==m3::Phase::ready && engine.snapshot().renderCount>ready;});
            REQUIRE(engine.snapshot().analysisCount==before.analysisCount);
        }
        const auto map=processor->getCaptureSnapshot().manifest;
        CaptureAssetStore assets(engine.assetDirectory());SourceInterval original,newAudio;juce::String error;
        REQUIRE(readSourceInterval(assets,map,4096,5120,1024,original,error));
        REQUIRE(readSourceInterval(assets,map,65536+4096,65536+5120,1024,newAudio,error));
        const auto scale=replace?1.f:2.5f;
        REQUIRE(std::abs(original.samples[100]-newAudio.samples[100]*scale)<1e-6f);
        REQUIRE(captureContextAt(map,65536)->automatic==automatic);
    }
}
