#pragma once
#include <Core/ManagedAsset.h>

namespace m3
{
inline constexpr const char* defaultImpulseId = "preverb-hall";
inline constexpr const char* defaultImpulseContent = "e07178229546d127295fee703375c233090f02faa312f9ce9a639beee2bfaacf";
inline constexpr const char* customImpulseId = "custom-ir";
inline constexpr float minimumDefaultLevelDb = -12.f, maximumDefaultLevelDb = 12.f;

// Static definitions have no private parameters. Level belongs to the owning
// swell shape or future defaults, independently of the selected response.
struct SoundPresetIdentity
{
    juce::String id;
    int version = 1;
    bool operator==(const SoundPresetIdentity&) const = default;
};

SoundPresetIdentity inferSoundPreset(const Asset&);
bool validSoundPreset(const SoundPresetIdentity&, const Asset&);
inline SoundPresetIdentity resolvedSoundPreset(const SoundPresetIdentity& preset,const Asset& asset)
{
    return preset.id.isEmpty() ? inferSoundPreset(asset) : preset;
}
}
