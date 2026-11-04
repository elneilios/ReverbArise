#pragma once
#include <UI/WorkspaceTheme.h>
#include <UI/SwellViewport.h>
#include <limits>
namespace m3::ui::controls
{
// Read-only gestures; owners provide accepted bounds, never a DAW seek callback.
class TimelineNavigation final:public juce::Component,public juce::SettableTooltipClient
{
public:
    enum class PanGesture {leftOrShiftLeft,rightDrag};
    TimelineNavigation(){setWantsKeyboardFocus(true);setMouseClickGrabsKeyboardFocus(true);
        setTitle("Read-only waveform navigation");setTooltip("Wheel: zoom at pointer. Drag or Shift-wheel: pan. +/-: zoom. Arrows: pan. Home or double-click: Fit.");}
    std::function<void()> onChange;
    std::function<void(int64_t)> onPosition;
    void cancelGesture(){positioning=false;panning=false;}
    void setPanGesture(PanGesture gesture){cancelGesture();panGesture=gesture;}
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
    void mouseDown(const juce::MouseEvent& e)override{grabKeyboardFocus();down=view;
        panning=isPanGesture(e.mods);
        positioning=e.mods.isLeftButtonDown() && !e.mods.isRightButtonDown() && !e.mods.isShiftDown() && !panning && onPosition!=nullptr && positioningEnabled;
        if(positioning && e.getNumberOfClicks()==1)position(e.position.x);}
    void mouseDrag(const juce::MouseEvent& e)override
    {if(getWidth()<=0)return;if(positioning){if(e.mods.isLeftButtonDown() && !e.mods.isRightButtonDown())position(e.position.x);return;}
        if(!panning || !isPanGesture(e.mods))return;view=down;view.shift(-static_cast<double>(e.getDistanceFromDragStartX())/getWidth());clamp();changed();}
    void mouseUp(const juce::MouseEvent&)override{cancelGesture();}
    void mouseDoubleClick(const juce::MouseEvent& e)override{if(e.mods.isLeftButtonDown() && !e.mods.isRightButtonDown())fit();}
    void mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel)override
    {if(e.mods.isShiftDown() || std::abs(wheel.deltaX)>std::abs(wheel.deltaY))pan(-2*(std::abs(wheel.deltaX)>std::abs(wheel.deltaY)?wheel.deltaX:wheel.deltaY));
        else zoom(std::pow(1.25,-wheel.deltaY*4),getWidth()>0?e.position.x/getWidth():.5);}
    bool keyPressed(const juce::KeyPress& key)override
    {
        if(key==juce::KeyPress::homeKey || key.getTextCharacter()=='f' || key.getTextCharacter()=='F'){fit();return true;}
        if(key==juce::KeyPress::leftKey){pan(-.1);return true;}if(key==juce::KeyPress::rightKey){pan(.1);return true;}
        if(key.getTextCharacter()=='+' || key.getTextCharacter()=='='){zoom(.8);return true;}
        if(key.getTextCharacter()=='-'){zoom(1.25);return true;}return false;
    }
    void focusGained(FocusChangeType)override{repaint();}void focusLost(FocusChangeType)override{repaint();}
    void paint(juce::Graphics& g)override{if(hasKeyboardFocus(false)){g.setColour(aqua);g.drawRoundedRectangle(controlGeometry::insideStroke(getLocalBounds(),controlGeometry::focusStroke),radius,controlGeometry::focusStroke);}}
private:
    bool isPanGesture(juce::ModifierKeys keys)const{return panGesture==PanGesture::rightDrag?
        keys.isRightButtonDown() && !keys.isLeftButtonDown():keys.isLeftButtonDown() && (keys.isShiftDown() || onPosition==nullptr);}
    void position(float x){if(positioningEnabled && onPosition && getWidth()>0)onPosition(view.sample(std::clamp(x,0.f,static_cast<float>(getWidth())),0,getWidth()));}
    void changed(){repaint();if(onChange)onChange();}
    void clamp(){const auto span=std::clamp(view.span(),std::min(minimum,limits.span()),limits.span());
        view.first=std::clamp(view.first,limits.first,std::max(limits.first,limits.last-span));view.last=view.first+span;}
    SwellViewport view,down,fitView,limits;double minimum=1;
    bool positioningEnabled=false,positioning=false,panning=false;
    PanGesture panGesture=PanGesture::leftOrShiftLeft;
};
}
