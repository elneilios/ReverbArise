#pragma once

#include <UI/SoundPresetBinding.h>

namespace m3::ui
{
struct SoundResponsePeaksFixture
{
    static void recovery()
    {
        TestDirectory directory;
        SoundResponsePeaks cache;
        ImpulseResponse response;
        response.rate = 48000;
        response.channels = 1;
        response.samples = {.75f, .25f};
        Asset asset;
        juce::String error;
        REQUIRE(SwellAssetStore(directory.file).writeIR(response, asset, error));
        const auto wav = directory.file.getChildFile(asset.path);
        const auto backup = directory.file.getChildFile("recover.wav");
        REQUIRE(wav.moveFileTo(backup));
        REQUIRE(!cache.managed(directory.file, asset));
        convolutionIrCases::settle([&] { return cache.revision() == 1; });
        REQUIRE(backup.moveFileTo(wav));
        REQUIRE(!cache.managed(directory.file, asset));
        {
            const juce::ScopedLock guard(cache.lock);
            REQUIRE(cache.pending == 0 && cache.cache.size() == 1);
            REQUIRE(cache.cache.begin()->second.retryAt > juce::Time::getMillisecondCounterHiRes());
            cache.cache.begin()->second.retryAt = 0.;
        }
        REQUIRE(!cache.managed(directory.file, asset));
        convolutionIrCases::settle([&] { return cache.revision() == 2; });
        const auto peaks = cache.managed(directory.file, asset);
        REQUIRE(peaks && peaks->at(0)[0] == .75f);
    }

    static void run()
    {
        juce::WaitableEvent gate(true);
        SoundResponsePeaks cache;
        const auto decode = [&](ImpulseResponse& response)
        {
            if (!gate.wait(10000)) return false;
            response.channels = 2;
            response.samples = {.2f, -.1f, .5f, .3f};
            return true;
        };
        for (size_t index = 0; index < cache.pendingLimit + 1; ++index)
            REQUIRE(!cache.get(juce::String(index), decode));
        {
            const juce::ScopedLock guard(cache.lock);
            REQUIRE(cache.pending == cache.pendingLimit && cache.cache.size() == cache.pendingLimit);
        }
        gate.signal();
        convolutionIrCases::settle([&] { return cache.revision() == cache.pendingLimit; });
        const auto first = cache.get("0", decode);
        REQUIRE(first && first->size() == 2 && first->at(0)[1] == .5f && first->at(1)[0] == .1f);
        for (size_t index = cache.pendingLimit; index < cache.cacheLimit; ++index)
        {
            const auto revision = cache.revision();
            cache.get(juce::String(index), decode);
            convolutionIrCases::settle([&] { return cache.revision() > revision; });
        }
        const auto revision = cache.revision();
        cache.get("different", [](ImpulseResponse& response)
        {
            response.channels = 1;
            response.samples = {.8f};
            return true;
        });
        convolutionIrCases::settle([&] { return cache.revision() > revision; });
        const juce::ScopedLock guard(cache.lock);
        REQUIRE(cache.cache.size() == cache.cacheLimit && cache.cache.contains("0") && !cache.cache.contains("1"));
        REQUIRE(cache.cache.at("different").peaks->at(0)[0] == .8f);
    }
};
}

namespace soundPresetCases
{
using namespace m3;
using namespace m3::ui;
using namespace m3::ui::controls;
using convolutionIrCases::settle;

template<class Control>
Control& control(juce::Component& root, const juce::String& name)
{
    auto* result = dynamic_cast<Control*>(convolutionIrCases::findReviewComponent(root, name));
    REQUIRE(result);
    return *result;
}

inline void select(ChoiceSelector& picker, int id)
{
    picker.beforeInvocation();
    picker.setSelectedId(id, juce::sendNotificationSync);
}

inline void componentCases()
{
    WorkspaceTheme theme;
    juce::Component host;
    SoundPresetEditor panel;
    host.setLookAndFeel(&theme);
    host.setBounds(80, 80, 276, 300);
    host.addAndMakeVisible(panel);
    panel.setBounds(host.getLocalBounds());
    host.addToDesktop(0);
    host.setVisible(true);

    SoundPresetEditor::View view;
    const juce::String longName("Stereo Ping Pong Crescendo with an intentionally long accessible name");
    view.presets = {{"ping", longName, {}}, {"metal", "Metal Bloom", {}}, {"custom-ir", "Custom IR", {}}};
    view.acceptedPeaks = std::make_shared<SoundPresetEditor::Peaks>(SoundPresetEditor::Peaks{{1.f, 0.f}});
    view.presets.front().peaks = view.acceptedPeaks;
    view.accepted = "ping";
    view.editable = true;
    view.tempoSync = true;
    view.values = {{{"rate", "Echo Rate", 0, 3, 1, {"1/4", "1/8", "1/8D", "1/16"}}, 1},
                   {{"level", "Level (dB)", -12, 12, .1}, 0}};
    juce::String requested;
    int commits = 0;
    panel.onPreset = [&](const auto& value) { requested = value; ++commits; };
    panel.display(view);
    auto& picker = panel.presetPicker();
    for (const auto* paint : {&picker.closedDecoration, &picker.rowDecoration})
    {
        juce::Image waveform(juce::Image::ARGB, 80, 32, true, juce::SoftwareImageType{});
        {
            juce::Graphics graphics(waveform);
            (*paint)(graphics, waveform.getBounds(), 1);
        }
        REQUIRE(waveform.getPixelAt(20, 16).isTransparent());
        REQUIRE(!waveform.getPixelAt(60, 16).isTransparent());
    }
    REQUIRE(picker.displayedText() == longName && !panel.importAction().isVisible());
    REQUIRE(picker.getAccessibilityHandler()->getValueInterface()->getCurrentValueAsString() == longName);
    auto& previous = control<CommandButton>(panel, "soundPrevious");
    controlReview::pointerClick(previous);
    REQUIRE(requested == "custom-ir" && picker.getSelectedId() == 1);
    if (juce::Process::isForegroundProcess()) REQUIRE(!previous.isFocusVisible());
    picker.keyPressed(juce::KeyPress(juce::KeyPress::downKey));
    auto* popup = picker.activePopup();
    REQUIRE(popup && popup->state().items().front().selected);
    popup->keyPressed(juce::KeyPress(juce::KeyPress::downKey));
    REQUIRE(commits == 1 && picker.getSelectedId() == 1);
    popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(commits == 2 && requested == "metal" && picker.getSelectedId() == 1);

    const auto point = picker.getLocalBounds().getCentre().toFloat();
    const auto time = juce::Time::getCurrentTime();
    juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(), point, {}, 0,0,0,0,0,
                           &picker, &picker, time, point, time, 0, false);
    juce::MouseWheelDetails wheel{};
    wheel.deltaY = 1;
    picker.mouseWheelMove(event, wheel);
    REQUIRE(commits == 2 && picker.getSelectedId() == 1);
    view.accepted = "custom-ir";
    view.showFile = true;
    panel.display(view);
    control<CommandButton>(panel, "soundNext").invokeCommand();
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(requested == "ping" && panel.importAction().isVisible());

    panel.onValue = [&](const auto& id, double value)
    {
        for (auto& parameter : view.values) if (parameter.parameter.id == id) parameter.value = value;
        panel.display(view);
    };
    auto& rate = control<KnobValue>(panel, "rateCell");
    auto* accessible = rate.valueKnob().getAccessibilityHandler()->getValueInterface();
    REQUIRE(accessible && accessible->getCurrentValueAsString() == "1/8");
    static_cast<juce::Component&>(rate.valueKnob()).keyPressed(juce::KeyPress(juce::KeyPress::rightKey));
    REQUIRE(accessible->getCurrentValueAsString() == "1/8D" && rate.valueField().isReadOnly());
    accessible->setValueAsString("1/16");
    REQUIRE(accessible->getCurrentValueAsString() == "1/16");
    accessible->setValueAsString("1/32");
    REQUIRE(accessible->getCurrentValueAsString() == "1/16");
    auto& level = control<KnobValue>(panel, "levelCell").valueField();
    level.setText("2.756", false);
    level.onReturnKey();
    REQUIRE(view.values.back().value == 2.756);
    bool sync = true;
    panel.onSync = [&](bool value) { sync = value; };
    controlReview::pointerClick(control<juce::Button>(panel, "soundTempoSync"));
    REQUIRE(!sync);

    panel.showBusy("Preparing Metal Bloom", true);
    auto& busy = control<juce::Component>(panel, "soundPreparation");
    REQUIRE(busy.isVisible() && busy.getTitle() == "Preparing Metal Bloom" && !picker.isEnabled());
    const auto image = busy.createComponentSnapshot(busy.getLocalBounds());
    juce::MessageManager::getInstance()->runDispatchLoopUntil(40);
    const auto later = busy.createComponentSnapshot(busy.getLocalBounds());
    for (int y = 0; y < image.getHeight(); ++y)
        for (int x = 0; x < image.getWidth(); ++x) REQUIRE(image.getPixelAt(x,y) == later.getPixelAt(x,y));
    panel.showBusy({}, false);
    REQUIRE(!busy.isVisible() && picker.isEnabled());
    host.removeFromDesktop();
    host.setLookAndFeel(nullptr);
}

inline void bindingCases()
{
    convolutionIrCases::Fixture fixture;
    auto& engine = *fixture.engine;
    SoundPresetEditor defaults, selected;
    juce::Component host;
    host.setBounds(80, 80, 552, 300);
    host.addAndMakeVisible(defaults);
    host.addAndMakeVisible(selected);
    defaults.setBounds(0, 0, 276, 300);
    selected.setBounds(276, 0, 276, 300);
    host.addToDesktop(0);
    host.setVisible(true);
    double time = 100.;
    SoundPresetBinding learn(defaults, engine, [&] { return time; }), perform(selected, engine, [&] { return time; });
    const auto refresh = [&]
    {
        const auto state = engine.snapshot();
        learn.displayDefault(state, true);
        perform.displaySelected(state, true);
    };
    learn.onRefresh = refresh;
    perform.onRefresh = refresh;
    fixture.select(fixture.a);
    refresh();
    const auto baseline = engine.savedSession();
    REQUIRE(defaults.presetPicker().getNumItems() == 7 && selected.presetPicker().getNumItems() == 7);
    REQUIRE(!convolutionIrCases::findReviewComponent(defaults, "rateCell"));
    auto& level = control<KnobValue>(defaults, "levelCell").valueField();
    level.setText("2.75", false);
    level.onReturnKey();
    REQUIRE(engine.savedSession().defaultLevelDb == 2.75f && find(engine.savedSession(), fixture.a)->shape == find(baseline, fixture.a)->shape);

    SoundPresetBinding::FileResult finish;
    learn.chooseFile = [&](auto result) { finish = std::move(result); };
    perform.chooseFile = learn.chooseFile;
    const auto start = [&](SoundPresetEditor& panel)
    {
        panel.beforeInvocation();
        panel.onImport();
        REQUIRE(finish);
    };
    start(defaults);
    finish({});
    REQUIRE(engine.savedSession().defaultIR == baseline.defaultIR);
    start(selected);
    auto obsolete = finish;
    fixture.select(fixture.b);
    refresh();
    obsolete(fixture.second);
    REQUIRE(!engine.snapshot().irRequests.contains(fixture.a) && !engine.snapshot().irRequests.contains(fixture.b));

    const auto bundled = bundledImpulses().front();
    const auto same = fixture.root.file.getChildFile("Matching bundled response.wav");
    REQUIRE(same.replaceWithData(bundled.bytes, bundled.size));
    start(defaults);
    finish(same);
    settle([&] { return !engine.snapshot().defaultIRPending; });
    refresh();
    REQUIRE(engine.savedSession().defaultSoundPreset.id == customImpulseId);
    REQUIRE(engine.savedSession().defaultIR.content == bundled.decodedHash);
    REQUIRE(defaults.filePicker().getText() == same.getFileName());
    select(defaults.presetPicker(), 1);
    settle([&] { return !engine.snapshot().defaultIRPending; });
    refresh();
    REQUIRE(engine.savedSession().defaultSoundPreset.id == bundled.id);
    REQUIRE(same.deleteFile());
    select(defaults.presetPicker(), 7);
    auto& files = defaults.filePicker();
    REQUIRE(files.isVisible() && defaults.presetPicker().displayedText().contains("choose file"));
    for (int index = 0; index < files.getNumItems(); ++index)
        if (files.getItemText(index) == same.getFileName()) { select(files, index + 1); break; }
    settle([&] { return !engine.snapshot().defaultIRPending; });
    refresh();
    REQUIRE(engine.savedSession().defaultSoundPreset.id == customImpulseId && files.getText() == same.getFileName());

    const auto accepted = engine.savedSession().defaultIR;
    start(defaults);
    finish(fixture.root.file.getChildFile("Missing.wav"));
    settle([&] { return !engine.snapshot().defaultIRPending; });
    refresh();
    REQUIRE(engine.savedSession().defaultIR == accepted && files.getValidationMessage().isNotEmpty());

    fixture.select(fixture.a);
    refresh();
    auto& opener = selected.importAction();
    auto& otherField = selected.filePicker();
    host.toFront(true);
    opener.grabKeyboardFocus();
    const auto inputBeforeImport = find(engine.savedSession(), fixture.a)->inputGeneration;
    start(selected);
    otherField.grabKeyboardFocus();
    const bool canCheckFocus = juce::Process::isForegroundProcess() && otherField.hasKeyboardFocus(false);
    finish(fixture.second);
    REQUIRE(find(engine.savedSession(), fixture.a)->inputGeneration != inputBeforeImport);
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    if (canCheckFocus) REQUIRE(opener.hasKeyboardFocus(false));
    settle([&]
    {
        const auto session = engine.savedSession();
        const auto* swell = find(session, fixture.a);
        return swell && swell->phase == Phase::ready && swell->irName == fixture.second.getFileName();
    });
    refresh();
    auto& selectedFiles = selected.filePicker();
    for (int index = 0; index < selectedFiles.getNumItems(); ++index)
        if (selectedFiles.getItemText(index) == same.getFileName()) { select(selectedFiles, index + 1); break; }
    settle([&]
    {
        const auto session = engine.savedSession();
        const auto* swell = find(session, fixture.a);
        return swell && swell->phase == Phase::ready && swell->ir == accepted;
    });
    REQUIRE(find(engine.savedSession(), fixture.b)->ir == find(baseline, fixture.b)->ir);
    REQUIRE(engine.savedSession().defaultIR == accepted);

    engine.holdDefaultImportsForTesting(true);
    select(defaults.presetPicker(), 2);
    settle([&] { return engine.defaultImportWaitingForTesting(); });
    refresh();
    REQUIRE(defaults.presetPicker().getText() == "Custom IR" && engine.savedSession().defaultIR == accepted);
    select(defaults.presetPicker(), 6);
    engine.holdDefaultImportsForTesting(false);
    settle([&] { return !engine.snapshot().defaultIRPending; });
    refresh();
    REQUIRE(defaults.presetPicker().getText() == "Whisper");

    fixture.select(fixture.b);
    REQUIRE(engine.setFrozen(fixture.b, true));
    refresh();
    REQUIRE(!selected.presetPicker().isEnabled() && selected.presetPicker().getDescription().contains("Unfreeze"));
    REQUIRE(fixture.flow.requestMode(workflow::Mode::capture, {true,true,true,false}));
    refresh();
    start(defaults);
    auto lockedPicker = finish;
    defaults.onValueBegin();
    const auto pass = fixture.flow.begin(true);
    REQUIRE(pass != 0);
    refresh();
    REQUIRE(!defaults.presetPicker().isEnabled());
    lockedPicker(fixture.second);
    defaults.onValue("level", 7.);
    REQUIRE(!engine.snapshot().defaultIRPending && engine.savedSession().defaultLevelDb == 2.75f);
    REQUIRE(fixture.flow.cancelCapture(pass));
    refresh();
    select(defaults.presetPicker(), 7);
    auto state = engine.snapshot();
    state.defaultIRError.clear();
    state.defaultIRPending = true;
    state.requestedDefaultIRName = "Same name";
    state.defaultIRRequest = 12;
    auto& busy = control<juce::Component>(defaults, "soundPreparation");
    learn.displayDefault(state, true);
    time += 1999.;
    learn.displayDefault(state, true);
    REQUIRE(!busy.isVisible());
    time += 1.;
    learn.displayDefault(state, true);
    REQUIRE(busy.isVisible() && busy.getTitle().contains("Same name"));
    ++state.defaultIRRequest;
    learn.displayDefault(state, true);
    REQUIRE(!busy.isVisible());
    time += 2000.;
    learn.displayDefault(state, true);
    REQUIRE(busy.isVisible());
    learn.setActive(false);
    REQUIRE(!busy.isVisible() && defaults.presetPicker().isEnabled());
    learn.displayDefault(state, true);
    REQUIRE(!busy.isVisible());
    select(defaults.presetPicker(), 7);
    state.defaultIRPending = false;
    state.defaultIRError = "Invalid file";
    learn.displayDefault(state, true);
    REQUIRE(!busy.isVisible() && files.getValidationMessage() == "Invalid file");
    host.removeFromDesktop();
}

inline void run()
{
    componentCases();
    bindingCases();
    SoundResponsePeaksFixture::run();
    SoundResponsePeaksFixture::recovery();
}
}
