#pragma once
#include <atomic>
#include <cstdint>
#include <thread>
#include <type_traits>

namespace workflow
{
enum class Mode : uint8_t { capture, live, finalise };
enum class Phase : uint8_t { idle, recording, draining, validating, failed, armed };
enum class Result : uint8_t { none, accepted, empty, failed, cancelled };
struct Readiness { bool usefulSource=false, compatible=false, readySwell=false,frozenWet=false; };
struct Snapshot
{
    Mode mode=Mode::capture;Phase phase=Phase::idle;Result result=Result::none;
    bool recapture=false;uint64_t pass=0;uint8_t configurationSlot=0;bool admissionFailed=false;
    bool generationCommit=false;
    bool locked() const noexcept {return generationCommit || phase==Phase::armed || phase==Phase::recording || phase==Phase::draining || phase==Phase::validating;}
};
struct Capabilities {bool capture=false,scheduledWet=false,edit=false,preview=false,switchMode=false;};
inline Capabilities capabilities(const Snapshot& s,Readiness r) noexcept
{
    const bool valid=(r.usefulSource || r.frozenWet) && r.compatible;
    return {s.mode==Mode::capture && !s.locked() && r.compatible,
        s.mode==Mode::live && valid,s.mode==Mode::live && valid,
        (s.mode==Mode::finalise?r.readySwell && r.compatible:s.mode==Mode::live && valid),!s.locked()};
}
inline const char* modeName(Mode mode) noexcept
{switch(mode){case Mode::capture:return "Capture";case Mode::live:return "Live";case Mode::finalise:return "Finalise";}return "Unknown";}
enum class EntryBlock : uint8_t { none, recording, settling, source, timing, wet };
inline EntryBlock entryBlock(Snapshot s,Mode target,Readiness r) noexcept
{
    if(s.locked())return s.phase==Phase::recording?EntryBlock::recording:EntryBlock::settling;
    if(target==Mode::live && !r.usefulSource && !r.frozenWet)return EntryBlock::source;
    if(target!=Mode::capture && !r.compatible)return EntryBlock::timing;
    if(target==Mode::finalise && !r.readySwell)return EntryBlock::wet;
    return EntryBlock::none;
}
// A single lock-free word owns mode, phase, arm and generation. All callback
// transitions are bounded CAS attempts; no UI/worker lock enters processing.
class State
{
public:
    Snapshot snapshot() const noexcept {auto s=decode(word.load(std::memory_order_acquire));s.admissionFailed=startFailed.load();return s;}
    bool publishConfiguration(uint8_t slot,uint64_t expectedPass) noexcept
    {
        auto old=word.load(std::memory_order_acquire);const auto s=decode(old);
        if(slot>1 || s.pass!=expectedPass || s.locked())return false;
        return word.compare_exchange_strong(old,pack(s.mode,s.phase,s.recapture,s.result,s.pass,slot),std::memory_order_acq_rel);
    }
    bool requestMode(Mode target,Readiness r) noexcept
    {
        if(static_cast<unsigned>(target)>2)return false;
        auto old=word.load(std::memory_order_acquire);const auto s=decode(old);
        if(entryBlock(s,target,r)!=EntryBlock::none)return false;
        return word.compare_exchange_strong(old,pack(target,Phase::idle,false,Result::none,s.pass+1,s.configurationSlot),std::memory_order_acq_rel);
    }
    bool arm(bool enabled) noexcept
    {
        auto old=word.load(std::memory_order_acquire);auto s=decode(old);
        if(s.mode!=Mode::capture || s.locked())return false;
        return word.compare_exchange_strong(old,pack(s.mode,s.phase,enabled,s.result,s.pass,s.configurationSlot),std::memory_order_acq_rel);
    }
    uint64_t begin(bool compatible) noexcept
    {
        auto old=word.load(std::memory_order_acquire);
        for(unsigned attempt=0;attempt<8;++attempt)
        {
            const auto s=decode(old);
            if(s.mode!=Mode::capture || s.locked() || !compatible)return 0;
            if(word.compare_exchange_strong(old,pack(s.mode,Phase::recording,s.recapture,Result::none,s.pass+1,s.configurationSlot),std::memory_order_acq_rel))
            {startFailed.store(false);return s.pass+1;}
        }
        // Bounded admission failure is explicit; never accept a truncated pass.
        startFailed.store(true);return 0;
    }
    uint64_t reserveCapture(uint64_t expectedPass) noexcept
    {
        auto old=word.load(std::memory_order_acquire);
        const auto s=decode(old);
        if(s.pass!=expectedPass || s.mode!=Mode::capture || s.locked())return 0;
        return word.compare_exchange_strong(old,pack(s.mode,Phase::armed,false,Result::none,s.pass+1,s.configurationSlot),
                                            std::memory_order_acq_rel) ? s.pass+1 : 0;
    }
    bool startReserved(uint64_t pass) noexcept {return changePhase(pass,Phase::armed,Phase::recording);}
    bool cancelReservedCapture(uint64_t pass) noexcept {return cancelUnsettledCapture(pass,Phase::armed);}
    bool cancelUnsettledCapture(uint64_t pass) noexcept
    {return cancelUnsettledCapture(pass,Phase::recording);}
    bool rejectReserved(uint64_t pass) noexcept
    {
        auto old=word.load(std::memory_order_acquire);
        const auto s=decode(old);
        return s.pass==pass && s.phase==Phase::armed
            && word.compare_exchange_strong(old,pack(s.mode,Phase::idle,false,Result::none,s.pass,s.configurationSlot),
                                            std::memory_order_acq_rel);
    }
    bool stop(uint64_t pass) noexcept {return changePhase(pass,Phase::recording,Phase::draining);}
    bool validating(uint64_t pass) noexcept {return changePhase(pass,Phase::draining,Phase::validating);}
    bool complete(uint64_t pass,bool success,bool useful,bool consumedRecapture) noexcept
    {
        auto old=word.load(std::memory_order_acquire);const auto s=decode(old);
        if(s.pass!=pass || (s.phase!=Phase::draining && s.phase!=Phase::validating))return false;
        return word.compare_exchange_strong(old,pack(Mode::capture,
            success?Phase::idle:Phase::failed,s.recapture && !(success && (consumedRecapture || useful)),
            !success?Result::failed:useful?Result::accepted:Result::empty,s.pass,s.configurationSlot),std::memory_order_acq_rel);
    }
    // Non-realtime publisher only. Preparation and validation happen before this
    // reservation; the callback merely loses admission or observes the complete
    // publication. The supplied swap must not allocate, throw or acquire locks.
    template<class Publish> bool publishGenerated(const Snapshot& expected,Publish&& publish) noexcept
    {
        static_assert(std::is_nothrow_invocable_v<Publish>);
        if(expected.mode!=Mode::capture || (expected.phase!=Phase::idle && expected.phase!=Phase::failed) || expected.locked())return false;
        auto old=pack(expected.mode,expected.phase,expected.recapture,expected.result,expected.pass,expected.configurationSlot);
        if(!word.compare_exchange_strong(old,old|commitBit,std::memory_order_acq_rel))return false;
        publish();
        word.store(pack(Mode::live,Phase::idle,false,Result::accepted,expected.pass+1,expected.configurationSlot),std::memory_order_release);
        return true;
    }
    // Reset/cancel are control-thread operations. They serialize after the tiny
    // publication swap; no realtime path waits on a generation reservation.
    void reset(Mode mode=Mode::capture) noexcept
    {resetTo(mode,Result::none);}
    void cancel() noexcept
    {resetTo(Mode::capture,Result::cancelled);}
    bool cancelCapture(uint64_t pass) noexcept
    {
        auto old=word.load(std::memory_order_acquire);
        for(;;)
        {
            const auto current=decode(old);
            if(current.pass!=pass || current.mode!=Mode::capture || !current.locked() || current.generationCommit)
                return false;
            const auto cancelled=pack(Mode::capture,Phase::idle,false,Result::cancelled,pass+1,current.configurationSlot);
            if(word.compare_exchange_weak(old,cancelled,std::memory_order_acq_rel))
            {
                startFailed.store(false);
                return true;
            }
        }
    }
private:
    bool cancelUnsettledCapture(uint64_t pass,Phase lastPhase) noexcept
    {
        auto old=word.load(std::memory_order_acquire);
        const auto s=decode(old);
        if(s.pass!=pass || s.mode!=Mode::capture || (s.phase!=Phase::armed && s.phase!=lastPhase))
            return false;
        return word.compare_exchange_strong(old,pack(s.mode,Phase::idle,false,Result::cancelled,s.pass+1,s.configurationSlot),
                                            std::memory_order_acq_rel);
    }
    static constexpr uint64_t commitBit=uint64_t{1}<<11;
    static_assert(std::atomic<uint64_t>::is_always_lock_free);
    static_assert(std::atomic<bool>::is_always_lock_free);
    static uint64_t pack(Mode m,Phase p,bool arm,Result result,uint64_t pass,uint8_t slot) noexcept
    {return (pass<<12)|(static_cast<uint64_t>(slot)<<10)|static_cast<uint64_t>(m)|(static_cast<uint64_t>(p)<<2)|(arm?32u:0u)|(static_cast<uint64_t>(result)<<6);}
    static Snapshot decode(uint64_t w) noexcept
    {return {static_cast<Mode>(w&3),static_cast<Phase>((w>>2)&7),static_cast<Result>((w>>6)&15),(w&32)!=0,w>>12,static_cast<uint8_t>((w>>10)&1),false,(w&commitBit)!=0};}
    void resetTo(Mode mode,Result result) noexcept
    {
        auto old=word.load(std::memory_order_acquire);
        for(;;)
        {
            if(old&commitBit){std::this_thread::yield();old=word.load(std::memory_order_acquire);continue;}
            const auto current=decode(old);
            if(word.compare_exchange_weak(old,pack(mode,Phase::idle,false,result,current.pass+1,current.configurationSlot),std::memory_order_acq_rel))break;
        }
        startFailed.store(false);
    }
    bool changePhase(uint64_t pass,Phase from,Phase to) noexcept
    {auto old=word.load(std::memory_order_acquire);const auto s=decode(old);return s.pass==pass && s.phase==from && word.compare_exchange_strong(old,pack(s.mode,to,s.recapture,s.result,s.pass,s.configurationSlot),std::memory_order_acq_rel);}
    std::atomic<uint64_t> word{0};
    std::atomic<bool> startFailed{false};
};
}
