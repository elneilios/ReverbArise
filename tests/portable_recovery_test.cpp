#include "PortableFixture.h"
#include <Plugin/PluginProcessor.h>
#include <Storage/PortableAssets.h>
using namespace m3::storage::portable;
template<class F>static void waitFor(F predicate){const auto end=juce::Time::getMillisecondCounterHiRes()+15000;while(!predicate()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<end);juce::Thread::sleep(2);}}
static void load(PreverbAudioProcessor& p,const juce::String& xml){auto tree=juce::XmlDocument::parse(xml);REQUIRE(tree);juce::MemoryBlock block;juce::AudioProcessor::copyXmlToBinary(*tree,block);p.setStateInformation(block.getData(),static_cast<int>(block.getSize()));REQUIRE(p.flushCaptureForTesting());}
#include "PortableProfileValidation.h"
#include "RecoveryPreview.h"
int main(int argc,char** argv){if(argc==4 && juce::String(argv[1])=="--fresh-profile")initialiseProfileEnvironment();juce::ScopedJuceInitialiser_GUI gui;if(argc==2 && juce::String(argv[1])=="--review")return runTest([&]{recoveryPreview(true);});if(argc==2 && juce::String(argv[1])=="--ui")return runTest([&]{recoveryPreview(false);});if(argc==3 && juce::String(argv[1])=="--prepare-profile")return runTest([&]{profilePrepare(juce::File(argv[2]));});if(argc==4 && juce::String(argv[1])=="--fresh-profile")return runTest([&]{profileValidate(juce::File(argv[2]),argv[3]);});return runTest([]{
    Fixture fixture;juce::String error;std::shared_ptr<Prepared> pin;
    REQUIRE(prepare({fixture.root,fixture.xml,fixture.manifest},pin,error));
    TestDirectory backup,destination;REQUIRE(backup.file.createDirectory().wasOk() && destination.file.createDirectory().wasOk());
    const auto packed=m3::storage::portable::backup(pin,backup.file);REQUIRE(packed.success);pin.reset();
    std::shared_ptr<Verified> verified;REQUIRE(locateReceipt(backup.file.getChildFile("receipts/"+packed.receipt.locator.snapshot+".json"),verified,error));
    // Preserve a newer creative edit instead of adopting older backup metadata.
    fixture.session.swells[0].name="Edited after backup";fixture.session.swells[0].shape.gainDb=-7;fixture.session.autoGenerateOnStop=true;fixture.refresh();
    auto owner=std::make_unique<PreverbAudioProcessor>(destination.file);auto& processor=*owner;load(processor,fixture.xml);
    REQUIRE(processor.getCaptureSnapshot().unavailable);Input target;
    REQUIRE(!processor.pinPortableAssets(target,error));
    waitFor([&]{return processor.pinPortableAssets(target,error,true);});REQUIRE(target.sourceUnavailable);
    const auto before=processor.swellEngine().savedSession();const auto source=processor.getCaptureSnapshot();const auto renders=processor.swellEngine().snapshot().renderCount;
    REQUIRE(before.swells.size()==1 && before.swells[0].id==fixture.session.swells[0].id && before.swells[0].frozen);
    REQUIRE(!prepare(target,pin,error));REQUIRE(matchesRecoveryTarget(*verified,target,error));
    auto wrong=target;auto tree=juce::XmlDocument::parse(target.processorXml);REQUIRE(tree);tree->setAttribute("manifestRevision",juce::String(static_cast<juce::int64>(source.manifest.revision+1)));wrong.processorXml=tree->toString();REQUIRE(!matchesRecoveryTarget(*verified,wrong,error));
    Hooks cancelled;cancelled.cancelled=[]{return true;};REQUIRE(!restoreCopy(verified,destination.file,cancelled).success);
    REQUIRE(processor.getCaptureSnapshot().reference==source.reference && processor.getCaptureSnapshot().unavailable);
    REQUIRE(restoreCopy(verified,destination.file).success);REQUIRE(processor.reloadRecoveredAssets(target,error));REQUIRE(processor.flushCaptureForTesting());
    waitFor([&]{return processor.pinPortableAssets(target,error);});REQUIRE(!target.sourceUnavailable);
    const auto after=processor.swellEngine().savedSession();
    REQUIRE(after.swells[0].name==before.swells[0].name && after.swells[0].shape==before.swells[0].shape && after.swells[0].frozen);
    REQUIRE(after.swells[0].id==before.swells[0].id && after.swells[0].generationSource==before.swells[0].generationSource);
    REQUIRE(after.defaultIR==before.defaultIR && after.autoGenerateOnStop==before.autoGenerateOnStop && after.exportState.history.size()==before.exportState.history.size());
    REQUIRE(processor.getCaptureSnapshot().reference==source.reference && processor.getCaptureSnapshot().manifest.contextSpans==fixture.observed.capture.contextSpans);
    REQUIRE(processor.swellEngine().snapshot().renderCount==renders);
    const auto restored=processor.swellEngine().assetDirectory();for(const auto& e:snapshot(*verified).files)REQUIRE(fileHash(restored.getChildFile(e.path))==e.sha256);
    // Failed reload is transactional: keep the previously accepted source/map.
    // Original legacy folders use the same exact closure validation, without a
    // portable association or receipt. Cancellation also covers same-root copies.
    std::shared_ptr<Prepared> original;REQUIRE(prepareLegacyRecovery(fixture.root,target,original,error));
    TestDirectory legacyDestination;REQUIRE(legacyDestination.file.createDirectory().wasOk());
    const auto legacy=restoreLegacyCopy(original,legacyDestination.file);REQUIRE(legacy.success && legacy.receipt.locator.store.isEmpty());
    for(const auto& entry:snapshot(*verified).files)REQUIRE(fileHash(legacyDestination.file.getChildFile(fixture.observed.project).getChildFile(fixture.observed.instance).getChildFile(entry.path))==entry.sha256);
    REQUIRE(!restoreLegacyCopy(original,fixture.directory.file,cancelled).success);
    std::shared_ptr<Prepared> wrongOriginal;REQUIRE(!prepareLegacyRecovery(backup.file,target,wrongOriginal,error));
    original.reset();
    const auto manifest=restored.getChildFile(source.reference),offline=manifest.getSiblingFile("offline.json");REQUIRE(manifest.moveFileTo(offline));
    const auto accepted=processor.getCaptureSnapshot();REQUIRE(processor.reloadRecoveredAssets(target,error));REQUIRE(processor.flushCaptureForTesting());
    const auto retained=processor.getCaptureSnapshot();REQUIRE(retained.reference==accepted.reference && retained.manifest.ranges.size()==accepted.manifest.ranges.size() && sourceDependency(retained.manifest,0,6)==sourceDependency(accepted.manifest,0,6) && !retained.unavailable && retained.status.contains("Recovery reload failed"));
    REQUIRE(offline.moveFileTo(manifest));
    // Explicit re-learning stages fresh source without clearing accepted history.
    TestDirectory relearnRoot;CaptureSession relearn(relearnRoot.file);
    REQUIRE(!relearn.restore(fixture.observed.project.toStdString(),fixture.observed.instance.toStdString(),fixture.observed.reference,fixture.observed.capture.revision));
    const auto prior=relearn.snapshot();const std::vector<std::pair<int64_t,int64_t>> required{{0,2},{4,6}};
    const auto context=fixture.observed.capture.generationContexts.front();const float fresh[]{.2f,-.3f,.4f,-.5f,0,0,0,0,.6f,-.7f,.8f,-.9f};
    REQUIRE(relearn.beginRecoveryPass(1,1,required,&context));REQUIRE(relearn.stagePass(1,0,48000,fresh,2));
    REQUIRE(!relearn.finishPass(1,1).success && relearn.snapshot().reference==prior.reference && relearn.snapshot().unavailable);
    REQUIRE(relearn.beginRecoveryPass(2,1,required,&context));REQUIRE(relearn.stagePass(2,0,48000,fresh,6));relearn.abortPass();
    REQUIRE(relearn.snapshot().reference==prior.reference && relearn.snapshot().unavailable);
    REQUIRE(relearn.beginRecoveryPass(3,1,required,&context));const float invalid[]{std::numeric_limits<float>::infinity(),0};
    REQUIRE(!relearn.stagePass(3,0,48000,invalid,1));REQUIRE(!relearn.finishPass(3,1).success && relearn.snapshot().reference==prior.reference);
    REQUIRE(relearn.beginRecoveryPass(4,1,required,&context));REQUIRE(relearn.stagePass(4,0,48000,fresh,6));REQUIRE(relearn.finishPass(4,1).success);
    REQUIRE(!relearn.snapshot().unavailable && relearn.snapshot().manifest.revision==prior.manifest.revision+1 && relearn.snapshot().reference!=prior.reference);
    // Another accepted session fences stale reload completions.
    Fixture another;load(processor,another.xml);REQUIRE(!processor.reloadRecoveredAssets(target,error));
    REQUIRE(processor.getCaptureSnapshot().manifest.instanceId==another.observed.instance.toStdString());
    const auto preserved=processor.swellEngine().savedSession();load(processor,"<PreverbState stateVersion=\"999\"/>");
    REQUIRE(!processor.pinPortableAssets(target,error,true) && error.contains("could not be decoded"));REQUIRE(processor.swellEngine().savedSession().swells[0].id==preserved.swells[0].id);
    std::cout<<"Recovery: missing-source admission, exact target closure, cancel preservation, transactional reload, retained creative/frozen policy, no rendering and rejected-state diagnostics passed.\n";
});}
