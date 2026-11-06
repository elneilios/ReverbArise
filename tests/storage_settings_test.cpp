#include "AudioTestFiles.h"
#include <Plugin/PluginEditor.h>
#include <Core/ContentHash.h>
#include <UI/WorkflowHeader.h>
static juce::Component* find(juce::Component& root,const juce::String& name)
{if(root.getName()==name)return &root;for(auto* child:root.getChildren())if(auto* result=find(*child,name))return result;return nullptr;}
template<class F>static void wait(F predicate)
{const auto end=juce::Time::getMillisecondCounterHiRes()+30000;while(!predicate()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<end);juce::MessageManager::getInstance()->runDispatchLoopUntil(10);}}
static juce::MemoryBlock stateFor(const CaptureSessionSnapshot& capture)
{
    juce::XmlElement xml("PreverbState");xml.setAttribute("stateVersion",8);xml.setAttribute("workflowMode","Capture");
    xml.setAttribute("projectId",juce::String(capture.manifest.projectId));xml.setAttribute("instanceId",juce::String(capture.manifest.instanceId));
    xml.setAttribute("manifestReference",capture.reference);xml.setAttribute("manifestRevision",juce::String(static_cast<juce::int64>(capture.manifest.revision)));
    m3::Session session;juce::String encoded,error;REQUIRE(m3::encode(session,encoded,error));xml.createNewChildElement("SwellSession")->addTextElement(encoded);
    juce::MemoryBlock bytes;juce::AudioProcessor::copyXmlToBinary(xml,bytes);return bytes;
}
int main(int argc,char** argv){juce::ScopedJuceInitialiser_GUI gui;return runTest([&]{
    TestDirectory root;CaptureSession source(root.file);std::vector<float> audio(96000,.125f);REQUIRE(source.commit(0,48000,audio.data(),48000));const auto captured=source.snapshot();
    auto processorOwner=std::make_unique<PreverbAudioProcessor>(root.file);auto& processor=*processorOwner;const auto bytes=stateFor(captured);processor.setStateInformation(bytes.getData(),static_cast<int>(bytes.getSize()));REQUIRE(processor.flushCaptureForTesting());
    wait([&]{return processor.swellEngine().savedSession().defaultIR.present() && !processor.swellEngine().snapshot().defaultIRPending;});
    auto editorOwner=std::make_unique<PreverbAudioProcessorEditor>(processor);auto& editor=*editorOwner;auto& tabs=dynamic_cast<juce::TabbedComponent&>(*editor.settingsPanel());
    REQUIRE(tabs.getNumTabs()==7 && tabs.getTabNames()[5]=="About" && tabs.getTabNames()[6]=="Storage");
    auto* initialViewport=dynamic_cast<m3::ui::controls::FocusViewport*>(tabs.getTabContentComponent(6));REQUIRE(initialViewport);auto* storage=dynamic_cast<m3::ui::PortableStoragePanel*>(initialViewport->getViewedComponent());REQUIRE(storage && !storage->scanning() && !storage->displayedReport());
    editor.enableResponsiveReview();REQUIRE(tabs.getNumTabs()==8 && tabs.getTabNames()[6]=="Appearance" && tabs.getTabNames()[7]=="Storage");
    auto* header=dynamic_cast<m3::ui::WorkflowHeader*>(find(editor,"workflowHeader"));REQUIRE(header && header->onSettings);header->onSettings();
    auto* settings=dynamic_cast<juce::DocumentWindow*>(tabs.getTopLevelComponent());REQUIRE(settings && settings->isVisible());
    REQUIRE(!storage->scanning());tabs.setCurrentTabIndex(7);wait([&]{return storage->displayedReport()!=nullptr;});
    const auto* report=storage->displayedReport();if(!report->inventoryComplete)for(const auto& issue:report->issues)std::cerr<<issue<<'\n';
    REQUIRE(report->inventoryComplete && report->dependencyVerificationComplete && report->referencedFiles>0 && report->missingReferences==0 && report->reclaimableBytes==0);
    const auto firstRoot=processor.swellEngine().assetDirectory();for(const auto& file:report->files)REQUIRE(file.root==firstRoot.getFullPathName());
    std::map<juce::String,juce::String> originalHashes;for(const auto& file:report->files)originalHashes[file.path]=file.sha256;
    juce::MemoryBlock before;processor.getStateInformation(before);const auto beforeCapture=processor.getCaptureSnapshot();const auto renders=processor.swellEngine().snapshot().renderCount;
    tabs.setCurrentTabIndex(5);storage->poll();REQUIRE(!storage->scanning() && !storage->displayedReport());tabs.setCurrentTabIndex(7);wait([&]{return storage->displayedReport()!=nullptr;});
    juce::MemoryBlock after;processor.getStateInformation(after);REQUIRE(after==before && processor.getCaptureSnapshot().reference==beforeCapture.reference && processor.swellEngine().snapshot().renderCount==renders);
    for(const auto& [path,hash]:originalHashes)REQUIRE(m3::fileHash(firstRoot.getChildFile(path))==hash);
    settings->setVisible(false);storage->poll();REQUIRE(!storage->scanning() && !storage->displayedReport());header->onSettings();wait([&]{return storage->displayedReport()!=nullptr;});
    // Exercise the production Backup/Locate/Restore controls with real accepted state.
    const auto action=[&](const juce::String& name)->m3::ui::controls::ActionButton&{auto* c=dynamic_cast<m3::ui::controls::ActionButton*>(find(*storage,name));REQUIRE(c);return *c;};
    auto* viewport=dynamic_cast<m3::ui::controls::FocusViewport*>(tabs.getTabContentComponent(7));REQUIRE(viewport && viewport->getViewedComponent()==storage);
    REQUIRE(storage->getHeight()>=m3::ui::PortableStoragePanel::preferredHeight());
    TestDirectory backup;REQUIRE(backup.file.createDirectory().wasOk());
    storage->folderPicker=[&](const auto&,const auto&,auto complete){complete(backup.file);};
    const auto operation=[&](const juce::String& name){action(name).onClick();wait([&]{storage->poll();return !storage->busy() && storage->lastResult()!=nullptr;});};
    viewport->reveal(action("backupAssets"));REQUIRE(viewport->getViewArea().intersects(action("backupAssets").getBounds()));
    operation("backupAssets");if(!storage->lastResult()->success)std::cerr<<storage->lastResult()->error<<'\n';REQUIRE(storage->lastResult()->success);
    const auto locator=storage->lastResult()->receipt.locator;
    operation("locateAssets");REQUIRE(storage->lastResult()->success);
    operation("restoreAssets");REQUIRE(storage->lastResult()->success);
    processor.getStateInformation(after);REQUIRE(after==before && processor.getCaptureSnapshot().reference==beforeCapture.reference && processor.swellEngine().snapshot().renderCount==renders);
    for(const auto& [path,hash]:originalHashes)REQUIRE(m3::fileHash(firstRoot.getChildFile(path))==hash);
    // Picker completion after a session switch must not initiate a copy.
    std::function<void(juce::File)> pendingPicker;storage->folderPicker=[&](const auto&,const auto&,auto complete){pendingPicker=std::move(complete);};
    action("backupAssets").onClick();REQUIRE(pendingPicker && !storage->busy());
    // Loading another session while Storage is open must replace its scope.
    auto otherOwner=std::make_unique<PreverbAudioProcessor>(root.file);auto& other=*otherOwner;wait([&]{return other.swellEngine().savedSession().defaultIR.present() && !other.swellEngine().snapshot().defaultIRPending;});
    juce::MemoryBlock otherState;other.getStateInformation(otherState);const auto otherRoot=other.swellEngine().assetDirectory();REQUIRE(otherRoot!=firstRoot);
    processor.setStateInformation(otherState.getData(),static_cast<int>(otherState.getSize()));REQUIRE(processor.flushCaptureForTesting());editor.refresh();
    wait([&]{editor.refresh();const auto* current=storage->displayedReport();return current && !current->files.empty() && current->files.front().root==otherRoot.getFullPathName();});
    REQUIRE(storage->displayedReport()->dependencyVerificationComplete);for(const auto& file:storage->displayedReport()->files)REQUIRE(file.root==otherRoot.getFullPathName());
    pendingPicker(backup.file);REQUIRE(!storage->busy() && storage->statusText().contains("session changed"));
    // Explicitly selected receipts from another project are rejected.
    storage->receiptPicker=[&](const auto&,const auto&,auto complete){complete(backup.file.getChildFile("receipts/"+locator.snapshot+".json"));};
    operation("restoreAssets");REQUIRE(!storage->lastResult()->success && storage->lastResult()->error.contains("another project or instance"));
    processor.setStateInformation(before.getData(),static_cast<int>(before.getSize()));REQUIRE(processor.flushCaptureForTesting());editor.refresh();
    wait([&]{editor.refresh();const auto* current=storage->displayedReport();return current && !current->files.empty() && current->files.front().root==firstRoot.getFullPathName();});REQUIRE(storage->displayedReport()->dependencyVerificationComplete);
    // Reopening without an in-memory locator selects the exact receipt, never a latest head.
    operation("locateAssets");REQUIRE(storage->lastResult()->success);
    storage->folderPicker=[&](const auto&,const auto&,auto complete){complete(backup.file);};
    operation("restoreAssets");REQUIRE(storage->lastResult()->success);
    storage->folderPicker={};storage->receiptPicker={};viewport->setViewPosition(0,0);
    const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_STORAGE_SETTINGS_OUTPUT",{});
    const auto screenshot=[&](const juce::File& path){auto stream=path.createOutputStream();REQUIRE(stream && stream->setPosition(0) && stream->truncate().wasOk() && juce::PNGImageFormat().writeImageToStream(tabs.createComponentSnapshot(tabs.getLocalBounds()),*stream));};
    if(output.isNotEmpty())screenshot(juce::File(output));
    viewport->setViewPosition(0,storage->getHeight());
    if(output.isNotEmpty())screenshot(juce::File(output).getSiblingFile(juce::File(output).getFileNameWithoutExtension()+"-controls.png"));
    if(argc>1 && juce::String(argv[1])=="--review")juce::MessageManager::getInstance()->runDispatchLoop();
    std::cout<<"Storage Settings: production page/tab indexes, automatic scanning, current-session references, reopen, unchanged saved state/render counts/hashes and restore-scope fencing passed.\n";
});}
