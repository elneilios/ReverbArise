#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <limits>
#include "CaptureFailure.h"
#include <algorithm>

inline constexpr int maxCaptureBlockFrames = 8192;

struct AudioCaptureBlock
{
    int64_t startSample = 0;
    double sampleRate = 0.0;
    uint32_t frameCount = 0;
    uint64_t generation = 0;
    uint64_t recaptureToken = 0;
    uint64_t queueOrdinal = 0;
    CaptureHostTiming timing;
    std::array<float, maxCaptureBlockFrames * 2> interleavedSamples{};
};

template <size_t Capacity>
class SpscAudioCaptureQueue
{
public:
    bool tryPush(int64_t startSample, double sampleRate, const float* left, const float* right,
                 int frameCount, uint64_t generation = 0, uint64_t recaptureToken = 0, CaptureHostTiming timing = {}) noexcept
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
        block.queueOrdinal = write+1;
        block.timing = timing;

        for (int frame = 0; frame < frameCount; ++frame)
        {
            block.interleavedSamples[static_cast<size_t>(frame) * 2] = left[frame];
            block.interleavedSamples[static_cast<size_t>(frame) * 2 + 1] = right[frame];
        }

        writeIndex.store(write + 1, std::memory_order_release);
        return true;
    }

    // Producer/consumer observation only; a third-party sample is approximate.
    uint32_t depth() const noexcept
    {
        const auto r=readIndex.load(std::memory_order_acquire),w=writeIndex.load(std::memory_order_acquire);
        return static_cast<uint32_t>(std::min<uint64_t>(w-r,Capacity));
    }
    static constexpr uint32_t capacity=static_cast<uint32_t>(Capacity);
    uint64_t totalQueued() const noexcept {return writeIndex.load(std::memory_order_acquire);}
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
