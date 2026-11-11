#include <UI/PortableStoragePanel.h>
#include "PortableFixture.h"
static m3::ui::controls::ActionButton& button(juce::Component& root,const juce::String& name)
{for(auto* c:root.getChildren())if(c->getName()==name){auto* b=dynamic_cast<m3::ui::controls::ActionButton*>(c);REQUIRE(b);return *b;}throw std::runtime_error("Missing action");}
#include "working_folder_preview.h"
#include "scratch_preview_review.h"
int main(int argc,char** argv){juce::ScopedJuceInitialiser_GUI gui;if(argc>1 && juce::String(argv[1]).startsWith("--scratch"))return scratchPreviewReview(juce::String(argv[1])=="--scratch-review");if(argc>1 && juce::String(argv[1]).startsWith("--working-folder"))return workingFolderPreview(juce::String(argv[1])=="--working-folder-review");return runTest([&]{
    Fixture fixture;TestDirectory backup,restore;REQUIRE(backup.file.createDirectory().wasOk() && restore.file.createDirectory().wasOk());
    m3::storage::portable::Input input{fixture.root,fixture.xml,fixture.manifest};
    m3::ui::WorkspaceTheme theme;
    m3::ui::PortableStoragePanel panel([&](auto& value,auto& error){value=input;error.clear();return true;},restore.file);
    panel.setLookAndFeel(&theme);
    struct Window:juce::DocumentWindow{Window():DocumentWindow("Storage backup and recovery - disposable fixture review",m3::ui::background,DocumentWindow::closeButton){}void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}} window;
    window.setUsingNativeTitleBar(true);window.setContentNonOwned(&panel,false);window.setSize(680,800);window.setVisible(true);
    int picks=0;panel.folderPicker=[&](const auto&,const auto&,auto completed){++picks;completed(backup.file);};
    const auto wait=[&]{const auto end=juce::Time::getMillisecondCounterHiRes()+30000;while(panel.busy() || !panel.lastResult()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<end);juce::MessageManager::getInstance()->runDispatchLoopUntil(10);panel.poll();}};
    REQUIRE(button(panel,"restoreAssets").isEnabled());button(panel,"backupAssets").onClick();wait();if(!panel.lastResult()->success)std::cerr<<panel.lastResult()->error<<'\n';REQUIRE(panel.lastResult()->success && picks==1);
    button(panel,"locateAssets").onClick();wait();REQUIRE(panel.lastResult()->success && picks==2);
    button(panel,"restoreAssets").onClick();wait();REQUIRE(panel.lastResult()->success && picks==3);
    const auto restored=restore.file.getChildFile(fixture.observed.project).getChildFile(fixture.observed.instance);for(const auto& dependency:fixture.observed.dependencies)REQUIRE(restored.getChildFile(dependency.path).existsAsFile());
    for(const auto& name:juce::StringArray{"backupAssets","restoreAssets","locateAssets"})REQUIRE(panel.getLocalBounds().contains(button(panel,name).getBounds()));
    // Cancelling a picker performs no operation and a changed session is rejected.
    const auto previous=panel.lastResult();panel.folderPicker=[](const auto&,const auto&,auto complete){complete({});};button(panel,"backupAssets").onClick();REQUIRE(!panel.busy() && panel.lastResult()==previous);
    int pendingPicks=0;std::function<void(juce::File)> cancelledPicker;
    panel.folderPicker=[&](const auto&,const auto&,auto complete){++pendingPicks;cancelledPicker=std::move(complete);};
    button(panel,"backupAssets").onClick();button(panel,"backupAssets").onClick();REQUIRE(pendingPicks==1);
    cancelledPicker({});cancelledPicker(backup.file);REQUIRE(!panel.busy() && panel.lastResult()==previous);
    std::function<void(juce::File)> orphanedPicker;
    {auto orphan=std::make_unique<m3::ui::PortableStoragePanel>([&](auto& value,auto&){value=input;return true;},restore.file);
        orphan->folderPicker=[&](const auto&,const auto&,auto complete){orphanedPicker=std::move(complete);};button(*orphan,"backupAssets").onClick();}
    REQUIRE(orphanedPicker);orphanedPicker(backup.file);REQUIRE(!panel.busy() && panel.lastResult()==previous);
    const auto original=input.instanceRoot;panel.folderPicker=[&](const auto&,const auto&,auto complete){input.instanceRoot=fixture.directory.file;complete(backup.file);};button(panel,"backupAssets").onClick();REQUIRE(!panel.busy() && panel.statusText().contains("session changed"));input.instanceRoot=original;
    std::function<void(juce::File)> finishPicker;panel.folderPicker=[&](const auto&,const auto&,auto complete){finishPicker=std::move(complete);};button(panel,"backupAssets").onClick();window.setVisible(false);finishPicker(backup.file);REQUIRE(!panel.busy());window.setVisible(true);
    button(panel,"backupAssets").onClick();++input.controlEpoch;finishPicker(backup.file);REQUIRE(!panel.busy() && panel.statusText().contains("session changed"));--input.controlEpoch;
    panel.folderPicker=[&](const auto&,const auto&,auto complete){complete(backup.file);};button(panel,"locateAssets").onClick();wait();REQUIRE(panel.lastResult()->success);
    const auto receipt=backup.file.getChildFile("receipts/"+panel.lastResult()->receipt.locator.snapshot+".json");
    // A newly bound page can restore without a process-local known locator.
    panel.bind([&](auto& value,auto& error){value=input;error.clear();return true;});
    int receipts=0;panel.receiptPicker=[&](const auto&,const auto&,auto complete){++receipts;complete(receipt);};
    button(panel,"restoreAssets").onClick();wait();REQUIRE(panel.lastResult()->success && receipts==1);
    // A valid backup of another instance must not be restored into this session.
    Fixture unrelated;auto otherInput=m3::storage::portable::Input{unrelated.root,unrelated.xml,unrelated.manifest};
    panel.bind([&](auto& value,auto& error){value=otherInput;error.clear();return true;});
    button(panel,"restoreAssets").onClick();wait();REQUIRE(!panel.lastResult()->success && panel.lastResult()->error.contains("another project or instance"));
    panel.bind([&](auto& value,auto& error){value=input;error.clear();return true;});
    button(panel,"locateAssets").onClick();wait();REQUIRE(panel.lastResult()->success);
    panel.folderPicker={};panel.receiptPicker={}; // Review uses native selectors.
    const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_PORTABLE_PREVIEW_OUTPUT",{});if(output.isNotEmpty()){auto stream=juce::File(output).createOutputStream();REQUIRE(stream && stream->setPosition(0) && stream->truncate().wasOk() && juce::PNGImageFormat().writeImageToStream(panel.createComponentSnapshot(panel.getLocalBounds()),*stream));}
    if(argc>1 && juce::String(argv[1])=="--review")juce::MessageManager::getInstance()->runDispatchLoop();
    std::cout<<"Portable Storage preview: working backup, locate, restore bindings, picker cancellation, stale-session rejection and bounds passed.\n";
    window.setLookAndFeel(nullptr);panel.setLookAndFeel(nullptr);
});}
