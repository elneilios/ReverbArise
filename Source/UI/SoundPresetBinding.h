#pragma once

#include "SoundResponsePeaks.h"
#include "Controls/SurfaceLifecycle.h"
#include <DSP/SwellEngine.h>

#if JUCE_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace m3::ui
{
class SoundPresetBinding final : private juce::Timer
{
    using Editor = controls::SoundPresetEditor;
    struct Target
    {
        uint64_t session = 0, pass = 0, input = 0;
        juce::String swell;
        bool defaults = true;
        bool operator==(const Target&) const = default;
        bool sameOwner(const Target& other) const
        { return session == other.session && pass == other.pass && swell == other.swell && defaults == other.defaults; }
    };
    struct Pending
    {
        uint64_t serial;
        juce::String name;
        double since;
        bool reducedMotion;
    };

public:
    using FileResult = std::function<void(const juce::File&)>;

    SoundPresetBinding(Editor& component, SwellEngine& source,
                       std::function<double()> clock = juce::Time::getMillisecondCounterHiRes)
        : editor(component), engine(source), now(std::move(clock))
    {
        editor.beforeInvocation = [this] { invocation = target; };
        editor.onPreset = [this](const auto& id) { choose(id, false); };
        editor.onFile = [this](const auto& id) { choose(id, true); };
        editor.onImport = [this] { openFile(); };
        editor.onValueBegin = [this] { levelTarget = target; };
        editor.onValue = [this](const auto& id, double value)
        {
            if (id == "level" && active && levelTarget == target && target.defaults && target.session != 0)
            {
                engine.setDefaultLevel(static_cast<float>(value), target.session, target.pass);
                refresh();
            }
        };
        startTimerHz(10);
    }
    ~SoundPresetBinding() override
    {
        stopTimer();
        lifetime.reset();
        editor.beforeInvocation = {};
        editor.onPreset = {};
        editor.onFile = {};
        editor.onImport = {};
        editor.onValueBegin = {};
        editor.onValue = {};
    }

    std::function<void()> onRefresh;
    std::function<void(FileResult)> chooseFile;

    void displayDefault(const EditorSnapshot& state, bool visible)
    {
        display(state, nullptr, {state.sessionEpoch, state.workflowState.pass}, visible);
    }

    void displaySelected(const EditorSnapshot& state, bool visible)
    {
        const auto* swell = find(state.session, state.session.primary);
        display(state, swell, {state.sessionEpoch, state.workflowState.pass, swell ? swell->inputGeneration : 0,
                               swell ? swell->id : juce::String{}, false}, visible);
    }

    void setActive(bool visible)
    {
        if (active == visible) return;
        active = visible;
        if (!active) resetContext();
    }

    void openFile(juce::Component* opener = nullptr)
    {
        if (!editable || target.session == 0 || (!active && opener == nullptr)) return;
        if (opener == nullptr)
        {
            const auto source = std::exchange(invocation, std::nullopt);
            if (!source || *source != target) return;
        }
        auto& opening = opener ? *opener : editor.importAction();
        const auto ticket = surface.begin(opening, opener ? opener->getTopLevelComponent() : &editor.presetPicker());
        if (!ticket) return;
        customIntent = true;
        const auto owner = target;
        const auto token = ++fileSerial;
        const std::weak_ptr<int> weak = lifetime;
        FileResult complete = [this, weak, owner, token, ticket = *ticket](const juce::File& file)
        {
            if (weak.expired()) return;
            const auto focusOwner = [this, weak, owner, token]
            { return !weak.expired() && token == fileSerial && owner.sameOwner(target); };
            const bool maySubmit = token == fileSerial && owner == target;
            if (!ticket.complete(focusOwner) || !maySubmit) return;
            if (file != juce::File{}) submit(owner, {}, file);
            refresh();
        };
        if (chooseFile) chooseFile(std::move(complete));
        else
        {
            chooser = std::make_unique<juce::FileChooser>("Import impulse response", juce::File{}, "*.wav;*.wv");
            chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                [complete = std::move(complete)](const auto& dialog) { complete(dialog.getResult()); });
        }
        refresh();
    }

private:
    static constexpr double busyDelayMs = 2000.;

    static bool reducedMotion()
    {
#if JUCE_WINDOWS
        BOOL enabled = TRUE;
        return SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &enabled, 0) && enabled == FALSE;
#else
        return false;
#endif
    }

    void resetContext()
    {
        invocation.reset();
        pending.reset();
        managedRetryAt.reset();
        customIntent = false;
        ++fileSerial;
        surface.invalidate();
        editor.cancelInvocations();
    }

    void refresh() { if (onRefresh) onRefresh(); }

    void choose(const juce::String& id, bool file)
    {
        const auto source = std::exchange(invocation, std::nullopt);
        if (!active || !editable || !source || *source != target || target.session == 0) return;
        ++fileSerial;
        if (!file && id == customImpulseId)
            customIntent = true;
        else
        {
            customIntent = file;
            const auto match = std::find_if(choices.begin(), choices.end(), [&](const auto& choice)
            { return file ? choice.presetId.isEmpty() && choice.id == id : choice.presetId == id; });
            if (match != choices.end() && (pending || (file ? id != acceptedFile : id != acceptedPreset))) submit(*source, *match, {});
        }
        refresh();
    }

    void submit(const Target& owner, const IRChoice& choice, const juce::File& file)
    {
        if (owner.defaults)
        {
            if (file != juce::File{}) engine.assignDefaultIR(file, owner.session, owner.pass);
            else if (choice.presetId.isNotEmpty()) engine.assignDefaultBundledIR(choice.presetId, owner.session, owner.pass);
            else engine.assignDefaultManagedIR(choice.managed, choice.name, owner.session, owner.pass);
        }
        else
        {
            if (file != juce::File{}) engine.assignIR(owner.swell, owner.input, file, false, owner.session);
            else if (choice.presetId.isNotEmpty()) engine.assignBundledIR(owner.swell, owner.input, choice.presetId, owner.session);
            else engine.assignManagedIR(owner.swell, owner.input, choice.managed, choice.name, owner.session);
        }
    }

    void display(const EditorSnapshot& state, const Swell* swell, Target next, bool visible)
    {
        if (!target.sameOwner(next) || lastMode != state.workflowState.mode) resetContext();
        target = std::move(next);
        lastMode = state.workflowState.mode;
        setActive(visible);
        const bool locked = state.workflowState.locked();
        const bool selectedEditable = swell && !locked && state.workflowState.mode == workflow::Mode::live;
        editable = target.defaults ? !locked && state.workflowState.mode != workflow::Mode::finalise
                                  : selectedEditable && !swell->frozen;
        Editor::View view;
        view.owner = juce::String(target.session) + "/" + juce::String(target.pass) + "/" + target.swell;
        view.editable = editable;
        if (!editable)
            view.disabledReason = target.defaults ? "Defaults are locked during capture, settlement or Export."
                : !swell ? "Select a swell to change its sound." : swell->frozen ? "Unfreeze this swell to change its response."
                : "Sound editing is available in Perform.";

        const auto* asset = target.defaults ? &state.session.defaultIR : swell ? &swell->ir : nullptr;
        if (asset && asset->present())
        {
            const auto& identity = target.defaults ? state.session.defaultSoundPreset : swell->soundPreset;
            view.accepted = resolvedSoundPreset(identity, *asset).id;
            if (view.accepted == customImpulseId) view.file = irChoiceId(identity, *asset);
        }
        acceptedPreset = view.accepted;
        acceptedFile = view.file;
        view.showFile = customIntent || view.accepted == customImpulseId;
        if (view.showFile) view.browseFrom = customImpulseId;

        choices = active ? engine.irChoices() : std::vector<IRChoice>{};
        const auto root = active ? engine.assetDirectory() : juce::File{};
        peakRevision = peaks->revision();
        if (active && asset && asset->present())
        {
            view.acceptedPeaks = peaks->managed(root, *asset);
            if (!view.acceptedPeaks && !managedRetryAt) managedRetryAt = now() + SoundResponsePeaks::managedRetryMs;
        }
        if (!active || !asset || !asset->present() || view.acceptedPeaks) managedRetryAt.reset();
        for (const auto& choice : choices)
        {
            if (choice.presetId.isNotEmpty())
            {
                view.presets.push_back({choice.presetId, choice.name, peaks->bundled(choice.presetId)});
                if (!view.showFile && view.accepted == choice.presetId) view.description = choice.description;
            }
            else if (view.showFile) view.files.push_back({choice.id, choice.name, {}});
        }
        view.presets.push_back({customImpulseId, "Custom IR", view.file.isNotEmpty() ? view.acceptedPeaks : nullptr});
        if (target.defaults)
            view.values.push_back({{"level", "Level (dB)", minimumDefaultLevelDb, maximumDefaultLevelDb, .1},
                                  state.session.defaultLevelDb, editable});

        uint64_t request = 0;
        juce::String name, error;
        if (target.defaults)
        {
            if (state.defaultIRPending) { request = state.defaultIRRequest; name = state.requestedDefaultIRName; }
            error = state.defaultIRError;
        }
        else if (swell)
        {
            if (const auto entry = state.irRequests.find(swell->id); entry != state.irRequests.end())
            {
                error = entry->second.error;
                if (error.isEmpty()) { request = swell->inputGeneration; name = entry->second.name; }
            }
        }
        if (view.showFile) view.fileError = error;
        else view.presetError = error;
        view.preparing = name.isNotEmpty();
        editor.display(std::move(view));
        if (!active || !editable || name.isEmpty()) pending.reset();
        else if (!pending || pending->serial != request)
            pending = Pending{request, std::move(name), now(), reducedMotion()};
        updateBusy();
    }

    void updateBusy()
    {
        const auto covering = active && pending && editor.isShowing()
            && now() - pending->since >= busyDelayMs;
        editor.showBusy(covering ? "Preparing " + pending->name + juce::String::charToString(0x2026) : juce::String{},
                        covering && pending->reducedMotion);
    }
    void timerCallback() override
    {
        if (!active) return;
        const auto retryDue = managedRetryAt && now() >= *managedRetryAt;
        if (retryDue) managedRetryAt.reset();
        if (retryDue || peakRevision != peaks->revision()) refresh();
        updateBusy();
    }

    Editor& editor;
    SwellEngine& engine;
    std::function<double()> now;
    juce::SharedResourcePointer<SoundResponsePeaks> peaks;
    uint64_t peakRevision = 0;
    std::optional<double> managedRetryAt;
    Target target, levelTarget;
    std::optional<Target> invocation;
    workflow::Mode lastMode = workflow::Mode::capture;
    std::vector<IRChoice> choices;
    juce::String acceptedPreset, acceptedFile;
    bool active = false, editable = false, customIntent = false;
    std::optional<Pending> pending;
    uint64_t fileSerial = 0;
    std::shared_ptr<int> lifetime = std::make_shared<int>(0);
    std::unique_ptr<juce::FileChooser> chooser;
    controls::SurfaceInvocation surface;
};
}
