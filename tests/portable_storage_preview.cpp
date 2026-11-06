#include <UI/PortableStoragePanel.h>
#include "PortableFixture.h"
static m3::ui::controls::ActionButton& button(juce::Component& root,const juce::String& name)
{for(auto* c:root.getChildren())if(c->getName()==name){auto* b=dynamic_cast<m3::ui::controls::ActionButton*>(c);REQUIRE(b);return *b;}throw std::runtime_error("Missing action");}
int main(int argc,char** argv){juce::ScopedJuceInitialiser_GUI gui;return runTest([&]{
    Fixture fixture;TestDirectory backup,restore;REQUIRE(backup.file.createDirectory().wasOk() && restore.file.createDirectory().wasOk());
    m3::storage::portable::Input input{fixture.root,fixture.xml,fixture.manifest};
    m3::ui::WorkspaceTheme theme;
    m3::ui::PortableStoragePanel panel([&](auto& value,auto& error){value=input;error.clear();return true;},restore.file);
    panel.setLookAndFeel(&theme);
    struct Window:juce::DocumentWindow{Window():DocumentWindow("Storage backup and recovery - disposable fixture review",m3::ui::background,DocumentWindow::closeButton){}void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}} window;
    window.setUsingNativeTitleBar(true);window.setContentNonOwned(&panel,false);window.setSize(680,800);window.setVisible(true);
    int picks=0;panel.folderPicker=[&](const auto&,const auto&,auto completed){++picks;completed(backup.file);};
    const auto wait=[&]{const auto end=juce::Time::getMillisecondCounterHiRes()+30000;while(panel.busy() || !panel.lastResult()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<end);juce::MessageManager::getInstance()->runDispatchLoopUntil(10);panel.poll();}};
    REQUIRE(!button(panel,"restoreAssets").isEnabled());button(panel,"backupAssets").onClick();wait();if(!panel.lastResult()->success)std::cerr<<panel.lastResult()->error<<'\n';REQUIRE(panel.lastResult()->success && picks==1);
    button(panel,"locateAssets").onClick();wait();REQUIRE(panel.lastResult()->success && picks==2);
    button(panel,"restoreAssets").onClick();wait();REQUIRE(panel.lastResult()->success && picks==3);
    const auto restored=restore.file.getChildFile(fixture.observed.project).getChildFile(fixture.observed.instance);for(const auto& dependency:fixture.observed.dependencies)REQUIRE(restored.getChildFile(dependency.path).existsAsFile());
    for(const auto& name:juce::StringArray{"backupAssets","restoreAssets","locateAssets"})REQUIRE(panel.getLocalBounds().contains(button(panel,name).getBounds()));
    // Cancelling a picker performs no operation and a changed session is rejected.
    const auto previous=panel.lastResult();panel.folderPicker=[](const auto&,const auto&,auto complete){complete({});};button(panel,"backupAssets").onClick();REQUIRE(!panel.busy() && panel.lastResult()==previous);
    const auto original=input.instanceRoot;panel.folderPicker=[&](const auto&,const auto&,auto complete){input.instanceRoot=fixture.directory.file;complete(backup.file);};button(panel,"backupAssets").onClick();REQUIRE(!panel.busy() && panel.statusText().contains("session changed"));input.instanceRoot=original;
    std::function<void(juce::File)> finishPicker;panel.folderPicker=[&](const auto&,const auto&,auto complete){finishPicker=std::move(complete);};button(panel,"backupAssets").onClick();window.setVisible(false);finishPicker(backup.file);REQUIRE(!panel.busy());window.setVisible(true);
    button(panel,"backupAssets").onClick();++input.controlEpoch;finishPicker(backup.file);REQUIRE(!panel.busy() && panel.statusText().contains("session changed"));--input.controlEpoch;
    panel.folderPicker=[&](const auto&,const auto&,auto complete){complete(backup.file);};button(panel,"locateAssets").onClick();wait();REQUIRE(panel.lastResult()->success);
    panel.folderPicker={}; // Owner review uses the real native folder picker.
    const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_PORTABLE_PREVIEW_OUTPUT",{});if(output.isNotEmpty()){auto stream=juce::File(output).createOutputStream();REQUIRE(stream && stream->setPosition(0) && stream->truncate().wasOk() && juce::PNGImageFormat().writeImageToStream(panel.createComponentSnapshot(panel.getLocalBounds()),*stream));}
    if(argc>1 && juce::String(argv[1])=="--review")juce::MessageManager::getInstance()->runDispatchLoop();
    std::cout<<"Portable Storage preview: working backup, locate, restore bindings, picker cancellation, stale-session rejection and bounds passed.\n";
    window.setLookAndFeel(nullptr);panel.setLookAndFeel(nullptr);
});}
