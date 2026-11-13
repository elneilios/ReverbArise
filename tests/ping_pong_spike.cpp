#include "AudioTestFiles.h"
#include <DSP/SwellShape.h>
#include <UI/SongBarRuler.h>
#include <UI/TimingEdit.h>
#include <numbers>
#include <optional>

namespace
{
constexpr int64_t rate = 48000;
constexpr int64_t frameCount = 120000;
constexpr int64_t beatOne = 4800;
constexpr int64_t arrival = beatOne + 12 * 19200;
constexpr int64_t preRoll = frameCount - 1;

struct Burst
{
    int64_t first, last, peak;
    int channel;
    float amplitude;
};

struct PositionedBurst
{
    Burst burst;
    int64_t peak;
};

struct Response
{
    ImpulseResponse ir;
    std::vector<PositionedBurst> hits;
};

float audibleSample(const ImpulseResponse& ir, int64_t frame, int channel)
{
    return ir.samples[static_cast<size_t>(preRoll - frame) * 2 + channel];
}

std::vector<Burst> findBursts(const ImpulseResponse& reference)
{
    std::vector<Burst> bursts;
    int64_t lastActive = -frameCount;
    for (int64_t frame = 0; frame < frameCount; ++frame)
    {
        const auto left = std::abs(audibleSample(reference, frame, 0));
        const auto right = std::abs(audibleSample(reference, frame, 1));
        if (left == 0 && right == 0) continue;
        REQUIRE(left == 0 || right == 0);
        const auto channel = left > right ? 0 : 1;
        if (frame - lastActive > rate / 20)
            bursts.push_back({frame, frame, frame, channel, 0});
        auto& burst = bursts.back();
        REQUIRE(burst.channel == channel);
        burst.last = frame;
        const auto amplitude = std::max(left, right);
        if (amplitude > burst.amplitude)
        {
            burst.peak = frame;
            burst.amplitude = amplitude;
        }
        lastActive = frame;
    }
    REQUIRE(bursts.size() == 11);
    for (size_t i = 1; i < bursts.size(); ++i)
    {
        REQUIRE(bursts[i].channel != bursts[i - 1].channel);
        REQUIRE(bursts[i].amplitude > bursts[i - 1].amplitude);
        REQUIRE(std::abs(bursts[i].peak - bursts[i - 1].peak - 10560) <= 2);
    }
    return bursts;
}

SongTime songClock(double bpm = 150)
{
    return {true, beatOne, 0.0, static_cast<double>(rate), bpm, 4, 4};
}

Response makeResponse(const ImpulseResponse& reference, const std::vector<Burst>& bursts,
                      const m3::Swell& swell, double division, const SongTime& song,
                      bool rulerAligned)
{
    Response response{reference, {}};
    std::fill(response.ir.samples.begin(), response.ir.samples.end(), 0.0f);
    // The impulse is at the arrival in the source. Move Wet translates that render.
    const auto irEnd = swell.arrival + swell.placement;
    const auto irStart = irEnd - preRoll;
    const auto last = bursts.back();
    const auto availableEnd = std::min(swell.arrival, irEnd);
    const auto maxPeak = availableEnd - (last.last - last.peak);
    const auto step = static_cast<long double>(rate) * 60 * division / song.bpm;
    long double origin = 0;
    REQUIRE(songOrigin(song, origin));
    auto tick = static_cast<int64_t>(std::floor((maxPeak + 0.5L - origin) / step));
    int64_t gridSample = 0;
    REQUIRE(resolvePpqSample(song, tick * division, gridSample));
    if (gridSample > maxPeak) --tick;
    for (size_t offset = 0; offset < bursts.size(); ++offset)
    {
        const auto& burst = bursts[bursts.size() - 1 - offset];
        int64_t peak = irStart + last.peak - std::llround(step * offset);
        if (rulerAligned)
            REQUIRE(resolvePpqSample(song, (tick - static_cast<int64_t>(offset)) * division, peak));
        const auto first = peak - (burst.peak - burst.first);
        const auto end = peak + (burst.last - burst.peak);
        if (first < irStart) break;
        REQUIRE(end <= irEnd);
        if (rulerAligned) REQUIRE(end <= swell.arrival);
        for (int64_t frame = burst.first; frame <= burst.last; ++frame)
        {
            const auto stored = irEnd - (peak + frame - burst.peak);
            response.ir.samples[static_cast<size_t>(stored) * 2 + burst.channel] +=
                audibleSample(reference, frame, burst.channel);
        }
        response.hits.push_back({burst, peak});
    }
    REQUIRE(!response.hits.empty());
    std::reverse(response.hits.begin(), response.hits.end());
    return response;
}

m3::Swell makeSwell(int64_t at)
{
    m3::Swell swell;
    swell.arrival = swell.sourceStart = at;
    swell.sourceEnd = at + rate / 4;
    swell.shape.endOffset = rate / 5;
    return swell;
}

std::vector<float> impulseAtArrival(const m3::Swell& swell)
{
    const auto index = static_cast<size_t>(swell.arrival - swell.sourceStart) * 2;
    std::vector<float> input(index + 2, 0.0f);
    input[index] = input[index + 1] = 1.0f;
    return input;
}

WetRender render(m3::Swell swell, const ImpulseResponse& ir, const std::vector<float>& dry)
{
    WetRender raw, shaped;
    juce::String error;
    REQUIRE(renderReverseConvolution(dry, swell.sourceStart, rate, ir, 0, raw, error));
    const auto end = raw.start + static_cast<int64_t>(raw.frames());
    REQUIRE(m3::fitWetShapeToResponse(swell, raw.start, end, error));
    REQUIRE(m3::prepareShape(swell, raw, previewMemoryBudget, shaped, error));
    return shaped;
}

float sampleAt(const WetRender& wet, int64_t at, int channel)
{
    const auto frame = at - wet.start;
    return frame >= 0 && frame < static_cast<int64_t>(wet.frames())
        ? wet.samples[static_cast<size_t>(frame) * 2 + channel] : 0.0f;
}

void verify(const char* label, const m3::Swell& swell, const Response& response,
            std::optional<double> division = {}, double bpm = 150)
{
    const auto wet = render(swell, response.ir, impulseAtArrival(swell));
    int64_t worst = 0;
    float previous = 0;
    REQUIRE(wet.peak < 1.0f);
    for (const auto& hit : response.hits)
    {
        const auto& burst = hit.burst;
        auto peak = hit.peak - 32;
        for (auto at = peak + 1; at <= hit.peak + 32; ++at)
            if (std::abs(sampleAt(wet, at, burst.channel)) > std::abs(sampleAt(wet, peak, burst.channel)))
                peak = at;
        const auto value = std::abs(sampleAt(wet, peak, burst.channel));
        REQUIRE(std::abs(value - burst.amplitude) < 0.00002f && value > previous);
        REQUIRE(value > 2 * std::abs(sampleAt(wet, peak, 1 - burst.channel)));
        REQUIRE(std::abs(peak - hit.peak) <= 1);
        if (division)
        {
            const auto step = static_cast<long double>(rate) * 60 * *division / bpm;
            const auto grid = beatOne + std::llround(std::round((peak - beatOne) / step) * step);
            worst = std::max(worst, static_cast<int64_t>(std::abs(peak - grid)));
            REQUIRE(worst <= 1);
        }
        previous = value;
    }
    std::cout << label << ": " << response.hits.size() << " hits, last gap "
              << 1000.0 * (swell.arrival - response.hits.back().peak) / rate
              << " ms, peak " << wet.peak;
    if (division) std::cout << ", max grid error " << worst << " samples";
    std::cout << '\n';
}

std::vector<float> pluck(const m3::Swell& swell)
{
    const auto frames = swell.sourceEnd - swell.sourceStart;
    std::vector<float> input(static_cast<size_t>(frames) * 2, 0);
    for (int64_t i = swell.arrival - swell.sourceStart; i < frames; ++i)
    {
        const auto t = static_cast<double>(swell.sourceStart + i - swell.arrival) / rate;
        const auto attack = std::min(1.0, t / 0.0015);
        const auto tail = std::clamp((0.25 - t) / 0.025, 0.0, 1.0);
        double value = 0;
        for (const auto hz : {130.81, 155.56, 196.0, 293.66})
        {
            const auto phase = 2 * std::numbers::pi * hz * t;
            value += (std::sin(phase) + 0.32 * std::sin(phase * 2) + 0.12 * std::sin(phase * 3));
        }
        const auto sample = static_cast<float>(0.13 * value * attack * tail * std::exp(-18 * t));
        input[static_cast<size_t>(i) * 2] = input[static_cast<size_t>(i) * 2 + 1] = sample;
    }
    return input;
}

struct Clip
{
    const char* name;
    m3::Swell swell;
    const ImpulseResponse& ir;
};

void writeListeningClips(const juce::File& folder, const std::vector<Clip>& clips)
{
    constexpr auto start = arrival - 132000;
    constexpr auto frames = 192000;
    std::vector<std::vector<float>> mixes;
    float wetPeak = 0;
    for (const auto& clip : clips)
    {
        auto dry = pluck(clip.swell);
        const auto wet = render(clip.swell, clip.ir, dry);
        wetPeak = std::max(wetPeak, wet.peak);
        auto& mix = mixes.emplace_back(frames * 2, 0.0f);
        for (int64_t i = 0; i < frames; ++i)
            for (int channel = 0; channel < 2; ++channel)
                mix[static_cast<size_t>(i) * 2 + channel] = sampleAt(wet, start + i, channel);
    }
    REQUIRE(wetPeak > 0);
    const auto sharedWetGain = 0.48f / wetPeak;
    for (size_t clipIndex = 0; clipIndex < clips.size(); ++clipIndex)
    {
        const auto& clip = clips[clipIndex];
        const auto dry = pluck(clip.swell);
        auto& mix = mixes[clipIndex];
        for (int64_t i = 0; i < frames; ++i)
        {
            const auto at = start + i;
            const auto dryIndex = at - clip.swell.sourceStart;
            const auto beat = (at - beatOne) % 19200;
            const auto t = static_cast<double>(beat) / rate;
            const auto click = beat >= 0 && beat < 960
                ? static_cast<float>(0.045 * std::sin(2 * std::numbers::pi * 1450 * t) * std::exp(-240 * t))
                : 0.0f;
            for (int channel = 0; channel < 2; ++channel)
            {
                const auto plucked = dryIndex >= 0 && dryIndex < static_cast<int64_t>(dry.size() / 2)
                    ? dry[static_cast<size_t>(dryIndex) * 2 + channel] : 0.0f;
                auto& output = mix[static_cast<size_t>(i) * 2 + channel];
                output = output * sharedWetGain + plucked * 0.65f + click;
                REQUIRE(std::abs(output) < 1.0f);
            }
        }
        const auto file = folder.getChildFile(clip.name);
        REQUIRE(!file.exists() || file.deleteFile());
        writeTestWav(file, 2, rate, mix, 24);
    }
    std::cout << "Shared listening wet gain " << sharedWetGain << '\n';
}
}

int main()
{
    return runTest([]
    {
        const auto referencePath = juce::SystemStats::getEnvironmentVariable("REVERBARISE_PING_PONG_SPIKE_REFERENCE", {});
        const auto outputPath = juce::SystemStats::getEnvironmentVariable("REVERBARISE_PING_PONG_SPIKE_OUTPUT", {});
        REQUIRE(referencePath.isNotEmpty() && outputPath.isNotEmpty());
        ImpulseResponse reference;
        juce::String error;
        REQUIRE(loadImpulseResponse(juce::File(referencePath), previewMemoryBudget, reference, error));
        REQUIRE(reference.rate == rate && reference.channels == 2 && reference.frames() == frameCount);
        const auto bursts = findBursts(reference);
        const auto swell = makeSwell(arrival);
        Response unchanged{reference, {}};
        for (const auto& burst : bursts)
            unchanged.hits.push_back({burst, arrival - preRoll + burst.peak});
        const auto eighth = makeResponse(reference, bursts, swell, 0.5, songClock(), false);
        const auto dotted = makeResponse(reference, bursts, swell, 0.75, songClock(), false);
        const auto tight = makeResponse(reference, bursts, swell, 0.25, songClock(300), false);
        verify("reference", swell, unchanged);
        verify("arrival-relative eighth", swell, eighth);
        verify("arrival-relative dotted eighth", swell, dotted);
        verify("tight sixteenth at 300 BPM", swell, tight);
        REQUIRE(eighth.hits.back().peak == unchanged.hits.back().peak);
        REQUIRE(dotted.hits.back().peak == unchanged.hits.back().peak);
        REQUIRE(eighth.hits[1].peak - eighth.hits[0].peak == 9600);
        REQUIRE(dotted.hits[1].peak - dotted.hits[0].peak == 14400);
        REQUIRE(tight.hits[1].peak - tight.hits[0].peak == 2400);

        const auto song = songClock();
        const auto bars = makeSongBarRuler(beatOne, arrival, 900, rate, song.bpm, 4, 4, song.sample, song.ppq);
        REQUIRE(!bars.empty() && bars.front().bar == 1 && bars.front().samplePosition == beatOne);
        const auto grid = m3::HostGrid::make(song, song, rate, m3::GridDivision::beat);
        REQUIRE(grid.has_value());
        const auto edit = [&](m3::Swell& candidate, m3::TimingAnchor kind, int64_t requested, bool snap)
        {
            const auto anchor = candidate;
            REQUIRE(m3::applyTimingCandidate(candidate, anchor, kind, requested,
                anchor.sourceStart - preRoll, anchor.sourceEnd, snap, &*grid, beatOne, error));
        };
        auto on = swell;
        edit(on, m3::TimingAnchor::arrival, arrival + 1200, true);
        REQUIRE(on.arrival == arrival);
        auto before = swell;
        edit(before, m3::TimingAnchor::arrival, arrival - 1200, false);
        auto after = swell;
        edit(after, m3::TimingAnchor::arrival, arrival + 1200, false);
        REQUIRE(before.arrival == arrival - 1200 && after.arrival == arrival + 1200);
        const auto onGrid = makeResponse(reference, bursts, on, 0.5, song, true);
        const auto beforeGrid = makeResponse(reference, bursts, before, 0.5, song, true);
        const auto afterGrid = makeResponse(reference, bursts, after, 0.5, song, true);
        const auto afterRelative = makeResponse(reference, bursts, after, 0.5, song, false);
        const auto beforeRelative = makeResponse(reference, bursts, before, 0.5, song, false);
        const auto dottedGrid = makeResponse(reference, bursts, after, 0.75, song, true);
        const auto dottedRelative = makeResponse(reference, bursts, after, 0.75, song, false);
        verify("snapped Target, ruler eighth", on, onGrid, 0.5);
        verify("off-grid Target before boundary, ruler eighth", before, beforeGrid, 0.5);
        verify("off-grid Target after boundary, ruler eighth", after, afterGrid, 0.5);
        verify("off-grid Target, ruler dotted eighth", after, dottedGrid, 0.75);
        REQUIRE(afterGrid.hits.back().peak - beforeGrid.hits.back().peak == 9600);
        REQUIRE(dottedGrid.hits.front().peak < arrival && dottedGrid.hits.back().peak >= arrival);
        REQUIRE(dottedGrid.hits.back().peak - dottedGrid.hits[dottedGrid.hits.size() - 2].peak == 14400);
        const auto fractional = makeResponse(reference, bursts, after, 0.75, songClock(137), true);
        verify("off-grid Target, fractional sample spacing at 137 BPM", after, fractional, 0.75, 137);

        auto moved = after;
        const auto moveAnchor = after.sourceStart - preRoll;
        edit(moved, m3::TimingAnchor::move, moveAnchor - 1800, false);
        REQUIRE(moved.placement == -1800);
        const auto movedGrid = makeResponse(reference, bursts, moved, 0.5, song, true);
        verify("Move Wet with Snap off", moved, movedGrid, 0.5);
        auto snappedMove = after;
        edit(snappedMove, m3::TimingAnchor::move, moveAnchor - 5000, true);
        REQUIRE(snappedMove.placement != 0 && snappedMove.placement != -5000);
        const auto snappedGrid = makeResponse(reference, bursts, snappedMove, 0.5, song, true);
        verify("Move Wet with Snap on", snappedMove, snappedGrid, 0.5);

        auto bounded = after;
        edit(bounded, m3::TimingAnchor::sourceStart, after.sourceStart - 950, false);
        const auto boundedGrid = makeResponse(reference, bursts, bounded, 0.5, song, true);
        verify("edited source start", bounded, boundedGrid, 0.5);
        edit(bounded, m3::TimingAnchor::sourceEnd, bounded.sourceEnd + 900, false);
        const auto originalInput = impulseAtArrival(bounded);
        auto extendedInput = originalInput;
        extendedInput.resize(static_cast<size_t>(bounded.sourceEnd - bounded.sourceStart) * 2);
        const auto extended = render(bounded, boundedGrid.ir, extendedInput);
        for (const auto& hit : boundedGrid.hits)
            REQUIRE(std::abs(std::abs(sampleAt(extended, hit.peak, hit.burst.channel))
                             - hit.burst.amplitude) < 0.00002f);
        const auto crop = boundedGrid.hits[1].peak + 100;
        edit(bounded, m3::TimingAnchor::start, crop, false);
        edit(bounded, m3::TimingAnchor::fadeIn, crop + 16000, false);
        const auto shaped = render(bounded, boundedGrid.ir, extendedInput);
        REQUIRE(shaped.start == crop);
        REQUIRE(sampleAt(shaped, boundedGrid.hits.front().peak, boundedGrid.hits.front().burst.channel) == 0);
        float previous = 0;
        for (const auto& hit : boundedGrid.hits)
        {
            if (hit.peak < crop) continue;
            const auto shapedSample = sampleAt(shaped, hit.peak, hit.burst.channel);
            const auto gain = m3::swellEnvelope(bounded, hit.peak, shaped.start,
                shaped.start + static_cast<int64_t>(shaped.frames()));
            REQUIRE(std::abs(shapedSample - sampleAt(extended, hit.peak, hit.burst.channel) * gain) < 0.00002);
            REQUIRE(std::abs(shapedSample) > previous);
            previous = std::abs(shapedSample);
        }
        std::cout << "Source bounds, crop and fade: retained anchors; first hits masked\n";

        writeListeningClips(juce::File(outputPath), {
            {"01-original-220ms.wav", swell, reference},
            {"02-eighth-arrival-relative.wav", swell, eighth.ir},
            {"03-dotted-arrival-relative.wav", swell, dotted.ir},
            {"04-before-relative.wav", before, beforeRelative.ir},
            {"05-before-ruler.wav", before, beforeGrid.ir},
            {"06-after-relative.wav", after, afterRelative.ir},
            {"07-after-ruler.wav", after, afterGrid.ir},
            {"08-dotted-off-grid-relative.wav", after, dottedRelative.ir},
            {"09-dotted-off-grid-ruler.wav", after, dottedGrid.ir}
        });
    });
}
