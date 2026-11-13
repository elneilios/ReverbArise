#pragma once

#include "ChoiceSelector.h"
#include "EllipsisLabel.h"
#include "FocusViewport.h"
#include "IconActionButton.h"
#include "Switch.h"
#include <UI/KnobValue.h>
#include <UI/PerformText.h>
#include <algorithm>
#include <charconv>

namespace m3::ui::controls
{
// The hosts supply accepted state and handle edit authority. Peak summaries
// are immutable presentation data, independent of the assigned audio.
class SoundPresetEditor final : public juce::Component
{
    using Rect = juce::Rectangle<int>;
public:
    using Peaks = std::vector<std::vector<float>>;
    struct Choice
    {
        juce::String id, name;
        std::shared_ptr<const Peaks> peaks;
    };
    struct Parameter
    {
        juce::String id, label;
        double minimum = 0, maximum = 1, step = .01;
        juce::StringArray steps;
        juce::String unit;
        juce::StringArray accessibleSteps;
        juce::String help;
        std::optional<double> initialValue;
        bool cutoff = false, offAtTop = false;
        bool operator==(const Parameter&) const = default;
    };
    struct Value
    {
        Parameter parameter;
        double value = 0;
        bool editable = true;
    };
    struct View
    {
        std::vector<Choice> presets, files;
        juce::String owner, accepted, browseFrom, file, description, disabledReason, fileError, presetError;
        std::vector<Value> values;
        std::optional<bool> tempoSync;
        bool editable = false, showFile = false, preparing = false;
        std::shared_ptr<const Peaks> acceptedPeaks;
    };

    SoundPresetEditor()
    {
        setName("soundPresetEditor");
        addAndMakeVisible(viewport);
        viewport.setViewedComponent(&content, false);
        viewport.setScrollBarsShown(true, false);
        viewport.setFocusRevealEnabled(true);
        for (auto* child : std::array<juce::Component*, 11>{&presetLabel, &previous, &presets, &next,
                 &fileLabel, &files, &import, &fileFeedback, &presetFeedback, &sync, &description})
            content.addAndMakeVisible(child);
        setupLabel(presetLabel, "Sound Preset");
        setupLabel(fileLabel, "IR File");
        presets.setName("soundPreset");
        files.setName("soundIrFile");
        previous.setName("soundPrevious");
        next.setName("soundNext");
        import.setName("soundImport");
        sync.setName("soundTempoSync");
        presets.setTitle("Sound Preset");
        files.setTitle("IR File");
        presets.setTextWhenNothingSelected("Unavailable");
        files.setTextWhenNothingSelected("Choose an IR file");
        bindSelector(presets, presetRows, &View::presets, &View::accepted, &SoundPresetEditor::onPreset);
        bindSelector(files, fileRows, &View::files, &View::file, &SoundPresetEditor::onFile);
        presets.onReselect = [this]
        {
            if ((view.preparing || (view.showFile && view.browseFrom != view.accepted)) && onPreset) onPreset(view.accepted);
        };
        files.onReselect = [this] { if (view.preparing && onFile) onFile(view.file); };
        previous.beforeInvocation = [this] { prepareCycle(-1, previousChoice); };
        next.beforeInvocation = [this] { prepareCycle(1, nextChoice); };
        previous.onClick = [this] { commitCycle(previousChoice); };
        next.onClick = [this] { commitCycle(nextChoice); };
        import.beforeInvocation = [this] { if (beforeInvocation) beforeInvocation(); };
        import.onClick = [this] { if (import.isEnabled() && onImport) onImport(); };
        sync.onClick = [this] { if (sync.isEnabled() && onSync) onSync(sync.getToggleState()); };
        presets.closedDecoration = [this](auto& g, auto area, int)
        { if (view.acceptedPeaks) paintPeaks(g, area, *view.acceptedPeaks); };
        presets.rowDecoration = [this](auto& g, auto area, int row)
        {
            if (row > 0 && static_cast<size_t>(row) <= view.presets.size())
                if (const auto& peaks = view.presets[static_cast<size_t>(row - 1)].peaks)
                    paintPeaks(g, area, *peaks, true);
        };
        addChildComponent(busy);
        setOpaque(true);
    }

    ~SoundPresetEditor() override
    {
        presets.hidePopup();
        files.hidePopup();
        viewport.setViewedComponent(nullptr, false);
    }

    std::function<void()> beforeInvocation, onImport;
    std::function<void(const juce::String&)> onPreset, onFile;
    std::function<void(const juce::String&, double)> onValue;
    std::function<std::function<void()>(const juce::String&, double)> onPrepareValueReset;
    std::function<void()> onValueBegin, onValueEnd;
    std::function<void()> onValueCancel;
    std::function<void(bool)> onSync;
    juce::Component& importAction() noexcept { return import; }
    ChoiceSelector& presetPicker() noexcept { return presets; }
    ChoiceSelector& filePicker() noexcept { return files; }
    juce::Label& pickerLabel() noexcept { return presetLabel; }
    bool hasResponsePreview() const noexcept { return view.acceptedPeaks != nullptr; }
    void previewValue(const juce::String& id, double value)
    {
        if (auto entry = std::find_if(view.values.begin(), view.values.end(),
                [&](const auto& item) { return item.parameter.id == id; }); entry != view.values.end())
        {
            entry->value = value;
            updateValues();
        }
    }
    void useLevelControl(KnobValue& level)
    {
        externalLevel = &level;
        content.addAndMakeVisible(level);
    }
    int preferredHeight(int width) { return layout(width); }
    void cancelInvocations()
    {
        presets.hidePopup();
        files.hidePopup();
        presetRows.clear();
        fileRows.clear();
        previousChoice.reset();
        nextChoice.reset();
        restoreFocus = nullptr;
        showBusy({}, false);
    }

    void display(View nextView)
    {
        const bool contextChanged = view.owner != nextView.owner;
        if (contextChanged)
        {
            presets.hidePopup();
            files.hidePopup();
            restoreFocus = nullptr;
            showBusy({}, false);
        }
        updateRows(presets, view.presets, nextView.presets);
        updateRows(files, view.files, nextView.files);
        view = std::move(nextView);
        select(presets, view.presets, view.accepted);
        select(files, view.files, view.file);
        const bool choosingFile = view.showFile && view.browseFrom.isNotEmpty() && view.browseFrom != view.accepted;
        const auto custom = std::find_if(view.presets.begin(), view.presets.end(), [&](const auto& choice) { return choice.id == view.browseFrom; });
        presets.setClosedText(choosingFile && custom != view.presets.end() ? custom->name + " (choose file)" : juce::String{});
        presets.repaint();
        if (auto* popup = presets.activePopup()) popup->repaint();
        for (auto* button : {&previous, &next})
            setHelp(*button, view.editable && view.presets.size() > 1, button->getButtonText());
        setHelp(presets, view.editable && !view.presets.empty(), "Choose a sound preset");
        presets.setValidationMessage(view.editable ? view.presetError : juce::String{});
        setHelp(files, view.editable && !view.files.empty(), "Choose a retained IR file");
        setHelp(import, view.editable, "Import WAV or WavPack IR");
        if (view.editable && view.showFile && view.fileError.isNotEmpty())
        {
            files.setTooltip(view.fileError);
            import.setTooltip(view.fileError);
            import.setDescription(view.fileError);
            import.setHelpText(view.fileError);
        }
        files.setValidationMessage(view.editable ? view.fileError : juce::String{});
        for (auto* child : std::array<juce::Component*, 4>{&fileLabel, &files, &import, &fileFeedback})
            child->setVisible(view.showFile);
        sync.setVisible(view.tempoSync.has_value());
        sync.setToggleState(view.tempoSync.value_or(false), juce::dontSendNotification);
        setHelp(sync, view.editable, "Tempo Sync");
        sync.setUnavailableReason(view.editable ? juce::String{} : view.disabledReason);
        updateValues();
        resized();
    }

    // The binding supplies a caption only after its request crosses the delay.
    void showBusy(const juce::String& caption, bool reducedMotion)
    {
        const bool covering = caption.isNotEmpty();
        const bool wasCovered = busy.isVisible();
        if (covering && !wasCovered)
        {
            auto* focused = getCurrentlyFocusedComponent();
            if (presets.isPopupActive()) focused = &presets;
            if (files.isPopupActive()) focused = &files;
            if (focused && viewport.isParentOf(focused)) restoreFocus = focused;
            presets.hidePopup();
            files.hidePopup();
            for (auto& dial : dials) if (dial.control->editing()) dial.control->cancel();
            if (externalLevel && externalLevel->editing()) externalLevel->cancel();
            viewport.setEnabled(false);
            viewport.setAccessible(false);
            busy.setVisible(true);
            if (restoreFocus && isShowing()) busy.grabKeyboardFocus();
        }
        busy.display(caption, reducedMotion);
        if (!covering && wasCovered)
        {
            const bool focusOwned = busy.hasKeyboardFocus(true);
            busy.setVisible(false);
            viewport.setEnabled(true);
            viewport.setAccessible(true);
            if (focusOwned && restoreFocus && restoreFocus->isShowing() && restoreFocus->isEnabled())
                restoreFocus->grabKeyboardFocus();
            restoreFocus = nullptr;
        }
    }

    void paint(juce::Graphics& g) override { g.fillAll(background); }

    void resized() override
    {
        viewport.setBounds(getLocalBounds());
        busy.setBounds(getLocalBounds());
        auto height = layout(getWidth());
        if (height > getHeight()) height = layout(std::max(0, getWidth() - viewport.getScrollBarThickness()));
        content.setSize(height > getHeight() ? std::max(0, getWidth() - viewport.getScrollBarThickness()) : getWidth(),
                        std::max(height, getHeight()));
    }

private:
    class BusyPanel final : public juce::Component, private juce::Timer
    {
    public:
        BusyPanel()
        {
            setName("soundPreparation");
            setWantsKeyboardFocus(true);
            setMouseClickGrabsKeyboardFocus(false);
            addAndMakeVisible(label);
            applyTextRole(label, TextRole::controlLabel, juce::Justification::centred);
        }
        void display(const juce::String& caption, bool reduced)
        {
            const bool changed = label.getText() != caption;
            label.setText(caption, juce::dontSendNotification);
            setTitle(caption);
            if (caption.isNotEmpty() && !reduced) { if (!isTimerRunning()) startTimerHz(30); }
            else stopTimer();
            if (changed && caption.isNotEmpty() && isShowing())
                juce::AccessibilityHandler::postAnnouncement(caption, juce::AccessibilityHandler::AnnouncementPriority::medium);
            repaint();
        }
        void resized() override
        {
            label.setBounds(getLocalBounds().reduced(spacing::related, 0)
                .withTrimmedTop(getHeight() / 2 + spacing::related).withHeight(controlGeometry::labelHeight));
        }
        void paint(juce::Graphics& g) override
        {
            constexpr float tintOpacity = .84f;
            g.fillAll(background.withAlpha(tintOpacity));
            g.setColour(aqua);
            constexpr std::array<float, 7> heights{8, 13, 10, 23, 17, 32, 20};
            const auto centre = getLocalBounds().toFloat().getCentre().translated(0, -spacing::related);
            const auto phase = isTimerRunning() ? juce::Time::getMillisecondCounterHiRes() / 180. : 0.;
            for (size_t i = 0; i < heights.size(); ++i)
            {
                const float height = heights[i] * (isTimerRunning() ? .65f + .35f * static_cast<float>(std::sin(phase + i)) : 1.f);
                g.fillRoundedRectangle(centre.x + (static_cast<float>(i) - 3.f) * spacing::related - rule,
                                       centre.y - height * .5f, 2 * rule, height, rule);
            }
        }
    private:
        void timerCallback() override { repaint(); }
        juce::Label label;
    };

    struct Dial
    {
        Parameter definition;
        std::unique_ptr<KnobValue> control;
    };

    using Rows = std::vector<juce::String>;
    static void setupLabel(juce::Label& label, const juce::String& name)
    {
        label.setText(name, juce::dontSendNotification);
        label.setBorderSize({});
        applyTextRole(label, TextRole::controlLabel);
    }
    void bindSelector(ChoiceSelector& selector, Rows& rows, std::vector<Choice> View::* choices,
                      juce::String View::* accepted, std::function<void(const juce::String&)> SoundPresetEditor::* action)
    {
        selector.beforeInvocation = [this, &rows, choices]
        {
            rows.clear();
            for (const auto& choice : view.*choices) rows.push_back(choice.id);
            if (beforeInvocation) beforeInvocation();
        };
        selector.onChange = [this, &selector, &rows, choices, accepted, action]
        {
            const auto row = selector.getSelectedId();
            const auto captured = std::exchange(rows, {});
            select(selector, view.*choices, view.*accepted);
            if (selector.isEnabled() && row > 0 && static_cast<size_t>(row) <= captured.size() && (this->*action))
                (this->*action)(captured[static_cast<size_t>(row - 1)]);
        };
    }
    static void select(ChoiceSelector& selector, const std::vector<Choice>& choices, const juce::String& key)
    {
        const auto it = std::find_if(choices.begin(), choices.end(), [&](const auto& choice) { return choice.id == key; });
        const auto row = it == choices.end() ? 0 : static_cast<int>(it - choices.begin()) + 1;
        selector.setSelectedId(row, juce::dontSendNotification);
    }
    static void updateRows(ChoiceSelector& selector, const std::vector<Choice>& old, const std::vector<Choice>& choices)
    {
        if (std::ranges::equal(old, choices, [](const auto& a, const auto& b) { return a.id == b.id && a.name == b.name; })) return;
        selector.clear(juce::dontSendNotification);
        int row = 0;
        for (const auto& choice : choices) selector.addItem(choice.name, ++row);
    }
    template<class Control>
    void setHelp(Control& control, bool enabled, const juce::String& label)
    {
        const auto help = !enabled && view.disabledReason.isNotEmpty() ? view.disabledReason : label;
        control.setEnabled(enabled);
        control.setTooltip(help);
        control.setDescription(help);
        if constexpr (std::is_base_of_v<CommandButton, Control>) control.setHelpText(help);
    }
    void prepareCycle(int direction, std::optional<juce::String>& choice)
    {
        choice.reset();
        const int count = static_cast<int>(view.presets.size());
        if (!view.editable || count < 2 || !onPreset) return;
        const auto from = view.browseFrom.isEmpty() ? view.accepted : view.browseFrom;
        const auto position = std::find_if(view.presets.begin(), view.presets.end(), [&](const auto& choice) { return choice.id == from; });
        const int row = position != view.presets.end()
                      ? (static_cast<int>(position - view.presets.begin()) + direction + count) % count
                      : direction > 0 ? 0 : count - 1;
        choice = view.presets[static_cast<size_t>(row)].id;
        if (beforeInvocation) beforeInvocation();
    }
    void commitCycle(std::optional<juce::String>& choice)
    {
        const auto key = std::exchange(choice, std::nullopt);
        if (key && view.editable && onPreset) onPreset(*key);
    }
    static void paintPeaks(juce::Graphics& g, Rect area, const Peaks& channels, bool emphasise = false)
    {
        if (channels.empty() || area.isEmpty()) return;
        // Scale both channels against the same peak.
        float peakScale = 0.f;
        for (const auto& peaks : channels)
            for (const auto peak : peaks) peakScale = std::max(peakScale, peak);
        if (peakScale <= 0.f) return;
        juce::Graphics::ScopedSaveState state(g);
        g.reduceClipRegion(area);
        if (!emphasise) area = area.reduced(0, spacing::tight);
        const float band = static_cast<float>(area.getHeight()) / static_cast<float>(channels.size());
        constexpr float waveformOpacity = .33f;
        g.setColour(aqua.withAlpha(waveformOpacity));
        for (size_t channel = 0; channel < channels.size(); ++channel)
        {
            const auto& peaks = channels[channel];
            if (peaks.empty()) continue;
            const float middle = area.getY() + (static_cast<float>(channel) + .5f) * band;
            const float width = static_cast<float>(area.getWidth()) / static_cast<float>(peaks.size());
            for (size_t i = 0; i < peaks.size(); ++i)
            {
                // Display the response in the time direction heard in a reverse swell.
                const float peak = std::clamp(peaks[peaks.size() - 1 - i] / peakScale, 0.f, 1.f);
                const float amplitude = (emphasise ? std::sqrt(peak) * .48f : peak * .34f) * band;
                g.fillRect(area.getX() + static_cast<float>(i) * width, middle - amplitude, width, 2 * amplitude);
            }
        }
    }

    static juce::String displayText(const Parameter& p, double value)
    {
        if (p.cutoff)
            return value == 0 ? "Off" : value < 1000 ? juce::String(juce::roundToInt(value)) + " Hz"
                : compactValue(value / 1000, 5) + " kHz";
        return p.steps.isEmpty() ? compactValue(value, 4) + p.unit
                                 : p.steps[juce::roundToInt(value)];
    }
    static constexpr double cutoffTravel = .985;
    static double fraction(const Parameter& p, double value)
    {
        if (!p.cutoff) return (value - p.minimum) / (p.maximum - p.minimum);
        if (value == 0) return p.offAtTop ? 1. : 0.;
        const auto active = std::log(value / 20.) / std::log(1000.);
        return (p.offAtTop ? 0. : 1. - cutoffTravel) + active * cutoffTravel;
    }
    static double cutoffAt(const Parameter& p, double position)
    {
        if (p.offAtTop ? position > cutoffTravel : position < 1. - cutoffTravel) return 0.;
        const auto active = std::clamp((position - (p.offAtTop ? 0. : 1. - cutoffTravel)) / cutoffTravel, 0., 1.);
        return std::round(20. * std::pow(1000., active));
    }
    void updateValues()
    {
        if (!std::ranges::equal(dials, view.values, [](const auto& dial, const auto& value) { return dial.definition == value.parameter; }))
        {
            dials.clear();
            for (const auto& value : view.values) addDial(value.parameter);
        }
        for (size_t i = 0; i < dials.size(); ++i)
        {
            auto& cell = *dials[i].control;
            const auto& item = view.values[i];
            const auto& p = item.parameter;
            const auto shown = displayText(p, item.value);
            const auto span = p.maximum - p.minimum;
            cell.displayDomain({item.value, p.minimum, p.maximum, p.step, span, shown});
            const auto exact = p.cutoff && item.value == 0 ? juce::String("Off") : p.steps.isEmpty() ? preciseValue(item.value)
                : p.accessibleSteps.isEmpty() ? shown : p.accessibleSteps[juce::roundToInt(item.value)];
            cell.display(view.owner + "/" + view.accepted, shown, span > 0 ? fraction(p, item.value) : 0.,
                         item.editable, exact);
            const auto help = item.editable ? (p.help.isEmpty() ? p.label : p.help) : view.disabledReason;
            cell.setValueHelp(help);
            cell.setValueDescription(help);
            cell.setHelpText(help);
        }
    }
    const Value* current(const juce::String& id) const
    {
        const auto it = std::find_if(view.values.begin(), view.values.end(), [&](const auto& value) { return value.parameter.id == id; });
        return it == view.values.end() ? nullptr : &*it;
    }
    void submit(const juce::String& id, double value)
    {
        if (const auto* item = current(id); item && item->editable && onValue && std::isfinite(value) && !busy.isVisible())
        {
            const auto& p = item->parameter;
            if (!p.steps.isEmpty()) value = std::round(value);
            onValue(id, std::clamp(value, p.minimum, p.maximum));
        }
    }
    void addDial(const Parameter& definition)
    {
        auto cell = std::make_unique<KnobValue>();
        const auto id = definition.id;
        cell->configure(definition.label, id, definition.label);
        cell->setCompactPresentation(true);
        cell->setSteppedReadout(!definition.steps.isEmpty());
        if (definition.cutoff) cell->valueField().setNumericCharacters("0123456789.oOfF");
        const auto anchor = std::make_shared<double>();
        const auto capture = [this, id, anchor]
        {
            if (const auto* value = current(id)) *anchor = value->value;
        };
        cell->onBegin = [this, capture] { capture(); if (onValueBegin) onValueBegin(); };
        cell->onGestureCommitted = [this] { if (onValueEnd) onValueEnd(); };
        cell->onEnd = [this] { if (onValueCancel) onValueCancel(); };
        cell->onRebase = capture;
        cell->onDelta = [this, id, anchor](double fraction)
        {
            if (const auto* value = current(id))
            {
                const auto& p = value->parameter;
                submit(id, p.cutoff ? cutoffAt(p, SoundPresetEditor::fraction(p, *anchor) + fraction)
                                    : *anchor + fraction * (p.maximum - p.minimum));
            }
        };
        cell->onAbsolute = [this, id](double value) { submit(id, value); };
        cell->onStep = [this, id](double delta, bool fine)
        {
            if (const auto* value = current(id))
            {
                if (value->parameter.cutoff)
                {
                    const auto& p = value->parameter;
                    if (value->value == 0 && (p.offAtTop ? delta < 0 : delta > 0))
                        submit(id, p.offAtTop ? p.maximum : 20.);
                    else
                        submit(id, cutoffAt(p, fraction(p, value->value) + delta * (fine ? .004 : .02)));
                    return;
                }
                const auto change = value->parameter.steps.isEmpty() ? delta * value->parameter.step * (fine ? .1 : 1.)
                                                                     : std::copysign(1., delta);
                submit(id, value->value + change);
            }
        };
        cell->onCommit = [this, id](const juce::String& input)
        {
            const auto* value = current(id);
            if (!value) return false;
            if (value->parameter.cutoff && input.trim().equalsIgnoreCase("Off"))
            {
                submit(id, 0);
                return true;
            }
            if (!value->parameter.steps.isEmpty())
            {
                const auto index = value->parameter.steps.indexOf(input.trim());
                if (index < 0) return false;
                submit(id, index);
                return true;
            }
            const auto number = input.trim().toStdString();
            const char* first = number.data();
            const auto* last = first + number.size();
            if (first != last && *first == '+') ++first;
            double parsed = 0;
            const auto result = std::from_chars(first, last, parsed);
            const auto valid = result.ec == std::errc{} && result.ptr == last && std::isfinite(parsed)
                            && parsed >= value->parameter.minimum && parsed <= value->parameter.maximum
                            && (!value->parameter.cutoff || parsed == 0 || parsed >= 20);
            if (valid) submit(id, parsed);
            else if (const auto it = std::find_if(dials.begin(), dials.end(), [&](const auto& dial) { return dial.definition.id == id; }); it != dials.end())
                it->control->setValidation(value->parameter.cutoff ? "Enter Off or a frequency from 20 to 20000 Hz."
                    : "Enter a value from " + displayText(value->parameter, value->parameter.minimum)
                        + " to " + displayText(value->parameter, value->parameter.maximum) + ".");
            return valid;
        };
        cell->onFeedbackChanged = [this] { if (!layingOut) resized(); };
        if (definition.initialValue)
        {
            const auto initial = *definition.initialValue;
            const auto valueText = displayText(definition, initial) + (id == "level" ? " dB" : "");
            cell->setResetAction([this, id, initial, valueText]() -> KnobValue::ResetAction
            {
                const auto* value = current(id);
                if (!value || !value->editable || busy.isVisible() || !onPrepareValueReset)
                    return {"Reset unavailable", {}};
                return {"Reset to default (" + valueText + ")", onPrepareValueReset(id, initial)};
            });
        }
        if (id == "level") cell->setHomeDetent({0., .3});
        content.addAndMakeVisible(*cell);
        dials.push_back({definition, std::move(cell)});
    }
    int layout(int width)
    {
        const juce::ScopedValueSetter<bool> guard(layingOut, true);
        using namespace responsiveGeometry;
        constexpr int margin = static_cast<int>(controlGeometry::focusOutset + controlGeometry::focusStroke);
        Rect area(margin, margin, std::max(0, width - 2 * margin), 10000);
        presetLabel.setBounds(area.removeFromTop(captionHeight));
        area.removeFromTop(spacing::labelHelp);
        auto row = area.removeFromTop(controlHeight);
        previous.setBounds(row.removeFromLeft(controlHeight + 2 * margin).expanded(0, margin));
        next.setBounds(row.removeFromRight(controlHeight + 2 * margin).expanded(0, margin));
        presets.setRowBounds(row);
        presetFeedback.arrange(view.presetError, true, row);
        if (view.presetError.isNotEmpty()) area.setY(presetFeedback.getBottom());
        if (view.showFile)
        {
            area.removeFromTop(contentPadding);
            fileLabel.setBounds(area.removeFromTop(captionHeight));
            area.removeFromTop(spacing::labelHelp);
            auto fileRow = area.removeFromTop(controlHeight);
            const auto feedbackBounds = fileRow;
            import.setBounds(fileRow.removeFromRight(controlHeight + 2 * margin).expanded(0, margin));
            fileRow.removeFromRight(spacing::tight);
            files.setRowBounds(fileRow);
            fileFeedback.arrange(view.fileError, true, feedbackBounds);
            if (view.fileError.isNotEmpty()) area.setY(fileFeedback.getBottom());
        }
        area.removeFromTop(contentPadding);
        if (view.tempoSync) sync.setBounds(area.removeFromTop(controlGeometry::switchHeight));
        const auto count = dials.size() + (externalLevel ? 1 : 0);
        const auto layoutDial = [&](size_t index, Rect bounds)
        {
            if (index < dials.size()) return dials[index].control->setValueBounds(bounds);
            return index == dials.size() && externalLevel ? externalLevel->setValueBounds(bounds) : 0;
        };
        for (size_t i = 0; i < count; i += 2)
        {
            auto pair = area.removeFromTop(valueRowHeight);
            const auto left = pair.removeFromLeft(std::max(0, (pair.getWidth() - contentPadding) / 2));
            pair.removeFromLeft(contentPadding);
            const auto firstHeight = layoutDial(i, left);
            const auto secondHeight = layoutDial(i + 1, pair);
            area.removeFromTop(std::max(firstHeight, secondHeight) - valueRowHeight);
        }
        description.arrange(view.description, false, area.withHeight(spacing::tight));
        return view.description.isEmpty() ? area.getY() + margin : description.getBottom();
    }

    View view;
    Rows presetRows, fileRows;
    std::optional<juce::String> previousChoice, nextChoice;
    juce::Component content;
    FocusViewport viewport;
    EllipsisLabel presetLabel, fileLabel;
    IconActionButton previous{ActionIcon::previous}, next{ActionIcon::next}, import{ActionIcon::openFile};
    ChoiceSelector presets, files;
    FieldFeedbackLabel fileFeedback, presetFeedback, description;
    Switch sync{"Tempo Sync", ToggleLabelSide::beforeIndicator};
    std::vector<Dial> dials;
    BusyPanel busy;
    juce::Component::SafePointer<KnobValue> externalLevel;
    juce::Component::SafePointer<juce::Component> restoreFocus;
    bool layingOut = false;
};
}
