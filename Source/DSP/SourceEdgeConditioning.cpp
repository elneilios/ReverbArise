#include "SourceEdgeConditioning.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace m3
{
bool conditionWetSourceEdges(std::vector<float>& stereo, double rate, juce::String& error,
                             const RenderCancellation& cancelled)
{
    if(stereo.empty() || stereo.size()%2 || !std::isfinite(rate) || rate<=0)
    {error="Invalid wet source for edge conditioning";return false;}
    if(cancelled && cancelled()){error="Cancelled";return false;}
    const auto frames=stereo.size()/2;
    const auto ramp=static_cast<size_t>(std::floor(std::min(rate*0.001+0.5,static_cast<double>(frames/2))));
    // A one-sample ramp would erase tiny selections without a transition.
    if(ramp<2)return true;
    for(size_t i=0;i<ramp;++i)
    {
        if(i%1024==0 && cancelled && cancelled()){error="Cancelled";return false;}
        const auto gain=static_cast<float>(0.5-0.5*std::cos(std::numbers::pi*static_cast<double>(i)/static_cast<double>(ramp-1)));
        const auto head=i*2,tail=(frames-1-i)*2;
        stereo[head]*=gain;stereo[head+1]*=gain;
        stereo[tail]*=gain;stereo[tail+1]*=gain;
    }
    return true;
}
}
