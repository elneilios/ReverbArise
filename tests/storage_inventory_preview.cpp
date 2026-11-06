#include "AudioTestFiles.h"
#include <UI/StorageInventoryPanel.h>
using namespace m3;
using namespace m3::ui;
int main(int argc,char** argv){juce::ScopedJuceInitialiser_GUI gui;return runTest([&]{
    TestDirectory directory;const auto scope=directory.file.getChildFile("Fixture project/Fixture instance");
    REQUIRE(scope.getChildFile("segments").createDirectory().wasOk());writeTestWav(scope.getChildFile("segments/source.wav"),2,48000,std::vector<float>(48000,.125f));
    REQUIRE(scope.getChildFile("segments/source.wav").copyFileTo(scope.getChildFile("segments/retained.wav")));
    std::atomic<int> requested=0;StorageInventoryPanel panel([&]{++requested;storage::Request r;r.roots={scope};return r;});WorkspaceTheme theme;panel.setLookAndFeel(&theme);panel.setSize(640,540);
    class Window final:public juce::DocumentWindow
    {
    public:Window(juce::Component& content):DocumentWindow("Issue #8 - Storage Settings preview",background,closeButton){setUsingNativeTitleBar(true);setContentNonOwned(&content,false);centreWithSize(640,580);setResizable(false,false);}
        void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}
    };
    Window window(panel);window.setLookAndFeel(&theme);
    const auto wait=[&](const std::function<bool()>& predicate){const auto deadline=juce::Time::getMillisecondCounterHiRes()+10000;while(!predicate()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<deadline);juce::MessageManager::getInstance()->runDispatchLoopUntil(10);panel.poll();}};
    panel.poll();REQUIRE(requested==0);
    for(auto* child:panel.getChildren())REQUIRE(child->getName()!="inspectStorage");
    window.setVisible(true);wait([&]{return panel.displayedReport()!=nullptr;});
    REQUIRE(requested==1 && panel.displayedReport()->physicalFiles==2 && panel.displayedReport()->reclaimableBytes==0);
    for(auto* child:panel.getChildren())REQUIRE(panel.getLocalBounds().contains(child->getBounds()));
    for(int i=0;i<20;++i)panel.poll();REQUIRE(requested==1);
    const auto before=panel.displayedReport()->physicalBytes;window.setVisible(false);panel.poll();REQUIRE(!panel.scanning() && panel.displayedReport());
    const auto* retained=panel.displayedReport();window.setVisible(true);REQUIRE(panel.displayedReport()==retained);wait([&]{return panel.displayedReport()!=retained;});REQUIRE(requested==2 && panel.displayedReport()->physicalBytes==before);
    panel.bind([&]{storage::Request r;r.roots={scope.getChildFile("offline")};return r;});REQUIRE(!panel.displayedReport());wait([&]{return panel.displayedReport()!=nullptr;});REQUIRE(!panel.displayedReport()->inventoryComplete);
    window.setVisible(false);panel.poll();panel.bind([&]{++requested;storage::Request r;r.roots={scope};return r;});panel.poll();REQUIRE(requested==2 && !panel.scanning());
    window.setVisible(true);wait([&]{return panel.displayedReport()!=nullptr;});REQUIRE(requested==3);
    panel.bind([&]{++requested;storage::Request r;r.roots={scope};return r;});
    m3::ui::controls::ActionButton* stop=nullptr;for(auto* child:panel.getChildren())if(child->getName()=="cancelStorageScan")stop=dynamic_cast<m3::ui::controls::ActionButton*>(child);REQUIRE(stop && stop->isVisible());stop->onClick();
    for(int i=0;i<20;++i)panel.poll();REQUIRE(requested==4 && !panel.scanning() && !panel.displayedReport());
    window.setVisible(false);panel.poll();window.setVisible(true);wait([&]{return panel.displayedReport()!=nullptr;});REQUIRE(requested==5);
    if(argc>1 && juce::String(argv[1])=="--review")juce::MessageManager::getInstance()->runDispatchLoop();
    else{
        const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_STORAGE_PREVIEW_OUTPUT",{});if(output.isNotEmpty()){const juce::File destination(output);REQUIRE(destination.getParentDirectory().createDirectory().wasOk());auto stream=destination.createOutputStream();REQUIRE(stream && juce::PNGImageFormat().writeImageToStream(panel.createComponentSnapshot(panel.getLocalBounds()),*stream));}
        std::cout<<"Storage panel: automatic page-open scan, hidden-page inactivity, no refresh loop, reopen refresh and scope rebinding passed.\n";
    }
    window.setLookAndFeel(nullptr);panel.setLookAndFeel(nullptr);
});}
