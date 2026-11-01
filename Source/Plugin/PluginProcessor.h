#pragma once

#include <atomic>
#include <cstdint>

#include <juce_audio_utils/juce_audio_utils.h>

#include <Capture/AudioCaptureQueue.h>
#include <Capture/CaptureWorker.h>
#include <Capture/SparseSongMap.h>
#include <DSP/PreviewEngine.h>
#include <DSP/SwellEngine.h>

struct HostPositionDiagnostics
{
    bool positionAvailable = false;
    bool samplePositionAvailable = false;
    int64_t samplePosition = 0;
    bool ppqPositionAvailable = false;
    double ppqPosition = 0.0;
    bool tempoAvailable = false;
    double bpm = 0.0;
    bool meterAvailable = false;
    int numerator = 0;
    int denominator = 0;
    bool isPlaying = false;
    bool isRecording = false;
    bool isLooping = false;
    int64_t loopStart=0,loopEnd=0;
    int blockSize = 0;
    double sampleRate = 0.0;
};

class PreverbAudioProcessor final : public juce::AudioProcessor
{
public:
    explicit PreverbAudioProcessor(juce::File projectsRoot = defaultProjectsRoot());
    ~PreverbAudioProcessor() override;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    HostPositionDiagnostics getHostPositionDiagnostics() const noexcept;
    void setMappingEnabled(bool enabled);
    bool isMappingEnabled() const noexcept;
    void requestClearCapture();
    size_t getCapturedRegionCount() const noexcept;
    uint64_t getDroppedCaptureBlockCount() const noexcept;
    uint64_t getTimelineDiscontinuityCount() const noexcept;
    void setTestToneEnabled(bool enabled) noexcept;
    bool isTestToneEnabled() const noexcept;
    float getGeneratedTonePeak() const noexcept { return generatedTonePeak.load(std::memory_order_relaxed); }
    void setToneStartBar(int bar) noexcept;
    int getToneStartBar() const noexcept;
    void setToneEndBar(int bar) noexcept;
    int getToneEndBar() const noexcept;
    static juce::File defaultProjectsRoot();
    CaptureSessionSnapshot getCaptureSnapshot() const { return captureWorker.snapshot(); }
    bool flushCaptureForTesting() { return captureWorker.flush(); }
    SongTime getSongTime() const noexcept;
    PreviewSnapshot getPreviewSnapshot() const;
    void loadPreviewIR(const juce::File& file);
    void setSourceBars(int64_t start, int64_t end);
    void generatePreview() { swells->generate(); }
    bool auditionPreview() { return swells->audition(); }
    void setAuditionLevelDb(float db);
    void stopPreview() { swells->stop(); }
    m3::SwellEngine& swellEngine() noexcept { return *swells; }
    const m3::SwellEngine& swellEngine() const noexcept { return *swells; }
    bool armRecapture(int64_t start,int64_t end);
    void cancelRecapture() { captureWorker.cancelRecapture(); }
    m3::RecaptureSnapshot getRecaptureSnapshot() const { return captureWorker.recaptureSnapshot(); }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destinationData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

private:
    void updateHostPositionDiagnostics(int blockSize) noexcept;
    std::atomic<uint64_t> diagnosticsSequence{0};

    std::atomic<bool> positionAvailable{false};
    std::atomic<bool> samplePositionAvailable{false};
    std::atomic<int64_t> samplePosition{0};
    std::atomic<bool> ppqPositionAvailable{false};
    std::atomic<double> ppqPosition{0.0};
    std::atomic<bool> tempoAvailable{false};
    std::atomic<double> bpm{0.0};
    std::atomic<bool> meterAvailable{false};
    std::atomic<int> numerator{0};
    std::atomic<int> denominator{0};
    std::atomic<bool> isPlaying{false};
    std::atomic<bool> isRecording{false};
    std::atomic<bool> isLooping{false};
    std::atomic<int64_t> loopStart{0},loopEnd{0};
    std::atomic<int> blockSize{0};
    std::atomic<double> sampleRate{0.0};
    std::atomic<bool> awaitingRestoredPosition{false};
    bool wasOffline=false;
    PluginCaptureQueue captureQueue;
    std::atomic<bool> mappingEnabled{true};
    std::atomic<uint64_t> droppedCaptureBlockCount{0};
    std::atomic<bool> testToneEnabled{false};
    std::atomic<float> generatedTonePeak{0};
    std::atomic<int> toneStartBar{8};
    std::atomic<int> toneEndBar{10};
    CaptureWorker captureWorker;
    std::unique_ptr<m3::SwellEngine> swells;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreverbAudioProcessor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
