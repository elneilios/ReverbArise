#pragma once
#include "WaveformGestures.h"
#include <limits>
namespace m3::ui::controls
{
// Read-only gestures; owners provide accepted bounds, never a DAW seek callback.
class TimelineNavigation final:public juce::Component,public juce::SettableTooltipClient
{
public:
    TimelineNavigation(){setWantsKeyboardFocus(true);setMouseClickGrabsKeyboardFocus(false);
        setTitle("Export preview waveform");setTooltip("Left-click or drag: position ready preview. Wheel: zoom at pointer. Middle-drag: pan. Home: Fit.");}
    std::function<void()> onChange;
    std::function<void(int64_t)> onPosition;
    void cancelGesture(){positioning=false;panning=false;}
    void setPositioningEnabled(bool enabled){if(positioningEnabled && !enabled)cancelGesture();positioningEnabled=enabled;}
    const SwellViewport& viewport()const noexcept{return view;}
    double minimumSpan()const noexcept{return std::min(minimum,limits.span());}
    double maximumSpan()const noexcept{return limits.span();}
    void setRange(double audibleFirst,double audibleLast,double fileFirst,double fileLast,double rate,bool reset=true)
    {
        cancelGesture();
        if(!std::isfinite(audibleFirst) || !std::isfinite(audibleLast) || !std::isfinite(fileFirst) || !std::isfinite(fileLast))return;
        if(audibleLast<=audibleFirst)audibleLast=std::nextafter(audibleFirst,std::numeric_limits<double>::infinity());
        if(fileLast<=fileFirst)fileLast=std::nextafter(fileFirst,std::numeric_limits<double>::infinity());
        fitView.fit(audibleFirst,audibleLast);limits.fit(std::min(audibleFirst,fileFirst),std::max(audibleLast,fileLast));
        const auto pad=std::max(1.,std::abs(fileLast)*std::numeric_limits<double>::epsilon()*16);
        fitView.first=std::min(fitView.first,audibleFirst-pad);fitView.last=std::max(fitView.last,audibleLast+pad);
        limits.first=std::min(limits.first,fitView.first);limits.last=std::max(limits.last,fitView.last);
        minimum=std::max({1.,rate*.01,std::abs(fileLast)*std::numeric_limits<double>::epsilon()*16});if(reset)fit();else{clamp();changed();}
    }
    void fit(){view=fitView;clamp();changed();}
    void zoom(double factor,double anchor=.5){view.zoom(factor,std::clamp(anchor,0.,1.));clamp();changed();}
    void pan(double fraction){view.shift(fraction);clamp();changed();}
    void mouseDown(const juce::MouseEvent& e)override
    {
        cancelGesture();
        panning=waveformButton(e.mods,juce::ModifierKeys::middleButtonModifier);
        const auto left=waveformButton(e.mods,juce::ModifierKeys::leftButtonModifier);
        if(!panning && !left)return;
        focus->pointerInput();
        if(isShowing())grabKeyboardFocus();
        down=view;
        positioning=left && onPosition && positioningEnabled;
        if(positioning)position(e.position.x);
    }
    void mouseDrag(const juce::MouseEvent& e)override
    {
        if(getWidth()<=0)return;
        if(positioning && waveformButton(e.mods,juce::ModifierKeys::leftButtonModifier))position(e.position.x);
        if(!panning || !waveformButton(e.mods,juce::ModifierKeys::middleButtonModifier))return;
        view=down;view.shift(-static_cast<double>(e.getDistanceFromDragStartX())/getWidth());clamp();changed();
    }
    void mouseUp(const juce::MouseEvent&)override{cancelGesture();}
    void mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel)override
    {if(getWidth()>0 && zoomWaveform(view,wheel.deltaY,e.position.x/getWidth(),limits,minimum))changed();}
    bool keyPressed(const juce::KeyPress& key)override
    {
        focus->keyboardInput();
        if(key==juce::KeyPress::homeKey || key.getTextCharacter()=='f' || key.getTextCharacter()=='F'){fit();return true;}
        if(key==juce::KeyPress::leftKey){pan(-.1);return true;}if(key==juce::KeyPress::rightKey){pan(.1);return true;}
        if(key.getTextCharacter()=='+' || key.getTextCharacter()=='='){zoom(.8);return true;}
        if(key.getTextCharacter()=='-'){zoom(1.25);return true;}return false;
    }
    void focusGained(FocusChangeType cause)override{if(cause==focusChangedByTabKey)focus->keyboardInput();repaint();}
    void focusLost(FocusChangeType)override{cancelGesture();repaint();}
    void visibilityChanged()override{if(!isVisible())cancelGesture();}
    void paint(juce::Graphics& g)override{paintWaveformFocus(g,*this,getLocalBounds(),*focus);}
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override{return waveformAccessibility(*this,*focus);}
private:
    juce::SharedResourcePointer<FocusVisibility> focus;
    void position(float x){if(positioningEnabled && onPosition && getWidth()>0)onPosition(view.sample(std::clamp(x,0.f,static_cast<float>(getWidth())),0,getWidth()));}
    void changed(){repaint();if(onChange)onChange();}
    void clamp(){view.constrain(limits,minimum);}
    SwellViewport view,down,fitView,limits;double minimum=1;
    bool positioningEnabled=false,positioning=false,panning=false;
};
}
