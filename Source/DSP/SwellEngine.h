#pragma once
#include "PreviewEngine.h"
#include "SwellAssetStore.h"
#include "SwellShape.h"
#include <Analysis/OnsetAnalysis.h>
#include <Analysis/AutomaticTargets.h>
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
struct EditorSnapshot
{
    Session session;
    std::vector<Candidate> candidates;
    std::shared_ptr<const Waveform> raw,shaped,sourceDetail;
    std::map<juce::String,std::shared_ptr<const Waveform>> wetSummaries;
    juce::String message;
    juce::String actionError;
    std::optional<int64_t> auditionSample;
    int64_t auditionDryStart=0,auditionDryEnd=0;
    bool playing=false,auditionReady=false,analyzing=false;
    float attenuationDb=0;
    float blendAttenuationDb=0,monitorPeak=0;
    bool dryAvailable=false,cachedWetOnly=false;
    juce::String dryStatus;
    juce::String defaultIRError;
    juce::String retainedIRName,requestedIRName;
    uint64_t renderCount=0;
    uint64_t analysisCount=0;
    size_t pendingIntervals=0;
    std::vector<juce::String> auditionMembers;
    bool liveActive=false,liveMissing=false,liveIncompatible=false,liveReady=false;
    juce::String liveStatus="Awaiting host position";
    float liveAttenuationDb=0;
    float gainPlanPeak=0;double gainPlanPreparationMs=0;
    uint64_t gainPlanBuilds=0,gainPlanCacheHits=0;
    int64_t liveLoopDestination=0;
    uint64_t offlineBlocks=0;
    juce::String offlineStatus="No offline callbacks observed";
};
class SwellEngine final : private juce::Thread
{
public:
    SwellEngine(juce::File projectsRoot,std::function<PreviewContext()>,std::function<void()> persistentChange={});
    ~SwellEngine() override;
    void useWorkflow(workflow::State& state,std::function<uint64_t()> captureEpoch={}) {workflowState=&state;captureEpochProvider=std::move(captureEpoch);}
    EditorSnapshot snapshot() const;
    Session savedSession() const;
    void restore(Session,juce::String expectedCaptureReference={});
    void clear();
    bool addTarget(int64_t arrival);
    bool confirmCandidate(size_t index);
    bool confirmCandidateAt(int64_t sample);
    void removeSelected();
    bool selectTargets(const std::vector<juce::String>&,const juce::String& primary);
    bool edit(const juce::String&,const std::function<bool(Swell&,juce::String&)>&);
    void loadIR(const juce::File&,bool primaryOnly=false);
    uint64_t assignmentEpoch()const noexcept{return sessionGeneration.load();}
    bool assignIR(const juce::String& id,uint64_t expectedInput,const juce::File& file,bool bundled=false,uint64_t expectedSession=0);
    bool assignBundledIR(const juce::String& id,uint64_t expectedInput,const juce::String& presetId,uint64_t expectedSession=0);
    bool resetSwell(const juce::String& id,bool allSettings);
    void generate(bool all=false);
    void analyze(int64_t start,int64_t end);
    void setPreferences(float auditionDb,float sensitivity,double separation,bool autoRegenerate);
    void setAutomaticProcessing(bool);
    void setAutoMarkers(bool);
    void setIsolateSelected(bool);
    void setIsolateDry(bool);
    void setPostRoll(double seconds);
    void setMonitoring(AuditionMode,float blend);
    void setLivePlayback(bool,LiveRouting,float db);
    bool setGainPolicy(GainPolicy);
    void setRegenerationSensitivity(float);
    float changeTolerance() const noexcept {return (1-regeneration.load())*.3f;}
    bool setFrozen(const juce::String&,bool);
    void loadDefaultIR(const juce::File&);
    void resetSuppression();
    void captureCommitted(int64_t start,int64_t end,uint64_t generation,bool explicitRecapture=false);
    void transportPlaying(bool playing) noexcept {transportActive.store(playing);}
    void captureStarted(int64_t sample) noexcept {currentPassStart=sample;previousTransportPlaying=false;}
    void captureStopped() noexcept;
    bool audition();
    void stop();
    void diagnostic(const juce::String& text) { const juce::ScopedLock guard(lock);message=text; }
    void processAudio(float*,float*,int,const SongTime&) noexcept;
    void processTransportAudio(float*,float*,int,const TransportTime&) noexcept;
    void processOfflineAudio(float*,float*,int,const TransportTime&);
    void endOffline() noexcept {offlineActive.store(false);}
    void recaptureCompleted(int64_t start,int64_t end);
    uint64_t residentBytes() const;
    juce::File assetDirectory() const {return instanceRoot(contextProvider().capture.manifest);}
private:
    std::function<void()> persistentChange;
    void persistentChangedLocked() {if(persistentChange)persistentChange();}
    bool permitCreative();
    workflow::State* workflowState=nullptr;
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
    bool queueIR(const juce::String&,uint64_t,const juce::File&,const juce::String& presetId,uint64_t expectedSession);
    struct RetainedIR { Swell accepted;SongTime timing;juce::String revision,requestedName; };
    bool retainedCompatible(const RetainedIR&,const Swell&,const PreviewContext&) const;
    bool retainIRLocked(const Swell&,const PreviewContext&,const juce::String& requestedName);
    std::map<juce::String,RetainedIR> retainedIR;
    struct Import { juce::File file; std::vector<std::pair<juce::String,uint64_t>> targets; uint64_t session=0; bool futureDefault=false;juce::String presetId; };
    struct Reset { Swell expected,candidate;uint64_t session=0;SongTime time;juce::String captureScope,sourceRevision; };
    struct Analysis { int64_t start,end; float sensitivity; double separation; uint64_t session,request; };
    void run() override;
    void inspect(const PreviewContext&);
    void prepareDefault(const PreviewContext&);
    bool automaticJob();
    void collectCompletedPass(const PreviewContext&);
    CompletedPasses applicableCompletedPasses(const CaptureManifest&) const;
    void dispatchAutomatic(const PreviewContext&);
    void dirtyLocked(int64_t,int64_t);
    void importJob(const Import&);
    void resetJob(const Reset&);
    bool editChecked(const juce::String&,const std::function<bool(Swell&,juce::String&)>&,bool allowReset=false);
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
    static std::shared_ptr<Waveform> waveform(const WetRender&);
    juce::File root;
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
    juce::String captureReference,captureScope;
    juce::String awaitingCaptureReference;
    std::set<juce::String> forceRefresh;
    uint64_t captureGeneration=0,renderCount=0,useSerial=0;
    uint64_t activeJobBytes=0;
    std::atomic<uint64_t> sessionGeneration{1},analysisRequest{0};
    std::atomic<bool> audioAllowed{false},timingChanged{false};
    std::atomic<double> expectedRate{0},expectedBpm{0};
    std::atomic<int> expectedNumerator{0},expectedDenominator{0};
    AuditionVoice voice;
    TransportPlayback live;
    std::atomic<bool> offlineActive{false};
    std::atomic<float> regeneration{.5f};
    std::unique_ptr<TransportPlayback> offlinePlayer;
    Session offlineSession;
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
    bool recoverPassBounds=false;
    std::map<juce::String,size_t> liveSlots;
    std::array<uint64_t,maximumSwells> liveGenerations{};
    struct LiveCache {juce::String key;float peak=0;std::weak_ptr<const WetRender> main,loop;};
    std::map<juce::String,LiveCache> liveCache;
    juce::String gainPlanKey;
    float gainPlanAttenuation=1,gainPlanPeak=0;
    double gainPlanPreparationMs=0;
    uint64_t gainPlanBuilds=0,gainPlanCacheHits=0;
    uint64_t liveRevision=1,preparedLiveRevision=0;
    int64_t liveWindowStart=0,liveWindowEnd=0;
    juce::String liveStatus="Awaiting host position";
    float liveAttenuationDb=0;
    bool analyzing=false;
    std::vector<std::pair<int64_t,int64_t>> dirty;
    std::map<std::pair<int64_t,int64_t>,juce::String> analyzedContent;
    std::optional<std::pair<int64_t,int64_t>> trailing;
    juce::String defaultAttemptScope;
    juce::String defaultError;
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
