#pragma once
#include "ActionButton.h"
#include "TrackSlider.h"
#include <cmath>

namespace m3::ui::controls
{
struct ZoomControlState
{
    double fraction=0; // 0 = widest, 1 = closest; owner defines the zoom domain.
    bool canZoomIn=false,canZoomOut=false,canFit=false;
};

// Presentation and intents only. No viewport, Fit policy, timing or model state.
class TimelineZoomControls final:public juce::Component
{
public:
    TimelineZoomControls()
    {
        for(auto* child:std::initializer_list<juce::Component*>{&minus,&slider,&plus,&fit})addAndMakeVisible(child);
        minus.setTitle("Zoom out");minus.setTooltip("Zoom out horizontally");
        plus.setTitle("Zoom in");plus.setTooltip("Zoom in horizontally");
        fit.setTitle("Fit waveform");fit.setTooltip("Fit the workspace's available range");
        slider.setTitle("Horizontal zoom");slider.setTooltip("Horizontal zoom: wide to close");
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        slider.setRange(0,1,.001);slider.setWantsKeyboardFocus(true);
        slider.setWheelSteps(.05,.005);
        auto begin=[this]{if(beforeInvocation)beforeInvocation();};
        minus.beforeInvocation=plus.beforeInvocation=fit.beforeInvocation=slider.beforeInvocation=begin;
        minus.onClick=[this]{if(isEnabled() && state.canZoomOut && onZoomStep)onZoomStep(1.25);};
        plus.onClick=[this]{if(isEnabled() && state.canZoomIn && onZoomStep)onZoomStep(.8);};
        fit.onClick=[this]{if(isEnabled() && state.canFit && onFit)onFit();};
        slider.onValueChange=[this]{
            const auto value=slider.getValue();
            if(isEnabled() && slider.isEnabled() && value!=displayedFraction && onZoomFraction)onZoomFraction(value);
        };
        display({});
    }
    std::function<void()> beforeInvocation,onFit;
    std::function<void(double factor)> onZoomStep;
    std::function<void(double fraction)> onZoomFraction;
    void display(ZoomControlState accepted)
    {
        if(!std::isfinite(accepted.fraction)){accepted.fraction=0;accepted.canZoomIn=accepted.canZoomOut=false;}
        state=accepted;state.fraction=std::clamp(state.fraction,0.,1.);
        minus.setEnabled(state.canZoomOut);plus.setEnabled(state.canZoomIn);fit.setEnabled(state.canFit);
        slider.setEnabled(state.canZoomIn || state.canZoomOut);
        slider.setValue(state.fraction,juce::dontSendNotification);
        displayedFraction=slider.getValue(); // Also suppress a queued pre-refresh slider notification.
    }
    static int minimumWidth()noexcept{return 3*controlGeometry::height+controlGeometry::valueWidth+3*spacing::related;}
    void resized()override
    {
        auto area=getLocalBounds();
        minus.setBounds(area.removeFromLeft(std::min(area.getWidth(),controlGeometry::height)));
        area.removeFromLeft(std::min(area.getWidth(),spacing::related));
        fit.setBounds(area.removeFromRight(std::min(area.getWidth(),controlGeometry::valueWidth)));
        area.removeFromRight(std::min(area.getWidth(),spacing::related));
        plus.setBounds(area.removeFromRight(std::min(area.getWidth(),controlGeometry::height)));
        area.removeFromRight(std::min(area.getWidth(),spacing::related));slider.setBounds(area);
    }
    ActionButton& zoomOutButton()noexcept{return minus;}
    ActionButton& zoomInButton()noexcept{return plus;}
    ActionButton& fitButton()noexcept{return fit;}
    InvocationSlider& zoomSlider()noexcept{return slider;}
private:
    ActionButton minus{"-"},plus{"+"},fit{"Fit"};
    TrackSlider slider;
    ZoomControlState state;
    double displayedFraction=0;
};
}
