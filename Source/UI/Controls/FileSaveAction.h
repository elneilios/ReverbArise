#pragma once
#include "ActionButton.h"
#include <optional>
#include <utility>

namespace m3::ui::controls
{
// A file action captures a continuation for each input before it can be queued.
class FileSaveAction : public ActionButton
{
public:
    using Save = std::function<void()>;
    using CaptureSave = std::function<Save()>;
    using PressId = uint64_t;
    using ActionButton::ActionButton;

    void bindSave(CaptureSave capture)
    {
        captureSave = std::move(capture);
        onClick = [this]
        {
            auto save = std::exchange(pointerSave, {});
            if (save) save();
        };
    }

    void invokeCommand() override
    {
        if (!captureSave) { ActionButton::invokeCommand(); return; }
        if (!isEnabled()) return;
        auto save = captureSave();
        if (!save) return;
        juce::MessageManager::callAsync([safe = juce::Component::SafePointer<FileSaveAction>(this), save = std::move(save)]
        {
            if (safe != nullptr && safe->isEnabled()) save();
        });
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        if (captureSave)
        {
            if (!isEnabled() || !event.mods.isLeftButtonDown()) return;
            if (press && (press->source != event.source || press->down == event.mouseDownTime)) return;
            press = Press{event.source, event.mouseDownTime, ++serial};
            pointerSave = captureSave();
        }
        ActionButton::mouseDown(event);
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (!captureSave || ownsPress(event)) juce::Button::mouseDrag(event);
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        if (!captureSave) { juce::Button::mouseUp(event); return; }
        if (!ownsPress(event)) return;
        press.reset();
        if (event.source.getCurrentModifiers().isAnyMouseButtonDown()
            || !getLocalBounds().toFloat().contains(event.position))
            juce::Button::mouseExit(event);
        else
        {
            const juce::Component::SafePointer<FileSaveAction> safe(this);
            juce::Button::mouseUp(event);
            if (safe == nullptr) return;
        }
        pointerSave = {};
    }

    bool hasPress(PressId id) const noexcept { return press && press->id == id; }
    void finishPress(PressId id)
    {
        if (!hasPress(id)) return;
        press.reset();
        pointerSave = {};
        setState(juce::Button::buttonNormal);
    }
    bool isPressActive(PressId id) const noexcept
    {
        return isShowing() && hasPress(id) && press->source.isDragging()
            && press->source.getCurrentModifiers().isLeftButtonDown()
            && press->source.getLastMouseDownTime() == press->down;
    }

protected:
    bool usesBoundSave() const noexcept { return static_cast<bool>(captureSave); }
    bool ownsPress(const juce::MouseEvent& event) const noexcept
    {
        return press && press->source == event.source && press->down == event.mouseDownTime;
    }
    PressId pointerPress() const noexcept { return press ? press->id : 0; }
    void suppressPointerSave() { pointerSave = {}; }

private:
    struct Press { juce::MouseInputSource source; juce::Time down; PressId id; };
    CaptureSave captureSave;
    Save pointerSave;
    std::optional<Press> press;
    PressId serial = 0;
};
}
