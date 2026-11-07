#pragma once

#include "ScratchPreviewFixture.h"
#include <thread>

namespace scratchReview
{
using namespace m3::ui;
using namespace m3::storage::portable;

class Panel final:public FlatSurface
{
public:
    Panel(juce::File store,juce::File original):initialStore(std::move(store)),source(std::move(original))
    {
        heading.setText("Cancelled Backup copies",juce::dontSendNotification);
        help.setText("Preview one backup folder. Only verified cancelled copies with an intact original can appear here.",juce::dontSendNotification);
        applyTextRole(heading,TextRole::sectionHeading);
        applyTextRole(help,TextRole::helper);
        applyTextRole(status,TextRole::controlLabel);
        status.setText("Choose a backup folder to preview its cancelled copies.",juce::dontSendNotification);
        results.setName("scratchPreviewResults");
        results.setTitle("Read-only scratch preview results");
        results.setMultiLine(true,true);
        results.setReadOnly(true);
        results.setCaretVisible(false);
        results.setScrollbarsShown(true);
        results.setFont(textFont(TextRole::helper));
        results.setColour(juce::TextEditor::textColourId,textStyle(TextRole::helper).ink);
        for(auto* c:std::initializer_list<juce::Component*>{&heading,&help,&status,&results,&choose,&stop})addAndMakeVisible(c);
        choose.onClick=[this]{select();};
        stop.onClick=[this]{worker.request_stop();};
        stop.setEnabled(false);
    }

    void resized()override
    {
        auto area=getLocalBounds().reduced(spacing::section);
        heading.setBounds(area.removeFromTop(controlGeometry::headingHeight));
        area.removeFromTop(spacing::related);
        help.setBounds(area.removeFromTop(controlGeometry::labelHeight));
        status.setBounds(area.removeFromTop(controlGeometry::labelHeight));
        area.removeFromTop(spacing::related);
        auto footer=area.removeFromBottom(controlGeometry::height);
        choose.setBounds(footer.removeFromLeft((footer.getWidth()-spacing::related)/2));
        footer.removeFromLeft(spacing::related);
        stop.setBounds(footer);
        area.removeFromBottom(spacing::related);
        results.setBounds(area);
    }

    controls::ActionButton choose{"Preview backup folder..."},stop{"Stop preview"};
    std::function<void(std::function<void(juce::File)>)> picker;
    const std::optional<ScratchPreview>& report()const{return shown;}
    juce::String resultText()const{return results.getText();}

private:
    void select()
    {
        if(!choose.isEnabled())return;
        choose.setEnabled(false);
        auto complete=[safe=juce::Component::SafePointer<Panel>(this)](juce::File folder){
            if(!safe)return;
            if(folder==juce::File{}){safe->choose.setEnabled(true);return;}
            safe->start(std::move(folder));
        };
        if(picker){picker(std::move(complete));return;}
        chooser=std::make_unique<juce::FileChooser>("Choose a backup folder to preview",initialStore);
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectDirectories,
            [complete=std::move(complete)](const juce::FileChooser& c)mutable{complete(c.getResult());});
    }

    void start(juce::File folder)
    {
        stop.setEnabled(true);
        shown.reset();
        results.clear();
        status.setText("Checking cancelled Backup copies...",juce::dontSendNotification);
        worker=std::jthread([safe=juce::Component::SafePointer<Panel>(this),folder=std::move(folder),original=source](std::stop_token token){
            Hooks hooks;
            hooks.cancelled=[token]{return token.stop_requested();};
            auto report=previewScratch(folder,original,hooks);
            juce::MessageManager::callAsync([safe,folder,report=std::move(report)]()mutable{
                if(safe)safe->finish(folder,std::move(report));
            });
        });
    }

    void finish(const juce::File& folder,ScratchPreview result)
    {
        shown=std::move(result);
        choose.setEnabled(true);
        stop.setEnabled(false);
        if(!shown->complete){
            status.setText(shown->cancelled?"Preview stopped":"Preview incomplete",juce::dontSendNotification);
            results.setText(shown->error,false);
            return;
        }
        const auto count=shown->eligible.size();
        status.setText(juce::String(static_cast<int>(count))+(count==1?" eligible copy, ":" eligible copies, ")
            +juce::String(static_cast<juce::int64>(shown->bytes))+" bytes",juce::dontSendNotification);
        juce::String text="Backup folder: "+folder.getFullPathName()+"\nStore: "+shown->store+"\n\n";
        for(const auto& item:shown->eligible)
            text+=item.path+"\n"+juce::String(static_cast<juce::int64>(item.bytes))+" bytes. "+item.reason+".\n\n";
        text+="Protected: recordings, capture staging, IRs, render history, exports and published backups.\n";
        if(!shown->excluded.empty())text+="\nTransactions with no eligible files:\n";
        for(const auto& item:shown->excluded)text+=item.path+"\n"+item.reason+"\n";
        results.setText(text,false);
        results.moveCaretToTop(false);
    }

    juce::File initialStore,source;
    juce::Label heading,help,status;
    juce::TextEditor results;
    std::optional<ScratchPreview> shown;
    std::unique_ptr<juce::FileChooser> chooser;
    std::jthread worker;
};
}

static int scratchPreviewReview(bool review)
{
    return runTest([&]{
        ScratchPreviewFixture fixture;
        m3::ui::WorkspaceTheme theme;
        scratchReview::Panel panel(fixture.store.file,fixture.original.root);
        panel.setLookAndFeel(&theme);
        panel.setSize(680,420);
        if(review){
            struct Window final:juce::DocumentWindow
            {
                Window():DocumentWindow("Cancelled Backup copies - disposable review",m3::ui::background,closeButton){}
                void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}
            } window;
            window.setUsingNativeTitleBar(true);
            window.setContentNonOwned(&panel,false);
            window.centreWithSize(680,420);
            window.setVisible(true);
            juce::MessageManager::getInstance()->runDispatchLoop();
            window.clearContentComponent();
        }else{
            REQUIRE(!panel.report() && !panel.stop.isEnabled());
            panel.picker=[](auto complete){complete({});};
            panel.choose.onClick();
            REQUIRE(!panel.report() && panel.choose.isEnabled());
            panel.picker=[&](auto complete){complete(fixture.store.file);};
            panel.choose.onClick();
            REQUIRE(!panel.choose.isEnabled() && panel.stop.isEnabled());
            const auto end=juce::Time::getMillisecondCounterHiRes()+10000;
            while(!panel.report()){
                REQUIRE(juce::Time::getMillisecondCounterHiRes()<end);
                juce::MessageManager::getInstance()->runDispatchLoopUntil(10);
            }
            REQUIRE(panel.report()->complete && panel.report()->eligible.size()==1 && panel.report()->bytes==fixture.bytes);
            REQUIRE(panel.resultText().contains(fixture.path) && panel.resultText().contains("Protected: recordings"));
            REQUIRE(panel.choose.isEnabled() && !panel.stop.isEnabled());
            REQUIRE(m3::fileHash(fixture.store.file.getChildFile(fixture.path))==fixture.sha);
            const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_SCRATCH_PREVIEW_OUTPUT",{});
            if(output.isNotEmpty()){
                auto stream=juce::File(output).createOutputStream();
                REQUIRE(stream && stream->setPosition(0) && stream->truncate().wasOk()
                    && juce::PNGImageFormat().writeImageToStream(panel.createComponentSnapshot(panel.getLocalBounds()),*stream));
            }
            std::cout<<"Scratch preview bindings: opt-in, cancelled picker, worker report, exact displayed scope and read-only assets passed.\n";
        }
        panel.setLookAndFeel(nullptr);
    });
}
