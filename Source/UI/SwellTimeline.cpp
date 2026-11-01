#include "SwellTimeline.h"
#include "SongBarRuler.h"

namespace m3
{
SwellTimeline::SwellTimeline(SwellEngine& e):engine(e)
{setName("swellTimeline");scrollbar.addListener(this);addAndMakeVisible(scrollbar);setMouseCursor(juce::MouseCursor::CrosshairCursor);}
void SwellTimeline::setData(CaptureSessionSnapshot c,EditorSnapshot s,SongTime t,bool available)
{capture=std::move(c);data=std::move(s);time=t;playheadAvailable=available;updateScroll();repaint();}
std::pair<double,double> SwellTimeline::globalBounds() const
{
    double a=0,b=time.rate>0?time.rate*8:384000;
    if(!capture.manifest.ranges.empty()) {a=std::min(a,static_cast<double>(capture.manifest.ranges.front().startSample));const auto& r=capture.manifest.ranges.back();b=std::max(b,static_cast<double>(r.startSample)+r.frameCount);}
    for(const auto& w:data.session.swells)if(w.render.present()) {a=std::min(a,static_cast<double>(w.render.start)+w.placement);b=std::max(b,static_cast<double>(w.render.sourceEnd)+w.placement);}
    return {a,b};
}
void SwellTimeline::updateScroll()
{
    const auto [a,b]=globalBounds();updatingScroll=true;
    scrollbar.setRangeLimits(std::min(a-view.span(),view.first),std::max(b+view.span(),view.last));
    scrollbar.setCurrentRange(view.first,view.span(),juce::dontSendNotification);updatingScroll=false;
}
void SwellTimeline::scrollBarMoved(juce::ScrollBar*,double start)
{if(updatingScroll)return;const auto size=view.span();view.first=start;view.last=start+size;repaint();}
void SwellTimeline::fitSelection()
{
    if(data.raw)
    {
        auto a=data.raw->start,b=data.raw->end;
        if(const auto w=find(data.session,data.session.primary)){checkedAdd(a,w->placement,a);checkedAdd(b,w->placement,b);}
        if(const auto w=find(data.session,data.session.primary)){int64_t requestedA,requestedB;
            if(requestedShapeBounds(*w,data.raw->start,requestedA,requestedB)){a=std::min(a,requestedA);b=std::max(b,requestedB);}}
        view.fit(static_cast<double>(a),static_cast<double>(b));
    }
    else if(const auto w=find(data.session,data.session.primary)) view.fit(static_cast<double>(w->sourceStart),static_cast<double>(w->sourceEnd));
    else {const auto [a,b]=globalBounds();view.fit(a,b);}updateScroll();repaint();
}
void SwellTimeline::page(int bars){view.page(bars,time);updateScroll();repaint();}
void SwellTimeline::previous(){view.shift(-1);updateScroll();repaint();}
void SwellTimeline::next(){view.shift(1);updateScroll();repaint();}
void SwellTimeline::zoom(double factor){view.zoom(factor);updateScroll();repaint();}
void SwellTimeline::resized()
{
    auto area=getLocalBounds().toFloat().reduced(2);scrollbar.setBounds(area.removeFromBottom(16).toNearestInt());area.removeFromBottom(4);
    if(inlineOverview){overview=area.removeFromBottom(80);area.removeFromBottom(24);}
    else overview={-10000,-10000,0,0};
    area.removeFromTop(42);
    source=area.removeFromTop(std::max(40.0f,(area.getHeight()-32)/2));area.removeFromTop(32);wet=area;
    updateScroll();
}
void SwellTimeline::drawWave(juce::Graphics& g,const Waveform& wave,juce::Rectangle<float> lane,juce::Colour colour,float gain,int64_t placement)
{
    juce::Graphics::ScopedSaveState saved(g);g.reduceClipRegion(lane.toNearestInt());g.setColour(colour);
    for(const auto& p:wave.peaks)
    {
        const auto a=static_cast<float>(view.pixel(static_cast<double>(p.startSample)+placement,source.getX(),source.getWidth())),b=static_cast<float>(view.pixel(static_cast<double>(p.startSample)+placement+p.frameCount,source.getX(),source.getWidth()));if(b<lane.getX() || a>lane.getRight())continue;
        const auto top=lane.getCentreY()-std::clamp(p.maximum*gain,-1.0f,1.0f)*lane.getHeight()*0.42f;
        const auto bottom=lane.getCentreY()-std::clamp(p.minimum*gain,-1.0f,1.0f)*lane.getHeight()*0.42f;
        g.drawVerticalLine(static_cast<int>(a),top,std::max(top+1.0f,bottom));
    }
}
void SwellTimeline::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff10181c));g.setFont(12.0f);const auto [globalA,globalB]=globalBounds();
    const auto mini=[&](double sample){return overview.getX()+static_cast<float>((sample-globalA)/(globalB-globalA))*overview.getWidth();};
    g.setColour(juce::Colour(0xff10181c));g.fillRect(overview);g.fillRect(source);g.fillRect(wet);
    for(const auto& item:capture.overview)
    {
        const auto a=item.range.startSample,b=a+static_cast<int64_t>(item.range.frameCount);
        const auto colour=!item.available?juce::Colour(0xffa54444):item.range.kind==CaptureCoverageKind::silence?juce::Colour(0xff30383a):juce::Colour(0xff193332);
        g.setColour(colour);g.fillRect(juce::Rectangle<float>(mini(static_cast<double>(a)),overview.getY(),std::max(1.0f,mini(static_cast<double>(b))-mini(static_cast<double>(a))),overview.getHeight()));
        if(item.available)
        {
            g.setColour(juce::Colour(0xff83b9b5));
            for(const auto& peak:item.peaks)
            {const auto at=static_cast<int>(mini(static_cast<double>(peak.startSample)));g.drawVerticalLine(at,overview.getCentreY()-peak.maximum*25,overview.getCentreY()-peak.minimum*25+1);}
        }
        const auto left=std::max(source.getX(),x(a)),right=std::min(source.getRight(),x(b));if(right<=left)continue;
        g.setColour(colour);g.fillRect(juce::Rectangle<float>(left,source.getY(),right-left,source.getHeight()));
        if(item.available && item.range.kind==CaptureCoverageKind::audio)
        {Waveform wave;wave.peaks=item.peaks;drawWave(g,wave,source,juce::Colour(0xffc6d3d0),1);}
        else if(item.available){g.setColour(juce::Colour(0xffa7b7ba));g.drawHorizontalLine(static_cast<int>(source.getCentreY()),left,right);}
    }
    if(data.sourceDetail)
    {
        // Replace coarse overview columns only inside the selected source interval.
        const auto left=std::max(source.getX(),x(data.sourceDetail->start)),right=std::min(source.getRight(),x(data.sourceDetail->end));
        if(right>left)
        {g.setColour(juce::Colour(0xff193332));g.fillRect(juce::Rectangle<float>(left,source.getY(),right-left,source.getHeight()));drawWave(g,*data.sourceDetail,source,juce::Colour(0xffc6d3d0),1);}
    }
    g.setColour(juce::Colours::white.withAlpha(0.8f));const auto miniLeft=std::clamp(mini(view.first),overview.getX(),overview.getRight()),miniRight=std::clamp(mini(view.last),overview.getX(),overview.getRight());
    g.drawRect(juce::Rectangle<float>(miniLeft,overview.getY(),std::max(1.0f,miniRight-miniLeft),overview.getHeight()),1);
    g.drawText("Song overview | last captured",overview.translated(0,-22).withHeight(20),juce::Justification::centredLeft);
    for(const auto& swell:data.session.swells)
    {const auto at=mini(static_cast<double>(swell.arrival));g.setColour(swell.id==data.session.primary?juce::Colour(0xff37d6cf):juce::Colour(0xffa4b6b6));g.drawVerticalLine(static_cast<int>(at),overview.getBottom()-13,overview.getBottom());}
    g.setColour(juce::Colour(0xffb7c4c9));g.drawText(data.session.auditionMode==AuditionMode::dryWet?"Source / arrivals | highlighted span: audition dry":"Source / arrivals   (double-click source to add a target)",source.translated(0,-24).withHeight(20),juce::Justification::centredLeft);
    const auto ticks=time.valid?makeSongBarRuler(view.first,view.last,source.getWidth(),time.rate,time.bpm,time.numerator,time.denominator,time.sample,time.ppq):std::vector<SongBarTick>{};
    for(const auto& tick:ticks)
    {
        const auto at=static_cast<float>(view.pixel(tick.samplePosition,source.getX(),source.getWidth()));
        g.setColour(juce::Colour(0x4083e2ce));g.drawVerticalLine(static_cast<int>(at),source.getY(),wet.getBottom());
        g.setColour(juce::Colour(0xffd6e1e4));g.drawText(juce::String(tick.bar),static_cast<int>(at)+2,static_cast<int>(source.getY())-42,48,16,juce::Justification::left);
    }
    std::set<int> candidateColumns;
    for(size_t i=0;i<data.candidates.size();++i)
    {
        const auto at=x(data.candidates[i].sample);if(at<source.getX() || at>source.getRight() || !candidateColumns.insert(static_cast<int>(at)/4).second)continue;
        juce::Path triangle;triangle.addTriangle(at-4,source.getY()+2,at+4,source.getY()+2,at,source.getY()+10);
        g.setColour(juce::Colour(0xff37d6cf));g.strokePath(triangle,juce::PathStrokeType(1));
    }
    std::set<int> markerColumns;const auto primary=find(data.session,data.session.primary);
    for(const auto& w:data.session.swells)
    {
        const auto at=x(w.arrival);if(at<source.getX() || at>source.getRight())continue;
        if(w.id==data.session.primary || (primary && std::abs(at-x(primary->arrival))<8) || !markerColumns.insert(static_cast<int>(at)/8).second)continue;
        const auto selected=std::find(data.session.selected.begin(),data.session.selected.end(),w.id)!=data.session.selected.end();
        g.setColour(selected?juce::Colour(0xfff3e3ff):juce::Colour(0xff9cacaa));g.drawVerticalLine(static_cast<int>(at),source.getY(),wet.getBottom());
        g.fillEllipse(at-4,source.getY()+12,8,8);g.drawText(w.name,juce::Rectangle<float>(at+6,source.getY()+10,120,18),juce::Justification::left);
    }
    if(primary && x(primary->arrival)>=source.getX() && x(primary->arrival)<=source.getRight())
    {const auto at=x(primary->arrival);g.setColour(juce::Colour(0xfff3e3ff));g.drawVerticalLine(static_cast<int>(at),source.getY(),wet.getBottom());g.fillEllipse(at-4,source.getY()+12,8,8);g.drawText(primary->name,juce::Rectangle<float>(at+6,source.getY()+10,120,18),juce::Justification::left);}
    for(const auto& [id,wave]:data.wetSummaries)if(id!=data.session.primary && wave)
        drawWave(g,*wave,wet,juce::Colour(0xff80938f),1.0f/std::max(1.0f,wave->peak),find(data.session,id)?find(data.session,id)->placement:0);
    if(data.raw)
    {
        const auto peak=std::max(data.raw->peak,data.shaped?data.shaped->peak:0.0f),gain=1.0f/std::max(1.0f,peak);
        drawWave(g,*data.raw,wet,juce::Colour(0xff63716e),gain,primary?primary->placement:0);
        const auto current=primary && primary->phase==Phase::ready;
        const auto colour=current?juce::Colour(0xff53d8d0):juce::Colour(0xffdba66f);
        if(data.shaped)drawWave(g,*data.shaped,wet,colour,gain);
        g.setColour(colour);g.drawText("Full wet / shaped overlay | "+juce::String(current?(data.shaped?"current":"shape pending"):"previous / stale")+" | display "+juce::String(juce::Decibels::gainToDecibels(gain),1)+" dB",wet.translated(0,-25).withHeight(20),juce::Justification::left);
    }
    else{g.setColour(juce::Colour(0xffb7c4c9));g.drawText(primary?primary->status:juce::String("Play source in Song mode to prepare swells"),wet,juce::Justification::centred);}
    if(data.dryAvailable && data.session.auditionMode==AuditionMode::dryWet){
        const auto a=std::clamp(x(data.auditionDryStart),source.getX(),source.getRight()),b=std::clamp(x(data.auditionDryEnd),source.getX(),source.getRight());
        if(b>a){g.setColour(juce::Colour(0x99ffb4e9));g.fillRect(a,source.getBottom()-5,b-a,5.0f);}}
    if(const auto w=find(data.session,data.session.primary))
    {
        if(renderInputEnd(*w)>w->sourceEnd)
        {
            juce::Graphics::ScopedSaveState clip(g);g.reduceClipRegion(source.toNearestInt());
            g.setColour(juce::Colour(0x60e6bb72));g.fillRect(x(w->sourceEnd),source.getY(),std::max(1.0f,x(renderInputEnd(*w))-x(w->sourceEnd)),6.0f);
        }
        g.setColour(juce::Colour(0xffa3d9e8));
        for(auto point:{w->sourceStart,w->sourceEnd})if(x(point)>=source.getX() && x(point)<=source.getRight())g.fillRect(x(point)-4,source.getBottom()-12,8.0f,12.0f);
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
                g.setColour(juce::Colour(0xff37d6cf));g.strokePath(envelopeLine,juce::PathStrokeType(2.0f));
                if(static_cast<long double>(b)-w->placement>data.raw->end){g.setColour(juce::Colour(0xffe6bb72));g.drawText("Requested wet support - capture through endpoint",wet.withHeight(22),juce::Justification::left);}
                g.setColour(juce::Colour(0xff7ba5a0));g.drawText("Drag wet body to move audio + envelope",wet.withY(wet.getBottom()-22).withHeight(20),juce::Justification::centred);
                const auto handles=grabPoints();for(const auto& handle:handles)
                {
                    const auto active=hover && *hover==handle.kind;
                    if(handle.point.getDistanceFrom(handle.anchor)>1){g.setColour(juce::Colour(0xff9db2af));g.drawLine({handle.anchor,handle.point},1);
                        const juce::String label=handle.kind==EnvelopePoint::fadeIn?"Fade in":handle.kind==EnvelopePoint::fadeOut?"Fade out":handle.kind==EnvelopePoint::start?"Start":handle.kind==EnvelopePoint::end?"End":"Tension";
                        if(handle.kind==EnvelopePoint::fadeIn || handle.kind==EnvelopePoint::fadeOut){
                            const std::array<juce::Rectangle<float>,4> labels{{{handle.point.x+12,handle.point.y-7,56,15},
                                {handle.point.x-68,handle.point.y-7,56,15},{handle.point.x+12,handle.point.y+16,56,15},{handle.point.x+12,handle.point.y-34,56,15}}};
                            for(const auto& box:labels)if(wet.contains(box) && std::none_of(handles.begin(),handles.end(),[&](const auto& other){
                                return box.expanded(2).intersects(juce::Rectangle<float>(16,16).withCentre(other.point));})){
                                g.setColour(juce::Colour(0xdd10181c));g.fillRoundedRectangle(box.expanded(1),2);
                                g.setColour(juce::Colour(0xffb7ceca));g.drawText(label,box,juce::Justification::left);break;}}
                        g.fillEllipse(juce::Rectangle<float>(4,4).withCentre(handle.anchor));}const auto r=juce::Rectangle<float>(14,14).withCentre(handle.point);
                    g.setColour(active?juce::Colours::white:juce::Colour(0xff37d6cf));
                    if(handle.kind==EnvelopePoint::inTension || handle.kind==EnvelopePoint::outTension)
                    {juce::Path diamond;diamond.startNewSubPath(r.getCentreX(),r.getY());diamond.lineTo(r.getRight(),r.getCentreY());diamond.lineTo(r.getCentreX(),r.getBottom());diamond.lineTo(r.getX(),r.getCentreY());diamond.closeSubPath();g.fillPath(diamond);}
                    else if(handle.kind==EnvelopePoint::start || handle.kind==EnvelopePoint::end)g.fillRoundedRectangle(r,2);
                    else g.fillEllipse(r);
                    g.setColour(juce::Colour(0xff10181c));g.drawEllipse(r.reduced(3),1);
                    if(active){const juce::String text=handle.kind==EnvelopePoint::end?"Wet end (gap / overlap)":handle.kind==EnvelopePoint::start?"Wet start":handle.kind==EnvelopePoint::inTension?"Fade-in tension":handle.kind==EnvelopePoint::outTension?"Fade-out tension":handle.kind==EnvelopePoint::fadeIn?"Fade-in duration":"Fade-out duration";
                        g.setColour(juce::Colours::white);g.drawText(text,wet.toNearestInt().withHeight(22),juce::Justification::right);}
                }
            }
        }
    }
    if(playheadAvailable && x(time.sample)>=source.getX() && x(time.sample)<=source.getRight())
    {g.setColour(juce::Colour(0xfff6dc84));g.drawVerticalLine(static_cast<int>(x(time.sample)),source.getY(),wet.getBottom());}
    if(data.auditionSample){const auto play=x(*data.auditionSample);if(play>=source.getX() && play<=source.getRight()){
        g.setColour(juce::Colour(0xffffb4e9));g.drawVerticalLine(static_cast<int>(play),source.getY(),wet.getBottom());
        g.drawText("Audition",juce::Rectangle<float>(play+4,source.getY()+3,65,16),juce::Justification::left);}}

}
bool SwellTimeline::beginDrag(juce::Point<float> point)
{
    drag=Drag::none;
    if(overview.contains(point))
    {
        drag=Drag::overview;dragTo(point);return true;
    }
    if(const auto w=find(data.session,data.session.primary))
    {
        dragId=w->id;
        if(source.contains(point) && point.y>source.getBottom()-18)
        {if(std::abs(point.x-x(w->sourceStart))<7)drag=Drag::sourceStart;else if(std::abs(point.x-x(w->sourceEnd))<7)drag=Drag::sourceEnd;}
        if(wet.expanded(8).contains(point) && data.raw)
        {
            if(const auto hit=hitEnvelope(grabPoints(),point))
            {
                hover=hit;dragOrigin=point;initialFade=*hit==EnvelopePoint::fadeIn?w->shape.fadeIn:w->shape.fadeOut;initialTension=*hit==EnvelopePoint::inTension?w->shape.inTension:w->shape.outTension;
                int64_t a,b;if(!requestedShapeBounds(*w,data.raw->start,a,b))return false;
                initialSample=*hit==EnvelopePoint::start?a:b;
                initialEnvelope=w->shape;
                switch(*hit){case EnvelopePoint::start:drag=Drag::audibleStart;break;case EnvelopePoint::end:drag=Drag::audibleEnd;break;
                    case EnvelopePoint::fadeIn:drag=Drag::fadeIn;break;case EnvelopePoint::fadeOut:drag=Drag::fadeOut;break;
                    case EnvelopePoint::inTension:drag=Drag::inTension;break;case EnvelopePoint::outTension:drag=Drag::outTension;break;}
            }
        }
        if(drag==Drag::none && wet.contains(point) && data.raw)
        {int64_t a,b;if(requestedShapeBounds(*w,data.raw->start,a,b) && point.x>=x(a) && point.x<=x(b))
            {drag=Drag::move;dragOrigin=point;initialPlacement=w->placement;hover.reset();}}
        if(drag!=Drag::none)return true;
    }
    if(source.contains(point))
    {
        for(const auto& w:data.session.swells)if(std::abs(point.x-x(w.arrival))<7)
        {engine.selectTargets({w.id},w.id);dragId=w.id;drag=Drag::arrival;return true;}
        for(size_t i=0;i<data.candidates.size();++i)if(std::abs(point.x-x(data.candidates[i].sample))<7)
        {if(onCandidateSelected)onCandidateSelected(static_cast<int>(i));return true;}
    }
    return false;
}
void SwellTimeline::dragTo(juce::Point<float> point)
{
    if(drag==Drag::overview)
    {const auto [a,b]=globalBounds();const auto at=a+std::clamp((point.x-overview.getX())/overview.getWidth(),0.0f,1.0f)*(b-a),size=view.span();view.first=at-size/2;view.last=at+size/2;updateScroll();repaint();return;}
    if(drag==Drag::none)return;
    const auto editPoint=fineDrag?dragOrigin+(point-dragOrigin)*.1f:point;
    const auto coarse=drag==Drag::arrival || drag==Drag::sourceStart || drag==Drag::sourceEnd;
    const auto sample=view.snap(view.sample(editPoint.x,source.getX(),source.getWidth()),time,coarse && snapEnabled);
    const auto raw=data.raw;const auto kind=drag;
    engine.edit(dragId,[&](Swell& w,juce::String& error)
    {
        if(kind==Drag::arrival)return moveArrival(w,sample,error);
        if(kind==Drag::sourceStart)return setSource(w,std::min(sample,w.arrival),w.sourceEnd,error);
        if(kind==Drag::sourceEnd){int64_t minimum;if(!checkedAdd(w.arrival,1,minimum))return false;return setSource(w,w.sourceStart,std::max(sample,minimum),error);}
        if(kind==Drag::move){const auto delta=static_cast<long double>(editPoint.x-dragOrigin.x)*view.span()/source.getWidth();
            const auto moved=static_cast<long double>(initialPlacement)+std::round(delta);
            if(!std::isfinite(moved) || moved< -9223372036854775808.0L || moved>=9223372036854775808.0L){error="Wet placement overflow";return false;}return moveWet(w,static_cast<int64_t>(moved),error);}
        if(!raw)return false;auto shape=w.shape;int64_t a,b;
        if(!requestedShapeBounds(w,raw->start,a,b))return false;
        int64_t positionedStart;if(!checkedAdd(raw->start,w.placement,positionedStart))return false;
        const auto toOffset=[&](int64_t at,int64_t& out){int64_t logical;return checkedSubtract(at,w.placement,logical) && checkedSubtract(logical,w.arrival,out);};
        if(kind==Drag::audibleStart || kind==Drag::audibleEnd){
            shape=initialEnvelope;
            const auto moved=static_cast<long double>(initialSample)+std::round(static_cast<long double>(editPoint.x-dragOrigin.x)*view.span()/source.getWidth());
            if(!std::isfinite(moved) || moved< -9223372036854775808.0L || moved>=9223372036854775808.0L){error="Wet endpoint overflow";return false;}
            const auto endpoint=static_cast<int64_t>(moved);
            if(kind==Drag::audibleStart){if(positionedStart>=b){error="Wet start has no valid support before its end";return false;}shape.automaticStart=false;if(!toOffset(std::clamp(endpoint,positionedStart,b-1),shape.startOffset))return false;}
            else if(!toOffset(std::max(endpoint,a+1),shape.endOffset))return false;
        }
        const auto durationDelta=static_cast<long double>(editPoint.x-dragOrigin.x)*view.span()/source.getWidth();
        if(kind==Drag::fadeIn || kind==Drag::fadeOut){const auto duration=std::clamp(static_cast<long double>(initialFade)+(kind==Drag::fadeIn?durationDelta:-durationDelta),0.0L,static_cast<long double>(b)-a-1);
            if(kind==Drag::fadeIn)shape.fadeIn=static_cast<int64_t>(std::llround(duration));else shape.fadeOut=static_cast<int64_t>(std::llround(duration));}

        if(kind==Drag::inTension)shape.inTension=std::clamp(initialTension+(editPoint.y-dragOrigin.y)/80.0,-1.0,1.0);
        if(kind==Drag::outTension)shape.outTension=std::clamp(initialTension+(editPoint.y-dragOrigin.y)/80.0,-1.0,1.0);
        return setShape(w,shape,raw->start,raw->end,error);
    });
    // Canonical metadata only: no waveform preparation or FFT on the message thread.
    data.session=engine.savedSession();repaint();if(onEnvelopeEdited)onEnvelopeEdited();
}
std::vector<EnvelopeGrab> SwellTimeline::grabPoints() const
{const auto w=find(data.session,data.session.primary);return w && data.raw?envelopeHandles(*w,data.raw->start,view,wet):std::vector<EnvelopeGrab>{};}
void SwellTimeline::mouseMove(const juce::MouseEvent& e){const auto hit=hitEnvelope(grabPoints(),e.position);if(hit!=hover){hover=hit;repaint();}}
bool SwellTimeline::keyPressed(const juce::KeyPress& key)
{
    if(key.getKeyCode()!=juce::KeyPress::leftKey && key.getKeyCode()!=juce::KeyPress::rightKey)return false;
    for(const auto& p:grabPoints())if(hover && p.kind==*hover){if(!beginDrag(p.point))return false;
        const auto step=key.getKeyCode()==juce::KeyPress::leftKey?-1:1;
        if(p.kind==EnvelopePoint::inTension || p.kind==EnvelopePoint::outTension)dragTo(p.point+juce::Point<float>{0,static_cast<float>(step)});
        else dragTo(p.point+juce::Point<float>{static_cast<float>(view.pixel(view.first+step,source.getX(),source.getWidth())-view.pixel(view.first,source.getX(),source.getWidth())),0});
        drag=Drag::none;return true;}return false;
}
void SwellTimeline::mouseDown(const juce::MouseEvent& e){setWantsKeyboardFocus(true);grabKeyboardFocus();fineDrag=e.mods.isShiftDown();beginDrag(e.position);repaint();}
void SwellTimeline::mouseDrag(const juce::MouseEvent& e){fineDrag=e.mods.isShiftDown();dragTo(e.position);}
void SwellTimeline::mouseDoubleClick(const juce::MouseEvent& e)
{
    if(!source.contains(e.position))return;
    for(const auto& c:data.candidates)if(std::abs(e.position.x-x(c.sample))<7){engine.confirmCandidateAt(c.sample);return;}
    for(const auto& w:data.session.swells)if(std::abs(e.position.x-x(w.arrival))<7){engine.selectTargets({w.id},w.id);return;}
    engine.addTarget(view.snap(view.sample(e.position.x,source.getX(),source.getWidth()),time,snapEnabled));
}
void SwellTimeline::mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel)
{if(e.mods.isCtrlDown())view.zoom(wheel.deltaY>0?0.8:1.25,(e.position.x-source.getX())/source.getWidth());else view.shift(-wheel.deltaY*0.5);updateScroll();repaint();}
void SwellTimeline::navigateOverview(double fraction)
{const auto [a,b]=globalBounds();const auto at=a+std::clamp(fraction,0.0,1.0)*(b-a),size=view.span();view.first=at-size/2;view.last=at+size/2;updateScroll();repaint();}
void SwellOverview::paint(juce::Graphics& g)
{
    const auto panel=getLocalBounds().toFloat().reduced(1);g.setColour(juce::Colour(0xff10181c));g.fillRoundedRectangle(panel,7);
    g.setColour(juce::Colour(0xff526064));g.drawRoundedRectangle(panel,7,1);
    g.setColour(juce::Colour(0xffdde7e8));g.setFont(juce::FontOptions(17,juce::Font::bold));g.drawText("Song Overview",12,5,getWidth()-24,25,juce::Justification::left);
    const auto lane=panel.reduced(12,8).withTrimmedTop(24);const auto [a,b]=timeline.globalBounds();
    const auto x=[&](double position){return lane.getX()+static_cast<float>((position-a)/(b-a))*lane.getWidth();};
    for(const auto& item:timeline.capture.overview)
    {
        const auto left=x(static_cast<double>(item.range.startSample)),right=x(static_cast<double>(item.range.startSample)+item.range.frameCount);
        g.setColour(!item.available?juce::Colour(0xff773d3d):item.range.kind==CaptureCoverageKind::silence?juce::Colour(0xff3c484c):juce::Colour(0xff193b3c));
        g.fillRect(juce::Rectangle<float>(left,lane.getY(),std::max(1.0f,right-left),lane.getHeight()));
        g.setColour(juce::Colour(0xffb4c5c3));
        for(const auto& peak:item.peaks)
        {const auto at=static_cast<int>(x(static_cast<double>(peak.startSample)));g.drawVerticalLine(at,lane.getY()+lane.getHeight()*.3f-peak.maximum*lane.getHeight()*.25f,lane.getY()+lane.getHeight()*.3f-peak.minimum*lane.getHeight()*.25f+1);}
    }
    for(const auto& swell:timeline.data.session.swells)
    {
        const auto at=x(static_cast<double>(swell.arrival)+swell.placement);g.setColour(swell.id==timeline.data.session.primary?juce::Colour(0xff37d6cf):juce::Colour(0xff8c9f9c));
        juce::Path shape;const auto start=swell.render.present()?x(static_cast<double>(swell.render.start)+swell.placement):at-8;
        shape.startNewSubPath(start,lane.getBottom()-2);shape.quadraticTo(at-3,lane.getBottom()-2,at,lane.getY()+lane.getHeight()*.6f);shape.lineTo(at,lane.getBottom()-2);shape.closeSubPath();g.fillPath(shape);
    }
    const auto left=std::clamp(x(timeline.view.first),lane.getX(),lane.getRight()),right=std::clamp(x(timeline.view.last),lane.getX(),lane.getRight());
    const juce::Rectangle<float> viewport(left,lane.getY(),std::max(1.0f,right-left),lane.getHeight());
    g.setColour(juce::Colour(0x2437d6cf));g.fillRoundedRectangle(viewport,5);g.setColour(juce::Colour(0xff37d6cf));g.drawRoundedRectangle(viewport,5,2);
    if(timeline.playheadAvailable){const auto play=x(static_cast<double>(timeline.time.sample));if(play>=lane.getX() && play<=lane.getRight()){
        g.setColour(juce::Colour(0xfff6dc84));g.drawVerticalLine(static_cast<int>(play),lane.getY(),lane.getBottom());}}
    if(timeline.data.auditionSample){const auto play=x(static_cast<double>(*timeline.data.auditionSample));if(play>=lane.getX() && play<=lane.getRight()){
        g.setColour(juce::Colour(0xffffb4e9));g.drawVerticalLine(static_cast<int>(play),lane.getY(),lane.getBottom());}}

}
}
