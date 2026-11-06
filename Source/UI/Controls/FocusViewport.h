#pragma once
#include <UI/WorkspaceTheme.h>

namespace m3::ui::controls
{
// Reuse native scrolling; keyboard focus reveals its control and focus margin.
// Coordinates are in the viewed surface, including any content transform.
class FocusViewport final:public FlatViewport
{
public:
    void setFocusRevealEnabled(bool enabled)noexcept{focusReveal=enabled;}
    void focusOfChildComponentChanged(FocusChangeType cause)override
    {
        FlatViewport::focusOfChildComponentChanged(cause);
        if(!focusReveal)return;
        auto* focused=juce::Component::getCurrentlyFocusedComponent();auto* viewed=getViewedComponent();
        if(!focused || !viewed || !viewed->isParentOf(focused))return;
        reveal(*focused);
    }
    void reveal(juce::Component& control)
    {
        auto* viewed=getViewedComponent();if(!viewed || (viewed!=&control && !viewed->isParentOf(&control)))return;
        const auto target=viewed->getLocalArea(&control,control.getLocalBounds()).expanded(static_cast<int>(controlGeometry::focusPadding));
        const auto visible=getViewArea();auto x=visible.getX(),y=visible.getY();
        if(target.getX()<x)x=target.getX();if(target.getRight()>x+visible.getWidth())x=target.getRight()-visible.getWidth();
        if(target.getY()<y)y=target.getY();if(target.getBottom()>y+visible.getHeight())y=target.getBottom()-visible.getHeight();
        setViewPosition(std::max(0,x),std::max(0,y));
    }
private:bool focusReveal=false;
};
}
