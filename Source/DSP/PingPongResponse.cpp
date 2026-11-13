#include "PingPongResponse.h"
#include <Core/ContentHash.h>
#include <Core/SwellSession.h>
#include <Core/TransportTime.h>
#include <PreverbResources.h>
#include <array>
#include <cstring>
#include <numbers>

namespace m3
{
namespace
{
constexpr double sourceRate = 48000;
constexpr double referenceBpm = 30.0 / 0.22;
constexpr std::array<double,6> beatFractions{1, 0.5, 0.75, 0.25, 0.375, 0.125};
constexpr const char* resourceStem = "stereo-ping-pong-crescendo-v1";

struct Burst
{
    ImpulseResponse kernel;
    int peak = 0, channel = 0;
    float magnitude = 0;
};

const char* resource(const char* extension, int& size)
{
    const auto filename = juce::String(resourceStem) + extension;
    for (int i = 0; i < PreverbResources::namedResourceListSize; ++i)
        if (filename == PreverbResources::originalFilenames[i])
            return PreverbResources::getNamedResource(PreverbResources::namedResourceList[i], size);
    return nullptr;
}

struct Source
{
    std::array<Burst,11> bursts;
    int finalLead = 0;
    bool valid = false;
};

Source readSource()
{
    Source source;
    int wavBytes = 0, manifestBytes = 0;
    const auto* wav = resource(".wav", wavBytes);
    const auto* manifest = resource(".json", manifestBytes);
    if (!wav || !manifest || wavBytes <= 0 || manifestBytes <= 0) return {};
    const auto info = juce::JSON::parse(juce::String::fromUTF8(manifest, manifestBytes));
    const auto* items = info.getProperty("bursts", {}).getArray();
    if (!items || items->size() != static_cast<int>(source.bursts.size())
        || info.getProperty("preset_id", {}).toString() != pingPongPresetId
        || static_cast<int>(info.getProperty("source_version", {})) != 1) return {};
    ContentHash hash;
    hash.update(wav, static_cast<size_t>(wavBytes));
    if (hash.finish() != info.getProperty("packed_wav_sha256", {}).toString()) return {};
    juce::WavAudioFormat format;
    auto stream = std::make_unique<juce::MemoryInputStream>(wav, static_cast<size_t>(wavBytes), false);
    std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(stream.release(), true));
    if (!reader || reader->sampleRate != sourceRate || reader->numChannels != 1
        || reader->bitsPerSample != 24 || reader->lengthInSamples != 32989) return {};
    juce::AudioBuffer<float> pcm(1, static_cast<int>(reader->lengthInSamples));
    if (!reader->read(&pcm, 0, pcm.getNumSamples(), 0, true, false)) return {};
    int expectedStart = 0;
    for (size_t i = 0; i < source.bursts.size(); ++i)
    {
        const auto& item = items->getReference(static_cast<int>(i));
        const auto start = static_cast<int>(item.getProperty("packed_start_frame", {}));
        const auto count = static_cast<int>(item.getProperty("frames", {}));
        const auto peak = static_cast<int>(item.getProperty("peak_offset_frames", {}));
        const auto channel = static_cast<int>(item.getProperty("channel", {}));
        if (start != expectedStart || count <= 0 || count > pcm.getNumSamples() - start
            || peak < 0 || peak >= count || channel != static_cast<int>(i % 2)) return {};
        expectedStart += count;
        const auto* begin = pcm.getReadPointer(0, start);
        auto& burst = source.bursts[i];
        burst.kernel = {sourceRate, 1, {begin, begin + count}, {}, {}};
        burst.peak = peak;
        burst.channel = channel;
        burst.magnitude = std::abs(begin[peak]);
        if (!(burst.magnitude > 0)) return {};
        if (i + 1 == source.bursts.size())
            source.finalLead = 119999 - static_cast<int>(item.getProperty("audible_start_frame", {})) - peak;
    }
    source.valid = expectedStart == pcm.getNumSamples() && source.finalLead > 0;
    return source;
}

bool fail(juce::String& error, const char* reason) { error = reason; return false; }

const Source& pingPongSource()
{
    static const Source source = readSource();
    return source;
}

double buildExponent(int build)
{
    constexpr double reference = PingPongSettings{}.build;
    return build <= reference ? build / reference : 1.0 + 0.5 * (build - reference) / (100.0 - reference);
}

long double echoSpacing(const PingPongSettings& settings, double rate, double bpm)
{
    return static_cast<long double>(rate) * 60 * beatFractions[static_cast<size_t>(settings.rate)] / bpm;
}
}

bool preparePingPongExcitation(std::vector<float>& stereo, double rate, size_t arrivalFrame,
                               const SoundPresetIdentity& preset, double rulerBpm, const RenderCancellation& cancel)
{
    if (!preset.pingPong || !preset.pingPong->valid()) return false;
    const auto amount = preset.pingPong->gateAmount / 100.0f;
    if (amount == 0.0f) return true;
    const auto spacing = echoSpacing(*preset.pingPong, rate, preset.rulerAligned ? rulerBpm : referenceBpm);
    if (!std::isfinite(spacing) || spacing < 1) return false;
    const auto frames = stereo.size() / 2;
    const auto first = static_cast<size_t>(std::max(0.0L, arrivalFrame - std::ceil(spacing)));
    const auto last = static_cast<size_t>(std::min(static_cast<long double>(frames), arrivalFrame + std::ceil(spacing)));
    double correlation = 0;
    for (auto i = first; i < last; ++i)
    {
        if (i % 4096 == 0 && cancel && cancel()) return false;
        correlation += static_cast<double>(stereo[2 * i]) * stereo[2 * i + 1];
    }
    // A phase-inverted stereo capture still needs to excite both wet lanes.
    const auto polarity = correlation < 0 ? -1.0f : 1.0f;
    for (size_t i = 0; i < frames; ++i)
    {
        if (i % 4096 == 0 && cancel && cancel()) return false;
        const auto distance = i < arrivalFrame ? arrivalFrame - i : i - arrivalFrame;
        const auto fade = std::clamp(2.0L * distance / spacing - 1.0L, 0.0L, 1.0L);
        const auto weight = static_cast<float>(1 - fade * fade * (3 - 2 * fade));
        const auto wet = (stereo[2 * i] + polarity * stereo[2 * i + 1]) * (0.5f * weight);
        stereo[2 * i] = std::lerp(stereo[2 * i], wet, amount);
        stereo[2 * i + 1] = std::lerp(stereo[2 * i + 1], wet, amount);
    }
    return true;
}

float pingPongMatchingOffsetDb(const SoundPresetIdentity& preset)
{
    if (preset.id != pingPongPresetId || preset.version < 3 || !preset.pingPong)
        return 0.0f;
    const auto& source = pingPongSource();
    if (!source.valid) return 0.0f;

    // Matching against the loudest wet window otherwise makes the late hits
    // come back up when Build reduces them. Preserve their authored change
    // relative to the reference build, before the existing headroom check.
    const auto riseDb = 20.0 * std::log10(source.bursts.back().magnitude / source.bursts.front().magnitude);
    const auto buildOffset = riseDb * (buildExponent(preset.pingPong->build) - 1.0);
    // Let the strongly damped response recede in the mix. Matching it
    // back to the dry attack can hide the reduction in audible ringing.
    const auto dampingOffset = preset.version >= pingPongExtendedDampingVersion
        ? -6.0 * std::max(0, preset.pingPong->damping - 50) / 50.0
        : 0.0;
    return static_cast<float>(buildOffset + dampingOffset);
}

bool preparePingPong(const PingPongSettings& settings, const PingPongTiming& timing, uint64_t budget,
                     ImpulseResponse& output, SoundPresetIdentity& identity, juce::String& error)
{
    const auto rate = timing.sampleRate;
    if (!settings.valid() || !std::isfinite(rate) || rate < 8000 || rate > 384000)
        return fail(error, "Invalid Ping Pong settings or sample rate");
    const auto frames = static_cast<int64_t>(std::llround(2.5 * rate));
    if (static_cast<uint64_t>(frames) * 2 * sizeof(float) + 1024 * 1024 > budget)
        return fail(error, "Ping Pong response exceeds working budget");
    const auto& source = pingPongSource();
    if (!source.valid) return fail(error, "The embedded Ping Pong source could not be verified");

    int64_t positionedEnd, positionedStart;
    if (!checkedAdd(timing.arrival, timing.wetPlacement, positionedEnd)
        || !checkedSubtract(positionedEnd, frames - 1, positionedStart))
        return fail(error, "Ping Pong response is outside the timeline range");
    const auto ratio = rate / sourceRate;
    long double origin = 0;
    const auto aligned = settings.tempoSync && timing.ruler.rate == rate && songOrigin(timing.ruler, origin);
    const auto bpm = aligned ? timing.ruler.bpm : referenceBpm;
    const auto step = echoSpacing(settings, rate, bpm);
    if (step < 1 || !std::isfinite(step)) return fail(error, "Ping Pong tempo is outside the usable range");
    const auto& last = source.bursts.back();
    const auto lastTail = static_cast<int64_t>(std::llround(last.kernel.frames() * ratio)) - 1
        - static_cast<int64_t>(std::llround(last.peak * ratio));
    int64_t lastAllowed;
    if (!checkedSubtract(std::min(timing.arrival, positionedEnd), lastTail, lastAllowed))
        return fail(error, "Ping Pong response is outside the timeline range");
    const auto lastTick = aligned ? std::floor((static_cast<long double>(lastAllowed) + 0.5L - origin) / step) : 0;
    auto gridEnd = aligned ? std::round(origin + lastTick * step) : 0;
    if (aligned && gridEnd > lastAllowed) gridEnd = std::round(origin + (lastTick - 1) * step);
    const auto gridIndex = lastTick - (aligned && gridEnd != std::round(origin + lastTick * step) ? 1 : 0);
    const auto localLast = frames - 1 - std::llround(source.finalLead * ratio);
    ImpulseResponse result{rate, 2, std::vector<float>(static_cast<size_t>(frames) * 2), pingPongPresetName, {}};
    const auto pan = (1.0 - settings.spread / 100.0) * std::numbers::pi / 4.0;
    const auto primary = static_cast<float>(std::cos(pan));
    const auto secondary = static_cast<float>(std::sin(pan));
    const auto build = buildExponent(settings.build);
    const auto firstMagnitude = source.bursts.front().magnitude;
    const auto damping = settings.damping <= 50
        ? std::lerp(0.0, 1.0 / .013, settings.damping / 50.0)
        : std::lerp(1.0 / .013, 1.0 / .0012, (settings.damping - 50) / 50.0);
    const auto protectedPeakSeconds = std::lerp(.0025, .00025, std::max(0, settings.damping - 50) / 50.0);
    bool placed = false;
    for (size_t i = 0; i < source.bursts.size(); ++i)
    {
        const auto& burst = source.bursts[source.bursts.size() - 1 - i];
        const auto anchor = aligned ? std::round(origin + (gridIndex - i) * step) - positionedStart
                                    : static_cast<long double>(localLast) - std::round(step * i);
        const auto peakOffset = std::llround(burst.peak * ratio);
        if (anchor < peakOffset) break;
        if (anchor >= frames) return fail(error, "No complete Ping Pong burst fits at this placement");
        const auto start = static_cast<int64_t>(anchor) - peakOffset;
        ImpulseResponse resampled;
        const auto* kernel = &burst.kernel;
        if (rate != sourceRate)
        {
            if (!prepareImpulseResponse(*kernel, rate, budget - result.samples.size() * sizeof(float),
                                        resampled, error)) return false;
            kernel = &resampled;
        }
        if (kernel->frames() > static_cast<size_t>(frames - start))
            return fail(error, "No complete Ping Pong burst fits at this placement");
        // Keep the first burst as the reference so reducing Build quiets
        // the later hits instead of making the earlier hits louder.
        const auto gain = static_cast<float>(std::pow(burst.magnitude / firstMagnitude, build - 1));
        for (size_t frame = 0; frame < kernel->frames(); ++frame)
        {
            const auto index = static_cast<size_t>(frames - 1 - start) - frame;
            auto sample = kernel->samples[frame] * gain;
            if (damping > 0.0)
            {
                // The packed wet burst peaks at the end of its resonant
                // lead-in. Preserve the peak and taper away from it.
                const auto distance = std::abs(static_cast<double>(frame) - peakOffset) / rate;
                const auto tail = std::max(0.0, distance - protectedPeakSeconds) * damping;
                sample *= static_cast<float>(std::exp(-tail - std::expm1(-tail)));
            }
            result.samples[index * 2 + burst.channel] += sample * primary;
            result.samples[index * 2 + 1 - burst.channel] += sample * secondary;
        }
        placed = true;
    }
    if (!placed) return fail(error, "No complete Ping Pong burst fits at this placement");
    float peak = 0;
    for (const auto sample : result.samples) peak = std::max(peak, std::abs(sample));
    if (peak > 0.98f)
        for (auto& sample : result.samples) sample *= 0.98f / peak;
    result.identity = sampleHash(result.samples, rate, 2);
    identity = {pingPongPresetId, pingPongPresetVersion, settings, result.identity, aligned};
    output = std::move(result);
    error.clear();
    return true;
}
}
