#pragma once
#include <UI/Controls/OwnedAlertWindow.h>
#include <Plugin/PluginEditor.h>
static juce::Component* recoveryFind(juce::Component& root,const juce::String& name)
{if(root.getName()==name)return &root;for(auto* child:root.getChildren())if(auto* found=recoveryFind(*child,name))return found;return nullptr;}
static void recoveryPreview(bool review,bool responsive=true)
{
    Fixture fixture;fixture.session.swells[0].shape.endOffset=2;fixture.session.swells[0].shape.fadeIn=fixture.session.swells[0].shape.fadeOut=0;
    fixture.session.swells[0].frozen=false;fixture.session.automaticProcessing=false;fixture.session.autoMarkersEnabled=false;fixture.session.autoGenerateOnStop=false;fixture.refresh();
    TestDirectory backup,destination;REQUIRE(backup.file.createDirectory().wasOk() && destination.file.createDirectory().wasOk());juce::String error;
    std::shared_ptr<m3::storage::portable::Prepared> prepared;REQUIRE(m3::storage::portable::prepare({fixture.root,fixture.xml,fixture.manifest},prepared,error));
    const auto packed=m3::storage::portable::backup(prepared,backup.file);REQUIRE(packed.success);prepared.reset();
    const auto retainedRoot=destination.file.getChildFile(fixture.observed.project).getChildFile(fixture.observed.instance);REQUIRE(retainedRoot.createDirectory().wasOk());REQUIRE(fixture.root.getChildFile("irs").copyDirectoryTo(retainedRoot.getChildFile("irs")));
    auto processor=std::make_unique<PreverbAudioProcessor>(destination.file);load(*processor,fixture.xml);
    ProfileHost host;host.position.setTimeInSamples(0);host.position.setPpqPosition(0.0);host.position.setBpm(120);host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});host.position.setIsPlaying(false);
    processor->setPlayHead(&host);processor->prepareToPlay(48000,6);juce::AudioBuffer<float> audio(2,6);audio.clear();juce::MidiBuffer midi;processor->processBlock(audio,midi);
    auto editor=std::make_unique<PreverbAudioProcessorEditor>(*processor);if(responsive)editor->enableResponsiveReview();
    struct Window:juce::DocumentWindow{Window():DocumentWindow("Missing assets - normal Learn recovery preview",m3::ui::background,closeButton){}void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}} window;
    window.setUsingNativeTitleBar(true);window.setContentNonOwned(editor.get(),false);window.centreWithSize(1100,850);window.setVisible(true);
    auto& tabs=*editor->settingsPanel();
    auto* viewport=dynamic_cast<m3::ui::controls::FocusViewport*>(tabs.page(m3::ui::SettingsPage::storage));REQUIRE(viewport);
    auto* storage=dynamic_cast<m3::ui::PortableStoragePanel*>(viewport->getViewedComponent());REQUIRE(storage);
    juce::String stage="opening warning";
    const auto pump=[&](auto predicate){const auto end=juce::Time::getMillisecondCounterHiRes()+20000;while(!predicate()){if(juce::Time::getMillisecondCounterHiRes()>=end)throw std::runtime_error((stage+" | "+processor->getCaptureSnapshot().status+" | "+processor->getCaptureSnapshot().failure.detail+" | "+processor->swellEngine().snapshot().learnGeneration.reason+" | "+processor->swellEngine().savedSession().swells[0].status).toStdString());juce::MessageManager::getInstance()->runDispatchLoopUntil(10);editor->refresh();}};
    pump([&]{return editor->assetRecoveryWarningVisible();});
    auto* warning=dynamic_cast<juce::AlertWindow*>(juce::Component::getCurrentlyModalComponent());REQUIRE(warning && warning->getComponentID()=="missingAssetRecoveryDialog");
    REQUIRE(warning->getDescription().contains("Detect events off") && warning->getDescription().contains("markers and edits are preserved"));
    const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_RECOVERY_PREVIEW_OUTPUT",{});
    if(output.isNotEmpty()){auto stream=juce::File(output).createOutputStream();REQUIRE(stream && stream->setPosition(0) && stream->truncate().wasOk() && juce::PNGImageFormat().writeImageToStream(warning->createComponentSnapshot(warning->getLocalBounds()),*stream));}
    if(review){juce::MessageManager::getInstance()->runDispatchLoop();window.setContentNonOwned(nullptr,false);editor.reset();processor->setPlayHead(nullptr);return;}
    REQUIRE(warning->isCurrentlyModal() && warning->isOnDesktop() && warning->isUsingNativeTitleBar() && editor->isCurrentlyBlockedByAnotherModalComponent());
    auto* initial=dynamic_cast<m3::ui::controls::ActionButton*>(juce::Component::getCurrentlyFocusedComponent());
    REQUIRE(initial && initial->getButtonText()=="Continue");
    const auto unavailableReference=processor->getCaptureSnapshot().reference;
    int cancelledPicks=0;
    storage->folderPicker=[&](const auto&,const auto&,auto complete){++cancelledPicks;complete({});};
    auto* locate=dynamic_cast<m3::ui::controls::ActionButton*>(warning->getCustomComponent(0)->getChildComponent(0));
    REQUIRE(locate && locate->getButtonText()=="Locate assets...");
    locate->grabKeyboardFocus();REQUIRE(locate->keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));
    stage="cancelled popup Locate";pump([&]{return cancelledPicks==1;});
    REQUIRE(!storage->busy() && !storage->reloadPending() && !editor->assetRecoveryWarningVisible());
    REQUIRE(processor->getCaptureSnapshot().unavailable && processor->getCaptureSnapshot().reference==unavailableReference);
    load(*processor,fixture.xml);processor->processBlock(audio,midi);editor->refresh();
    stage="reopen warning after cancelled picker";pump([&]{return editor->assetRecoveryWarningVisible();});
    warning=dynamic_cast<juce::AlertWindow*>(juce::Component::getCurrentlyModalComponent());REQUIRE(warning);
    // The popup routes to the actual original-folder picker and transactional
    // reload, then a later missing-source session can select an exact receipt.
    storage->folderPicker=[&](const auto&,const auto&,auto complete){complete(fixture.root);};
    warning->exitModalState(1);warning->setVisible(false);
    stage="popup Locate";pump([&]{return storage->statusText().contains("source reloaded") && !processor->getCaptureSnapshot().unavailable && !storage->reloadPending();});REQUIRE(!processor->getCaptureSnapshot().unavailable);
    REQUIRE(tabs.selectedPage()==m3::ui::SettingsPage::storage && viewport->isShowing());
    auto restoredRoot=processor->swellEngine().assetDirectory();auto sourceFile=restoredRoot.getChildFile(fixture.observed.reference);
    waitFor([&]{return sourceFile.moveFileTo(sourceFile.getSiblingFile("source-offline-one.json"));});load(*processor,fixture.xml);processor->processBlock(audio,midi);editor->refresh();
    pump([&]{return editor->assetRecoveryWarningVisible();});warning=dynamic_cast<juce::AlertWindow*>(juce::Component::getCurrentlyModalComponent());REQUIRE(warning);
    storage->receiptPicker=[&](const auto&,const auto&,auto complete){complete(backup.file.getChildFile("receipts/"+packed.receipt.locator.snapshot+".json"));};
    warning->exitModalState(2);warning->setVisible(false);
    stage="popup Restore";pump([&]{return storage->statusText().contains("source reloaded") && !processor->getCaptureSnapshot().unavailable && !storage->reloadPending();});REQUIRE(!processor->getCaptureSnapshot().unavailable);
    sourceFile=processor->swellEngine().assetDirectory().getChildFile(fixture.observed.reference);
    waitFor([&]{return sourceFile.moveFileTo(sourceFile.getSiblingFile("source-offline-two.json"));});load(*processor,fixture.xml);processor->processBlock(audio,midi);editor->refresh();
    pump([&]{return editor->assetRecoveryWarningVisible();});warning=dynamic_cast<juce::AlertWindow*>(juce::Component::getCurrentlyModalComponent());REQUIRE(warning);
    // Continue leaves the arrangement unchanged and does not repeat the warning.
    static_cast<m3::ui::controls::OwnedAlertWindow*>(warning)->userTriedToCloseWindow();juce::MessageManager::getInstance()->runDispatchLoopUntil(10);editor->refresh();REQUIRE(!editor->assetRecoveryWarningVisible());
    const auto old=processor->getCaptureSnapshot();const auto before=processor->swellEngine().savedSession();
    // Use the normal Learn Recapture control; partial source cannot publish.
    if(!processor->setRecaptureExisting(true))throw std::runtime_error(processor->swellEngine().snapshot().message.toStdString());juce::AudioBuffer<float> partial(2,1);partial.clear();partial.setSample(0,0,.5f);
    REQUIRE(setFixtureCaptureArmed(*processor));host.position.setIsPlaying(true);processor->processBlock(partial,midi);host.position.setIsPlaying(false);processor->processBlock(partial,midi);REQUIRE(processor->flushCaptureForTesting());
    REQUIRE(processor->getCaptureSnapshot().reference==old.reference && processor->getCaptureSnapshot().unavailable);
    processor->cancelCapturePass();if(!processor->setRecaptureExisting(true))throw std::runtime_error(processor->swellEngine().snapshot().message.toStdString());
    REQUIRE(setFixtureCaptureArmed(*processor));audio.setSample(0,0,.5f);audio.setSample(1,0,-.5f);host.position.setIsPlaying(true);processor->processBlock(audio,midi);host.position.setIsPlaying(false);processor->processBlock(audio,midi);REQUIRE(processor->flushCaptureForTesting());
    // No Generate action: complete recovery rebuilds saved swells automatically.
    stage="saved swell generation";pump([&]{return processor->swellEngine().savedSession().swells[0].phase==m3::Phase::ready;});
    const auto after=processor->swellEngine().savedSession();REQUIRE(after.swells.size()==before.swells.size() && after.swells[0].id==before.swells[0].id && after.swells[0].shape==before.swells[0].shape && after.swells[0].name==before.swells[0].name);
    REQUIRE(!after.autoMarkersEnabled && !after.automaticProcessing && !after.autoGenerateOnStop);
    stage="stale popup";
    load(*processor,fixture.xml);processor->processBlock(audio,midi);editor->refresh();pump([&]{return editor->assetRecoveryWarningVisible();});
    warning=dynamic_cast<juce::AlertWindow*>(juce::Component::getCurrentlyModalComponent());REQUIRE(warning);
    int stalePicks=0;storage->folderPicker=[&](const auto&,const auto&,auto){++stalePicks;};Fixture another;load(*processor,another.xml);
    warning->exitModalState(1);warning->setVisible(false);juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    REQUIRE(stalePicks==0 && processor->getCaptureSnapshot().manifest.instanceId==another.observed.instance.toStdString());
    stage="metadata warning";
    // Missing metadata is a separate warning and never claims intact edits.
    load(*processor,"<PreverbState stateVersion=\"999\"/>");
    // Keep the same editor: a prior asset warning must not suppress or mask
    // the stricter metadata-decode failure warning.
    editor->refresh();
    pump([&]{return editor->assetRecoveryWarningVisible();});warning=dynamic_cast<juce::AlertWindow*>(juce::Component::getCurrentlyModalComponent());REQUIRE(warning && warning->getDescription().contains("cannot be confirmed intact"));
    REQUIRE(warning->getNumCustomComponents()==1 && warning->getCustomComponent(0)->getNumChildComponents()==1);
    REQUIRE(static_cast<m3::ui::controls::OwnedAlertWindow*>(warning)->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey)));
    warning->exitModalState(0);warning->setVisible(false);window.setContentNonOwned(nullptr,false);editor.reset();processor->setPlayHead(nullptr);
    std::cout<<"Recovery UI: one editor-open warning, preserved metadata, normal Recapture without Generate, fixed/responsive modality and stale callback fences, complete-only recovery and decode-failure distinction passed.\n";
}
