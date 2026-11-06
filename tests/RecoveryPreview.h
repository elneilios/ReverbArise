#pragma once
#include <UI/RecoveryPanel.h>
static juce::Component* recoveryFind(juce::Component& root,const juce::String& name)
{if(root.getName()==name)return &root;for(auto* child:root.getChildren())if(auto* found=recoveryFind(*child,name))return found;return nullptr;}
static void recoveryPreview(bool review)
{
    Fixture fixture;fixture.session.swells[0].shape.endOffset=2;fixture.session.swells[0].supportOffset=0;fixture.session.autoGenerateOnStop=true;fixture.refresh();
    TestDirectory backup,destination;REQUIRE(backup.file.createDirectory().wasOk() && destination.file.createDirectory().wasOk());juce::String error;
    std::shared_ptr<m3::storage::portable::Prepared> prepared;REQUIRE(m3::storage::portable::prepare({fixture.root,fixture.xml,fixture.manifest},prepared,error));
    const auto packed=m3::storage::portable::backup(prepared,backup.file);REQUIRE(packed.success);prepared.reset();
    auto processor=std::make_unique<PreverbAudioProcessor>(destination.file);load(*processor,fixture.xml);
    ProfileHost host;host.position.setTimeInSamples(0);host.position.setPpqPosition(0.0);host.position.setBpm(120);host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});host.position.setIsPlaying(false);
    processor->setPlayHead(&host);processor->prepareToPlay(48000,6);juce::AudioBuffer<float> audio(2,6);audio.clear();juce::MidiBuffer midi;processor->processBlock(audio,midi);
    m3::ui::WorkspaceTheme theme;
    m3::ui::RecoveryPanel panel([&](auto& scope,auto& why){return processor->pinPortableAssets(scope,why,true);},destination.file,
        [&](const auto& scope,auto& why){return processor->reloadRecoveredAssets(scope,why);},
        [&](const auto& scope,auto& why){return processor->beginRelearning(scope,why);},[&]{processor->cancelRelearning();},
        [&](bool consent,auto& why,bool check){return processor->regenerateRecovered(consent,why,check);});
    panel.setLookAndFeel(&theme);
    struct Window:juce::DocumentWindow{Window():DocumentWindow("Missing-asset recovery - disposable fixture review",m3::ui::background,closeButton){}void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}} window;
    window.setUsingNativeTitleBar(true);window.setContentNonOwned(&panel,false);window.centreWithSize(680,700);window.setVisible(true);
    const auto press=[&](const juce::String& name){auto* button=dynamic_cast<juce::Button*>(recoveryFind(panel,name));REQUIRE(button && button->isEnabled());button->onClick();};
    auto& storage=panel.storagePanel();
    const auto pump=[&](auto predicate){const auto end=juce::Time::getMillisecondCounterHiRes()+20000;while(!predicate()){if(juce::Time::getMillisecondCounterHiRes()>=end)throw std::runtime_error(storage.statusText().toStdString());juce::MessageManager::getInstance()->runDispatchLoopUntil(10);panel.poll();}};
    storage.receiptPicker=[&](const auto&,const auto&,auto completed){completed(backup.file.getChildFile("receipts/"+packed.receipt.locator.snapshot+".json"));};
    press("restoreAssets");pump([&]{return storage.statusText().contains("source reloaded");});REQUIRE(!processor->getCaptureSnapshot().unavailable);
    const auto old=processor->getCaptureSnapshot();const auto renders=processor->swellEngine().snapshot().renderCount;
    press("relearnSong");press("relearnSong");REQUIRE(processor->isRelearning());press("cancelRelearning");REQUIRE(!processor->isRelearning() && processor->getCaptureSnapshot().reference==old.reference);
    // Complete a real callback/worker recovery pass with unchanged saved policy.
    press("relearnSong");press("relearnSong");juce::Thread::sleep(10);
    host.position.setIsPlaying(true);audio.setSample(0,0,.25f);audio.setSample(1,0,-.25f);processor->processBlock(audio,midi);
    host.position.setIsPlaying(false);processor->processBlock(audio,midi);REQUIRE(processor->flushCaptureForTesting());
    pump([&]{return !processor->isRelearning();});REQUIRE(processor->getCaptureSnapshot().reference!=old.reference);
    REQUIRE(processor->swellEngine().snapshot().renderCount==renders && processor->swellEngine().savedSession().autoGenerateOnStop);
    // Surviving frozen output is kept; eligibility validation does not render.
    REQUIRE(processor->regenerateRecovered(false,error,true));REQUIRE(processor->swellEngine().snapshot().renderCount==renders);
    processor->cancelRelearning();
    // Return to a missing-source scene for owner inspection; all data is disposable.
    const auto currentRoot=processor->swellEngine().assetDirectory();const auto offline=currentRoot.getSiblingFile(currentRoot.getFileName()+"-offline");waitFor([&]{return currentRoot.moveFileTo(offline);});load(*processor,fixture.xml);
    storage.receiptPicker={};storage.setKnownBackup(packed.receipt.locator,backup.file);panel.poll();
    const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_RECOVERY_PREVIEW_OUTPUT",{});if(output.isNotEmpty()){auto stream=juce::File(output).createOutputStream();REQUIRE(stream && stream->setPosition(0) && stream->truncate().wasOk() && juce::PNGImageFormat().writeImageToStream(panel.createComponentSnapshot(panel.getLocalBounds()),*stream));}
    if(review)juce::MessageManager::getInstance()->runDispatchLoop();
    window.setLookAndFeel(nullptr);panel.setLookAndFeel(nullptr);processor->setPlayHead(nullptr);
    std::cout<<"Recovery UI: missing-source Restore/reload, explicit re-learn confirmation/cancel, callback capture and suppressed automatic generation passed.\n";
}
