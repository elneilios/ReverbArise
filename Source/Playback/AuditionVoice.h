#pragma once
#include <DSP/ReverseConvolution.h>
#include <array>
#include <atomic>
#include <memory>
#include <optional>
#include <limits>
#include <algorithm>
#include <cmath>

// Control methods have one serialized caller. process() is the only audio-thread method.
// Shared ownership is confined to control code; the callback uses raw pinned pointers.
class AuditionVoice
{
public:
    bool start(std::shared_ptr<const WetRender> buffer, float attenuation = 1.0f);
    bool replace(std::shared_ptr<const WetRender> buffer);
    void setLevel(float gain) noexcept { level.store(std::isfinite(gain) ? std::clamp(gain, 0.0f, 1.0f) : 0.0f); }
    void stop(bool immediately = false) noexcept;
    void collect();
    void process(float* left, float* right, int frames, double rate, bool allowed = true) noexcept;
    std::optional<int64_t> position() const noexcept
    {
        for(int attempt=0;attempt<8;++attempt){const auto seq=positionSequence.load(std::memory_order_acquire);if(seq&1)continue;
            const auto activeNow=positionActive.load();const auto sample=positionSample.load();
            if(seq==positionSequence.load(std::memory_order_acquire))return activeNow?std::optional<int64_t>{sample}:std::nullopt;}
        return std::nullopt;
    }
    bool isPlaying() const noexcept { return playing.load(); }
    bool isActiveOrPending() const noexcept {return playing.load() || write.load()!=read.load();}
    size_t pinnedCount() const noexcept { return pins.size(); }
    uint64_t pinnedBytes() const noexcept;
private:
    struct Command { const WetRender* buffer = nullptr; uint64_t serial = 0, epoch = 0; float attenuation = 1; bool replace=false; };
    struct Pin { uint64_t serial; std::shared_ptr<const WetRender> buffer; };
    std::array<Command, 8> commands;
    std::vector<Pin> pins;
    std::atomic<uint64_t> write{0}, read{0}, acknowledged{0}, activeSerial{0};
    std::atomic<uint64_t> transitionSerial{0};
    std::atomic<uint64_t> ownershipSequence{0};
    std::atomic<uint64_t> stopEpoch{0}, immediateEpoch{0};
    std::atomic<bool> playing{false},positionActive{false};
    std::atomic<int64_t> positionSample{0};
    std::atomic<uint64_t> positionSequence{0};
    uint64_t serial = 0, seenEpoch = 0, consumed = 0, activeId = 0;
    const WetRender* active = nullptr;
    const WetRender* previous=nullptr;
    uint64_t previousId=0;
    size_t previousCursor=0;
    int transitionRemaining=0,transitionLength=1;
    float previousAttenuation=1;
    size_t cursor = 0;
    int rampRemaining = 0, rampLength = 1;
    std::atomic<float> level{1};
    float activeAttenuation = 1, smoothedLevel = 1, levelStep = 0, targetLevel = 1;
    int levelRemaining = 0;
};
