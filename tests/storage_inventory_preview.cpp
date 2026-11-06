#include "AudioTestFiles.h"
#include <UI/StorageInventoryPanel.h>
#include <UI/Controls/PresentationSurface.h>
using namespace m3;
using namespace m3::ui;
int main(int argc,char** argv){juce::ScopedJuceInitialiser_GUI gui;return runTest([&]{
    TestDirectory directory;const auto scope=directory.file.getChildFile("Fixture project/Fixture instance");
    REQUIRE(scope.getChildFile("segments").createDirectory().wasOk());writeTestWav(scope.getChildFile("segments/source.wav"),2,48000,std::vector<float>(48000,.125f));
    REQUIRE(scope.getChildFile("segments/source.wav").copyFileTo(scope.getChildFile("segments/retained.wav")));
    std::atomic<int> requested=0;StorageInventoryPanel panel([&]{++requested;storage::Request r;r.roots={scope};return r;});WorkspaceTheme theme;panel.setLookAndFeel(&theme);panel.setSize(640,540);
    REQUIRE(requested==0);m3::ui::controls::ActionButton* action=nullptr;for(auto* child:panel.getChildren())if(child->getName()=="inspectStorage")action=dynamic_cast<m3::ui::controls::ActionButton*>(child);REQUIRE(action && action->keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));const auto deadline=juce::Time::getMillisecondCounterHiRes()+10000;
    while(!panel.displayedReport()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<deadline);juce::MessageManager::getInstance()->runDispatchLoopUntil(10);panel.poll();}
    REQUIRE(requested==1 && panel.displayedReport()->physicalFiles==2 && panel.displayedReport()->reclaimableBytes==0);
    for(auto* child:panel.getChildren())REQUIRE(panel.getLocalBounds().contains(child->getBounds()));
    const auto before=panel.displayedReport()->physicalBytes;panel.begin();while(panel.scanning()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<deadline);juce::MessageManager::getInstance()->runDispatchLoopUntil(10);}panel.poll();REQUIRE(requested==2 && panel.displayedReport()->physicalBytes==before);
    panel.bind([&]{storage::Request r;r.roots={scope.getChildFile("offline")};return r;});panel.poll();REQUIRE(!panel.displayedReport());
    panel.begin();while(panel.scanning()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<deadline);juce::MessageManager::getInstance()->runDispatchLoopUntil(10);}panel.poll();REQUIRE(!panel.displayedReport()->inventoryComplete);
    panel.bind([&]{storage::Request r;r.roots={scope};return r;});panel.begin();while(panel.scanning()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<deadline);juce::MessageManager::getInstance()->runDispatchLoopUntil(10);}panel.poll();
    if(argc>1 && juce::String(argv[1])=="--review"){
        class Window final:public juce::DocumentWindow
        {
        public:Window(juce::Component& content):DocumentWindow("Issue #8 - Storage Settings preview",background,closeButton){setUsingNativeTitleBar(true);setContentNonOwned(&content,false);centreWithSize(640,580);setResizable(false,false);setVisible(true);}
            void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}
        };
        Window window(panel);window.setLookAndFeel(&theme);juce::MessageManager::getInstance()->runDispatchLoop();window.setLookAndFeel(nullptr);
    }else{
        const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_STORAGE_PREVIEW_OUTPUT",{});if(output.isNotEmpty()){const juce::File destination(output);REQUIRE(destination.getParentDirectory().createDirectory().wasOk());auto stream=destination.createOutputStream();REQUIRE(stream && juce::PNGImageFormat().writeImageToStream(panel.createComponentSnapshot(panel.getLocalBounds()),*stream));}
        std::cout<<"Storage panel: explicit inspection, worker publication, protected summary and contained production-token layout passed.\n";
    }
    panel.setLookAndFeel(nullptr);
});}
