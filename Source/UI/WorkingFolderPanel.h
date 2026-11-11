#pragma once
#include "Controls/SurfaceLifecycle.h"
#include <Storage/WorkingFolder.h>
#include <UI/Controls/ActionButton.h>
#include <UI/Controls/WrappedLabel.h>
#include <UI/Controls/FocusViewport.h>

namespace m3::ui
{
// Owner-approved working-folder composition; the binding owns publication.
// The binding owns live association publication and host save notification.
class WorkingFolderPanel final:public FlatSurface,private juce::Timer
{
    using Input=storage::portable::Input;
    using Folder=storage::portable::WorkingFolder;
    struct Reply {std::shared_ptr<Folder> folder;juce::String error;bool use=false,success=false,committed=false;};
    class Worker final:public juce::Thread
    {
    public:
        Worker():Thread("Working-folder verification"){}
        ~Worker()override{cancel=true;signalThreadShouldExit();stopThread(-1);}
        std::atomic<bool> cancel{false},active{false};std::atomic<std::shared_ptr<const Reply>> answer;
        bool begin(std::function<Reply(const storage::portable::Hooks&)> next)
        {if(active.load())return false;stopThread(-1);task=std::move(next);answer.store(nullptr);cancel=false;active=true;if(startThread())return true;active=false;return false;}
    private:
        void run()override{storage::portable::Hooks hooks;hooks.cancelled=[this]{return cancel.load() || threadShouldExit();};Reply reply;
            try{reply=task(hooks);}catch(const std::exception& e){reply.error=e.what();}answer.store(std::make_shared<Reply>(std::move(reply)));active=false;}
        std::function<Reply(const storage::portable::Hooks&)> task;
    };
public:
    using Provider=std::function<bool(Input&,juce::String&)>;
    using Picker=std::function<void(std::function<void(juce::File)>)>;
    using Commit=std::function<bool(const std::shared_ptr<Folder>&,const Input&,juce::String&)>;
    WorkingFolderPanel(Provider next,juce::File registry,Commit commit):provider(std::move(next)),registryRoot(std::move(registry)),publish(std::move(commit))
    {
        setName("workingFolderPanel");viewport.setViewedComponent(&content,false);viewport.setFocusRevealEnabled(true);addAndMakeVisible(viewport);
        heading.setText("Asset working folder",juce::dontSendNotification);applyTextRole(heading,TextRole::sectionHeading);
        applyTextRole(current,TextRole::helper);current.setName("currentWorkingFolder");
        applyTextRole(selected,TextRole::helper);selected.setName("preparedWorkingFolder");
        applyTextRole(help,TextRole::helper,juce::Justification::topLeft);
        help.setText("Choose a dedicated asset folder beside your DAW project, or keep the current user-data folder. Choosing a folder copies and verifies this instance's saved assets. Use This Folder then changes where future assets are written. Originals stay in place.",juce::dontSendNotification);
        applyTextRole(status,TextRole::helper,juce::Justification::topLeft);status.setName("workingFolderStatus");
        for(auto* label:std::initializer_list<juce::Component*>{&heading,&current,&selected,&help,&status})content.addAndMakeVisible(label);
        choose.setName("chooseWorkingFolder");use.setName("useWorkingFolder");stop.setName("stopWorkingFolder");
        choose.setTooltip("Choose an empty dedicated folder or a supported asset store. Copy and verify assets without switching this instance.");
        use.setTooltip("Recheck the prepared copy, change this instance's working folder, and request a DAW project save. Original files remain in place.");
        stop.setTooltip("Stop copying or checking. Keep the current folder and all original assets.");
        for(auto* control:{&choose,&use,&stop})addAndMakeVisible(control);
        choose.onClick=[this]{select();};use.onClick=[this]{activate();};stop.onClick=[this]{worker.cancel=true;setStatus("Stopping safely. The current working folder is retained.");};
        refreshLocation();refresh();startTimerHz(10);
    }
    ~WorkingFolderPanel()override{stopTimer();worker.cancel=true;pickerInvocation.invalidate();chooser.reset();}
    // Production publication runs on this panel's worker and completes before
    // success feedback. The processor outlives the joined panel worker.
    using WorkerCommit=std::function<bool(const std::shared_ptr<Folder>&,const Input&,juce::String&,const storage::portable::Hooks&)>;
    WorkerCommit publishOnWorker;
    void setFooterHost(juce::Component& host){externalFooter=true;for(auto* action:{&choose,&use,&stop})host.addAndMakeVisible(action);refresh();}
    void layoutFooter(juce::Rectangle<int> area){const auto width=(area.getWidth()-spacing::related)/2;choose.setBounds(area.removeFromLeft(width));area.removeFromLeft(spacing::related);use.setBounds(area);stop.setBounds(choose.getBounds().getUnion(use.getBounds()));}
    int preferredHeight(int width){setSize(width,std::max(1,getHeight()));return content.getHeight();}
    void setAvailable(bool value){available=value;refresh();}
    void setRecoveryNeeded(bool value){recoveryNeeded=value;choose.setButtonText(value?"Choose recovery folder...":"Choose folder...");
        help.setText(value?"Choose an empty recovery workspace. Swell metadata, original locators and verified surviving files are retained. Missing assets stay unavailable; this is not a complete copy or backup. Use This Folder explicitly adopts it, then use normal Learn to capture fresh audio.":"Choose a dedicated asset folder beside your DAW project, or keep the current user-data folder. Choosing a folder copies and verifies this instance's saved assets. Use This Folder then changes where future assets are written. Originals stay in place.",juce::dontSendNotification);}
    void resetForSession(){const auto reply=worker.answer.load();if(busy() || (reply && reply!=shown && reply->committed))return;
        Input current;juce::String error;if(provider(current,error)){
            if(candidate && same(current,operation))return;
            if(reply && reply->committed && reply->folder)if(const auto xml=juce::XmlDocument::parse(current.processorXml);xml)
                if(const auto* association=xml->getChildByName("WorkingFolder");association && association->getStringAttribute("writer")==storage::portable::workingFolderInfo(*reply->folder).association.writer)return;
        }
        candidate.reset();setStatus({});refreshLocation();refresh();}
    void locateExisting(juce::Component* opener=nullptr){select(true,{},opener);}
    void restoreFromBackup(const juce::File& receipt,juce::Component* opener=nullptr){select(false,receipt,opener);}
    Picker picker;
    bool busy()const{return worker.active.load();}
    bool occupied()const{return busy() || choosing;}
    bool ready()const{return candidate!=nullptr;}
    juce::String statusText()const{return status.getText();}
    void visibilityChanged()override{if(!isShowing()){worker.cancel=true;candidate.reset();refresh();}}
    void poll()
    {
        const auto pending=worker.answer.load();
        if(!isShowing() && !occupied() && (!pending || pending==shown))return;
        refreshLocation();
        if(busy())return;const auto reply=worker.answer.load();if(!reply || reply==shown)return;shown=reply;
        if(reply->committed){candidate.reset();refreshLocation();setStatus(reply->folder && storage::portable::workingFolderInfo(*reply->folder).incompleteRecovery
            ?"Recovery workspace active. Some assets remain missing; swell markers and original locators are preserved. Use normal Learn with Detect events off and Recapture audio on. Save your DAW project."
            :"Working folder changed. Save your DAW project to keep this association. Original assets remain in their previous folder.");refresh();return;}
        Input currentInput;juce::String error;
        if(!provider(currentInput,error) || !same(currentInput,operation)){
            candidate.reset();setStatus("The session changed. Prepare the folder again; the current folder is retained.");refresh();return;}
        if(!isShowing() || worker.cancel.load()){
            candidate.reset();setStatus("Operation stopped. The current folder and originals are retained.");refresh();return;}
        if(!reply->success){candidate.reset();setStatus("Could not finish. "+reply->error);refresh();return;}
        if(reply->use){
            if(!publish || !publish(reply->folder,operation,error)){candidate.reset();setStatus("The current folder is retained. "+error);refresh();return;}
            candidate.reset();refreshLocation();setStatus("Working folder changed. Save your DAW project to keep this association. Original assets remain in their previous folder.");
        }else{
            candidate=reply->folder;const auto& info=storage::portable::workingFolderInfo(*candidate);
            selected.setText("Prepared: "+info.folder.getFullPathName(),juce::dontSendNotification);selected.setTooltip(info.folder.getFullPathName());selected.setDescription(info.folder.getFullPathName());
            if(info.incompleteRecovery)setStatus("Incomplete recovery workspace prepared: "+juce::String(info.missing.size())+" missing asset paths. Swell metadata and original locators are retained. This is not a complete copy or backup. Use This Folder to adopt it explicitly.");else setStatus("Verified "+juce::String(info.files)+" asset paths / "+juce::File::descriptionOfSizeInBytes(static_cast<juce::int64>(info.bytes))+". The current working folder has not changed.");
        }
        refresh();
    }
    void resized()override
    {
        auto bounds=getLocalBounds();if(!externalFooter){auto footer=bounds.removeFromBottom(controlGeometry::height+2*spacing::related).reduced(spacing::section,spacing::related);layoutFooter(footer);}
        viewport.setBounds(bounds);
        const auto contentWidth=std::max(1,viewport.getWidth()-viewport.getScrollBarThickness());
        auto top=spacing::section;const auto inner=std::max(1,contentWidth-2*spacing::section);
        const auto place=[&](juce::Label& label,int height){label.setBounds(spacing::section,top,inner,height);top+=height+spacing::related;};
        const auto messageHeight=[&](const controls::WrappedLabel& label){return std::max(learnTypography::secondaryLineHeight,label.preferredHeight(inner));};
        place(heading,controlGeometry::headingHeight);place(current,controlGeometry::labelHeight);place(help,messageHeight(help));
        selected.setVisible(candidate!=nullptr);if(candidate)place(selected,controlGeometry::labelHeight);
        status.setVisible(status.getText().isNotEmpty());if(status.isVisible())place(status,messageHeight(status));content.setSize(contentWidth,top);
    }
private:
    static bool same(const Input& a,const Input& b){return a.instanceRoot==b.instanceRoot && a.controlEpoch==b.controlEpoch && a.processorXml==b.processorXml && a.manifestJson==b.manifestJson && a.sourceUnavailable==b.sourceUnavailable && a.folderUnavailable==b.folderUnavailable;}
    void setStatus(const juce::String& text){status.setText(text,juce::dontSendNotification);status.setDescription(text);resized();}
    void refreshLocation(){Input input;juce::String error;if(provider(input,error)){
        auto folder=input.instanceRoot.getParentDirectory().getParentDirectory();
        if(const auto xml=juce::XmlDocument::parse(input.processorXml);xml && xml->getIntAttribute("stateVersion")==9)folder=folder.getParentDirectory();
        const auto text=input.folderUnavailable?juce::String("Associated assets unavailable: locate the folder or choose a recovery workspace"):"Current: "+folder.getFullPathName();
        current.setText(text,juce::dontSendNotification);current.setTooltip(text);current.setDescription(text);
    }}
    void refresh(){const bool idle=available && !busy() && !choosing;choose.setVisible(!busy());use.setVisible(!busy());choose.setEnabled(idle);use.setEnabled(idle && candidate!=nullptr);stop.setVisible(busy());resized();}
    void select(bool locate=false,juce::File receipt={},juce::Component* opener=nullptr)
    {
        if(!available || busy() || choosing)return;Input input;juce::String error;if(!provider(input,error)){setStatus(error);return;}
        const auto ticket=pickerInvocation.begin(opener?*opener:choose,this);if(!ticket)return;
        choosing=true;refresh();auto complete=[safe=juce::Component::SafePointer<WorkingFolderPanel>(this),input,locate,receipt,ticket=*ticket](juce::File folder){
            if(!safe || !ticket.complete())return;safe->choosing=false;if(folder==juce::File{} || !safe->isShowing()){safe->refresh();return;}
            Input current;juce::String error;if(!safe->provider(current,error) || !same(current,input)){safe->candidate.reset();safe->setStatus("The session changed while choosing. Prepare the folder again.");safe->refresh();return;}
            safe->operation=input;safe->candidate.reset();const auto registry=safe->registryRoot;const bool recovery=safe->recoveryNeeded;const auto commit=safe->publishOnWorker;
            const bool started=safe->worker.begin([input,folder,registry,recovery,locate,commit,receipt](const auto& hooks){Reply reply;reply.success=receipt!=juce::File{}?storage::portable::prepareRestoredWorkingFolder(input,receipt,folder,registry,reply.folder,reply.error,hooks):locate?storage::portable::prepareAssociatedLocation(input,folder,registry,reply.folder,reply.error,hooks):recovery?storage::portable::prepareRecoveryWorkspace(input,folder,registry,reply.folder,reply.error,hooks):storage::portable::prepareWorkingFolder(input,folder,registry,reply.folder,reply.error,hooks);
                if((locate || receipt!=juce::File{}) && reply.success && commit && !(hooks.cancelled && hooks.cancelled()))reply.success=reply.committed=commit(reply.folder,input,reply.error,hooks);return reply;});
            safe->setStatus(started?"Copying and verifying assets...":"Could not start the copy. The current folder is retained.");safe->refresh();};
        if(picker)picker(std::move(complete));else{chooser=std::make_unique<juce::FileChooser>((receipt!=juce::File{}?"Choose an empty folder for restored project assets":locate?"Locate the associated asset store folder":recoveryNeeded?"Choose an empty recovery workspace; this is not a complete backup":"Choose a dedicated asset working folder"),juce::File{},"*");chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectDirectories,[complete=std::move(complete)](const auto& dialog){complete(dialog.getResult());});}
    }
    void activate()
    {
        if(!available || busy() || choosing || !candidate)return;Input current;juce::String error;
        if(!provider(current,error) || !same(current,operation)){candidate.reset();setStatus("The session changed. Prepare the folder again.");refresh();return;}
        const auto prepared=candidate;const auto commit=publishOnWorker;
        const bool started=worker.begin([prepared,current,commit](const auto& hooks){Reply reply;reply.folder=prepared;reply.use=true;
            reply.success=commit?true:storage::portable::workingFolderInfo(*prepared).incompleteRecovery?storage::portable::revalidateRecoveryWorkspace(*prepared,current,reply.error,hooks):storage::portable::revalidateWorkingFolder(*prepared,current,reply.error,hooks);
            if(reply.success && commit){if(hooks.cancelled && hooks.cancelled()){reply.success=false;reply.error="Operation stopped before handoff";}
                else reply.success=reply.committed=commit(prepared,current,reply.error,hooks);}return reply;});
        setStatus(started?"Checking the prepared folder before switching...":"Could not start verification. The current folder is retained.");refresh();
    }
    void timerCallback()override{poll();}
    Provider provider;juce::File registryRoot;Commit publish;Input operation;bool choosing=false,externalFooter=false,available=true,recoveryNeeded=false;
    controls::SurfaceInvocation pickerInvocation;
    Worker worker;std::shared_ptr<Folder> candidate;std::shared_ptr<const Reply> shown;std::unique_ptr<juce::FileChooser> chooser;
    FlatSurface content;controls::FocusViewport viewport;controls::EllipsisLabel heading;controls::WrappedLabel help,status;controls::EllipsisLabel current,selected;
    controls::ActionButton choose{"Choose folder..."},use{"Use This Folder"},stop{"Stop operation"};
};
}
