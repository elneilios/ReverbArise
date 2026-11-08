#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <DSP/ReverseConvolution.h>
#include <atomic>
#include <memory>
#include <optional>
#include <vector>

class LoopPlayback
{
public:
    enum class Status { waiting, standby, queued, leadIn, repeating, stopping, referenceMismatch, syncLost };
    static constexpr int phraseNote=12, cueNote=16, stopNote=17;
    struct Mix
    {
        uint64_t revision=0;
        int period=0, lead=0;
        std::vector<float> initial, repeat;
        uint64_t bytes() const noexcept { return (initial.capacity()+repeat.capacity())*sizeof(float); }
    };
    static std::unique_ptr<Mix> prepare(const WetRender&,int period,uint64_t budget,uint64_t revision);

    // Serialized control methods. Each audio hazard holds an immutable mix;
    // allocation and retirement stay with the publisher.
    void configure(int rate,int frames) noexcept;
    void publish(std::unique_ptr<Mix>);
    void collect();
    uint64_t pinnedBytes() const noexcept;
    bool cue() noexcept;
    void stop() noexcept;
    bool ready() const noexcept { return published.load()!=nullptr; }
    Status status() const noexcept { return currentStatus.load(); }
    std::optional<int> cursor() const noexcept
    {
        const auto sample=cursorSample.load(std::memory_order_relaxed);
        return sample==INT32_MIN?std::nullopt:std::optional<int>{sample};
    }
    void process(float*,float*,int,double,const juce::MidiBuffer&,bool allowed,bool reset) noexcept;

private:
    static constexpr uint64_t stopCommand=UINT64_MAX;
    void reset() noexcept;
    void stopWet(int fade) noexcept;
    void releaseWet() noexcept;

    std::vector<std::unique_ptr<Mix>> pins;
    std::atomic<const Mix*> published{nullptr}, reading{nullptr}, held{nullptr};
    std::atomic<uint64_t> configuration{0},command{0};
    std::atomic<Status> currentStatus{Status::waiting};
    std::atomic<int> cursorSample{INT32_MIN};
    uint64_t activeConfiguration=0;
    const Mix* active=nullptr;
    int64_t position=0,anchor=-1,launch=-1;
    int matches=0,fadeRemaining=0;
    Status unlockedStatus=Status::waiting;
};
