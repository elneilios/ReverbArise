#pragma once

#include "ActionButton.h"
#include "EllipsisLabel.h"
#include "SelectorPresentation.h"

namespace m3::ui::controls
{
// The action owns the seam. Its paint bounds reserve the normal focus frame,
// including the brackets that extend over the start of the adjacent field.
class ActionButtonStateField final : public juce::Component, public juce::SettableTooltipClient
{
    static constexpr int focusMargin = SelectorPresentation::focusMargin;
    static constexpr int fieldPadding = 12;

    class StateLabel final : public EllipsisLabel, private juce::Timer
    {
    public:
        void setHovered(bool next)
        {
            if (hovered == next)
                return;
            hovered = next;
            resetScroll();
        }

        void resetScroll()
        {
            stopTimer();
            offset = 0.f;
            if (hovered && overflow() > 0)
            {
                scrollStart = juce::Time::getMillisecondCounterHiRes() + hoverDelayMs;
                startTimerHz(30);
            }
            repaint();
        }

        void resized() override { resetScroll(); }
        void lookAndFeelChanged() override { resetScroll(); }

        void paint(juce::Graphics& g) override
        {
            const auto bounds = getLocalBounds();
            if (bounds.isEmpty())
                return;

            const auto font = getLookAndFeel().getLabelFont(*this);
            const auto width = textWidth(font);
            const auto ink = findColour(juce::Label::textColourId);
            const auto fadeLeft = offset > 0.f;
            const auto fadeRight = static_cast<float>(width - bounds.getWidth()) > offset;
            g.setFont(font);
            if (fadeLeft || fadeRight)
            {
                const auto clear = ink.withAlpha(0.f);
                const auto fraction = std::min(0.5, static_cast<double>(spacing::related) / bounds.getWidth());
                juce::ColourGradient glyphInk(fadeLeft ? clear : ink, 0.f, 0.f,
                                              fadeRight ? clear : ink, static_cast<float>(bounds.getWidth()), 0.f, false);
                if (fadeLeft)
                    glyphInk.addColour(fraction, ink);
                if (fadeRight)
                    glyphInk.addColour(1.0 - fraction, ink);
                g.setGradientFill(glyphInk);
            }
            else
                g.setColour(ink);
            g.drawText(getText(), bounds.toFloat().withWidth(static_cast<float>(width)).translated(-offset, 0.f),
                       juce::Justification::centredLeft, false);
        }

    private:
        static constexpr double hoverDelayMs = 600.0;
        static constexpr double pixelsPerSecond = 24.0;

        int textWidth(const juce::Font& font) const
        {
            return static_cast<int>(std::ceil(juce::GlyphArrangement::getStringWidth(font, getText())));
        }

        int overflow()
        {
            return std::max(0, textWidth(getLookAndFeel().getLabelFont(*this)) - getWidth());
        }

        void timerCallback() override
        {
            const auto elapsed = juce::Time::getMillisecondCounterHiRes() - scrollStart;
            const auto end = static_cast<float>(overflow());
            const auto next = std::clamp(static_cast<float>(elapsed * pixelsPerSecond / 1000.0), 0.f, end);
            if (next != offset)
            {
                offset = next;
                repaint();
            }
            if (next >= end)
                stopTimer();
        }

        bool hovered = false;
        float offset = 0.f;
        double scrollStart = 0.0;
    };

    static juce::Path segment(juce::Rectangle<float> bounds, float cornerRadius, bool left)
    {
        juce::Path path;
        if (!bounds.isEmpty())
            path.addRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                     cornerRadius, cornerRadius, left, !left, left, !left);
        return path;
    }

    class JoinedAction final : public ActionButton
    {
    public:
        using ActionButton::ActionButton;
        std::function<void(bool)> onContextMenu;

        void mouseDown(const juce::MouseEvent& event) override
        {
            if(event.mods.isPopupMenu() && onContextMenu)
            {
                if(isEnabled())onContextMenu(false);
                return;
            }
            ActionButton::mouseDown(event);
        }

        bool keyPressed(const juce::KeyPress& key) override
        {
            if(key.isKeyCode(juce::KeyPress::F10Key) && key.getModifiers().isShiftDown() && isEnabled() && onContextMenu)
            {
                onContextMenu(true);
                return true;
            }
            return ActionButton::keyPressed(key);
        }

        bool hitTest(int x, int y) override
        {
            return x < getWidth() - focusMargin && ActionButton::hitTest(x, y);
        }

    private:
        void enablementChanged() override
        {
            ActionButton::enablementChanged();
            if (auto* parent = getParentComponent())
                parent->repaint();
        }

        void paintActionSurface(juce::Graphics& g, CommandVariant variant, CommandState state) override
        {
            const auto body = commandBodyBounds(getLocalBounds()).toFloat();
            const auto ink = commandInk(variant, state);
            const auto outside = segment(body, radius, true);
            const auto inside = segment(body.reduced(ink.stroke), radius - ink.stroke, true);
            auto border = outside;
            border.addPath(inside);
            border.setUsingNonZeroWinding(false);
            g.setColour(ink.fill);
            g.fillPath(inside);
            g.setColour(ink.border);
            g.fillPath(border);
            if (state.focused)
                paintCornerBrackets(g, getLocalBounds(), ink.focus);
        }
    };

public:
    static constexpr int defaultStateWidth = 124;

    ActionButtonStateField(const juce::String& label, CommandIcon icon, CommandVariant variant,
                           const juce::String& stateName, int actionWidth, int stateWidth = defaultStateWidth)
        : button(label, icon, variant), valueName(stateName), actionAllocation(actionWidth),
          stateAllocation(stateWidth)
    {
        applyTextRole(field, TextRole::controlValue);
        field.setColour(juce::Label::textColourId, text);
        field.setJustificationType(juce::Justification::centredLeft);
        field.setBorderSize({});
        field.setInterceptsMouseClicks(false, false);
        field.setTitle(valueName);
        addAndMakeVisible(field);
        addAndMakeVisible(button);
    }

    ActionButton& action() noexcept { return button; }
    void setContextMenu(std::function<void(bool)> showMenu) { button.onContextMenu = std::move(showMenu); }
    void setActionVariant(CommandVariant variant)
    {
        if (button.getVariant() == variant)
            return;
        button.setVariant(variant);
        if (!button.isEnabled())
            repaint();
    }
    void setStateFieldName(const juce::String& name) { field.setName(name); }

    int preferredWidth() const noexcept { return actionAllocation + stateAllocation; }

    void setState(const juce::String& acceptedValue, const juce::String& fullValue = {})
    {
        const auto description = valueName + ", " + (fullValue.isEmpty() ? acceptedValue : fullValue);
        if (field.getText() != acceptedValue)
        {
            field.setText(acceptedValue, juce::dontSendNotification);
            field.resetScroll();
        }
        if (field.getDescription() != description)
            field.setDescription(description);
        field.setTooltip(description);
    }

    void setActionHelp(const juce::String& help)
    {
        setTooltip(help);
        button.setTooltip(help);
        button.setDescription(help);
        button.setHelpText(help);
    }

    // The editor's fixed hover-help footer reads TooltipClient for context.
    juce::String getTooltip() override
    {
        if (fieldBody.contains(getMouseXYRelative()))
            return field.getTooltip();
        return juce::SettableTooltipClient::getTooltip();
    }

    void mouseEnter(const juce::MouseEvent& event) override { mouseMove(event); }
    void mouseMove(const juce::MouseEvent& event) override { field.setHovered(fieldBody.contains(event.getPosition())); }
    void mouseExit(const juce::MouseEvent&) override { field.setHovered(false); }
    void mouseDown(const juce::MouseEvent& event) override
    {
        if(event.mods.isPopupMenu() && fieldBody.contains(event.getPosition()) && button.isEnabled() && button.onContextMenu)
            button.onContextMenu(false);
    }

    void resized() override
    {
        auto row = getLocalBounds();
        button.setBounds(row.removeFromLeft(actionAllocation));
        const auto visible = getLocalBounds().withWidth(std::min(getWidth(), preferredWidth()));
        fieldBody = commandBodyBounds(visible).withLeft(
            commandBodyBounds(button.getBounds()).getRight());
        field.setBounds(fieldBody.reduced(fieldPadding, static_cast<int>(rule)));
    }

    void paint(juce::Graphics& g) override
    {
        const auto body = fieldBody.toFloat();
        const auto insideBounds = body.reduced(rule).withLeft(body.getX());
        const auto inside = segment(insideBounds, radius - rule, false);
        auto border = segment(body, radius, false);
        border.addPath(inside);
        border.setUsingNonZeroWinding(false);
        g.setColour(SelectorPresentation::raised());
        g.fillPath(inside);
        const auto rail = button.isEnabled() ? outline : commandInk(button.getVariant(),{false}).border;
        g.setColour(rail);
        g.fillPath(border);
    }

private:
    JoinedAction button;
    StateLabel field;
    juce::String valueName;
    int actionAllocation;
    int stateAllocation;
    juce::Rectangle<int> fieldBody;
};
}
