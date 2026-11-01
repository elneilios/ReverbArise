#pragma once
#include <Core/SwellSession.h>
#include <atomic>

namespace m3
{
struct CaptureAdmission { bool armed=false; int offset=0,count=0; uint64_t token=0; };
struct RecaptureSnapshot
{
    bool active=false; uint64_t token=0; int64_t start=0,end=0,covered=0; double rate=0;
    juce::String status="No recapture armed";
};
class ScopedRecapture
{
public:
    bool begin(int64_t start,int64_t end,double rate);
    void cancel();
    CaptureAdmission admit(int64_t start,int frames,double rate) const noexcept;
    bool committed(int64_t start,uint32_t frames,uint64_t token);
    void failed(uint64_t token);
    RecaptureSnapshot snapshot() const;
private:
    mutable juce::CriticalSection lock;
    std::atomic<uint64_t> sequence{0},generation{0};
    std::atomic<int64_t> first{0},last{0};
    std::atomic<double> sampleRate{0};
    std::atomic<bool> active{false};
    std::vector<std::pair<int64_t,int64_t>> coverage;
    juce::String status="No recapture armed";
};
}
