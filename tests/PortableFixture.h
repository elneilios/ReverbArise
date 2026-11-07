#pragma once
#define NOMINMAX
#include "AudioTestFiles.h"
#include <Storage/AssetInventory.h>
#include <Capture/CaptureSession.h>
#include <DSP/SwellAssetStore.h>
#include <Core/ContentHash.h>


using namespace m3;
using namespace m3::storage;
struct Fixture
{
    TestDirectory directory;CaptureSession capture{directory.file};juce::File root;
    Session session;std::vector<Asset> irs;ObservedSnapshot observed;juce::String xml,manifest,error;
    Fixture()
    {
        const auto ids=capture.snapshot().manifest;root=directory.file.getChildFile(ids.projectId).getChildFile(ids.instanceId);SwellAssetStore store(root);
        for(int i=0;i<4;++i){ImpulseResponse ir;ir.channels=2;ir.rate=48000;ir.samples={.1f+i*.01f,-.1f,0,0};Asset a;REQUIRE(store.writeIR(ir,a,error));irs.push_back(a);}
        CaptureGenerationContext context;context.ir=irs[0];context.irName="Historical IR";context.automatic=false;context.id=captureGenerationContextId(context);
        const float audio[]{.1f,-.2f,.3f,-.4f},silence[4]{};
        REQUIRE(capture.commit(0,48000,audio,2,0,&context));REQUIRE(capture.commit(4,48000,silence,2,0,&context));
        session.defaultIR=irs[1];session.importedIRs.push_back({irs[2],"Unused retained library entry"});session.timing={true,0,0,48000,120,4,4};
        Swell w;w.name="Retained disabled/frozen";w.arrival=0;w.sourceStart=0;w.sourceEnd=2;w.shape=initialShape(48000);w.enabled=false;w.frozen=true;w.ir=irs[3];w.generationSource=juce::String::repeatedString("a",64);
        WetRender wet;wet.rate=48000;wet.start=-1;wet.sourceStart=0;wet.sourceEnd=2;wet.samples={0,0,.1f,-.2f,.3f,-.4f};
        REQUIRE(store.writeRender(wet,w.ir,"source",session.timing,w.render,error));session.swells.push_back(w);
        REQUIRE(root.getChildFile("exports").createDirectory().wasOk());const auto exported=root.getChildFile("exports/Swells-old.wav");writeTestWav(exported,2,48000,audioVector());
        ExportFile file;file.path="exports/Swells-old.wav";file.checksum=fileHash(exported);file.composition=juce::String::repeatedString("b",64);file.rate=48000;file.frames=2;file.dry=.35f;file.wet=.65f;file.gainLaw=1;
        session.exportState.initialized=true;session.exportState.history.push_back(file);session.exportState.completedPath=file.path;
        refresh();
    }
    static std::vector<float> audioVector(){return {.1f,-.2f,.3f,-.4f};}
    void refresh(int envelope=8)
    {
        const auto snapshot=capture.snapshot();REQUIRE(serializeCaptureManifest(snapshot.manifest,manifest,error));juce::String creative;REQUIRE(encode(session,creative,error));
        juce::XmlElement state("PreverbState");state.setAttribute("stateVersion",envelope);state.setAttribute("workflowMode","Capture");state.setAttribute("projectId",juce::String(snapshot.manifest.projectId));state.setAttribute("instanceId",juce::String(snapshot.manifest.instanceId));
        state.setAttribute("manifestReference",snapshot.reference);state.setAttribute("manifestRevision",juce::String(static_cast<juce::int64>(snapshot.manifest.revision)));state.createNewChildElement("SwellSession")->addTextElement(creative);xml=state.toString();
        observed.label="Saved fixture";REQUIRE(readLegacySnapshot(xml,manifest,observed,error));
    }
    Request request(){Request r;r.roots={root};r.snapshots={observed};return r;}
};
