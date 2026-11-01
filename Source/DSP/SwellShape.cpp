#include "SwellShape.h"
#include <algorithm>
#include <cmath>

namespace m3
{
double fadeFactor(double u,Curve curve,double tension) noexcept
{
    u=std::clamp(u,0.0,1.0);
    if(std::abs(tension)>1e-8)u=std::expm1(6*tension*u)/std::expm1(6*tension);
    if(curve==Curve::smooth) return u*u*(3-2*u);
    if(curve==Curve::exponential) return std::expm1(4*u)/std::expm1(4.0);
    return u;
}
double envelope(const Shape& shape,int64_t position,int64_t start,int64_t end) noexcept
{
    if(position<start || position>=end) return 0;
    const auto fadeIn=shape.fadeIn ? fadeFactor(static_cast<double>(position-start)/shape.fadeIn,shape.inCurve,shape.inTension) : 1.0;
    const auto fadeOut=shape.fadeOut ? fadeFactor(static_cast<double>(end-1-position)/shape.fadeOut,shape.outCurve,shape.outTension) : 1.0;
    return std::pow(10.0,shape.gainDb/20.0)*fadeIn*fadeOut;
}
bool prepareShape(const Swell& swell,const WetRender& raw,uint64_t available,WetRender& output,
                  juce::String& error,const RenderCancellation& cancelled)
{
    int64_t rawEnd,a,b;
    if(raw.frames()>INT64_MAX || !checkedAdd(raw.start,static_cast<int64_t>(raw.frames()),rawEnd)
        || !shapeBounds(swell,raw.start,rawEnd,a,b,error)) return false;
    const auto frames=static_cast<uint64_t>(b-a);
    if(frames>std::min(available,previewMemoryBudget)/(2*sizeof(float)))
    { error="Edited preview exceeds 256 MiB working budget"; return false; }
    WetRender shaped; shaped.start=a; shaped.sourceStart=raw.sourceStart; shaped.sourceEnd=raw.sourceEnd; shaped.rate=raw.rate;
    shaped.samples.resize(static_cast<size_t>(frames)*2);
    for(uint64_t i=0;i<frames;++i)
    {
        if(i%4096==0 && cancelled && cancelled()) { error="Shape preparation cancelled"; return false; }
        const auto gain=envelope(swell.shape,a+static_cast<int64_t>(i),a,b);
        for(size_t channel=0;channel<2;++channel)
        {
            const auto value=static_cast<float>(raw.samples[(static_cast<size_t>(a-swell.placement-raw.start)+i)*2+channel]*gain);
            if(!std::isfinite(value)) { error="Edited preview contains non-finite output"; return false; }
            shaped.samples[static_cast<size_t>(i)*2+channel]=value; shaped.peak=std::max(shaped.peak,std::abs(value));
        }
    }
    output=std::move(shaped); error.clear(); return true;
}
}
