#pragma once

#include <atomic>
#include <cstdint>

#include <juce_audio_utils/juce_audio_utils.h>

#include <Capture/AudioCaptureQueue.h>
#include <Capture/CaptureWorker.h>
#include <Capture/SparseSongMap.h>
#include <DSP/PreviewEngine.h>
#include <DSP/SwellEngine.h>
#if PREVERB_COMPATIBILITY_TEST
#include "CompatibilityTelemetry.h"
#endif
#include <Storage/PortableAssets.h>

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

class PreverbAudioProcessor final : public juce::AudioProcessor, private juce::AsyncUpdater
{
public:
    explicit PreverbAudioProcessor(juce::File projectsRoot = defaultProjectsRoot());
    ~PreverbAudioProcessor() override;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void reset() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    HostPositionDiagnostics getHostPositionDiagnostics() const noexcept;
#if PREVERB_COMPATIBILITY_TEST
    compatibility::Telemetry compatibilityTelemetry;
    juce::String compatibilityReport(bool detailed=true);
    void clearCompatibilityReport(){compatibilityTelemetry.clear();}
#endif
    void setMappingEnabled(bool enabled);
    bool isMappingEnabled() const noexcept;
    void requestClearCapture();
    size_t getCapturedRegionCount() const noexcept;
    uint64_t getDroppedCaptureBlockCount() const noexcept;
    uint64_t getTimelineDiscontinuityCount() const noexcept;
    uint64_t getOmittedCaptureFailureCount() const noexcept {return captureWorker.omittedFailureEvents();}
    void setTestToneEnabled(bool enabled) noexcept;
    bool isTestToneEnabled() const noexcept;
    float getGeneratedTonePeak() const noexcept { return generatedTonePeak.load(std::memory_order_relaxed); }
    void setToneStartBar(int bar) noexcept;
    int getToneStartBar() const noexcept;
    void setToneEndBar(int bar) noexcept;
    int getToneEndBar() const noexcept;
    static juce::File defaultProjectsRoot();
    bool pinPortableAssets(m3::storage::portable::Input&,juce::String& error,bool allowUnavailable=false) const;
    bool beginRelearning(const m3::storage::portable::Input& expected,juce::String& error);
    void cancelRelearning();
    // Worker/control only: validates current source/IRs before queuing explicit recovery rendering.
    uint64_t captureStageCountForTesting()const{return captureWorker.metrics().stages;}
    bool isRelearning()const noexcept{return captureWorker.recoveryArmed();}
    bool reloadRecoveredAssets(const m3::storage::portable::Input& expected,juce::String& error);
    CaptureSessionSnapshot getCaptureSnapshot() const { return captureWorker.snapshot(); }
    CaptureProgress getCaptureProgress() const {return captureWorker.progressSnapshot();}
    bool flushCaptureForTesting() { return captureWorker.flush(); }
    void flushHostStateChangesForTesting() { handleUpdateNowIfNeeded(); }
    SongTime getSongTime() const noexcept;
    workflow::Snapshot getWorkflowSnapshot() const noexcept {return workflowState.snapshot();}
    workflow::Readiness getWorkflowReadiness() const;
    bool requestWorkflowMode(workflow::Mode);
    bool setRecaptureExisting(bool enabled);
    void cancelCapturePass();
    void holdCaptureValidationForTesting(bool hold) noexcept {captureWorker.holdValidationForTesting(hold);}
    void holdCaptureWorkerForTesting(bool hold) noexcept {captureWorker.holdWorkerForTesting(hold);}
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
    void cancelRecapture() { cancelCapturePass(); }
    m3::RecaptureSnapshot getRecaptureSnapshot() const
    {const auto s=workflowState.snapshot();m3::RecaptureSnapshot result;result.active=s.recapture;result.token=s.pass;
        result.status=s.recapture?"Pass-wide recapture armed; DAW Stop commits encountered source":"Recapture off; mapped source protected";return result;}

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
    void handleAsyncUpdate() override {updateHostDisplay(ChangeDetails{}.withNonParameterStateChanged(true));}
    void persistentStateChanged() {if(hostNotificationsEnabled.load() && juce::MessageManager::getInstanceWithoutCreating()!=nullptr)triggerAsyncUpdate();}
    struct RecoveryIntent{m3::storage::portable::Input original;std::vector<std::pair<int64_t,int64_t>> required;};
    std::optional<RecoveryIntent> recoveryIntent;
    mutable juce::CriticalSection storageStateLock;
    std::atomic<bool> stateDecodeRejected{false};
    std::atomic<bool> hostNotificationsEnabled{false};
    void updateHostPositionDiagnostics(int blockSize) noexcept;
#if PREVERB_COMPATIBILITY_TEST
    compatibility::Observation compatibilityObservation;
    bool compatibilityPriorSample=false;
    int64_t compatibilityExpectedSample=0;
#endif
    CaptureHostTiming capturePassDiagnosticBaseline;
    uint64_t passQueuedBlocks=0,passDroppedBlocks=0;
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
    workflow::Mode offlineWorkflowMode=workflow::Mode::capture;
    workflow::State workflowState;
    bool previousHostPlaying=false;
    workflow::Mode previousWorkflowMode=workflow::Mode::capture;
    SongTime capturePassTime;
    uint64_t observedCapturePass=0;
    PluginCaptureQueue captureQueue;
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
