#pragma once

#include <UI/Controls/BooleanToggle.h>

namespace toggleReview
{
using namespace m3::ui;
using namespace m3::ui::controls;

class Scene final : public juce::Component
{
public:
    Scene()
    {
        for (auto* control : switches)
            addAndMakeVisible(*control);
        for (auto* control : flags)
            addAndMakeVisible(*control);
        for (auto* control : {&live, &isolate, &enabled, &unavailable})
            control->setToggleState(true, juce::dontSendNotification);
        unavailable.setEnabled(false);
        unavailable.setDescription("Unavailable during wet-only audition. The chosen value is retained.");
        unavailable.setTooltip(unavailable.getDescription());
        automatic.setTooltip("Capture and generate swells automatically during an eligible Learn pass.");
        longLabel.setTooltip("This longer label checks the width available beside the switch.");
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(spacing::section, spacing::group);
        area.removeFromTop(74);
        auto cards = area.removeFromTop(286);
        learnCard = cards.removeFromLeft((cards.getWidth() - spacing::group) / 2);
        cards.removeFromLeft(spacing::group);
        settingsCard = cards;
        auto learn = learnCard.reduced(spacing::related);
        auto settings = settingsCard.reduced(spacing::related);
        learn.removeFromTop(42);
        settings.removeFromTop(42);
        for (int i = 0; i < 4; ++i)
        {
            switches[static_cast<size_t>(i)]->setBounds(learn.removeFromTop(controlGeometry::switchHeight));
            switches[static_cast<size_t>(i + 4)]->setBounds(settings.removeFromTop(controlGeometry::switchHeight));
        }
        unavailableHelp = settings.removeFromTop(44).withTrimmedLeft(controlGeometry::switchWidth + spacing::tight);

        area.removeFromTop(spacing::group);
        inspectorCard = area.removeFromTop(104).withWidth(responsiveGeometry::inspectorWidth);
        auto row = inspectorCard.reduced(responsiveGeometry::contentPadding);
        row.removeFromTop(controlGeometry::headingHeight);
        row = row.removeFromTop(responsiveGeometry::flagsHeight);
        enabled.setBounds(row.removeFromLeft(90));
        solo.setBounds(row.removeFromLeft(65));
        freeze.setBounds(row);
    }

    void paint(juce::Graphics& graphics) override
    {
        graphics.fillAll(background);
        auto header = getLocalBounds().reduced(spacing::section, spacing::group);
        applyTextRole(graphics, TextRole::sectionHeading);
        graphics.drawText("Switches and inline checkboxes", header.removeFromTop(34), juce::Justification::centredLeft);
        applyTextRole(graphics, TextRole::helper);
        graphics.drawText("Sample values. Click a label or indicator. Tab shows focus and Space toggles.",
                          header.removeFromTop(24), juce::Justification::centredLeft);
        card(graphics, learnCard, "Learn");
        card(graphics, settingsCard, "Settings");
        card(graphics, inspectorCard, "Swell Inspector");
        applyTextRole(graphics, TextRole::helper);
        graphics.drawFittedText(unavailable.getDescription(), unavailableHelp, juce::Justification::centredLeft, 2, 1.f);
    }

    void checkLayout() const
    {
        for (const auto* control : switches)
            checkControl(*control);
        for (const auto* control : flags)
        {
            checkControl(*control);
            REQUIRE(inspectorCard.contains(control->getBounds()));
            REQUIRE(control->getY() == enabled.getY());
        }
        REQUIRE(!enabled.getBounds().intersects(solo.getBounds()));
        REQUIRE(!solo.getBounds().intersects(freeze.getBounds()));
    }

private:
    static void checkControl(const BooleanToggle& control)
    {
        const auto indicator = control.indicatorBounds();
        REQUIRE(!indicator.isEmpty());
        REQUIRE(control.getLocalBounds().contains(indicator.expanded(5)));
        REQUIRE(!indicator.intersects(control.labelBounds()));
        REQUIRE(juce::GlyphArrangement::getStringWidth(textFont(TextRole::controlLabel), control.getButtonText())
                <= control.labelBounds().getWidth());
    }

    static void card(juce::Graphics& graphics, juce::Rectangle<int> bounds, const juce::String& title)
    {
        graphics.setColour(background.interpolatedWith(outline, .07f));
        graphics.fillRoundedRectangle(bounds.toFloat(), radius);
        graphics.setColour(separator);
        graphics.drawRoundedRectangle(controlGeometry::insideStroke(bounds, rule), radius, rule);
        applyTextRole(graphics, TextRole::controlLabel);
        graphics.drawText(title, bounds.reduced(spacing::related, spacing::tight).removeFromTop(24),
                          juce::Justification::centredLeft);
    }

    BooleanToggle recapture{"Recapture existing audio", ToggleForm::track};
    BooleanToggle automatic{"Enable automatic swells", ToggleForm::track};
    BooleanToggle autoStop{"Auto-generate on Stop", ToggleForm::track};
    BooleanToggle longLabel{"Auto-refresh after recapturing existing audio", ToggleForm::track};
    BooleanToggle live{"Live playback", ToggleForm::track};
    BooleanToggle map{"Map while playing", ToggleForm::track};
    BooleanToggle isolate{"Isolate selected", ToggleForm::track};
    BooleanToggle unavailable{"Isolate dry source", ToggleForm::track};
    BooleanToggle enabled{"Enabled", ToggleForm::checkbox};
    BooleanToggle solo{"Solo", ToggleForm::checkbox};
    BooleanToggle freeze{"Freeze", ToggleForm::checkbox};
    const std::array<BooleanToggle*, 8> switches{&recapture, &automatic, &autoStop, &longLabel,
                                               &live, &map, &isolate, &unavailable};
    const std::array<BooleanToggle*, 3> flags{&enabled, &solo, &freeze};
    juce::Rectangle<int> learnCard, settingsCard, inspectorCard, unavailableHelp;
};

class Panel final : public juce::Component
{
public:
    Panel()
    {
        addAndMakeVisible(scene);
        addAndMakeVisible(sizeChoice);
        addAndMakeVisible(scaleChoice);
        sizeChoice.setTitle("Preview size");
        scaleChoice.setTitle("Preview UI scale");
        sizeChoice.addItem("900 x 600", 1);
        sizeChoice.addItem("1100 x 700", 2);
        scaleChoice.addItem("100%", 1);
        scaleChoice.addItem("125%", 2);
        sizeChoice.setSelectedId(1, juce::dontSendNotification);
        scaleChoice.setSelectedId(1, juce::dontSendNotification);
        sizeChoice.onChange = [this] { refreshSize(); };
        scaleChoice.onChange = [this] { refreshSize(); };
        refreshSize();
    }

    void paint(juce::Graphics& graphics) override { graphics.fillAll(background); }

    void resized() override
    {
        auto bar = getLocalBounds().removeFromTop(36).reduced(spacing::related, spacing::tight);
        sizeChoice.setBounds(bar.removeFromLeft(132));
        bar.removeFromLeft(spacing::tight);
        scaleChoice.setBounds(bar.removeFromLeft(84));
        const int logicalWidth = sizeChoice.getSelectedId() == 2 ? 1100 : 900;
        const int logicalHeight = sizeChoice.getSelectedId() == 2 ? 700 : 600;
        scene.setBounds(0, 0, logicalWidth, logicalHeight);
        scene.setTransform(juce::AffineTransform::scale(scale).translated(0.f, 36.f));
    }

    void checkLayout()
    {
        for (const int size : {1, 2})
            for (const int scaleId : {1, 2})
            {
                sizeChoice.setSelectedId(size, juce::dontSendNotification);
                scaleChoice.setSelectedId(scaleId, juce::dontSendNotification);
                refreshSize();
                scene.checkLayout();
                REQUIRE(getLocalBounds().contains(getLocalArea(&scene, scene.getLocalBounds())));
            }
    }

private:
    void refreshSize()
    {
        scale = scaleChoice.getSelectedId() == 2 ? 1.25f : 1.f;
        const bool large = sizeChoice.getSelectedId() == 2;
        setSize(juce::roundToInt((large ? 1100 : 900) * scale),
                36 + juce::roundToInt((large ? 700 : 600) * scale));
    }

    Scene scene;
    juce::ComboBox sizeChoice, scaleChoice;
    float scale = 1.f;
};

inline void run()
{
    class Window final : public juce::DocumentWindow
    {
    public:
        Window() : DocumentWindow("ReverbArise - #107 P6 JUCE switch and checkbox review", background, closeButton)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new Panel(), true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }
        void closeButtonPressed() override { juce::MessageManager::getInstance()->stopDispatchLoop(); }
    };
    WorkspaceTheme theme;
    juce::LookAndFeel::setDefaultLookAndFeel(&theme);
    {
        Window window;
        juce::MessageManager::getInstance()->runDispatchLoop();
    }
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}
}
