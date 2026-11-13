#pragma once

#include <Core/SongTime.h>
#include <Core/SoundPreset.h>
#include "ReverseConvolution.h"

namespace m3
{
struct PingPongTiming
{
    double sampleRate = 48000;
    int64_t arrival = 0;
    int64_t wetPlacement = 0;
    SongTime ruler;
};

bool preparePingPong(const PingPongSettings&, const PingPongTiming&, uint64_t budget,
                     ImpulseResponse&, SoundPresetIdentity&, juce::String& error);

float pingPongMatchingOffsetDb(const SoundPresetIdentity&);
bool preparePingPongExcitation(std::vector<float>& stereo, double rate, size_t arrivalFrame,
                               const SoundPresetIdentity&, double rulerBpm, const RenderCancellation&);
}
