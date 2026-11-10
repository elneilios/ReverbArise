#pragma once

#include <Capture/CaptureManifest.h>
#include <Capture/LoopPhraseClock.h>
#include <algorithm>
#include <optional>

namespace m3
{
// This describes a live reference route, independent of the saved capture.
struct LoopReferenceSetup
{
    loop_capture::ClockProfile clock;
    int beats = 0;
    int numerator = 0;
    int denominator = 0;
    bool phraseInput = false;
    bool observeCadence = false;

    bool operator==(const LoopReferenceSetup&) const = default;
};

class LoopLiveReference
{
public:
    enum class State { unconfigured, mismatch, waiting, ready, lost, unsupported };
    struct Window { int64_t earliest, latest; };
    struct Observation { bool boundary = false, lost = false; };

    bool configure(LoopReferenceSetup next, double sourceRate,
                   std::optional<LoopPhraseTiming> source) noexcept
    {
        if (next == setup && sourceRate == acceptedRate && source == accepted) return false;
        setup = next;
        accepted = source;
        acceptedRate = sourceRate;
        origin = last = lastPhrase = -1;
        cycles = 0;
        cadenceMismatch = false;
        if (!source || !next.phraseInput || next.beats <= 0)
            unavailable = State::unconfigured;
        else if (!next.clock.valid())
            unavailable = State::unsupported;
        else if (next.clock.rate != sourceRate || next.beats != source->beats
                 || next.numerator != source->meterNumerator || next.denominator != source->meterDenominator
                 || next.numerator != 4 || next.denominator != 4
                 || !loop_capture::ClockProfile{sourceRate,source->tempoBpm}.valid()
                 || !roundedLength(source->endSample, period())
                 || !roundedLength(source->endSample, phrasePeriod(sourceRate, source->tempoBpm, next.beats)))
            unavailable = State::mismatch;
        else
            unavailable = State::waiting;
        return true;
    }

    State state() const noexcept
    {
        if (cycles >= 2) return State::ready;
        return cadenceMismatch ? State::mismatch : unavailable;
    }

    Observation observe(int64_t sample, uint16_t roles) noexcept
    {
        if (unavailable == State::unconfigured || unavailable == State::mismatch || unavailable == State::unsupported) return {};
        if ((roles & loop_capture::reset) || sample < 0)
        {
            revoke();
            return {false, true};
        }
        if (setup.observeCadence && (roles & loop_capture::phrase))
        {
            if (lastPhrase >= 0)
                cadenceMismatch = !roundedLength(sample - lastPhrase,period());
            lastPhrase = sample;
        }
        if (advance(sample)) return {false,true};
        if (!(roles & loop_capture::phrase)) return {};
        if (origin < 0) { origin = last = sample; updateDeadline(); return {}; }
        const auto next = cycles + 1;
        const auto boundary = epochWindow(next);
        if (!boundary || sample < boundary->earliest || sample > boundary->latest
            || !roundedLength(sample - last, period()))
        {
            revokeEpoch();
            return {false,true};
        }
        last = sample;
        cycles = next;
        updateDeadline();
        return {cycles >= 2, false};
    }

    bool advance(int64_t sample) noexcept
    {
        if (origin < 0) return false;
        if (sample >= last && sample <= deadline) return false;
        revokeEpoch();
        return true;
    }

    void revoke() noexcept
    {
        revokeEpoch();
        lastPhrase = -1;
        cadenceMismatch = false;
    }
    int64_t ordinal() const noexcept { return cycles; }
    long double period() const noexcept { return phrasePeriod(setup.clock.rate, setup.clock.tempo, setup.beats); }

    std::optional<Window> window(int64_t cycle) const noexcept
    {
        if (state() != State::ready || cycle < 0) return {};
        if (cycle == 0) return Window{origin,origin};
        return epochWindow(cycle);
    }

    std::optional<Window> cueWindow(int64_t cycle) const noexcept
    {
        auto forecast = window(cycle);
        if (!forecast || cycle <= cycles) return {};
        const auto remaining = cycle - cycles;
        const auto minimum = static_cast<int64_t>(std::floor(period()));
        const auto maximum = static_cast<int64_t>(std::ceil(period()));
        const auto headroom = INT64_MAX - last;
        if (remaining > headroom / minimum) return {};
        forecast->earliest = std::max(forecast->earliest,last + remaining * minimum);
        if (remaining <= headroom / maximum)
            forecast->latest = std::min(forecast->latest,last + remaining * maximum);
        return forecast->earliest <= forecast->latest ? forecast : std::nullopt;
    }

private:
    void revokeEpoch() noexcept
    {
        if (cycles >= 2) unavailable = State::lost;
        origin = last = -1;
        cycles = 0;
    }

    void updateDeadline() noexcept
    {
        const auto boundary = epochWindow(cycles + 1);
        const auto interval = static_cast<int64_t>(std::ceil(period()));
        deadline = boundary ? std::min(boundary->latest,last + std::min(interval,INT64_MAX-last)) : last;
    }

    std::optional<Window> epochWindow(int64_t cycle) const noexcept
    {
        const auto relative = static_cast<long double>(setup.clock.rate) * 60 * setup.beats * cycle / setup.clock.tempo;
        const auto nominal = period();
        const auto allowance = std::floor(nominal) == std::ceil(nominal) ? 0 : 1;
        const auto first = std::ceil(relative - allowance);
        const auto final = std::floor(relative + allowance);
        if (!(final < std::ldexp(1.0L, 63))) return {};
        const auto upper = static_cast<int64_t>(final);
        if (upper > INT64_MAX - origin) return {};
        return Window{origin + static_cast<int64_t>(first), origin + upper};
    }

    static long double phrasePeriod(double rate, double bpm, int beats) noexcept
    { return static_cast<long double>(rate) * 60 * beats / bpm; }

    static bool roundedLength(int64_t samples, long double nominal) noexcept
    { return samples > 0 && (samples == std::floor(nominal) || samples == std::ceil(nominal)); }

    LoopReferenceSetup setup;
    std::optional<LoopPhraseTiming> accepted;
    double acceptedRate = 0;
    int64_t origin = -1, last = -1, cycles = 0, deadline = 0;
    int64_t lastPhrase = -1;
    bool cadenceMismatch = false;
    State unavailable = State::unconfigured;
};
}
