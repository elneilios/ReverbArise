#include "TestSupport.h"
#include <DSP/SongExport.h>
#include <DSP/ExportSource.h>
#include <Capture/SourceRangeReader.h>
#include <limits>
#include "SongMixCases.h"
#include "ExportPresetListening.h"
#include <UI/Controls/ExternalFileDrag.h>
using namespace m3;
int main(int argc,char** argv){return runTest([&]{
    if(argc==3 && juce::String(argv[1])=="--render-preset-listening"){renderExportPresetListening(juce::File(argv[2]));return;}
    songMixCases();
    {
        juce::String error;
        ExportState pair;pair.initialized=true;pair.dry=.87321f;pair.wet=.45678f;
        ExportFile old;old.path="exports/Swells-65pct-wet-old.wav";
        old.composition=juce::String::repeatedString("a",64);old.checksum=juce::String::repeatedString("b",64);
        old.rate=48000;old.frames=96000;old.wet=.65f;old.dry=.35f;old.gainLaw=1;
        pair.history.push_back(old);pair.completedPath=old.path;pair.desiredIdentity=old.composition;
        ExportState restored;const auto encoded=exportStateMetadata(pair);
        Session levels;levels.exportState=pair;const auto originalLevels=levels.exportState;
        REQUIRE(!setExportLevels(levels,-.1f,.5f,error) && levels.exportState==originalLevels);
        REQUIRE(setExportLevels(levels,1,1,error) && levels.exportState.dry==1 && levels.exportState.wet==1 && levels.exportState.desiredIdentity.isEmpty());
        const auto unchanged=levels.exportState;REQUIRE(setExportLevels(levels,1,1,error) && levels.exportState==unchanged);
        REQUIRE(parseExportState(encoded,restored,error) && restored==pair);
        auto legacy=exportStateMetadata(pair);legacy.getDynamicObject()->removeProperty("version");
        legacy.getDynamicObject()->removeProperty("dry");
        for(auto& item:*legacy["history"].getArray()){item.getDynamicObject()->removeProperty("dry");item.getDynamicObject()->removeProperty("gainLaw");}
        REQUIRE(parseExportState(legacy,restored,error));
        REQUIRE(restored.dry==0 && restored.wet==1 && restored.desiredIdentity.isEmpty() && restored.completedPath.isEmpty());
        REQUIRE(restored.history.size()==1 && restored.history[0].path==old.path && restored.history[0].checksum==old.checksum && restored.history[0].gainLaw==1);
        for(int fault=0;fault<5;++fault){auto invalid=exportStateMetadata(pair);
            if(fault==0)invalid.getDynamicObject()->setProperty("dry",1.0000000001);
            if(fault==1)invalid.getDynamicObject()->setProperty("dry","0.5");
            if(fault==2)invalid.getDynamicObject()->setProperty("version",3);
            if(fault==3)invalid["history"][0].getDynamicObject()->setProperty("gainLaw",2.5);
            if(fault==4)invalid["history"][0].getDynamicObject()->setProperty("dry",-1);
            const auto before=restored;REQUIRE(!parseExportState(invalid,restored,error) && restored==before);
        }
    }
    {
        using namespace m3::ui::controls;unsigned invoked=0;
        const auto accept=[&](const juce::String& path){++invoked;REQUIRE(path=="C:/verified.wav");return true;};
        REQUIRE(invokeExternalFileDrag({},true,true,accept)==ExternalDragResult::notReady);
        REQUIRE(invokeExternalFileDrag("C:/verified.wav",false,true,accept)==ExternalDragResult::notReady);
        REQUIRE(invokeExternalFileDrag("C:/verified.wav",true,false,accept)==ExternalDragResult::released && invoked==0);
        REQUIRE(invokeExternalFileDrag("C:/verified.wav",true,true,accept)==ExternalDragResult::started && invoked==1);
        REQUIRE(invokeExternalFileDrag("C:/verified.wav",true,true,[](const auto&){return false;})==ExternalDragResult::unsupported);
    }

    {
        Session old;old.auditionMode=AuditionMode::dryWet;old.wetBlend=.65f;juce::String encoded,error;
        REQUIRE(encode(old,encoded,error));auto legacy=juce::JSON::parse(encoded);legacy.getDynamicObject()->setProperty("version",9);legacy.getDynamicObject()->removeProperty("export");
        Session restored;REQUIRE(decode(juce::JSON::toString(legacy),restored,error) && !restored.exportState.initialized);
        REQUIRE(initializeExport(restored,error) && restored.exportState.dry==0 && restored.exportState.wet==1);
        REQUIRE(setExportMix(restored,1,error) && restored.wetBlend==.65f);REQUIRE(initializeExport(restored,error) && restored.exportState.wet==1);
        old.auditionMode=AuditionMode::wetOnly;REQUIRE(initializeExport(old,error) && old.exportState.wet==1 && old.wetBlend==.65f);
        ExportFile file;file.path="exports/Swells-65pct-wet-aaaa.wav";file.composition=juce::String::repeatedString("a",64);file.checksum=juce::String::repeatedString("b",64);
        file.rate=48000;file.frames=96000;file.wet=.65f;restored.exportState.history.push_back(file);restored.exportState.completedPath=file.path;
        restored.exportState.desiredIdentity=juce::String::repeatedString("c",64);REQUIRE(encode(restored,encoded,error));
        Session roundtrip;REQUIRE(decode(encoded,roundtrip,error) && roundtrip.exportState==restored.exportState);
        for(int change=0;change<7;++change){auto invalid=juce::JSON::parse(encoded);auto ex=invalid["export"];auto history=ex["history"];
            if(change==0)ex.getDynamicObject()->setProperty("wet",1.0000000001);
            if(change==1)ex.getDynamicObject()->setProperty("wet","0.5");
            if(change==2)history[0].getDynamicObject()->setProperty("path","exports/../outside.wav");
            if(change==3)history[0].getDynamicObject()->setProperty("frames","9223372036854775808");
            if(change==4)history[0].getDynamicObject()->setProperty("checksum","not-a-hash");
            if(change==5)ex.getDynamicObject()->setProperty("completedPath","exports/absent.wav");
            if(change==6)ex.getDynamicObject()->setProperty("initialized",false);
            const auto before=roundtrip.exportState;REQUIRE(!decode(juce::JSON::toString(invalid),roundtrip,error) && roundtrip.exportState==before);
        }
        auto capped=restored.exportState;capped.history.resize(maximumExportRevisions+1);REQUIRE(!validExportState(capped,error));
        const auto before=restored.exportState;REQUIRE(!setExportMix(restored,std::numeric_limits<float>::quiet_NaN(),error) && restored.exportState==before);
    }
    Session session;session.timing={true,0,0,48000,120,4,4};
    CaptureManifest source;source.projectId=juce::Uuid().toDashedString().toStdString();source.instanceId=juce::Uuid().toDashedString().toStdString();
    Swell w;w.name="First";w.arrival=48000;w.sourceStart=48000;w.sourceEnd=96000;w.shape.endOffset=0;
    w.render.path="renders/aaaa.wav";w.render.content=juce::String::repeatedString("a",64);w.render.checksum=juce::String::repeatedString("b",64);
    w.render.start=0;w.render.frames=96000;w.render.sourceStart=48000;w.render.sourceEnd=96000;w.render.channels=2;w.render.rate=48000;w.render.bpm=120;
    w.phase=Phase::ready;session.swells.push_back(w);
    std::shared_ptr<const SongExportPlan> first,other;juce::String error;
    REQUIRE(prepareSongExportPlan(session,source,"source-v1",session.timing,.65f,first,error));
    REQUIRE(prepareSongExportPlan(session,source,"source-v1",session.timing,1,.65f,other,error));
    REQUIRE(other->dry==1 && other->wet==.65f && other->identity!=first->identity);
    REQUIRE(first->origin==0 && first->end==48000 && first->members.size()==1 && first->identity.length()==64);
    session.selected={w.id};session.primary=w.id;session.isolateSelected=true;session.isolateDry=true;session.auditionDb=-24;
    session.liveWetDb=-10;session.auditionMode=AuditionMode::dryWet;session.wetBlend=.2f;
    REQUIRE(prepareSongExportPlan(session,source,"source-v1",session.timing,.65f,other,error) && other->identity==first->identity);
    session.swells[0].name="Renamed";
    REQUIRE(prepareSongExportPlan(session,source,"source-v1",session.timing,.65f,other,error) && other->identity==first->identity);
    for(int change=0;change<6;++change){auto changed=session;
        if(change==0)changed.swells[0].placement=5;if(change==1)changed.swells[0].shape.gainDb=3;
        if(change==2)changed.swells[0].shape.inTension=.3;if(change==3)changed.swells[0].render.checksum=juce::String::repeatedString("c",64);
        if(change==4)changed.swells[0].render.content=juce::String::repeatedString("d",64);if(change==5)changed.swells[0].shape.fadeOut=500;
        REQUIRE(prepareSongExportPlan(changed,source,"source-v1",changed.timing,.65f,other,error) && other->identity!=first->identity);
        if(change==0)REQUIRE(other->members[0].start==5 && other->members[0].end==48005);}
    REQUIRE(prepareSongExportPlan(session,source,"source-v2",session.timing,.65f,other,error) && other->identity!=first->identity);
    REQUIRE(prepareSongExportPlan(session,source,"source-v1",session.timing,1,other,error) && other->identity!=first->identity);
    auto warning=*first;warning.warnings.push_back({0,2,ExportCoverage::gap});REQUIRE(songExportIdentity(warning)!=first->identity);
    auto second=w;second.id=juce::Uuid().toDashedString();session.swells.push_back(second);
    REQUIRE(prepareSongExportPlan(session,source,"source-v1",session.timing,.65f,first,error));std::reverse(session.swells.begin(),session.swells.end());
    REQUIRE(prepareSongExportPlan(session,source,"source-v1",session.timing,.65f,other,error) && first->identity==other->identity);
    {
        auto activity=session;activity.swells[0].muted=true;
        REQUIRE(prepareSongExportPlan(activity,source,"activity",activity.timing,.65f,other,error) && other->members.size()==1 && other->members[0].accepted.id==activity.swells[1].id);
        activity.swells[0].muted=false;activity.swells[1].solo=true;
        REQUIRE(prepareSongExportPlan(activity,source,"activity",activity.timing,.65f,other,error) && other->members.size()==1 && other->members[0].accepted.solo);
    }
    const auto prior=other;session.swells[0].phase=Phase::checking;
    REQUIRE(!prepareSongExportPlan(session,source,"source-v1",session.timing,.65f,other,error) && prior==other && error.contains(session.swells[0].id));
    session.swells[0].enabled=false;REQUIRE(prepareSongExportPlan(session,source,"source-v1",session.timing,.65f,other,error) && other->members.size()==1);
    {
        TestDirectory directory;CaptureAssetStore store(directory.file);auto map=source;
        CaptureManifestRange audio{100,40,CaptureCoverageKind::audio,1,"audio/aaaa.wav",{}};
        std::vector<float> samples(80,.125f);samples[0]=1e-10f;REQUIRE(store.writeAudioChunk(audio,48000,samples.data(),error));
        auto missingAudio=audio;missingAudio.startSample=180;missingAudio.frameCount=20;missingAudio.assetPath="audio/missing.wav";
        map.ranges={audio,{140,20,CaptureCoverageKind::silence,1,{},{}},missingAudio};
        auto input=std::make_shared<SongExportPlan>(*other);input->source=map;input->wet=.5f;input->origin=0;input->end=240;
        ExportSourceReader reader(directory.file);REQUIRE(reader.prepare(input,1024*1024,error));
        const auto warnings=reader.warnings();REQUIRE(warnings.size()==4);
        REQUIRE((warnings[0]==ExportWarning{0,100,ExportCoverage::gap}));
        REQUIRE((warnings[1]==ExportWarning{160,180,ExportCoverage::gap}));
        REQUIRE((warnings[2]==ExportWarning{180,200,ExportCoverage::unavailable}));
        std::vector<float> page(480,9);REQUIRE(reader.read(0,page,error));
        for(int i=0;i<240;++i)for(int c=0;c<2;++c)REQUIRE(page[i*2+c]==(i>=100 && i<140?samples[(i-100)*2+c]:0));
        SourceInterval strict;REQUIRE(!readSourceInterval(store,map,0,240,240,strict,error));
        REQUIRE(readSourceInterval(store,map,100,160,60,strict,error));
        auto corrupt=std::make_shared<SongExportPlan>(*input);corrupt->source.ranges[0].checksum=std::string(64,'e');
        REQUIRE(reader.prepare(corrupt,1024*1024,error));REQUIRE(reader.read(0,page,error));
        REQUIRE(std::all_of(page.begin(),page.end(),[](float v){return v==0;}));
        REQUIRE(std::any_of(reader.warnings().begin(),reader.warnings().end(),[](const auto& w){return w.start==100 && w.kind==ExportCoverage::unavailable;}));
        REQUIRE(reader.prepare(input,1024*1024,error));
        const auto reads=reader.audioReads();input=std::make_shared<SongExportPlan>(*input);input->dry=0;input->wet=1;
        REQUIRE(reader.prepare(input,0,error) && reader.warnings().empty() && reader.audioReads()==reads);
        REQUIRE(reader.read(0,page,error) && std::all_of(page.begin(),page.end(),[](float v){return v==0;}));
        input=std::make_shared<SongExportPlan>(*input);input->dry=1;input->wet=0;input->source.ranges.clear();
        REQUIRE(reader.prepare(input,0,error) && reader.warnings().size()==1 && reader.warnings()[0].start==0 && reader.warnings()[0].end==240);
        input=std::make_shared<SongExportPlan>(*input);input->source=map;
        REQUIRE(!reader.prepare(input,8,error) && error.contains("memory"));
        REQUIRE(!reader.prepare(input,1024*1024,error,[]{return true;}));
        REQUIRE(reader.prepare(input,1024*1024,error));
        REQUIRE(directory.file.getChildFile(audio.assetPath).deleteFile());
        // Force another page/chunk load; prepared valid audio cannot silently
        // change into a warned zero branch during an existing revision.
        input=std::make_shared<SongExportPlan>(*input);REQUIRE(reader.prepare(input,1024*1024,error));
        REQUIRE(reader.warnings()[1].kind==ExportCoverage::unavailable || reader.warnings()[0].kind==ExportCoverage::unavailable);
    }
    auto clock=session.timing;clock.ppq=2;REQUIRE(!prepareSongExportPlan(session,source,"source-v1",clock,.65f,other,error));
    REQUIRE(!prepareSongExportPlan(session,source,"source-v1",session.timing,2,other,error));
});}
