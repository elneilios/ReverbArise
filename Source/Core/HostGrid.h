#pragma once
#include "TransportTime.h"
#include <array>
#include <optional>
#include <string_view>

namespace m3
{
enum class GridDivision {sixteenthBeat,eighthBeat,quarterBeat,beat,bar,twoBars,fourBars};
struct GridChoice {GridDivision division;std::string_view key,label;};
inline constexpr std::array<GridChoice,7> gridChoices{{
    {GridDivision::sixteenthBeat,"sixteenth-beat","1/16 beat"},
    {GridDivision::eighthBeat,"eighth-beat","1/8 beat"},
    {GridDivision::quarterBeat,"quarter-beat","1/4 beat"},
    {GridDivision::beat,"beat","1 beat"},{GridDivision::bar,"bar","1 bar"},
    {GridDivision::twoBars,"two-bars","2 bars"},{GridDivision::fourBars,"four-bars","4 bars"}}};

// Fixed accepted mapping only. The existing transport authority decides whether
// a new host observation still describes the captured material's Song mapping.
class HostGrid
{
public:
    static std::optional<HostGrid> make(const SongTime& accepted,const SongTime& now,
                                      double captureRate,GridDivision division)noexcept
    {
        if(!std::isfinite(captureRate) || captureRate<=0 || std::abs(captureRate-accepted.rate)>.001
            || !transportMappingMatches(accepted,now))return {};
        long double beats;
        switch(division)
        {
            case GridDivision::sixteenthBeat:beats=1.L/16;break;
            case GridDivision::eighthBeat:beats=1.L/8;break;
            case GridDivision::quarterBeat:beats=1.L/4;break;
            case GridDivision::beat:beats=1;break;
            case GridDivision::bar:beats=accepted.numerator;break;
            case GridDivision::twoBars:beats=2.L*accepted.numerator;break;
            case GridDivision::fourBars:beats=4.L*accepted.numerator;break;
            default:return {};
        }
        const auto step=beats*4/accepted.denominator;
        const auto samples=step*accepted.rate*60/accepted.bpm;
        long double origin;
        if(!std::isfinite(step) || step<=0 || !std::isfinite(samples) || samples<=0
            || !songOrigin(accepted,origin))return {};
        return HostGrid(accepted,step,samples,origin);
    }
    std::optional<int64_t> snap(int64_t candidate)const noexcept
    {
        const auto index=std::round((static_cast<long double>(candidate)-origin)/sampleStep);
        // Windows long double is double. Above this limit integer grid indices
        // cannot retain unit precision; reject rather than claim exact snapping.
        if(!std::isfinite(index) || std::abs(index)>4503599627370495.L)return {};
        const auto ppq=index*quarterStep;
        if(!std::isfinite(ppq))return {};
        int64_t result;
        if(!resolvePpqSample(mapping,static_cast<double>(ppq),result))return {};
        return result;
    }
private:
    HostGrid(SongTime time,long double quarters,long double samples,long double zero)
        :mapping(time),quarterStep(quarters),sampleStep(samples),origin(zero){}
    SongTime mapping;
    long double quarterStep,sampleStep,origin;
};
}
