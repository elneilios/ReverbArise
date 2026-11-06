#pragma once
#include <Core/SongTime.h>
#include <Core/HostGrid.h>
#include <algorithm>
#include <cmath>

namespace m3
{
struct SwellViewport
{
    double first=-24000,last=384000;
    double span() const noexcept {return last-first;}
    double pixel(double sample,double x,double width) const noexcept {return x+(sample-first)*width/span();}
    int64_t sample(double pixel,double x,double width) const noexcept
    {
        if(width<=0)return 0;
        const auto value=std::round(static_cast<long double>(first)+(pixel-x)/width*span());
        if(value<=-9223372036854775808.0L)return INT64_MIN;
        if(value>=9223372036854775808.0L)return INT64_MAX;
        return static_cast<int64_t>(value);
    }
    void fit(double a,double b) noexcept
    {
        if(!std::isfinite(a) || !std::isfinite(b) || b<=a)return;
        const auto pad=std::max(1.0,(b-a)*0.04);first=a-pad;last=b+pad;
    }
    void shift(double fraction) noexcept {const auto delta=span()*fraction;first+=delta;last+=delta;}
    void zoom(double factor,double anchor=0.5) noexcept
    {
        if(!std::isfinite(factor) || factor<=0)return;
        const auto size=std::clamp(span()*factor,1.0,1.0e12),center=first+span()*anchor;
        first=center-size*anchor;last=first+size;
    }
    void page(int bars,const SongTime& time) noexcept
    {
        if(!time.valid || time.rate<=0 || time.bpm<=0 || time.numerator<=0 || time.denominator<=0)return;
        const auto size=time.rate*60/time.bpm*time.numerator*4/time.denominator*bars;
        if(std::isfinite(size) && size>0)last=first+size;
    }
    // Opt-in bounded navigation; existing unbounded arithmetic remains available
    // to workspace adapters with their own accepted limits (including Export).
    bool constrain(const SwellViewport& limits,double minimum)noexcept
    {
        if(!std::isfinite(limits.first) || !std::isfinite(limits.last) || !std::isfinite(limits.span())
            || limits.span()<=0 || !std::isfinite(minimum) || minimum<=0)return false;
        const auto size=std::isfinite(span()) && span()>0?std::clamp(span(),std::min(minimum,limits.span()),limits.span()):limits.span();
        if(size>=limits.span()){first=limits.first;last=limits.last;return true;}
        first=std::isfinite(first)?std::clamp(first,limits.first,std::max(limits.first,limits.last-size)):limits.first;
        last=std::min(limits.last,first+size);return last>first;
    }
    bool zoomBounded(double factor,double anchor,const SwellViewport& limits,double minimum)noexcept
    {
        if(!std::isfinite(factor) || factor<=0 || !std::isfinite(anchor) || !constrain(limits,minimum))return false;
        anchor=std::clamp(anchor,0.,1.);const auto at=first+span()*anchor;
        const auto size=std::clamp(span()*factor,std::min(minimum,limits.span()),limits.span());
        first=at-size*anchor;last=first+size;return constrain(limits,minimum);
    }
    std::optional<int64_t> snapToGrid(int64_t position,const SongTime& accepted,const SongTime& now,
                                    double captureRate,GridDivision division,const GridClockContext* clock=nullptr)const noexcept
    {
        const auto grid=HostGrid::make(accepted,now,captureRate,division,nullptr,true,clock);
        return grid?grid->snap(position):std::optional<int64_t>{};
    }
    std::optional<int64_t> snapToGridWithin(int64_t position,int64_t lower,int64_t upper,const SongTime& accepted,
                                          const SongTime& now,double captureRate,GridDivision division,const GridClockContext* clock=nullptr)const noexcept
    {
        const auto grid=HostGrid::make(accepted,now,captureRate,division,nullptr,true,clock);
        return grid?grid->snapWithin(position,lower,upper):std::optional<int64_t>{};
    }
    int64_t snap(int64_t position,const SongTime& time,bool enabled) const noexcept
    {
        if(!enabled || !time.valid || time.rate<=0 || time.bpm<=0)return position;
        const auto grid=time.rate*60/time.bpm/4,origin=time.sample-time.ppq*time.rate*60/time.bpm;
        const auto result=std::round((position-origin)/grid)*grid+origin;
        if(result<=-9223372036854775808.0)return INT64_MIN;if(result>=9223372036854775808.0)return INT64_MAX;
        return static_cast<int64_t>(std::llround(result));
    }
};
}
