#include "ExportVoice.h"
#include <Core/MonitoringSafety.h>
#include <algorithm>
#include <cmath>

namespace m3
{
uint64_t ExportVoice::requestStart()noexcept
{
    auto old=intent.load();uint64_t next;
    do{next=(old+2)|1;}while(!intent.compare_exchange_weak(old,next));underrun=false;return next;
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
        std::copy(samples.begin(),samples.end(),p.samples.begin());p.state.store(ready,std::memory_order_release);return true;
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
        if(active<0){underrun=true;finished.store(ticket);}
        else if(pages[static_cast<size_t>(active)].rate!=rate){finished.store(ticket);release();}
    }
    for(int n=0;n<frames && active>=0;++n){const auto& p=pages[static_cast<size_t>(active)];
        const auto at=p.at+static_cast<int64_t>(cursor);
        const auto gain=monitorEdgeGain(static_cast<size_t>(at-p.start),static_cast<size_t>(p.end-p.start),rate,.005)
            *(stopRemaining?static_cast<float>(stopRemaining-1)/std::max(1,stopLength-1):1.f);
        left[n]+=static_cast<float>(p.samples[cursor*2]*gain);right[n]+=static_cast<float>(p.samples[cursor*2+1]*gain);++cursor;
        if((stopRemaining && --stopRemaining==0) || at+1==p.end){finished.store(playingTicket);release();break;}
        if(cursor==p.frames){release();active=take(playingTicket,at+1);
            if(active<0){if(intent.load()==playingTicket)underrun=true;finished.store(playingTicket);break;}}
    }
    positionSequence.fetch_add(1,std::memory_order_acq_rel);positionActive=active>=0;
    positionIntent=playingTicket;
    if(active>=0)positionSample=pages[static_cast<size_t>(active)].at+static_cast<int64_t>(cursor);
    positionSequence.fetch_add(1,std::memory_order_release);
}
}
