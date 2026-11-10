#pragma once

#include "LoopCapturePreference.h"
#include "LoopPhraseClock.h"
#include "CaptureManifest.h"

namespace loop_capture
{
class Take
{
public:
    static constexpr int maximumFrames = 48000 * 120;
    enum class Edge { none, start, end, invalid };

    void arm(LoopCapturePreference settings) noexcept
    {
        *this = {};
        preference = settings;
        armed = true;
    }

    Edge observe(int64_t sample, uint16_t roles, PhraseClock::Boundary boundary, bool qualifiedEnd = false) noexcept
    {
        if (!armed) return Edge::none;
        if (boundary.revoked) return finish(Edge::invalid);
        if (origin < 0)
        {
            if (roles & gateLow) sawLow = true;
            const bool rise = (roles & gateRise) && sawLow;
            if (roles & gateRise) sawLow = false;
            const bool starts = preference.method == LoopCapturePreference::Method::duration
                ? boundary.verifiedPhrase : rise && boundary.verifiedBeat;
            if (starts) { origin = sample; return Edge::start; }
            return Edge::none;
        }
        if (sample - origin > maximumFrames) return finish(Edge::invalid);
        if (boundary.verifiedBeat) ++beats;
        if (preference.method == LoopCapturePreference::Method::duration)
        {
            const auto target = preference.count * (preference.unit == LoopCapturePreference::Unit::bars ? PhraseClock::beatsPerBar : 1);
            if (beats == target && (preference.unit == LoopCapturePreference::Unit::beats || boundary.verifiedBar))
                return finish(Edge::end);
        }
        else
        {
            pendingEnd |= qualifiedEnd;
            if (pendingEnd && boundary.verifiedBar) return finish(beats > 0 ? Edge::end : Edge::invalid);
        }
        return Edge::none;
    }

    int64_t start() const noexcept { return origin; }
    unsigned beatCount() const noexcept { return beats; }
    bool stopPending() const noexcept { return stopSample >= 0; }
    bool duration() const noexcept { return preference.method == LoopCapturePreference::Method::duration; }
    bool endOnRepeat() const noexcept { return preference.method == LoopCapturePreference::Method::endOnRepeat; }
    void requestStop(int64_t sample) noexcept
    {
        if (!stopPending())
            stopSample = sample;
    }
    Edge observeClock(int64_t sample, bool repeatStart = false) noexcept
    {
        if (!armed) return Edge::none;
        if (origin < 0)
        {
            if (endOnRepeat() && !repeatStart) return Edge::none;
            origin = sample;
            return Edge::start;
        }
        ++beats;
        if (endOnRepeat())
            return repeatStart ? finish(beats % PhraseClock::beatsPerBar == 0 ? Edge::end : Edge::invalid) : Edge::none;
        const auto target = preference.count
            * (preference.unit == LoopCapturePreference::Unit::bars ? PhraseClock::beatsPerBar : 1);
        const auto complete = duration() ? beats == target
            : stopPending() && sample > stopSample && beats % PhraseClock::beatsPerBar == 0;
        return complete ? finish(Edge::end) : Edge::none;
    }
    LoopPhraseTiming timing(double tempo, int64_t end) const noexcept
    { return {tempo, PhraseClock::beatsPerBar, 4, static_cast<int>(beats), origin, end - origin}; }

private:
    Edge finish(Edge edge) noexcept { armed = false; return edge; }
    LoopCapturePreference preference;
    int64_t origin = -1, stopSample = -1;
    unsigned beats = 0;
    bool armed = false, sawLow = false, pendingEnd = false;
};
}
