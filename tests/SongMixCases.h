#pragma once
#include <DSP/SongMix.h>
#include <DSP/ExportFiles.h>
#include <DSP/GainStaging.h>
#include <Core/ContentHash.h>
inline void songMixCases()
{
    using namespace m3;TestDirectory directory;CaptureAssetStore captures(directory.file);SwellAssetStore assets(directory.file);
    Session session;session.timing={true,0,0,1000,120,4,4};session.gainPolicy=GainPolicy::musical;session.gainRevision=approvedGainRevision;
    CaptureManifest source;source.sampleRate=1000;source.projectId=juce::Uuid().toDashedString().toStdString();source.instanceId=juce::Uuid().toDashedString().toStdString();
    std::vector<float> dry(400);for(int n=0;n<200;++n){dry[n*2]=.2f;dry[n*2+1]=.1f;}
    CaptureManifestRange range{100,200,CaptureCoverageKind::audio,1,"audio/test.wav",{}};juce::String error;
    REQUIRE(captures.writeAudioChunk(range,1000,dry.data(),error));source.ranges={range};
    WetRender raw;raw.start=0;raw.sourceStart=100;raw.sourceEnd=200;raw.rate=1000;raw.samples.resize(400);
    for(int n=0;n<200;++n)for(int c=0;c<2;++c)raw.samples[n*2+c]=static_cast<float>((n<100?n/100.:1.-(n-100)/100.)*(c==0?2.:1.));
    Swell swell;swell.arrival=swell.sourceStart=100;swell.sourceEnd=200;swell.shape.endOffset=100;swell.phase=Phase::ready;
    REQUIRE(assets.writeRender(raw,{},"fixture",session.timing,swell.render,error));session.swells={swell};
    auto neighbour=swell;neighbour.id=juce::Uuid().toDashedString();neighbour.placement=20;session.swells.push_back(neighbour);
    const auto original=fileHash(directory.file.getChildFile(swell.render.path));
    for(const float blend:{0.f,.5f,.65f,1.f}){
        std::shared_ptr<const SongExportPlan> plan;REQUIRE(prepareSongExportPlan(session,source,"source",session.timing,blend,plan,error));
        SongMix mix(directory.file);REQUIRE(mix.prepare(plan,previewMemoryBudget,error) && mix.measure(error));const auto metrics=mix.metrics();
        REQUIRE(metrics.audibleStart && *metrics.audibleStart==(blend==0?100:1));REQUIRE(metrics.audibleEnd && *metrics.audibleEnd==(blend==1?219:300));
        float peak=0;std::vector<float> oracle(600);
        for(int n=0;n<300;++n)for(int c=0;c<2;++c){float wet=0;for(int move:{0,20}){const auto at=n-move;if(at>=0 && at<200){const auto edge=std::min({1.,at/2.,(199-at)/2.});wet+=static_cast<float>(raw.samples[at*2+c]*edge);}}peak=std::max(peak,std::abs(wet));oracle[n*2+c]=wet;}
        const auto guard=peak>juce::Decibels::decibelsToGain(-1.f)?juce::Decibels::decibelsToGain(-1.f)/peak:1.f;
        REQUIRE(std::abs(metrics.wetGain-(blend==0?1:guard))<1e-6f);
        for(int n=0;n<300;++n)for(int c=0;c<2;++c)oracle[n*2+c]=(n>=100?dry[(n-100)*2+c]*(1-blend):0)+oracle[n*2+c]*guard*blend;
        for(int pageSize:{1,13,127,4096}){std::vector<float> actual(600);for(int at=0;at<300;){const auto count=std::min(pageSize,300-at);REQUIRE(mix.read(at,{actual.data()+at*2,static_cast<size_t>(count)*2},error));at+=count;}
            for(size_t i=0;i<actual.size();++i)REQUIRE(std::abs(actual[i]-oracle[i]*metrics.sumGain)<2e-6f);}
        auto detailReader=std::make_unique<SongMix>(directory.file);REQUIRE(detailReader->prepare(plan,previewMemoryBudget,error));
        REQUIRE(detailReader->adoptMeasurements(metrics,mix.plan()->identity,error));SongMixDetail detail;
        REQUIRE(detailReader->summarize(95,130,35,detail,error) && detail.dry.size()==35 && detail.wet.size()==35);
        for(int n=95;n<130;++n){const auto i=static_cast<size_t>(n-95);const auto expectedDry=n>=100?.2f*(1-blend)*metrics.sumGain:0;
            REQUIRE(std::abs(detail.dry[i].maximum-expectedDry)<1e-6f);
            REQUIRE(std::abs(detail.dry[i].maximum+detail.wet[i].maximum-oracle[n*2]*metrics.sumGain)<2e-6f);}
        const auto priorDetail=detail;REQUIRE(!detailReader->summarize(95,130,35,detail,error,[]{return true;}) && detail.composition==priorDetail.composition);
        REQUIRE(metrics.dryPeaks.size()<=4096 && mix.workingBytes()<previewMemoryBudget);
        REQUIRE(fileHash(directory.file.getChildFile(swell.render.path))==original);
        ExportFile file;REQUIRE(publishSongExport(mix,directory.file,file,error));REQUIRE(file.frames==300 && file.origin==0 && file.wet==blend && verifyExportFile(directory.file,file,error));
        juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> decoded(format.createReaderFor(directory.file.getChildFile(file.path).createInputStream().release(),true));
        REQUIRE(decoded && decoded->usesFloatingPointData && decoded->numChannels==2 && decoded->sampleRate==1000 && decoded->lengthInSamples==300);
        juce::AudioBuffer<float> data(2,300);REQUIRE(decoded->read(&data,0,300,0,true,true));for(int n=0;n<300;++n)for(int c=0;c<2;++c)REQUIRE(std::abs(data.getSample(c,n)-oracle[n*2+c]*metrics.sumGain)<2e-6f);
        const auto prior=file;
        REQUIRE(!saveExportFile(directory.file,file,directory.file.getChildFile(swell.render.path),true,error) && fileHash(directory.file.getChildFile(swell.render.path))==original);
        for(const auto failure:{ExportFileStage::write,ExportFileStage::shortWrite,ExportFileStage::close,ExportFileStage::verify,ExportFileStage::publish}){
            REQUIRE(!publishSongExport(mix,directory.file,file,error,{},[&](auto stage){return stage==failure;}) && file==prior);
            REQUIRE(verifyExportFile(directory.file,prior,error));}
        REQUIRE(!publishSongExport(mix,directory.file,file,error,[]{return true;}) && file==prior);
        TestDirectory external;REQUIRE(external.file.createDirectory().wasOk());const auto target=external.file.getChildFile("saved.wav");REQUIRE(target.replaceWithText("previous bytes"));
        REQUIRE(!saveExportFile(directory.file,file,target,false,error) && target.loadFileAsString()=="previous bytes");
        REQUIRE(!saveExportFile(directory.file,file,target,true,error,{},[](auto stage){return stage==ExportFileStage::publish;}) && target.loadFileAsString()=="previous bytes");
        REQUIRE(!saveExportFile(directory.file,file,target,true,error,{}, {},[](const auto&){return false;}) && target.loadFileAsString()=="previous bytes");
        REQUIRE(saveExportFile(directory.file,file,target,true,error) && fileHash(target)==file.checksum && verifyExportFile(directory.file,file,error));
        for(const auto& f:directory.file.getChildFile("exports").findChildFiles(juce::File::findFiles,false))REQUIRE(!f.getFileName().startsWith(".unpublished-"));

    }
    {
        auto hotSource=source;auto hotRange=range;hotRange.assetPath="audio/hot.wav";std::vector<float> hot(400,4.f);
        REQUIRE(captures.writeAudioChunk(hotRange,1000,hot.data(),error));hotSource.ranges={hotRange};
        std::shared_ptr<const SongExportPlan> input;REQUIRE(prepareSongExportPlan(session,hotSource,"hot",session.timing,.5f,input,error));
        SongMix mixed(directory.file);REQUIRE(mixed.prepare(input,previewMemoryBudget,error) && mixed.measure(error));
        REQUIRE(mixed.metrics().sumGain<1 && mixed.metrics().peak<=juce::Decibels::decibelsToGain(-1.f)+1e-6f);
        std::array<float,2> sample;REQUIRE(mixed.read(150,sample,error));
        float wet=raw.samples[150*2]+raw.samples[130*2];
        REQUIRE(std::abs(sample[0]-(2.f+wet*mixed.metrics().wetGain*.5f)*mixed.metrics().sumGain)<1e-6f);
        REQUIRE(!mixed.measure(error,[]{return true;}));REQUIRE(!mixed.read(0,sample,error));
    }
    {
        auto negative=raw;for(auto& value:negative.samples)value=-value;Asset asset;
        REQUIRE(assets.writeRender(negative,{},"negative",session.timing,asset,error));
        auto cancellation=session;cancellation.swells[1].placement=0;cancellation.swells[1].render=asset;
        std::shared_ptr<const SongExportPlan> input;REQUIRE(prepareSongExportPlan(cancellation,source,"source",session.timing,1,input,error));
        SongMix silence(directory.file);REQUIRE(silence.prepare(input,previewMemoryBudget,error) && silence.measure(error));
        REQUIRE(!silence.metrics().audibleStart && !silence.metrics().audibleEnd && silence.metrics().peak==0 && silence.metrics().wetGain==1);
        std::vector<float> samples(600);REQUIRE(silence.read(0,samples,error) && std::all_of(samples.begin(),samples.end(),[](float v){return v==0;}));
        ExportFile silent;REQUIRE(publishSongExport(silence,directory.file,silent,error) && silent.frames==300);
    }
    {
        ImpulseResponse impulse;impulse.rate=1000;impulse.channels=1;impulse.samples={1};Asset ir;REQUIRE(assets.writeIR(impulse,ir,error));
        auto low=raw;for(auto& value:low.samples)value*=.01f;Asset rendered;
        REQUIRE(assets.writeRender(low,ir,"fixed-source",session.timing,rendered,error));
        auto calibrated=session;calibrated.gainRevision=candidateGainRevision;calibrated.swells.resize(1);
        auto& member=calibrated.swells[0];member.ir=ir;member.render=rendered;member.shape.gainDb=6;member.shape.fadeIn=10;member.shape.fadeOut=13;
        member.calibration=fixedCalibration(calibrated,member,6,rendered.content);
        std::shared_ptr<const SongExportPlan> input;REQUIRE(prepareSongExportPlan(calibrated,source,"source",session.timing,1,input,error));
        SongMix shaped(directory.file);REQUIRE(shaped.prepare(input,previewMemoryBudget,error) && shaped.measure(error));
        std::array<float,2> values;REQUIRE(shaped.read(50,values,error));
        REQUIRE(shaped.metrics().wetGain==1 && shaped.metrics().sumGain==1 && std::abs(values[0]-low.samples[100]*std::pow(10.f,12.f/20))<1e-6f);
        REQUIRE(shaped.read(0,values,error) && values[0]==0 && shaped.read(199,values,error) && values[0]==0);
    }
    for(const int64_t origin:{int64_t{0},int64_t{500}}){
        TestDirectory shiftedRoot;CaptureAssetStore shiftedCapture(shiftedRoot.file);SwellAssetStore shiftedAssets(shiftedRoot.file);
        auto shiftedRaw=raw;shiftedRaw.start+=origin;shiftedRaw.sourceStart+=origin;shiftedRaw.sourceEnd+=origin;
        shiftedRaw.samples[396]=shiftedRaw.samples[397]=1e-12f;
        auto shiftedSession=session;shiftedSession.swells.resize(1);shiftedSession.timing.sample=origin;auto& member=shiftedSession.swells[0];
        member.arrival+=origin;member.sourceStart+=origin;member.sourceEnd+=origin;
        REQUIRE(shiftedAssets.writeRender(shiftedRaw,{},"shifted",shiftedSession.timing,member.render,error));
        auto shiftedSource=source;auto r=range;r.startSample+=origin;REQUIRE(shiftedCapture.writeAudioChunk(r,1000,dry.data(),error));shiftedSource.ranges={r};
        std::shared_ptr<const SongExportPlan> input;REQUIRE(prepareSongExportPlan(shiftedSession,shiftedSource,"shifted",shiftedSession.timing,1,input,error));
        REQUIRE(input->origin==origin && input->end==origin+300);
        SongMix shifted(shiftedRoot.file);REQUIRE(shifted.prepare(input,previewMemoryBudget,error) && shifted.measure(error));
        REQUIRE(shifted.metrics().audibleStart==origin+1 && shifted.metrics().audibleEnd==origin+199);
        ExportFile file;REQUIRE(publishSongExport(shifted,shiftedRoot.file,file,error) && file.origin==origin && file.frames==300);
        member.placement=-150;REQUIRE(prepareSongExportPlan(shiftedSession,shiftedSource,"shifted",shiftedSession.timing,1,input,error));
        SongMix negativeLead(shiftedRoot.file);REQUIRE(negativeLead.prepare(input,previewMemoryBudget,error) && negativeLead.measure(error));
        REQUIRE(negativeLead.metrics().audibleStart==origin && negativeLead.metrics().audibleEnd==origin+49);
        std::array<float,2> sample;REQUIRE(negativeLead.read(origin,sample,error) && sample[0]!=0);
    }
    {
        // Bar 9 at 120 BPM / 1000 Hz: preserve sixteen seconds of file lead-in.
        auto late=session;late.swells.resize(1);late.swells[0].placement=16000;
        auto sparse=source;auto r=range;r.startSample+=16000;r.assetPath="audio/late.wav";
        REQUIRE(captures.writeAudioChunk(r,1000,dry.data(),error));sparse.ranges={r};
        std::shared_ptr<const SongExportPlan> input;REQUIRE(prepareSongExportPlan(late,sparse,"bar-9",late.timing,.5f,input,error));
        SongMix mix(directory.file);REQUIRE(mix.prepare(input,previewMemoryBudget,error) && mix.measure(error));
        REQUIRE(input->origin==0 && input->end==16300 && mix.metrics().audibleStart==16001);
        ExportFile file;REQUIRE(publishSongExport(mix,directory.file,file,error) && file.frames==16300);
        juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(directory.file.getChildFile(file.path).createInputStream().release(),true));
        juce::AudioBuffer<float> decoded(2,16300);REQUIRE(reader && reader->read(&decoded,0,16300,0,true,true));
        for(int at=0;at<16300;++at){std::array<float,2> expected;REQUIRE(mix.read(at,expected,error));
            for(int c=0;c<2;++c)REQUIRE(decoded.getSample(c,at)==expected[c]);}
        auto wrong=late.timing;wrong.rate=48000;REQUIRE(!prepareSongExportPlan(late,sparse,"bad-rate",wrong,.5f,input,error));
        wrong=late.timing;wrong.valid=false;REQUIRE(!prepareSongExportPlan(late,sparse,"no-time",wrong,.5f,input,error));
    }
    {
        auto maximum=session;maximum.swells.clear();
        for(size_t i=0;i<maximumSwells;++i){auto member=swell;member.id=juce::Uuid().toDashedString();maximum.swells.push_back(member);}
        std::shared_ptr<const SongExportPlan> input;REQUIRE(prepareSongExportPlan(maximum,source,"maximum",maximum.timing,1,input,error));
        SongMix overlap(directory.file);REQUIRE(overlap.prepare(input,128*1024*1024,error) && overlap.measure(error));
        REQUIRE(overlap.metrics().wetPeak>100 && overlap.metrics().peak<=juce::Decibels::decibelsToGain(-1.f)+1e-6f && overlap.workingBytes()<128*1024*1024);
        auto sparse=session;sparse.swells.resize(1);sparse.swells[0].placement=3600000;auto absent=source;absent.ranges.clear();
        REQUIRE(prepareSongExportPlan(sparse,absent,"hour-sparse",sparse.timing,1,input,error));
        SongMix hour(directory.file);REQUIRE(hour.prepare(input,8*1024*1024,error) && hour.measure(error));
        REQUIRE(input->end==3600200 && hour.metrics().audibleStart==3600001 && hour.workingBytes()<1024*1024);
        std::array<float,8192> zeros;REQUIRE(hour.read(0,zeros,error) && std::all_of(zeros.begin(),zeros.end(),[](float x){return x==0;}));
        std::cerr<<"Export tracked storage: maximum overlap "<<overlap.workingBytes()<<", hour-sparse "<<hour.workingBytes()<<" bytes\n";
    }
    {
        auto huge=source;huge.ranges.push_back({INT64_MAX-10,10,CaptureCoverageKind::silence});
        std::shared_ptr<const SongExportPlan> input;REQUIRE(!prepareSongExportPlan(session,huge,"huge",session.timing,1,input,error));
    }
    std::shared_ptr<const SongExportPlan> plan;REQUIRE(prepareSongExportPlan(session,source,"source",session.timing,1,plan,error));
    SongMix cancelled(directory.file);REQUIRE(!cancelled.prepare(plan,previewMemoryBudget,error,[]{return true;}));
    REQUIRE(!cancelled.prepare(plan,8,error));
    SongMix unreadable(directory.file);REQUIRE(directory.file.getChildFile(swell.render.path).deleteFile());
    REQUIRE(!unreadable.prepare(plan,previewMemoryBudget,error));
}
