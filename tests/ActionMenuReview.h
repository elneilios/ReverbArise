#pragma once
#include <UI/Controls/ActionButton.h>
#include <UI/Controls/ChoicePopup.h>
#include <UI/Controls/SwellList.h>

namespace actionMenuReview
{
using namespace m3::ui;
using namespace m3::ui::controls;

class Fixture final : public juce::Component
{
public:
    Fixture()
    {
        addAndMakeVisible(swells);
        addAndMakeVisible(reset);
        addAndMakeVisible(edge);
        swells.setTitle("Swells actions");
        reset.setTitle("Knob Reset actions");
        edge.setTitle("Screen-edge actions");
        swells.display({{"preview-swell", "Room swell", "Preverb Hall", "Ready"}}, {}, {});
        swells.onMenu = [this](const SwellList::MenuRequest& request)
        {
            open(swells, false, false, request.anchor);
        };
        reset.onClick = [this] { open(reset, true, false); };
        edge.onClick = [this] { open(edge, false, true); };
    }

    void resized() override
    {
        auto row = getLocalBounds().reduced(spacing::section, 0).withTrimmedTop(112).removeFromTop(swells.getRowHeight());
        const auto width = (row.getWidth() - 2 * spacing::group) / 3;
        swells.setBounds(row.removeFromLeft(width));
        row.removeFromLeft(spacing::group);
        reset.setBounds(row.removeFromLeft(width).withHeight(44));
        row.removeFromLeft(spacing::group);
        edge.setBounds(row.withHeight(44));
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(background);
        const auto width = getWidth() - 2 * spacing::section;
        applyTextRole(g, *this, TextRole::sectionHeading);
        g.drawText("Action menus", spacing::section, 16, width, 30, juce::Justification::centredLeft);
        applyTextRole(g, *this, TextRole::helper);
        g.drawFittedText("Right-click the Room swell row or use its three dots. Try hovering, holding and moving off a row. Arrows inspect, Enter invokes and Escape dismisses.",
                         spacing::section, 51, width, 42, juce::Justification::centredLeft, 2, 1.f);
        g.drawFittedText(message, spacing::section, 186, width, 46, juce::Justification::centredLeft, 2, 1.f);
    }

    void open(juce::Component& trigger, bool resetSample, bool screenEdge, juce::Rectangle<int> anchor = {})
    {
        if (popup != nullptr)
            return;

        if (anchor.isEmpty())
            anchor = trigger.getScreenBounds();
        if (screenEdge)
            if (const auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect(anchor))
                anchor = display->userArea.withTrimmedLeft(display->userArea.getWidth() - 1)
                                          .withTrimmedTop(display->userArea.getHeight() - 1);

        using Role = PopupRowRole;
        std::vector<PopupChoice> rows;
        if (resetSample)
        {
            rows = {{1, "Reset to the initial value captured when this swell was created", {}, true, false, Role::action},
                    {2, "Reset to automatic start", "Automatic start is unavailable while this fixture has no source range", false, false, Role::action}};
        }
        else
        {
            rows = {{1, "Rename...", {}, true, false, Role::action},
                    {2, "Delete", {}, true, false, Role::destructiveAction}};
        }

        const auto actions = rows;
        const juce::Component::SafePointer<Fixture> safe(this);
        const juce::Component::SafePointer<juce::Component> owner(&trigger);
        popup = std::make_unique<ChoicePopup>(trigger, std::move(rows), [safe, owner, actions](int id, PopupDismissal why)
        {
            if (safe == nullptr)
                return;

            safe->popup.reset();
            safe->message = id == 0 ? "Menu dismissed" : "Preview action: " + actions[static_cast<size_t>(id - 1)].label;
            safe->repaint();
            if (owner != nullptr && why != PopupDismissal::outside && why != PopupDismissal::ownerGone)
            {
                owner->grabKeyboardFocus();
                if (owner != nullptr && (why == PopupDismissal::tabForward || why == PopupDismissal::tabBackward))
                    owner->moveKeyboardFocusToSibling(why == PopupDismissal::tabForward);
            }
        });
        popup->show(anchor, focusVisibility->isVisible());
    }

    SwellList swells;
    ActionButton reset{"Reset menu"}, edge{"Screen-edge menu"};
    std::unique_ptr<ChoicePopup> popup;
private:
    juce::SharedResourcePointer<FocusVisibility> focusVisibility;
    juce::String message{"Preview actions report here."};
};

class Panel final : public juce::Component
{
public:
    Panel()
    {
        setLookAndFeel(&theme);
        addAndMakeVisible(sizes);
        addAndMakeVisible(scales);
        addAndMakeVisible(fixture);
        sizes.setTitle("Review window size");
        scales.setTitle("Review UI scale");
        sizes.addItemList({"900 x 600", "1000 x 700", "1100 x 850", "1400 x 1000"}, 1);
        scales.addItemList({"100% UI scale", "125% UI scale"}, 1);
        sizes.setSelectedId(1, juce::dontSendNotification);
        scales.setSelectedId(1, juce::dontSendNotification);
        sizes.onChange = [this]
        {
            if (onSize && sizes.getSelectedItemIndex() >= 0)
                onSize(reviewSizes[static_cast<size_t>(sizes.getSelectedItemIndex())]);
        };
        scales.onChange = [this] { resized(); };
        setSize(900, 600);
    }
    ~Panel() override { setLookAndFeel(nullptr); }

    void resized() override
    {
        auto area = getLocalBounds();
        auto toolbar = area.removeFromTop(48).reduced(spacing::section, spacing::tight);
        sizes.setBounds(toolbar.removeFromLeft(152));
        toolbar.removeFromLeft(spacing::group);
        scales.setBounds(toolbar.removeFromLeft(152));
        const float scale = scales.getSelectedId() == 2 ? 1.25f : 1.f;
        fixture.setTransform(juce::AffineTransform::scale(scale));
        fixture.setBounds(0, area.getY(), static_cast<int>(area.getWidth() / scale), static_cast<int>(area.getHeight() / scale));
    }
    void paint(juce::Graphics& g) override { g.fillAll(background); }
    void setReviewScale(int percent) { scales.setSelectedId(percent == 125 ? 2 : 1, juce::sendNotificationSync); }

    static constexpr std::array<juce::Point<int>, 4> reviewSizes{{{900,600}, {1000,700}, {1100,850}, {1400,1000}}};
    std::function<void(juce::Point<int>)> onSize;
private:
    WorkspaceTheme theme;
    juce::ComboBox sizes, scales;
public:
    Fixture fixture;
};

inline void run()
{
    class Window final : public juce::DocumentWindow
    {
    public:
        Window() : juce::DocumentWindow("ReverbArise - #107 action menu review", background, closeButton)
        {
            setUsingNativeTitleBar(true);
            auto* panel = new Panel;
            setContentOwned(panel, true);
            panel->onSize = [this](juce::Point<int> size)
            {
                setContentComponentSize(size.x, size.y);
                centreWithSize(getWidth(), getHeight());
            };
            setResizable(true, false);
            setResizeLimits(760, 480, 1800, 1300);
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
