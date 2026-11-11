#pragma once
#include <UI/ResponsivePresentation.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <DSP/SwellEngine.h>
#include "SwellViewport.h"
#include "EnvelopeHandles.h"
#include "WaveformDisplay.h"
#include "Controls/SliderControl.h"
#include "Controls/WaveformGestures.h"

namespace m3
{
class SwellTimeline final : public juce::Component,public juce::SettableTooltipClient,private juce::ScrollBar::Listener
{
public:
    explicit SwellTimeline(SwellEngine& engine);
    void setData(CaptureSessionSnapshot capture,EditorSnapshot session,SongTime time,bool samplePositionAvailable=true,bool hostPlaying=false);
    void setLoopPlayhead(std::optional<int64_t> sample) noexcept { loopPlayhead = sample; }
    void setSongPlayheadAvailable(bool available) noexcept { songPlayheadAvailable = available; }
    std::optional<int64_t> transportPlayhead() const noexcept;
    SwellEngine::TargetIdentity targetIdentity() const
    {return {data.sessionEpoch,capture.manifest.revision,capture.reference,capture.sourceKind()};}
    std::optional<int64_t> loopInsertionSample() const noexcept;
    void paint(juce::Graphics&) override;
    void resized() override;
    void lookAndFeelChanged()override{invalidateWaveforms();}
    void colourChanged()override{invalidateWaveforms();}
    const ui::WaveformPaintMetrics& waveformPaintMetrics()const noexcept{return waveformLayer.metrics();}
    const ui::WaveformPaintMetrics& wetPaintMetrics()const noexcept{return wetWaveformLayer.metrics();}
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override {endPointerGesture();}
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
    void focusGained(FocusChangeType cause)override{if(cause==focusChangedByTabKey)focus->keyboardInput();repaint();}
    void focusLost(FocusChangeType)override{endPointerGesture();fineDrag=false;snapBypassed=false;}
    void visibilityChanged()override{if(!isVisible()){localPlayhead.reset();endPointerGesture();}}
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override{return ui::controls::waveformAccessibility(*this,*focus);}
    void setGridDivision(GridDivision division);
    GridDivision selectedGrid()const noexcept{return gridDivision;}
    std::optional<HostGrid> makeGrid(GridEvidence* evidence=nullptr)const{return HostGrid::make(gridTiming(),time,capture.manifest.sampleRate,gridDivision,evidence,playheadAvailable,&clockContext);}
    bool gridAvailable()const{return makeGrid().has_value();}
    GridEvidence gridEvidence()const{GridEvidence result;makeGrid(&result);return result;}
    const SongTime& acceptedGridTiming()const{return gridTiming();}
    const juce::String& interactionStatus()const noexcept{return gestureStatus;}
    void setEditingEnabled(bool enabled){if(!enabled){drag=Drag::none;hover.reset();}editingEnabled=enabled;}
    bool beginDrag(juce::Point<float>);void dragTo(juce::Point<float>);
    const SwellViewport& viewport() const noexcept {return view;}
    std::optional<int64_t> localPlayheadSample()const noexcept{return localPlayhead;}
    void setViewport(SwellViewport v) {if(!std::isfinite(v.first) || !std::isfinite(v.last) || !std::isfinite(v.span()) || v.span()<=0)return;if(v.first!=view.first || v.last!=view.last)drag=Drag::none;view=v;updateScroll();repaint();}
    void setInlineOverview(bool enabled){inlineOverview=enabled;resized();repaint();}
    void setResponsivePresentation(bool enabled){if(responsivePresentation!=enabled){responsivePresentation=enabled;resized();repaint();}}
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
    enum class Drag {none,arrival,sourceStart,sourceEnd,audibleStart,audibleEnd,fadeIn,fadeOut,inTension,outTension,move};
    enum class Press {open,handled,blocked};
    enum class PointerGesture {none,position,detailPan,overviewPan};
    void scrollBarMoved(juce::ScrollBar*,double) override;
    void updateScroll();
    void navigationChanged(){drag=Drag::none;updateScroll();repaint();if(onViewportChanged)onViewportChanged();}
    Press beginEdit(juce::Point<float>);
    void endPointerGesture(){pointerGesture=PointerGesture::none;drag=Drag::none;repaint();}
    void positionPlayhead(float x);
    void panFrom(SwellViewport initial,double fraction);
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
    juce::SharedResourcePointer<ui::controls::FocusVisibility> focus;
    std::optional<int64_t> localPlayhead;
    PointerGesture pointerGesture=PointerGesture::none;
    SwellViewport panStart;
    ui::controls::SliderControl waveZoom{{"Height (dB)", "Display zoom for source and wet waveforms. Audio gain stays the same.", false}};
    juce::Label waveZoomLabel;
    juce::Rectangle<float> overview,source,wet;
    juce::String dragId;Drag drag=Drag::none;
    juce::String sourceHover;
    juce::Point<float> dragOrigin;double initialTension=0;int64_t initialPlacement=0,initialSample=0;
    Shape initialEnvelope;
    SwellViewport dragView;
    SongTime dragTime,dragAcceptedTime;
    GridClockContext clockContext,dragClock;
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
    std::optional<int64_t> loopPlayhead;
    juce::Rectangle<float> loopLeadInBounds() const noexcept;
    bool songPlayheadAvailable=true;
    bool inlineOverview=true,responsivePresentation=false;
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
    juce::Rectangle<float> waveformBounds()const noexcept
    {return timeline.responsivePresentation?getLocalBounds().toFloat().withTrimmedLeft(ui::responsiveGeometry::overviewHeadingWidth+ui::responsiveGeometry::contentPadding).reduced(1):getLocalBounds().toFloat().reduced(12,1).withTrimmedTop(50);}
    juce::Rectangle<float> detailBounds()const;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&)override;
    void mouseDrag(const juce::MouseEvent&)override;
    void mouseUp(const juce::MouseEvent&)override;
    void mouseMove(const juce::MouseEvent&)override;
    void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&)override{}
    void visibilityChanged()override{if(!isVisible())panning=false;}
private:
    void paintWaveforms(juce::Graphics&,ui::WaveformPaintMetrics&);
    SwellTimeline& timeline;juce::Label heading;
    SwellViewport panStart;
    uint64_t panEpoch=0;
    bool panning=false;
    ui::WaveformLayer waveformLayer;
    uint64_t waveformRevision=UINT64_MAX;
};
}
