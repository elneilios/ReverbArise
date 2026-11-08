#include "LoopPlayback.h"
#include <algorithm>
#include <cmath>

std::unique_ptr<LoopPlayback::Mix> LoopPlayback::prepare(const WetRender& wet,int period,uint64_t budget,uint64_t revision)
{
    const auto end=wet.start+static_cast<int64_t>(wet.frames());
    if (period<=0 || wet.start < -period || end>period || wet.frames()==0)
        return {};
    const auto lead=static_cast<int>(std::max<int64_t>(0,-wet.start));
    if ((static_cast<uint64_t>(lead)+period)*2*sizeof(float)>budget)
        return {};
    auto mix=std::make_unique<Mix>();
    mix->revision=revision;
    mix->period=period;
    mix->lead=lead;
    mix->initial.resize(static_cast<size_t>(lead)*2);
    mix->repeat.resize(static_cast<size_t>(period)*2);
    for (size_t frame=0;frame<wet.frames();++frame)
    {
        const auto position=wet.start+static_cast<int64_t>(frame);
        const auto wrapped=position<0?position+period:position;
        for (size_t channel=0;channel<2;++channel)
        {
            const auto value=wet.samples[2*frame+channel];
            mix->repeat[2*static_cast<size_t>(wrapped)+channel]+=value;
            if (position<0)
                mix->initial[2*static_cast<size_t>(position+lead)+channel]=value;
        }
    }
    constexpr float wetCeiling=0.5011872f;
    auto peak=wet.peak;
    for (auto sample:mix->repeat)
        peak=std::max(peak,std::abs(sample));
    if (peak>wetCeiling)
    {
        const auto gain=wetCeiling/peak;
        for (auto& sample:mix->initial) sample*=gain;
        for (auto& sample:mix->repeat) sample*=gain;
    }
    return mix;
}

void LoopPlayback::configure(int rate,int frames) noexcept
{
    configuration.store((uint64_t{static_cast<uint32_t>(rate)}<<32)|static_cast<uint32_t>(frames));
}

void LoopPlayback::publish(std::unique_ptr<Mix> mix)
{
    const auto* next=mix.get();
    if (mix) pins.push_back(std::move(mix));
    published.store(next);
    collect();
}

void LoopPlayback::collect()
{
    const auto* latest=published.load();
    // Read the block hazard first. The callback promotes it to held before
    // clearing reading, so at least one protects an active former publication.
    const auto* inBlock=reading.load();
    const auto* playing=held.load();
    std::erase_if(pins,[&](const auto& mix)
    { return mix.get()!=latest && mix.get()!=inBlock && mix.get()!=playing; });
}

uint64_t LoopPlayback::pinnedBytes() const noexcept
{
    uint64_t bytes=0;
    for (const auto& mix:pins) bytes+=mix->bytes();
    return bytes;
}

bool LoopPlayback::cue() noexcept
{
    const auto* mix=published.load();
    if (!mix || status()!=Status::standby)
        return false;
    auto empty=uint64_t{0};
    return command.compare_exchange_strong(empty,mix->revision);
}

void LoopPlayback::stop() noexcept { command.store(stopCommand); }

void LoopPlayback::releaseWet() noexcept
{
    active=nullptr;
    held.store(nullptr);
    launch=-1;
    fadeRemaining=0;
}

void LoopPlayback::reset() noexcept
{
    releaseWet();
    anchor=-1;
    position=0;
    matches=0;
    unlockedStatus=Status::waiting;
    command.exchange(0);
    cursorSample.store(INT32_MIN,std::memory_order_relaxed);
    currentStatus.store(Status::waiting);
}

void LoopPlayback::stopWet(int fade) noexcept
{
    if (!active || fadeRemaining)
        return;
    if (position<launch-active->lead)
        releaseWet();
    else
        fadeRemaining=fade;
}

void LoopPlayback::process(float* left,float* right,int count,double rate,const juce::MidiBuffer& midi,
                           bool allowed,bool invalidate) noexcept
{
    const auto config=configuration.load();
    const auto period=static_cast<int>(config&UINT32_MAX);
    const bool compatible=allowed && period>0 && rate==static_cast<double>(config>>32);
    if (invalidate || activeConfiguration!=(compatible?config:0))
    {
        activeConfiguration=compatible?config:0;
        reset();
    }
    if (!compatible || count<=0) return;

    const auto* next=published.load();
    reading.store(next);
    if (next!=published.load()) next=nullptr;
    const auto ui=command.exchange(0);
    const auto fade=std::max(1,static_cast<int>(rate/200));
    auto event=midi.cbegin();
    const auto end=midi.cend();
    for (int offset=0;offset<count;++offset,++position)
    {
        bool marker=false, cue=offset==0 && next && ui==next->revision, stop=offset==0 && ui==stopCommand;
        while (event!=end)
        {
            const auto message=*event;
            if (message.samplePosition>offset) break;
            if (message.samplePosition==offset && message.numBytes==3
                && (message.data[0]&0xf0)==0x90 && message.data[2]!=0)
            {
                marker|=message.data[1]==phraseNote;
                cue|=message.data[1]==cueNote;
                stop|=message.data[1]==stopNote;
            }
            ++event;
        }
        if (anchor>=0 && position==anchor+period && !marker)
        {
            if (matches>=2) unlockedStatus=Status::syncLost;
            else if (unlockedStatus==Status::waiting) unlockedStatus=Status::referenceMismatch;
            stopWet(fade);
            anchor=-1;
            matches=0;
        }
        if (marker)
        {
            if (anchor>=0 && position-anchor==period)
            {
                matches=std::min(matches+1,2);
                if (matches==2) unlockedStatus=Status::waiting;
            }
            else
            {
                if (matches>=2) unlockedStatus=Status::syncLost;
                else if (anchor>=0 && unlockedStatus==Status::waiting) unlockedStatus=Status::referenceMismatch;
                stopWet(fade);
                matches=0;
            }
            anchor=position;
        }
        if (stop)
            stopWet(fade);
        else if (cue && matches>=2 && !active && next && next->period==period && next==published.load())
        {
            active=next;
            held.store(active);
            const auto deadline=position+active->lead-anchor;
            launch=anchor+std::max<int64_t>(1,(deadline+period-1)/period)*period;
        }
        if (active && position>=launch-active->lead)
        {
            const auto local=position-launch;
            const auto& samples=local<0?active->initial:active->repeat;
            const auto index=2*static_cast<size_t>(local<0?local+active->lead:local%period);
            const auto gain=fadeRemaining?static_cast<float>(fadeRemaining)/fade:1.0f;
            left[offset]+=samples[index]*gain;
            right[offset]+=samples[index+1]*gain;
            if (fadeRemaining && --fadeRemaining==0) releaseWet();
        }
    }
    reading.store(nullptr);
    auto result=matches>=2?Status::standby:unlockedStatus;
    if (active)
        result=fadeRemaining?Status::stopping:position<=launch-active->lead?Status::queued:
            position<=launch?Status::leadIn:Status::repeating;
    const auto last=position-1-launch;
    const auto sounding=active && last>=-active->lead;
    cursorSample.store(sounding?static_cast<int>(last<0?last:last%period):INT32_MIN,std::memory_order_relaxed);
    currentStatus.store(result);
}
