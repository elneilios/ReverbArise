#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include <atomic>
#if PROBE_CAPTURE
#include "OneTake.h"
#endif

namespace
{
constexpr int queueCapacity = 4096;
constexpr int reportLimit = 12000;
constexpr std::array<const char*, 11> streamNames{
    "callback", "gate_parameter", "pulse_parameter", "gate_audio", "pulse_audio", "pulse_midi", "gate_midi",
    "take_start", "take_end", "arm", "cancel"
};

struct Event
{
    juce::int64 sample = 0;
    juce::int64 blockStart = 0;
    int blockSize = 0;
    int sampleRate = 0;
    int stream = 0;
    bool high = false;
    float value = 0;
};

class Probe final : public juce::AudioProcessor, private juce::Timer
{
public:
    Probe()
        : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          traceFile(juce::File(PROBE_TRACE_DIRECTORY).getChildFile("trace-" + juce::Uuid().toString() + ".csv"))
    {
        addParameter(gate = new juce::AudioParameterFloat({ "gate", 1 }, "Capture Gate", 0.0f, 1.0f, 0.0f));
        addParameter(pulse = new juce::AudioParameterFloat({ "pulse", 1 }, "Phrase Pulse", 0.0f, 1.0f, 0.0f));
        output = traceFile.createOutputStream();
        if (output && output->openedOk())
            output->writeText("sample,blockStart,blockSize,sampleRate,stream,high,value\n", false, false, "\n");
        startTimerHz(5);
    }

    ~Probe() override
    {
        stopTimer();
        timerCallback();
    }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return PROBE_MIDI_INPUT; }
    bool producesMidi() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}
    void releaseResources() override
    {
#if PROBE_CAPTURE
        capture.prepare(0);
        displayedState.store(capture.state, std::memory_order_relaxed);
#endif
    }

    void prepareToPlay(double rate, int) override
    {
        lastBlockSize = 0;
        known.fill(false);
#if PROBE_CAPTURE
        capture.prepare(juce::roundToInt(rate));
        displayedState.store(capture.state, std::memory_order_relaxed);
#else
        juce::ignoreUnused(rate);
#endif
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
    {
        const int blockSize = buffer.getNumSamples();
        if (blockSize == 0)
            return;

        const int sampleRate = juce::roundToInt(getSampleRate());
        const auto channels = juce::jmin(2, buffer.getNumChannels(), getTotalNumInputChannels());
#if PROBE_CAPTURE
        if (!takePending.load(std::memory_order_acquire))
            capture.releaseTake();
        const auto command = requestedCommand.exchange(Command::none, std::memory_order_acquire);
        if (command == Command::arm)
        {
            capture.arm();
            record({ samples, samples, blockSize, sampleRate, 9,
                     capture.state == OneTake::State::waitForRise, 0.0f });
        }
        if (command == Command::cancel)
        {
            capture.cancel();
            record({ samples, samples, blockSize, sampleRate, 10, false, 0.0f });
        }
        const auto* leftInput = channels > 0 ? buffer.getReadPointer(0) : nullptr;
        const auto* rightInput = channels > 1 ? buffer.getReadPointer(1) : nullptr;
        const auto append = [&](int from, int to) {
            capture.append(leftInput != nullptr ? leftInput + from : nullptr,
                           rightInput != nullptr ? rightInput + from : nullptr, to - from);
        };
        int copiedUntil = 0;
#endif
        if (blockSize != lastBlockSize || sampleRate != lastSampleRate)
        {
            record({ samples, samples, blockSize, sampleRate, 0, false, 0.0f });
#if PROBE_MIDI_INPUT
            record({ samples, samples, blockSize, sampleRate, 5, false, 0.0f });
            record({ samples, samples, blockSize, sampleRate, 6, false, 0.0f });
#endif
            lastBlockSize = blockSize;
            lastSampleRate = sampleRate;
        }

        observe(1, gate->get(), 0, blockSize, sampleRate);
        observe(2, pulse->get(), 0, blockSize, sampleRate);

#if PROBE_MIDI_INPUT
        for (const auto message : midi)
        {
            if (message.numBytes != 3)
                continue;
            const auto type = message.data[0] & 0xf0;
            if (type == 0x80 || type == 0x90)
            {
                constexpr int pulseNote = 12;
                constexpr int gateNote = 14;
                const auto note = message.data[1];
                if (note != pulseNote && note != gateNote)
                    continue;
                const bool high = type == 0x90 && message.data[2] != 0;
                if (message.samplePosition < 0 || message.samplePosition >= blockSize)
                    continue;
                record({ samples + message.samplePosition, samples, blockSize, sampleRate,
                         note == pulseNote ? 5 : 6, high, static_cast<float>(note) });
#if PROBE_CAPTURE
                if (note == gateNote && message.samplePosition >= copiedUntil)
                {
                    append(copiedUntil, message.samplePosition);
                    copiedUntil = message.samplePosition;
                    const auto before = capture.state;
                    capture.edge(high, samples + message.samplePosition);
                    if (before != OneTake::State::recording && capture.state == OneTake::State::recording)
                        record({ capture.take.start, samples, blockSize, sampleRate, 7, true, 0.0f });
                    if (before == OneTake::State::recording && capture.state == OneTake::State::complete)
                    {
                        record({ capture.take.end, samples, blockSize, sampleRate, 8, false, 0.0f });
                        takePending.store(true, std::memory_order_release);
                    }
                }
#endif
            }
        }
#else
        juce::ignoreUnused(midi);
#endif

#if PROBE_CAPTURE
        append(copiedUntil, blockSize);
        displayedState.store(capture.state, std::memory_order_relaxed);
#endif
        for (int channel = 0; channel < channels; ++channel)
        {
            const auto* input = buffer.getReadPointer(channel);
            for (int offset = 0; offset < blockSize; ++offset)
                observe(channel + 3, input[offset], offset, blockSize, sampleRate);
        }
        samples += blockSize;
    }

private:
#if PROBE_CAPTURE
    enum class Command { none, arm, cancel };
    class CaptureEditor;

    void writeCompletedTake()
    {
        if (!takePending.load(std::memory_order_acquire))
            return;
        const auto take = capture.take;
        const auto name = "take-" + traceFile.getFileNameWithoutExtension().substring(6)
                          + "-" + juce::String(take.start);
        const auto wav = traceFile.getSiblingFile(name + ".wav");
        const auto temporary = wav.getSiblingFile(name + ".partial");
        juce::WavAudioFormat format;
        std::unique_ptr<juce::OutputStream> stream = temporary.createOutputStream();
        auto writer = stream ? format.createWriterFor(stream, juce::AudioFormatWriterOptions{}
            .withSampleRate(take.rate).withNumChannels(2).withBitsPerSample(32)) : nullptr;
        const float* channels[]{ capture.left.data(), capture.right.data() };
        const bool written = writer && writer->writeFromFloatArrays(channels, 2, take.frames);
        writer.reset();
        stream.reset();
        if (written && !wav.exists() && temporary.moveFileTo(wav))
        {
            lastWav = wav;
            lastSave = "Saved " + juce::String(take.frames) + " frames at " + juce::String(take.rate) + " Hz";
        }
        else
        {
            temporary.deleteFile();
            lastSave = "Could not write the take WAV";
        }
        takePending.store(false, std::memory_order_release);
    }
#endif

    void observe(int stream, float value, int offset, int blockSize, int sampleRate) noexcept
    {
        constexpr float threshold = 0.20f;
        const auto index = static_cast<std::size_t>(stream - 1);
        const bool high = value >= threshold;
        if (!known[index] || high != levels[index])
        {
            record({ samples + offset, samples, blockSize, sampleRate, stream, high, value });
            known[index] = true;
            levels[index] = high;
        }
    }

    void record(Event event) noexcept
    {
#if PROBE_CAPTURE
        if (event.stream == 3 || event.stream == 4)
        {
            if (audioEventCount >= 10000)
            {
                audioTraceExhausted.store(true, std::memory_order_relaxed);
                return;
            }
            if (recordInQueue(event))
                ++audioEventCount;
            return;
        }
#endif
        if (eventCount >= reportLimit)
        {
            exhausted.store(true, std::memory_order_relaxed);
            return;
        }
        if (recordInQueue(event))
            ++eventCount;
    }

    bool recordInQueue(Event event) noexcept
    {
        const auto slot = queue.write(1);
        if (slot.blockSize1 != 1)
        {
            dropped.fetch_add(1, std::memory_order_relaxed);
            return false;
        }
        events[static_cast<std::size_t>(slot.startIndex1)] = event;
        return true;
    }

    void timerCallback() override
    {
#if PROBE_CAPTURE
        writeCompletedTake();
#endif
        if (!output || !output->openedOk())
            return;

        const auto pending = queue.read(queueCapacity);
        writeEvents(pending.startIndex1, pending.blockSize1);
        writeEvents(pending.startIndex2, pending.blockSize2);
        const int losses = dropped.exchange(0, std::memory_order_relaxed);
        if (losses != 0)
            *output << "# queue dropped " << losses << "\n";
        if (!reportedExhaustion && exhausted.load(std::memory_order_relaxed))
        {
            *output << "# report limit reached\n";
            reportedExhaustion = true;
        }
#if PROBE_CAPTURE
        if (!reportedAudioExhaustion && audioTraceExhausted.load(std::memory_order_relaxed))
        {
            *output << "# audio trace limit reached\n";
            reportedAudioExhaustion = true;
        }
#endif
        output->flush();
    }

    void writeEvents(int start, int count)
    {
        for (int index = start; index < start + count; ++index)
        {
            const auto& event = events[static_cast<std::size_t>(index)];
            *output << event.sample << "," << event.blockStart << "," << event.blockSize << ","
                    << event.sampleRate << "," << streamNames[static_cast<std::size_t>(event.stream)] << ","
                    << static_cast<int>(event.high) << "," << event.value << "\n";
        }
    }

    juce::AudioParameterFloat* gate = nullptr;
    juce::AudioParameterFloat* pulse = nullptr;
    const juce::File traceFile;
    std::unique_ptr<juce::FileOutputStream> output;
    juce::AbstractFifo queue{ queueCapacity };
    std::array<Event, queueCapacity> events{};
    std::array<bool, 4> known{};
    std::array<bool, 4> levels{};
    std::atomic<int> dropped{ 0 };
    std::atomic<bool> exhausted{ false };
    bool reportedExhaustion = false;
    int eventCount = 0;
    int lastBlockSize = 0;
    int lastSampleRate = 0;
    juce::int64 samples = 0;
#if PROBE_CAPTURE
    OneTake capture{ 15 * 192000 };
    int audioEventCount = 0;
    std::atomic<bool> audioTraceExhausted{ false };
    bool reportedAudioExhaustion = false;
    std::atomic<Command> requestedCommand{ Command::none };
    std::atomic<OneTake::State> displayedState{ OneTake::State::idle };
    std::atomic<bool> takePending{ false };
    juce::String lastSave;
    juce::File lastWav;
#endif
};

#if PROBE_CAPTURE
class Probe::CaptureEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit CaptureEditor(Probe& owner) : AudioProcessorEditor(owner), probe(owner)
    {
        addAndMakeVisible(status);
        addAndMakeVisible(result);
        addAndMakeVisible(arm);
        addAndMakeVisible(cancel);
        addAndMakeVisible(reveal);
        arm.onClick = [&] { probe.requestedCommand.store(Command::arm, std::memory_order_release); };
        cancel.onClick = [&] { probe.requestedCommand.store(Command::cancel, std::memory_order_release); };
        reveal.onClick = [&] { probe.lastWav.revealToUser(); };
        setSize(460, 118);
        timerCallback();
        startTimerHz(10);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        status.setBounds(area.removeFromTop(27));
        auto row = area.removeFromTop(32);
        arm.setBounds(row.removeFromLeft(146).reduced(2));
        cancel.setBounds(row.removeFromLeft(146).reduced(2));
        reveal.setBounds(row.removeFromLeft(146).reduced(2));
        result.setBounds(area.removeFromTop(28));
    }

private:
    void timerCallback() override
    {
        using State = OneTake::State;
        const auto state = probe.displayedState.load(std::memory_order_relaxed);
        const char* message = "Ready to arm one take";
        switch (state)
        {
            case State::waitForLow: message = "Armed | Waiting for the gate to go low"; break;
            case State::waitForRise: message = "Armed | Waiting for the next gate rise"; break;
            case State::recording: message = "Recording | Waiting for the gate to fall"; break;
            case State::complete: message = "Take complete"; break;
            case State::cancelled: message = "Take cancelled"; break;
            case State::rejected: message = "Take rejected | Check rate, input or 15 second limit"; break;
            case State::idle: break;
        }
        const bool active = state == State::waitForLow || state == State::waitForRise || state == State::recording;
        status.setText(message, juce::dontSendNotification);
        result.setText(probe.lastSave, juce::dontSendNotification);
        arm.setEnabled(!active && state != State::complete);
        cancel.setEnabled(active);
        reveal.setEnabled(probe.lastWav.existsAsFile());
    }

    Probe& probe;
    juce::Label status;
    juce::Label result;
    juce::TextButton arm{ "Arm one take" };
    juce::TextButton cancel{ "Cancel take" };
    juce::TextButton reveal{ "Show last WAV" };
};
#endif

juce::AudioProcessorEditor* Probe::createEditor()
{
#if PROBE_CAPTURE
    return new CaptureEditor(*this);
#else
    return new juce::GenericAudioProcessorEditor(*this);
#endif
}
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Probe();
}
