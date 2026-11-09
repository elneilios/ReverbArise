#pragma once
#include <Core/MusicalPosition.h>
#include <UI/Controls/ActionButton.h>
#include <UI/Controls/SliderControl.h>
#include <charconv>
#include <optional>

namespace numericTextFieldsReview
{
using namespace m3::ui;
using namespace m3::ui::controls;

enum class Scene { inspector = 1, compact, bars, diagnostics, rename };

class ReviewSlider final : public SliderControl
{
public:
    explicit ReviewSlider(FieldMetadata metadata) : SliderControl(std::move(metadata))
    {
        // JUCE normally lets Ctrl switch drag modes. The preview uses it only for the wheel.
        setVelocityModeParameters(1.0, 1, 0, false);
    }

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override
    {
        auto adjusted = wheel;
        if (event.mods.isCtrlDown())
        {
            adjusted.deltaX *= .1f;
            adjusted.deltaY *= .1f;
        }
        SliderControl::mouseWheelMove(event, adjusted);
    }
};

class Field final : public juce::Component
{
public:
    Field(const juce::String& title, FieldTextEditor::Role role = FieldTextEditor::Role::editable)
        : editor(title, role)
    {
        caption.setText(title, juce::dontSendNotification);
        applyTextRole(caption, TextRole::controlLabel);
        caption.setBorderSize({});
        caption.setAccessible(false);
        addAndMakeVisible(caption);
        addAndMakeVisible(editor);
        addAndMakeVisible(feedback);
    }

    void describe(const juce::String& message, bool error)
    {
        detail = message;
        editor.setFieldDescription(detail);
        editor.setInvalid(error);
        resized();
    }

    int place(int x, int y, int width)
    {
        setBounds(x, y, width, controlGeometry::labelHeight + frameHeight);
        resized();
        setSize(width, std::max(editor.getBottom(), feedback.getBottom()));
        return getBottom();
    }

    void resized() override
    {
        caption.setBounds(margin, 0, std::max(0, getWidth() - 2 * margin), controlGeometry::labelHeight);
        editor.setBounds(0, caption.getBottom(), getWidth(), frameHeight);
        feedback.arrange(detail, editor.isInvalid(), commandBodyBounds(editor.getBounds()));
    }

    static constexpr int margin = SelectorPresentation::focusMargin;
    static constexpr int frameHeight = controlGeometry::height + 2 * margin;
    FieldTextEditor editor;
    FieldFeedbackLabel feedback;

private:
    juce::Label caption;
    juce::String detail;
};

class Fixture final : public juce::Component
{
public:
    Fixture()
    {
        for (auto* component : std::array<juce::Component*, 15>{
                 &timing, &level, &reset, &sensitivity, &standalone, &standaloneFeedback, &barStart, &barEnd, &barFeedback, &apply,
                 &exact, &unavailable, &name, &rename, &cancel})
            addAndMakeVisible(*component);
        addAndMakeVisible(reopen);

        timing.editor.setNumericCharacters("0123456789.-");
        level.editor.setNumericCharacters("0123456789+.-");
        timing.editor.onReturnKey = [this] { submitInspector(timing); };
        level.editor.onReturnKey = [this] { submitInspector(level); };
        timing.editor.onFocusLost = [this] { if (timing.editor.getText() != timingValue) submitInspector(timing); };
        level.editor.onFocusLost = [this] { if (level.editor.getText() != levelValue) submitInspector(level); };
        timing.editor.onEscapeKey = [this] { restore(timing, timingValue, timingHelp); };
        level.editor.onEscapeKey = [this] { restore(level, levelValue, levelHelp); };
        reset.onClick = [this] { resetInspector(); };

        sensitivity.setRange(0, 100, .1);
        sensitivity.setTextValueSuffix(" %");
        sensitivity.setNumDecimalPlacesToDisplay(1);
        sensitivity.setValue(42, juce::dontSendNotification);
        sensitivity.setTooltip(sensitivityHelp);
        standalone.setEditable(true);
        standalone.setText(juce::String(standaloneValue), juce::dontSendNotification);
        standalone.onTextChange = [this] { submitStandalone(); };
        standalone.onStep = [this](int direction)
        {
            standalone.hideEditor(false);
            standaloneValue = juce::jlimit(0, 100, standaloneValue + direction);
            restoreStandalone(standaloneHelp, false);
        };
        for (auto* bar : {&barStart, &barEnd})
        {
            bar->editor.setInputRestrictions(12, "0123456789");
            bar->editor.setNumericCharacters("0123456789");
            bar->editor.onReturnKey = [this] { submitBars(); };
            bar->editor.onFocusLost = [this] { validateBars(); };
        }
        apply.onClick = [this] { submitBars(); };
        barStart.editor.setText("4", false);
        barEnd.editor.setText("8", false);
        describeBars(barHelp, false);

        exact.editor.setText("96037", false);
        exact.describe("Exact captured value. Select and copy this number for inspection.", false);
        unavailable.editor.setText("8", false);
        unavailable.editor.setEnabled(false);
        unavailable.describe("Unfreeze the selected swell to change its source range.", false);
        name.editor.onReturnKey = [this] { submitName(); };
        name.editor.onEscapeKey = [this] { closeRename(false); };
        name.editor.onFocusLost = [this] { if (renameOpen && scene == Scene::rename) validateName(); };
        rename.onClick = [this] { submitName(); };
        cancel.onClick = [this] { closeRename(false); };
        reopen.onClick = [this] { openRename(); };
        resetInspector();
        openRename();
        setScene(Scene::inspector);
    }

    void setScene(Scene next)
    {
        scene = next;
        for (auto* component : {static_cast<juce::Component*>(&timing), static_cast<juce::Component*>(&level),
                                static_cast<juce::Component*>(&reset)})
            component->setVisible(scene == Scene::inspector);
        sensitivity.setVisible(scene == Scene::compact);
        standalone.setVisible(scene == Scene::compact);
        standaloneFeedback.setVisible(scene == Scene::compact);
        for (auto* component : std::array<juce::Component*, 4>{&barStart, &barEnd, &barFeedback, &apply})
            component->setVisible(scene == Scene::bars);
        exact.setVisible(scene == Scene::diagnostics);
        unavailable.setVisible(scene == Scene::diagnostics);
        updateRenameVisibility();
        resized();
        repaint();
    }

    void resized() override
    {
        const auto left = spacing::section;
        const auto top = controlGeometry::headingHeight * 3;
        const auto width = std::min(getWidth() - 2 * left, 316);
        if (width <= Field::margin * 2) return;
        if (scene == Scene::inspector)
        {
            const auto afterTiming = timing.place(left, top, width);
            const auto afterLevel = level.place(left, afterTiming + spacing::group, width);
            reset.setBounds(left, afterLevel + spacing::group, width, Field::frameHeight);
        }
        else if (scene == Scene::compact)
        {
            sensitivity.setBounds(left, top + controlGeometry::labelHeight, width, controlGeometry::height);
            standalone.setBounds(left, sensitivity.getBottom() + spacing::section + controlGeometry::labelHeight,
                                 width / 2, Field::frameHeight);
            standaloneFeedback.arrange(standalone.getDescription(), standalone.isInvalid(),
                commandBodyBounds(standalone.getBounds().withTrimmedRight(controlGeometry::numericStepWidth)));
        }
        else if (scene == Scene::bars)
        {
            const auto fieldWidth = (width - spacing::related) / 2;
            barStart.place(left, top, fieldWidth);
            barEnd.place(barStart.getRight() + spacing::related, top, fieldWidth);
            const auto bodies = commandBodyBounds(getLocalArea(&barStart.editor, barStart.editor.getLocalBounds()))
                .getUnion(commandBodyBounds(getLocalArea(&barEnd.editor, barEnd.editor.getLocalBounds())));
            barFeedback.arrange(barMessage, barStart.editor.isInvalid() || barEnd.editor.isInvalid(), bodies);
            apply.setBounds(left, barFeedback.getBottom() + spacing::related, width, Field::frameHeight);
        }
        else if (scene == Scene::diagnostics)
        {
            const auto afterExact = exact.place(left, top, width);
            unavailable.place(left, afterExact + spacing::group, width);
        }
        else if (renameOpen)
        {
            const auto bottom = name.place(left, top, width);
            auto actions = juce::Rectangle<int>{left, bottom + spacing::related, width, Field::frameHeight};
            rename.setBounds(actions.removeFromLeft(width / 2));
            cancel.setBounds(actions.withTrimmedLeft(spacing::tight));
        }
        else
            reopen.setBounds(left, top + controlGeometry::labelHeight, width, Field::frameHeight);
    }

    void paint(juce::Graphics& graphics) override
    {
        graphics.fillAll(background);
        auto area = getLocalBounds().reduced(spacing::section, 0);
        applyTextRole(graphics, *this, TextRole::sectionHeading);
        graphics.drawText("Numeric and text fields", area.removeFromTop(controlGeometry::headingHeight),
                          juce::Justification::centredLeft);
        juce::String section, instruction;
        switch (scene)
        {
            case Scene::inspector:
                section = "Perform inspector";
                instruction = "Try beat 5 or Level above +12 dB. Press Return or leave the field to validate. Escape cancels.";
                break;
            case Scene::compact:
                section = "Slider and standalone number field";
                instruction = "Hold Ctrl while scrolling over the slider track. Scroll over either number box to step it.";
                break;
            case Scene::bars:
                section = "Settings · Source range (bars)";
                instruction = "Set End excl. to 4 and leave the field. Apply the range with the button or Return.";
                break;
            case Scene::diagnostics:
                section = "Diagnostics · Field states";
                instruction = "The exact value can be selected and copied. The unavailable field keeps its guidance.";
                break;
            case Scene::rename:
                section = "Rename swell";
                instruction = renameOpen ? "Clear the name and leave the field or press Return to try the local error. Escape cancels."
                                         : "Accepted name: " + acceptedName;
                break;
        }
        applyTextRole(graphics, *this, TextRole::controlLabel);
        graphics.drawText(section, area.removeFromTop(controlGeometry::headingHeight), juce::Justification::centredLeft);
        applyTextRole(graphics, *this, TextRole::helper);
        graphics.drawText(instruction, area.removeFromTop(controlGeometry::headingHeight), juce::Justification::centredLeft);
        if (scene == Scene::compact)
        {
            applyTextRole(graphics, *this, TextRole::controlLabel);
            for (const auto* control : std::array<const juce::Component*, 2>{&sensitivity, &standalone})
            {
                const auto caption = control == &sensitivity ? "Event sensitivity (%)" : "Standalone number";
                graphics.drawText(caption, control->getBounds().withHeight(controlGeometry::labelHeight)
                                  .translated(0, -controlGeometry::labelHeight), juce::Justification::centredLeft);
            }
        }
    }

    Field timing{"Source start (bars)"}, level{"Level (dB)"};
    ActionButton reset{"Reset fields"};
    const juce::String sensitivityHelp{"Percent. Lower values find fewer, stronger attacks."};
    ReviewSlider sensitivity{SliderControl::FieldMetadata{"Event sensitivity (percent)", sensitivityHelp}};
    const juce::String standaloneHelp{"Enter a whole number from 0 to 100. The arrows change it by one."};
    FieldNumericInput standalone{"Standalone number", standaloneHelp};
    FieldFeedbackLabel standaloneFeedback;
    int standaloneValue = 42;
    Field barStart{"Bar start"}, barEnd{"End excl."};
    FieldFeedbackLabel barFeedback;
    ActionButton apply{"Set source from bars"};
    int submittedRanges = 0;
    Field exact{"Exact source start (samples) · Read only", FieldTextEditor::Role::readOnly};
    Field unavailable{"End excl. · Unavailable"};
    Field name{"Name"};
    ActionButton rename{"OK"}, cancel{"Cancel"}, reopen{"Rename again"};
    juce::String acceptedName{"Room swell"};

private:
    template<class Number>
    static std::optional<Number> parseNumber(const juce::String& draft)
    {
        const auto value = draft.toStdString();
        Number number{};
        const auto first = value.data() + (!value.empty() && value.front() == '+' ? 1 : 0);
        const auto last = value.data() + value.size();
        const auto parsed = std::from_chars(first, last, number);
        if (parsed.ec == std::errc{} && parsed.ptr == last) return number;
        return {};
    }

    void restoreStandalone(const juce::String& message, bool invalid)
    {
        standalone.setText(juce::String(standaloneValue), juce::dontSendNotification);
        standalone.setFeedback(message, invalid);
        resized();
    }

    void submitStandalone()
    {
        const auto value = parseNumber<int>(standalone.getText());
        if (value && *value >= 0 && *value <= 100)
        {
            standaloneValue = *value;
            restoreStandalone(standaloneHelp, false);
        }
        else
            restoreStandalone("Enter a whole number from 0 to 100.", true);
    }

    void restore(Field& field, const juce::String& accepted, const juce::String& help)
    {
        field.editor.setText(accepted, false);
        field.describe(help, false);
        resized();
    }

    void resetInspector()
    {
        timingValue = "2.1.0";
        levelValue = "0.0";
        restore(timing, timingValue, timingHelp);
        restore(level, levelValue, levelHelp);
    }

    void submitInspector(Field& field)
    {
        const bool isTiming = &field == &timing;
        auto& accepted = isTiming ? timingValue : levelValue;
        const auto draft = field.editor.getText();
        if (draft == accepted && !field.editor.isInvalid()) return;
        bool valid = false;
        if (isTiming)
        {
            const SongTime clock{true, 0, 0, 48000, 120, 4, 4};
            int64_t sample = 0;
            valid = m3::parseMusicalPosition(draft, clock, sample);
        }
        else
        {
            const auto db = parseNumber<double>(draft);
            valid = db && *db >= -12 && *db <= 12;
        }
        if (valid)
            accepted = draft;
        field.editor.setText(accepted, false);
        const auto reason = isTiming
            ? "Enter bar.beat.fraction, such as 2.1.0. For sample positions, choose exact samples in Settings."
            : "Level must be between -12 and +12 dB. Enter a value in that range and press Return to try again.";
        field.describe(valid ? (isTiming ? timingHelp : levelHelp) : juce::String(reason), !valid);
        resized();
    }

    void describeBars(const juce::String& message, bool endInvalid, bool startInvalid = false)
    {
        barMessage = message;
        barStart.editor.setFieldDescription(message);
        barEnd.editor.setFieldDescription(message);
        barStart.editor.setInvalid(startInvalid);
        barEnd.editor.setInvalid(endInvalid);
        resized();
    }

    bool validateBars()
    {
        const auto start = barStart.editor.getText().getLargeIntValue();
        const auto end = barEnd.editor.getText().getLargeIntValue();
        if (start < 1)
            describeBars("The start bar must be 1 or later.", false, true);
        else if (end <= start)
            describeBars("The end bar must be after the start bar. Enter a later bar, then apply the corrected range.", true);
        else
            describeBars(barHelp, false);
        return start >= 1 && end > start;
    }

    void submitBars()
    {
        if (validateBars()) ++submittedRanges;
    }

    bool validateName()
    {
        const bool valid = name.editor.getText().isNotEmpty();
        name.describe(valid ? "This name appears in the Swells list." : "Enter a name for this swell before choosing OK.", !valid);
        resized();
        return valid;
    }

    void submitName()
    {
        if (!validateName())
        {
            if (!name.editor.hasKeyboardFocus(false)) name.editor.grabKeyboardFocus();
            return;
        }
        closeRename(true);
    }

    void openRename()
    {
        renameOpen = true;
        restore(name, acceptedName, "This name appears in the Swells list.");
        updateRenameVisibility();
        resized();
        repaint();
    }

    void closeRename(bool accept)
    {
        if (accept) acceptedName = name.editor.getText();
        renameOpen = false;
        updateRenameVisibility();
        resized();
        repaint();
    }

    void updateRenameVisibility()
    {
        name.setVisible(scene == Scene::rename && renameOpen);
        rename.setVisible(name.isVisible());
        cancel.setVisible(name.isVisible());
        reopen.setVisible(scene == Scene::rename && !renameOpen);
    }

    const juce::String timingHelp{"Enter a bar, beat and fraction."};
    const juce::String levelHelp{"Adjust the selected swell from -12 to +12 dB."};
    const juce::String barHelp{"The end bar is excluded."};
    juce::String timingValue, levelValue, barMessage;
    Scene scene = Scene::inspector;
    bool renameOpen = false;
};

class Panel final : public juce::Component
{
public:
    Panel()
    {
        setLookAndFeel(&theme);
        for (auto* component : std::array<juce::Component*, 4>{&scene, &size, &scale, &fixture})
            addAndMakeVisible(*component);
        scene.setTitle("Preview scene");
        scene.addItemList({"Perform inspector", "Compact numeric value", "Settings bar pair", "Diagnostics states", "Rename swell"}, 1);
        size.setTitle("Preview size");
        size.addItemList({"900 x 600", "1100 x 850"}, 1);
        scale.setTitle("Preview scale");
        scale.addItemList({"100% UI scale", "125% UI scale"}, 1);
        for (auto* box : {&scene, &size, &scale}) box->setSelectedId(1, juce::dontSendNotification);
        scene.onChange = [this] { fixture.setScene(static_cast<Scene>(scene.getSelectedId())); };
        scale.onChange = [this] { resized(); };
        size.onChange = [this]
        {
            if (onSize) onSize(size.getSelectedId() == 2 ? juce::Point<int>{1100, 850} : juce::Point<int>{900, 600});
        };
        setSize(900, 600);
    }

    ~Panel() override { setLookAndFeel(nullptr); }
    void setScale(int percent) { scale.setSelectedId(percent == 125 ? 2 : 1, juce::sendNotificationSync); }
    void paint(juce::Graphics& graphics) override { graphics.fillAll(background); }

    void resized() override
    {
        auto area = getLocalBounds();
        auto toolbar = area.removeFromTop(Field::frameHeight + spacing::tight).reduced(spacing::section, spacing::tight);
        scene.setBounds(toolbar.removeFromLeft(220));
        for (auto* box : {&size, &scale})
        {
            toolbar.removeFromLeft(spacing::related);
            box->setBounds(toolbar.removeFromLeft(152));
        }
        const float factor = scale.getSelectedId() == 2 ? 1.25f : 1.f;
        fixture.setTransform(juce::AffineTransform::scale(factor));
        fixture.setBounds(0, area.getY(), static_cast<int>(area.getWidth() / factor), static_cast<int>(area.getHeight() / factor));
    }

    Fixture fixture;
    std::function<void(juce::Point<int>)> onSize;

private:
    WorkspaceTheme theme;
    juce::ComboBox scene, size, scale;
};

inline void run()
{
    class Window final : public juce::DocumentWindow
    {
    public:
        Window() : juce::DocumentWindow("ReverbArise - #107 numeric and text fields review", background, closeButton)
        {
            setUsingNativeTitleBar(true);
            auto panel = std::make_unique<Panel>();
            panel->onSize = [this](juce::Point<int> size) { setContentComponentSize(size.x, size.y); };
            setContentOwned(panel.release(), true);
            setResizable(true, false);
            setResizeLimits(900, 600, 1600, 1100);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
            toFront(true);
        }

        void closeButtonPressed() override { juce::MessageManager::getInstance()->stopDispatchLoop(); }
    };

    Window window;
    juce::MessageManager::getInstance()->runDispatchLoop();
}
}
