#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <limits>

inline constexpr int maxCaptureBlockFrames = 8192;

struct AudioCaptureBlock
{
    int64_t startSample = 0;
    double sampleRate = 0.0;
    uint32_t frameCount = 0;
    uint64_t generation = 0;
    uint64_t recaptureToken = 0;
    std::array<float, maxCaptureBlockFrames * 2> interleavedSamples{};
};

template <size_t Capacity>
class SpscAudioCaptureQueue
{
public:
    bool tryPush(int64_t startSample, double sampleRate, const float* left, const float* right,
                 int frameCount, uint64_t generation = 0, uint64_t recaptureToken = 0) noexcept
    {
        if (frameCount <= 0 || frameCount > maxCaptureBlockFrames
            || startSample > std::numeric_limits<int64_t>::max() - frameCount)
            return false;

        const auto write = writeIndex.load(std::memory_order_relaxed);
        const auto read = readIndex.load(std::memory_order_acquire);
        if (write - read >= Capacity)
            return false;

        auto& block = blocks[write % Capacity];
        block.startSample = startSample;
        block.sampleRate = sampleRate;
        block.frameCount = static_cast<uint32_t>(frameCount);
        block.generation = generation;
        block.recaptureToken = recaptureToken;

        for (int frame = 0; frame < frameCount; ++frame)
        {
            block.interleavedSamples[static_cast<size_t>(frame) * 2] = left[frame];
            block.interleavedSamples[static_cast<size_t>(frame) * 2 + 1] = right[frame];
        }

        writeIndex.store(write + 1, std::memory_order_release);
        return true;
    }

    bool tryPop(AudioCaptureBlock& destination) noexcept
    {
        const auto read = readIndex.load(std::memory_order_relaxed);
        const auto write = writeIndex.load(std::memory_order_acquire);
        if (read == write)
            return false;

        destination = blocks[read % Capacity];
        readIndex.store(read + 1, std::memory_order_release);
        return true;
    }

private:
    std::array<AudioCaptureBlock, Capacity> blocks{};
    alignas(64) std::atomic<uint64_t> writeIndex{0};
    alignas(64) std::atomic<uint64_t> readIndex{0};
};
