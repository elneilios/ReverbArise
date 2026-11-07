#pragma once
#include "PreviewEngine.h"
#include "SwellAssetStore.h"
#include "SwellShape.h"
#include "GenerationConfiguration.h"
#include "ExportFiles.h"
#include <Playback/ExportVoice.h>
#include <Analysis/OnsetAnalysis.h>
#include <Analysis/AutomaticTargets.h>
#include <Analysis/AutomaticGeneration.h>
#include <Analysis/LearnCandidates.h>
#include <Core/TransportTime.h>
#include <Core/WorkflowState.h>
#include <Playback/TransportPlayback.h>
#include <deque>
#include <map>
#include <set>

namespace m3
{
struct Waveform
{
    int64_t start=0,end=0;
    float peak=0;
    std::vector<CapturePeak> peaks;
};
enum class ExportPhase { idle, preparing, ready, error };
struct PreparedExport
{
    std::shared_ptr<const SongExportPlan> plan;
    SongMixMetrics metrics;
    ExportFile file;
    juce::String acceptedNotice;
    std::shared_ptr<const CaptureStorageLocation> storage;
};
struct ExportSnapshot
{
    ExportPhase phase=ExportPhase::idle;
    juce::String message;
    std::shared_ptr<const PreparedExport> prepared;
    std::shared_ptr<const PreparedExport> previous;
    std::shared_ptr<const SongMixDetail> detail;
    juce::String detailMessage;
    std::optional<int64_t> cursor;
    std::optional<int64_t> previewPosition;
    uint64_t session=0,pass=0;
    bool previewRequested=false;
    std::optional<ExportVoice::AcceptedLevels> previewLevels;
    std::shared_ptr<const PreparedExport> preview;
    uint64_t delivery=0;bool delivering=false,dragReady=false;juce::String deliveryPath,deliveryMessage;
};
struct LivePreparationObservation
{
    uint64_t service=0,attempt=0,revision=0,preparedRevision=0;
    SongTime source{},observed{},latest{};
    std::optional<LiveClockSnapshot> clock;
    int64_t start=0,end=0,loopStart=0,loopEnd=0;
    double servicedAtMs=0,attemptedAtMs=0;
    juce::String result="Not serviced",lastAttemptResult="No attempt";
};
struct EditorSnapshot
{
    workflow::Snapshot workflowState; // Captured under the same model publication lock.
    LearnGenerationView learnGeneration;
    uint64_t initialPerformHandoff=0;
    ExportSnapshot songExport;
    Session session;
    uint64_t sessionEpoch=0; // Coherent UI ownership token; never persisted.
    std::optional<LiveClockSnapshot> gridClock; // Dedicated control authority, not telemetry.
    std::vector<Candidate> candidates;
    std::shared_ptr<const Waveform> raw,shaped,sourceDetail;
    std::map<juce::String,std::shared_ptr<const Waveform>> wetSummaries;
    juce::String message;
    juce::String actionError;
    std::optional<int64_t> auditionSample;
    int64_t auditionDryStart=0,auditionDryEnd=0;
    bool playing=false,auditionReady=false,auditionPreparing=false,analyzing=false;
    float attenuationDb=0;
    float blendAttenuationDb=0,monitorPeak=0;
    bool dryAvailable=false,cachedWetOnly=false;
    juce::String dryStatus;
    juce::String defaultIRError;
    bool defaultIRPending=false;
    juce::String requestedDefaultIRName,defaultIRStatus;
    juce::String retainedIRName,requestedIRName;
    uint64_t renderCount=0;
    uint64_t analysisCount=0;
    size_t pendingIntervals=0;
    std::vector<juce::String> auditionMembers;
    LivePreparationObservation livePreparation;
    std::optional<LiveCallbackObservation> liveObservation,liveLastRejection;
    std::optional<LiveCausalObservation> liveCausal;
    bool liveActive=false,liveMissing=false,liveIncompatible=false,liveReady=false;
    juce::String liveStatus="Awaiting host position";
    float liveAttenuationDb=0;
    float gainPlanPeak=0;double gainPlanPreparationMs=0;
    uint64_t gainPlanBuilds=0,gainPlanCacheHits=0;
    int64_t liveLoopDestination=0;
    uint64_t offlineBlocks=0;
    juce::String offlineStatus="No offline callbacks observed";
};
struct IRChoice
{
    juce::String id,name,description,presetId,content;
    Asset managed;
};
struct RegenerationResult
{
    bool accepted=false;
    size_t queued=0,frozen=0,importing=0;
    uint64_t sessionEpoch=0;
};
class SwellEngine final : private juce::Thread
{
public:
    SwellEngine(juce::File projectsRoot,std::function<PreviewContext()>,std::function<void()> persistentChange={});
    ~SwellEngine() override;
    void useWorkflow(workflow::State&,std::function<uint64_t()> captureEpoch={});
    // Non-realtime only: the worker copies the pinned configuration before settlement.
    std::optional<GenerationConfiguration> passConfiguration(uint64_t pass) const;
    EditorSnapshot snapshot() const;
    Session savedSession() const;
    // Pure control-thread capture of one accepted source/creative pair.
    bool acceptedStorageSnapshot(CaptureSessionSnapshot&,Session&,juce::File&,uint64_t&,juce::String&,bool allowUnavailable=false) const;
    bool reloadStorageAssets(uint64_t expectedSession,juce::String expectedReference);
    void beginSourceReplacement();
    bool setPerformNavigation(PerformNavigationState,uint64_t expectedSession);
    void restore(Session,juce::String expectedCaptureReference={});
    void clear();
    bool addTarget(int64_t arrival);
    bool confirmCandidate(size_t index);
    bool confirmCandidateAt(int64_t sample);
    void removeSelected();
    bool removeTarget(const juce::String&,uint64_t expectedSession);
    bool selectTargets(const std::vector<juce::String>&,const juce::String& primary);
    bool selectTargets(const std::vector<juce::String>&,const juce::String& primary,uint64_t expectedSession);
    bool edit(const juce::String&,const std::function<bool(Swell&,juce::String&)>&);
    bool edit(const juce::String&,const std::function<bool(Swell&,juce::String&)>&,uint64_t expectedSession);
    void loadIR(const juce::File&,bool primaryOnly=false);
    uint64_t assignmentEpoch()const noexcept{return sessionGeneration.load();}
    bool assignIR(const juce::String& id,uint64_t expectedInput,const juce::File& file,bool bundled=false,uint64_t expectedSession=0);
    bool assignBundledIR(const juce::String& id,uint64_t expectedInput,const juce::String& presetId,uint64_t expectedSession=0);
    bool assignManagedIR(const juce::String& id,uint64_t expectedInput,const Asset&,const juce::String& name,uint64_t expectedSession=0);
    std::vector<IRChoice> irChoices() const;
    bool resetSwell(const juce::String& id,bool allSettings);
    RegenerationResult generate(bool all=false);
    void analyze(int64_t start,int64_t end);
    void setPreferences(float auditionDb,float sensitivity,double separation,bool autoRegenerate,uint64_t expectedWorkflowPass=UINT64_MAX);
    void setAutomaticProcessing(bool,uint64_t expectedWorkflowPass=UINT64_MAX);
    void setAutoMarkers(bool,uint64_t expectedWorkflowPass=UINT64_MAX);
    void setLearningAutomatic(bool,uint64_t expectedWorkflowPass=UINT64_MAX);
    void useProvisionalSource(std::function<std::optional<ProvisionalCandidateSource>()>);
    void captureSettled(uint64_t pass,uint64_t captureGeneration,bool useful);
    bool setAutoGenerateOnStop(bool,uint64_t expectedSession=0,uint64_t expectedPass=UINT64_MAX);
    bool generateLearn(uint64_t expectedSession,uint64_t expectedPass,uint64_t candidateSerial);
    bool claimInitialPerform(uint64_t expectedSession,uint64_t handoff);
    bool selectInitialPerform(uint64_t expectedSession,uint64_t handoff,const juce::String& id);
    void establishPerformPosition(uint64_t expectedSession);
    void holdLearnAnalysisForTesting(bool hold)noexcept{learnAnalysisHold=hold;notify();}
    bool learnAnalysisWaitingForTesting()const noexcept{return learnAnalysisWaiting.load();}
    void holdLearnGenerationForTesting(bool hold)noexcept{learnGenerationHold=hold;notify();}
    bool learnGenerationWaitingForTesting()const noexcept{return learnGenerationWaiting.load();}
    void holdLearnRendersForTesting(bool hold)noexcept{learnRenderHold=hold;notify();}
    void holdRenderPublicationForTesting(bool hold)noexcept{renderPublicationHold=hold;notify();}
    bool renderPublicationWaitingForTesting()const noexcept{return renderPublicationWaiting.load();}
    void setIsolateSelected(bool);
    void setIsolateDry(bool);
    void setPostRoll(double seconds);
    void setMonitoring(AuditionMode,float blend);
    bool setAuditionWetOnly(bool,uint64_t expectedSession,uint64_t expectedPass=UINT64_MAX);
    void setLivePlayback(bool,LiveRouting,float db);
    bool setGainPolicy(GainPolicy);
    void setRegenerationSensitivity(float);
    float changeTolerance() const noexcept {return (1-regeneration.load())*.3f;}
    bool setFrozen(const juce::String&,bool);
    void loadDefaultIR(const juce::File&);
    bool assignDefaultIR(const juce::File&,uint64_t expectedSession=0,uint64_t expectedWorkflowPass=UINT64_MAX);
    bool assignDefaultBundledIR(const juce::String& presetId,uint64_t expectedSession=0,uint64_t expectedWorkflowPass=UINT64_MAX);
    bool assignDefaultManagedIR(const Asset&,const juce::String& name,uint64_t expectedSession=0,uint64_t expectedWorkflowPass=UINT64_MAX);
    void holdDefaultImportsForTesting(bool hold) noexcept {defaultImportHold.store(hold);notify();}
    bool defaultImportWaitingForTesting() const noexcept {return defaultImportWaiting.load();}
    void resetSuppression();
    void captureCommitted(int64_t start,int64_t end,uint64_t generation,bool explicitRecapture=false);
    void transportPlaying(bool playing) noexcept {transportActive.store(playing);}
    void captureStarted(int64_t sample) noexcept {currentPassStart=sample;previousTransportPlaying=false;}
    void captureStopped() noexcept;
    void resetTransportClock() noexcept {liveClock.reset();}
    std::optional<LiveClockSnapshot> transportClockSnapshot()const noexcept {return liveClock.snapshot(sessionGeneration.load());}
    void holdLivePublicationForTesting(bool hold) noexcept {livePublicationHold.store(hold);notify();}
    bool livePublicationWaitingForTesting() const noexcept {return livePublicationWaiting.load();}
    void failNextLivePublicationForTesting() noexcept {livePublicationFail.store(true);notify();}
    uint64_t livePublicationFailuresForTesting() const noexcept {return livePublicationFailures.load();}
    uint64_t liveClockRejectionsForTesting() const noexcept {return liveClockRejections.load();}
    bool hasExportableWet(const SongTime&)const;
    bool initializeExport();
    bool setExportMix(float wet,uint64_t expectedSession,uint64_t expectedPass=UINT64_MAX);
    bool setExportLevels(float dry,float wet,uint64_t expectedSession,uint64_t expectedPass=UINT64_MAX);
    bool playExport(uint64_t expectedSession);
    bool seekExport(int64_t sample,uint64_t expectedSession,uint64_t expectedPass,const juce::String& composition);
    void requestExportDetail(int64_t first,int64_t end,size_t bins,uint64_t expectedSession,const juce::String& composition);
    uint64_t deliverExport(uint64_t expectedSession,const ExportFile&,const juce::File& destination={},bool overwrite=false,bool drag=false,uint64_t expectedPass=UINT64_MAX);
    void holdExportForTesting(bool hold) noexcept {exportHold.store(hold);notify();}
    bool exportWaitingForTesting()const noexcept{return exportWaiting.load();}
    bool audition();
    void stop();
    void diagnostic(const juce::String& text) { const juce::ScopedLock guard(lock);message=text; }
    void processAudio(float*,float*,int,const SongTime&) noexcept;
    void processTransportAudio(float*,float*,int,const TransportTime&) noexcept;
    void processOfflineAudio(float*,float*,int,const TransportTime&);
    void endOffline() noexcept {offlineActive.store(false);}
    void recaptureCompleted(int64_t start,int64_t end);
    uint64_t residentBytes() const;
    // Control worker only: fence file jobs, then reserve model state only for
    // the final no-I/O capture actor swap. The audio callback takes neither lock.
    bool withStorageJobsPaused(const std::function<bool()>& operation)
    {const juce::ScopedLock work(workLock);return operation();}
    bool withStorageModelLocked(const std::function<bool()>& operation)
    {const juce::ScopedLock guard(lock);return operation();}
    void acceptStorageLocation(std::shared_ptr<const CaptureStorageLocation> location)
    {const juce::ScopedLock guard(lock);storageLocation.store(std::move(location));invalidateLiveLocked();liveCache.clear();stopLocked();notify();}
    juce::File assetDirectory() const {return instanceRoot(contextProvider().capture.manifest);}
private:
    std::function<void()> persistentChange;
    // Derived render metadata is durable, but is not a newer musical command.
    void persistentChangedLocked(bool musicalChange=true) {if(musicalChange)++modelRevision;if(persistentChange)persistentChange();}
    uint64_t modelRevision=0;
    struct LearnRequest
    {
        std::shared_ptr<const LearnCandidateSnapshot> candidates;
        Session expected;
        CaptureManifest source;
        workflow::Snapshot flow;
        uint64_t revision=0,cancellation=0;
        bool automatic=false;
    };
    struct StopIntent
    {
        CandidateIdentity source;
        uint64_t pass=0,configuration=0,revision=0;
        Asset ir;
    };
    std::function<std::optional<ProvisionalCandidateSource>()> provisionalSource;
    std::shared_ptr<const LearnCandidateSnapshot> learnCandidates;
    std::shared_ptr<const LearnCandidateSnapshot> settledLearnCandidates;
    std::optional<CandidateIdentity> learnAttempt;
    std::optional<LearnRequest> learnRequest;
    std::optional<StopIntent> stopIntent;
    std::set<juce::String> protectedGenerationCalibration;
    uint64_t learnCandidateSerial=0,successfulGeneration=0,initialPerformOffer=0;
    bool performPositionEstablished=false,learnUpdating=true,learnGenerating=false,learnUsable=false,learnFailed=false;
    juce::String learnReason,learnNotice;
    std::atomic<uint64_t> learnAnalysisCancellation{0},learnGenerationCancellation{0};
    std::atomic<uint64_t> learnRenderCancellation{0};
    std::atomic<bool> learnAnalysisHold{false},learnAnalysisWaiting{false},learnGenerationHold{false},learnGenerationWaiting{false},learnRenderHold{false};
    std::atomic<bool> renderPublicationHold{false},renderPublicationWaiting{false};
    CandidateIdentity learnIdentity(const PreviewContext&,bool provisional=false,uint64_t pass=0)const;
    LearnGenerationView learnViewLocked()const;
    void invalidateLearnLocked();
    void cancelLearnGenerationLocked();
    bool requestLearnLocked(const PreviewContext&,uint64_t session,uint64_t pass,uint64_t serial,bool automatic);
    bool learnGenerationStep(const PreviewContext&);
    void performLearnGeneration(LearnRequest);
    bool permitCreative();
    workflow::State* workflowState=nullptr;
    std::array<GenerationConfiguration,2> generationConfigurations;
    uint64_t configurationRevision=0;
    bool publishGenerationLocked(const Session&,uint64_t expectedWorkflowPass=UINT64_MAX,bool restoring=false,std::optional<uint64_t> configurationSession={});
    std::function<uint64_t()> captureEpochProvider;
    struct Runtime
    {
        std::shared_ptr<WetRender> raw,shaped,monitor;
        std::shared_ptr<Waveform> rawWave,shapeWave,sourceWave;
        juce::String revision,preparedRender;
        Asset verifiedRender;
        uint64_t preparedShape=UINT64_MAX;
        bool shapeError=false;
        uint64_t lastUse=0;
        uint64_t preparedMonitor=UINT64_MAX;
        juce::String monitorRevision;
        std::vector<juce::String> members;
        float wetAttenuation=0,blendAttenuation=0;
        bool dryAvailable=false,cachedWetOnly=false;
        juce::String dryStatus;
    };
    bool queueIR(const juce::String&,uint64_t,const juce::File&,const juce::String& presetId,uint64_t expectedSession,const Asset& managed={},const juce::String& label={});
    bool queueDefaultIR(const juce::File&,const juce::String& presetId,const Asset&,const juce::String& label,uint64_t expectedSession,uint64_t expectedWorkflowPass);
    struct RetainedIR { Swell accepted;SongTime timing;juce::String revision,requestedName; };
    bool retainedCompatible(const RetainedIR&,const Swell&,const PreviewContext&,const LiveClockSnapshot* liveContext=nullptr) const;
    bool retainIRLocked(const Swell&,const PreviewContext&,const juce::String& requestedName);
    std::map<juce::String,RetainedIR> retainedIR;
    struct Import { juce::File file; std::vector<std::pair<juce::String,uint64_t>> targets; uint64_t session=0; bool futureDefault=false;juce::String presetId;
        Asset managed;juce::String label;uint64_t defaultRequest=0,workflowPass=0; };
    struct Reset { Swell expected,candidate;uint64_t session=0;SongTime time;juce::String captureScope,sourceRevision; };
    struct Analysis { int64_t start,end; float sensitivity; double separation; uint64_t session,request; };
    void run() override;
    void prepareExport(const PreviewContext&);
    void serviceExportPreview();
    void serviceExportDelivery();
    void serviceExportDetail();
    void inspect(const PreviewContext&);
    void prepareDefault(const PreviewContext&);
    bool automaticJob();
    void collectCompletedPass(const PreviewContext&,bool acceptedCapture=false);
    CompletedPasses applicableCompletedPasses(const CaptureManifest&) const;
    void reconcileRenders(const PreviewContext&);
    void requestManualPreparationLocked(const Swell&,const PreviewContext&);
    void dirtyLocked(int64_t,int64_t,bool acceptedPass=false);
    void acceptedSourceLocked(const CaptureManifest&);
    void importJob(const Import&);
    void resetJob(const Reset&);
    bool editChecked(const juce::String&,const std::function<bool(Swell&,juce::String&)>&,bool allowReset=false,uint64_t expectedSession=0);
    void analysisJob(const Analysis&);
    void renderJob(Swell,const PreviewContext&,uint64_t session,bool restoreOrCheck);
    void shapeJob(Swell,uint64_t session);
    void monitorJob(Swell,uint64_t session,uint64_t settings);
    void prepareTransport(const PreviewContext&);
    void invalidateLiveLocked(const juce::String& id={});
    bool prepareSourceWave(const Swell&,const PreviewContext&,uint64_t extra,std::shared_ptr<Waveform>&,juce::String& error);
    bool cancelled(const juce::String&,uint64_t input,uint64_t session) const;
    void report(const Swell&,uint64_t session,Phase,const juce::String&,float progress=0);
    uint64_t occupiedLocked() const;
    uint64_t available(uint64_t extra=0);
    void stopLocked();
    void setExpected(const SongTime&);
    juce::File instanceRoot(const CaptureManifest&) const;
    bool storageReady(const PreviewContext&); // Worker/offline only; never the real-time callback.
    static std::shared_ptr<Waveform> waveform(const WetRender&);
    std::atomic<std::shared_ptr<const CaptureStorageLocation>> storageLocation;
    std::function<PreviewContext()> contextProvider;
    mutable juce::CriticalSection lock;
    juce::CriticalSection workLock;
    Session model;
    std::map<juce::String,Runtime> runtime;
    std::deque<Import> imports;
    std::deque<Reset> resets;
    std::optional<Analysis> analysis;
    std::vector<Candidate> candidates;
    juce::String message="Capture source, add or confirm a target, then assign an IR";
    juce::String actionError;
    bool initialiseDefaultIfMissing=true;
    juce::String captureReference,captureScope,storageRestoreReference;
    juce::String awaitingCaptureReference;
    std::set<juce::String> forceRefresh;
    uint64_t captureGeneration=0,renderCount=0,useSerial=0;
    uint64_t activeJobBytes=0;
    ExportSnapshot exportState;
    struct DetailRequest{uint64_t session,serial,mix,pass;int64_t first,end;size_t bins;double due;std::shared_ptr<const PreparedExport> accepted;};
    std::optional<DetailRequest> detailWork;std::atomic<uint64_t> detailSerial{0};
    struct ExportDelivery{uint64_t id,session,request,pass;ExportFile file;juce::File directory,destination;bool overwrite,drag;std::shared_ptr<const CaptureStorageLocation> storage;};
    uint64_t exportDeliverySerial=0;std::optional<ExportDelivery> exportDelivery;
    std::unique_ptr<ExportVoice> exportVoice=std::make_unique<ExportVoice>();
    std::unique_ptr<SongMix> exportPreviewReader;
    std::shared_ptr<const PreparedExport> exportPreviewPin;
    uint64_t exportPreviewSession=0,exportPreviewPass=0,exportPreviewRevision=0;
    bool exportPreviewWetAvailable=true;
    std::atomic<uint64_t> exportPreviewBytes{0};
    uint64_t exportPreviewTicket=0;
    int64_t exportPreviewNext=0,exportPreviewStart=0;
    juce::String exportAttempt;
    std::optional<std::pair<juce::String,ExportFile>> exportRetryFile;
    std::atomic<uint64_t> exportRequest{0};
    std::atomic<bool> exportHold{false},exportWaiting{false};
    std::atomic<uint64_t> sessionGeneration{1},analysisRequest{0};
    std::atomic<bool> audioAllowed{false},timingChanged{false};
    std::atomic<double> expectedRate{0},expectedBpm{0};
    std::atomic<int> expectedNumerator{0},expectedDenominator{0};
    AuditionVoice voice;
    std::atomic<bool> livePublicationHold{false},livePublicationWaiting{false},livePublicationFail{false};
    std::atomic<uint64_t> livePublicationFailures{0},liveClockRejections{0};
    LiveTransportClock liveClock;
    TransportPlayback live;
    std::atomic<bool> offlineActive{false};
    std::atomic<float> regeneration{.5f};
    std::unique_ptr<TransportPlayback> offlinePlayer;
    Session offlineSession;
    LiveClockSnapshot offlineClock;
    juce::File offlineRoot;
    std::vector<LiveEntry> offlineEntries;
    std::vector<Swell> offlineMembers;
    std::map<juce::String,Swell> offlineAccepted;
    juce::String offlineIRNotice;
    juce::String offlineWarning;
    int64_t offlineWindowStart=0,offlineWindowEnd=0;
    int64_t offlineLastPosition=INT64_MIN;
    juce::String offlineError;
    float offlineGainAttenuation=1,offlineGainPeak=0;
    std::atomic<uint64_t> offlineBlockCount{0};
    juce::String offlineStatus="No offline callbacks observed";
    std::atomic<bool> liveEnabled{true},hostObserved{false};
    bool songPriority=false;
    std::atomic<uint64_t> loopSequence{0};
    std::atomic<bool> loopHint{false};std::atomic<int64_t> loopHintStart{0},loopHintEnd{0};
    int64_t preparedLoopStart=0,preparedLoopEnd=0;
    std::atomic<uint64_t> passSequence{0},passSession{0};
    std::atomic<int64_t> passStart{0},passEnd{0};
    uint64_t acceptedPassSequence=0,previousTransportSession=0;
    SongTime previousTransportTime;
    bool previousTransportPlaying=false;
    int64_t previousTransportEnd=0,currentPassStart=0;
    CompletedPasses completedPasses;
    bool automaticBoundsDirty=true;
    bool performWindowsRequested=false;
    bool recoverPassBounds=false;
    std::map<juce::String,size_t> liveSlots;
    std::array<uint64_t,maximumSwells> liveGenerations{};
    struct LiveCache {juce::String key;float peak=0;std::weak_ptr<const WetRender> main,loop;};
    std::map<juce::String,LiveCache> liveCache;
    juce::String gainPlanKey;
    float gainPlanAttenuation=1,gainPlanPeak=0;
    double gainPlanPreparationMs=0;
    uint64_t gainPlanBuilds=0,gainPlanCacheHits=0;
    LivePreparationObservation livePreparation;
    uint64_t liveRevision=1,preparedLiveRevision=0;
    int64_t liveWindowStart=0,liveWindowEnd=0;
    juce::String liveStatus="Awaiting host position";
    float liveAttenuationDb=0;
    bool analyzing=false;
    std::vector<std::pair<int64_t,int64_t>> dirty,passDirty;
    std::optional<CaptureManifest> acceptedSource;
    std::set<juce::String> passRenderRequests;
    std::map<std::pair<int64_t,int64_t>,juce::String> analyzedContent;
    std::optional<std::pair<int64_t,int64_t>> trailing;
    juce::String defaultAttemptScope;
    juce::String defaultError;
    juce::String requestedDefaultName,defaultStatus;
    bool defaultPending=false;
    uint64_t defaultRequest=0;
    std::atomic<bool> defaultImportHold{false};
    std::atomic<bool> defaultImportWaiting{false};
    double nextAutomaticEdit=0;
    std::atomic<bool> transportActive{false};
    bool wasTransportActive=false,analysisTurn=true;
    uint64_t monitoringGeneration=0;
    uint64_t automaticAnalysisCount=0;
    std::atomic<uint64_t> markerGeneration{0};
    double nextMarkerAnalysis=0;
    std::atomic<bool> automaticBusy{false};
};
}
