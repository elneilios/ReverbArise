#include "TestSupport.h"
#include <Capture/CaptureSession.h>
#include <Core/SwellSession.h>
#include <Core/ContentHash.h>
#include <Capture/SourceVisitor.h>
#include <Capture/SourceRangeReader.h>
#include <limits>

int main(){return runTest([]
{
    TestDirectory root;CaptureSession source(root.file);const float audio[]{.25f,-.25f,.5f,-.5f,.1f,-.1f,.2f,-.2f};float zeros[4]{};
    REQUIRE(source.commit(0,48000,audio,4) && source.commit(4,48000,zeros,2) && source.commit(10,48000,audio,4));
    const auto accepted=source.snapshot();juce::String error,legacyJson;
    REQUIRE(serializeCaptureManifest(accepted.manifest,legacyJson,error));
    REQUIRE(accepted.manifest.schemaVersion==1 && !legacyJson.contains("contextSpans"));
    CaptureManifest legacy;REQUIRE(parseCaptureManifest(legacyJson,legacy,error));
    CaptureGenerationContext baseline;baseline.automatic=true;baseline.sensitivity=.375f;baseline.separationSeconds=.08;
    baseline.ir.path="irs/"+juce::String::repeatedString("a",64)+".wav";
    baseline.ir.checksum=baseline.ir.content=juce::String::repeatedString("a",64);
    baseline.ir.rate=48000;baseline.ir.channels=2;baseline.ir.frames=32;baseline.irName="Accepted response";
    m3::Session saved;saved.defaultIR=baseline.ir;saved.defaultIRName=baseline.irName;
    REQUIRE(m3::add(saved,0,legacy,{true,0,0,48000,120,4,4},{},error));saved.swells[0].ir=baseline.ir;saved.swells[0].irName=baseline.irName;
    juce::String savedBefore,savedAfter;REQUIRE(m3::encode(saved,savedBefore,error));
    CaptureManifest upgraded;REQUIRE(captureWithLegacyContext(legacy,baseline,upgraded,error));
    REQUIRE(upgraded.schemaVersion==2 && upgraded.generationContexts.size()==1 && upgraded.contextSpans.size()==2);
    REQUIRE(upgraded.generationContexts[0].origin==CaptureContextOrigin::legacy);
    REQUIRE(captureContextAt(upgraded,4) && !captureContextAt(upgraded,7) && !captureContextAt(upgraded,14));
    REQUIRE(upgraded.contextSpans[0].start==0 && upgraded.contextSpans[0].end==6);
    CaptureManifest repeated;REQUIRE(captureWithLegacyContext(legacy,baseline,repeated,error));
    REQUIRE(repeated.generationContexts==upgraded.generationContexts && repeated.contextSpans==upgraded.contextSpans);
    REQUIRE(m3::encode(saved,savedAfter,error) && savedBefore==savedAfter);
    auto unchanged=upgraded;unchanged.schemaVersion=1;unchanged.generationContexts.clear();unchanged.contextSpans.clear();
    juce::String unchangedJson;REQUIRE(serializeCaptureManifest(unchanged,unchangedJson,error) && unchangedJson==legacyJson);
    float left=0,right=0;REQUIRE(source.snapshot().reference==accepted.reference && source.readSample(0,left,right) && left==.25f && right==-.25f);
    juce::String json;REQUIRE(serializeCaptureManifest(upgraded,json,error));CaptureManifest parsed;
    REQUIRE(parseCaptureManifest(json,parsed,error) && parsed.generationContexts==upgraded.generationContexts && parsed.contextSpans==upgraded.contextSpans);
    auto mixed=upgraded;auto pass=baseline;pass.sensitivity=.125f;pass.id=captureGenerationContextId(pass);mixed.generationContexts.push_back(pass);
    mixed.contextSpans={{0,2,upgraded.generationContexts[0].id},{2,6,pass.id},{10,14,upgraded.generationContexts[0].id}};
    juce::String mixedJson;CaptureManifest mixedParsed;REQUIRE(serializeCaptureManifest(mixed,mixedJson,error) && parseCaptureManifest(mixedJson,mixedParsed,error));
    REQUIRE(captureContextAt(mixedParsed,1)->origin==CaptureContextOrigin::legacy && captureContextAt(mixedParsed,2)->id==pass.id);
    auto negative=upgraded;negative.ranges={{-16,8,CaptureCoverageKind::silence}};negative.contextSpans={{-16,-8,negative.generationContexts[0].id}};
    REQUIRE(isValidCaptureManifest(negative,error) && captureContextAt(negative,-16) && !captureContextAt(negative,-8));
    const auto reject=[&](auto mutate)
    {auto bad=upgraded;mutate(bad);juce::String output="unchanged";REQUIRE(!serializeCaptureManifest(bad,output,error) && output=="unchanged");};
    reject([](auto& m){m.generationContexts[0].sensitivity=std::numeric_limits<float>::quiet_NaN();});
    reject([](auto& m){m.generationContexts[0].separationSeconds=std::numeric_limits<double>::infinity();});
    reject([](auto& m){m.generationContexts[0].origin=static_cast<CaptureContextOrigin>(99);});
    reject([](auto& m){m.generationContexts[0].id="wrong";});
    reject([](auto& m){m.generationContexts.push_back(m.generationContexts.front());});
    reject([](auto& m){m.generationContexts[0].ir.path="../outside.wav";});
    reject([](auto& m){m.generationContexts[0].ir.path="renders/"+juce::String::repeatedString("a",64)+".wav";});
    reject([](auto& m){m.contextSpans[0].context="unknown";});
    reject([](auto& m){m.contextSpans[0].end=5;});
    reject([](auto& m){m.contextSpans[1].start=5;});
    reject([](auto& m){m.contextSpans[0].end=11;m.contextSpans.erase(m.contextSpans.begin()+1);});
    reject([](auto& m){m.contextSpans.clear();});
    reject([](auto& m){m.generationContexts.resize(maximumCaptureContexts+1);});
    reject([](auto& m){m.contextSpans.resize(maximumCaptureContextSpans+1);});
    reject([](auto& m){m.schemaVersion=1;});
    reject([](auto& m){m.schemaVersion=3;});
    auto duplicateSpan=upgraded;duplicateSpan.contextSpans.insert(duplicateSpan.contextSpans.begin()+1,{3,6,duplicateSpan.contextSpans[0].context});duplicateSpan.contextSpans[0].end=3;
    REQUIRE(!isValidCaptureManifest(duplicateSpan,error)); // Adjacent same-owner spans must coalesce.
    auto pendingIR=baseline;pendingIR.ir={};pendingIR.irName.clear();CaptureManifest missingIR;
    REQUIRE(captureWithLegacyContext(legacy,pendingIR,missingIR,error)); // Missing IR never invalidates dry source metadata.
    auto unnamedIR=baseline;unnamedIR.irName.clear();CaptureManifest unnamed;
    REQUIRE(captureWithLegacyContext(legacy,unnamedIR,unnamed,error) && unnamed.generationContexts[0].irName.isEmpty()); // Valid old session labels stay unchanged.
    auto badRoot=juce::JSON::parse(json);(*badRoot["generationContexts"].getArray())[0].getDynamicObject()->setProperty("automatic",1);
    const auto before=parsed.contextSpans;REQUIRE(!parseCaptureManifest(juce::JSON::toString(badRoot),parsed,error) && parsed.contextSpans==before);
    badRoot=juce::JSON::parse(json);badRoot.getDynamicObject()->removeProperty("contextSpans");
    REQUIRE(!parseCaptureManifest(juce::JSON::toString(badRoot),parsed,error) && parsed.contextSpans==before);
    badRoot=juce::JSON::parse(json);badRoot.getDynamicObject()->setProperty("schemaVersion",1);
    REQUIRE(!parseCaptureManifest(juce::JSON::toString(badRoot),parsed,error));
    badRoot=juce::JSON::parse(json);(*badRoot["generationContexts"].getArray())[0].getDynamicObject()->setProperty("sensitivity",1.0000000001);
    REQUIRE(!parseCaptureManifest(juce::JSON::toString(badRoot),parsed,error));
    badRoot=juce::JSON::parse(json);(*badRoot["generationContexts"].getArray())[0].getDynamicObject()->setProperty("unsupportedPolicy",true);
    REQUIRE(!parseCaptureManifest(juce::JSON::toString(badRoot),parsed,error));
    auto invalidBaseline=baseline;invalidBaseline.sensitivity=std::numeric_limits<float>::quiet_NaN();
    REQUIRE(!captureWithLegacyContext(legacy,invalidBaseline,parsed,error) && parsed.contextSpans==before);
    auto oversized=legacy;oversized.schemaVersion=2;oversized.ranges={{0,maximumCaptureContextSpans,CaptureCoverageKind::silence}};
    for(size_t i=0;i<maximumCaptureContexts;++i)
    {
        auto c=pendingIR;c.irName=juce::String::repeatedString("x",1000)+juce::String(static_cast<int>(i));c.id=captureGenerationContextId(c);oversized.generationContexts.push_back(std::move(c));
    }
    for(size_t i=0;i<maximumCaptureContextSpans;++i)oversized.contextSpans.push_back({static_cast<int64_t>(i),static_cast<int64_t>(i+1),oversized.generationContexts[i%maximumCaptureContexts].id});
    REQUIRE(!isValidCaptureManifest(oversized,error) && error.contains("8 MiB"));
    auto oversizedJson=juce::JSON::parse(legacyJson);const auto metadata=captureContextMetadata(oversized);
    oversizedJson.getDynamicObject()->setProperty("schemaVersion",2);
    oversizedJson.getDynamicObject()->setProperty("generationContexts",metadata["generationContexts"]);
    oversizedJson.getDynamicObject()->setProperty("contextSpans",metadata["contextSpans"]);
    REQUIRE(!parseCaptureManifest(juce::JSON::toString(oversizedJson),parsed,error) && error.contains("8 MiB") && parsed.contextSpans==before);
    {
        const auto passRoot=root.file.getChildFile("passes");CaptureSession session(passRoot);
        auto a=baseline;a.id=captureGenerationContextId(a);auto mutableConfiguration=a;
        auto b=a;b.sensitivity=.625f;b.irName="Pass B";b.id=captureGenerationContextId(b);
        REQUIRE(session.beginPass(1,7,false,&mutableConfiguration));mutableConfiguration=b;
        REQUIRE(session.stagePass(1,10,48000,audio,4) && session.stagePass(1,20,48000,zeros,2) && session.stagePass(1,1002,48000,audio,4));
        float replacement[60];for(int i=0;i<30;++i){replacement[i*2]=.8f;replacement[i*2+1]=-.8f;}
        REQUIRE(session.stagePass(1,10,48000,replacement,4)); // First received remains fixed.
        REQUIRE(session.snapshot().manifest.ranges.empty() && session.snapshot().manifest.contextSpans.empty());
        auto result=session.preparePass(1,7);REQUIRE(result.success && session.snapshot().reference.isEmpty());
        REQUIRE(session.acceptPreparedPass(1,7));const auto first=session.snapshot();
        REQUIRE(first.manifest.schemaVersion==2 && first.manifest.generationContexts.size()==1);
        REQUIRE(captureContextAt(first.manifest,10)->id==a.id && captureContextAt(first.manifest,20)->id==a.id && !captureContextAt(first.manifest,900));
        REQUIRE(session.readSample(10,left,right) && left==.25f);
        REQUIRE(session.beginPass(2,7,false,&b) && session.stagePass(2,0,48000,replacement,30));
        result=session.finishPass(2,7);REQUIRE(result.success && result.changedIntervals.size()==3);
        REQUIRE(captureContextAt(session.snapshot().manifest,0)->id==b.id && captureContextAt(session.snapshot().manifest,12)->id==a.id && captureContextAt(session.snapshot().manifest,21)->id==a.id);
        REQUIRE(session.readSample(21,left,right) && left==0); // Protected captured silence owns its original context.
        auto c=b;c.automatic=false;c.id=captureGenerationContextId(c);const float last[]{.3f,-.3f,.4f,-.4f};
        REQUIRE(session.beginPass(3,7,true,&c) && session.stagePass(3,10,48000,replacement,4) && session.stagePass(3,12,48000,last,2));
        REQUIRE(session.readSample(12,left,right) && left==.1f);
        REQUIRE(session.finishPass(3,7).success && session.readSample(12,left,right) && left==.3f);
        REQUIRE(captureContextAt(session.snapshot().manifest,13)->id==c.id && captureContextAt(session.snapshot().manifest,14)->id==b.id);
        const auto beforeMetadata=session.snapshot();const auto scope=passRoot.getChildFile(beforeMetadata.manifest.projectId+"/"+beforeMetadata.manifest.instanceId);
        CaptureAssetStore assets(scope);juce::String beforeFingerprint,afterFingerprint;
        REQUIRE(m3::sourceFingerprint(assets,beforeMetadata.manifest,0,30,beforeFingerprint,error));
        const auto pcmFiles=scope.findChildFiles(juce::File::findFiles,true,"*.wav").size();
        const float same[]{.8f,-.8f,.8f,-.8f,.3f,-.3f,.4f,-.4f};
        REQUIRE(session.beginPass(4,7,true,&b) && session.stagePass(4,10,48000,same,4));
        result=session.finishPass(4,7);REQUIRE(result.success && result.changed && result.changedIntervals.empty() && result.contextChangedIntervals.size()==1);
        REQUIRE(captureContextAt(session.snapshot().manifest,10)->id==b.id && session.snapshot().reference!=beforeMetadata.reference);
        REQUIRE(m3::sourceFingerprint(assets,session.snapshot().manifest,0,30,afterFingerprint,error) && beforeFingerprint==afterFingerprint);
        REQUIRE(sourceDependency(beforeMetadata.manifest,0,30)==sourceDependency(session.snapshot().manifest,0,30));
        REQUIRE(scope.findChildFiles(juce::File::findFiles,true,"*.wav").size()==pcmFiles);
        const auto stable=session.snapshot();REQUIRE(session.beginPass(5,7,true,&b) && session.stagePass(5,10,48000,same,4));
        result=session.finishPass(5,7);REQUIRE(result.success && !result.changed && session.snapshot().reference==stable.reference);
        REQUIRE(session.beginPass(6,7,true,&c) && session.stagePass(6,10,48000,replacement,4));session.abortPass();
        REQUIRE(session.snapshot().reference==stable.reference && captureContextAt(session.snapshot().manifest,10)->id==b.id);
        const auto oldManifestHash=m3::fileHash(scope.getChildFile(stable.reference));
        const auto segments=scope.getChildFile("segments"),heldSegments=scope.getChildFile("held-segments");
        REQUIRE(segments.moveFileTo(heldSegments) && segments.replaceWithText("blocked"));
        REQUIRE(session.beginPass(7,7,false,&c));
        const auto sourceWriteFailed=!session.stagePass(7,2000,48000,replacement,4);
        REQUIRE(segments.deleteFile() && heldSegments.moveFileTo(segments));
        REQUIRE(sourceWriteFailed && !session.finishPass(7,7).success && session.snapshot().reference==stable.reference);
        REQUIRE(session.beginPass(8,7,false,&c) && session.stagePass(8,2000,48000,replacement,4));
        const auto manifests=scope.getChildFile("manifests"),heldManifests=scope.getChildFile("held-manifests");
        REQUIRE(manifests.moveFileTo(heldManifests) && manifests.replaceWithText("blocked"));
        const auto metadataWriteFailed=!session.finishPass(8,7).success;
        REQUIRE(manifests.deleteFile() && heldManifests.moveFileTo(manifests));
        REQUIRE(metadataWriteFailed && session.snapshot().reference==stable.reference && !captureContextAt(session.snapshot().manifest,2000));
        REQUIRE(m3::fileHash(scope.getChildFile(stable.reference))==oldManifestHash && session.readSample(12,left,right) && left==.3f);
        REQUIRE(session.beginPass(9,7,true,&c) && session.stagePass(9,10,48000,replacement,4));
        REQUIRE(session.preparePass(9,7).success && session.snapshot().reference==stable.reference);
        REQUIRE(session.restore(stable.manifest.projectId,stable.manifest.instanceId,stable.reference,stable.manifest.revision));
        REQUIRE(!session.acceptPreparedPass(9,7) && captureContextAt(session.snapshot().manifest,10)->id==b.id);
        CaptureSession copy(passRoot);REQUIRE(copy.restore(stable.manifest.projectId,stable.manifest.instanceId,stable.reference,stable.manifest.revision));
        REQUIRE(copy.beginPass(1,0,true,&c) && copy.stagePass(1,10,48000,replacement,4) && copy.finishPass(1,0).success);
        REQUIRE(captureContextAt(copy.snapshot().manifest,10)->id==c.id && captureContextAt(session.snapshot().manifest,10)->id==b.id);
        REQUIRE(session.snapshot().reference==stable.reference && m3::fileHash(scope.getChildFile(stable.reference))==oldManifestHash);
        REQUIRE(!session.commit(2000,48000,replacement,4) && session.snapshot().reference==stable.reference); // Contextless writes cannot corrupt an owned map.
        REQUIRE(session.clear(48000) && session.snapshot().manifest.contextSpans.empty() && session.snapshot().manifest.generationContexts.empty());
    }
});}
