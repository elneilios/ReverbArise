#include "ScopedRecapture.h"

namespace m3
{
bool ScopedRecapture::begin(int64_t a,int64_t b,double rate)
{
    int64_t length;
    if(!checkedSubtract(b,a,length) || length<=0 || !std::isfinite(rate) || rate<=0) return false;
    const juce::ScopedLock guard(lock); sequence.fetch_add(1);
    first=a;last=b;sampleRate=rate;generation.fetch_add(1);coverage.clear();status="Move playhead to armed range and press Play";
    active=true;sequence.fetch_add(1,std::memory_order_release);return true;
}
void ScopedRecapture::cancel()
{
    const juce::ScopedLock guard(lock);sequence.fetch_add(1);const auto wasActive=active.load();active=false;generation.fetch_add(1);coverage.clear();status=wasActive?"Recapture cancelled":"No recapture armed";
    sequence.fetch_add(1,std::memory_order_release);
}
CaptureAdmission ScopedRecapture::admit(int64_t start,int frames,double rate) const noexcept
{
    CaptureAdmission result;
    for(int attempt=0;attempt<8;++attempt)
    {
        const auto seq=sequence.load(std::memory_order_acquire);
        result.armed=active.load(); if(seq&1) {result.armed=true;continue;}
        const auto a=first.load(),b=last.load();const auto captureRate=sampleRate.load();const auto token=generation.load();
        if(seq!=sequence.load(std::memory_order_acquire)) continue;
        if(!result.armed || frames<=0 || rate!=captureRate) return result;
        int64_t end;if(!checkedAdd(start,frames,end)) return result;
        const auto begin=std::max(start,a),finish=std::min(end,b);
        if(finish>begin) {result.offset=static_cast<int>(begin-start);result.count=static_cast<int>(finish-begin);result.token=token;}
        return result;
    }
    result.armed=true;result.count=0;return result;
}
bool ScopedRecapture::committed(int64_t start,uint32_t frames,uint64_t token)
{
    const juce::ScopedLock guard(lock);
    if(!active.load() || token==0 || token!=generation.load()) return false;
    int64_t end;if(!checkedAdd(start,frames,end)) return false;
    auto a=std::max(first.load(),start),b=std::min(last.load(),end);if(b<=a)return false;
    std::vector<std::pair<int64_t,int64_t>> merged; bool added=false;
    for(const auto& span:coverage)
    {
        if(span.second<a) merged.push_back(span);
        else if(b<span.first) {if(!added){merged.push_back({a,b});added=true;}merged.push_back(span);}
        else {a=std::min(a,span.first);b=std::max(b,span.second);}
    }
    if(!added)merged.push_back({a,b});
    if(merged.size()>4096){status="Fresh coverage too fragmented; cancel and retry";active=false;return false;}
    coverage=std::move(merged);status="Capturing fresh committed coverage";
    if(coverage.size()==1 && coverage[0].first==first.load() && coverage[0].second==last.load())
    {active=false;status="Recapture committed; checking affected swells";return true;}
    return false;
}
void ScopedRecapture::failed(uint64_t token)
{const juce::ScopedLock guard(lock);if(active.load() && token==generation.load())status="Capture commit failed; fresh coverage incomplete";}
RecaptureSnapshot ScopedRecapture::snapshot() const
{
    const juce::ScopedLock guard(lock);RecaptureSnapshot s;s.active=active;s.token=generation;s.start=first;s.end=last;s.rate=sampleRate;s.status=status;
    for(const auto& span:coverage)s.covered+=span.second-span.first;return s;
}
}
