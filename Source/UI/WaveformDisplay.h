#pragma once
#include <Capture/CaptureSession.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace m3::ui
{
struct WaveformColumn
{
    float minimum=0,maximum=0;
    bool present=false;
};

// GUI-only display reduction. Ordered, nonoverlapping peak intervals retain
// their extrema in every intersecting pixel; absent columns remain absent.
// Translate in integer space when representable (MSVC long double is double).
inline double displayPosition(int64_t sample,int64_t placement)
{
    if((placement>0 && sample>INT64_MAX-placement) || (placement<0 && sample<INT64_MIN-placement))
        return static_cast<double>(sample)+static_cast<double>(placement);
    return static_cast<double>(sample+placement);
}
template<class Extrema>
std::vector<WaveformColumn> waveformColumns(const std::vector<CapturePeak>& peaks,
    double first,double last,int width,int64_t placement,Extrema extrema,uint64_t& visited,bool detailStrokes=false)
{
    if(width<=0 || !std::isfinite(first) || !std::isfinite(last) || last<=first)return {};
    std::vector<WaveformColumn> columns(static_cast<size_t>(width));
    const auto begin=static_cast<long double>(first),end=static_cast<long double>(last);
    const auto position=[&](const auto& p){return displayPosition(p.startSample,placement);};
    auto peak=std::lower_bound(peaks.begin(),peaks.end(),begin,[&](const auto& p,long double at)
        {return position(p)+p.frameCount<=at;});
    for(;peak!=peaks.end() && position(*peak)<end;++peak)
    {
        ++visited;
        const auto a=std::max(begin,static_cast<long double>(position(*peak)));
        const auto b=std::min(end,static_cast<long double>(position(*peak))+peak->frameCount);
        if(b<=a)continue;
        const auto values=extrema(*peak);
        if(!std::isfinite(values.first) || !std::isfinite(values.second))continue;
        // Deep zoom must not stretch a coarse summary into a solid rectangle.
        // Retain a narrow summary stroke when a bucket spans multiple pixels.
        // A bucket clipped by the viewport's left edge is represented there,
        // rather than disappearing until the next bucket starts. This denotes
        // the intersecting bucket's extrema, not a sample at the clipped edge.
        if(detailStrokes && peak->frameCount/(end-begin)*width>1)
        {
            const auto x=std::clamp(static_cast<int>((a-begin)/(end-begin)*width),0,width-1);
            auto& c=columns[static_cast<size_t>(x)];
            if(!c.present)c={values.first,values.second,true};
            else {c.minimum=std::min(c.minimum,values.first);c.maximum=std::max(c.maximum,values.second);}
            continue;
        }
        // Widen by one candidate column, then use interval intersections. This
        // avoids ceil/floor roundoff leaking a peak across an exact boundary.
        const auto left=std::clamp(static_cast<int>(std::floor((a-begin)/(end-begin)*width))-1,0,width-1);
        const auto right=std::clamp(static_cast<int>(std::ceil((b-begin)/(end-begin)*width)),0,width-1);
        for(int x=left;x<=right;++x)
        {
            const auto columnFirst=begin+(end-begin)*x/width,columnLast=begin+(end-begin)*(x+1)/width;
            if(a>=columnLast || b<=columnFirst)continue;
            auto& c=columns[static_cast<size_t>(x)];
            if(!c.present){c.minimum=values.first;c.maximum=values.second;c.present=true;}
            else {c.minimum=std::min(c.minimum,values.first);c.maximum=std::max(c.maximum,values.second);}
        }
    }
    return columns;
}

struct WaveformPaintMetrics
{
    uint64_t builds=0,hits=0,peaksVisited=0,columnsPainted=0,bytes=0;
};

// A single replaceable layer per consumer, owned and retired on the message
// thread. Neither keys nor rasterization involve audio publication locks.
class WaveformLayer
{
public:
    void invalidate(){dirty=true;}
    const WaveformPaintMetrics& metrics()const noexcept{return counters;}
    template<class Painter>
    void paint(juce::Graphics& destination,int width,int height,double first,double last,Painter painter)
    {
        if(width<=0 || height<=0)return;
        auto scale=std::max(1.0f,destination.getInternalContext().getPhysicalPixelScaleFactor());
        // Bound a layer to 16 million pixels, including unusual unsupported
        // sizes/transforms. Supported 200% editor rasters remain full resolution.
        // Leave room for ceil-to-pixel rounding and bound skinny dimensions too.
        scale=std::min({scale,16384.0f/std::max(width,height),
            static_cast<float>(std::sqrt(15900000.0/(static_cast<double>(width)*height)))});
        const auto pixelWidth=std::max(1,static_cast<int>(std::ceil(width*scale)));
        const auto pixelHeight=std::max(1,static_cast<int>(std::ceil(height*scale)));
        if(dirty || image.getWidth()!=pixelWidth || image.getHeight()!=pixelHeight || scale!=imageScale || first!=viewFirst || last!=viewLast)
        {
            image=juce::Image(juce::Image::ARGB,pixelWidth,pixelHeight,true,juce::SoftwareImageType{});
            imageScale=scale;viewFirst=first;viewLast=last;
            juce::Graphics drawing(image);drawing.addTransform(juce::AffineTransform::scale(scale));
            counters.peaksVisited=0;counters.columnsPainted=0;
            painter(drawing,scale,counters);++counters.builds;
            counters.bytes=static_cast<uint64_t>(pixelWidth)*pixelHeight*4;dirty=false;
        }
        else ++counters.hits;
        destination.drawImageTransformed(image,juce::AffineTransform::scale(1.0f/imageScale));
    }
private:
    juce::Image image;
    WaveformPaintMetrics counters;
    double viewFirst=0,viewLast=0;
    float imageScale=0;
    bool dirty=true;
};
}
