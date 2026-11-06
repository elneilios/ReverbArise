#pragma once
#include <UI/PortableStoragePanel.h>
#include <UI/Controls/InvocationToggle.h>

namespace m3::ui
{
// Feature composition. All filesystem validation/render admission runs in the
// worker callbacks; the screen never clears an arrangement or adopts backup edits.
class RecoveryPanel final:public FlatSurface,private juce::Timer
{
    using Input=storage::portable::Input;
    struct Reply{bool success=false,check=false;juce::String error;};
    class Worker final:public juce::Thread
    {
    public:
        Worker():Thread("Recovery validation"){}
        ~Worker()override{stopThread(-1);}
        void begin(std::function<Reply()> next){stopThread(-1);task=std::move(next);answer.store(nullptr);active=true;startThread();}
        std::atomic<bool> active{false};std::atomic<std::shared_ptr<const Reply>> answer;
    private:
        void run()override{try{answer.store(std::make_shared<Reply>(task()));}catch(const std::exception& e){auto reply=std::make_shared<Reply>();reply->error=e.what();answer.store(reply);}active=false;}
        std::function<Reply()> task;
    };
public:
    using Provider=PortableStoragePanel::Provider;
    using Begin=std::function<bool(const Input&,juce::String&)>;
    using Generate=std::function<bool(bool,juce::String&,bool)>;
    RecoveryPanel(Provider source,juce::File destination,Begin reload,Begin arm,std::function<void()> cancel,Generate generation)
        :provider(std::move(source)),begin(std::move(arm)),cancelRecovery(std::move(cancel)),generate(std::move(generation)),storage(provider,std::move(destination))
    {
        setName("assetRecovery");addAndMakeVisible(storage);storage.onRestored=std::move(reload);
        armButton.setName("relearnSong");cancelButton.setName("cancelRelearning");generateButton.setName("regenerateRecoveredSwells");
        frozenConsent.setName("replaceMissingFrozenResults");frozenConsent.setTooltip("Explicitly allow replacement of missing frozen audio. Surviving frozen results stay unchanged; old files are retained.");
        armButton.setTooltip("Review recovery limits, then explicitly arm a fresh DAW playback capture. Existing source and swell edits remain accepted until complete required coverage commits.");
        generateButton.setTooltip("Available only after complete fresh source and verified required IRs. Re-generation may sound different; missing exports and Playlist files cannot be recreated.");
        cancelButton.setTooltip("Cancel re-learning without clearing the arrangement or accepting partial source.");
        for(auto* c:std::initializer_list<juce::Component*>{&armButton,&cancelButton,&generateButton,&frozenConsent})addAndMakeVisible(c);
        armButton.onClick=[this]{this->arm();};cancelButton.onClick=[this]{cancelRecovery();phase=Phase::idle;ready=false;storage.showRecoveryStatus("Re-learning cancelled. Accepted source and swell edits are retained.");refresh();};
        frozenConsent.onClick=[this]{if(phase==Phase::fresh)check();};
        generateButton.onClick=[this]{start(false);};
        Input input;juce::String error;
        if(provider(input,error)){identity=input.instanceRoot;epoch=input.controlEpoch;if(input.sourceUnavailable)showMissing();}
        else storage.showRecoveryStatus(error);
        refresh();startTimerHz(10);
    }
    ~RecoveryPanel()override{stopTimer();}
    PortableStoragePanel& storagePanel(){return storage;}
    void poll()
    {
        storage.poll();Input current;juce::String error;
        if(provider(current,error)){
            if(current.instanceRoot!=identity || current.controlEpoch!=epoch){identity=current.instanceRoot;epoch=current.controlEpoch;phase=Phase::idle;ready=false;frozenConsent.setToggleState(false,juce::dontSendNotification);if(current.sourceUnavailable)showMissing();}
            if(phase==Phase::armed && !current.sourceUnavailable){const auto xml=juce::XmlDocument::parse(current.processorXml);if(xml && xml->getStringAttribute("manifestReference")!=originalReference){phase=Phase::fresh;check();}}
        }
        else {ready=false;phase=Phase::idle;storage.showRecoveryStatus(error);}
        const auto report=storage.displayedReport();if(phase==Phase::idle && report && report!=observed){observed=report;if(!report->dependencyVerificationComplete)showMissing();}
        if(!worker.active.load())if(const auto reply=worker.answer.load();reply && reply!=shown){shown=reply;
            if(provider(current,error) && current.instanceRoot==workRoot && current.controlEpoch==workEpoch){
                ready=reply->check && reply->success;
                storage.showRecoveryStatus(reply->success?(reply->check?"Fresh source and required IRs are verified. Choose Regenerate swells to rebuild missing audio.":"Requested swells are queued for regeneration. Existing files and surviving frozen audio are retained."):reply->error);
            }else{ready=false;storage.showRecoveryStatus("Session changed; no recovery result was adopted.");}}
        refresh();
    }
    void resized()override
    {
        auto area=getLocalBounds();auto footer=area.removeFromBottom(2*controlGeometry::height+3*spacing::related).reduced(spacing::section,spacing::related);
        storage.setBounds(area);frozenConsent.setBounds(footer.removeFromTop(controlGeometry::height));footer.removeFromTop(spacing::related);
        const auto width=(footer.getWidth()-2*spacing::related)/3;
        armButton.setBounds(footer.removeFromLeft(width));footer.removeFromLeft(spacing::related);cancelButton.setBounds(footer.removeFromLeft(width));footer.removeFromLeft(spacing::related);generateButton.setBounds(footer);
    }
private:
    enum class Phase{idle,review,armed,fresh};Phase phase=Phase::idle;
    void showMissing(){storage.showRecoveryStatus("Some project audio is unavailable. Swell positions and edits are preserved. Locate the original assets or restore a backup; otherwise re-learn the song.");}
    void arm()
    {
        if(phase!=Phase::review){phase=Phase::review;storage.showRecoveryStatus("Re-learning preserves swell positions and edits. Play the entire song in the DAW; partial capture will not replace accepted source. Rebuilt audio may sound different, and missing exports cannot be recreated. Click Arm re-learning to proceed.");refresh();return;}
        Input input;juce::String error;
        if(!provider(input,error) || !begin(input,error)){storage.showRecoveryStatus(error);return;}
        const auto xml=juce::XmlDocument::parse(input.processorXml);originalReference=xml?xml->getStringAttribute("manifestReference"):juce::String{};
        identity=input.instanceRoot;epoch=input.controlEpoch;phase=Phase::armed;ready=false;
        storage.showRecoveryStatus("Re-learning armed. Play the entire song in the DAW, then stop. Required saved source/swell spans must be fully captured; generation stays off until explicit recovery checks succeed.");refresh();
    }
    void check(){ready=false;if(!worker.active.load())start(true);}
    void start(bool validateOnly)
    {
        Input input;juce::String error;if(worker.active.load() || !provider(input,error))return;
        workRoot=input.instanceRoot;workEpoch=input.controlEpoch;shown.reset();const bool consent=frozenConsent.getToggleState();const auto operation=generate;
        worker.begin([operation,consent,validateOnly]{Reply reply;reply.check=validateOnly;reply.success=operation(consent,reply.error,validateOnly);return reply;});
        storage.showRecoveryStatus(validateOnly?"Checking fresh source, required IRs and frozen-result consent...":"Preparing explicit recovery generation...");refresh();
    }
    void refresh()
    {
        const bool idle=!worker.active.load() && !storage.busy();armButton.setButtonText(phase==Phase::review?"Arm re-learning":"Re-learn song...");
        armButton.setEnabled(idle && phase!=Phase::armed);cancelButton.setEnabled(idle && phase!=Phase::idle);generateButton.setEnabled(idle && ready);frozenConsent.setEnabled(idle && phase==Phase::fresh);
    }
    void timerCallback()override{poll();}
    Provider provider;Begin begin;std::function<void()> cancelRecovery;Generate generate;
    Worker worker;std::shared_ptr<const Reply> shown;juce::File identity,workRoot;uint64_t epoch=0,workEpoch=0;juce::String originalReference;bool ready=false;
    const storage::Report* observed=nullptr;
    PortableStoragePanel storage;
    controls::InvocationToggle frozenConsent{"Replace missing frozen results"};
    controls::ActionButton armButton{"Re-learn song..."},cancelButton{"Cancel re-learning"},generateButton{"Regenerate swells"};
};
}
