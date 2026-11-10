#pragma once

#include <Plugin/PluginProcessor.h>
#include <array>

// Local host probe. Only the worker touches the filesystem.
class FlStudioHostTrace final : private juce::Thread
{
public:
    static std::unique_ptr<FlStudioHostTrace> create();
    ~FlStudioHostTrace() override;

    void observe(const HostPositionDiagnostics&, const juce::AudioBuffer<float>&,
                 const juce::MidiBuffer&, bool offline) noexcept;

    std::atomic<unsigned> prepared{0}, reset{0}, released{0};

private:
    struct Row
    {
        HostPositionDiagnostics timing;
        int64_t block = 0, callbackTicks = 0;
        unsigned prepares = 0, resets = 0, releases = 0;
        bool offline = false;
        int firstDry = -1, midiOffset = -1, midiSize = 0;
        int status = 0, data1 = 0, data2 = 0;
    };

    explicit FlStudioHostTrace(std::unique_ptr<juce::FileOutputStream>);
    void publish(const Row&) noexcept;
    void run() override;
    void drain();

    // FL Studio can render the full disposable Playlist faster than the CSV
    // worker drains one batch. Leave room for that entire burst.
    static constexpr int capacity = 32768;
    static constexpr int64_t maximumRows = 200000;
    juce::AbstractFifo fifo{capacity};
    std::array<Row, capacity> rows{};
    std::unique_ptr<juce::FileOutputStream> output;
    int64_t block = 0, published = 0;
    std::atomic<unsigned> dropped{0}, midiOverflow{0};
};
