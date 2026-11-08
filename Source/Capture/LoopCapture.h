#pragma once

#include "CaptureSession.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

// Local phrase capture has no dependency on the Song clock or sparse map.
class LoopCapture final : private juce::Thread
{
public:
    static constexpr int gateNote = 14, phraseNote = 12, barNote = 19, beatNote = 21, resetNote = 22;
    static constexpr int maxBars = 16, maxBeats = 15;
    enum class Method : uint8_t { followGate, fixedLength, legacyGate };
    struct Settings
    {
        Method method = Method::followGate;
        int bars = 1, beats = 0;
        bool operator==(const Settings&) const = default;
    };
    struct MusicalLength
    {
        int beats = 0, beatsPerBar = 0;
        bool operator==(const MusicalLength&) const = default;
    };
    static constexpr int maxSeconds = 15;
    static constexpr int maxRate = 192000;
    enum class Phase : uint8_t
    {
        idle, armed, waitForLow, waitForRise, recording, waitForBar, settling,
        cancelled, interrupted, tooLong, invalidAudio, silent, storageError, ready, invalidStart, invalidLength
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
        Settings settings;
        bool clockReady = false;
        MusicalLength musicalLength;
        bool busy() const noexcept;
    };
    struct Session
    {
        std::shared_ptr<const CaptureStorageLocation> location;
        juce::String project, instance;
        uint64_t generation = 0;
    };
    struct Source
    {
        juce::String id, hash;
        int rate = 0, frames = 0;
        juce::File file;
        MusicalLength musicalLength;
        bool read(int first, int end, uint64_t budget, std::vector<float>&, juce::String&) const;
        bool operator==(const Source&) const = default;
    };
    LoopCapture(std::function<std::optional<Session>()>, std::function<uint64_t()>, std::function<void()>);
    ~LoopCapture() override;
    void shutdown();

    bool enabled() const noexcept { return loopEnabled.load(); }
    bool busy() const noexcept;
    bool hasAcceptedLoop() const noexcept { return acceptedLoop.load(); }
    bool isPreparedAt(int rate) const noexcept { return rate>0 && preparedRate.load()==rate; }
    bool setEnabled(bool);
    bool setSettings(Settings);
    bool arm(const Session&);
    void cancel();
    void prepare(double rate) noexcept;
    void invalidate() noexcept
    {
        prepareEpoch.fetch_add(1);
        clockReady.store(false, std::memory_order_relaxed);
    }
    void process(const juce::AudioBuffer<float>&, const juce::MidiBuffer&, bool eligible,
                 uint64_t generation) noexcept;
    View view() const;
    std::optional<Source> source() const;
    void acceptSong(uint64_t generation);
    void save(juce::XmlElement&) const;
    void restore(const juce::XmlElement&, uint64_t generation);

private:
    enum class Gate { unknown, low, high };
    struct Take
    {
        juce::String id, hash;
        int rate = 0, frames = 0;
        MusicalLength musicalLength;
        bool valid() const;
    };
    void processMusical(const juce::AudioBuffer<float>&, const juce::MidiBuffer&, uint64_t, bool, uint64_t) noexcept;
    struct Clock
    {
        int64_t nextSample = 0, lastBeat = -1;
        int period = 0, beatsSinceBar = 0, meter = 0;
        bool sawBar = false;
        void clear() noexcept;
        bool expired(int64_t sample) const noexcept;
        bool tick(int64_t sample, int rate, bool bar) noexcept;
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
    Settings settings;
    std::atomic<uint32_t> armedSettings{0};
    std::atomic<bool> musicalRoute{true}, clockReady{false};
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
    Clock clock;
    MusicalLength recordedLength;
    int fixedEnd = 0;
};
