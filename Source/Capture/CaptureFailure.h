#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <type_traits>

enum class CaptureFailureReason : uint8_t
{
    none, queueFull, blockSize, range, channels, timingUnavailable, timingChanged,
    sampleRate, mappingUnavailable, invalidData, context, assetWrite, assetValidation,
    publication, workerException, rejected
};
inline const char* captureFailureReasonText(CaptureFailureReason reason) noexcept
{
    switch(reason)
    {
        case CaptureFailureReason::none:return "No capture failure";
        case CaptureFailureReason::queueFull:return "Capture queue filled";
        case CaptureFailureReason::blockSize:return "Unsupported capture block size";
        case CaptureFailureReason::range:return "Invalid capture sample range";
        case CaptureFailureReason::channels:return "Stereo input unavailable";
        case CaptureFailureReason::timingUnavailable:return "Host timing unavailable";
        case CaptureFailureReason::timingChanged:return "Host timing changed during capture";
        case CaptureFailureReason::sampleRate:return "Capture sample rate changed";
        case CaptureFailureReason::mappingUnavailable:return "Source mapping unavailable";
        case CaptureFailureReason::invalidData:return "Invalid or non-finite capture data";
        case CaptureFailureReason::context:return "Capture settings could not be pinned";
        case CaptureFailureReason::assetWrite:return "Capture asset could not be written";
        case CaptureFailureReason::assetValidation:return "Capture asset validation failed";
        case CaptureFailureReason::publication:return "Capture manifest could not be published";
        case CaptureFailureReason::workerException:return "Capture worker exception";
        case CaptureFailureReason::rejected:return "Capture pass rejected";
    }
    return "Capture pass rejected";
}

struct CaptureHostTiming
{
    bool position=false,sample=false,ppq=false,tempo=false,meter=false,playing=false,looping=false;
    int64_t samplePosition=0,loopStart=0,loopEnd=0;
    double ppqPosition=0,rate=0,bpm=0;
    int numerator=0,denominator=0;
};
struct CaptureFailureEvent
{
    CaptureFailureReason reason=CaptureFailureReason::none;
    uint64_t pass=0,epoch=0,order=0,queued=0,dropped=0;
    int64_t start=0;
    int frames=0;
    uint32_t depth=0,capacity=0;
    CaptureHostTiming timing,baseline;
};
static_assert(std::is_trivially_copyable_v<CaptureFailureEvent>);
static_assert(std::atomic<uint64_t>::is_always_lock_free);

// Single callback producer, single worker consumer. Slot ownership is transferred
// by release/acquire indices; a consumed slot cannot be reused during its copy.
// One event per pass, never waiting, allocating or overwriting an unread event.
class CaptureFailureMailbox
{
public:
    bool push(const CaptureFailureEvent& event) noexcept
    {
        const auto w=write.load(std::memory_order_relaxed),r=read.load(std::memory_order_acquire);
        if(w-r==events.size())return false;
        events[w%events.size()]=event;write.store(w+1,std::memory_order_release);return true;
    }
    bool pop(CaptureFailureEvent& event) noexcept
    {
        const auto r=read.load(std::memory_order_relaxed),w=write.load(std::memory_order_acquire);
        if(r==w)return false;
        event=events[r%events.size()];read.store(r+1,std::memory_order_release);return true;
    }
private:
    std::array<CaptureFailureEvent,64> events{};
    alignas(64) std::atomic<uint64_t> write{0},read{0};
};
