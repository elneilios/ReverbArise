#pragma once
#include <UI/Controls/ActionButton.h>
#include <UI/Controls/ChoiceSelector.h>
#include <UI/Controls/FieldTextEditor.h>

namespace fieldFeedbackReview
{
using namespace m3::ui;
using namespace m3::ui::controls;

enum class Scene { normal = 1, longHelp, validation, unavailable };

class Fixture final : public juce::Component
{
public:
    Fixture()
    {
        for (auto* control : std::array<juce::Component*, 4>{&ir, &start, &end, &apply})
            addAndMakeVisible(*control);
        ir.setTitle("Impulse response");
        start.setInputRestrictions(12, "0123456789");
        end.setInputRestrictions(12, "0123456789");
        ir.addItemList({"Preverb Hall", "Short Plate", "Studio Room"}, 1);
        ir.onChange = [this]
        {
            irError = false;
            refreshFeedback();
        };
        start.onReturnKey = end.onReturnKey = apply.onClick = [this] { submit(); };
        setScene(Scene::longHelp);
    }

    void setScene(Scene next)
    {
        scene = next;
        ir.hidePopup();
        irError = rangeError = scene == Scene::validation;
        ir.setEnabled(scene != Scene::unavailable);
        apply.setEnabled(scene != Scene::unavailable);
        ir.setSelectedId(1, juce::dontSendNotification);
        start.setText("4", false);
        end.setText(rangeError ? "4" : "8", false);
        refreshFeedback();
    }

    void resized() override
    {
        const auto content = getLocalBounds().reduced(spacing::section, 0);
        const auto columnWidth = (content.getWidth() - spacing::section) / 2;
        const auto left = content.getX();
        const auto right = left + columnWidth + spacing::section;
        const auto width = std::min(columnWidth, 300);
        const auto editorWidth = (width - spacing::related) / 2;
        const auto editorY = controlGeometry::headingHeight * 4 + controlGeometry::labelHeight;
        const auto frameHeight = controlGeometry::height + 2 * static_cast<int>(controlGeometry::focusOutset + controlGeometry::focusStroke);
        ir.setBounds(left, editorY, width, frameHeight);
        start.setBounds(right, editorY, editorWidth, frameHeight);
        end.setBounds(start.getRight() + spacing::related, editorY, editorWidth, frameHeight);
        layoutFeedback();
    }

    void paint(juce::Graphics& graphics) override
    {
        graphics.fillAll(background);
        applyTextRole(graphics, *this, TextRole::sectionHeading);
        auto area = getLocalBounds().reduced(spacing::section, 0);
        graphics.drawText("Field help and validation", area.removeFromTop(controlGeometry::headingHeight), juce::Justification::centredLeft);
        applyTextRole(graphics, *this, TextRole::helper);
        graphics.drawText("Choose a scene above. Edit the end bar and apply the range to clear a validation error.",
                          area.removeFromTop(controlGeometry::headingHeight), juce::Justification::centredLeft);
        graphics.drawText("Perform · Sound", area.getX(), area.getY(), ir.getWidth(), controlGeometry::headingHeight, juce::Justification::centredLeft);
        graphics.drawText("Settings · Swell", start.getX(), area.getY(), end.getRight() - start.getX(),
                          controlGeometry::headingHeight, juce::Justification::centredLeft);
        applyTextRole(graphics, *this, TextRole::controlLabel);
        graphics.drawText("Source range (bars)", commandBodyBounds(start.getBounds()).getX(),
                          area.getY() + controlGeometry::headingHeight, end.getRight() - start.getX(),
                          controlGeometry::headingHeight, juce::Justification::centredLeft);
        for (const auto* field : std::array<const juce::Component*, 3>{&ir, &start, &end})
        {
            const auto body = commandBodyBounds(field->getBounds());
            const auto caption = body.withHeight(controlGeometry::labelHeight).withY(field->getY() - controlGeometry::labelHeight);
            graphics.drawText(field->getTitle(), caption, juce::Justification::centredLeft);
        }
        singleFeedback.paint(graphics);
        pairFeedback.paint(graphics);
    }

    ChoiceSelector ir;
    FieldTextEditor start{"Bar start"}, end{"End excl."};
    ActionButton apply{"Set source from bars"};
    FieldPresentation::WrappedFeedback singleFeedback, pairFeedback;

private:
    void submit()
    {
        if (!apply.isEnabled())
            return;
        const auto first = start.getText().getLargeIntValue();
        rangeError = first < 1 || end.getText().getLargeIntValue() <= first;
        refreshFeedback();
    }

    void refreshFeedback()
    {
        if (scene == Scene::unavailable)
        {
            irMessage = "Select a swell to choose its impulse response.";
            rangeMessage = "Unfreeze the selected swell to change its source range.";
        }
        else
        {
            irMessage = irError ? "The imported impulse response could not be read. Choose another response to clear this error."
                                : scene == Scene::longHelp ? "This response applies to the selected swell. Learn keeps its own default for new swells."
                                                           : "Applies to the selected swell.";
            rangeMessage = rangeError ? "The end bar must be after the start bar. Enter a later bar, then choose Set source from bars to apply the corrected range."
                                      : scene == Scene::longHelp ? "The start bar is included. The end bar marks the first bar after the source range. A start of 4 and an end of 8 use bars 4 through 7."
                                                                 : "The end bar is excluded.";
        }

        ir.setDescription(irMessage);
        ir.setValidationMessage(irError ? irMessage : juce::String{});
        start.setFieldDescription(rangeMessage);
        end.setFieldDescription(rangeMessage);
        end.setInvalid(rangeError);
        layoutFeedback();
        repaint();
    }

    void layoutFeedback()
    {
        const auto singleBody = commandBodyBounds(ir.getBounds());
        const auto pairBodies = commandBodyBounds(start.getBounds()).getUnion(commandBodyBounds(end.getBounds()));
        singleFeedback.setMessage(irMessage, irError, singleBody);
        pairFeedback.setMessage(rangeMessage, rangeError, pairBodies);
        const auto bottom = std::max(pairBodies.getBottom(), pairFeedback.getBounds().getBottom());
        apply.setBounds(start.getBounds().getUnion(end.getBounds()).withY(bottom + spacing::related));
    }

    Scene scene = Scene::longHelp;
    juce::String irMessage, rangeMessage;
    bool irError = false, rangeError = false;
};

class Panel final : public juce::Component
{
public:
    Panel()
    {
        setLookAndFeel(&theme);
        for (auto* component : std::array<juce::Component*, 4>{&sizes, &scales, &scenes, &fixture})
            addAndMakeVisible(*component);
        sizes.setTitle("Review window size");
        sizes.addItemList({"900 x 600", "1100 x 850"}, 1);
        scales.setTitle("Review UI scale");
        scales.addItemList({"100% UI scale", "125% UI scale"}, 1);
        scenes.setTitle("Review scene");
        scenes.addItemList({"Normal help", "Wrapped help", "Validation errors", "Unavailable"}, 1);
        sizes.setSelectedId(1, juce::dontSendNotification);
        scales.setSelectedId(1, juce::dontSendNotification);
        scenes.setSelectedId(static_cast<int>(Scene::longHelp), juce::dontSendNotification);
        sizes.onChange = [this]
        {
            const auto size = sizes.getSelectedId() == 2 ? juce::Point<int>{1100, 850} : juce::Point<int>{900, 600};
            if (onSize)
                onSize(size);
        };
        scales.onChange = [this] { resized(); };
        scenes.onChange = [this] { fixture.setScene(static_cast<Scene>(scenes.getSelectedId())); };
        setSize(900, 600);
    }

    ~Panel() override { setLookAndFeel(nullptr); }

    void resized() override
    {
        auto area = getLocalBounds();
        auto toolbar = area.removeFromTop(48).reduced(spacing::section, spacing::tight);
        for (auto* control : {&sizes, &scales, &scenes})
        {
            control->setBounds(toolbar.removeFromLeft(164));
            toolbar.removeFromLeft(spacing::related);
        }
        const float scale = scales.getSelectedId() == 2 ? 1.25f : 1.f;
        fixture.setTransform(juce::AffineTransform::scale(scale));
        fixture.setBounds(0, area.getY(), static_cast<int>(area.getWidth() / scale), static_cast<int>(area.getHeight() / scale));
    }

    void paint(juce::Graphics& graphics) override { graphics.fillAll(background); }
    void setScale(int percent) { scales.setSelectedId(percent == 125 ? 2 : 1, juce::sendNotificationSync); }

    std::function<void(juce::Point<int>)> onSize;
    Fixture fixture;

private:
    WorkspaceTheme theme;
    juce::ComboBox sizes, scales, scenes;
};

inline void run()
{
    class Window final : public juce::DocumentWindow
    {
    public:
        Window() : juce::DocumentWindow("ReverbArise - #107 field feedback review", background, closeButton)
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
