#pragma once

#include "CaptureSession.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

// Local phrase capture has no dependency on the Song clock or sparse map.
class LoopCapture final : private juce::Thread
{
public:
    static constexpr int gateNote = 14;
    static constexpr int maxSeconds = 15;
    static constexpr int maxRate = 192000;
    enum class Phase : uint8_t
    {
        idle, armed, waitForLow, waitForRise, recording, settling,
        cancelled, interrupted, tooLong, invalidAudio, silent, storageError, ready
    };
    struct Waveform
    {
        struct Peak { float minimum = 1, maximum = -1; };
        std::array<Peak, 180> peaks{};
        int size = 0;
    };
    struct View
    {
        bool enabled = false;
        Phase phase = Phase::idle;
        bool acceptedLoop = false, checking = false;
        bool pendingInterruption = false;
        double seconds = 0;
        juce::File wav;
        Waveform waveform;
        bool busy() const noexcept;
    };
    struct Session
    {
        std::shared_ptr<const CaptureStorageLocation> location;
        juce::String project, instance;
        uint64_t generation = 0;
    };
    LoopCapture(std::function<std::optional<Session>()>, std::function<uint64_t()>, std::function<void()>);
    ~LoopCapture() override;
    void shutdown();

    bool enabled() const noexcept { return loopEnabled.load(); }
    bool busy() const noexcept;
    bool hasAcceptedLoop() const noexcept { return acceptedLoop.load(); }
    bool setEnabled(bool);
    bool arm(const Session&);
    void cancel();
    void prepare(double rate) noexcept;
    void invalidate() noexcept { prepareEpoch.fetch_add(1); }
    void process(const juce::AudioBuffer<float>&, const juce::MidiBuffer&, bool eligible,
                 uint64_t generation) noexcept;
    View view() const;
    void acceptSong(uint64_t generation);
    void save(juce::XmlElement&) const;
    void restore(const juce::XmlElement&, uint64_t generation);

private:
    enum class Gate { unknown, low, high };
    struct Take
    {
        juce::String id, hash;
        int rate = 0, frames = 0;
        bool valid() const;
    };
    static Phase phase(uint64_t state) noexcept { return static_cast<Phase>(state & 255); }
    static uint64_t withPhase(uint64_t state, Phase value) noexcept
    {
        return (state & ~uint64_t{255}) | static_cast<uint8_t>(value);
    }
    static bool active(Phase value) noexcept { return value >= Phase::armed && value <= Phase::settling; }
    static juce::File takeFile(const Session&, const Take&);
    static std::optional<Waveform> verify(const juce::File&, const Take&);
    void clearAccepted();
    void run() override;
    void settle(uint64_t state);
    void verifyRestored();

    const std::function<std::optional<Session>()> sessionNow;
    const std::function<uint64_t()> generationNow;
    const std::function<void()> changed;
    mutable juce::CriticalSection control;
    std::atomic<bool> loopEnabled{false}, acceptedLoop{false};
    std::atomic<uint64_t> state{0}, prepareEpoch{0}, armEpoch{0}, armGeneration{0};
    std::atomic<int> preparedRate{0};
    Session target;
    Take accepted;
    juce::File acceptedFile;
    Waveform acceptedWaveform;
    std::optional<uint64_t> restoreGeneration;
    bool settlementCancelled = false;

    // The callback owns the recorder until it publishes Phase::settling.
    // The worker owns its immutable samples until it publishes a terminal phase.
    juce::AudioBuffer<float> audio;
    uint64_t observedEpoch = 0, recordingTicket = 0;
    Gate gate = Gate::unknown;
    int frames = 0, recordingRate = 0;
};
