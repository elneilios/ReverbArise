#include "UI/SongBarRuler.h"
#include <cmath>
#include <limits>

int main()
{
    // FL Studio example: bar 37 is PPQ 144 at 82 BPM, with sample-position rounding.
    const auto samplesPerBar = 48000.0 * 60.0 / 82.0 * 4.0;
    const auto hostSample = static_cast<int64_t>(std::llround(36 * samplesPerBar));
    const auto ticks = makeSongBarRuler(0, 40 * samplesPerBar, 804, 48000, 82, 4, 4, hostSample, 144);
    if (ticks.empty() || ticks.front().bar != 1 || std::abs(ticks.front().samplePosition) > 1) return 1;
    bool found37 = false;
    for (const auto& tick : ticks)
        if (tick.bar == 37)
            found37 = std::abs(tick.samplePosition - hostSample) < 1;
    if (! found37 || ticks.size() > 14) return 2;
    // Seeked view: host sample/PPQ origin offset must be respected, including 6/8 bars.
    const auto shifted = makeSongBarRuler(1000, 289000, 800, 48000, 120, 6, 8, 145000, 6);
    if (shifted.size() != 5 || shifted.front().bar != 1 || shifted.front().samplePosition != 1000
        || shifted[2].bar != 3 || shifted[2].samplePosition != 145000) return 3;
    const auto leadIn = makeSongBarRuler(-96000, 96000, 800, 48000, 120, 4, 4, 0, 0);
    if (leadIn.size() != 3 || leadIn.front().bar != 0 || leadIn[1].bar != 1) return 4;
    if (! makeSongBarRuler(0, 100, 800, 48000, 0, 4, 4, 0, 0).empty()
        || ! makeSongBarRuler(0, 100, 800, 48000, 120, 0, 4, 0, 0).empty()
        || ! makeSongBarRuler(0, 100, 800, 48000, 120, 4, 4, 0,
                             std::numeric_limits<double>::quiet_NaN()).empty()) return 5;
    return 0;
}
