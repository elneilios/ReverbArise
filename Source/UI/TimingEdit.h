#pragma once
#include <DSP/SwellShape.h>
#include <Core/HostGrid.h>

namespace m3
{
enum class TimingAnchor {arrival,sourceStart,sourceEnd,start,end,fadeIn,fadeOut,move};
inline std::optional<int64_t> timingAnchorSample(const Swell& w,TimingAnchor kind,int64_t rawStart)
{
    if(kind==TimingAnchor::arrival)return w.arrival;
    if(kind==TimingAnchor::sourceStart)return w.sourceStart;
    if(kind==TimingAnchor::sourceEnd)return w.sourceEnd;
    int64_t a,b;if(!requestedShapeBounds(w,rawStart,a,b))return {};
    if(kind==TimingAnchor::end)return b;
    if(kind==TimingAnchor::fadeIn)return a+std::min(w.shape.fadeIn,b-a-1);
    if(kind==TimingAnchor::fadeOut)return w.shape.fadeOut==0?b:b-1-std::min(w.shape.fadeOut,b-a-1);
    return a;
}
// Shared canvas/inspector candidate policy. Callers retain invocation ownership
// and model transaction guards; no geometry, UI preference or worker authority.
inline bool applyTimingCandidate(Swell& w,const Swell& anchor,TimingAnchor kind,int64_t requested,
    int64_t rawStart,int64_t rawEnd,bool snapping,const HostGrid* grid,long double origin,juce::String& error)
{
    if(snapping && !grid){error="Snap unavailable: compatible fixed host timing required. Hold Ctrl or turn Snap off.";return false;}
    const auto quantize=[&](int64_t at,int64_t lower,int64_t upper)->std::optional<int64_t>
    {if(lower>upper)return {};return snapping?grid->snapWithin(at,lower,upper):std::optional<int64_t>{std::clamp(at,lower,upper)};};
    const auto candidate=[&](int64_t lower,int64_t upper){auto at=quantize(requested,lower,upper);if(!at)error="No valid grid point within this handle's timing bounds";return at;};
    if(kind==TimingAnchor::arrival){const auto at=candidate(INT64_MIN,INT64_MAX);return at && moveArrival(w,*at,error);}
    if(kind==TimingAnchor::sourceStart){const auto at=candidate(INT64_MIN,w.arrival);return at && setSource(w,*at,w.sourceEnd,error);}
    if(kind==TimingAnchor::sourceEnd){int64_t minimum;if(!checkedAdd(w.arrival,1,minimum))return false;const auto at=candidate(minimum,INT64_MAX);return at && setSource(w,w.sourceStart,*at,error);}
    if(kind==TimingAnchor::move)
    {const auto initial=timingAnchorSample(anchor,kind,rawStart),at=candidate(INT64_MIN,INT64_MAX);int64_t delta,placement;
        return initial && at && checkedSubtract(*at,*initial,delta) && checkedAdd(anchor.placement,delta,placement) && moveWet(w,placement,error);}
    auto shape=w.shape;int64_t a,b,positionedStart;
    if(!requestedShapeBounds(w,rawStart,a,b) || !checkedAdd(rawStart,w.placement,positionedStart))return false;
    const auto toOffset=[&](int64_t at,int64_t& out){int64_t logical;return checkedSubtract(at,w.placement,logical) && checkedSubtract(logical,w.arrival,out);};
    if(kind==TimingAnchor::start || kind==TimingAnchor::end)
    {shape=anchor.shape;const auto at=kind==TimingAnchor::start?candidate(positionedStart,b-1):candidate(a+1,INT64_MAX);if(!at)return false;
        if(kind==TimingAnchor::start){shape.automaticStart=false;if(!toOffset(*at,shape.startOffset))return false;}else if(!toOffset(*at,shape.endOffset))return false;}
    if(kind==TimingAnchor::fadeIn){const auto at=candidate(a,b-1);if(!at || !checkedSubtract(*at,a,shape.fadeIn))return false;}
    if(kind==TimingAnchor::fadeOut)
    {
        auto at=quantize(requested,a,b);
        if(snapping)
        {at=b-a>1?quantize(requested,a,b-2):std::optional<int64_t>{};const auto zero=quantize(b,b,b);
            if(zero && (!at || std::abs(static_cast<long double>(*zero)-requested)<std::abs(static_cast<long double>(*at)-requested)
                || (std::abs(static_cast<long double>(*zero)-requested)==std::abs(static_cast<long double>(*at)-requested) && std::abs(*zero-origin)>std::abs(*at-origin))))at=zero;}
        if(!at){error="No valid fade-out grid point";return false;}shape.fadeOut=*at>=b-1?0:b-1-*at;
    }
    return setShape(w,shape,rawStart,rawEnd,error);
}
}
