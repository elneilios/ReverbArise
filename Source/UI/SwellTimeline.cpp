#include "SwellTimeline.h"
#include "TimingEdit.h"
#include "SongBarRuler.h"
#include "Controls/MusicalTimelineRuler.h"
#include "WorkspaceTheme.h"
#include "InitialPerformView.h"
#include <limits>

namespace m3
{
using ui::controls::waveformButton;
namespace
{
bool sameCaptureDisplay(const CaptureSessionSnapshot& a,const CaptureSessionSnapshot& b)
{
    // CaptureSession is the production overview publisher: changed PCM advances
    // the accepted manifest/reference (including prepared-pass acceptance).
    // Restoring an unchanged immutable/checksummed asset reproduces its peaks;
    // failed asset reads change availability. Do not scan every peak per refresh.
    // waveform_paint_test exercises these producer paths through this cache.
    if(a.reference!=b.reference || a.manifest.revision!=b.manifest.revision || a.manifest.projectId!=b.manifest.projectId
        || a.manifest.instanceId!=b.manifest.instanceId || a.manifest.sampleRate!=b.manifest.sampleRate
        || a.unavailable!=b.unavailable || a.overview.size()!=b.overview.size() || a.manifest.ranges.size()!=b.manifest.ranges.size())return false;
    const auto sameRange=[](const auto& x,const auto& y){return x.startSample==y.startSample && x.frameCount==y.frameCount
        && x.kind==y.kind && x.revision==y.revision && x.assetPath==y.assetPath && x.checksum==y.checksum;};
    for(size_t i=0;i<a.manifest.ranges.size();++i)if(!sameRange(a.manifest.ranges[i],b.manifest.ranges[i]))return false;
    for(size_t i=0;i<a.overview.size();++i)if(!sameRange(a.overview[i].range,b.overview[i].range)
        || a.overview[i].available!=b.overview[i].available || a.overview[i].peaks.size()!=b.overview[i].peaks.size())return false;
    return true;
}
bool sameWetDisplay(const EditorSnapshot& a,const EditorSnapshot& b)
{
    if(a.sessionEpoch!=b.sessionEpoch || a.session.primary!=b.session.primary || a.raw!=b.raw || a.shaped!=b.shaped
        || a.sourceDetail!=b.sourceDetail || a.wetSummaries!=b.wetSummaries || a.loopLeadIn!=b.loopLeadIn
        || a.session.swells.size()!=b.session.swells.size())return false;
    for(size_t i=0;i<a.session.swells.size();++i)
    {
        const auto& x=a.session.swells[i];const auto& y=b.session.swells[i];
        if(x.id!=y.id || x.arrival!=y.arrival || x.sourceStart!=y.sourceStart || x.sourceEnd!=y.sourceEnd || x.placement!=y.placement
            || x.shape!=y.shape || x.enabled!=y.enabled || x.muted!=y.muted || x.phase!=y.phase
            || x.inputGeneration!=y.inputGeneration || x.shapeGeneration!=y.shapeGeneration
            || x.render.content!=y.render.content || x.render.path!=y.render.path || x.render.start!=y.render.start || x.render.frames!=y.render.frames)return false;
    }
    return true;
}
}
SwellTimeline::SwellTimeline(SwellEngine& e):engine(e)
{
    setTooltip("Left-click or drag open waveform to position the local playhead. Wheel zooms at the pointer. Middle-drag pans.");
    setWantsKeyboardFocus(true);setMouseClickGrabsKeyboardFocus(false);
    setName("swellTimeline");scrollbar.addListener(this);addAndMakeVisible(scrollbar);setMouseCursor(juce::MouseCursor::CrosshairCursor);
    waveZoom.setName("waveZoom");waveZoom.setSliderStyle(juce::Slider::LinearHorizontal);
    waveZoom.setRange(-24,48,1);waveZoom.setWheelSteps(3,1);waveZoom.setValue(0,juce::dontSendNotification);
    waveZoom.setTextBoxStyle(juce::Slider::TextBoxRight,false,60,20);waveZoom.setTextValueSuffix(" dB");
    waveZoom.setDoubleClickReturnValue(true,0);waveZoom.setTooltip("Display zoom for both source and audible wet. Does not change audio gain. Double-click to reset.");
    waveZoom.onValueChange=[this]{invalidateWaveforms();repaint();};addAndMakeVisible(waveZoom);
    waveZoomLabel.setTooltip(waveZoom.getTooltip());waveZoomLabel.setText("Height",juce::dontSendNotification);ui::applyTextRole(waveZoomLabel,ui::TextRole::controlLabel);addAndMakeVisible(waveZoomLabel);
}
void SwellTimeline::setData(CaptureSessionSnapshot c,EditorSnapshot s,SongTime t,bool available,bool hostPlaying)
{
    const auto nextClock=gridClockContext(s.gridClock,hostPlaying);
    if(s.sessionEpoch!=data.sessionEpoch){localPlayhead.reset();pointerGesture=PointerGesture::none;}
    if(drag!=Drag::none)
    {
        const bool wetDrag=drag!=Drag::arrival && drag!=Drag::sourceStart && drag!=Drag::sourceEnd;
        const auto owner=find(s.session,dragId);
        const bool timingChanged=!dragClock.sameAuthority(nextClock) || t.valid!=dragTime.valid || (t.valid && !transportMappingMatches(dragTime,t))
            || s.session.timing.valid!=dragAcceptedTime.valid || (s.session.timing.valid && !transportMappingMatches(dragAcceptedTime,s.session.timing));
        const bool loopChanged=(capture.manifest.loop || c.manifest.loop) && capture.reference!=c.reference;
        if(loopChanged || s.sessionEpoch!=dragEpoch || s.session.primary!=dragId || s.session.selected!=dragSelection || !owner || (owner && owner->frozen!=dragFrozen) || timingChanged
            || available!=dragSampleAvailable || (wetDrag && (owner->inputGeneration!=dragInput || owner->render.content!=dragRender
                || !s.raw || !dragRaw || s.raw->start!=dragRaw->start || s.raw->end!=dragRaw->end)))
        {drag=Drag::none;timingDraft.reset();gestureMessage("Gesture cancelled: selection, source, mode or fixed timing changed");}
    }
    if(!sameCaptureDisplay(capture,c) || !sameWetDisplay(data,s) || time.valid!=t.valid || time.rate!=t.rate
        || time.bpm!=t.bpm || time.numerator!=t.numerator || time.denominator!=t.denominator)invalidateWaveforms();
    capture=std::move(c);data=std::move(s);time=t;playheadAvailable=available;clockContext=nextClock;
    scrollbar.setVisible(!capture.manifest.ranges.empty() || !data.session.swells.empty());updateScroll();repaint();
}
void SwellTimeline::setSnap(bool enabled){if(enabled!=snapEnabled){snapEnabled=enabled;rebaseDrag();}}
void SwellTimeline::setGridDivision(GridDivision division)
{
    if(division==gridDivision || !std::any_of(gridChoices.begin(),gridChoices.end(),[&](const auto& c){return c.division==division;}))return;
    gridDivision=division;if(drag!=Drag::none){drag=Drag::none;gestureMessage("Grid changed; start a new gesture");}
}
void SwellTimeline::setGestureModifiers(bool fine,bool bypassed)
{if(fineDrag!=fine || snapBypassed!=bypassed){rebaseDrag();fineDrag=fine;snapBypassed=bypassed;}}
void SwellTimeline::rebaseDrag()
{
    if(drag==Drag::none)return;
    const auto w=timingDraft?&timingDraft->swell:find(data.session,dragId);if(!w){drag=Drag::none;return;}
    dragOrigin=lastPointer;initialEnvelope=w->shape;initialPlacement=w->placement;
    initialTension=drag==Drag::inTension?w->shape.inTension:w->shape.outTension;
    if(drag==Drag::arrival)initialSample=w->arrival;
    else if(drag==Drag::sourceStart)initialSample=w->sourceStart;
    else if(drag==Drag::sourceEnd)initialSample=w->sourceEnd;
    else if(dragRaw)
    {
        int64_t a,b;if(!requestedShapeBounds(*w,dragRaw->start,a,b)){drag=Drag::none;return;}
        initialSample=drag==Drag::audibleEnd?b:drag==Drag::fadeIn?a+std::min(w->shape.fadeIn,b-a-1)
            :drag==Drag::fadeOut?(w->shape.fadeOut==0?b:b-1-std::min(w->shape.fadeOut,b-a-1)):a;
    }
}
bool SwellTimeline::prepareDrag(juce::Point<float> point)
{
    if(data.sessionEpoch!=engine.assignmentEpoch()){drag=Drag::none;gestureMessage("Session changed; start a new gesture");return false;}
    const auto w=find(data.session,dragId);if(!w){drag=Drag::none;return false;}
    dragEpoch=data.sessionEpoch;dragSelection=data.session.selected;dragTime=time;dragAcceptedTime=data.session.timing;
    dragClock=clockContext;
    dragRaw=data.raw;dragInput=w->inputGeneration;dragRender=w->render.content;dragSampleAvailable=playheadAvailable;dragFrozen=w->frozen;
    dragGrid=makeGrid();
    dragView=view;lastPointer=point;rebaseDrag();gestureMessage({});return drag!=Drag::none;
}
std::pair<double,double> SwellTimeline::globalBounds() const
{
    double a=std::numeric_limits<double>::infinity(),b=-a;
    const auto include=[&](double first,double last){if(std::isfinite(first) && std::isfinite(last) && last>first){a=std::min(a,first);b=std::max(b,last);}};
    for(const auto& range:capture.manifest.ranges)include(static_cast<double>(range.startSample),static_cast<double>(range.startSample)+range.frameCount);
    for(const auto& w:data.session.swells)
    {
        include(static_cast<double>(w.sourceStart),static_cast<double>(w.sourceEnd));
        // Pending source/IR timing keeps the initial lead-in inside navigation bounds.
        if(w.ir.present() && w.phase!=Phase::ready)
        {const auto [lead,end]=ui::initialSwellBounds(w,time);include(lead,end);}
        if(w.render.present())include(static_cast<double>(w.render.start)+w.placement,static_cast<double>(w.render.start)+w.render.frames+w.placement);
        int64_t first,last;if(requestedShapeBounds(w,w.render.present()?w.render.start:w.sourceStart,first,last))include(static_cast<double>(first),static_cast<double>(last));
    }
    if(!std::isfinite(a) || !std::isfinite(b) || b<=a){a=0;b=std::isfinite(time.rate) && time.rate>0?time.rate*8:384000;if(!std::isfinite(b))b=384000;}
    return {a,b};
}
SwellViewport SwellTimeline::navigationBounds()const
{
    const auto [first,last]=globalBounds();SwellViewport bounds;bounds.fit(first,last);
    bounds.first=std::max(bounds.first,-9223372036854775808.);
    bounds.last=std::min(bounds.last,std::nextafter(9223372036854775808.,0.));
    if(bounds.last<=bounds.first){bounds.first=0;bounds.last=384000;}
    return bounds;
}
double SwellTimeline::minimumViewSpan()const
{
    const auto bounds=navigationBounds();
    const auto precision=std::max(std::abs(bounds.first),std::abs(bounds.last))*std::numeric_limits<double>::epsilon()*16;
    return std::min(bounds.span(),std::max({1.,static_cast<double>(source.getWidth()),precision}));
}
double SwellTimeline::navigationAnchor()const
{
    const auto visible=[&](double at){return at>=view.first && at<=view.last;};
    if(const auto w=find(data.session,data.session.primary);w && visible(static_cast<double>(w->arrival)))return (w->arrival-view.first)/view.span();
    if(playheadAvailable && visible(static_cast<double>(time.sample)))return (time.sample-view.first)/view.span();
    return .5;
}
void SwellTimeline::constrainNavigation(){view.constrain(navigationBounds(),minimumViewSpan());}
double SwellTimeline::zoomFraction()const
{
    const auto maximum=navigationBounds().span(),minimum=minimumViewSpan();
    return maximum>minimum?std::clamp(std::log(maximum/std::clamp(view.span(),minimum,maximum))/std::log(maximum/minimum),0.,1.):0.;
}
void SwellTimeline::setZoomFraction(double fraction)
{
    if(!std::isfinite(fraction))return;
    if(fraction<=0){view=navigationBounds();navigationChanged();return;}
    const auto maximum=navigationBounds().span(),minimum=minimumViewSpan();
    const auto wanted=maximum*std::pow(minimum/maximum,std::clamp(fraction,0.,1.));
    view.zoomBounded(wanted/view.span(),navigationAnchor(),navigationBounds(),minimum);navigationChanged();
}
void SwellTimeline::updateScroll()
{
    const auto [a,b]=globalBounds();updatingScroll=true;
    scrollbar.setRangeLimits(std::min(a-view.span(),view.first),std::max(b+view.span(),view.last),juce::dontSendNotification);
    scrollbar.setCurrentRange(view.first,view.span(),juce::dontSendNotification);updatingScroll=false;
}
void SwellTimeline::scrollBarMoved(juce::ScrollBar*,double start)
{if(updatingScroll)return;const auto size=view.span();view.first=start;view.last=start+size;constrainNavigation();navigationChanged();}
void SwellTimeline::fitSelection()
{
    if(const auto selected=find(data.session,data.session.primary))
    {
        double a=static_cast<double>(selected->sourceStart),b=static_cast<double>(selected->sourceEnd);
        const auto wetBounds=[&](const Swell& w)
        {
            double first=static_cast<double>(w.arrival)+w.placement,last=first+1;
            if(w.render.present()){first=static_cast<double>(w.render.start)+w.placement;last=first+w.render.frames;}
            int64_t requestedA,requestedB;
            const auto rawStart=w.id==selected->id && data.raw?data.raw->start:w.render.present()?w.render.start:w.sourceStart;
            if(requestedShapeBounds(w,rawStart,requestedA,requestedB)){first=std::min(first,static_cast<double>(requestedA));last=std::max(last,static_cast<double>(requestedB));}
            return std::pair{first,last};
        };
        const auto [first,last]=wetBounds(*selected);a=std::min(a,first);b=std::max(b,last);
        const auto contextFirst=a,contextLast=b;
        for(const auto& other:data.session.swells)if(other.id!=selected->id)
        {const auto [left,right]=wetBounds(other);if(left<contextLast && right>contextFirst){a=std::min(a,left);b=std::max(b,right);}}
        view.fit(a,b);
    }
    else if(!capture.manifest.ranges.empty())
    {const auto& first=capture.manifest.ranges.front();const auto& last=capture.manifest.ranges.back();view.fit(static_cast<double>(first.startSample),static_cast<double>(last.startSample)+last.frameCount);}
    else {const auto [a,b]=globalBounds();view.fit(a,b);}constrainNavigation();navigationChanged();
}
void SwellTimeline::page(int bars){view.page(bars,time);constrainNavigation();navigationChanged();}
void SwellTimeline::previous(){view.shift(-1);constrainNavigation();navigationChanged();}
void SwellTimeline::next(){view.shift(1);constrainNavigation();navigationChanged();}
void SwellTimeline::zoom(double factor){view.zoomBounded(factor,navigationAnchor(),navigationBounds(),minimumViewSpan());navigationChanged();}
void SwellTimeline::revealInitialSelection()
{
    if(const auto* swell=find(data.session,data.session.primary))
    {view=ui::revealInitialSwell(view,*swell,time);constrainNavigation();}
    navigationChanged();
}
void SwellTimeline::resized()
{
    invalidateWaveforms();
    drag=Drag::none;
    auto area=getLocalBounds().toFloat().reduced(2);scrollbar.setBounds(area.removeFromBottom(16).toNearestInt());area.removeFromBottom(4);
    if(inlineOverview){overview=area.removeFromBottom(80);area.removeFromBottom(24);}
    else overview={-10000,-10000,0,0};
    auto waveControls=area.removeFromTop(24);waveZoomLabel.setBounds(waveControls.removeFromLeft(76).toNearestInt());
    waveZoom.setBounds(waveControls.removeFromLeft(std::min(240.0f,waveControls.getWidth())).toNearestInt());
    ruler=area.removeFromTop(responsivePresentation?ui::responsiveGeometry::footerHeight:ui::controlGeometry::headingHeight).toNearestInt();
    area.removeFromTop(responsivePresentation?ui::responsiveGeometry::laneCaptionHeight+ui::responsiveGeometry::laneCaptionGap:ui::controlGeometry::feedbackHeight+ui::spacing::tight);
    const auto laneGap=responsivePresentation?float(ui::responsiveGeometry::laneCaptionBand):32.f;
    source=area.removeFromTop(std::max(40.0f,(area.getHeight()-laneGap)/2));area.removeFromTop(laneGap);wet=area;
    updateScroll();
}
void SwellTimeline::drawWave(juce::Graphics& g,const std::vector<CapturePeak>& peaks,juce::Rectangle<float> lane,juce::Colour colour,float gain,ui::WaveformPaintMetrics& metrics,int64_t placement)
{
    juce::Graphics::ScopedSaveState saved(g);g.reduceClipRegion(lane.toNearestInt());g.setColour(colour);
    const auto columns=ui::waveformColumns(peaks,view.first,view.last,static_cast<int>(std::ceil(lane.getWidth())),placement,
        [](const auto& p){return std::pair{p.minimum,p.maximum};},metrics.peaksVisited,true);
    for(size_t i=0;i<columns.size();++i)if(columns[i].present)
    {
        const auto top=lane.getCentreY()-std::clamp(columns[i].maximum*gain,-1.0f,1.0f)*lane.getHeight()*0.42f;
        const auto bottom=lane.getCentreY()-std::clamp(columns[i].minimum*gain,-1.0f,1.0f)*lane.getHeight()*0.42f;
        g.drawVerticalLine(static_cast<int>(lane.getX())+static_cast<int>(i),top,std::max(top+1.0f,bottom));++metrics.columnsPainted;
    }
}
juce::Rectangle<float> SwellTimeline::loopLeadInBounds() const noexcept
{
    if (!data.loopLeadIn.start || !capture.manifest.loop) return {};
    const auto left = std::clamp(x(*data.loopLeadIn.start),wet.getX(),wet.getRight());
    const auto right = std::clamp(x(capture.manifest.loop->endSample),wet.getX(),wet.getRight());
    return {left,wet.getY(),std::max(0.0f,right - left),wet.getHeight()};
}

void SwellTimeline::paintWaveforms(juce::Graphics& g,ui::WaveformPaintMetrics& metrics,bool wetOnly)
{
    const auto waveGain=juce::Decibels::decibelsToGain(static_cast<float>(waveZoom.getValue()));
    if(wetOnly)
    {
        const auto primary=find(data.session,data.session.primary);
        const auto leadIn = loopLeadInBounds();
        if (!leadIn.isEmpty() && data.loopLeadIn.peaks)
        {
            juce::Graphics::ScopedSaveState clip(g);
            g.reduceClipRegion(leadIn.getSmallestIntegerContainer());
            drawWave(g,*data.loopLeadIn.peaks,wet,ui::aqua.withAlpha(.38f),waveGain * data.loopLeadIn.gain,metrics,capture.manifest.loop->endSample);
        }
        for(const auto& [id,wave]:data.wetSummaries)if(id!=data.session.primary && wave)
            drawWave(g,wave->peaks,wet,ui::otherWet.withAlpha(.22f),.45f/std::max(.000001f,wave->peak),metrics,find(data.session,id)?find(data.session,id)->placement:0);
        if(data.raw)
        {
            drawWave(g,data.raw->peaks,wet,ui::rawWet.withAlpha(.3f),.55f/std::max(.000001f,data.raw->peak),metrics,primary?primary->placement:0);
            if(data.shaped)drawWave(g,data.shaped->peaks,wet,primary && primary->phase==Phase::ready?ui::aqua:ui::staleWet,waveGain,metrics);
        }
        return;
    }
    const auto [globalA,globalB]=globalBounds();
    const auto mini=[&](double sample){return overview.getX()+static_cast<float>((sample-globalA)/(globalB-globalA))*overview.getWidth();};
    g.setColour(ui::background);if(inlineOverview)g.fillRect(overview);g.fillRect(source);g.fillRect(wet);
    for(const auto& item:capture.overview)
    {
        const auto a=item.range.startSample,b=a+static_cast<int64_t>(item.range.frameCount);
        const auto colour=!item.available?ui::unavailableSource:item.range.kind==CaptureCoverageKind::silence?ui::capturedSilence:ui::background;
        if(inlineOverview)
        {
            g.setColour(colour);g.fillRect(juce::Rectangle<float>(mini(static_cast<double>(a)),overview.getY(),std::max(1.0f,mini(static_cast<double>(b))-mini(static_cast<double>(a))),overview.getHeight()));
            if(item.available)
            {
                g.setColour(ui::secondary);
                const auto columns=ui::waveformColumns(item.peaks,globalA,globalB,static_cast<int>(std::ceil(overview.getWidth())),0,
                    [](const auto& p){return std::pair{p.minimum,p.maximum};},metrics.peaksVisited);
                for(size_t i=0;i<columns.size();++i)if(columns[i].present)
                {g.drawVerticalLine(static_cast<int>(overview.getX())+static_cast<int>(i),overview.getCentreY()-columns[i].maximum*25,overview.getCentreY()-columns[i].minimum*25+1);++metrics.columnsPainted;}
            }
        }
        const auto left=std::max(source.getX(),x(a)),right=std::min(source.getRight(),x(b));if(right<=left)continue;
        g.setColour(colour);g.fillRect(juce::Rectangle<float>(left,source.getY(),right-left,source.getHeight()));
        if(item.available && item.range.kind==CaptureCoverageKind::audio)
        {drawWave(g,item.peaks,source,ui::secondary,waveGain,metrics);}
        else if(item.available){g.setColour(ui::secondary);g.drawHorizontalLine(static_cast<int>(source.getCentreY()),left,right);}
    }
    if(data.sourceDetail)
    {
        // A retained detail cache cannot paint over newly unavailable/unmapped
        // coverage. Only current, available audio intersections can replace it.
        for(const auto& item:capture.overview)if(item.available && item.range.kind==CaptureCoverageKind::audio)
        {
            int64_t end;if(item.range.frameCount>INT64_MAX || !checkedAdd(item.range.startSample,static_cast<int64_t>(item.range.frameCount),end))continue;
            const auto left=std::max(source.getX(),x(std::max(data.sourceDetail->start,item.range.startSample)));
            const auto right=std::min(source.getRight(),x(std::min(data.sourceDetail->end,end)));
            if(right<=left)continue;
            juce::Graphics::ScopedSaveState clip(g);g.reduceClipRegion(juce::Rectangle<float>(left,source.getY(),right-left,source.getHeight()).toNearestInt());
            g.setColour(ui::background);g.fillRect(source);drawWave(g,data.sourceDetail->peaks,source,ui::secondary,waveGain,metrics);
        }
    }

}
void SwellTimeline::paint(juce::Graphics& g)
{
    g.fillAll(ui::background);g.setFont(ui::fontFor(*this,ui::smallSize));
    ui::controls::paintMusicalTimelineRuler(g,ruler,view,gridTiming(),this);
    if(!time.valid){ui::applyTextRole(g,*this,ui::TextRole::helper);g.drawText("Musical timing unavailable",ruler,juce::Justification::centredLeft);}
    if(capture.manifest.ranges.empty() && data.session.swells.empty())
    {
        const auto area=source.getUnion(wet);auto headline=area.withHeight(64).withCentre(area.getCentre().translated(0,-30));
        g.setColour(ui::text);g.setFont(ui::fontFor(*this,ui::emptyHeadingSize,juce::Font::bold));
        g.drawFittedText("Press play to begin\nmapping your song",headline.toNearestInt(),juce::Justification::centred,2);
        g.setFont(ui::fontFor(*this,ui::bodySize));g.setColour(ui::secondary);
        g.drawFittedText("Arm capture in Learn, then start playback in your DAW.\nStop playback to finish a Song take.",headline.translated(0,76).withHeight(62).toNearestInt(),juce::Justification::centred,3);
        return;
    }
    waveformLayer.paint(g,getWidth(),getHeight(),view.first,view.last,[this](auto& drawing,float,auto& metrics){paintWaveforms(drawing,metrics,false);});
    const auto [globalA,globalB]=globalBounds();
    const auto mini=[&](double sample){return overview.getX()+static_cast<float>((sample-globalA)/(globalB-globalA))*overview.getWidth();};
    if(inlineOverview)
    {
    g.setColour(juce::Colours::white.withAlpha(0.8f));const auto miniLeft=std::clamp(mini(view.first),overview.getX(),overview.getRight()),miniRight=std::clamp(mini(view.last),overview.getX(),overview.getRight());
    g.drawRect(juce::Rectangle<float>(miniLeft,overview.getY(),std::max(1.0f,miniRight-miniLeft),overview.getHeight()),1);
    g.drawText("Song overview | last captured",overview.translated(0,-22).withHeight(20),juce::Justification::centredLeft);
    for(const auto& swell:data.session.swells)
    {const auto at=mini(static_cast<double>(swell.arrival));g.setColour(swell.id==data.session.primary?ui::aqua:ui::secondary);g.drawVerticalLine(static_cast<int>(at),overview.getBottom()-13,overview.getBottom());}
    }
    ui::applyTextRole(g,*this,ui::TextRole::helper);g.drawText("Dry",source.translated(0,responsivePresentation?-(ui::responsiveGeometry::laneCaptionHeight+ui::responsiveGeometry::laneCaptionGap):-24).withHeight(20),juce::Justification::centredLeft);
    const auto ticks=ui::controls::musicalTimelineTicks(view,ruler.getWidth(),gridTiming());
    for(const auto& tick:ticks)
    {
        const auto at=static_cast<float>(view.pixel(tick.sample,source.getX(),source.getWidth()));
        g.setColour(ui::separator.withAlpha(tick.bar?.35f:.15f));g.drawVerticalLine(static_cast<int>(at),source.getY(),wet.getBottom());
    }
    std::set<int> candidateColumns;
    for(size_t i=0;i<data.candidates.size();++i)
    {
        const auto at=x(data.candidates[i].sample);if(at<source.getX() || at>source.getRight() || !candidateColumns.insert(static_cast<int>(at)/4).second)continue;
        juce::Path triangle;triangle.addTriangle(at-4,source.getY()+2,at+4,source.getY()+2,at,source.getY()+10);
        g.setColour(ui::aqua);g.strokePath(triangle,juce::PathStrokeType(1));
    }
    std::set<int> markerColumns;const auto primary=find(data.session,data.session.primary);
    std::optional<EditorSnapshot::ProposedTiming> proposed;
    if(primary)
    {
        if(drag!=Drag::none && timingDraft)
        {
            const auto& w=timingDraft->swell;
            proposed=EditorSnapshot::ProposedTiming{w.arrival,w.sourceStart,w.sourceEnd,w.placement};
        }
        else if(const auto request=data.irRequests.find(primary->id);request!=data.irRequests.end())
            proposed=request->second.timing;
    }
    for(const auto& w:data.session.swells)
    {
        const auto at=x(w.arrival);if(at<source.getX() || at>source.getRight())continue;
        if(w.id==data.session.primary || (primary && std::abs(at-x(primary->arrival))<8) || !markerColumns.insert(static_cast<int>(at)/8).second)continue;
        const auto selected=std::find(data.session.selected.begin(),data.session.selected.end(),w.id)!=data.session.selected.end();
        g.setColour(selected?ui::text:ui::secondary);g.drawVerticalLine(static_cast<int>(at),source.getY(),wet.getBottom());
        g.fillEllipse(at-4,source.getY()+12,8,8);
    }
    const auto target=primary?x(proposed?proposed->arrival:primary->arrival):source.getX()-1;
    if(target>=source.getX() && target<=source.getRight())
    {const auto at=target;g.setColour(ui::timing);const float dashes[]{4,4};
        g.drawDashedLine({at,source.getY(),at,wet.getBottom()},dashes,2,1);
        juce::Path marker;marker.addTriangle(at-5,source.getY()+5,at+5,source.getY()+5,at,source.getY()+14);g.fillPath(marker);
        g.drawText("Target",juce::Rectangle<float>(at+7,source.getY()+3,140,18),juce::Justification::left);}
    const auto leadIn = loopLeadInBounds();
    if (!leadIn.isEmpty())
    {
        g.setColour(ui::aqua.withAlpha(.12f));
        g.fillRect(leadIn);
    }
    // The cached waveforms already carry their individual opacities.
    g.setOpacity(1.0f);
    wetWaveformLayer.paint(g,getWidth(),getHeight(),view.first,view.last,[this](auto& drawing,float,auto& metrics){paintWaveforms(drawing,metrics,true);});
    if (!leadIn.isEmpty())
    {
        ui::applyTextRole(g,*this,ui::TextRole::helper);
        const juce::String label = "Lead-In";
        const auto labelArea = leadIn.reduced(ui::spacing::tight,0).withHeight(ui::responsiveGeometry::laneCaptionHeight);
        if (labelArea.getWidth() >= juce::GlyphArrangement::getStringWidth(g.getCurrentFont(),label))
            g.drawText(label,labelArea,juce::Justification::centredRight);
    }
    if(data.raw)
    {
        const auto current=primary && primary->phase==Phase::ready;
        const auto colour=current?ui::aqua:ui::staleWet;
        ui::applyTextRole(g,*this,ui::TextRole::helper);g.setColour(colour);g.drawText(current?(data.shaped?"Wet":"Wet - preparing"):"Wet - stale",wet.translated(0,responsivePresentation?-(ui::responsiveGeometry::laneCaptionHeight+ui::responsiveGeometry::laneCaptionGap):-25).withHeight(20),juce::Justification::left);
    }
    else{g.setColour(ui::secondary);g.drawText(primary?primary->status:juce::String("Select a swell to inspect its wet audio"),wet,juce::Justification::centred);}
    if(data.dryAvailable && data.session.auditionMode==AuditionMode::dryWet){
        const auto a=std::clamp(x(data.auditionDryStart),source.getX(),source.getRight()),b=std::clamp(x(data.auditionDryEnd),source.getX(),source.getRight());
        if(b>a){g.setColour(ui::previewCursor.withAlpha(.6f));g.fillRect(a,source.getBottom()-5,b-a,5.0f);}}
    if(const auto w=find(data.session,data.session.primary))
    {
        if(renderInputEnd(*w)>w->sourceEnd)
        {
            juce::Graphics::ScopedSaveState clip(g);g.reduceClipRegion(source.toNearestInt());
            g.setColour(ui::warning.withAlpha(.38f));g.fillRect(x(w->sourceEnd),source.getY(),std::max(1.0f,x(renderInputEnd(*w))-x(w->sourceEnd)),6.0f);
        }
        const auto left=std::clamp(x(w->sourceStart),source.getX(),source.getRight()),right=std::clamp(x(w->sourceEnd),source.getX(),source.getRight());
        if(right>left){g.setColour(ui::text.withAlpha(.055f));g.fillRect(left,source.getY(),right-left,source.getHeight());}
        for(const auto point:{w->sourceStart,w->sourceEnd})if(x(point)>=source.getX() && x(point)<=source.getRight())
        {
            const auto at=x(point);g.setColour(ui::secondary);g.drawVerticalLine(static_cast<int>(at),source.getY(),source.getBottom());
            const auto handle=juce::Rectangle<float>(12,12).withCentre({at,source.getBottom()-9});
            g.setColour(ui::background);g.fillEllipse(handle);g.setColour(ui::secondary);g.drawEllipse(handle,1.5f);
        }
        const auto hint=gestureStatus.isNotEmpty()?gestureStatus:effectiveSnap() && !gridAvailable()?juce::String("Snap unavailable: fixed timing required; hold Ctrl or turn Snap off"):juce::String{};
        if(hint.isNotEmpty()){const juce::Graphics::ScopedSaveState saved(g);ui::applyTextRole(g,*this,ui::TextRole::helper);g.setColour(hint==sourceHover?ui::text:ui::warning);g.drawText(hint,source.reduced(8).withHeight(ui::textStyle(ui::TextRole::helper).lineHeight).translated(0,22),juce::Justification::left);}
        if(data.raw)
        {
            int64_t a,b;juce::String error;
            if(requestedShapeBounds(*w,data.raw->start,a,b))
            {
                juce::Graphics::ScopedSaveState saved(g);g.reduceClipRegion(wet.toNearestInt());
                g.setColour(juce::Colours::black.withAlpha(0.35f));g.fillRect(juce::Rectangle<float>(wet.getX(),wet.getY(),std::max(0.0f,x(a)-wet.getX()),wet.getHeight()));
                g.fillRect(juce::Rectangle<float>(x(b),wet.getY(),std::max(0.0f,wet.getRight()-x(b)),wet.getHeight()));
                
                
                auto relative=w->shape;relative.gainDb=0;juce::Path envelopeLine;envelopeLine.startNewSubPath(x(a),wet.getCentreY());
                for(int point=0;point<=256;++point)
                {
                    const auto at=a+static_cast<int64_t>((static_cast<long double>(b)-a-1)*point/256);
                    const auto y=wet.getCentreY()-static_cast<float>(envelope(relative,at,a,b))*wet.getHeight()*.38f;
                    envelopeLine.lineTo(x(at),y);
                }
                envelopeLine.lineTo(x(b),wet.getCentreY());
                g.setColour(ui::aqua);g.strokePath(envelopeLine,juce::PathStrokeType(2.0f));
                if(static_cast<long double>(b)-w->placement>data.raw->end){g.setColour(ui::warning);g.drawText("Requested wet support - capture through endpoint",wet.withHeight(22),juce::Justification::left);}

                const auto handles=grabPoints();for(const auto& handle:handles)
                {
                    const auto active=hover && *hover==handle.kind;
                    if(handle.point.getDistanceFrom(handle.anchor)>1){g.setColour(ui::secondary);g.drawLine({handle.anchor,handle.point},1);
                        const juce::String label=handle.kind==EnvelopePoint::fadeIn?"Fade in":handle.kind==EnvelopePoint::fadeOut?"Fade out":handle.kind==EnvelopePoint::start?"Start":handle.kind==EnvelopePoint::end?"End":"Tension";
                        if(handle.kind==EnvelopePoint::fadeIn || handle.kind==EnvelopePoint::fadeOut){
                            const std::array<juce::Rectangle<float>,4> labels{{{handle.point.x+12,handle.point.y-7,56,15},
                                {handle.point.x-68,handle.point.y-7,56,15},{handle.point.x+12,handle.point.y+16,56,15},{handle.point.x+12,handle.point.y-34,56,15}}};
                            for(const auto& box:labels)if(wet.contains(box) && std::none_of(handles.begin(),handles.end(),[&](const auto& other){
                                return box.expanded(2).intersects(juce::Rectangle<float>(16,16).withCentre(other.point));})){
                                g.setColour(ui::background.withAlpha(.9f));g.fillRoundedRectangle(box.expanded(1),2);
                                g.setColour(ui::secondary);g.drawText(label,box,juce::Justification::left);break;}}
                        g.fillEllipse(juce::Rectangle<float>(4,4).withCentre(handle.anchor));}const auto r=juce::Rectangle<float>(14,14).withCentre(handle.point);
                    g.setColour(ui::background);g.fillEllipse(r);
                    g.setColour(active?ui::text:ui::aqua);g.drawEllipse(r,1.5f);
                    if(handle.kind==EnvelopePoint::inTension || handle.kind==EnvelopePoint::outTension)g.fillEllipse(r.reduced(5));
                    if(active){const juce::String text=handle.kind==EnvelopePoint::end?"Wet end (gap / overlap)":handle.kind==EnvelopePoint::start?"Wet start":handle.kind==EnvelopePoint::inTension?"Fade-in tension":handle.kind==EnvelopePoint::outTension?"Fade-out tension":handle.kind==EnvelopePoint::fadeIn?"Fade-in duration":"Fade-out duration";
                        g.setColour(juce::Colours::white);g.drawText(text,wet.toNearestInt().withHeight(22),juce::Justification::right);}
                }
            }
        }
    }
    if(target>=source.getX() && target<=source.getRight())
    {g.setColour(ui::timing);const float dashes[]{4,4};g.drawDashedLine({target,source.getY()+24,target,wet.getBottom()},dashes,2,1);}
    if(primary && proposed)
    {
        const float dashes[]{4,4};
        const auto guide=[&](int64_t sample,juce::Rectangle<float> lane,juce::Colour colour)
        {
            const auto at=x(sample);
            if(at<lane.getX() || at>lane.getRight())return;
            g.setColour(colour);
            g.drawDashedLine({at,lane.getY(),at,lane.getBottom()},dashes,2,ui::rule);
        };
        if(proposed->arrival==primary->arrival)
        {
            if(proposed->sourceStart!=primary->sourceStart)guide(proposed->sourceStart,source,ui::secondary);
            if(proposed->sourceEnd!=primary->sourceEnd)guide(proposed->sourceEnd,source,ui::secondary);
        }
        if(data.raw && proposed->placement!=primary->placement)
        {
            auto moved=*primary;
            moved.placement=proposed->placement;
            int64_t first,last;
            if(requestedShapeBounds(moved,data.raw->start,first,last))
            {guide(first,wet,ui::aqua);guide(last,wet,ui::aqua);}
        }
    }
    if(const auto position=transportPlayhead())
    {
        const auto play=x(*position);
        if(play>=source.getX() && play<=source.getRight())
        {g.setColour(ui::timing);g.drawVerticalLine(static_cast<int>(play),source.getY(),wet.getBottom());}
    }
    if(data.auditionSample){const auto play=x(*data.auditionSample);if(play>=source.getX() && play<=source.getRight()){
        g.setColour(ui::previewCursor);g.drawVerticalLine(static_cast<int>(play),source.getY(),wet.getBottom());
        g.drawText("Audition",juce::Rectangle<float>(play+4,source.getY()+3,65,16),juce::Justification::left);}}

    if(localPlayhead)
    {
        const auto play=x(*localPlayhead);
        if(play>=source.getX() && play<source.getRight())
        {
            g.setColour(ui::previewCursor);
            const float dashes[]{4,4};
            for(const auto& lane:{source,wet})g.drawDashedLine({play,lane.getY(),play,lane.getBottom()},dashes,2,ui::rule);
        }
    }
    ui::controls::paintWaveformFocus(g,*this,source.getUnion(wet).getSmallestIntegerContainer().expanded(ui::controlGeometry::focusPadding),*focus);
}
std::optional<int64_t> SwellTimeline::transportPlayhead() const noexcept
{
    if(capture.sourceKind()==CaptureSourceKind::loop) return loopPlayhead;
    if(songPlayheadAvailable && playheadAvailable && time.valid && capture.sourceKind()==CaptureSourceKind::song) return time.sample;
    return {};
}

std::optional<int64_t> SwellTimeline::loopInsertionSample() const noexcept
{
    if(!capture.manifest.loop || capture.manifest.loop->endSample<=0)return {};
    const auto last=capture.manifest.loop->endSample-1;
    if(loopPlayhead && *loopPlayhead>=0 && *loopPlayhead<=last)return loopPlayhead;
    return static_cast<int64_t>(std::clamp(view.first,0.0,static_cast<double>(last)));
}

bool SwellTimeline::beginDrag(juce::Point<float> point)
{return beginEdit(point)==Press::handled;}

SwellTimeline::Press SwellTimeline::beginEdit(juce::Point<float> point)
{
    drag=Drag::none;
    if(const auto w=find(data.session,data.session.primary))
    {
        dragId=w->id;
        if(source.contains(point) && point.y>source.getY()+24)
        {if(std::abs(point.x-x(w->sourceStart))<7)drag=Drag::sourceStart;else if(std::abs(point.x-x(w->sourceEnd))<7)drag=Drag::sourceEnd;}
        if(wet.expanded(8).contains(point) && data.raw)
        {
            if(const auto hit=hitEnvelope(grabPoints(),point))
            {
                hover=hit;
                switch(*hit){case EnvelopePoint::start:drag=Drag::audibleStart;break;case EnvelopePoint::end:drag=Drag::audibleEnd;break;
                    case EnvelopePoint::fadeIn:drag=Drag::fadeIn;break;case EnvelopePoint::fadeOut:drag=Drag::fadeOut;break;
                    case EnvelopePoint::inTension:drag=Drag::inTension;break;case EnvelopePoint::outTension:drag=Drag::outTension;break;}
            }
        }
        if(drag==Drag::none && wet.contains(point) && data.raw)
        {int64_t a,b;if(requestedShapeBounds(*w,data.raw->start,a,b) && point.x>=x(a) && point.x<=x(b))
            {drag=Drag::move;hover.reset();}}
        if(drag!=Drag::none)
        {
            if(editingEnabled && prepareDrag(point))return Press::handled;
            drag=Drag::none;return Press::blocked;
        }
    }
    if(source.contains(point))
    {
        for(const auto& w:data.session.swells)if(std::abs(point.x-x(w.arrival))<7)
        {if(!editingEnabled || !engine.selectTargets({w.id},w.id,data.sessionEpoch))return Press::blocked;
            dragId=w.id;drag=Drag::arrival;data.session=engine.savedSession();return prepareDrag(point)?Press::handled:Press::blocked;}
        for(size_t i=0;i<data.candidates.size();++i)if(std::abs(point.x-x(data.candidates[i].sample))<7)
        {if(!editingEnabled)return Press::blocked;
            if(onCandidateSelected)onCandidateSelected(static_cast<int>(i));return Press::handled;}
    }
    return Press::open;
}
void SwellTimeline::dragTo(juce::Point<float> point)
{
    if(drag==Drag::none)return;
    if(!std::isfinite(point.x) || !std::isfinite(point.y) || source.getWidth()<=0 || point==lastPointer)return;
    lastPointer=point;const auto kind=drag;const bool vertical=kind==Drag::inTension || kind==Drag::outTension;
    const auto snapping=effectiveSnap() && !vertical;
    if(snapping && !dragGrid){gestureMessage("Snap unavailable: compatible fixed host timing required. Hold Ctrl or turn Snap off.");return;}
    const auto delta=static_cast<long double>(point.x-dragOrigin.x)*dragView.span()/source.getWidth()*(fineDrag?.1L:1.L);
    const auto candidate=static_cast<long double>(initialSample)+std::round(delta);
    if(!vertical && (!std::isfinite(candidate) || candidate< -9223372036854775808.L || candidate>=9223372036854775808.L))
    {gestureMessage("Timing candidate overflow");return;}
    const auto requested=vertical?int64_t{0}:static_cast<int64_t>(candidate);
    if(deferTimingDrag)
    {
        const auto* current=find(data.session,dragId);
        if(!current)return;
        auto proposed=*current;
        auto anchor=*current;
        anchor.shape=initialEnvelope;
        anchor.placement=initialPlacement;
        const auto kindOfEdit=kind==Drag::arrival?TimingAnchor::arrival:kind==Drag::sourceStart?TimingAnchor::sourceStart
            :kind==Drag::sourceEnd?TimingAnchor::sourceEnd:TimingAnchor::move;
        long double origin=0;
        songOrigin(dragAcceptedTime,origin);
        juce::String error;
        if(!applyTimingCandidate(proposed,anchor,kindOfEdit,requested,dragRaw?dragRaw->start:0,dragRaw?dragRaw->end:0,
                                 snapping,dragGrid?&*dragGrid:nullptr,origin,error))
        {gestureMessage(error);return;}
        timingDraft=TimingDraft{std::move(proposed),requested,snapping};
        gestureMessage({});
        repaint();
        return;
    }
    submitDrag(requested,snapping,(point.y-dragOrigin.y)*(fineDrag?.1:1.));
}

void SwellTimeline::submitDrag(int64_t requested,bool snapping,double verticalDelta)
{
    const auto kind=drag;
    const auto vertical=kind==Drag::inTension || kind==Drag::outTension;
    const auto raw=dragRaw;juce::String failure;bool stale=false,invoked=false;
    const auto changed=engine.edit(dragId,[&](Swell& w,juce::String& error)
    {
        invoked=true;
        if(dragClock.playing){const auto latest=engine.transportClockSnapshot();
            if(!dragClock.sameAuthority(gridClockContext(latest,true)) || (snapping && (!latest || !HostGrid::make(dragAcceptedTime,latest->observed,capture.manifest.sampleRate,gridDivision,nullptr,true,&dragClock))))
            {stale=true;error=failure="Playback clock changed; start a new gesture";return false;}}
        // edit(expected epoch) owns the engine's recursive UI lock. Recheck the
        // current selection inside that same transaction, including secondary IDs.
        const auto current=engine.savedSession();
        if(current.primary!=dragId || current.selected!=dragSelection){stale=true;error=failure="Selection changed; start a new gesture";return false;}
        if(deferTimingDrag && w.inputGeneration!=dragInput)
        {stale=true;error=failure="Source changed; start a new gesture";return false;}
        if(!vertical)
        {
            const auto anchorKind=kind==Drag::arrival?TimingAnchor::arrival:kind==Drag::sourceStart?TimingAnchor::sourceStart:kind==Drag::sourceEnd?TimingAnchor::sourceEnd:
                kind==Drag::audibleStart?TimingAnchor::start:kind==Drag::audibleEnd?TimingAnchor::end:kind==Drag::fadeIn?TimingAnchor::fadeIn:kind==Drag::fadeOut?TimingAnchor::fadeOut:TimingAnchor::move;
            auto anchor=w;anchor.shape=initialEnvelope;anchor.placement=initialPlacement;long double origin=0;songOrigin(dragAcceptedTime,origin);
            const auto ok=applyTimingCandidate(w,anchor,anchorKind,requested,raw?raw->start:0,raw?raw->end:0,snapping,dragGrid?&*dragGrid:nullptr,origin,error);
            if(!ok)failure=error;return ok;
        }
        if(!raw)return false;auto shape=w.shape;
        if(kind==Drag::inTension)shape.inTension=std::clamp(initialTension+verticalDelta/80.0,-1.0,1.0);
        if(kind==Drag::outTension)shape.outTension=std::clamp(initialTension+verticalDelta/80.0,-1.0,1.0);
        return setShape(w,shape,raw->start,raw->end,error);
    },dragEpoch,nullptr,capture.manifest.loop?capture.manifest.revision:0,
        kind==Drag::arrival || kind==Drag::sourceStart || kind==Drag::sourceEnd || kind==Drag::move);
    if(!changed){if(!invoked || stale || engine.assignmentEpoch()!=dragEpoch)drag=Drag::none;gestureMessage(failure.isNotEmpty()?failure:"Edit rejected by current source/session/mode guard");return;}
    gestureMessage({});
    // Canonical metadata only: no waveform preparation or FFT on the message thread.
    data.session=engine.savedSession();repaint();if(onEnvelopeEdited)onEnvelopeEdited();
}
std::vector<EnvelopeGrab> SwellTimeline::grabPoints() const
{const auto w=find(data.session,data.session.primary);return w && data.raw?envelopeHandles(*w,data.raw->start,view,wet):std::vector<EnvelopeGrab>{};}
void SwellTimeline::mouseMove(const juce::MouseEvent& e)
{
    const auto hit=hitEnvelope(grabPoints(),e.position);if(hit!=hover){hover=hit;repaint();}
    juce::String help;
    if(const auto w=find(data.session,data.session.primary);w && source.contains(e.position))
    {
        if(e.position.y>source.getY()+24 && std::abs(e.position.x-x(w->sourceStart))<7)help="Source Start (drag; exact value in inspector)";
        else if(e.position.y>source.getY()+24 && std::abs(e.position.x-x(w->sourceEnd))<7)help="Source End, exclusive (drag; exact value in inspector)";
        else if(std::abs(e.position.x-x(w->arrival))<7)help="Target / Arrival (yellow; drag its top marker)";
    }
    if(help!=sourceHover){sourceHover=help;repaint();}
    if(source.contains(e.position) || wet.contains(e.position))
    {
        const auto detail=help.isNotEmpty()?help:wet.contains(e.position)
            ?juce::String("Audible shaped wet. Grey is the raw reference. Both lanes share a display amplitude scale. Drag the wet body to move audio and envelope. Ctrl bypasses Snap. Shift fine-adjusts.")
            :juce::String("Captured dry source. Double-click to add a target or drag its source handles.");
        setTooltip(detail+" Left-click or drag open waveform to place the local playhead. Wheel zooms. Middle-drag pans.");
    }
    else setTooltip("Musical ruler: bars and denominator beats, aligned with both lanes. Height changes display amplitude only.");
    setTitle(help.isEmpty()?juce::String("Dry/wet waveform editor"):help);setMouseCursor(help.isNotEmpty()?juce::MouseCursor::LeftRightResizeCursor:juce::MouseCursor::NormalCursor);
}
bool SwellTimeline::keyPressed(const juce::KeyPress& key)
{
    focus->keyboardInput();
    if(key.getKeyCode()!=juce::KeyPress::leftKey && key.getKeyCode()!=juce::KeyPress::rightKey)return false;
    setGestureModifiers(key.getModifiers().isShiftDown(),key.getModifiers().isCtrlDown());
    for(const auto& p:grabPoints())if(hover && p.kind==*hover){if(!beginDrag(p.point))return false;
        const auto step=key.getKeyCode()==juce::KeyPress::leftKey?-1:1;
        if(p.kind==EnvelopePoint::inTension || p.kind==EnvelopePoint::outTension)dragTo(p.point+juce::Point<float>{0,static_cast<float>(step)});
        else
        {
            auto next=std::optional<int64_t>{};
            if(effectiveSnap()){if(dragGrid)next=dragGrid->adjacent(initialSample,step);}
            else {int64_t at;if(checkedAdd(initialSample,step,at))next=at;}
            if(next){const juce::ScopedValueSetter<bool> normal(fineDrag,false);const auto pixels=(static_cast<long double>(*next)-initialSample)*source.getWidth()/dragView.span();dragTo(p.point+juce::Point<float>{static_cast<float>(pixels),0});}
            else gestureMessage("No available grid point for this keyboard nudge");
        }
        drag=Drag::none;return true;}return false;
}
void SwellTimeline::mouseDown(const juce::MouseEvent& e)
{
    endPointerGesture();
    const auto left=waveformButton(e.mods,juce::ModifierKeys::leftButtonModifier);
    const auto middle=waveformButton(e.mods,juce::ModifierKeys::middleButtonModifier);
    if(left && inlineOverview && overview.contains(e.position))
    {
        pointerGesture=PointerGesture::overviewPan;panStart=view;return;
    }
    if((!left && !middle) || (!source.contains(e.position) && !wet.expanded(left?8.f:0.f).contains(e.position)))return;
    focus->pointerInput();
    if(isShowing())grabKeyboardFocus();
    if(middle){pointerGesture=PointerGesture::detailPan;panStart=view;return;}
    setGestureModifiers(e.mods.isShiftDown(),e.mods.isCtrlDown());
    if(beginEdit(e.position)==Press::open && (source.contains(e.position) || wet.contains(e.position)))
    {pointerGesture=PointerGesture::position;positionPlayhead(e.position.x);}
    if(drag==Drag::arrival || drag==Drag::sourceStart || drag==Drag::sourceEnd || drag==Drag::move)
        if(const auto* swell=find(data.session,dragId))
            deferTimingDrag=swell->soundPreset.pingPong && swell->soundPreset.pingPong->tempoSync;
    repaint();
}
void SwellTimeline::mouseDrag(const juce::MouseEvent& e)
{
    const auto dx=static_cast<double>(e.getDistanceFromDragStartX());
    if(pointerGesture==PointerGesture::detailPan && waveformButton(e.mods,juce::ModifierKeys::middleButtonModifier) && source.getWidth()>0)
        panFrom(panStart,-dx/source.getWidth());
    if(!waveformButton(e.mods,juce::ModifierKeys::leftButtonModifier))return;
    if(pointerGesture==PointerGesture::overviewPan && overview.getWidth()>0)
    {
        const auto [first,last]=globalBounds();
        panFrom(panStart,dx/overview.getWidth()*(last-first)/panStart.span());
    }
    else if(pointerGesture==PointerGesture::position)positionPlayhead(e.position.x);
    else {setGestureModifiers(e.mods.isShiftDown(),e.mods.isCtrlDown());dragTo(e.position);}
}
void SwellTimeline::mouseUp(const juce::MouseEvent& event)
{
    if(drag!=Drag::none && timingDraft && event.position!=lastPointer)dragTo(event.position);
    if(drag!=Drag::none && timingDraft)
    {
        const auto requested=timingDraft->requested;
        const auto snapping=timingDraft->snapping;
        submitDrag(requested,snapping,0);
    }
    endPointerGesture();
}
void SwellTimeline::positionPlayhead(float at)
{
    localPlayhead=view.sample(std::clamp(at,source.getX(),source.getRight()),source.getX(),source.getWidth());
    repaint();
}
void SwellTimeline::panFrom(SwellViewport initial,double fraction)
{
    view=initial;view.shift(fraction);constrainNavigation();navigationChanged();
}
void SwellTimeline::mouseDoubleClick(const juce::MouseEvent& e)
{
    if(!waveformButton(e.mods,juce::ModifierKeys::leftButtonModifier) || !editingEnabled || data.sessionEpoch!=engine.assignmentEpoch())return;
    setGestureModifiers(e.mods.isShiftDown(),e.mods.isCtrlDown());
    if(!source.contains(e.position))return;
    const auto identity=targetIdentity();
    for(const auto& c:data.candidates)if(std::abs(e.position.x-x(c.sample))<7){engine.confirmCandidateAt(c.sample,identity);return;}
    for(const auto& w:data.session.swells)if(std::abs(e.position.x-x(w.arrival))<7){engine.selectTargets({w.id},w.id);return;}
    const auto candidate=view.sample(e.position.x,source.getX(),source.getWidth());
    if(effectiveSnap())
    {auto grid=makeGrid();
        if(clockContext.playing){const auto latest=engine.transportClockSnapshot();
            if(!clockContext.sameAuthority(gridClockContext(latest,true)) || !latest
                || !HostGrid::make(gridTiming(),latest->observed,capture.manifest.sampleRate,gridDivision,nullptr,playheadAvailable,&clockContext))grid.reset();}
        const auto at=grid?grid->snap(candidate):std::optional<int64_t>{};
        if(!at){gestureMessage("Snap unavailable: compatible fixed host timing required. Hold Ctrl or turn Snap off.");return;}engine.addTarget(*at,nullptr,identity);}
    else engine.addTarget(candidate,nullptr,identity);
}
void SwellTimeline::mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel)
{
    if((source.contains(e.position) || wet.contains(e.position)) && ui::controls::zoomWaveform(view,wheel.deltaY,
        (e.position.x-source.getX())/source.getWidth(),navigationBounds(),minimumViewSpan()))navigationChanged();
}
void SwellTimeline::navigateOverview(double fraction)
{if(!std::isfinite(fraction))return;const auto [a,b]=globalBounds();const auto at=a+std::clamp(fraction,0.0,1.0)*(b-a),size=view.span();view.first=at-size/2;view.last=at+size/2;constrainNavigation();navigationChanged();}
SwellOverview::SwellOverview(SwellTimeline& value):timeline(value)
{
    setTooltip("Left-drag the Overview to pan the detail waveform at its current zoom.");
    setName("songOverview");addAndMakeVisible(heading);heading.setName("overviewHeading");heading.setText("Overview",juce::dontSendNotification);
    ui::applyTextRole(heading,ui::TextRole::sectionHeading);heading.setInterceptsMouseClicks(false,false);
}
void SwellOverview::mouseDown(const juce::MouseEvent& e)
{
    panning=waveformBounds().contains(e.position) && waveformButton(e.mods,juce::ModifierKeys::leftButtonModifier);
    if(!panning)return;
    panStart=timeline.view;panEpoch=timeline.data.sessionEpoch;
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
}
void SwellOverview::mouseDrag(const juce::MouseEvent& e)
{
    const auto width=waveformBounds().getWidth();
    if(!panning || !waveformButton(e.mods,juce::ModifierKeys::leftButtonModifier) || width<=0
        || panEpoch!=timeline.data.sessionEpoch || panEpoch!=timeline.engine.assignmentEpoch())return;
    const auto [first,last]=timeline.globalBounds();
    timeline.panFrom(panStart,static_cast<double>(e.getDistanceFromDragStartX())/width*(last-first)/panStart.span());
    repaint();
}
void SwellOverview::mouseUp(const juce::MouseEvent& e)
{
    panning=false;mouseMove(e);
}
void SwellOverview::mouseMove(const juce::MouseEvent& e)
{
    setMouseCursor(waveformBounds().contains(e.position)?juce::MouseCursor::LeftRightResizeCursor:juce::MouseCursor::NormalCursor);
}
void SwellOverview::resized()
{
    if(timeline.responsivePresentation){heading.setBounds(getLocalBounds().removeFromLeft(ui::responsiveGeometry::overviewHeadingWidth).withSizeKeepingCentre(ui::responsiveGeometry::overviewHeadingWidth,ui::responsiveGeometry::headingHeight));return;}
    heading.setBounds(getLocalBounds().removeFromTop(ui::controlGeometry::headingHeight));
}
juce::Rectangle<float> SwellOverview::detailBounds()const
{
    const auto lane=waveformBounds();const auto [a,b]=timeline.globalBounds();
    if(lane.getWidth()<=0)return {};
    const auto pixel=[&](double sample){return std::clamp(lane.getX()+static_cast<float>((sample-a)/(b-a))*lane.getWidth(),lane.getX(),lane.getRight());};
    const auto left=std::min(pixel(timeline.view.first),lane.getRight()-1),right=std::max(pixel(timeline.view.last),left+1);
    return {left,lane.getY(),std::max(1.0f,right-left),lane.getHeight()};
}
void SwellOverview::paintWaveforms(juce::Graphics& g,ui::WaveformPaintMetrics& metrics)
{
    const auto lane=waveformBounds();const auto [a,b]=timeline.globalBounds();
    const auto x=[&](double position){return lane.getX()+static_cast<float>((position-a)/(b-a))*lane.getWidth();};
    for(const auto& item:timeline.capture.overview)
    {
        const auto left=x(static_cast<double>(item.range.startSample)),right=x(static_cast<double>(item.range.startSample)+item.range.frameCount);
        g.setColour(!item.available?ui::unavailableOverview:item.range.kind==CaptureCoverageKind::silence?ui::capturedSilence:ui::background);
        g.fillRect(juce::Rectangle<float>(left,lane.getY(),std::max(1.0f,right-left),lane.getHeight()));
        g.setColour(ui::secondary);
        if(item.available)
        {
            const auto columns=ui::waveformColumns(item.peaks,a,b,static_cast<int>(std::ceil(lane.getWidth())),0,
                [](const auto& p){return std::pair{p.minimum,p.maximum};},metrics.peaksVisited);
            for(size_t i=0;i<columns.size();++i)if(columns[i].present)
            {g.drawVerticalLine(static_cast<int>(lane.getX())+static_cast<int>(i),lane.getY()+lane.getHeight()*.3f-columns[i].maximum*lane.getHeight()*.25f,lane.getY()+lane.getHeight()*.3f-columns[i].minimum*lane.getHeight()*.25f+1);++metrics.columnsPainted;}
        }
    }
    for(const auto& swell:timeline.data.session.swells)
    {
        const auto summary=timeline.data.wetSummaries.find(swell.id);
        const auto colour=swell.id==timeline.data.session.primary?ui::aqua:ui::secondary;
        g.setColour(colour.withAlpha(swell.enabled && !swell.muted?.85f:.3f));
        if(summary==timeline.data.wetSummaries.end() || !summary->second)
        {const auto at=x(static_cast<double>(swell.arrival));g.drawVerticalLine(static_cast<int>(at),lane.getBottom()-10,lane.getBottom()-2);continue;}
        const auto& wave=*summary->second;int64_t start,end;if(!requestedShapeBounds(swell,wave.start,start,end))continue;
        const auto left=std::max(lane.getX(),x(static_cast<double>(start))),right=std::min(lane.getRight(),x(static_cast<double>(end)));
        if(right<=left)continue;
        juce::Graphics::ScopedSaveState clip(g);g.reduceClipRegion(juce::Rectangle<float>(left,lane.getY(),right-left,lane.getHeight()).getSmallestIntegerContainer());
        const auto scale=1.0/std::max(1.0,static_cast<double>(wave.peak)*juce::Decibels::decibelsToGain(swell.shape.gainDb));
        const auto columns=ui::waveformColumns(wave.peaks,a,b,static_cast<int>(std::ceil(lane.getWidth())),swell.placement,
            [&](const auto& peak)
            {
                int64_t first,last;if(!checkedAdd(peak.startSample,swell.placement,first) || !checkedAdd(first,peak.frameCount,last))return std::pair{0.f,0.f};
                first=std::max(first,start);last=std::min(last,end);if(last<=first)return std::pair{0.f,0.f};
                const auto middle=first+(last-first)/2;
                const auto factor=std::max({envelope(swell.shape,first,start,end),envelope(swell.shape,middle,start,end),envelope(swell.shape,last-1,start,end)});
                const auto amplitude=static_cast<float>(std::clamp(std::max(std::abs(peak.minimum),std::abs(peak.maximum))*factor*scale,0.0,1.0));
                return std::pair{0.f,amplitude};
            },metrics.peaksVisited);
        for(size_t i=0;i<columns.size();++i)if(columns[i].present && columns[i].maximum>0)
        {
            const auto height=columns[i].maximum*lane.getHeight()*.4f;
            g.fillRect(lane.getX()+static_cast<float>(i),lane.getBottom()-2-height,1.0f,std::max(1.0f,height));++metrics.columnsPainted;
        }
    }
}
void SwellOverview::paint(juce::Graphics& g)
{
    g.fillAll(ui::background);

    const auto lane=waveformBounds();const auto [a,b]=timeline.globalBounds();
    g.setColour(ui::outline);g.drawRoundedRectangle(lane,6,1);
    if(timeline.capture.manifest.ranges.empty())
    {g.setColour(ui::secondary);g.setFont(ui::fontFor(*this,ui::bodySize));g.drawText("No audio mapped yet",lane,juce::Justification::centred);return;}
    if(timeline.time.valid)
    {
        const auto& t=timeline.gridTiming();
        for(const auto& tick:makeSongBarRuler(a,b,lane.getWidth(),t.rate,t.bpm,t.numerator,t.denominator,t.sample,t.ppq))
        {const auto at=lane.getX()+static_cast<float>((tick.samplePosition-a)/(b-a))*lane.getWidth();
            g.setColour(ui::secondary);g.setFont(ui::fontFor(*this,ui::smallSize));g.drawText(juce::String(tick.bar)+".1",static_cast<int>(std::clamp(at+2,lane.getX(),lane.getRight()-48)),timeline.responsivePresentation?2:27,48,timeline.responsivePresentation?14:17,juce::Justification::left);
            g.drawVerticalLine(static_cast<int>(at),43,49);}
    }
    const auto x=[&](double position){return lane.getX()+static_cast<float>((position-a)/(b-a))*lane.getWidth();};
    if(waveformRevision!=timeline.waveformRevision){waveformRevision=timeline.waveformRevision;waveformLayer.invalidate();}
    waveformLayer.paint(g,getWidth(),getHeight(),a,b,[this](auto& drawing,float,auto& metrics){paintWaveforms(drawing,metrics);});
    const auto viewport=detailBounds();
    g.setColour(ui::aqua.withAlpha(.12f));g.fillRoundedRectangle(viewport,5);g.setColour(ui::aqua);g.drawRoundedRectangle(viewport,5,2);
    if(const auto position=timeline.transportPlayhead()){const auto play=x(static_cast<double>(*position));if(play>=lane.getX() && play<=lane.getRight()){
        g.setColour(ui::timing);g.drawVerticalLine(static_cast<int>(play),lane.getY(),lane.getBottom());}}
    if(timeline.data.auditionSample){const auto play=x(static_cast<double>(*timeline.data.auditionSample));if(play>=lane.getX() && play<=lane.getRight()){
        g.setColour(ui::previewCursor);g.drawVerticalLine(static_cast<int>(play),lane.getY(),lane.getBottom());}}

}
}
