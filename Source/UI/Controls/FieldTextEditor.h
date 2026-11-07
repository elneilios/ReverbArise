#pragma once
#include "FocusVisibility.h"
#include "SelectorPresentation.h"

namespace m3::ui::controls
{
// Opt-in field surface around the ordinary JUCE editing and text interface.
class FieldTextEditor final : public juce::TextEditor
{
public:
    explicit FieldTextEditor(const juce::String& caption)
    {
        setTitle(caption);
        setFont(interFont(textStyle(TextRole::controlValue).height, InterFace::regular));
        // The shared surface paints the body and focus, so JUCE's native
        // background and outlines must leave that surface visible.
        const auto hiddenNativeSurface = juce::Colours::transparentBlack;
        for (const auto colour : {backgroundColourId, outlineColourId, focusedOutlineColourId})
            setColour(colour, hiddenNativeSurface);
        setOpaque(false);
        setIndents(spacing::related, spacing::tight);
        setJustification(juce::Justification::centredLeft);
    }

    bool isFocusVisible() const { return hasKeyboardFocus(false) && visibility->isVisible(); }
    bool isInvalid() const { return invalid; }

    void setInvalid(bool value)
    {
        if (invalid == value) return;
        invalid = value;
        repaint();
    }

    void setFieldDescription(const juce::String& description)
    {
        if (getDescription() == description) return;
        setDescription(description);
        if (isShowing())
            if (auto* handler = getAccessibilityHandler())
                handler->notifyAccessibilityEvent(juce::AccessibilityEvent::valueChanged);
    }

    void paint(juce::Graphics& graphics) override
    {
        SelectorPresentation::surface(graphics, getLocalBounds(),
                                      {isEnabled(), false, false, isFocusVisible(), false}, invalid);
    }

    void focusGained(FocusChangeType cause) override
    {
        if (cause == focusChangedByMouseClick)
            visibility->pointerInput();
        else
            visibility->keyboardInput();
        juce::TextEditor::focusGained(cause);
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        visibility->pointerInput();
        juce::TextEditor::mouseDown(event);
    }

private:
    juce::SharedResourcePointer<FocusVisibility> visibility;
    bool invalid = false;
};
}
