#pragma once

inline void loopExportPlanCases(const m3::Session& baseline,const CaptureManifest& songSource)
{
    using namespace m3;
    auto session=baseline;auto source=songSource;
    source.schemaVersion=CaptureManifest::loopSchemaVersion;source.revision=1;
    source.loop=LoopPhraseTiming{120,4,4,16,4321,384000};
    source.ranges={{0,384000,CaptureCoverageKind::audio,1,"audio/loop.wav",std::string(64,'f')}};
    CaptureGenerationContext context;context.origin=CaptureContextOrigin::legacy;
    context.id=captureGenerationContextId(context);
    source.generationContexts={context};source.contextSpans={{0,384000,context.id}};
    auto& swell=session.swells.front();swell.arrival=swell.sourceStart=0;swell.sourceEnd=48000;
    swell.render.start=-230400;swell.render.sourceStart=0;swell.render.sourceEnd=48000;swell.render.frames=278400;
    std::shared_ptr<const SongExportPlan> plan,other;juce::String error;
    REQUIRE(isValidCaptureManifest(source,error));
    REQUIRE(prepareLoopExportPlan(session,source,"loop-v1",{2,16},0,1,plan,error));
    REQUIRE(plan->loop && plan->loop->repeats==2 && plan->origin==-384000 && plan->end==768000);
    REQUIRE(plan->members.front().start==-230400 && plan->members.front().end==0);
    REQUIRE(plan->members.front().start-plan->origin==153600); // 1.6 bars of silence.
    REQUIRE(plan->timing.sample==0 && plan->timing.ppq==0 && plan->timing.valid);
    REQUIRE(prepareLoopExportPlan(session,source,"loop-v1",{2,16},.35f,.65f,other,error));
    REQUIRE(other->dry==.35f && other->wet==.65f && other->identity!=plan->identity);
    REQUIRE(prepareLoopExportPlan(session,source,"loop-v1",{3,16},0,1,other,error) && other->end==1152000 && other->identity!=plan->identity);
    REQUIRE(prepareLoopExportPlan(session,source,"loop-v1",{2,4},0,1,other,error) && other->origin==-288000 && other->identity!=plan->identity);
    {
        auto fractional=session;
        auto capture=source;
        fractional.timing.bpm=137;
        capture.loop->tempoBpm=137;
        capture.loop->endSample=336350;
        capture.ranges.front().frameCount=336350;
        capture.contextSpans.front().end=336350;
        auto& member=fractional.swells.front();
        member.render.bpm=137;
        for(const auto start:{-84089,-84088,-84087}){
            member.render.start=start;
            member.render.frames=member.render.sourceEnd-start;
            REQUIRE(prepareLoopExportPlan(fractional,capture,"fractional-grid",{1,4},0,1,other,error));
            REQUIRE(other->members.front().start==start);
            REQUIRE(other->origin==(start==-84089?-168175:-84088));
        }
    }
    REQUIRE(prepareLoopExportPlan(session,source,"loop-v2",{2,16},0,1,other,error) && other->identity!=plan->identity);
    REQUIRE(prepareLoopExportPlan(session,source,"loop-v1",{2,16},1,0,other,error));
    REQUIRE(other->origin==0 && other->end==768000 && other->wet==0);
    auto noLead=session;noLead.swells.front()=baseline.swells.front();
    REQUIRE(prepareLoopExportPlan(noLead,source,"loop-v1",{2,16},0,1,other,error) && other->origin==0);
    auto unverified=session;unverified.timing.valid=false;
    const auto prior=other;
    REQUIRE(!prepareLoopExportPlan(unverified,source,"loop-v1",{2,16},0,1,other,error) && other==prior);
    for(auto options:std::array<LoopExportOptions,5>{{{0,16},{-1,16},{1025,16},{1,0},{1,3}}})
        REQUIRE(!prepareLoopExportPlan(session,source,"loop-v1",options,0,1,other,error) && other==prior);
    for(auto levels:std::array<std::pair<float,float>,4>{{{0,0},{-1,1},{0,2},{std::numeric_limits<float>::quiet_NaN(),1}}})
        REQUIRE(!prepareLoopExportPlan(session,source,"loop-v1",{2,16},levels.first,levels.second,other,error) && other==prior);
    auto invalid=source;invalid.sampleRate=0;
    REQUIRE(!prepareLoopExportPlan(session,invalid,"loop-v1",{2,16},0,1,other,error) && other==prior);
    invalid=source;invalid.loop->meterNumerator=0;
    REQUIRE(!prepareLoopExportPlan(session,invalid,"loop-v1",{2,16},0,1,other,error) && other==prior);
    invalid=source;invalid.loop.reset();
    REQUIRE(!prepareLoopExportPlan(session,invalid,"loop-v1",{2,16},0,1,other,error) && other==prior);
    auto huge=source;huge.loop->beats=64;huge.loop->endSample=1536000;
    huge.ranges.front().frameCount=1536000;huge.contextSpans.front().end=1536000;
    REQUIRE(!prepareLoopExportPlan(session,huge,"loop-v1",{1024,16},0,1,other,error) && other==prior && error.contains("WAV size"));
    auto overflowing=session;overflowing.swells.front().placement=INT64_MAX;
    REQUIRE(!prepareLoopExportPlan(overflowing,source,"loop-v1",{2,16},0,1,other,error) && other==prior);
    // Presentation/monitoring and unrelated current host position are not plan inputs.
    auto ignored=session;ignored.selected={swell.id};ignored.primary=swell.id;ignored.liveWetDb=-12;ignored.auditionDb=-24;
    REQUIRE(prepareLoopExportPlan(ignored,source,"loop-v1",{2,16},0,1,other,error) && other->identity==plan->identity);
    auto songCopy=*plan;songCopy.loop.reset();REQUIRE(songExportIdentity(songCopy)!=plan->identity);
    Session preferences;
    REQUIRE(setExportLevels(preferences,.8f,.2f,error));
    REQUIRE(initializeExport(preferences,error,true));
    REQUIRE(preferences.exportState.dry==.8f && preferences.exportState.wet==.2f);
    REQUIRE(preferences.exportState.loop.initialized && preferences.exportState.loop.levels==ExportMixLevels(0,1));
    REQUIRE(setExportLevels(preferences,.35f,.65f,error,true));
    preferences.exportState.loop.options={3,16};
    ExportFile file;file.path="exports/Swells-loop-v1-test.wav";file.composition=plan->identity;file.checksum=juce::String::repeatedString("a",64);
    file.rate=48000;file.origin=-384000;file.frames=1536000;file.dry=.35f;file.wet=.65f;file.loop=LoopExportDescriptor{{3,16},384000};
    preferences.exportState.history={file};preferences.exportState.completedPath=file.path;
    REQUIRE(validExportState(preferences.exportState,error));
    ExportState restored;const auto metadata=exportStateMetadata(preferences.exportState);
    REQUIRE(parseExportState(metadata,restored,error) && restored==preferences.exportState);
    juce::String encoded;REQUIRE(encode(preferences,encoded,error));Session decoded;
    REQUIRE(decode(encoded,decoded,error) && decoded.exportState==preferences.exportState);
    const auto preserved=restored;
    for(int fault=0;fault<7;++fault){auto invalid=exportStateMetadata(preferences.exportState);auto loop=invalid["loopSettings"];
        if(fault==0)loop.getDynamicObject()->setProperty("repeats",0);
        if(fault==1)loop.getDynamicObject()->setProperty("gridBeats",1.5);
        if(fault==2)loop.getDynamicObject()->setProperty("wet",1.0000000001);
        if(fault==3)loop.getDynamicObject()->setProperty("dry","0.5");
        if(fault==4)loop.getDynamicObject()->setProperty("version",2);
        if(fault==5)invalid["history"][0]["loop"].getDynamicObject()->setProperty("periodFrames","0");
        if(fault==6)invalid["history"][0]["loop"].getDynamicObject()->setProperty("repeats",1000);
        REQUIRE(!parseExportState(invalid,restored,error) && restored==preserved);
    }
    auto previous=exportStateMetadata(preferences.exportState);previous.getDynamicObject()->removeProperty("loopSettings");
    REQUIRE(parseExportState(previous,restored,error) && restored.loop==LoopExportPreferences{} && restored.history==preserved.history);
}
