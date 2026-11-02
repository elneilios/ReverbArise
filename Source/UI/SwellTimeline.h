#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <DSP/SwellEngine.h>
#include "SwellViewport.h"
#include "EnvelopeHandles.h"

namespace m3
{
class SwellTimeline final : public juce::Component,private juce::ScrollBar::Listener
{
public:
    explicit SwellTimeline(SwellEngine& engine);
    void setData(CaptureSessionSnapshot capture,EditorSnapshot session,SongTime time,bool samplePositionAvailable=true);
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override {drag=Drag::none;repaint();}
    void mouseMove(const juce::MouseEvent&) override;
    bool keyPressed(const juce::KeyPress&) override;
    std::vector<EnvelopeGrab> grabPoints() const;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&) override;
    void fitSelection();
    void page(int bars);void previous();void next();void zoom(double factor);
    void setWaveZoomDb(double db){waveZoom.setValue(db,juce::sendNotificationSync);}
    double waveZoomDb() const noexcept{return waveZoom.getValue();}
    void setSnap(bool enabled) {snapEnabled=enabled;}
    bool beginDrag(juce::Point<float>);void dragTo(juce::Point<float>);
    const SwellViewport& viewport() const noexcept {return view;}
    void setViewport(SwellViewport v) {view=v;updateScroll();repaint();}
    void setInlineOverview(bool enabled){inlineOverview=enabled;resized();repaint();}
    void navigateOverview(double fraction);
    juce::Rectangle<float> sourceLane() const noexcept {return source;}
    juce::Rectangle<float> wetLane() const noexcept {return wet;}
    std::function<void(int)> onCandidateSelected;
    std::function<void()> onEnvelopeEdited;
private:
    friend class SwellOverview;
    enum class Drag {none,overview,arrival,sourceStart,sourceEnd,audibleStart,audibleEnd,fadeIn,fadeOut,inTension,outTension,move};
    void scrollBarMoved(juce::ScrollBar*,double) override;
    void updateScroll();
    std::pair<double,double> globalBounds() const;
    void drawWave(juce::Graphics&,const Waveform&,juce::Rectangle<float>,juce::Colour,float gain,int64_t placement=0);
    float x(int64_t sample) const {return static_cast<float>(view.pixel(static_cast<double>(sample),source.getX(),source.getWidth()));}
    SwellEngine& engine;CaptureSessionSnapshot capture;EditorSnapshot data;SongTime time;
    SwellViewport view;juce::ScrollBar scrollbar{false};
    juce::Slider waveZoom;
    juce::Label waveZoomLabel;
    juce::Rectangle<float> overview,source,wet;
    juce::String dragId;Drag drag=Drag::none;
    juce::String sourceHover;
    juce::Point<float> dragOrigin;double initialTension=0;int64_t initialFade=0,initialPlacement=0,initialSample=0;
    Shape initialEnvelope;
    std::optional<EnvelopePoint> hover;bool fineDrag=false;
    bool snapEnabled=true,updatingScroll=false;
    bool playheadAvailable=false;
    bool inlineOverview=true;
};
class SwellOverview final : public juce::Component
{
public:
    explicit SwellOverview(SwellTimeline& value):timeline(value){setName("songOverview");}
    juce::Rectangle<float> waveformBounds()const noexcept{return getLocalBounds().toFloat().reduced(12,1).withTrimmedTop(50);}
    juce::Rectangle<float> detailBounds()const;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent& e)override{mouseDrag(e);}
    void mouseDrag(const juce::MouseEvent& e)override{const auto lane=waveformBounds();timeline.navigateOverview((e.position.x-lane.getX())/std::max(1.0f,lane.getWidth()));repaint();}
private:SwellTimeline& timeline;
};
}
