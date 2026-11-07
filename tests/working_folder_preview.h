#include <UI/WorkingFolderPanel.h>
static int workingFolderPreview(bool review)
{
    return runTest([&]{
        Fixture fixture;TestDirectory registry,target;REQUIRE(registry.file.createDirectory().wasOk() && target.file.createDirectory().wasOk());
        using namespace m3::storage::portable;Input input{fixture.root,fixture.xml,fixture.manifest};
        std::shared_ptr<WorkingFolder> live;int saves=0;bool reject=false;
        m3::ui::WorkspaceTheme theme;
        m3::ui::WorkingFolderPanel panel([&](auto& value,auto& error){value=input;error.clear();return true;},registry.file,
            [&](const auto& folder,const auto& expected,auto& error){
                if(reject || input.controlEpoch!=expected.controlEpoch){error="The session is busy or changed. Try again.";return false;}
                live=folder;const auto& info=workingFolderInfo(*folder);input.instanceRoot=info.projectsRoot.getChildFile(fixture.observed.project).getChildFile(fixture.observed.instance);
                auto association=info.association;REQUIRE(encodeAssociatedState(fixture.xml,fixture.manifest,association,input.processorXml,error));++input.controlEpoch;++saves;return true;});
        panel.setLookAndFeel(&theme);
        struct Window:juce::DocumentWindow{Window():DocumentWindow("Asset working folder - disposable native review",m3::ui::background,DocumentWindow::closeButton){}void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}} window;
        window.setUsingNativeTitleBar(true);window.setContentNonOwned(&panel,false);window.centreWithSize(680,420);window.setVisible(true);
        const auto find=[&](const juce::String& name)->m3::ui::controls::ActionButton&{for(auto* child:panel.getChildren())if(child->getName()==name){auto* found=dynamic_cast<m3::ui::controls::ActionButton*>(child);REQUIRE(found);return *found;}throw std::runtime_error("Missing working-folder action");};
        const auto wait=[&]{const auto end=juce::Time::getMillisecondCounterHiRes()+30000;do{REQUIRE(juce::Time::getMillisecondCounterHiRes()<end);juce::MessageManager::getInstance()->runDispatchLoopUntil(10);panel.poll();}while(panel.busy());panel.poll();};
        auto& choose=find("chooseWorkingFolder");auto& use=find("useWorkingFolder");
        if(review){juce::MessageManager::getInstance()->runDispatchLoop();window.setVisible(false);window.clearContentComponent();panel.setLookAndFeel(nullptr);return;}
        REQUIRE(!use.isEnabled());const auto originalRoot=input.instanceRoot;
        panel.picker=[](auto complete){complete({});};choose.onClick();REQUIRE(!panel.busy() && !panel.ready() && saves==0);
        std::function<void(juce::File)> delayed;panel.picker=[&](auto complete){delayed=std::move(complete);};choose.onClick();++input.controlEpoch;delayed(target.file);REQUIRE(panel.statusText().contains("session changed") && saves==0);
        panel.picker=[&](auto complete){complete(target.file);};choose.onClick();wait();if(!panel.ready())throw std::runtime_error(panel.statusText().toStdString());
        REQUIRE(use.isEnabled() && input.instanceRoot==originalRoot && saves==0);
        // Late edits invalidate prepared copies and cannot publish the folder.
        ++input.controlEpoch;use.onClick();REQUIRE(!panel.ready() && saves==0 && input.instanceRoot==originalRoot);
        choose.onClick();wait();REQUIRE(panel.ready());reject=true;use.onClick();wait();REQUIRE(!panel.ready() && saves==0 && input.instanceRoot==originalRoot);reject=false;
        choose.onClick();wait();REQUIRE(panel.ready());use.onClick();wait();REQUIRE(saves==1 && live && input.instanceRoot!=originalRoot && panel.statusText().contains("Save your DAW project"));
        // A successful binding requests save; this preview never writes a DAW project.
        REQUIRE(originalRoot.exists());
        for(const auto size:{juce::Point<int>(420,240),juce::Point<int>(680,420)}){panel.setSize(size.x,size.y);REQUIRE(panel.getLocalBounds().contains(choose.getBounds()) && panel.getLocalBounds().contains(use.getBounds()));REQUIRE(choose.getBottom()==panel.getHeight()-m3::ui::spacing::related);}
        window.centreWithSize(680,420);panel.picker={};
        window.setVisible(false);window.clearContentComponent();panel.setLookAndFeel(nullptr);
        std::cout<<"Working-folder control bindings: picker cancellation, stale choices, prepare without switch, late edits, binding rejection, explicit use/save request and pinned footer passed.\n";
    });
}
