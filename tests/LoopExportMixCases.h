#pragma once

inline void loopExportMixCases()
{
    using namespace m3;
    TestDirectory directory;CaptureAssetStore captures(directory.file);SwellAssetStore assets(directory.file);juce::String error;
    Session session;session.timing={true,0,0,1000,120,4,4};
    CaptureManifest source;source.schemaVersion=3;source.sampleRate=1000;source.revision=1;
    source.projectId=juce::Uuid().toDashedString().toStdString();source.instanceId=juce::Uuid().toDashedString().toStdString();
    source.loop=LoopPhraseTiming{120,4,4,4,713,2000};
    CaptureManifestRange range{0,2000,CaptureCoverageKind::audio,1,"audio/loop.wav",{}};
    std::vector<float> dry(4000);
    for(int frame=0;frame<2000;++frame){dry[frame*2]=.1f+static_cast<float>(frame%31)*.01f;dry[frame*2+1]=-.2f;}
    REQUIRE(captures.writeAudioChunk(range,1000,dry.data(),error));source.ranges={range};
    CaptureGenerationContext context;context.origin=CaptureContextOrigin::legacy;context.id=captureGenerationContextId(context);
    source.generationContexts={context};source.contextSpans={{0,2000,context.id}};
    WetRender raw;raw.start=-3000;raw.sourceStart=0;raw.sourceEnd=3500;raw.rate=1000;raw.samples.resize(13000);
    for(size_t frame=0;frame<6500;++frame){raw.samples[frame*2]=.4f+static_cast<float>(frame%29)*.01f;raw.samples[frame*2+1]=-.3f;}
    Swell swell;swell.arrival=swell.sourceStart=0;swell.sourceEnd=2000;swell.supportOffset=1500;swell.shape.endOffset=3500;swell.phase=Phase::ready;
    ImpulseResponse impulse;impulse.rate=1000;impulse.channels=1;impulse.samples={1};
    REQUIRE(assets.writeIR(impulse,swell.ir,error));session.gainPolicy=GainPolicy::musical;session.gainRevision=candidateGainRevision;swell.shape.gainDb=-3;
    REQUIRE(assets.writeRender(raw,swell.ir,"loop-overlap",session.timing,swell.render,error));
    swell.calibration=fixedCalibration(session,swell,6,swell.render.content);session.swells={swell};
    const auto dryHash=fileHash(directory.file.getChildFile(range.assetPath));
    const auto wetHash=fileHash(directory.file.getChildFile(swell.render.path));
    for(int repeats:{1,2,4})for(auto levels:std::array<std::pair<float,float>,4>{{{0,1},{1,0},{1,1},{.35f,.65f}}}){
        std::shared_ptr<const SongExportPlan> plan;
        REQUIRE(prepareLoopExportPlan(session,source,"loop-mix",{repeats,4},levels.first,levels.second,plan,error));
        SongMix mix(directory.file);REQUIRE(mix.prepare(plan,previewMemoryBudget,error) && mix.measure(error));
        REQUIRE(mix.metrics().wetGain==1 && mix.metrics().sumGain==1 && mix.workingBytes()<2*1024*1024);
        std::vector<float> samples(static_cast<size_t>(plan->end-plan->origin)*2);
        for(auto at=plan->origin;at<plan->end;){const auto frames=static_cast<size_t>(std::min(int64_t{127},plan->end-at));
            REQUIRE(mix.read(at,{samples.data()+static_cast<size_t>(at-plan->origin)*2,frames*2},error));at+=static_cast<int64_t>(frames);}
        // Independent finite convolution-style sum: no shared cycle iterator,
        // scheduler positions, or export branch reader is used by this oracle.
        for(auto at=plan->origin;at<plan->end;++at)for(int channel=0;channel<2;++channel){
            float wet=0;
            for(int pass=0;pass<repeats;++pass){const auto index=at-pass*2000-raw.start;
                if(index<0 || index>=6500)continue;
                const auto edge=static_cast<float>(std::min({1.,index/2.,(6499-index)/2.}));
                wet+=static_cast<float>(raw.samples[static_cast<size_t>(index)*2+channel]*edge*std::pow(10.,3./20));
            }
            const auto dryValue=at>=0 && at<repeats*2000?dry[static_cast<size_t>(at%2000)*2+channel]:0;
            const auto expected=dryValue*levels.first+wet*levels.second;
            REQUIRE(std::abs(samples[static_cast<size_t>(at-plan->origin)*2+channel]-expected)<2e-6f);
        }
        REQUIRE(plan->end==(levels.second>0?(repeats-1)*2000+3500:repeats*2000));
        if(repeats==4 && levels.second==1)REQUIRE(mix.metrics().peak>1);
        ExportFile file;REQUIRE(publishSongExport(mix,directory.file,file,error) && verifyExportFile(directory.file,file,error));
        juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(directory.file.getChildFile(file.path).createInputStream().release(),true));
        REQUIRE(reader && reader->lengthInSamples==plan->end-plan->origin);
        juce::AudioBuffer<float> decoded(2,static_cast<int>(reader->lengthInSamples));
        REQUIRE(reader->read(&decoded,0,decoded.getNumSamples(),0,true,true));
        for(int n=0;n<decoded.getNumSamples();++n)for(int c=0;c<2;++c)REQUIRE(std::abs(decoded.getSample(c,n)-samples[static_cast<size_t>(n)*2+c])<2e-6f);
        REQUIRE(fileHash(directory.file.getChildFile(range.assetPath))==dryHash && fileHash(directory.file.getChildFile(swell.render.path))==wetHash);
        if(repeats==1 && levels.first==1 && levels.second==1){
            TestDirectory saveRoot;const auto target=saveRoot.file.getChildFile("kept.wav");REQUIRE(target.replaceWithText("previous bytes"));
            REQUIRE(!saveExportFile(directory.file,file,target,true,error,{},[](auto stage){return stage==ExportFileStage::space;}));
            REQUIRE(target.loadFileAsString()=="previous bytes" && error.contains("space"));
        }
    }
    // A missing dry chunk is warned only over scheduled dry passes, never over
    // deliberately silent lead-in padding or the final wet-only tail.
    REQUIRE(directory.file.getChildFile(range.assetPath).deleteFile());
    std::shared_ptr<const SongExportPlan> plan;
    REQUIRE(prepareLoopExportPlan(session,source,"missing-dry",{4,4},1,1,plan,error));
    SongMix missing(directory.file);REQUIRE(missing.prepare(plan,previewMemoryBudget,error));
    REQUIRE(missing.plan()->warnings.size()==1);
    REQUIRE((missing.plan()->warnings.front()==ExportWarning{0,8000,ExportCoverage::unavailable}));
    REQUIRE(prepareLoopExportPlan(session,source,"missing-dry",{4,4},0,1,plan,error));
    REQUIRE(missing.prepare(plan,previewMemoryBudget,error) && missing.plan()->warnings.empty());
    REQUIRE(missing.measure(error));
    for(auto stage:{ExportFileStage::write,ExportFileStage::shortWrite,ExportFileStage::close,ExportFileStage::verify,ExportFileStage::publish,ExportFileStage::space}){
        ExportFile unpublished;
        REQUIRE(!publishSongExport(missing,directory.file,unpublished,error,{},[&](auto at){return at==stage;}));
        REQUIRE(unpublished.path.isEmpty());
    }
    for(const auto& file:directory.file.getChildFile("exports").findChildFiles(juce::File::findFiles,false))
        REQUIRE(!file.getFileName().startsWith(".unpublished-"));
    // Capture methods converge on the same exact accepted extent. Exercise
    // non-whole-bar (three-beat) and bar captures at fractional sample periods.
    for(double rate:{44100.,48000.})for(int beats:{3,4})for(bool roundUp:{false,true}){
        const auto ideal=rate*60*beats/137.3;
        const auto period=static_cast<int64_t>(roundUp?std::ceil(ideal):std::floor(ideal));
        auto fractional=source;fractional.sampleRate=rate;
        fractional.loop=LoopPhraseTiming{137.3,4,4,beats,987,period};
        CaptureManifestRange pulse{0,static_cast<uint64_t>(period),CaptureCoverageKind::audio,1,"audio/fractional-"+std::to_string(static_cast<int>(rate))+"-"+std::to_string(period)+".wav",{}};
        std::vector<float> pulses(static_cast<size_t>(period)*2);pulses[0]=1;pulses[1]=-.5f;pulses[2]=.25f;
        REQUIRE(captures.writeAudioChunk(pulse,rate,pulses.data(),error));
        fractional.ranges={pulse};fractional.contextSpans={{0,period,context.id}};
        auto fixed=session;fixed.timing={true,0,0,rate,137.3,4,4};
        REQUIRE(prepareLoopExportPlan(fixed,fractional,"fractional",{64,4},1,0,plan,error));
        REQUIRE(plan->end==64*period && plan->origin==0);
        SongMix clocked(directory.file);REQUIRE(clocked.prepare(plan,previewMemoryBudget,error));
        std::array<float,6> sourcePage{},wetPage{};
        for(int cycle=1;cycle<64;++cycle){
            const auto anchor=static_cast<int64_t>(cycle)*period;
            REQUIRE(clocked.readLayers(anchor-1,sourcePage,wetPage,error));
            REQUIRE(sourcePage[0]==0 && sourcePage[1]==0 && sourcePage[2]==1 && sourcePage[3]==-.5f && sourcePage[4]==.25f);
            REQUIRE(std::all_of(wetPage.begin(),wetPage.end(),[](float value){return value==0;}));
        }
    }
}
