#pragma once
#include <DSP/SwellShape.h>
#include "SwellViewport.h"
#include <array>
namespace m3
{
enum class EnvelopePoint {start,end,fadeIn,fadeOut,inTension,outTension};
struct EnvelopeGrab {EnvelopePoint kind;juce::Point<float> point,anchor;int64_t sample=0;};
inline std::vector<EnvelopeGrab> envelopeHandles(const Swell& w,int64_t rawStart,const SwellViewport& view,juce::Rectangle<float> lane)
{
    int64_t a,b,length;if(!requestedShapeBounds(w,rawStart,a,b) || !checkedSubtract(b,a,length) || length<=0 || lane.getWidth()<24 || lane.getHeight()<40)return {};
    const auto at=[&](int64_t sample){auto shape=w.shape;shape.gainDb=0;
        return juce::Point<float>{static_cast<float>(view.pixel(static_cast<double>(sample),lane.getX(),lane.getWidth())),
            lane.getCentreY()-static_cast<float>(envelope(shape,std::min(sample,b-1),a,b))*lane.getHeight()*.38f};};
    // Sample identity is retained before conversion to pixels: visual overlap
    // at a distant zoom is not an intrinsic coincidence.
    const auto grab=[&](EnvelopePoint kind,int64_t sample){auto point=at(sample);
        if(kind==EnvelopePoint::start || kind==EnvelopePoint::end)point.y=lane.getCentreY();
        return EnvelopeGrab{kind,point,point,sample};};
    std::vector<EnvelopeGrab> points{grab(EnvelopePoint::end,b),grab(EnvelopePoint::start,a)};
    points.push_back(grab(EnvelopePoint::fadeIn,a+std::min(w.shape.fadeIn,b-a-1)));
    points.push_back(grab(EnvelopePoint::fadeOut,w.shape.fadeOut==0?b:b-1-std::min(w.shape.fadeOut,b-a-1)));
    if(w.shape.fadeIn>0)points.push_back(grab(EnvelopePoint::inTension,a+std::min(w.shape.fadeIn,b-a-1)/2));
    if(w.shape.fadeOut>0)points.push_back(grab(EnvelopePoint::outTension,b-1-std::min(w.shape.fadeOut,b-a-1)/2));
    for(size_t i=0;i<points.size();++i){auto& p=points[i];
        if(p.anchor.x<lane.getX() || p.anchor.x>lane.getRight())continue;
        const auto coincides=std::any_of(points.begin(),points.begin()+i,[&](const auto& other){return other.sample==p.sample && other.anchor==p.anchor;});
        if(!coincides)continue;
        // Offset only identical time and amplitude anchors, never nearby glyphs.
        const std::array<juce::Point<float>,8> offsets{{{0,24},{0,-24},{0,48},{0,-48},{24,0},{-24,0},{24,24},{-24,-24}}};
        bool placed=false;
        for(const auto offset:offsets){const auto candidate=p.anchor+offset;if(!lane.contains(candidate))continue;
            bool clear=true;for(size_t j=0;j<i;++j)if(points[j].sample==p.sample && points[j].anchor==p.anchor && candidate.getDistanceFrom(points[j].point)<23)clear=false;
            if(clear){p.point=candidate;placed=true;break;}}
        if(!placed){p.point=p.anchor+juce::Point<float>{0,12};p.point.y=std::clamp(p.point.y,lane.getY()+1,lane.getBottom()-1);}
    }
    return points;
}
inline std::optional<EnvelopePoint> hitEnvelope(const std::vector<EnvelopeGrab>& points,juce::Point<float> cursor)
{
    // Last painted is topmost, including hit regions that overlap at low zoom.
    for(auto p=points.rbegin();p!=points.rend();++p)if(p->point.getDistanceFrom(cursor)<11)return p->kind;
    return std::nullopt;
}
}
