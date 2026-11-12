#pragma once

#include <DSP/DefaultImpulse.h>
#include <UI/Controls/SoundPresetEditor.h>

namespace preview46
{
class SoundPresetReview final : public juce::Component, private juce::Timer
{
    using Sound = controls::SoundPresetEditor;
    using Choice = Sound::Choice;
    struct Request { double started, duration; };

    struct FixtureState
    {
        juce::String accepted = "ping-pong", file, error;
        bool showFile = false, sync = true;
        double rate = 1, spread = 85, intensity = 75, level = 2.8;
        std::optional<Request> pending;
    };

public:
    explicit SoundPresetReview(std::unique_ptr<Review> scope)
        : fixture(std::move(scope)), editor(fixture->composition.productionEditor())
    {
        setLookAndFeel(&theme);
        editor->enableResponsiveReview(std::make_shared<TransientPresentationStorage>());
        require<controls::SegmentedSelector>(*editor, "learnSections").onRequest(2);
        require<controls::SegmentedSelector>(*editor, "inspectorSections").onRequest(3);
        require<controls::SegmentedSelector>(*editor, "workflowMode").onRequest(2);

        choices.push_back({"ping-pong", "Stereo Ping Pong Crescendo", waveform(0, true)});
        int seed = 1;
        for (const auto& impulse : m3::bundledImpulses())
            choices.push_back({impulse.id, impulse.name, waveform(seed++, false)});
        choices.push_back({"metal", "Metal Bloom", waveform(seed++, true)});
        choices.push_back({"custom", "Custom IR", {}});
        managed.push_back({"air", "Airy Vocal Reflection.wav", waveform(seed++, false)});
        managed.push_back({"long", "My long stereo impulse response recorded in the stone hall.wav", waveform(seed, true)});

        addAndMakeVisible(*editor);
        addAndMakeVisible(scenes);
        addAndMakeVisible(sizes);
        addAndMakeVisible(motion);
        addAndMakeVisible(note);
        scenes.setTitle("Preview scene");
        scenes.addItemList({"Ping Pong", "Custom IR", "Invalid import", "Frozen", "Prepare (5 s)", "Prepare (short)"}, 1);
        scenes.setSelectedId(1, juce::dontSendNotification);
        scenes.onChange = [this] { scene(scenes.getSelectedId()); };
        sizes.setTitle("Preview size and scale");
        sizes.addItemList({"900 x 600 / 100%", "1000 x 700 / 100%", "1000 x 700 / 125%"}, 1);
        sizes.setSelectedId(1, juce::dontSendNotification);
        sizes.onChange = [this] { selectSize(sizes.getSelectedId()); };
        applyTextRole(note, TextRole::helper);
        note.setText("Native Sound preview - fixture responses", juce::dontSendNotification);
        motion.onClick = [this] { refresh(); };
        wire(learn, states[0]);
        wire(perform, states[1]);
        setSize(editor->getWidth(), editor->getHeight() + stripHeight);
        refresh();
        startTimerHz(20);
    }

    ~SoundPresetReview() override
    {
        stopTimer();
        setLookAndFeel(nullptr);
    }
    void paint(juce::Graphics& g) override { g.fillAll(background); }
    void capture(const juce::File& directory)
    {
        stopTimer();
        setVisible(true);
        REQUIRE(directory.createDirectory().wasOk());
        const auto write = [&](const juce::String& name)
        {
            mount();
            writeImage(directory.getChildFile(name + ".png"), editor->createComponentSnapshot(editor->getLocalBounds()));
        };
        require<controls::SegmentedSelector>(*editor, "workflowMode").onRequest(2);
        write("perform-900");
        captureMenu(directory);
        scene(2);
        write("perform-custom-900");
        scene(1);
        perform.onPreset("custom");
        write("perform-custom-choose-file-900");
        perform.onPreset("metal");
        write("perform-metal-900");
        scene(3);
        write("perform-invalid-900");
        scene(1);
        perform.showBusy(preparingCaption(), true);
        write("perform-preparing-900");
        perform.showBusy({}, false);
        selectSize(2);
        write("perform-expanded-100");
        selectSize(3);
        write("perform-expanded-125");
        selectSize(1);
        require<controls::SegmentedSelector>(*editor, "workflowMode").onRequest(1);
        mount();
        write("learn-900");
    }
    void resized() override
    {
        auto area = getLocalBounds();
        auto strip = area.removeFromTop(stripHeight).reduced(spacing::related, spacing::tight);
        scenes.setRowBounds(strip.removeFromLeft(158));
        strip.removeFromLeft(spacing::related);
        sizes.setRowBounds(strip.removeFromLeft(172));
        strip.removeFromLeft(spacing::related);
        motion.setBounds(strip.removeFromLeft(150));
        note.setBounds(strip.withTrimmedLeft(spacing::related));
        editor->setTopLeftPosition(area.getPosition());
        mount();
    }

private:
    static constexpr int stripHeight = 48;
    static constexpr double busyDelay = 2000., slowDuration = 5000., shortDuration = 550.;
    static juce::String preparingCaption() { return "Preparing Metal Bloom" + juce::String::charToString(0x2026); }

    void captureMenu(const juce::File& directory)
    {
        auto& source = require<controls::ChoiceSelector>(perform, "soundPreset");
        std::vector<controls::PopupChoice> rows;
        for (const auto& choice : choices)
            rows.push_back({static_cast<int>(rows.size()) + 1, choice.name, {}, true, choice.id == states[1].accepted});
        controls::ChoicePopup menu(source, std::move(rows), {}, source.rowDecoration);
        menu.setSize(248, 330);
        menu.inspect(1);
        writeImage(directory.getChildFile("preset-menu.png"), menu.createComponentSnapshot(menu.getLocalBounds()));
    }
    static void writeImage(const juce::File& file, const juce::Image& image)
    {
        juce::MemoryOutputStream bytes;
        REQUIRE(juce::PNGImageFormat().writeImageToStream(image, bytes));
        REQUIRE(file.replaceWithData(bytes.getData(), bytes.getDataSize()));
    }
    void selectSize(int size)
    {
        editor->setPresentationScale(100);
        editor->setPresentationSize(size == 1 ? 900 : 1000, size == 1 ? 600 : 700);
        if (size == 3) editor->setPresentationScale(125);
        setSize(editor->getWidth(), editor->getHeight() + stripHeight);
        mount();
    }
    static std::shared_ptr<const Sound::Peaks> waveform(int seed, bool stereo)
    {
        auto result = std::make_shared<Sound::Peaks>();
        for (int channel = 0; channel < (stereo ? 2 : 1); ++channel)
        {
            auto& bins = result->emplace_back();
            for (int i = 0; i < 104; ++i)
            {
                const auto time = i / 103.;
                const auto pulse = std::pow(std::abs(std::sin(time * (12. + seed * 2.) + channel * 1.6)), 7.);
                const auto texture = .35 + .65 * std::abs(std::sin(i * (2.1 + .075 * seed) + channel));
                const auto envelope = seed == 0 ? .12 + time * .85 : std::exp(-time * (1. + seed * .22));
                bins.push_back(static_cast<float>((.12 + .88 * pulse) * texture * envelope));
            }
        }
        return result;
    }
    void wire(Sound& panel, FixtureState& state)
    {
        panel.onPreset = [this, &state](const juce::String& id)
        {
            state.pending.reset();
            state.error.clear();
            state.showFile = id == "custom";
            if (id == "custom" && state.file.isNotEmpty()) state.accepted = id;
            else if (id != "custom") state.accepted = id;
            refresh();
        };
        panel.onFile = [this, &state](const juce::String& id)
        {
            state.file = id;
            state.accepted = "custom";
            state.error.clear();
            refresh();
        };
        panel.onImport = [this, &state]
        {
            state.error = "Couldn't read this IR. Choose another WAV or WavPack file.";
            refresh();
        };
        panel.onSync = [this, &state](bool value) { state.sync = value; refresh(); };
        panel.onValue = [this, &state](const juce::String& id, double value)
        {
            if (id == "rate") state.rate = value;
            if (id == "spread") state.spread = value;
            if (id == "intensity") state.intensity = value;
            if (id == "level") state.level = value;
            refresh();
        };
    }
    void mount()
    {
        auto& original = require<juce::Component>(*editor, "learnIrEditor");
        auto& oldSound = require<juce::Component>(*editor, "selectedIrEditor");
        const bool showLearn = visibleInEditor(original);
        const bool showPerform = visibleInEditor(oldSound);
        if (learn.getParentComponent() != original.getParentComponent()) original.getParentComponent()->addChildComponent(learn);
        if (perform.getParentComponent() != oldSound.getParentComponent()) oldSound.getParentComponent()->addChildComponent(perform);
        if (showLearn)
        {
            learn.setBounds(original.getBounds());
            if (!learn.isVisible()) learn.toFront(false);
        }
        if (showPerform)
        {
            perform.setBounds(oldSound.getBounds());
            if (!perform.isVisible()) perform.toFront(false);
        }
        if (!showLearn && learn.isVisible()) { states[0].pending.reset(); learn.showBusy({}, false); }
        if (!showPerform && perform.isVisible()) { states[1].pending.reset(); perform.showBusy({}, false); }
        learn.setVisible(showLearn);
        perform.setVisible(showPerform);
    }
    bool visibleInEditor(juce::Component& component) const
    {
        for (auto* parent = &component; parent != nullptr; parent = parent->getParentComponent())
        {
            if (!parent->isVisible()) return false;
            if (parent == editor.get()) return true;
        }
        return false;
    }
    Sound::View view(const FixtureState& state, bool forLearn) const
    {
        Sound::View result;
        result.owner = forLearn ? "learn" : "perform";
        result.presets = choices;
        result.files = managed;
        result.accepted = state.accepted;
        result.file = state.file;
        result.fileError = state.error;
        result.showFile = state.showFile || state.accepted == "custom";
        if (result.showFile) result.browseFrom = "custom";
        const auto frozen = !forLearn && isFrozen();
        result.editable = !frozen;
        result.disabledReason = frozen ? "Unfreeze this swell to change its response." : juce::String{};
        const auto* custom = byId(managed, state.file);
        if (custom) result.presets.back().peaks = custom->peaks;
        if (const auto* accepted = byId(result.presets, state.accepted)) result.acceptedPeaks = accepted->peaks;
        const auto displayedPreset = result.showFile ? juce::String("custom") : state.accepted;
        if (displayedPreset == "ping-pong")
        {
            result.description = "Stereo echoes build towards the swell target.";
            result.tempoSync = state.sync;
            result.values = {
                {{"rate", "Echo Rate", 0, 3, 1, {"1/4", "1/8", "1/8D", "1/16"}}, state.rate, !frozen},
                {{"spread", "Stereo Spread", 0, 100, 1, {}, "%"}, state.spread, !frozen},
                {{"intensity", "Build Intensity", 0, 100, 1, {}, "%"}, state.intensity, !frozen}};
        }
        else if (displayedPreset == "metal")
        {
            result.description = "Metallic resonances grow into a shimmer.";
            result.values = {
                {{"spread", "Brightness", 0, 100, 1, {}, "%"}, state.spread, !frozen},
                {{"intensity", "Build Intensity", 0, 100, 1, {}, "%"}, state.intensity, !frozen}};
        }
        else
            for (const auto& impulse : m3::bundledImpulses())
                if (displayedPreset == impulse.id) result.description = impulse.description;
        result.values.push_back({{"level", "Level (dB)", -12, 12, .1}, state.level});
        return result;
    }
    static const Choice* byId(const std::vector<Choice>& options, const juce::String& key)
    {
        const auto it = std::find_if(options.begin(), options.end(), [&](const auto& choice) { return choice.id == key; });
        return it == options.end() ? nullptr : &*it;
    }
    bool isFrozen() const { return require<juce::Button>(*editor, "freezeRender").getToggleState(); }
    void refresh()
    {
        learn.display(view(states[0], true));
        perform.display(view(states[1], false));
        displayBusy(learn, states[0]);
        displayBusy(perform, states[1]);
    }
    void displayBusy(Sound& panel, const FixtureState& state)
    {
        const auto elapsed = state.pending ? juce::Time::getMillisecondCounterHiRes() - state.pending->started : 0.;
        const bool cover = panel.isShowing() && state.pending && elapsed >= busyDelay && elapsed < state.pending->duration;
        panel.showBusy(cover ? preparingCaption() : juce::String{}, motion.getToggleState());
    }
    static void startRequest(FixtureState& state, double duration)
    {
        state.pending = Request{juce::Time::getMillisecondCounterHiRes(), duration};
        state.error.clear();
    }
    void timerCallback() override
    {
        mount();
        if (frozen != isFrozen()) { frozen = isFrozen(); refresh(); }
        for (size_t i = 0; i < states.size(); ++i)
        {
            auto& state = states[i];
            auto& panel = i == 0 ? learn : perform;
            if (state.pending && juce::Time::getMillisecondCounterHiRes() - state.pending->started >= state.pending->duration)
            {
                state.accepted = "metal";
                state.pending.reset();
                refresh();
            }
            displayBusy(panel, state);
        }
    }
    void scene(int id)
    {
        if (id == 4 && !perform.isVisible()) require<controls::SegmentedSelector>(*editor, "workflowMode").onRequest(2);
        auto& state = learn.isVisible() && id != 4 ? states[0] : states[1];
        state = {};
        if (id == 2 || id == 3)
        {
            state.showFile = true;
            state.file = id == 2 ? "air" : "";
            if (id == 2) state.accepted = "custom";
            else state.error = "Couldn't read this IR. Choose another WAV or WavPack file.";
        }
        if (id == 5 || id == 6) startRequest(state, id == 5 ? slowDuration : shortDuration);
        if ((id == 4) != isFrozen() && perform.isVisible())
            require<juce::Button>(*editor, "freezeRender").triggerClick();
        refresh();
    }
    WorkspaceTheme theme;
    std::unique_ptr<Review> fixture;
    std::unique_ptr<PreverbAudioProcessorEditor> editor;
    Sound learn, perform;
    std::array<FixtureState, 2> states;
    std::vector<Choice> choices, managed;
    controls::ChoiceSelector scenes, sizes;
    controls::Switch motion{"Reduced motion"};
    juce::Label note;
    bool frozen = false;
};

inline void reviewSoundPresets(std::unique_ptr<Review> fixture)
{
    class Window final : public juce::DocumentWindow
    {
    public:
        explicit Window(std::unique_ptr<Review> scope)
            : DocumentWindow("ReverbArise - Sound Preset native review", background, closeButton)
        {
            setUsingNativeTitleBar(true);
            auto review = std::make_unique<SoundPresetReview>(std::move(scope));
            const auto width = review->getWidth(), height = review->getHeight();
            setContentOwned(review.release(), true);
            centreWithSize(width, height);
            setVisible(true);
        }
        void closeButtonPressed() override { juce::MessageManager::getInstance()->stopDispatchLoop(); }
    };
    Window window(std::move(fixture));
    juce::MessageManager::getInstance()->runDispatchLoop();
}
}
