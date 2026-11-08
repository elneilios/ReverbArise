#pragma once

#include <Capture/LoopCapture.h>
#include <Playback/AuditionVoice.h>
#include <Playback/LoopPlayback.h>

// Separate preparation and audition for a verified phrase, independent of the Song map.
class LoopPerformance final : private juce::Thread
{
public:
    struct Edit
    {
        int first=0, end=0, placement=0, lead=0, fadeOut=0;
        juce::String impulse;
        bool operator==(const Edit&) const = default;
    };
    struct View
    {
        uint64_t binding=0;
        int rate=0, frames=0;
        Edit edit;
        juce::String status;
        std::shared_ptr<const std::vector<CapturePeak>> wet;
        bool playing=false;
        std::optional<int64_t> auditionPosition;
        bool cueReady=false;
        LoopPlayback::Status sync=LoopPlayback::Status::waiting;
        juce::String cueStatus;
        std::optional<int> loopPosition;
    };
    static Edit defaultEdit(int frames,int rate);

    explicit LoopPerformance(std::function<void()> onEdit);
    ~LoopPerformance() override;
    void select(const LoopCapture::Source&);
    void leave();
    void save(juce::XmlElement&) const;
    void restore(const juce::XmlElement&);
    bool update(Edit,uint64_t binding);
    bool audition(uint64_t binding);
    void stop();
    bool cue(uint64_t binding);
    void stopLoop() noexcept { playback.stop(); }
    View view() const;
    void invalidateAudio() noexcept { audioEpoch.fetch_add(1); }
    void process(float*,float*,int,double,const juce::MidiBuffer&,bool) noexcept;

private:
    static constexpr uint64_t budget=previewMemoryBudget/2;
    struct Job { LoopCapture::Source source; Edit edit; uint64_t revision=0, binding=0; };
    void run() override;
    void prepare(const Job&);
    void queueLocked();
    void leaveLocked();
    static bool valid(const Edit&,int frames,int rate);
    static bool sameRawInput(const Job&,const Job&);

    const std::function<void()> onEdit;
    mutable juce::CriticalSection control;
    std::optional<LoopCapture::Source> source;
    Edit edit;
    juce::String takeId,takeHash,status,cueStatus;
    uint64_t binding=0;
    std::atomic<uint64_t> revision{0},audioEpoch{0};
    uint64_t seenAudioEpoch=0;
    std::optional<Job> pending;
    std::shared_ptr<const WetRender> shaped;
    std::shared_ptr<const std::vector<CapturePeak>> waveform;
    AuditionVoice voice;
    LoopPlayback playback;
    // Worker-only raw render cache. A change of crop or IR supersedes it.
    std::optional<Job> rendered;
    WetRender raw;
};
