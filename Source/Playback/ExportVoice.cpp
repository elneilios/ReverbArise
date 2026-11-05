#include "ExportVoice.h"
#include <Core/MonitoringSafety.h>
#include <algorithm>
#include <cmath>
#include <bit>

namespace m3
{
namespace {
uint64_t packLevels(float dry,float wet)noexcept{return uint64_t{std::bit_cast<uint32_t>(dry)}|(uint64_t{std::bit_cast<uint32_t>(wet)}<<32);}
}
void ExportVoice::setLevels(uint64_t ticket,float dry,float wet,int64_t end)noexcept
{
    if(!requested(ticket) || !std::isfinite(dry) || !std::isfinite(wet) || dry<0 || dry>1 || wet<0 || wet>1)return;
    // Writers are serialized by the engine's control lock. The audio reader
    // retries a bounded number of times and never waits for the writer.
    targetSequence.fetch_add(1,std::memory_order_acq_rel);targetTicket=ticket;
    targetPair=packLevels(dry,wet);targetEnd=end;targetSequence.fetch_add(1,std::memory_order_release);
}
ExportVoice::AcceptedLevels ExportVoice::acceptedLevels()const noexcept
{
    for(int n=0;n<8;++n){const auto seq=positionSequence.load(std::memory_order_acquire);if(seq&1)continue;
        const auto pair=acceptedPair.load();const auto revision=acceptedRevision.load();
        if(seq==positionSequence.load(std::memory_order_acquire))return {std::bit_cast<float>(static_cast<uint32_t>(pair)),std::bit_cast<float>(static_cast<uint32_t>(pair>>32)),revision};
    }return {};
}
uint64_t ExportVoice::requestStart()noexcept
{
    auto old=intent.load();uint64_t next;
    do{next=(old+2)|1;}while(!intent.compare_exchange_weak(old,next));underrun=false;invalidSamples=false;return next;
}
void ExportVoice::stop(bool now)noexcept
{
    auto old=intent.load();uint64_t next;
    do{next=(old+2)&~uint64_t{1};}while(!intent.compare_exchange_weak(old,next));if(now)immediate.store(next);
}
bool ExportVoice::requested(uint64_t ticket)const noexcept
{return (ticket&1) && intent.load(std::memory_order_acquire)==ticket && finished.load(std::memory_order_acquire)!=ticket;}
bool ExportVoice::hasRoom(uint64_t ticket)const noexcept
{return producerTicket!=ticket || std::any_of(pages.begin(),pages.end(),[](const auto& p){return p.state.load(std::memory_order_acquire)==free;});}
bool ExportVoice::push(uint64_t ticket,int64_t at,int64_t start,int64_t end,double rate,std::span<const float> samples)
{return pushPage(ticket,at,start,end,rate,samples,{});}
bool ExportVoice::pushLayers(uint64_t ticket,int64_t at,int64_t start,int64_t end,double rate,std::span<const float> dry,std::span<const float> wet)
{if(dry.size()!=wet.size() || wet.empty())return false;return pushPage(ticket,at,start,end,rate,dry,wet);}
bool ExportVoice::pushPage(uint64_t ticket,int64_t at,int64_t start,int64_t end,double rate,std::span<const float> samples,std::span<const float> wet)
{
    if(!requested(ticket) || !std::isfinite(rate) || rate<=0 || at<start || at>=end || samples.empty() || samples.size()%2
        || samples.size()/2>pageFrames || (static_cast<uint64_t>(end)-static_cast<uint64_t>(at))<samples.size()/2
        || (static_cast<uint64_t>(end)-static_cast<uint64_t>(start))>INT64_MAX)return false;
    if(producerTicket!=ticket){for(auto& p:pages){
        auto expected=unsigned{ready};
        if(p.state.compare_exchange_strong(expected,writing,std::memory_order_acq_rel)){
            if(p.ticket!=ticket)p.state.store(free,std::memory_order_release);else p.state.store(ready,std::memory_order_release);
        }
    }producerTicket=ticket;}
    for(auto& p:pages){
        auto expected=unsigned{free};if(!p.state.compare_exchange_strong(expected,writing,std::memory_order_acq_rel))continue;
        p.ticket=ticket;p.at=at;p.start=start;p.end=end;p.rate=rate;p.frames=samples.size()/2;
        p.layered=!wet.empty();std::copy(samples.begin(),samples.end(),p.samples.begin());
        if(p.layered)std::copy(wet.begin(),wet.end(),p.wetSamples.begin());p.state.store(ready,std::memory_order_release);return true;
    }return false;
}
int ExportVoice::take(uint64_t ticket,std::optional<int64_t> at)noexcept
{
    int chosen=-1;
    for(size_t i=0;i<pages.size();++i){auto& p=pages[i];auto expected=unsigned{ready};
        if(!p.state.compare_exchange_strong(expected,reading,std::memory_order_acq_rel))continue;
        if(p.ticket!=ticket){p.state.store(free,std::memory_order_release);continue;}
        if((at && p.at!=*at) || (!at && chosen>=0 && pages[static_cast<size_t>(chosen)].at<p.at)){
            p.state.store(ready,std::memory_order_release);continue;
        }
        if(chosen>=0)pages[static_cast<size_t>(chosen)].state.store(ready,std::memory_order_release);
        chosen=static_cast<int>(i);if(at)break;
    }return chosen;
}
void ExportVoice::release()noexcept
{if(active>=0)pages[static_cast<size_t>(active)].state.store(free,std::memory_order_release);active=-1;cursor=0;}
std::optional<int64_t> ExportVoice::position()const noexcept
{
    for(int n=0;n<8;++n){const auto sequence=positionSequence.load(std::memory_order_acquire);if(sequence&1)continue;
        const auto valid=positionActive.load() && positionIntent.load()==intent.load();const auto at=positionSample.load();
        if(sequence==positionSequence.load(std::memory_order_acquire))return valid?std::optional<int64_t>{at}:std::nullopt;
    }return {};
}
void ExportVoice::process(float* left,float* right,int frames,double rate,bool allowed)noexcept
{
    const auto ticket=intent.load(std::memory_order_acquire);
    if(!allowed && requested(ticket))finished.store(ticket);
    if(active>=0 && (!allowed || rate!=pages[static_cast<size_t>(active)].rate || !std::isfinite(rate))){finished.store(playingTicket);release();}
    if(active>=0 && playingTicket!=ticket && !stopRemaining){
        if(immediate.load()==ticket)release();else{stopLength=stopRemaining=std::max(1,static_cast<int>(rate*.005));}
    }
    if(active<0 && allowed && requested(ticket) && armed.load(std::memory_order_acquire)==ticket){
        active=take(ticket,{});cursor=0;stopRemaining=0;playingTicket=ticket;
        appliedRevision=0;gainRemaining=0;
        if(active<0){underrun=true;finished.store(ticket);}
        else if(pages[static_cast<size_t>(active)].rate!=rate){finished.store(ticket);release();}
    }
    if(active>=0 && pages[static_cast<size_t>(active)].layered){
        for(int attempt=0;attempt<8;++attempt){const auto revision=targetSequence.load(std::memory_order_acquire);if(revision&1)continue;
            const auto owner=targetTicket.load(),pair=targetPair.load();const auto end=targetEnd.load();
            if(revision!=targetSequence.load(std::memory_order_acquire))continue;
            if(owner==playingTicket && revision!=appliedRevision){
                const auto dry=std::bit_cast<float>(static_cast<uint32_t>(pair)),wet=std::bit_cast<float>(static_cast<uint32_t>(pair>>32));
                const bool changed=dry!=dryTarget || wet!=wetTarget;
                dryTarget=dry;wetTarget=wet;playingEnd=end;
                if(!appliedRevision){dryLevel=dryTarget;wetLevel=wetTarget;}else if(changed)gainRemaining=std::max(1,static_cast<int>(rate*.005));
                appliedRevision=revision;
            }break;
        }
        // A contended initial publication defers admission, never interpreting
        // an unaccepted default end/pair as a completed or audible transport.
        if(!appliedRevision)return;
    }
    for(int n=0;n<frames && active>=0;++n){const auto& p=pages[static_cast<size_t>(active)];
        const auto at=p.at+static_cast<int64_t>(cursor);
        const auto end=p.layered?std::clamp(playingEnd,p.start,p.end):p.end;
        if(at>=end){finished.store(playingTicket);release();break;}
        if(gainRemaining){dryLevel+=(dryTarget-dryLevel)/gainRemaining;wetLevel+=(wetTarget-wetLevel)/gainRemaining;--gainRemaining;}
        const auto gain=monitorEdgeGain(static_cast<size_t>(at-p.start),static_cast<size_t>(end-p.start),rate,.005)
            *(stopRemaining?static_cast<float>(stopRemaining-1)/std::max(1,stopLength-1):1.f);
        const auto l=(p.layered?p.samples[cursor*2]*dryLevel+p.wetSamples[cursor*2]*wetLevel:p.samples[cursor*2])*gain;
        const auto r=(p.layered?p.samples[cursor*2+1]*dryLevel+p.wetSamples[cursor*2+1]*wetLevel:p.samples[cursor*2+1])*gain;
        if(!std::isfinite(l) || !std::isfinite(r)){if(intent.load()==playingTicket)invalidSamples=true;finished.store(playingTicket);release();break;}
        left[n]+=l;right[n]+=r;++cursor;
        if((stopRemaining && --stopRemaining==0) || at+1==end){finished.store(playingTicket);release();break;}
        if(cursor==p.frames){release();active=take(playingTicket,at+1);
            if(active<0){if(intent.load()==playingTicket)underrun=true;finished.store(playingTicket);break;}}
    }
    positionSequence.fetch_add(1,std::memory_order_acq_rel);positionActive=active>=0;
    acceptedPair=packLevels(dryTarget,wetTarget);acceptedRevision=appliedRevision;
    positionIntent=playingTicket;
    if(active>=0)positionSample=pages[static_cast<size_t>(active)].at+static_cast<int64_t>(cursor);
    positionSequence.fetch_add(1,std::memory_order_release);
}
}
