#pragma once
#include "Controls/ConvolutionIrEditor.h"
#include "Controls/SurfaceLifecycle.h"
#include <DSP/SwellEngine.h>
#include <variant>

namespace m3::ui
{
// UI-thread adapter. Engine commands and worker publication remain the authority
// for validation; no state or audio settings are owned by the shared control.
class ConvolutionIrBinding final
{
public:
    struct FutureDefault {uint64_t session=0,pass=0;};
    struct SelectedSwell {uint64_t session=0;juce::String id;uint64_t input=0;};
    using Target=std::variant<FutureDefault,SelectedSwell>;
    ConvolutionIrBinding(controls::ConvolutionIrEditor& component,SwellEngine& model)
        :editor(component),engine(model),lifetime(std::make_shared<Lifetime>())
    {
        lifetime->owner=this;
        editor.beforeInvocation=[this]{invocation=Request{target,choices};};
        editor.onChoice=[this](const auto& content)
        {
            ++fileSerial;
            const auto request=std::exchange(invocation,std::nullopt).value_or(Request{target,choices});
            for(const auto& entry:request.choices)if(entry.content==content){submit(request.target,entry,{});break;}
            if(onRefresh)onRefresh();
        };
        editor.onImport=[this]{openFile();};
    }
    ~ConvolutionIrBinding()
    {
        lifetime->owner=nullptr;editor.beforeInvocation={};editor.onChoice={};editor.onImport={};
    }
    ConvolutionIrBinding(const ConvolutionIrBinding&)=delete;
    ConvolutionIrBinding& operator=(const ConvolutionIrBinding&)=delete;

    void displayDefault(const EditorSnapshot& state)
    {display(state,FutureDefault{state.sessionEpoch,state.workflowState.pass});}
    void displaySelected(const EditorSnapshot& state)
    {
        const auto* swell=find(state.session,state.session.primary);
        display(state,SelectedSwell{state.sessionEpoch,swell?swell->id:juce::String{},swell?swell->inputGeneration:0});
    }
    // The same captured continuation is used by the native chooser and tests.
    // It remains valid only for this binding's latest import/choice invocation.
    std::function<void(const juce::File&)> beginFileImport()
    {
        const auto captured=target;const auto serial=++fileSerial;
        return [weak=std::weak_ptr<Lifetime>(lifetime),captured,serial](const juce::File& file)
        {
            const auto keep=weak.lock();if(!keep || !keep->owner || file==juce::File{})return;
            auto& owner=*keep->owner;if(serial!=owner.fileSerial)return;
            ++owner.fileSerial;owner.submit(captured,{},file);
            if(owner.onRefresh)owner.onRefresh();
        };
    }
    void openFile(juce::Component* opening=nullptr)
    {
        const auto ticket=pickerInvocation.begin(opening?*opening:editor.fileButton,opening?opening->getTopLevelComponent():&editor.selector);if(!ticket)return;
        auto complete=beginFileImport();
        auto deliver=[ticket=*ticket,complete=std::move(complete)](const juce::File& file){if(ticket.complete())complete(file);};
        if(chooseFile){chooseFile(std::move(deliver));return;}
        chooser=std::make_unique<juce::FileChooser>("Import impulse response",juce::File{},"*.wav;*.wv");
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
            [deliver=std::move(deliver)](const auto& dialog){deliver(dialog.getResult());});
    }
    std::function<void(std::function<void(const juce::File&)>)> chooseFile;
    std::function<void()> onRefresh;
private:
    struct Request {Target target;std::vector<IRChoice> choices;};
    struct Lifetime {ConvolutionIrBinding* owner=nullptr;};
    bool submit(const Target& destination,const IRChoice& choice,const juce::File& file)
    {
        return std::visit([&](const auto& owner)
        {
            // Zero is the legacy engine wildcard. A UI intent must always carry
            // an actual session token, including unavailable/empty presentation.
            if(owner.session==0)return false;
            if constexpr(std::is_same_v<std::decay_t<decltype(owner)>,FutureDefault>)
            {
                if(file!=juce::File{})return engine.assignDefaultIR(file,owner.session,owner.pass);
                if(choice.presetId.isNotEmpty())return engine.assignDefaultBundledIR(choice.presetId,owner.session,owner.pass);
                return engine.assignDefaultManagedIR(choice.managed,choice.name,owner.session,owner.pass);
            }
            else
            {
                if(owner.id.isEmpty())return false;
                if(file!=juce::File{})return engine.assignIR(owner.id,owner.input,file,false,owner.session);
                if(choice.presetId.isNotEmpty())return engine.assignBundledIR(owner.id,owner.input,choice.presetId,owner.session);
                return engine.assignManagedIR(owner.id,owner.input,choice.managed,choice.name,owner.session);
            }
        },destination);
    }
    void display(const EditorSnapshot& state,Target destination)
    {
        target=std::move(destination);choices=engine.irChoices();
        controls::ConvolutionIrEditor::View view;
        for(const auto& entry:choices)view.choices.push_back({entry.content,entry.name,entry.description});
        if(std::holds_alternative<FutureDefault>(target))
        {
            view.label="Default IR";view.accepted=state.session.defaultIR.content;
            view.editable=!state.workflowState.locked() && state.workflowState.mode!=workflow::Mode::finalise;
            view.reason=view.editable?juce::String{}:"Defaults are locked during capture, settlement or Export.";
            const auto acceptedName=state.session.defaultIR.present()?state.session.defaultIRName:juce::String("None");
            view.error=state.defaultIRError.isNotEmpty();
            view.status=state.defaultIRPending?"Requested: "+state.requestedDefaultIRName+" | Accepted: "+acceptedName
                :view.error?"Couldn't load IR. Choose another or retry."
                :state.session.defaultIR.present()?juce::String{}:"Default IR unavailable";
            if(view.error)view.reason+=(view.reason.isEmpty()?"":"\n")+state.defaultIRError+"\nPrevious accepted default: "+acceptedName;
        }
        else
        {
            view.label="IR";
            const auto* swell=find(state.session,std::get<SelectedSwell>(target).id);
            view.editable=swell && !swell->frozen && state.workflowState.mode==workflow::Mode::live && !state.workflowState.locked();
            view.reason=!swell?"Select a swell to edit its IR.":swell->frozen?"Unfreeze this swell before changing its IR.":!view.editable?"IR editing is available in Perform.":juce::String{};
            if(swell)
            {
                view.accepted=swell->ir.content;
                const auto request=state.irRequests.find(swell->id);
                view.error=swell->phase==Phase::error || (request!=state.irRequests.end() && request->second.error.isNotEmpty());
                view.status=swell->phase==Phase::ready
                    ?(swell->wetTimingIntent?"Wet timing adjusted to fit this IR":juce::String{}):swell->status;
                if(request!=state.irRequests.end())
                {
                    const auto& [name,error]=request->second;
                    const juce::String available=swell->phase==Phase::ready?" | Available: "+swell->irName:juce::String(" | No ready wet render");
                    view.status=error.isEmpty()?"Preparing "+name+available:name+": "+error+available;
                    view.reason+=(view.reason.isEmpty()?"":"\n")+view.status;
                }
                if(swell->frozen)view.status="Unfreeze to change this IR";
                if(view.error && request==state.irRequests.end())
                {
                    view.reason+=(view.reason.isEmpty()?"":"\n")+swell->status;
                    view.status=state.retainedIRName.isEmpty()?"IR preparation failed. Choose another or retry."
                        :"IR failed. Audible: "+state.retainedIRName+" | Requested: "+state.requestedIRName+". Choose another or retry.";
                }
            }
            else view.status="No swell selected";
        }
        editor.display(std::move(view));
    }
    controls::ConvolutionIrEditor& editor;SwellEngine& engine;
    Target target{FutureDefault{}};std::vector<IRChoice> choices;std::optional<Request> invocation;
    std::shared_ptr<Lifetime> lifetime;uint64_t fileSerial=0;
    std::unique_ptr<juce::FileChooser> chooser;
    controls::SurfaceInvocation pickerInvocation;
};
}
