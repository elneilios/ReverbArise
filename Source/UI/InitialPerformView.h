#pragma once
#include <Core/SwellSession.h>
#include "SwellViewport.h"
#include "Controls/SwellListOrder.h"

namespace m3::ui
{
inline juce::String firstChronologicalSwell(const Session& session)
{
    if(session.swells.empty())return {};
    const auto order=controls::chronologicalSwellOrder(std::span<const Swell>{session.swells});
    return session.swells[order.front()].id;
}
inline std::pair<double,double> initialSwellBounds(const Swell& swell,const SongTime& timing)
{
    int64_t lead=swell.sourceStart,end=swell.sourceEnd;
    if(swell.phase==Phase::ready && swell.render.present() && swell.render.irContent==swell.ir.content)
        lead=swell.render.start;
    else if(swell.ir.present() && swell.ir.rate>0 && timing.rate>0)
    {
        const auto frames=std::round(swell.ir.frames*timing.rate/swell.ir.rate);
        int64_t predicted;
        if(std::isfinite(frames) && frames>=1 && frames<static_cast<double>(INT64_MAX)
            && checkedSubtract(swell.sourceStart,static_cast<int64_t>(frames)-1,predicted))lead=predicted;
    }
    else if(timing.valid && timing.rate>0 && timing.bpm>0)
    {
        const auto quarter=timing.rate*60/timing.bpm;int64_t predicted;
        if(std::isfinite(quarter) && quarter>=1 && quarter<static_cast<double>(INT64_MAX)
            && checkedSubtract(swell.arrival,static_cast<int64_t>(std::llround(quarter)),predicted))lead=std::min(lead,predicted);
    }
    int64_t shapedFirst,shapedLast;
    if(requestedShapeBounds(swell,lead,shapedFirst,shapedLast)){lead=std::min(lead,shapedFirst);end=std::max(end,shapedLast);}
    lead=std::min(lead,swell.arrival);
    if(end<=swell.arrival && swell.arrival<INT64_MAX)end=swell.arrival+1;
    return {static_cast<double>(lead),static_cast<double>(end)};
}
inline SwellViewport revealInitialSwell(SwellViewport current,const Swell& swell,const SongTime& timing)
{
    const auto [first,last]=initialSwellBounds(swell,timing);SwellViewport framed=current;framed.fit(first,last);
    // Domain navigation policy: do not leave a small target almost invisible in
    // an arrangement-wide view. Ordinary useful spans retain their exact zoom.
    constexpr double maximumContextRatio=8;
    if(!std::isfinite(current.span()) || current.span()<framed.span() || current.span()>framed.span()*maximumContextRatio)return framed;
    const auto delta=current.first>framed.first?framed.first-current.first:current.last<framed.last?framed.last-current.last:0;
    current.first+=delta;current.last+=delta;return current;
}
}
