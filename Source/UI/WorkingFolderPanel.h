#pragma once
#include <Storage/WorkingFolder.h>
#include <UI/Controls/ActionButton.h>
#include <UI/Controls/EllipsisLabel.h>
#include <UI/Controls/FocusViewport.h>

namespace m3::ui
{
// Native review candidate only. Production integration remains owner-gated.
// The binding owns live association publication and host save notification.
class WorkingFolderPanel final:public FlatSurface,private juce::Timer
{
    using Input=storage::portable::Input;
    using Folder=storage::portable::WorkingFolder;
    struct Reply {std::shared_ptr<Folder> folder;juce::String error;bool use=false,success=false;};
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
    ~WorkingFolderPanel()override{stopTimer();worker.cancel=true;}
    Picker picker;
    bool busy()const{return worker.active.load();}
    bool ready()const{return candidate!=nullptr;}
    juce::String statusText()const{return status.getText();}
    void visibilityChanged()override{if(!isShowing()){worker.cancel=true;candidate.reset();refresh();}}
    void poll()
    {
        if(busy())return;const auto reply=worker.answer.load();if(!reply || reply==shown)return;shown=reply;
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
            setStatus("Verified "+juce::String(info.files)+" asset paths / "+juce::File::descriptionOfSizeInBytes(static_cast<juce::int64>(info.bytes))+". The current working folder has not changed.");
        }
        refresh();
    }
    void resized()override
    {
        auto bounds=getLocalBounds();auto footer=bounds.removeFromBottom(controlGeometry::height+2*spacing::related).reduced(spacing::section,spacing::related);
        viewport.setBounds(bounds);const auto width=(footer.getWidth()-spacing::related)/2;
        choose.setBounds(footer.removeFromLeft(width));footer.removeFromLeft(spacing::related);use.setBounds(footer);stop.setBounds(choose.getBounds().getUnion(use.getBounds()));
        const auto contentWidth=std::max(1,viewport.getWidth()-viewport.getScrollBarThickness());
        auto top=spacing::section;const auto inner=std::max(1,contentWidth-2*spacing::section);
        const auto place=[&](juce::Label& label,int height){label.setBounds(spacing::section,top,inner,height);top+=height+spacing::related;};
        const auto messageHeight=[&](const juce::Label& label){juce::AttributedString text(label.getText());text.setFont(label.getFont());juce::TextLayout layout;layout.createLayout(text,static_cast<float>(inner));return std::max(learnTypography::secondaryLineHeight,static_cast<int>(std::ceil(layout.getHeight())));};
        place(heading,controlGeometry::headingHeight);place(current,controlGeometry::labelHeight);place(help,messageHeight(help));
        selected.setVisible(candidate!=nullptr);if(candidate)place(selected,controlGeometry::labelHeight);
        status.setVisible(status.getText().isNotEmpty());if(status.isVisible())place(status,messageHeight(status));content.setSize(contentWidth,top);
    }
private:
    static bool same(const Input& a,const Input& b){return a.instanceRoot==b.instanceRoot && a.controlEpoch==b.controlEpoch && a.processorXml==b.processorXml && a.manifestJson==b.manifestJson && a.sourceUnavailable==b.sourceUnavailable;}
    void setStatus(const juce::String& text){status.setText(text,juce::dontSendNotification);status.setDescription(text);resized();}
    void refreshLocation(){Input input;juce::String error;if(provider(input,error)){current.setText("Current: "+input.instanceRoot.getFullPathName(),juce::dontSendNotification);current.setTooltip(input.instanceRoot.getFullPathName());current.setDescription(input.instanceRoot.getFullPathName());}}
    void refresh(){const bool idle=!busy() && !choosing;choose.setVisible(!busy());use.setVisible(!busy());choose.setEnabled(idle);use.setEnabled(idle && candidate!=nullptr);stop.setVisible(busy());resized();}
    void select()
    {
        if(busy() || choosing)return;Input input;juce::String error;if(!provider(input,error)){setStatus(error);return;}
        choosing=true;refresh();auto complete=[safe=juce::Component::SafePointer<WorkingFolderPanel>(this),input](juce::File folder){
            if(!safe)return;safe->choosing=false;if(folder==juce::File{} || !safe->isShowing()){safe->refresh();return;}
            Input current;juce::String error;if(!safe->provider(current,error) || !same(current,input)){safe->candidate.reset();safe->setStatus("The session changed while choosing. Prepare the folder again.");safe->refresh();return;}
            safe->operation=input;safe->candidate.reset();const auto registry=safe->registryRoot;
            const bool started=safe->worker.begin([input,folder,registry](const auto& hooks){Reply reply;reply.success=storage::portable::prepareWorkingFolder(input,folder,registry,reply.folder,reply.error,hooks);return reply;});
            safe->setStatus(started?"Copying and verifying assets...":"Could not start the copy. The current folder is retained.");safe->refresh();};
        if(picker)picker(std::move(complete));else{chooser=std::make_unique<juce::FileChooser>("Choose a dedicated asset working folder",juce::File{},"*");chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectDirectories,[complete=std::move(complete)](const auto& dialog){complete(dialog.getResult());});}
    }
    void activate()
    {
        if(busy() || choosing || !candidate)return;Input current;juce::String error;
        if(!provider(current,error) || !same(current,operation)){candidate.reset();setStatus("The session changed. Prepare the folder again.");refresh();return;}
        const auto prepared=candidate;const bool started=worker.begin([prepared,current](const auto& hooks){Reply reply;reply.folder=prepared;reply.use=true;reply.success=storage::portable::revalidateWorkingFolder(*prepared,current,reply.error,hooks);return reply;});
        setStatus(started?"Checking the prepared folder before switching...":"Could not start verification. The current folder is retained.");refresh();
    }
    void timerCallback()override{poll();}
    Provider provider;juce::File registryRoot;Commit publish;Input operation;bool choosing=false;
    Worker worker;std::shared_ptr<Folder> candidate;std::shared_ptr<const Reply> shown;std::unique_ptr<juce::FileChooser> chooser;
    FlatSurface content;controls::FocusViewport viewport;juce::Label heading,help,status;controls::EllipsisLabel current,selected;
    controls::ActionButton choose{"Choose folder..."},use{"Use This Folder"},stop{"Stop operation"};
};
}
