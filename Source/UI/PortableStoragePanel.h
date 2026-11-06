#pragma once
#include <Storage/PortableAssets.h>
#include <UI/StorageInventoryPanel.h>
#include <UI/Controls/FocusViewport.h>

namespace m3::ui
{
// Owner-approved Storage composition; integration preserves the live association.
// The host binding owns accepted metadata and restore destination; this view
// never changes the live association, clears a map or enables cleanup.
class PortableStoragePanel final:public FlatSurface,private juce::Timer
{
    using Input=storage::portable::Input;using Result=storage::portable::Result;using Locator=storage::portable::Locator;
    class Worker final:public juce::Thread
    {
    public:
        Worker():Thread("Portable asset backup"){}
        ~Worker()override{cancel=true;signalThreadShouldExit();stopThread(-1);}
        bool begin(std::function<Result(const storage::portable::Hooks&)> next)
        {if(active.load())return false;stopThread(-1);task=std::move(next);answer.store(nullptr);cancel=false;active=true;if(!startThread()){active=false;return false;}return true;}
        std::atomic<bool> cancel{false},active{false};std::atomic<std::shared_ptr<const Result>> answer;
    private:
        void run()override{storage::portable::Hooks hooks;hooks.cancelled=[this]{return cancel.load() || threadShouldExit();};
            try{answer.store(std::make_shared<Result>(task(hooks)));}catch(const std::exception& e){Result failed;failed.error=e.what();answer.store(std::make_shared<Result>(failed));}active=false;}
        std::function<Result(const storage::portable::Hooks&)> task;
    };
public:
    using Provider=std::function<bool(Input&,juce::String&)>;
    using FolderPicker=std::function<void(const juce::String&,const juce::File&,std::function<void(juce::File)>)>;
    PortableStoragePanel(Provider source,juce::File fallback):provider(std::move(source)),restoreRoot(std::move(fallback)),inventory([this]{return inventoryRequest();})
    {
        setName("portableStorage");contentViewport.setName("storageContentViewport");contentViewport.setFocusRevealEnabled(true);contentViewport.setViewedComponent(&content,false);addAndMakeVisible(contentViewport);content.addAndMakeVisible(inventory);
        heading.setText("Backup and recovery",juce::dontSendNotification);applyTextRole(heading,TextRole::sectionHeading);content.addAndMakeVisible(heading);
        applyTextRole(status,TextRole::helper,juce::Justification::topLeft);status.setName("portableStatus");content.addAndMakeVisible(status);
        applyTextRole(destination,TextRole::helper);destination.setName("portableDestination");destination.setTitle("Backup folder");content.addAndMakeVisible(destination);
        applyTextRole(recoveryDestination,TextRole::helper);recoveryDestination.setTitle("Restore destination");recoveryDestination.setName("portableRestoreDestination");recoveryDestination.setText("Restore destination: "+restoreRoot.getFullPathName(),juce::dontSendNotification);recoveryDestination.setDescription(restoreRoot.getFullPathName());content.addAndMakeVisible(recoveryDestination);
        backupButton.setName("backupAssets");restoreButton.setName("restoreAssets");locateButton.setName("locateAssets");cancelButton.setName("cancelAssetBackup");
        backupButton.setTooltip("Copy this instance's accepted audio, IRs, saved wet results and metadata into a selected backup folder. Originals stay in place.");
        restoreButton.setTooltip("Select a completed backup receipt, or locate the known backup folder. Verify it, then copy its exact assets into the recovery destination. Existing conflicting files are preserved.");
        locateButton.setTooltip("Select a completed backup receipt, or verify the known backup folder. No files are copied or live settings changed.");
        cancelButton.setTooltip("Stop this operation. Originals remain intact; interrupted backup copies can be inspected and resumed.");
        for(auto* button:{&backupButton,&restoreButton,&locateButton,&cancelButton})addAndMakeVisible(button);
        backupButton.onClick=[this]{choose(Action::backup);};restoreButton.onClick=[this]{choose(Action::restore);};locateButton.onClick=[this]{if(!onRestored){choose(Action::locate);return;}
            juce::PopupMenu menu;menu.addItem(1,"Original asset folder (copy to user data)");menu.addItem(2,"Completed backup receipt");
            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&locateButton),[safe=juce::Component::SafePointer<PortableStoragePanel>(this)](int choice){if(!safe)return;if(choice==1)safe->choose(Action::legacy);if(choice==2){safe->known={};safe->choose(Action::locate);}});};cancelButton.onClick=[this]{worker.cancel=true;setStatus("Stopping safely. Originals and previous backups remain intact.");};
        setStatus("Save the DAW project and external audio separately.");refreshButtons();startTimerHz(10);
    }
    ~PortableStoragePanel()override{destroying=true;stopTimer();worker.cancel=true;}
    std::function<bool(const Input&,juce::String&)> onRestored;
    void showRecoveryStatus(const juce::String& message){setStatus(message);resized();}
    void locateOriginalAssets(){choose(Action::legacy);}
    FolderPicker folderPicker,receiptPicker; // Tests inject selection; otherwise native choosers.
    void bind(Provider next)
    {
        worker.cancel=true;waitingReload=false;provider=std::move(next);known={};shown.reset();selected={};
        inventory.bind([this]{return inventoryRequest();});
        setStatus("Save the DAW project and external audio separately.");refreshButtons();
    }
    bool scanning()const{return inventory.scanning();}
    const storage::Report* displayedReport()const{return inventory.displayedReport();}

    void setKnownBackup(Locator locator,juce::File folder){known=std::move(locator);selected=std::move(folder);Input input;juce::String error;if(provider(input,error)){knownRoot=input.instanceRoot;knownEpoch=input.controlEpoch;}refreshButtons();}
    bool busy()const{return worker.active.load();}
    const Result* lastResult()const{return shown.get();}
    juce::String statusText()const{return status.getText();}
    void visibilityChanged()override{if(!isShowing())worker.cancel=true;}
    void poll()
    {
        inventory.poll();resized();
        if(waitingReload){Input current;juce::String error;if(provider(current,error)){
            waitingReload=false;if(current.instanceRoot!=operationRoot || !sameSource(current,operationInput)){setStatus("Session changed; previous recovery result was not adopted.");return;}inventory.refresh();setStatus(current.sourceUnavailable?"Assets copied, but source reload failed. Originals and edits are retained.":"Assets copied and source reloaded. Current edits are retained.");resized();}}
        if(busy())return;const auto reply=worker.answer.load();if(!reply || reply==shown)return;shown=reply;
        Input current;juce::String currentError;const bool sameScope=provider(current,currentError) && current.instanceRoot==operationRoot && current.controlEpoch==operationEpoch;
        if(!sameScope){known={};setStatus("The operation finished for an earlier session. The current session and its folder were not changed.");refreshButtons();return;}
        if(reply->success && onRestored && (currentAction==Action::restore || currentAction==Action::legacy)){
            juce::String error;if(!onRestored(operationInput,error)){setStatus("Assets copied; current state retained. "+error);refreshButtons();return;}
            waitingReload=true;setStatus("Assets copied; reloading source...");known={};refreshButtons();return;}
        if(reply->success){known=reply->receipt.locator;knownRoot=operationRoot;knownEpoch=operationEpoch;const auto verb=currentAction==Action::backup?"Backup complete":currentAction==Action::restore?"Assets restored to recovery folder":"Backup verified";
            setStatus(juce::String(verb)+": "+juce::String(reply->receipt.files)+" asset paths / "+juce::File::descriptionOfSizeInBytes(static_cast<juce::int64>(reply->receipt.bytes))+".\n"+reply->receipt.exclusions+"");}
        else {setStatus(reply->cancelled?"Operation stopped. Originals and previous backups are safe.":"Could not finish. Check the backup and destination; originals are safe.");status.setDescription(status.getText()+"\nDetails: "+reply->error);}
        refreshButtons();
    }
    void resized()override
    {
        auto bounds=getLocalBounds();auto footer=bounds.removeFromBottom(controlGeometry::height+2*spacing::related);
        contentViewport.setBounds(bounds);footer=footer.reduced(spacing::section,spacing::related);
        const auto width=(footer.getWidth()-2*spacing::related)/3;
        backupButton.setBounds(footer.removeFromLeft(width));footer.removeFromLeft(spacing::related);
        restoreButton.setBounds(footer.removeFromLeft(width));footer.removeFromLeft(spacing::related);locateButton.setBounds(footer);
        cancelButton.setBounds(getLocalBounds().withTrimmedTop(bounds.getHeight()).reduced(spacing::section,spacing::related));
        const auto contentWidth=std::max(1,contentViewport.getWidth()-contentViewport.getScrollBarThickness());
        const auto inventoryHeight=inventory.preferredHeight(contentWidth);
        juce::AttributedString statusText(status.getText());statusText.setFont(status.getFont());juce::TextLayout layout;layout.createLayout(statusText,static_cast<float>(std::max(1,contentWidth-2*spacing::section)));
        const auto statusHeight=std::max(learnTypography::secondaryLineHeight,static_cast<int>(std::ceil(layout.getHeight())))+spacing::tight;
        const auto destinationHeight=selected==juce::File{}?0:controlGeometry::labelHeight+spacing::tight;destination.setVisible(destinationHeight!=0);
        const auto contentHeight=inventoryHeight+spacing::related+controlGeometry::headingHeight+spacing::related
            +destinationHeight+controlGeometry::labelHeight+spacing::tight+statusHeight+spacing::related;
        content.setSize(contentWidth,contentHeight);inventory.setBounds(0,0,contentWidth,inventoryHeight);
        auto area=content.getLocalBounds().withTrimmedTop(inventoryHeight+spacing::related).reduced(spacing::section,0);
        heading.setBounds(area.removeFromTop(controlGeometry::headingHeight));area.removeFromTop(spacing::related);
        if(destinationHeight){destination.setBounds(area.removeFromTop(controlGeometry::labelHeight));area.removeFromTop(spacing::tight);}
        recoveryDestination.setBounds(area.removeFromTop(controlGeometry::labelHeight));area.removeFromTop(spacing::tight);
        status.setBounds(area.removeFromTop(statusHeight));
    }

private:
    static bool sameSource(const Input& a,const Input& b){const auto x=juce::XmlDocument::parse(a.processorXml),y=juce::XmlDocument::parse(b.processorXml);return x && y && x->getStringAttribute("manifestReference")==y->getStringAttribute("manifestReference") && x->getStringAttribute("manifestRevision")==y->getStringAttribute("manifestRevision");}
    enum class Action{backup,restore,locate,legacy};Action currentAction=Action::backup;
    storage::Request inventoryRequest()const
    {
        storage::Request request;Input input;juce::String error;
        if(!provider(input,error)){request.observationError=error;return request;}
        request.roots={input.instanceRoot};storage::ObservedSnapshot observed;
        if(storage::readLegacySnapshot(input.processorXml,input.manifestJson,observed,error)){observed.label="Current accepted instance";request.snapshots.push_back(std::move(observed));}
        else request.observationError=error;return request;
    }
    void setStatus(const juce::String& value){status.setText(value,juce::dontSendNotification);status.setDescription(value);}
    void refreshButtons(){const bool idle=!busy() && !choosing;backupButton.setEnabled(idle);restoreButton.setEnabled(idle);locateButton.setEnabled(idle);cancelButton.setVisible(busy());for(auto* button:{&backupButton,&restoreButton,&locateButton})button->setVisible(!busy());resized();}
    void choose(Action action)
    {
        if(destroying || busy() || choosing)return;Input input;juce::String error;if(!provider(input,error)){setStatus(error);return;}
        if(knownRoot!=input.instanceRoot || knownEpoch!=input.controlEpoch)known={};
        const bool receiptSelection=(action==Action::restore || action==Action::locate) && known.snapshot.isEmpty();
        choosing=true;refreshButtons();const auto title=action==Action::legacy?"Choose the original project instance asset folder":action==Action::backup?"Choose an empty folder or an existing asset backup":receiptSelection?"Choose the exact completed backup receipt inside receipts":"Choose the folder containing this saved backup";
        auto complete=[safe=juce::Component::SafePointer<PortableStoragePanel>(this),action,input,receiptSelection](juce::File folder){if(!safe || safe->destroying)return;safe->choosing=false;
            if(!safe->isShowing()){safe->refreshButtons();return;}
            if(folder==juce::File{}){safe->refreshButtons();return;}Input current;juce::String error;
            if(!safe->provider(current,error) || current.instanceRoot!=input.instanceRoot || current.controlEpoch!=input.controlEpoch){safe->setStatus("The active session changed while the folder picker was open. Reopen Backup and try again.");safe->refreshButtons();return;}
            safe->start(action,folder,input,receiptSelection);};
        auto& picker=receiptSelection?receiptPicker:folderPicker;
        if(picker)picker(title,selected,std::move(complete));else{chooser=std::make_unique<juce::FileChooser>(title,selected,receiptSelection?"*.json":"*");chooser->launchAsync(juce::FileBrowserComponent::openMode|(receiptSelection?juce::FileBrowserComponent::canSelectFiles:juce::FileBrowserComponent::canSelectDirectories),
            [complete=std::move(complete)](const juce::FileChooser& dialog){complete(dialog.getResult());});}
    }
    void start(Action action,const juce::File& folder,const Input& input,bool receiptSelection)
    {
        currentAction=action;operationInput=input;selected=receiptSelection?folder.getParentDirectory().getParentDirectory():folder;operationRoot=input.instanceRoot;operationEpoch=input.controlEpoch;shown.reset();const auto expected=known;const auto fallback=restoreRoot;
        destination.setText("Backup folder: "+selected.getFullPathName(),juce::dontSendNotification);destination.setDescription(selected.getFullPathName());destination.setTooltip(selected.getFullPathName());
        const bool started=worker.begin([action,input,folder,expected,fallback,receiptSelection](const storage::portable::Hooks& hooks){Result result;juce::String error;
            if(action==Action::backup){std::shared_ptr<storage::portable::Prepared> prepared;if(!storage::portable::prepare(input,prepared,error,hooks)){result.error=error;result.cancelled=hooks.cancelled && hooks.cancelled();return result;}return storage::portable::backup(prepared,folder,hooks);}
            if(action==Action::legacy){std::shared_ptr<storage::portable::Prepared> prepared;if(!storage::portable::prepareLegacyRecovery(folder,input,prepared,error,hooks)){result.error=error;return result;}return storage::portable::restoreLegacyCopy(prepared,fallback,hooks);}
            std::shared_ptr<storage::portable::Verified> verified;if(!(receiptSelection?storage::portable::locateReceipt(folder,verified,error,hooks):storage::portable::locate(folder,expected,verified,error,hooks))){result.error=error;result.cancelled=hooks.cancelled && hooks.cancelled();return result;}
            const auto& receipt=storage::portable::receipt(*verified);
            if(receipt.project!=input.instanceRoot.getParentDirectory().getFileName() || receipt.instance!=input.instanceRoot.getFileName())
            {result.error="The selected backup belongs to another project or instance. Open its matching DAW project first.";return result;}
            if(action==Action::restore){if(!storage::portable::matchesRecoveryTarget(*verified,input,error)){result.error=error;return result;}return storage::portable::restoreCopy(verified,fallback,hooks);}
            result.success=true;result.receipt=storage::portable::receipt(*verified);return result;});
        setStatus(started?(action==Action::backup?"Copying and verifying assets...":"Verifying backup..."):"The worker could not start. Try again.");refreshButtons();
    }
    void timerCallback()override{poll();}
    Provider provider;juce::File restoreRoot,selected,operationRoot,knownRoot;Locator known;uint64_t operationEpoch=0,knownEpoch=0;bool choosing=false,destroying=false,waitingReload=false;Input operationInput;
    Worker worker;std::shared_ptr<const Result> shown;std::unique_ptr<juce::FileChooser> chooser;
    FlatSurface content;controls::FocusViewport contentViewport;
    StorageInventoryPanel inventory;juce::Label heading,status;controls::EllipsisLabel destination,recoveryDestination;
    controls::ActionButton backupButton{"Back up assets..."},restoreButton{"Restore backup..."},locateButton{"Locate assets..."},cancelButton{"Stop operation"};
};
}
