#pragma once
#include "FieldTextEditor.h"
#include "NumericInputControl.h"

namespace m3::ui::controls
{
// Uses the slider's Label lifecycle and keeps the existing stepper hit regions.
class FieldNumericInput final : public NumericInputControl
{
public:
    FieldNumericInput(const juce::String& caption, const juce::String& description, bool steppers = true)
        : NumericInputControl(steppers)
    {
        setTitle(caption);
        setDescription(description);
        setWantsKeyboardFocus(true);
    }

    void paint(juce::Graphics& graphics) override
    {
        if (!isBeingEdited())
        {
            SelectorPresentation::surface(graphics, valueBounds(),
                                          {isEnabled(), false, false, isFocusVisible(), false}, invalid);
            graphics.setFont(getFont());
            graphics.setColour(textStyle(TextRole::controlValue).ink.withMultipliedAlpha(isEnabled() ? 1.f : .35f));
            graphics.drawText(getText(), commandBodyBounds(valueBounds()), juce::Justification::centred);
        }
        paintStepDivider(graphics);
    }

    bool isFocusVisible() const { return hasKeyboardFocus(false) && visibility->isVisible(); }
    bool isInvalid() const { return invalid; }

    void setFeedback(const juce::String& message, bool isError)
    {
        const bool changed = getDescription() != message;
        setDescription(message);
        invalid = isError;
        if (auto* entry = dynamic_cast<FieldTextEditor*>(getCurrentTextEditor()))
        {
            entry->setFieldDescription(message);
            entry->setInvalid(invalid);
        }
        if (changed && isShowing())
            if (auto* handler = getAccessibilityHandler())
                handler->notifyAccessibilityEvent(juce::AccessibilityEvent::valueChanged);
        repaint();
    }

    void focusGained(FocusChangeType cause) override
    {
        if (cause == focusChangedByMouseClick) visibility->pointerInput();
        else visibility->keyboardInput();
        repaint();
    }

    void focusLost(FocusChangeType) override { repaint(); }

    bool keyPressed(const juce::KeyPress& key) override
    {
        if (isEnabled() && isEditable() && key.isKeyCode(juce::KeyPress::returnKey))
        {
            visibility->keyboardInput();
            showEditor();
            return true;
        }
        return NumericInputControl::keyPressed(key);
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        if (isEditableOnSingleClick() && isEnabled() && valueBounds().toFloat().contains(event.position)
            && !event.mouseWasDraggedSinceMouseDown() && !event.mods.isPopupMenu())
            showEditor();
        if (getCurrentTextEditor() != nullptr) visibility->pointerInput();
    }

    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        NumericInputControl::mouseDoubleClick(event);
        if (getCurrentTextEditor() != nullptr) visibility->pointerInput();
    }

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override
    {
        if (!isEnabled() || !onStep)
        {
            NumericInputControl::mouseWheelMove(event, wheel);
            return;
        }
        const auto amount = std::abs(wheel.deltaX) > std::abs(wheel.deltaY) ? -wheel.deltaX : wheel.deltaY;
        if (amount == 0 || event.mods.isAnyMouseButtonDown() || event.eventTime == lastWheel)
            return;
        lastWheel = event.eventTime;
        const auto direction = (amount > 0) != wheel.isReversed ? 1 : -1;
        const auto begin = onBegin;
        const auto step = onStep;
        const juce::Component::SafePointer<FieldNumericInput> safe(this);
        if (begin) begin();
        if (safe != nullptr && safe->isEnabled()) step(direction);
    }

    juce::TextEditor* createEditorComponent() override
    {
        auto* entry = new FieldTextEditor(getTitle(), FieldTextEditor::Role::embedded);
        entry->setFont(getFont());
        entry->setNumericCharacters("0123456789+.-");
        entry->setFieldDescription(getDescription());
        entry->setInvalid(invalid);
        entry->setTooltip(getTooltip());
        return entry;
    }

    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override
    {
        class Value final : public juce::AccessibilityTextValueInterface
        {
        public:
            explicit Value(FieldNumericInput& field) : owner(field) {}
            bool isReadOnly() const override { return true; }
            juce::String getCurrentValueAsString() const override { return owner.getText(); }
            void setValueAsString(const juce::String&) override {}
        private:
            FieldNumericInput& owner;
        };
        return std::make_unique<juce::AccessibilityHandler>(*this, juce::AccessibilityRole::editableText,
            juce::AccessibilityActions{}
                .addAction(juce::AccessibilityActionType::focus, [this]
                {
                    visibility->accessibilityInput();
                    grabKeyboardFocus();
                })
                .addAction(juce::AccessibilityActionType::press, [this]
                {
                    if (isEnabled() && isEditable())
                    {
                        visibility->accessibilityInput();
                        showEditor();
                    }
                }),
            juce::AccessibilityHandler::Interfaces{std::make_unique<Value>(*this)});
    }

private:
    juce::SharedResourcePointer<FocusVisibility> visibility;
    juce::Time lastWheel;
    bool invalid = false;
};
}
