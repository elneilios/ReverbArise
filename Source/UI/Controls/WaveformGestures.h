#pragma once
#include <UI/SwellViewport.h>
#include <UI/WorkspaceTheme.h>
#include "FocusVisibility.h"

namespace m3::ui::controls
{
inline bool waveformButton(juce::ModifierKeys keys, int button) noexcept
{
    constexpr auto buttons = juce::ModifierKeys::leftButtonModifier
        | juce::ModifierKeys::rightButtonModifier | juce::ModifierKeys::middleButtonModifier;
    return (keys.getRawFlags() & buttons) == button;
}

inline bool zoomWaveform(SwellViewport& view, float delta, double anchor,
                         const SwellViewport& limits, double minimum) noexcept
{
    return delta != 0 && view.zoomBounded(std::pow(1.25, -4.0 * delta), anchor, limits, minimum);
}

inline void paintWaveformFocus(juce::Graphics& g, const juce::Component& owner,
                              juce::Rectangle<int> bounds, const FocusVisibility& focus)
{
    if (owner.hasKeyboardFocus(false) && focus.isVisible())
        paintCornerBrackets(g, bounds, aqua);
}

inline std::unique_ptr<juce::AccessibilityHandler> waveformAccessibility(
    juce::Component& owner, FocusVisibility& focus)
{
    return std::make_unique<juce::AccessibilityHandler>(owner, juce::AccessibilityRole::group,
        juce::AccessibilityActions{}.addAction(juce::AccessibilityActionType::focus,
            [&owner, &focus] { focus.accessibilityInput(); owner.grabKeyboardFocus(); }));
}
}
