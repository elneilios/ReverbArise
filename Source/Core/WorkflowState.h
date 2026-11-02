#pragma once
#include <atomic>
#include <cstdint>

namespace workflow
{
enum class Mode : uint8_t { capture, live, finalise };
enum class Phase : uint8_t { idle, recording, draining, validating, failed };
enum class Result : uint8_t { none, accepted, empty, failed, cancelled };
struct Readiness { bool usefulSource=false, compatible=false, readySwell=false,frozenWet=false; };
struct Snapshot
{
    Mode mode=Mode::capture;Phase phase=Phase::idle;Result result=Result::none;
    bool recapture=false;uint64_t pass=0;
    bool locked() const noexcept {return phase==Phase::recording || phase==Phase::draining || phase==Phase::validating;}
};
struct Capabilities {bool capture=false,scheduledWet=false,edit=false,preview=false,switchMode=false;};
inline Capabilities capabilities(const Snapshot& s,Readiness r) noexcept
{
    const bool valid=(r.usefulSource || r.frozenWet) && r.compatible;
    return {s.mode==Mode::capture && !s.locked() && r.compatible,
        s.mode==Mode::live && valid,s.mode==Mode::live && valid,
        s.mode!=Mode::capture && valid,!s.locked()};
}
inline const char* modeName(Mode mode) noexcept
{switch(mode){case Mode::capture:return "Capture";case Mode::live:return "Live";case Mode::finalise:return "Finalise";}return "Unknown";}
enum class EntryBlock : uint8_t { none, recording, settling, source, timing, wet };
inline EntryBlock entryBlock(Snapshot s,Mode target,Readiness r) noexcept
{
    if(s.locked())return s.phase==Phase::recording?EntryBlock::recording:EntryBlock::settling;
    if(target!=Mode::capture && !r.usefulSource && !(target==Mode::live && r.frozenWet))return EntryBlock::source;
    if(target!=Mode::capture && !r.compatible)return EntryBlock::timing;
    if(target==Mode::finalise && !r.readySwell)return EntryBlock::wet;
    return EntryBlock::none;
}
inline const char* entryError(Snapshot s,Mode target,Readiness r) noexcept
{
    switch(entryBlock(s,target,r))
    {
        case EntryBlock::recording:return "Capture is recording; stop the DAW first";
        case EntryBlock::settling:return "Source commit/validation is settling";
        case EntryBlock::source:return "Capture useful source audio first (Live also supports verified frozen wet)";
        case EntryBlock::timing:return "Source needs compatible validated host timing";
        case EntryBlock::wet:return "Finalise needs an enabled ready swell";
        case EntryBlock::none:return nullptr;
    }
    return nullptr;
}
// A single lock-free word owns mode, phase, arm and generation. All callback
// transitions are bounded CAS attempts; no UI/worker lock enters processing.
class State
{
public:
    Snapshot snapshot() const noexcept {return decode(word.load(std::memory_order_acquire));}
    bool requestMode(Mode target,Readiness r) noexcept
    {
        if(static_cast<unsigned>(target)>2)return false;
        auto old=word.load(std::memory_order_acquire);const auto s=decode(old);
        if(entryError(s,target,r))return false;
        return word.compare_exchange_strong(old,pack(target,Phase::idle,false,Result::none,s.pass+1),std::memory_order_acq_rel);
    }
    bool arm(bool enabled) noexcept
    {
        auto old=word.load(std::memory_order_acquire);auto s=decode(old);
        if(s.mode!=Mode::capture || s.locked())return false;
        return word.compare_exchange_strong(old,pack(s.mode,s.phase,enabled,s.result,s.pass),std::memory_order_acq_rel);
    }
    uint64_t begin(bool compatible) noexcept
    {
        auto old=word.load(std::memory_order_acquire);const auto s=decode(old);
        if(s.mode!=Mode::capture || s.locked() || !compatible)return 0;
        return word.compare_exchange_strong(old,pack(s.mode,Phase::recording,s.recapture,Result::none,s.pass+1),std::memory_order_acq_rel)?s.pass+1:0;
    }
    bool stop(uint64_t pass) noexcept {return changePhase(pass,Phase::recording,Phase::draining);}
    bool validating(uint64_t pass) noexcept {return changePhase(pass,Phase::draining,Phase::validating);}
    bool complete(uint64_t pass,bool success,bool useful,bool consumedRecapture) noexcept
    {
        auto old=word.load(std::memory_order_acquire);const auto s=decode(old);
        if(s.pass!=pass || (s.phase!=Phase::draining && s.phase!=Phase::validating))return false;
        return word.compare_exchange_strong(old,pack(success && useful?Mode::live:Mode::capture,
            success?Phase::idle:Phase::failed,s.recapture && !(success && (consumedRecapture || useful)),
            !success?Result::failed:useful?Result::accepted:Result::empty,s.pass),std::memory_order_acq_rel);
    }
    void reset(Mode mode=Mode::capture) noexcept
    {auto old=word.load(std::memory_order_acquire);while(!word.compare_exchange_weak(old,pack(mode,Phase::idle,false,Result::none,decode(old).pass+1),std::memory_order_acq_rel)){} }
    void cancel() noexcept
    {auto old=word.load(std::memory_order_acquire);while(!word.compare_exchange_weak(old,pack(Mode::capture,Phase::idle,false,Result::cancelled,decode(old).pass+1),std::memory_order_acq_rel)){} }
private:
    static_assert(std::atomic<uint64_t>::is_always_lock_free);
    static uint64_t pack(Mode m,Phase p,bool arm,Result result,uint64_t pass) noexcept
    {return (pass<<10)|static_cast<uint64_t>(m)|(static_cast<uint64_t>(p)<<2)|(arm?32u:0u)|(static_cast<uint64_t>(result)<<6);}
    static Snapshot decode(uint64_t w) noexcept
    {return {static_cast<Mode>(w&3),static_cast<Phase>((w>>2)&7),static_cast<Result>((w>>6)&15),(w&32)!=0,w>>10};}
    bool changePhase(uint64_t pass,Phase from,Phase to) noexcept
    {auto old=word.load(std::memory_order_acquire);const auto s=decode(old);return s.pass==pass && s.phase==from && word.compare_exchange_strong(old,pack(s.mode,to,s.recapture,s.result,s.pass),std::memory_order_acq_rel);}
    std::atomic<uint64_t> word{0};
};
}
