#pragma once
#include "ActionButton.h"
#include <optional>
#include <utility>

namespace m3::ui::controls
{
// Capture a command continuation at input time, before dispatch can be queued.
class CapturedActionButton : public ActionButton
{
public:
    using Invocation = std::function<void()>;
    using Capture = std::function<Invocation()>;
    using PressId = uint64_t;
    using ActionButton::ActionButton;

    void bindInvocation(Capture capture)
    {
        captureInvocation = std::move(capture);
        onClick = [this]
        {
            auto invoke = std::exchange(pointerInvocation, {});
            if (invoke) invoke();
        };
    }

    void invokeCommand() override
    {
        if (!captureInvocation) { ActionButton::invokeCommand(); return; }
        if (!isEnabled()) return;
        auto invoke = captureInvocation();
        if (!invoke) return;
        juce::MessageManager::callAsync([safe = juce::Component::SafePointer<CapturedActionButton>(this), invoke = std::move(invoke)]
        {
            if (safe != nullptr && safe->isEnabled()) invoke();
        });
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        if (captureInvocation)
        {
            if (!isEnabled() || !event.mods.isLeftButtonDown()) return;
            if (press && (press->source != event.source || press->down == event.mouseDownTime)) return;
            press = Press{event.source, event.mouseDownTime, ++serial};
            pointerInvocation = captureInvocation();
        }
        ActionButton::mouseDown(event);
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (!captureInvocation || ownsPress(event)) juce::Button::mouseDrag(event);
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        if (!captureInvocation) { juce::Button::mouseUp(event); return; }
        if (!ownsPress(event)) return;
        press.reset();
        if (event.source.getCurrentModifiers().isAnyMouseButtonDown()
            || !getLocalBounds().toFloat().contains(event.position))
            juce::Button::mouseExit(event);
        else
        {
            const juce::Component::SafePointer<CapturedActionButton> safe(this);
            juce::Button::mouseUp(event);
            if (safe == nullptr) return;
        }
        pointerInvocation = {};
    }

    bool hasPress(PressId id) const noexcept { return press && press->id == id; }
    void finishPress(PressId id)
    {
        if (!hasPress(id)) return;
        press.reset();
        pointerInvocation = {};
        setState(juce::Button::buttonNormal);
    }
    bool isPressActive(PressId id) const noexcept
    {
        return isShowing() && hasPress(id) && press->source.isDragging()
            && press->source.getCurrentModifiers().isLeftButtonDown()
            && press->source.getLastMouseDownTime() == press->down;
    }

protected:
    bool hasCapturedInvocation() const noexcept { return static_cast<bool>(captureInvocation); }
    bool ownsPress(const juce::MouseEvent& event) const noexcept
    {
        return press && press->source == event.source && press->down == event.mouseDownTime;
    }
    PressId pointerPress() const noexcept { return press ? press->id : 0; }
    void suppressPointerInvocation() { pointerInvocation = {}; }

private:
    struct Press { juce::MouseInputSource source; juce::Time down; PressId id; };
    Capture captureInvocation;
    Invocation pointerInvocation;
    std::optional<Press> press;
    PressId serial = 0;
};
}
