#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace m3
{
inline float monitorEdgeGain(size_t at,size_t frames,double rate,double seconds) noexcept
{
    if(frames<3)return 0;
    const auto ramp=std::min((frames-1)/2,static_cast<size_t>(std::max(1.0,std::round(rate*seconds))));
    return static_cast<float>(std::min({1.0,static_cast<double>(at)/ramp,static_cast<double>(frames-1-at)/ramp}));
}
}
