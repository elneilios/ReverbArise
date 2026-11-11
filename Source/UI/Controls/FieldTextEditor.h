#pragma once
#include "FocusVisibility.h"
#include "SelectorPresentation.h"

namespace m3::ui::controls
{
// Opt-in field surface around the ordinary JUCE editing and text interface.
class FieldTextEditor : public juce::TextEditor
{
public:
    enum class Role { editable, embedded, readOnly };

    explicit FieldTextEditor(const juce::String& caption = {}, Role role = Role::editable)
    {
        setTitle(caption);
        setFont(interFont(textStyle(TextRole::controlValue).height, InterFace::regular));
        // The shared surface paints the body and focus, so JUCE's native
        // background and outlines must leave that surface visible.
        const auto hiddenNativeSurface = juce::Colours::transparentBlack;
        for (const auto colour : {backgroundColourId, outlineColourId, focusedOutlineColourId})
            setColour(colour, hiddenNativeSurface);
        setOpaque(false);
        const bool embedded = role == Role::embedded;
        setIndents(embedded ? spacing::tight : spacing::related, embedded ? 0 : spacing::tight);
        setJustification(embedded ? juce::Justification::centred : juce::Justification::centredLeft);
        setReadOnly(role == Role::readOnly);
    }

    bool isFocusVisible() const { return hasKeyboardFocus(false) && visibility->isVisible(); }
    bool isInvalid() const { return invalid; }

    void setNumericCharacters(juce::String characters)
    {
        numericCharacters = std::move(characters);
    }

    void insertTextAtCaret(const juce::String& draft) override
    {
        if (isReadOnly()) return;
        if (numericCharacters.isEmpty() || draft.isEmpty())
        {
            juce::TextEditor::insertTextAtCaret(draft);
            return;
        }

        const auto filtered = draft.retainCharacters(numericCharacters);
        // An entirely blocked key or paste must not erase the selection.
        if (filtered.isNotEmpty()) juce::TextEditor::insertTextAtCaret(filtered);
    }

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
        if (isReadOnly() && isEnabled())
        {
            graphics.setColour(SelectorPresentation::raised());
            const auto body = controlGeometry::insideStroke(commandBodyBounds(getLocalBounds()), rule);
            graphics.fillRoundedRectangle(body.reduced(rule), radius);
        }
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
    juce::String numericCharacters;
    bool invalid = false;
};
}
