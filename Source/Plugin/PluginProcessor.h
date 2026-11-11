#pragma once

#include <atomic>
#include <cstdint>
#include <chrono>

#include <juce_audio_utils/juce_audio_utils.h>

#include <Capture/AudioCaptureQueue.h>
#include <Capture/CaptureWorker.h>
#include <Capture/LoopCapturePreference.h>
#include <Capture/LoopTake.h>
#include <Capture/LoopBeatClock.h>
#include <Capture/LoopHostGrid.h>
#include <Capture/SparseSongMap.h>
#include <DSP/PreviewEngine.h>
#include <DSP/SwellEngine.h>
#include <Storage/WorkingFolder.h>

#if PREVERB_FL_STUDIO_TRACE
class FlStudioHostTrace;
#endif

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

    SongTime songTime() const noexcept
    {
        return {positionAvailable && samplePositionAvailable && ppqPositionAvailable && tempoAvailable && meterAvailable,
                samplePosition, ppqPosition, sampleRate, bpm, numerator, denominator};
    }
};

class PreverbAudioProcessor final : public juce::AudioProcessor, private juce::AsyncUpdater
{
public:
    explicit PreverbAudioProcessor(juce::File projectsRoot = defaultProjectsRoot(),
                                   bool flStudioLoopHost = juce::PluginHostType().isFruityLoops());
    ~PreverbAudioProcessor() override;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void reset() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    void processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    HostPositionDiagnostics getHostPositionDiagnostics() const noexcept;
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
    juce::File assetRegistryRoot() const;
    bool useWorkingFolder(const std::shared_ptr<m3::storage::portable::WorkingFolder>&,
                          const m3::storage::portable::Input&,juce::String& error,const m3::storage::portable::Hooks& hooks={});
    bool pinPortableAssets(m3::storage::portable::Input&,juce::String& error,bool allowUnavailable=false) const;
    bool beginRelearning(const m3::storage::portable::Input& expected,juce::String& error);
    void cancelRelearning();
    // Worker/control only: validates current source/IRs before queuing explicit recovery rendering.
    uint64_t captureStageCountForTesting()const{return captureWorker.metrics().stages;}
    bool isRelearning()const noexcept{return captureWorker.recoveryArmed();}
    bool reloadRecoveredAssets(const m3::storage::portable::Input& expected,juce::String& error);
    CaptureSessionSnapshot getCaptureSnapshot() const { return captureWorker.snapshot(); }
    CaptureProgress getCaptureProgress() const {return captureWorker.progressSnapshot();}
    LoopCapturePreference getLoopCapturePreference() const noexcept
    {return LoopCapturePreference::unpack(loopPreference.load(std::memory_order_relaxed));}
    bool setLoopCapturePreference(LoopCapturePreference preference);
    bool armLoopCapture();
    bool requestLoopStop(uint64_t pass) noexcept;
    bool loopRecaptureEnabled() const noexcept { return loopRecapture.load(std::memory_order_acquire); }
    bool setLoopRecapture(bool enabled);
    bool loopFinishing() const noexcept { return loopFinishPending.load(std::memory_order_acquire); }
    bool cancelLoopCapture(uint64_t pass);
    bool loopCaptureArmed() const noexcept { return loopArm.load(std::memory_order_acquire) != 0; }
    bool canArmLoopCapture() const;
    bool usesFlStudioLoopTiming() const noexcept { return flStudioLoopHost; }
    juce::String loopCaptureStatus() const;
    bool flushCaptureForTesting() { return captureWorker.flush(); }
    void flushHostStateChangesForTesting() { handleUpdateNowIfNeeded(); }
    // A serial UI fixture can pause for software painting without simulating a
    // host callback gap. Keep that fixture's next block continuous in wall time.
    void resumeContinuousLoopPlaybackForTesting() noexcept
    { previousLoopPlaybackCallback=std::chrono::steady_clock::now(); }
    SongTime getSongTime() const noexcept;
    SongTime getSourceEditTime(const CaptureSessionSnapshot&) const noexcept;
    workflow::Snapshot getWorkflowSnapshot() const noexcept {return workflowState.snapshot();}
    workflow::Readiness getWorkflowReadiness() const;
    bool requestWorkflowMode(workflow::Mode);
    bool enterLoopStandby();
    void setLoopPlaybackReference(const m3::LoopReferenceSetup&, bool externalCommands);
    m3::LoopPlayback::Status loopPlaybackStatus() const noexcept;
    bool requestLoopCue() noexcept;
    void requestLoopPlaybackStop() noexcept { swells->loopTransport().requestFinish(); }
    bool setRecaptureExisting(bool enabled);
    void cancelCapturePass();
    void holdCaptureValidationForTesting(bool hold) noexcept {captureWorker.holdValidationForTesting(hold);}
    void holdCaptureWorkerForTesting(bool hold) noexcept {captureWorker.holdWorkerForTesting(hold);}
    bool captureWorkerHeldForTesting() const noexcept {return captureWorker.workerHeldForTesting();}
    PreviewSnapshot getPreviewSnapshot() const;
    void loadPreviewIR(const juce::File& file);
    struct SourceBarSubmission
    {
        juce::String rejection;
        SourceBarFields affected=SourceBarFields::none;
    };
    struct SourceBarTarget
    {
        uint64_t epoch;
        juce::String primary;
        bool operator==(const SourceBarTarget&) const = default;
    };
    SourceBarSubmission setSourceBars(int64_t start, int64_t end, std::optional<SourceBarTarget> target = {});
    SourceBarSubmission checkSourceBars(int64_t start, int64_t end, const SourceBarTarget& target) const;
    void generatePreview() { swells->generate(); }
    bool auditionPreview() { return swells->audition(sampleRate.load(std::memory_order_relaxed)); }
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
#if PREVERB_FL_STUDIO_TRACE
    std::unique_ptr<FlStudioHostTrace> flStudioHostTrace;
#endif
    SourceBarSubmission resolveBarRange(int64_t start,int64_t end,int64_t& firstSample,int64_t& endSample) const;
    void handleAsyncUpdate() override {updateHostDisplay(ChangeDetails{}.withNonParameterStateChanged(true));}
    void persistentStateChanged() {if(hostNotificationsEnabled.load() && juce::MessageManager::getInstanceWithoutCreating()!=nullptr)triggerAsyncUpdate();}
    struct RecoveryIntent{m3::storage::portable::Input original;std::vector<std::pair<int64_t,int64_t>> required;};
    std::optional<RecoveryIntent> recoveryIntent;
    juce::File legacyProjectsRoot;
    CaptureSessionSnapshot pendingHostCapture;
    m3::storage::Association pendingHostAssociation;
    mutable juce::CriticalSection storageStateLock;
    std::atomic<bool> stateDecodeRejected{false};
    std::atomic<bool> hostNotificationsEnabled{false};
    std::atomic<uint32_t> loopPreference{LoopCapturePreference{}.packed()};
    enum class LoopRoute { waiting, marker, clock, host };
    const bool flStudioLoopHost;
    loop_capture::HostGrid loopHostGrid;
    loop_capture::HostGrid::Block loopHostBlock;
    m3::LoopClockGrid rackClockGrid;
    uint64_t rackClockResetEpoch = 0;
    int64_t rackClockSample = 0;
    double rackClockRate = 0;
    std::chrono::steady_clock::time_point previousRackClockCallback;
    std::atomic<uint64_t> loopArm{0}, loopStop{0}, loopReset{0}, loopArmResetEpoch{0};
    std::atomic<bool> loopRecapture{false}, loopFinishPending{false};
    std::atomic<LoopRoute> loopRouteHint{LoopRoute::waiting};
    std::atomic<loop_capture::ClockAdmission::State> loopClockState{loop_capture::ClockAdmission::State::notConfigured};
    std::atomic<double> loopTempo{0};
    std::atomic<bool> loopInterrupted{false};
    loop_capture::ClockAdmission loopClock;
    loop_capture::BeatClock captureBeatClock;
    loop_capture::Take loopTake;
    LoopRoute loopTakeRoute=LoopRoute::waiting;
    uint64_t seenLoopArm=0, seenLoopReset=0, loopPass=0;
    int64_t loopRunSample=0;
    int64_t pendingLoopEnd=-1;
    std::chrono::steady_clock::time_point previousLoopCallback;
    std::chrono::steady_clock::time_point previousLoopPlaybackCallback;
    std::optional<int64_t> loopPlaybackHostEnd;
    TransportTime loopPlaybackHostTime;
    m3::LoopReferenceSetup observedLoopReference;
    bool previousLoopSource = false;
    void invalidateLoopClock() noexcept;
    bool storagePlaybackActive() const noexcept;
    bool loopArmAllowed(workflow::Snapshot, bool external) const noexcept;
    bool flStudioLoopTimingUnsupported() const noexcept;
    uint64_t reserveLoopArm(bool external) noexcept;
    bool loopDurationFits(LoopCapturePreference) const noexcept;
    void observeRackClock(const juce::MidiBuffer&, int frames, double rate, loop_capture::MidiObservations&) noexcept;
    void processLoopCapture(const juce::AudioBuffer<float>&, const juce::MidiBuffer&, const HostPositionDiagnostics&,
                            bool, uint64_t, const loop_capture::MidiObservations& rackEvents) noexcept;
    void processLoopPlayback(juce::AudioBuffer<float>&, const juce::MidiBuffer&, const HostPositionDiagnostics&,
                             m3::LoopPlayback::Access, uint64_t, loop_capture::MidiObservations&) noexcept;
    void updateHostPositionDiagnostics(int blockSize) noexcept;
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
