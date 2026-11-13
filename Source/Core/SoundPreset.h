#pragma once
#include <Core/ManagedAsset.h>
#include <optional>

namespace m3
{
inline constexpr const char* defaultImpulseId = "preverb-hall";
inline constexpr const char* defaultImpulseContent = "e07178229546d127295fee703375c233090f02faa312f9ce9a639beee2bfaacf";
inline constexpr const char* customImpulseId = "custom-ir";
inline constexpr const char* pingPongPresetId = "stereo-ping-pong-crescendo";
inline constexpr const char* pingPongPresetName = "Stereo Ping Pong Crescendo";
inline constexpr int pingPongExcitationVersion = 4;
inline constexpr int pingPongGateVersion = 5;
inline constexpr int pingPongDampingVersion = 6;
inline constexpr int pingPongExtendedDampingVersion = 7;
inline constexpr int pingPongPresetVersion = pingPongExtendedDampingVersion;
inline constexpr float minimumDefaultLevelDb = -12.f, maximumDefaultLevelDb = 12.f;

enum class EchoRate { quarter, eighth, dottedEighth, sixteenth, dottedSixteenth, thirtySecond };
struct SoundShaping
{
    static constexpr int minimumCutoff = 20, maximumCutoff = 20000;
    int damping = 0, lowCut = 0, highCut = 0;

    bool valid() const noexcept
    {
        const auto cutoff = [](int hz) { return hz == 0 || (hz >= minimumCutoff && hz <= maximumCutoff); };
        return damping >= 0 && damping <= 100 && cutoff(lowCut) && cutoff(highCut);
    }
    bool operator==(const SoundShaping&) const = default;
};
struct PingPongSettings
{
    bool tempoSync = true;
    EchoRate rate = EchoRate::eighth;
    int spread = 85, build = 75, gateAmount = 0, damping = 0;
    bool valid() const noexcept
    {
        return rate >= EchoRate::quarter && rate <= EchoRate::thirtySecond
            && spread >= 0 && spread <= 100 && build >= 0 && build <= 100
            && gateAmount >= 0 && gateAmount <= 100 && damping >= 0 && damping <= 100;
    }
    bool operator==(const PingPongSettings&) const = default;
};

struct SoundPresetIdentity
{
    juce::String id;
    int version = 1;
    std::optional<PingPongSettings> pingPong;
    juce::String responseKey;
    bool rulerAligned = false;
    SoundShaping shaping;
    SoundShaping effectiveShaping() const noexcept
    {
        auto values = shaping;
        if (pingPong) values.damping = pingPong->damping;
        return values;
    }
    void setShaping(SoundShaping values) noexcept
    {
        if (pingPong) { pingPong->damping = values.damping; values.damping = 0; }
        shaping = values;
    }
    bool operator==(const SoundPresetIdentity&) const = default;
};

SoundPresetIdentity inferSoundPreset(const Asset&);
bool validSoundPreset(const SoundPresetIdentity&, const Asset&);
inline SoundPresetIdentity resolvedSoundPreset(const SoundPresetIdentity& preset,const Asset& asset)
{
    if (!preset.id.isEmpty()) return preset;
    auto inferred = inferSoundPreset(asset);
    inferred.shaping = preset.shaping;
    return inferred;
}
}
