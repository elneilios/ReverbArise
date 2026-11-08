#include "LoopPerformance.h"
#include "DefaultImpulse.h"
#include "SourceEdgeConditioning.h"
#include "SwellShape.h"
#include <utility>

LoopPerformance::LoopPerformance(std::function<void()> changed)
    : Thread("ReverbArise Loop preparation"), onEdit(std::move(changed)) {}
LoopPerformance::~LoopPerformance() { stopThread(-1); }

LoopPerformance::Edit LoopPerformance::defaultEdit(int frames,int rate)
{
    return {0,std::min(frames,std::max(1,rate/4)),0,std::min(frames,rate/2),
            rate/50,m3::defaultImpulseId};
}

bool LoopPerformance::valid(const Edit& value,int frames,int rate)
{
    return frames>0 && rate>0 && value.first>=0 && value.end>value.first && value.end<=frames
        && value.placement>=-frames && value.placement<=frames
        && value.lead>=0 && value.lead<=frames
        && value.fadeOut>=0 && value.fadeOut<=rate
        && m3::findBundledImpulse(value.impulse);
}

void LoopPerformance::select(const LoopCapture::Source& next)
{
    const juce::ScopedLock lock(control);
    if (source && *source==next)
        return;
    leaveLocked();
    source=next;
    if (takeId!=next.id || takeHash!=next.hash || !valid(edit,next.frames,next.rate))
    {
        takeId=next.id;
        takeHash=next.hash;
        edit=defaultEdit(next.frames,next.rate);
    }
    if (!isThreadRunning() && !startThread())
    {
        status="Could not start Loop preparation.";
        return;
    }
    playback.configure(next.rate,next.frames);
    queueLocked();
}

void LoopPerformance::leaveLocked()
{
    ++binding;
    ++revision;
    source.reset();
    pending.reset();
    shaped.reset();
    waveform.reset();
    voice.stop(true);
    playback.configure(0,0);
    playback.publish({});
    invalidateAudio();
    status.clear();
    cueStatus.clear();
    notify();
}

void LoopPerformance::leave()
{
    const juce::ScopedLock lock(control);
    leaveLocked();
}

void LoopPerformance::queueLocked()
{
    pending=Job{*source,edit,++revision,binding};
    shaped.reset();
    waveform.reset();
    voice.stop();
    playback.publish({});
    cueStatus.clear();
    status="Preparing Loop swell...";
    notify();
}

bool LoopPerformance::update(Edit next,uint64_t expected)
{
    const juce::ScopedLock lock(control);
    if (!source || binding!=expected || !valid(next,source->frames,source->rate))
        return false;
    if (edit!=next)
    {
        edit=std::move(next);
        queueLocked();
        onEdit();
    }
    return true;
}

bool LoopPerformance::audition(uint64_t expected)
{
    const juce::ScopedLock lock(control);
    if (binding!=expected || !source || !shaped)
        return false;
    const auto sync=playback.status();
    if (sync==LoopPlayback::Status::queued || sync==LoopPlayback::Status::leadIn
        || sync==LoopPlayback::Status::repeating || sync==LoopPlayback::Status::stopping)
        return false;
    constexpr float wetCeiling=0.5011872f;
    const auto gain=shaped->peak>wetCeiling?wetCeiling/shaped->peak:1.0f;
    return voice.start(shaped,gain);
}

void LoopPerformance::stop()
{
    const juce::ScopedLock lock(control);
    voice.stop();
}

bool LoopPerformance::cue(uint64_t expected)
{
    const juce::ScopedLock lock(control);
    if (!source || binding!=expected || !playback.cue()) return false;
    voice.stop(true);
    return true;
}

LoopPerformance::View LoopPerformance::view() const
{
    const juce::ScopedLock lock(control);
    return {binding,source?source->rate:0,source?source->frames:0,edit,status,waveform,
            voice.isPlaying(),voice.position(),playback.ready(),playback.status(),cueStatus,playback.cursor()};
}

void LoopPerformance::process(float* left,float* right,int count,double rate,const juce::MidiBuffer& midi,bool allowed) noexcept
{
    const auto epoch=audioEpoch.load();
    const auto current=epoch==seenAudioEpoch;
    seenAudioEpoch=epoch;
    playback.process(left,right,count,rate,midi,allowed,!current);
    const auto sync=playback.status();
    const bool loopActive=sync==LoopPlayback::Status::queued || sync==LoopPlayback::Status::leadIn
        || sync==LoopPlayback::Status::repeating || sync==LoopPlayback::Status::stopping;
    if (allowed || voice.isActiveOrPending())
        voice.process(left,right,count,rate,allowed && current && !loopActive);
}

void LoopPerformance::save(juce::XmlElement& xml) const
{
    const juce::ScopedLock lock(control);
    if (takeId.isEmpty() || xml.getStringAttribute("learnLoopTakeId")!=takeId
        || xml.getStringAttribute("learnLoopTakeHash")!=takeHash)
        return;
    xml.setAttribute("loopEditVersion",1);
    xml.setAttribute("loopEditFirst",edit.first);
    xml.setAttribute("loopEditEnd",edit.end);
    xml.setAttribute("loopEditPlacement",edit.placement);
    xml.setAttribute("loopEditLead",edit.lead);
    xml.setAttribute("loopEditFadeOut",edit.fadeOut);
    xml.setAttribute("loopEditImpulse",edit.impulse);
}

void LoopPerformance::restore(const juce::XmlElement& xml)
{
    const juce::ScopedLock lock(control);
    leaveLocked();
    edit={};
    takeId.clear();
    takeHash.clear();
    if (xml.getIntAttribute("loopEditVersion")!=1)
        return;
    Edit saved{xml.getIntAttribute("loopEditFirst"),xml.getIntAttribute("loopEditEnd"),
        xml.getIntAttribute("loopEditPlacement"),xml.getIntAttribute("loopEditLead"),
        xml.getIntAttribute("loopEditFadeOut"),xml.getStringAttribute("loopEditImpulse")};
    if (!valid(saved,xml.getIntAttribute("learnLoopTakeFrames"),xml.getIntAttribute("learnLoopTakeRate")))
        return;
    takeId=xml.getStringAttribute("learnLoopTakeId");
    takeHash=xml.getStringAttribute("learnLoopTakeHash");
    edit=std::move(saved);
}

bool LoopPerformance::sameRawInput(const Job& a,const Job& b)
{
    return a.binding==b.binding && a.source==b.source && a.edit.first==b.edit.first && a.edit.end==b.edit.end
        && a.edit.impulse==b.edit.impulse;
}

void LoopPerformance::run()
{
    while (!threadShouldExit())
    {
        std::optional<Job> job;
        {
            const juce::ScopedLock lock(control);
            voice.collect();
            playback.collect();
            job=std::exchange(pending,{});
            if (!source)
            {
                rendered.reset();
                raw={};
            }
        }
        if (job)
            prepare(*job);
        else
            wait(80);
    }
}

void LoopPerformance::prepare(const Job& job)
{
    const auto cancelled=[&]{return threadShouldExit() || revision.load()!=job.revision;};
    juce::String error;
    const auto fail=[&]
    {
        const juce::ScopedLock lock(control);
        if (!cancelled())
            status=error.isNotEmpty()?error:"Could not prepare the Loop swell.";
    };
    uint64_t pinned=0;
    {
        const juce::ScopedLock lock(control);
        pinned=voice.pinnedBytes()+playback.pinnedBytes();
    }
    if (pinned>=budget)
    {
        error="Wait for Audition to release the previous swell.";
        fail();
        return;
    }
    if (!rendered || !sameRawInput(*rendered,job))
    {
        rendered.reset();
        raw={};
        std::vector<float> dry;
        if (!job.source.read(job.edit.first,job.edit.end,budget-pinned,dry,error)
            || !m3::conditionWetSourceEdges(dry,job.source.rate,error,cancelled))
        {
            fail();
            return;
        }
        ImpulseResponse original,impulse;
        const auto dryBytes=dry.capacity()*sizeof(float);
        if (pinned+dryBytes>=budget
            || !m3::decodeBundledImpulse(*m3::findBundledImpulse(job.edit.impulse),budget-pinned-dryBytes,original,error))
        {
            fail();
            return;
        }
        const auto other=pinned+dryBytes+original.samples.capacity()*sizeof(float);
        if (other>=budget || !prepareImpulseResponse(original,job.source.rate,budget-other,impulse,error,cancelled)
            || !renderReverseConvolution(dry,job.edit.first,job.source.rate,impulse,previewMemoryBudget-budget+other,raw,error,cancelled))
        {
            fail();
            return;
        }
        rendered=job;
    }
    m3::Swell swell;
    swell.arrival=swell.sourceStart=job.edit.first;
    swell.sourceEnd=job.edit.end;
    swell.placement=job.edit.placement;
    swell.shape.automaticStart=false;
    swell.shape.startOffset=std::max<int64_t>(raw.start-job.edit.first,-job.edit.lead);
    swell.shape.endOffset=job.edit.end-job.edit.first;
    swell.shape.fadeOut=std::min<int64_t>(job.edit.fadeOut,swell.shape.endOffset-swell.shape.startOffset-1);
    swell.shape.outCurve=m3::Curve::smooth;
    auto wet=std::make_shared<WetRender>();
    const auto occupied=pinned+raw.samples.capacity()*sizeof(float);
    if (occupied>=budget || !m3::prepareShape(swell,raw,budget-occupied,*wet,error,cancelled))
    {
        fail();
        return;
    }
    auto peaks=std::make_shared<std::vector<CapturePeak>>();
    const auto step=(wet->frames()+255)/256;
    for (size_t first=0;first<wet->frames();first+=step)
    {
        const auto end=std::min(first+step,wet->frames());
        const auto [low,high]=std::minmax_element(wet->samples.begin()+2*first,wet->samples.begin()+2*end);
        peaks->push_back({wet->start+static_cast<int64_t>(first),static_cast<uint32_t>(end-first),*low,*high});
    }
    const auto bytes=occupied+wet->samples.capacity()*sizeof(float);
    const auto end=wet->start+static_cast<int64_t>(wet->frames());
    const bool outside=wet->start < -job.source.frames || end>job.source.frames;
    auto mix=LoopPlayback::prepare(*wet,job.source.frames,bytes<budget?budget-bytes:0,job.revision);
    const juce::ScopedLock lock(control);
    if (!cancelled())
    {
        shaped=std::move(wet);
        waveform=std::move(peaks);
        status="Ready to audition";
        cueStatus=mix?juce::String{}:outside?"Move the wet audio within one loop before Cue.":"Prepared loop exceeds the playback memory limit.";
        playback.publish(std::move(mix));
    }
}
