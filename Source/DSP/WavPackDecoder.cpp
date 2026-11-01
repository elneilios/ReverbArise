#include "WavPackDecoder.h"
#include <wavpack.h>
#include <algorithm>
#include <bit>
#include <cmath>
#include <memory>

bool loadWavPackImpulseResponse(const juce::File& file, uint64_t available,
                                ImpulseResponse& output, juce::String& error)
{
    char message[80]{};
    const auto path = file.getFullPathName().toUTF8();
    std::unique_ptr<WavpackContext, decltype(&WavpackCloseFile)> decoder(
        WavpackOpenFileInput(path.getAddress(), message, OPEN_FILE_UTF8 | OPEN_WVC, 0), &WavpackCloseFile);
    if (! decoder)
    { error = "Cannot decode WavPack IR: " + juce::String::fromUTF8(message); return false; }
    const auto channels = WavpackGetNumChannels(decoder.get());
    const auto frames = WavpackGetNumSamples64(decoder.get());
    const auto rate = WavpackGetSampleRate(decoder.get());
    const auto bytes = WavpackGetBytesPerSample(decoder.get());
    if (channels < 1 || channels > 2)
    { error = "WavPack IR must be mono or stereo"; return false; }
    if (frames <= 0 || rate == 0 || bytes < 1 || bytes > 4)
    { error = "WavPack IR needs a known non-empty PCM/float length and valid sample format"; return false; }
    // No lossy fallback for hybrid IRs whose correction file is missing.
    if ((WavpackGetMode(decoder.get()) & MODE_LOSSLESS) == 0)
    { error = "Hybrid/lossy WavPack IR requires its lossless .wvc correction file"; return false; }
    constexpr uint64_t decodeFrames = 4096;
    const auto scratchBytes = decodeFrames * static_cast<uint64_t>(channels) * sizeof(int32_t);
    const auto budget = std::min(available, previewMemoryBudget);
    if (budget < scratchBytes || static_cast<uint64_t>(frames) > (budget - scratchBytes) / (channels * sizeof(float)))
    { error = "WavPack IR exceeds 256 MiB preview memory budget"; return false; }
    ImpulseResponse result;
    result.channels = channels; result.rate = rate; result.name = file.getFileName();
    result.identity = juce::Uuid().toDashedString();
    result.samples.resize(static_cast<size_t>(frames) * channels);
    std::vector<int32_t> decoded(static_cast<size_t>(decodeFrames) * channels);
    const auto floatingPoint = (WavpackGetMode(decoder.get()) & MODE_FLOAT) != 0;
    const auto scale = std::ldexp(1.0, 1 - bytes * 8);
    int64_t position = 0;
    while (position < frames)
    {
        const auto count = static_cast<uint32_t>(std::min<int64_t>(decodeFrames, frames - position));
        if (WavpackUnpackSamples(decoder.get(), decoded.data(), count) != count)
        { error = "Truncated or corrupt WavPack IR"; return false; }
        for (size_t i = 0; i < static_cast<size_t>(count) * channels; ++i)
        {
            const auto value = floatingPoint ? std::bit_cast<float>(decoded[i]) : static_cast<float>(decoded[i] * scale);
            if (! std::isfinite(value)) { error = "WavPack IR contains non-finite samples"; return false; }
            result.samples[static_cast<size_t>(position) * channels + i] = value;
        }
        position += count;
    }
    if (WavpackGetNumErrors(decoder.get()) != 0 || WavpackLossyBlocks(decoder.get()))
    { error = "WavPack IR failed integrity verification"; return false; }
    output = std::move(result); error.clear(); return true;
}
