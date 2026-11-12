#pragma once
#include <Core/SwellSession.h>

namespace m3
{
// Prepared/copied under the engine lock off the callback. The workflow word
// pins only a two-slot index; no string/refcount work enters capture admission.
struct GenerationConfiguration
{
    bool automatic=false;
    float sensitivity=.5f;
    double separationSeconds=.08;
    Asset ir;
    juce::String irName;
    uint64_t revision=0,session=0;
    SoundPresetIdentity soundPreset;
    float levelDb=defaultSwellLevelDb;
};
}
