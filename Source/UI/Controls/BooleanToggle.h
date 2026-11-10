#pragma once

#include <UI/WorkspaceTheme.h>
#include "FocusVisibility.h"
#include "InvocationToggle.h"

namespace m3::ui::controls
{
enum class ToggleForm { track, checkbox };
enum class ToggleLabelSide { afterIndicator, beforeIndicator };

inline void paintToggleIndicator(juce::Graphics& graphics, juce::Rectangle<int> bounds,
                                 ToggleForm form, CommandState state)
{
    if (bounds.isEmpty()) return;

    const bool on = state.latched;
    const bool held = state.enabled && state.pressed;
    const bool over = state.enabled && state.hovered;
    const auto offFill = background.interpolatedWith(outline, held ? .36f : over ? .24f : .10f);
    const auto onFill = held ? hover : over ? aqua.interpolatedWith(secondary, .28f) : aqua;
    const auto unavailableFill = on ? background.interpolatedWith(aqua, .27f) : background;
    const auto fill = state.enabled ? (on ? onFill : offFill) : unavailableFill;
    const auto border = state.enabled
        ? (on ? (over || held ? fill.interpolatedWith(text, .42f) : fill)
              : outline.interpolatedWith(text, held ? .70f : over ? .45f : 0.f))
        : (on ? fill.interpolatedWith(outline, .35f) : separator);
    const auto mark = state.enabled ? (on ? background : text)
                                    : text.interpolatedWith(on ? fill : outline, on ? .25f : .45f);

    const auto body = bounds.toFloat();
    const auto pillRadius = body.getHeight() * .5f;
    const auto rounding = form == ToggleForm::track ? pillRadius : radius;
    graphics.setColour(fill);
    graphics.fillRoundedRectangle(body, rounding);
    graphics.setColour(border);
    graphics.drawRoundedRectangle(controlGeometry::insideStroke(bounds, rule), rounding, rule);
    graphics.setColour(mark);
    if (form == ToggleForm::track)
    {
        const auto thumb = std::max(0.f, std::min(body.getWidth(), body.getHeight()) - 2.f * spacing::tight);
        const auto x = on ? body.getRight() - pillRadius : body.getX() + pillRadius;
        graphics.fillEllipse(juce::Rectangle<float>(thumb, thumb).withCentre({x, body.getCentreY()}));
    }
    else if (on)
    {
        juce::Path tick;
        const auto ink = body.reduced(body.getWidth() * .22f, body.getHeight() * .27f);
        tick.startNewSubPath(ink.getX(), ink.getCentreY());
        tick.lineTo(ink.getX() + ink.getWidth() * .35f, ink.getBottom());
        tick.lineTo(ink.getRight(), ink.getY());
        graphics.strokePath(tick, juce::PathStrokeType(controlGeometry::focusStroke));
    }
    if (state.enabled && state.focused)
        paintCornerBrackets(graphics, bounds.expanded(static_cast<int>(controlGeometry::focusOutset
                                                                     + controlGeometry::focusStroke)), aqua);
}

// The form is chosen by the owning composition. Both forms are a single button
// so the label and indicator share focus, accessibility and the invocation.
class BooleanToggle : public InvocationToggle
{
public:
    BooleanToggle(const juce::String& label, ToggleForm presentation,
                  ToggleLabelSide side = ToggleLabelSide::afterIndicator)
        : InvocationToggle(label), form(presentation), labelSide(side)
    {
        setWantsKeyboardFocus(true);
        setMouseClickGrabsKeyboardFocus(true);
        setTooltip(label);
    }

    juce::Rectangle<int> indicatorBounds() const noexcept
    {
        auto row = getLocalBounds();
        auto area = labelSide == ToggleLabelSide::beforeIndicator
            ? row.removeFromRight(indicatorSlotWidth()) : row.removeFromLeft(indicatorSlotWidth());
        area = area.reduced(static_cast<int>(controlGeometry::focusPadding));
        const int width = form == ToggleForm::track ? controlGeometry::switchTrackWidth : checkboxSize;
        const int height = form == ToggleForm::track ? controlGeometry::switchTrackHeight : checkboxSize;
        return area.withSizeKeepingCentre(std::min(width, area.getWidth()), std::min(height, area.getHeight()));
    }

    juce::Rectangle<int> labelBounds() const noexcept
    {
        const auto row = getLocalBounds();
        return labelSide == ToggleLabelSide::beforeIndicator
            ? row.withTrimmedRight(indicatorSlotWidth() + spacing::related)
            : row.withTrimmedLeft(indicatorSlotWidth() + spacing::tight);
    }

    juce::Rectangle<float> focusInkBounds() const noexcept
    {
        return indicatorBounds().toFloat().expanded(controlGeometry::focusOutset + controlGeometry::focusStroke);
    }

    void setUnavailableReason(const juce::String& reason) { setDescription(reason); }

    juce::String getTooltip() override
    {
        return !isEnabled() && getDescription().isNotEmpty()
            ? getDescription() : InvocationToggle::getTooltip();
    }

    void paintButton(juce::Graphics& graphics, bool over, bool down) override
    {
        paintToggleIndicator(graphics, indicatorBounds(), form,
                             {isEnabled(), over, down, isFocusVisible(), getToggleState()});
        const auto role = textStyle(TextRole::controlLabel);
        graphics.setColour(isEnabled() ? role.ink : role.ink.interpolatedWith(outline, .45f));
        graphics.setFont(componentTextFont(*this, TextRole::controlLabel));
        graphics.drawText(getButtonText(), labelBounds(), juce::Justification::centredLeft);
    }

    bool isFocusVisible() const noexcept { return hasKeyboardFocus(false) && visibility->isVisible(); }

    void mouseDown(const juce::MouseEvent& event) override
    {
        visibility->pointerInput();
        if (event.mods.isLeftButtonDown() && prepareInvocation())
            juce::ToggleButton::mouseDown(event);
    }

    bool keyPressed(const juce::KeyPress& key) override
    {
        visibility->keyboardInput();
        const bool space = key.isKeyCode(juce::KeyPress::spaceKey);
        const bool enter = key.isKeyCode(juce::KeyPress::returnKey);
        if (!space && !enter) return juce::ToggleButton::keyPressed(key);
        const bool invoked = activate();
        return space || invoked;
    }

    void focusGained(FocusChangeType cause) override
    {
        if (cause == focusChangedByMouseClick) visibility->pointerInput();
        else if (cause == focusChangedByTabKey) visibility->keyboardInput();
        repaint();
    }

    void focusLost(FocusChangeType) override { repaint(); }

    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override
    {
        class Handler final : public juce::AccessibilityHandler
        {
        public:
            explicit Handler(BooleanToggle& toggle)
                : AccessibilityHandler(toggle, juce::AccessibilityRole::toggleButton,
                      juce::AccessibilityActions{}
                          .addAction(juce::AccessibilityActionType::focus, [&toggle] {
                              if (toggle.isEnabled())
                              {
                                  toggle.visibility->accessibilityInput();
                                  toggle.grabKeyboardFocus();
                              }
                          })
                          .addAction(juce::AccessibilityActionType::press, [&toggle] { toggle.invokeAccessible(); })
                          .addAction(juce::AccessibilityActionType::toggle, [&toggle] { toggle.invokeAccessible(); })),
                  owner(toggle) {}

            juce::AccessibleState getCurrentState() const override
            {
                const auto state = AccessibilityHandler::getCurrentState().withCheckable();
                return owner.getToggleState() ? state.withChecked() : state;
            }

            juce::String getTitle() const override { return owner.getButtonText(); }
            juce::String getHelp() const override { return owner.getTooltip(); }
            juce::String getDescription() const override
            {
                return owner.isEnabled() ? juce::String{} : owner.getDescription();
            }

        private:
            BooleanToggle& owner;
        };
        return std::make_unique<Handler>(*this);
    }

private:
    static constexpr int checkboxSize = 18;

    int indicatorSlotWidth() const noexcept
    {
        return form == ToggleForm::track ? controlGeometry::switchWidth
            : checkboxSize + static_cast<int>(2 * controlGeometry::focusPadding);
    }

    bool prepareInvocation()
    {
        const juce::Component::SafePointer<BooleanToggle> safe(this);
        const auto prepare = beforeInvocation;
        if (prepare) prepare();
        return safe && safe->isEnabled();
    }

    void invokeAccessible()
    {
        visibility->accessibilityInput();
        activate();
    }

    bool activate()
    {
        if (!prepareInvocation()) return false;
        // Match pointer release on the message thread. A later input cannot
        // replace the binding context of a queued keyboard or accessibility edit.
        setToggleState(!getToggleState(), juce::sendNotificationSync);
        return true;
    }

    const ToggleForm form;
    const ToggleLabelSide labelSide;
    juce::SharedResourcePointer<FocusVisibility> visibility;
};
}
