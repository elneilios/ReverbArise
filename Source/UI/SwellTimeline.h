#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <DSP/SwellEngine.h>
#include "SwellViewport.h"
#include "EnvelopeHandles.h"
#include "WaveformDisplay.h"

namespace m3
{
class SwellTimeline final : public juce::Component,public juce::SettableTooltipClient,private juce::ScrollBar::Listener
{
public:
    explicit SwellTimeline(SwellEngine& engine);
    void setData(CaptureSessionSnapshot capture,EditorSnapshot session,SongTime time,bool samplePositionAvailable=true);
    void paint(juce::Graphics&) override;
    void resized() override;
    void lookAndFeelChanged()override{invalidateWaveforms();}
    void colourChanged()override{invalidateWaveforms();}
    const ui::WaveformPaintMetrics& waveformPaintMetrics()const noexcept{return waveformLayer.metrics();}
    const ui::WaveformPaintMetrics& wetPaintMetrics()const noexcept{return wetWaveformLayer.metrics();}
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override {drag=Drag::none;repaint();}
    void mouseMove(const juce::MouseEvent&) override;
    bool keyPressed(const juce::KeyPress&) override;
    std::vector<EnvelopeGrab> grabPoints() const;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&) override;
    void fitSelection();
    void revealInitialSelection();
    void page(int bars);void previous();void next();void zoom(double factor);
    SwellViewport navigationBounds()const;
    double minimumViewSpan()const;
    double zoomFraction()const;
    void setZoomFraction(double fraction);
    void setWaveZoomDb(double db){waveZoom.setValue(db,juce::sendNotificationSync);}
    double waveZoomDb() const noexcept{return waveZoom.getValue();}
    void setSnap(bool enabled);
    bool requestedSnap()const noexcept{return snapEnabled;}
    bool effectiveSnap()const noexcept{return snapEnabled && !snapBypassed;}
    void setSnapBypassed(bool bypassed){setGestureModifiers(fineDrag,bypassed);}
    void setGestureModifiers(bool fine,bool bypassed);
    void modifierKeysChanged(const juce::ModifierKeys& mods)override{setGestureModifiers(mods.isShiftDown(),mods.isCtrlDown());}
    void focusLost(FocusChangeType)override{drag=Drag::none;fineDrag=false;snapBypassed=false;}
    void setGridDivision(GridDivision division);
    GridDivision selectedGrid()const noexcept{return gridDivision;}
    bool gridAvailable()const{return playheadAvailable && HostGrid::make(gridTiming(),time,capture.manifest.sampleRate,gridDivision).has_value();}
    const juce::String& interactionStatus()const noexcept{return gestureStatus;}
    void setEditingEnabled(bool enabled){if(!enabled){drag=Drag::none;hover.reset();}editingEnabled=enabled;}
    bool beginDrag(juce::Point<float>);void dragTo(juce::Point<float>);
    const SwellViewport& viewport() const noexcept {return view;}
    void setViewport(SwellViewport v) {if(!std::isfinite(v.first) || !std::isfinite(v.last) || !std::isfinite(v.span()) || v.span()<=0)return;if(v.first!=view.first || v.last!=view.last)drag=Drag::none;view=v;updateScroll();repaint();}
    void setInlineOverview(bool enabled){inlineOverview=enabled;resized();repaint();}
    void navigateOverview(double fraction);
    juce::Rectangle<float> sourceLane() const noexcept {return source;}
    juce::Rectangle<float> wetLane() const noexcept {return wet;}
    juce::Rectangle<int> musicalRulerBounds()const noexcept{return ruler;}
    std::function<void(int)> onCandidateSelected;
    std::function<void()> onEnvelopeEdited;
    std::function<void()> onViewportChanged;
private:
    friend class SwellOverview;
    // A dry-only session pins its fixed mapping when the first target is
    // accepted. Bootstrap that insertion only; retain established-map guards.
    const SongTime& gridTiming()const{return !data.session.timing.valid && data.session.swells.empty()?time:data.session.timing;}
    enum class Drag {none,overview,arrival,sourceStart,sourceEnd,audibleStart,audibleEnd,fadeIn,fadeOut,inTension,outTension,move};
    void scrollBarMoved(juce::ScrollBar*,double) override;
    void updateScroll();
    void navigationChanged(){if(drag!=Drag::overview)drag=Drag::none;updateScroll();repaint();if(onViewportChanged)onViewportChanged();}
    bool prepareDrag(juce::Point<float>);
    void rebaseDrag();
    void gestureMessage(const juce::String& text){if(gestureStatus!=text){gestureStatus=text;setDescription(text);repaint();}}
    void constrainNavigation();
    double navigationAnchor()const;
    std::pair<double,double> globalBounds() const;
    void paintWaveforms(juce::Graphics&,ui::WaveformPaintMetrics&,bool wetOnly);
    void invalidateWaveforms(){waveformLayer.invalidate();wetWaveformLayer.invalidate();++waveformRevision;}
    void drawWave(juce::Graphics&,const std::vector<CapturePeak>&,juce::Rectangle<float>,juce::Colour,float gain,ui::WaveformPaintMetrics&,int64_t placement=0);
    float x(int64_t sample) const {return static_cast<float>(view.pixel(static_cast<double>(sample),source.getX(),source.getWidth()));}
    SwellEngine& engine;CaptureSessionSnapshot capture;EditorSnapshot data;SongTime time;
    SwellViewport view;juce::ScrollBar scrollbar{false};juce::Rectangle<int> ruler;
    juce::Slider waveZoom;
    juce::Label waveZoomLabel;
    juce::Rectangle<float> overview,source,wet;
    juce::String dragId;Drag drag=Drag::none;
    juce::String sourceHover;
    juce::Point<float> dragOrigin;double initialTension=0;int64_t initialPlacement=0,initialSample=0;
    Shape initialEnvelope;
    SwellViewport dragView;
    SongTime dragTime,dragAcceptedTime;
    std::optional<HostGrid> dragGrid;
    uint64_t dragEpoch=0,dragInput=0;
    std::vector<juce::String> dragSelection;
    juce::Point<float> lastPointer;
    std::shared_ptr<const Waveform> dragRaw;
    juce::String gestureStatus,dragRender;
    bool dragSampleAvailable=false,dragFrozen=false;
    std::optional<EnvelopePoint> hover;bool fineDrag=false;
    bool snapEnabled=true,snapBypassed=false,updatingScroll=false;
    GridDivision gridDivision=GridDivision::quarterBeat;
    bool playheadAvailable=false;
    bool inlineOverview=true;
    bool editingEnabled=true;
    ui::WaveformLayer waveformLayer,wetWaveformLayer;
    uint64_t waveformRevision=0;
};
class SwellOverview final : public juce::Component,public juce::SettableTooltipClient
{
public:
    explicit SwellOverview(SwellTimeline& value);
    void resized()override;
    void lookAndFeelChanged()override{waveformLayer.invalidate();}
    void colourChanged()override{waveformLayer.invalidate();}
    const ui::WaveformPaintMetrics& waveformPaintMetrics()const noexcept{return waveformLayer.metrics();}
    juce::Rectangle<float> waveformBounds()const noexcept{return getLocalBounds().toFloat().reduced(12,1).withTrimmedTop(50);}
    juce::Rectangle<float> detailBounds()const;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent& e)override{mouseDrag(e);}
    void mouseDrag(const juce::MouseEvent& e)override{const auto lane=waveformBounds();timeline.navigateOverview((e.position.x-lane.getX())/std::max(1.0f,lane.getWidth()));repaint();}
private:
    void paintWaveforms(juce::Graphics&,ui::WaveformPaintMetrics&);
    SwellTimeline& timeline;juce::Label heading;
    ui::WaveformLayer waveformLayer;
    uint64_t waveformRevision=UINT64_MAX;
};
}
