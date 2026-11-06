#pragma once
#include <DSP/ExportFiles.h>
#include <Core/ContentHash.h>

inline void levelAudioPathsCases()
{
    using namespace m3;
    TestDirectory root;auto p=std::make_unique<PreverbAudioProcessor>(root.file);auto& engine=p->swellEngine();
    engine.setAutomaticProcessing(false);p->prepareToPlay(1000,1024);TestPlayHead head;p->setPlayHead(&head);
    head.position.setBpm(120);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    const auto position=[&](int64_t at,bool playing){head.position.setIsPlaying(playing);head.position.setTimeInSamples(at);head.position.setPpqPosition(at/500.);};
    juce::AudioBuffer<float> block(2,1000);juce::MidiBuffer midi;
    position(0,true);for(int n=0;n<1000;++n){block.setSample(0,n,.01f);block.setSample(1,n,-.005f);}
    p->processBlock(block,midi);REQUIRE(p->flushCaptureForTesting());finishCaptureFixture(*p,head.position);
    position(0,false);block.clear();p->processBlock(block,midi);
    REQUIRE(engine.addTarget(500));const auto id=engine.savedSession().primary;
    const auto ir=root.file.getChildFile("level-paths.wav");writeTestWav(ir,1,1000,std::vector<float>(101,.01f));engine.loadIR(ir);
    waitM3([&]{return find(engine.savedSession(),id)->irName==ir.getFileName();});engine.generate(true);
    // Select the admitted wet-only window before waiting: the fresh-instance
    // Dry + Wet audition context extends beyond this deliberately short capture.
    REQUIRE(engine.setAuditionWetOnly(true,engine.assignmentEpoch()));
    waitM3([&]{return engine.snapshot().auditionReady && engine.snapshot().liveReady;});
    REQUIRE(engine.edit(id,[](auto& w,auto&){w.shape.endOffset=0;w.shape.fadeIn=w.shape.fadeOut=0;return true;}));
    engine.setIsolateSelected(false);engine.setLivePlayback(true,LiveRouting::wetOnly,0);
    REQUIRE(engine.setAuditionWetOnly(true,engine.assignmentEpoch()));
    waitM3([&]{return engine.snapshot().auditionReady && engine.snapshot().liveReady;});
    const auto original=*find(engine.savedSession(),id);const auto directory=engine.assetDirectory();
    const auto rawHash=fileHash(directory.getChildFile(original.render.path)),irHash=fileHash(directory.getChildFile(original.ir.path));
    const auto capture=p->getCaptureSnapshot();const auto renders=engine.snapshot().renderCount;
    REQUIRE(original.calibration.state==CalibrationState::matched && original.calibration.baselineDb!=0);
    const auto tick=[&]{position(0,false);block.setSize(2,1);block.clear();p->processBlock(block,midi);};
    SongMixMetrics deliveredMetrics;
    const auto readWav=[&]
    {
        REQUIRE(p->requestWorkflowMode(workflow::Mode::finalise));
        REQUIRE(engine.setExportLevels(0,1,engine.assignmentEpoch()));
        waitM3([&]{const auto s=engine.snapshot();return s.songExport.phase==ExportPhase::ready && s.songExport.prepared
            && s.songExport.prepared->plan->identity==s.session.exportState.desiredIdentity;});
        const auto prepared=engine.snapshot().songExport.prepared;REQUIRE(prepared);
        deliveredMetrics=prepared->metrics;juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(directory.getChildFile(prepared->file.path).createInputStream().release(),true));
        REQUIRE(reader && reader->usesFloatingPointData && reader->sampleRate==1000);
        std::array<float,64> result{};juce::AudioBuffer<float> wav(2,32);
        REQUIRE(reader->read(&wav,0,32,450-prepared->file.origin,true,true));
        for(int n=0;n<32;++n)for(int c=0;c<2;++c)result[n*2+c]=wav.getSample(c,n);
        REQUIRE(p->requestWorkflowMode(workflow::Mode::live));return result;
    };
    for(const auto db:{-60.f,-30.f,-12.f,0.f,12.f})
    {
        p->setNonRealtime(false);tick();REQUIRE(engine.edit(id,[&](auto& w,auto&){w.shape.gainDb=db;return true;}));
        waitM3([&]{return engine.snapshot().auditionReady && engine.snapshot().liveReady;});
        REQUIRE(find(engine.savedSession(),id)->calibration==original.calibration && engine.snapshot().liveAttenuationDb==0);
        REQUIRE(engine.audition());
        float left=0,right=0;engine.processAudio(&left,&right,1,p->getSongTime());
        const auto cursor=engine.snapshot().auditionSample;REQUIRE(cursor && *cursor<=450);
        const auto skip=static_cast<int>(450-*cursor);std::vector<float> discardL(skip),discardR(skip);
        if(skip)engine.processAudio(discardL.data(),discardR.data(),skip,p->getSongTime());
        std::array<float,32> auditionL{},auditionR{};engine.processAudio(auditionL.data(),auditionR.data(),32,p->getSongTime());engine.stop();
        position(450,true);block.setSize(2,32);block.clear();
        {AudioAllocationGuard guard;p->processBlock(block,midi);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        std::array<float,64> live{};for(int n=0;n<32;++n)for(int c=0;c<2;++c)live[n*2+c]=block.getSample(c,n);
        engine.setLivePlayback(true,LiveRouting::dryWet,0);waitM3([&]{return engine.snapshot().liveReady;});
        position(450,true);for(int n=0;n<32;++n){block.setSample(0,n,.125f);block.setSample(1,n,-.25f);}p->processBlock(block,midi);
        for(int n=10;n<32;++n)for(int c=0;c<2;++c)
        {
            const auto wet=(51+n)*.01*(c==0?.01:-.005)*std::pow(10.,(original.calibration.baselineDb+db)/20.);
            REQUIRE(std::abs(block.getSample(c,n)-((c==0?.125:-.25)+wet))<1.e-7);
        }
        engine.setLivePlayback(true,LiveRouting::wetOnly,0);waitM3([&]{return engine.snapshot().liveReady;});
        tick();p->prepareToPlay(1000,1024);p->setNonRealtime(true);position(450,false);block.setSize(2,32);block.clear();p->processBlock(block,midi);
        REQUIRE(!engine.snapshot().liveStatus.startsWith("Export incomplete"));
        std::array<float,64> offline{};for(int n=0;n<32;++n)for(int c=0;c<2;++c)offline[n*2+c]=block.getSample(c,n);
        p->setNonRealtime(false);tick();const auto wav=readWav();
        // Independent box-convolution + saved calibration + requested dB oracle.
        // Sample 450 is 50 frames into the 101-frame IR lead-in. Ignore joins.
        for(int n=10;n<32;++n)for(int c=0;c<2;++c)
        {
            const auto expected=(51+n)*.01*(c==0?.01:-.005)*std::pow(10.,(original.calibration.baselineDb+db)/20.);
            const auto tolerance=std::max(1.e-9,std::abs(expected)*2.e-5);
            REQUIRE(std::abs(live[n*2+c]-expected)<tolerance && std::abs(offline[n*2+c]-expected)<tolerance
                && std::abs(wav[n*2+c]-expected)<tolerance && std::abs((c==0?auditionL[n]:auditionR[n])-expected)<tolerance);
        }
        REQUIRE(engine.snapshot().renderCount==renders && find(engine.savedSession(),id)->render==original.render);
        REQUIRE(fileHash(directory.getChildFile(original.render.path))==rawHash && fileHash(directory.getChildFile(original.ir.path))==irHash);
        REQUIRE(p->getCaptureSnapshot().reference==capture.reference);
    }
    // Real correlated overlapping members require one disclosed wet guard.
    auto hot=engine.savedSession();const auto member=*find(hot,id);
    for(int n=1;n<32;++n){auto copy=member;copy.id=juce::Uuid().toDashedString();hot.swells.push_back(copy);}
    engine.restore(hot);tick();waitM3([&]{return engine.snapshot().auditionReady && engine.snapshot().liveReady;});
    REQUIRE(engine.snapshot().liveAttenuationDb<0 && engine.snapshot().attenuationDb<0);
    position(450,true);block.setSize(2,32);block.clear();p->processBlock(block,midi);
    REQUIRE(block.getMagnitude(10,22)>0 && block.getMagnitude(10,22)<=juce::Decibels::decibelsToGain(candidateWetCeilingDb)+1.e-6f);
    // Plugin float WAV intentionally preserves aggregate overs rather than
    // inheriting audition/live protection; keep that existing path policy.
    tick();const auto hotWav=readWav();
    REQUIRE(deliveredMetrics.wetGain==1 && deliveredMetrics.wetPeak>juce::Decibels::decibelsToGain(candidateWetCeilingDb));
    for(const auto value:hotWav)REQUIRE(std::isfinite(value));
    REQUIRE(engine.snapshot().renderCount==renders && fileHash(directory.getChildFile(original.render.path))==rawHash);
}
