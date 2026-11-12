#include "TestSupport.h"
#include <Core/SwellSession.h>
#include <limits>
using namespace m3;
#include "PerformNavigationStateCases.h"
int main() { return runTest([]
{
    Session s; CaptureManifest map; juce::String error;
    {
        Swell swell;
        swell.arrival = 100;
        swell.placement = 17;
        swell.shape = {false, -50, 20, 60, 55, Curve::smooth, Curve::exponential, -7, .2, -.3};
        const auto original = swell.shape;
        REQUIRE(fitWetShapeToResponse(swell, 90, 130, error));
        REQUIRE(swell.shape.startOffset == -10 && swell.shape.endOffset == 20);
        REQUIRE(swell.shape.fadeIn == 29 && swell.shape.fadeOut == 29);
        REQUIRE(swell.shape.gainDb == -7 && swell.shape.inCurve == Curve::smooth);
        REQUIRE(swell.shape.outCurve == Curve::exponential && swell.shape.outTension == -.3);
        int64_t start, end;
        REQUIRE(shapeBounds(swell, 90, 130, start, end, error) && start == 107 && end == 137);
        REQUIRE(fitWetShapeToResponse(swell, 121, 122, error));
        REQUIRE(swell.shape.startOffset == 21 && swell.shape.endOffset == 22);
        REQUIRE(swell.shape.fadeIn == 0 && swell.shape.fadeOut == 0);
        REQUIRE(fitWetShapeToResponse(swell, 30, 160, error));
        REQUIRE(swell.shape == original && !swell.wetTimingIntent);
        REQUIRE(swell.arrival == 100 && swell.placement == 17);

        swell.shape.automaticStart = true;
        swell.shape.fadeIn = 0;
        REQUIRE(fitWetShapeToResponse(swell, 100, 110, error));
        REQUIRE(swell.shape.automaticStart && swell.shape.endOffset == 10);
        REQUIRE(swell.shape.fadeIn == 0 && swell.shape.fadeOut == 9);
        REQUIRE(fitWetShapeToResponse(swell, 30, 160, error));
        REQUIRE(swell.shape.automaticStart && swell.shape.endOffset == 20 && swell.shape.fadeOut == 55);
        const auto beforeOverflow = swell.shape;
        REQUIRE(!fitWetShapeToResponse(swell, 100, std::numeric_limits<int64_t>::max(), error));
        REQUIRE(swell.shape == beforeOverflow && !swell.wetTimingIntent);
    }
    {
        Swell swell;
        swell.shape.automaticStart=false;
        swell.shape.startOffset=-150;
        swell.shape.endOffset=0;
        swell.shape.fadeOut=120;
        REQUIRE(fitWetShapeToResponse(swell,-100,1,error));
        const auto gesture=swell.shape;
        REQUIRE(gesture.fadeOut==99);
        auto first=gesture;
        first.endOffset=-20;
        REQUIRE(setShape(swell,first,-100,1,error,WetTimingEdit::end));
        REQUIRE(swell.shape.fadeOut==79);
        auto second=gesture;
        second.endOffset=-10;
        REQUIRE(setShape(swell,second,-100,1,error,WetTimingEdit::end));
        REQUIRE(swell.shape.fadeOut==89 && swell.wetTimingIntent && swell.wetTimingIntent->fadeOut==120);
        REQUIRE(fitWetShapeToResponse(swell,-150,1,error));
        REQUIRE(swell.shape.endOffset==-10 && swell.shape.fadeOut==120);
    }
    {
        Session fresh,decoded;juce::String encoded;
        REQUIRE(!fresh.autoGenerateOnStop);
        for(bool automatic:{false,true})for(bool stop:{false,true})
        {
            fresh.automaticProcessing=automatic;fresh.autoGenerateOnStop=stop;
            REQUIRE(encode(fresh,encoded,error) && decode(encoded,decoded,error));
            REQUIRE(decoded.autoGenerateOnStop==stop && decoded.automaticProcessing==automatic);
        }
        for(int version=1;version<=10;++version)
        {
            auto legacy=juce::JSON::parse(encoded);legacy.getDynamicObject()->setProperty("version",version);
            legacy.getDynamicObject()->removeProperty("autoGenerateOnStop");
            REQUIRE(decode(juce::JSON::toString(legacy),decoded,error) && decoded.autoGenerateOnStop);
        }
        fresh.autoGenerateOnStop=false;REQUIRE(encode(fresh,encoded,error) && decode(encoded,decoded,error));
        for(int mutation=0;mutation<4;++mutation)
        {
            auto bad=juce::JSON::parse(encoded);auto* object=bad.getDynamicObject();
            if(mutation==0)object->removeProperty("autoGenerateOnStop");
            if(mutation==1)object->setProperty("autoGenerateOnStop",1);
            if(mutation==2)object->setProperty("autoGenerateOnStop","false");
            if(mutation==3)object->setProperty("autoGenerateOnStop",juce::var{});
            REQUIRE(!decode(juce::JSON::toString(bad),decoded,error) && !decoded.autoGenerateOnStop);
        }
    }
    {
        SongTime timing{true,0,0,48000,120,4,4};
        REQUIRE(defaultSwellName(34*48000,timing)=="Swell Bar 18.1");
        REQUIRE(defaultSwellName(35*48000,timing)=="Swell Bar 18.3");
        REQUIRE(defaultSwellName(34*48000+6000,timing)=="Swell Bar 18.1.25");
        timing.sample=48000;timing.ppq=12;timing.numerator=3;timing.denominator=4;
        REQUIRE(defaultSwellName(48000,timing)=="Swell Bar 5.1");
        timing.denominator=8;REQUIRE(defaultSwellName(60000,timing)=="Swell Bar 9.2");
        timing.valid=false;REQUIRE(defaultSwellName(0,timing)=="Swell");
        CaptureManifest covered;covered.sampleRate=48000;covered.ranges={{0,192000,CaptureCoverageKind::silence}};
        timing={true,0,0,48000,120,4,4};Session definitions;
        REQUIRE(add(definitions,100,covered,timing,{},error) && add(definitions,200,covered,timing,{},error));
        REQUIRE(definitions.swells[0].name=="Swell Bar 1.1.004166667" && definitions.swells[1].name=="Swell Bar 1.1.008333333" && definitions.swells[0].id!=definitions.swells[1].id);
        auto duplicate=definitions;REQUIRE(add(duplicate,100,covered,timing,{},error));
        REQUIRE(duplicate.swells.back().name=="Swell Bar 1.1.004166667 (2)");
        auto& edited=definitions.swells[0];edited.name="My saved name";edited.placement=1000;edited.shape.gainDb=-9;
        edited.shape.fadeIn=123;edited.shape.inTension=.7;edited.enabled=false;edited.solo=true;edited.frozen=true;
        edited.generatedGain=.375f;
        const auto identity=edited.id;
        REQUIRE(resetWetDefaults(edited,48000,error));
        REQUIRE(edited.shape==initialShape(48000) && edited.shape.endOffset==960 && edited.shape.fadeOut==960 && edited.shape.outCurve==Curve::smooth);
        REQUIRE(edited.placement==0 && edited.name=="My saved name" && edited.id==identity && !edited.enabled && edited.solo && edited.frozen);
        REQUIRE(edited.generatedGain==.375f);
        const auto unchanged=edited;REQUIRE(!resetWetDefaults(edited,0,error) && edited.shape==unchanged.shape && edited.frozen);
        Swell result;REQUIRE(!resetSwellDefaults(definitions,identity,covered,timing,result,error));
        definitions.defaultIR.path="irs/default.wav";definitions.defaultIRName="Future default";
        REQUIRE(resetSwellDefaults(definitions,identity,covered,timing,result,error));
        REQUIRE(result.id==identity && result.name==edited.name && result.arrival==100 && result.sourceStart==100 && result.sourceEnd==200);
        REQUIRE(result.enabled && !result.muted && !result.solo && !result.frozen && result.ir==definitions.defaultIR && result.shape==initialShape(48000));
        REQUIRE(!definitions.swells[0].enabled && definitions.swells[0].frozen && definitions.swells[1].arrival==200);
    }
    map.ranges = {{-100, 150, CaptureCoverageKind::audio}, {50, 1950, CaptureCoverageKind::silence}, {3000,100, CaptureCoverageKind::audio}};
    SongTime time{true,0,0,1000,120,4,4};
    REQUIRE(add(s,0,map,time,{100},error));
    REQUIRE(s.swells[0].sourceEnd == 100 && s.swells[0].shape.endOffset == 20);
    const auto first = s.swells[0].id;
    REQUIRE(add(s,1500,map,time,{},error));
    const auto second = s.swells[1].id;
    REQUIRE(first != second && s.swells[1].sourceEnd == 2000);
    REQUIRE(add(s,3050,map,time,{},error) && s.swells.back().sourceEnd == 3100);
    REQUIRE(! add(s,2500,map,time,{},error));
    REQUIRE(select(s,{first,second},first)); REQUIRE(! select(s,{first,first},first));
    auto& w = s.swells[0]; w.name="Edited"; w.phase=Phase::ready;
    REQUIRE(canAudition(s,w)); s.swells[1].solo=true; REQUIRE(! canAudition(s,w));
    w.solo=true; REQUIRE(canAudition(s,w)); w.muted=true; REQUIRE(! canAudition(s,w));
    w.muted=false; w.enabled=false; REQUIRE(! canAudition(s,w)); w.enabled=true;
    const auto otherStart=s.swells[1].sourceStart;
    REQUIRE(moveArrival(w,13,error)); REQUIRE(w.sourceStart==13 && w.sourceEnd==113 && w.shape.endOffset==20);
    REQUIRE(s.swells[1].sourceStart==otherStart);
    REQUIRE(setSource(w,3,40,error)); REQUIRE(! setSource(w,14,40,error));
    const auto savedStart=w.sourceStart;
    REQUIRE(! moveArrival(w,INT64_MIN,error) && w.sourceStart==savedStart);
    int64_t a,b;
    REQUIRE(shapeBounds(w,-100,40,a,b,error) && a==-100 && b==33);
    Shape shape; shape.automaticStart=false; shape.startOffset=-20; shape.endOffset=10;
    shape.fadeIn=100; shape.fadeOut=100; shape.gainDb=6; shape.inCurve=Curve::smooth; shape.outCurve=Curve::exponential;
    REQUIRE(setShape(w,shape,-100,40,error)); REQUIRE(w.shape.fadeIn==29 && w.shape.fadeOut==29);
    w.render.content="retained";
    REQUIRE(! shapeBounds(w,0,40,a,b,error)); // New short IR: retain invalid crop intent.
    REQUIRE(w.shape.startOffset==-20 && w.render.content=="retained");
    REQUIRE(fullShape(w,-100,40,error)); REQUIRE(shapeBounds(w,-100,40,a,b,error) && a==-100 && b==40);
    resetShape(w); REQUIRE(w.shape.automaticStart && w.shape.gainDb==0 && w.shape.endOffset==1);
    shape.gainDb=std::numeric_limits<float>::infinity(); REQUIRE(! setShape(w,shape,-100,40,error));
    for(const auto level:{-60.f,-30.f,-12.f,0.f,12.f,-30.12345f})
    {shape.gainDb=level;REQUIRE(setShape(w,shape,-100,40,error));REQUIRE(w.shape.gainDb==level);}
    for(const auto level:{-60.01f,12.01f,std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()})
    {const auto retained=w.shape;shape.gainDb=level;REQUIRE(!setShape(w,shape,-100,40,error) && w.shape==retained);}
    w.render={}; s.auditionDb=-24; s.autoRegenerate=true; s.sensitivity=0.7f; s.separationSeconds=0.12;
    shape.gainDb=-3;REQUIRE(setShape(w,shape,-100,40,error));
    juce::String json; REQUIRE(encode(s,json,error)); Session restored;performNavigationStateCases(s);
    REQUIRE(decode(json,restored,error));
    REQUIRE(restored.swells.size()==3 && restored.swells[0].name=="Edited" && restored.primary==first);
    REQUIRE(restored.auditionDb==-24 && restored.autoRegenerate && !restored.swells[0].shape.automaticStart);
    REQUIRE(restored.swells[0].shape.inCurve==Curve::smooth && restored.swells[0].shape.outCurve==Curve::exponential && restored.swells[0].shape.fadeIn==29);
    juce::String roundtrip;REQUIRE(encode(restored,roundtrip,error) && roundtrip==json);
    for(const auto policy:{GainPolicy::legacy,GainPolicy::musical})for(const auto level:{-60.f,-30.f,-30.12345f})
    {
        auto fixture=s;fixture.gainPolicy=policy;fixture.gainRevision=policy==GainPolicy::legacy?legacyGainRevision:approvedGainRevision;
        fixture.swells[0].shape.gainDb=level;
        Session reopened;juce::String bytes,again;
        REQUIRE(encode(fixture,bytes,error) && decode(bytes,reopened,error) && encode(reopened,again,error) && again==bytes);
        REQUIRE(reopened.swells[0].shape==fixture.swells[0].shape && reopened.swells[0].frozen==fixture.swells[0].frozen);
        REQUIRE(reopened.swells[0].ir==fixture.swells[0].ir && reopened.swells[0].render==fixture.swells[0].render
            && reopened.swells[0].calibration==fixture.swells[0].calibration && reopened.gainPolicy==policy);
    }
    REQUIRE(restored.timing.rate==1000 && restored.swells[1].solo);
    const auto reject = [&](const juce::String& bad)
    { REQUIRE(! decode(bad,restored,error)); REQUIRE(restored.primary==first && restored.auditionDb==-24); };
    reject(json.replace("\"version\":12", "\"version\":13"));
    for(const auto& fields:{
        juce::String("\"defaultLevelDb\":\"invalid\",\"defaultLevelDb\":0,"),
        juce::String("\"defaultLevelDb\":\"invalid\",\"defaultLevel\\u0044b\":0,"),
        juce::String("\"version\":13,\"version\":11,")})
    {
        reject("{" + fields + json.substring(1));
        REQUIRE(error.contains("Duplicate"));
    }
    reject(json.replace("\"soundPreset\":", "\"soundPreset\":null,\"soundPreset\":"));
    REQUIRE(error.contains("Duplicate"));
    for(const auto& invalid:{juce::var("0"),juce::var(13)})
    {
        const auto metadata=juce::JSON::parse(json);
        metadata.getDynamicObject()->setProperty("defaultLevelDb",invalid);
        reject(juce::JSON::toString(metadata));
    }
    reject(json.replace("\"defaultSoundPreset\":[\"\",1]", "\"defaultSoundPreset\":[\"preverb-hall\",1]"));
    reject(json.replace(second,first));
    reject(json.replace("\"gain\":-3", "\"gain\":\"nan\""));
    reject(json.replace("\"sourceEnd\":\"40\"", "\"sourceEnd\":\"3\""));
    reject(json.replace("\"path\":\"\"", "\"path\":\"../outside.wav\""));
    reject(juce::String::repeatedString("x",static_cast<int>(maximumStateBytes+1)));
    {
        auto accepted=s;
        auto& hall=accepted.defaultIR;
        hall.path="irs/ab-cd.wav";hall.checksum=juce::String::repeatedString("a",64);
        hall.content=defaultImpulseContent;hall.rate=48000;hall.channels=2;hall.frames=86400;
        accepted.defaultIRName="Historical label";accepted.defaultLevelDb=4.5f;
        accepted.swells.front().ir=hall;
        accepted.swells.front().soundPreset={customImpulseId,1}; // Import matching the factory PCM.
        juce::String bytes;Session reopened;
        REQUIRE(encode(accepted,bytes,error) && decode(bytes,reopened,error));
        REQUIRE(reopened.defaultSoundPreset.id==defaultImpulseId && reopened.defaultIRName==accepted.defaultIRName);
        REQUIRE(reopened.defaultLevelDb==4.5f && reopened.swells.front().soundPreset.id==customImpulseId);
        REQUIRE(reopened.defaultIR==hall && reopened.swells.front().ir==hall);
        const auto invalid=juce::JSON::parse(bytes);
        invalid.getDynamicObject()->setProperty("defaultSoundPreset",juce::Array<juce::var>{"whisper",1});
        REQUIRE(!decode(juce::JSON::toString(invalid),reopened,error));
        REQUIRE(reopened.defaultIR==hall && reopened.defaultSoundPreset.id==defaultImpulseId);
        const auto v11=juce::JSON::parse(bytes);
        v11.getDynamicObject()->setProperty("version",11);
        v11.getDynamicObject()->removeProperty("defaultSoundPreset");
        v11.getDynamicObject()->removeProperty("defaultLevelDb");
        v11["swells"][0].getDynamicObject()->removeProperty("soundPreset");
        REQUIRE(decode(juce::JSON::toString(v11),reopened,error));
        REQUIRE(reopened.defaultLevelDb==0 && reopened.defaultSoundPreset.id==defaultImpulseId);
        REQUIRE(reopened.swells.front().soundPreset.id==defaultImpulseId && reopened.swells.front().ir==hall);
        REQUIRE(reopened.swells.front().shape.gainDb==accepted.swells.front().shape.gainDb);
    }
    REQUIRE(erase(s,first) && ! find(s,first) && s.primary==second && s.selected.size()==1);
    REQUIRE(! erase(s,first));
    Session full; full.swells.resize(maximumSwells); REQUIRE(! add(full,0,map,time,{},error));
    map.ranges={{INT64_MAX-2,2,CaptureCoverageKind::silence}};
    REQUIRE(! add(s,INT64_MAX-1,map,time,{},error));
    map.ranges={{INT64_MIN,10,CaptureCoverageKind::silence}};
    REQUIRE(add(s,INT64_MIN,map,time,{},error));
    int64_t sum; REQUIRE(! checkedAdd(INT64_MAX,1,sum) && ! checkedSubtract(INT64_MIN,1,sum));
    Session automatic; REQUIRE(automatic.mappingEnabled && automatic.automaticProcessing && automatic.auditionDb==0);
    REQUIRE(automatic.auditionMode==AuditionMode::wetOnly && automatic.wetBlend==0.5f);
    automatic.swells.emplace_back(); auto& target=automatic.swells.back(); target.sourceEnd=500;
    target.provenance=Provenance::automatic; target.detectorAnchor=17;
    automatic.suppressed={-400,12345}; automatic.auditionMode=AuditionMode::dryWet;automatic.wetBlend=0.35f;
    REQUIRE(encode(automatic,json,error) && decode(json,restored,error));
    REQUIRE(restored.swells[0].id==target.id && restored.swells[0].detectorAnchor==17);
    REQUIRE(restored.swells[0].provenance==Provenance::automatic && restored.suppressed==automatic.suppressed);
    target.generationSource=juce::String::repeatedString("a",64);
    REQUIRE(encode(automatic,json,error) && decode(json,restored,error));
    REQUIRE(restored.swells[0].generationSource==target.generationSource);
    {auto bad=juce::JSON::parse(json);bad["swells"][0].getDynamicObject()->setProperty("generationSource","invalid");
        REQUIRE(!decode(juce::JSON::toString(bad),restored,error) && restored.swells[0].generationSource==target.generationSource);}
    REQUIRE(restored.auditionMode==AuditionMode::dryWet && restored.wetBlend==0.35f);
    automatic.auditionFollowsLive=true;automatic.auditionDb=-24;
    REQUIRE(encode(automatic,json,error) && decode(json,restored,error));
    REQUIRE(restored.auditionFollowsLive && restored.auditionDb==-24 && restored.wetBlend==.35f);
    auto metadata=juce::JSON::parse(json);metadata.getDynamicObject()->removeProperty("auditionFollowsLive");
    REQUIRE(decode(juce::JSON::toString(metadata),restored,error) && !restored.auditionFollowsLive && restored.auditionDb==-24);
    metadata.getDynamicObject()->setProperty("auditionFollowsLive","true");const auto safe=restored;
    REQUIRE(!decode(juce::JSON::toString(metadata),restored,error) && restored.auditionDb==safe.auditionDb && restored.swells[0].id==safe.swells[0].id);
    automatic.auditionFollowsLive=false;REQUIRE(encode(automatic,json,error) && decode(json,restored,error));
    const auto retained=restored.swells[0].id;
    auto legacy=juce::JSON::parse(json);legacy.getDynamicObject()->setProperty("version",1);
    legacy.getDynamicObject()->setProperty("auditionDb",-12.0);
    REQUIRE(decode(juce::JSON::toString(legacy),restored,error));
    REQUIRE(!restored.mappingEnabled && !restored.automaticProcessing && restored.auditionDb==-12);
    REQUIRE(restored.swells[0].provenance==Provenance::manualEdited && restored.swells[0].id==retained);
    for(int mutation=0;mutation<3;++mutation)
    {
        auto bad=juce::JSON::parse(json);
        if(mutation==0) bad.getDynamicObject()->setProperty("wetBlend",2.0);
        if(mutation==1) bad["swells"][0].getDynamicObject()->setProperty("provenance",9);
        if(mutation==2) bad["swells"][0].getDynamicObject()->setProperty("detectorAnchor",17);
        REQUIRE(!decode(juce::JSON::toString(bad),restored,error));REQUIRE(restored.swells[0].id==retained);
    }
    // Wet endpoint is independent of the fixed dry arrival and fade duration.
    auto& transition=automatic.swells[0];transition.arrival=100;transition.sourceStart=100;transition.sourceEnd=500;
    auto envelopeShape=transition.shape;envelopeShape.endOffset=700;envelopeShape.fadeOut=40;envelopeShape.inTension=.25;envelopeShape.outTension=-.6;
    REQUIRE(setShape(transition,envelopeShape,-200,500,error));
    int64_t extent,contextEnd;REQUIRE(effectiveSourceEnd(transition,extent) && extent==800);
    REQUIRE(transition.sourceEnd==500 && transition.arrival==100 && transition.shape.fadeOut==40);
    transition.supportOffset=300; // Simulate successfully committed extended support.
    envelopeShape.endOffset=-20;REQUIRE(setShape(transition,envelopeShape,-200,800,error));
    REQUIRE(transition.shape.fadeOut==40 && renderInputEnd(transition)==800);
    REQUIRE(auditionContextEnd(transition,1000,2,contextEnd) && contextEnd==2100);
    envelopeShape.endOffset=700;REQUIRE(setShape(transition,envelopeShape,-200,800,error));
    REQUIRE(auditionContextEnd(transition,1000,2,contextEnd) && contextEnd==2800);
    auto longSource=transition;longSource.sourceEnd=10000;longSource.supportOffset=0;
    REQUIRE(auditionContextEnd(longSource,1000,2,contextEnd) && contextEnd==10000);
    automatic.postRollSeconds=1.25;REQUIRE(encode(automatic,json,error) && decode(json,restored,error));
    REQUIRE(restored.postRollSeconds==1.25 && restored.swells[0].shape.outTension==-.6 && restored.swells[0].supportOffset==300);
    auto v2=juce::JSON::parse(json);v2.getDynamicObject()->setProperty("version",2);
    REQUIRE(decode(juce::JSON::toString(v2),restored,error));
    REQUIRE(restored.postRollSeconds==2 && restored.swells[0].shape.inTension==0 && restored.swells[0].shape.endOffset==700);
    for(int mutation=0;mutation<4;++mutation){auto bad=juce::JSON::parse(json);
        if(mutation==0)bad.getDynamicObject()->setProperty("postRollSeconds",5.01);
        if(mutation==1)bad["swells"][0]["shape"].getDynamicObject()->setProperty("inTension",1.01);
        if(mutation==2)bad["swells"][0].getDynamicObject()->setProperty("supportOffset","-1");
        if(mutation==3)bad["swells"][0]["shape"].getDynamicObject()->setProperty("end","9223372036854775807");
        REQUIRE(!decode(juce::JSON::toString(bad),restored,error));}
    Session placementState;placementState.swells.emplace_back();auto& placed=placementState.swells[0];placed.arrival=100;placed.sourceStart=100;placed.sourceEnd=500;
    placed.shape.automaticStart=false;placed.shape.startOffset=-50;placed.shape.endOffset=30;placed.shape.fadeIn=7;placed.shape.fadeOut=13;placed.shape.inTension=.3;
    REQUIRE(moveWet(placed,-1000,error));REQUIRE(requestedShapeBounds(placed,0,a,b) && a==-950 && b==-870);
    REQUIRE(renderInputEnd(placed)==500 && placed.arrival==100 && placed.shape.fadeOut==13);
    REQUIRE(!moveWet(placed,INT64_MAX,error) && placed.placement==-1000);
    placementState.autoMarkersEnabled=false;placementState.isolateSelected=true;
    REQUIRE(encode(placementState,json,error) && decode(json,restored,error));
    REQUIRE(!restored.autoMarkersEnabled && restored.isolateSelected && restored.swells[0].placement==-1000);
    {
        auto fitted = placementState;
        auto& swell = fitted.swells.front();
        const auto oldShape = swell.shape;
        REQUIRE(fitWetShapeToResponse(swell, 95, 105, error));
        auto edited = swell.shape;
        edited.fadeIn = 2;
        REQUIRE(setShape(swell, edited, 95, 105, error));
        REQUIRE(swell.wetTimingIntent && swell.wetTimingIntent->fadeIn == 2);
        REQUIRE(swell.wetTimingIntent->startOffset == oldShape.startOffset);
        REQUIRE(swell.wetTimingIntent->fadeOut == oldShape.fadeOut);
        Session reopened;
        juce::String fittedJson;
        REQUIRE(encode(fitted,fittedJson,error) && decode(fittedJson,reopened,error));
        REQUIRE(sameSavedSwell(swell,reopened.swells.front()));
        auto& recovered = reopened.swells.front();
        REQUIRE(fitWetShapeToResponse(recovered, 20, 150, error));
        REQUIRE(recovered.shape.startOffset == oldShape.startOffset && recovered.shape.endOffset == oldShape.endOffset);
        REQUIRE(recovered.shape.fadeIn == 2 && recovered.shape.fadeOut == oldShape.fadeOut);
        REQUIRE(!recovered.wetTimingIntent && recovered.ir == swell.ir && recovered.render == swell.render);

        auto badIntent = juce::JSON::parse(fittedJson);
        badIntent["swells"][0]["wetTimingIntent"].getDynamicObject()->setProperty("end", "9223372036854775807");
        REQUIRE(decode(juce::JSON::toString(badIntent),reopened,error));
        REQUIRE(reopened.swells.front().shape == swell.shape && !reopened.swells.front().wetTimingIntent);
        REQUIRE(decode(json,reopened,error));
        REQUIRE(reopened.swells.front().shape == placed.shape && !reopened.swells.front().wetTimingIntent);
    }
    auto oldVersion=juce::JSON::parse(json);oldVersion.getDynamicObject()->setProperty("version",3);
    REQUIRE(decode(juce::JSON::toString(oldVersion),restored,error));REQUIRE(restored.autoMarkersEnabled && !restored.isolateSelected && restored.swells[0].placement==0);
    auto badToggle=juce::JSON::parse(json);badToggle.getDynamicObject()->setProperty("autoMarkersEnabled",1);REQUIRE(!decode(juce::JSON::toString(badToggle),restored,error));
    REQUIRE(placementState.liveEnabled && placementState.liveRouting==LiveRouting::dryWet && placementState.liveWetDb==0);
    placementState.liveEnabled=false;placementState.liveRouting=LiveRouting::wetOnly;placementState.liveWetDb=-7.5f;
    REQUIRE(encode(placementState,json,error) && decode(json,restored,error));
    REQUIRE(!restored.liveEnabled && restored.liveRouting==LiveRouting::wetOnly && restored.liveWetDb==-7.5f);
    REQUIRE(restored.swells[0].placement==-1000 && restored.isolateSelected && !restored.autoMarkersEnabled);
    for(int version=1;version<=4;++version){auto old=juce::JSON::parse(json);old.getDynamicObject()->setProperty("version",version);
        REQUIRE(decode(juce::JSON::toString(old),restored,error));
        REQUIRE(restored.liveEnabled && restored.liveRouting==LiveRouting::dryWet && restored.liveWetDb==0);
        REQUIRE(restored.gainPolicy==GainPolicy::legacy && restored.gainRevision==legacyGainRevision);}
    const auto before=restored;
    for(int mutation=0;mutation<6;++mutation){auto bad=juce::JSON::parse(json);auto* obj=bad.getDynamicObject();
        if(mutation==0)obj->setProperty("liveEnabled",1);
        if(mutation==1)obj->setProperty("liveRouting",2);
        if(mutation==2)obj->setProperty("liveRouting",.5);
        if(mutation==3)obj->setProperty("liveWetDb",.01);
        if(mutation==4)obj->setProperty("liveWetDb",-60.01);
        if(mutation==5)obj->removeProperty("liveWetDb");
        REQUIRE(!decode(juce::JSON::toString(bad),restored,error));
        REQUIRE(restored.swells[0].id==before.swells[0].id && restored.liveWetDb==before.liveWetDb);}
    automatic.suppressed.resize(maximumSwells+1);REQUIRE(!encode(automatic,json,error));

    // Version 8 introduces an explicit gain policy and per-swell calibration.
    Session musical;musical.gainPolicy=GainPolicy::musical;musical.gainRevision=candidateGainRevision;musical.liveWetDb=4;musical.swells.emplace_back();auto& calibrated=musical.swells.back();
    calibrated.sourceEnd=1000;calibrated.calibration.state=CalibrationState::matched;calibrated.calibration.baselineDb=6;
    calibrated.calibration.dryPeak=.5f;calibrated.calibration.wetPeak=.25f;calibrated.calibration.referenceStart=0;
    calibrated.calibration.referenceEnd=100;calibrated.calibration.sourceIdentity="source";calibrated.calibration.irIdentity="ir";
    calibrated.calibration.renderIdentity="render";calibrated.calibration.revision=candidateGainRevision;
    calibrated.generatedGain=0.375f;
    REQUIRE(musical.gainPolicy==GainPolicy::musical && encode(musical,json,error));
    REQUIRE(json.contains("\"version\":12") && decode(json,restored,error));
    REQUIRE(restored.gainPolicy==GainPolicy::musical && restored.gainRevision==candidateGainRevision && restored.liveWetDb==4);
    REQUIRE(restored.swells[0].calibration==calibrated.calibration);
    REQUIRE(restored.swells[0].generatedGain==calibrated.generatedGain);
    for(int version=1;version<=7;++version){auto old=juce::JSON::parse(json);old.getDynamicObject()->setProperty("version",version);
        old.getDynamicObject()->setProperty("liveWetDb",0.0);
        REQUIRE(decode(juce::JSON::toString(old),restored,error));
        REQUIRE(restored.gainPolicy==GainPolicy::legacy && restored.gainRevision==legacyGainRevision);
        REQUIRE(restored.swells[0].calibration.state==CalibrationState::unavailable);}
    const auto validV8=musical;
    {auto energy=musical;energy.gainRevision=energyGainRevision;auto& g=energy.swells[0].calibration;
        g.revision=energyGainRevision;g.dryRms=.25f;g.wetRms=.125f;g.windowFrames=20;g.hopFrames=1;
        juce::String energyJson;REQUIRE(encode(energy,energyJson,error) && decode(energyJson,restored,error));
        REQUIRE(restored.swells[0].calibration==g);
        for(const auto* field:{"dryRms","wetRms","windowFrames","hopFrames"}){auto bad=juce::JSON::parse(energyJson);
            bad["swells"][0]["calibration"].getDynamicObject()->removeProperty(field);restored=energy;
            REQUIRE(!decode(juce::JSON::toString(bad),restored,error));REQUIRE(restored.swells[0].calibration==g);}}
    for(int mutation=0;mutation<6;++mutation){auto bad=juce::JSON::parse(json);auto* root=bad.getDynamicObject();auto calibration=bad["swells"][0]["calibration"].getDynamicObject();
        if(mutation==0)root->setProperty("gainPolicy",2);
        if(mutation==1)root->setProperty("gainRevision","unknown");
        if(mutation==2)calibration->setProperty("baselineDb",25.0);
        if(mutation==3)calibration->setProperty("dryPeak","nan");
        if(mutation==4)calibration->setProperty("referenceEnd","0");
        if(mutation==5)calibration->setProperty("revision","mismatch");
        restored=validV8;REQUIRE(!decode(juce::JSON::toString(bad),restored,error));REQUIRE(restored.liveWetDb==4 && restored.gainPolicy==GainPolicy::musical);}
    musical.gainPolicy=GainPolicy::legacy;musical.gainRevision=legacyGainRevision;musical.liveWetDb=4;
    REQUIRE(!validate(musical,error));musical.liveWetDb=0;musical.swells[0].calibration={};REQUIRE(validate(musical,error));
}); }
