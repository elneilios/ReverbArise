#include "AuditionVoice.h"
#include <Core/MonitoringSafety.h>
#include <algorithm>
#include <cmath>

bool AuditionVoice::start(std::shared_ptr<const WetRender> buffer, float attenuation, bool loop)
{
    collect();
    if (! buffer || buffer->samples.empty() || ! std::isfinite(attenuation) || attenuation < 0 || attenuation > 1) return false;
    const auto w = write.load(std::memory_order_relaxed);
    if (w - read.load(std::memory_order_acquire) >= commands.size()) return false;
    pins.push_back({++serial, buffer});
    commands[w % commands.size()] = {buffer.get(), serial, stopEpoch.load(), attenuation,false,loop};
    write.store(w + 1, std::memory_order_release);
    return true;
}

void AuditionVoice::stop(bool immediately) noexcept
{
    const auto next = stopEpoch.load() + 1;
    if (immediately) immediateEpoch.store(next);
    stopEpoch.store(next, std::memory_order_release);
}
bool AuditionVoice::canReplaceNow() const noexcept
{
    const auto sequence=ownershipSequence.load(std::memory_order_acquire);
    return !(sequence&1) && read.load(std::memory_order_acquire)==write.load(std::memory_order_relaxed)
        && !transitionSerial.load(std::memory_order_acquire)
        && sequence==ownershipSequence.load(std::memory_order_acquire);
}
bool AuditionVoice::replace(std::shared_ptr<const WetRender> buffer, bool waitForTransition)
{
    collect();if(!buffer || buffer->samples.empty())return false;
    if(waitForTransition && !canReplaceNow())return false;
    const auto w=write.load(std::memory_order_relaxed);if(w-read.load(std::memory_order_acquire)>=commands.size())return false;
    pins.push_back({++serial,buffer});commands[w%commands.size()]={buffer.get(),serial,stopEpoch.load(),1,true};
    write.store(w+1,std::memory_order_release);return true;
}

void AuditionVoice::collect()
{
    const auto sequence=ownershipSequence.load(std::memory_order_acquire);if(sequence&1)return;
    const auto ack = acknowledged.load(std::memory_order_acquire);
    const auto activeNow = activeSerial.load(std::memory_order_acquire);
    const auto transitionNow=transitionSerial.load(std::memory_order_acquire);
    if(sequence!=ownershipSequence.load(std::memory_order_acquire))return;
    std::erase_if(pins, [&](const Pin& pin) { return pin.serial <= ack && pin.serial != activeNow && pin.serial!=transitionNow; });
}

uint64_t AuditionVoice::pinnedBytes() const noexcept
{
    uint64_t bytes = 0;
    for (size_t i = 0; i < pins.size(); ++i)
    {
        bool counted = false;
        for (size_t j = 0; j < i; ++j) if (pins[j].buffer == pins[i].buffer) counted = true;
        if (! counted) bytes += pins[i].buffer->samples.capacity() * sizeof(float);
    }
    return bytes;
}

void AuditionVoice::process(float* left, float* right, int frames, double rate, bool allowed) noexcept
{
    ownershipSequence.fetch_add(1,std::memory_order_acq_rel);
    const auto epoch = stopEpoch.load(std::memory_order_acquire);
    if (epoch != seenEpoch)
    {
        seenEpoch = epoch;
        if (immediateEpoch.load() == epoch) { active = previous = nullptr; activeId = previousId = 0;transitionRemaining=0; }
        else if (active)
        {
            rampLength = std::max(1, static_cast<int>(rate * 0.005));
            rampRemaining = rampLength;
        }
    }
    auto r = read.load(std::memory_order_relaxed);
    const auto w = write.load(std::memory_order_acquire);
    while (r != w)
    {
        auto command = commands[r % commands.size()];
        if (command.replace && command.epoch == epoch && rampRemaining == 0 && active)
        {
            // Complete the audible transition before replacing its destination.
            if (transitionRemaining > 0)
                break;
            while (r + 1 != w)
            {
                const auto next = commands[(r + 1) % commands.size()];
                if (!next.replace || next.epoch != epoch)
                    break;
                command = next;
                ++r;
            }
        }
        consumed = command.serial;
        if (command.epoch == epoch)
        {
            if(command.replace)
            {
                if(active && rampRemaining==0 && active->rate==command.buffer->rate
                    && cursor<=static_cast<size_t>(INT64_MAX)
                    && active->start<=INT64_MAX-static_cast<int64_t>(cursor))
                {
                    const auto position=active->start+static_cast<int64_t>(cursor);
                    const auto start=command.buffer->start;
                    const auto span=command.buffer->frames();
                    const auto before=position<start;
                    const auto distance=before?static_cast<uint64_t>(start)-static_cast<uint64_t>(position)
                        :static_cast<uint64_t>(position)-static_cast<uint64_t>(start);
                    const auto outside=before || distance>=span;
                    if(outside && !looping)
                    {active=previous=nullptr;activeId=previousId=0;transitionRemaining=0;}
                    else
                    {
                        previous=active;previousId=activeId;previousCursor=cursor;previousAttenuation=activeAttenuation;
                        previousLooping=looping;
                        const auto wrapped=distance%span;
                        active=command.buffer;activeId=command.serial;cursor=before?(span-wrapped)%span:wrapped;activeAttenuation=1;
                        transitionLength=std::max(1,static_cast<int>(rate*.005));transitionRemaining=transitionLength;
                    }
                }
                ++r;continue;
            }
            previous=nullptr;previousId=0;transitionRemaining=0;
            active = command.buffer; activeId = command.serial;
            cursor = 0; rampRemaining = 0;looping=command.loop;
            activeAttenuation = command.attenuation;
            smoothedLevel = targetLevel = level.load(); levelRemaining = 0;
        }
        ++r;
    }
    // A new Start may be posted while this block runs; never process beyond w.
    read.store(r, std::memory_order_release);
    if (active && (! allowed || rate != active->rate || ! std::isfinite(rate) || rate <= 0))
    { active = previous = nullptr; activeId = previousId = 0;transitionRemaining=0; }
    const auto requestedLevel = level.load();
    if (active && requestedLevel != targetLevel)
    {
        targetLevel = requestedLevel;
        levelRemaining = std::max(1, static_cast<int>(rate * 0.005));
        levelStep = (targetLevel - smoothedLevel) / levelRemaining;
    }
    for (int frame = 0; active && frame < frames; ++frame)
    {
        if (levelRemaining > 0 && --levelRemaining == 0) smoothedLevel = targetLevel;
        else if (levelRemaining > 0) smoothedLevel += levelStep;
        const auto stopGain=rampRemaining>0?static_cast<float>(rampRemaining-1)/std::max(1,rampLength-1):1.0f;
        const auto oldWeight=transitionRemaining>0?static_cast<float>(transitionRemaining)/transitionLength:0.0f;
        for(int channel=0;channel<2;++channel)
        {
            auto sample=active->samples[cursor*2+channel]*activeAttenuation*(1-oldWeight)*m3::monitorEdgeGain(cursor,active->frames(),rate,.005);
            if(previous && previousCursor<previous->frames())sample+=previous->samples[previousCursor*2+channel]*previousAttenuation*oldWeight*m3::monitorEdgeGain(previousCursor,previous->frames(),rate,.005);
            (channel==0?left:right)[frame]+=sample*smoothedLevel*stopGain;
        }
        if(previous && ++previousCursor==previous->frames() && previousLooping)previousCursor=0;
        if(transitionRemaining>0 && --transitionRemaining==0){previous=nullptr;previousId=0;}
        ++cursor;
        const auto rampEnded = rampRemaining > 0 && --rampRemaining == 0;
        if(cursor==active->frames() && looping && rampRemaining==0 && !rampEnded)cursor=0;
        else if (cursor == active->frames() || rampEnded) { active = previous = nullptr; activeId = previousId = 0;transitionRemaining=0; }
    }
    activeSerial.store(activeId, std::memory_order_release);
    transitionSerial.store(previousId,std::memory_order_release);
    playing.store(active != nullptr);
    positionSequence.fetch_add(1,std::memory_order_acq_rel);
    const auto validPosition=active && cursor<=static_cast<size_t>(INT64_MAX) && active->start<=INT64_MAX-static_cast<int64_t>(cursor);
    positionActive=validPosition;if(validPosition)positionSample=active->start+static_cast<int64_t>(cursor);
    positionSequence.fetch_add(1,std::memory_order_release);
    acknowledged.store(consumed, std::memory_order_release);
    ownershipSequence.fetch_add(1,std::memory_order_release);
}
