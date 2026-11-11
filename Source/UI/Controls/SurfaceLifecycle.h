#pragma once
#include "FocusVisibility.h"
#include <memory>
#include <optional>

namespace m3::ui::controls
{
// UI ownership only: these continuations never authorize a model operation.
struct SurfaceFocusReturn
{
    juce::Component::SafePointer<juce::Component> opener, fallback;
    bool keyboard=true;

    static SurfaceFocusReturn capture(juce::Component& opening,juce::Component* owner=nullptr)
    {
        juce::SharedResourcePointer<FocusVisibility> input;
        return {&opening,owner?owner:opening.getTopLevelComponent(),input->isVisible()};
    }
    void useCurrentInput(){juce::SharedResourcePointer<FocusVisibility> input;keyboard=input->isVisible();}
    void post(std::function<bool()> current={},std::function<void(bool)> restore={})const
    {
        juce::MessageManager::callAsync([request=*this,current=std::move(current),restore=std::move(restore)]
        {
            if((current && !current()) || !juce::Process::isForegroundProcess() || juce::Component::getCurrentlyModalComponent()!=nullptr)return;
            const auto eligible=[](juce::Component* c){return c && c->isShowing() && c->isEnabled() && !c->isCurrentlyBlockedByAnotherModalComponent();};
            auto* target=eligible(request.opener)?request.opener.getComponent():eligible(request.fallback)?request.fallback.getComponent():nullptr;
            if(!target)return;
            auto* focused=juce::Component::getCurrentlyFocusedComponent();auto* activePeer=focused?focused->getPeer():nullptr;
            if(focused && focused->isShowing() && activePeer && activePeer->isFocused()
                && activePeer!=target->getPeer() && (!request.fallback || activePeer!=request.fallback->getPeer()))return;
            if(restore)restore(request.keyboard);
            else target->grabKeyboardFocus();
            juce::SharedResourcePointer<FocusVisibility> input;
            if(request.keyboard)input->keyboardInput();else input->pointerInput();
        });
    }
};

// One pending native-picker invocation per owner, with weak completion lifetime.
class SurfaceInvocation final
{
    struct State{bool pending=false;};
public:
    SurfaceInvocation()=default;
    SurfaceInvocation(const SurfaceInvocation&)=delete;
    SurfaceInvocation& operator=(const SurfaceInvocation&)=delete;
    class Ticket
    {
    public:
        bool complete(std::function<bool()> mayReturn={})const
        {
            const auto owner=state.lock();if(!owner || !owner->pending)return false;
            owner->pending=false;focus.post([weak=state,mayReturn=std::move(mayReturn)]{return !weak.expired() && (!mayReturn || mayReturn());});return true;
        }
    private:
        friend class SurfaceInvocation;
        std::weak_ptr<State> state;
        SurfaceFocusReturn focus;
    };
    std::optional<Ticket> begin(juce::Component& opener,juce::Component* fallback=nullptr)
    {
        if(state->pending)return {};
        state=std::make_shared<State>();state->pending=true;
        Ticket ticket;ticket.state=state;ticket.focus=SurfaceFocusReturn::capture(opener,fallback);return ticket;
    }
    bool pending()const noexcept{return state->pending;}
    void invalidate(){state=std::make_shared<State>();}
private:
    std::shared_ptr<State> state=std::make_shared<State>();
};

// Observe Escape before a focused editor consumes it, preserving its cancel hook.
class SurfaceEscape final:private juce::FocusChangeListener,private juce::KeyListener
{
public:
    SurfaceEscape(juce::Component& component,std::function<void()> close):root(component),dismiss(std::move(close))
    {juce::Desktop::getInstance().addFocusChangeListener(this);globalFocusChanged(juce::Component::getCurrentlyFocusedComponent());}
    ~SurfaceEscape()override
    {if(focused)focused->removeKeyListener(this);juce::Desktop::getInstance().removeFocusChangeListener(this);}
private:
    void globalFocusChanged(juce::Component* next)override
    {
        if(focused)focused->removeKeyListener(this);focused=nullptr;
        if(next && (&root==next || root.isParentOf(next))){focused=next;next->addKeyListener(this);}
    }
public:
    bool keyPressed(const juce::KeyPress& key,juce::Component* target)override
    {
        if(!key.isKeyCode(juce::KeyPress::escapeKey) || !root.isShowing() || root.isCurrentlyBlockedByAnotherModalComponent())return false;
        // This listener can consume Escape before the shared input observer sees it.
        juce::SharedResourcePointer<FocusVisibility> input;
        input->keyboardInput();
        const juce::Component::SafePointer<juce::Component> safe(&root);
        const auto close=dismiss;
        if(target && (&root==target || root.isParentOf(target)))
            if(auto* editor=dynamic_cast<juce::TextEditor*>(target))if(editor->onEscapeKey)editor->onEscapeKey();
        if(safe)close();return true;
    }
private:
    juce::Component& root;
    std::function<void()> dismiss;
    juce::Component::SafePointer<juce::Component> focused;
};
}
