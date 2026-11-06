#pragma once
#include "CaptureFailure.h"
#include <juce_core/juce_core.h>

// Worker-owned strings and accepted identity; never constructed by the callback.
struct CaptureFailureRecord
{
    CaptureFailureEvent event;
    juce::String detail,reference,project,instance;
    uint64_t revision=0,finalDropped=0;
    bool hadCommittedSource=false;
    explicit operator bool() const noexcept {return event.reason!=CaptureFailureReason::none;}
};

inline juce::String captureFailureFeedback(const CaptureFailureRecord& failure)
{
    return juce::String(captureFailureReasonText(failure.event.reason))+". "
        +(failure.hadCommittedSource?"Previous source retained.":"No earlier source was committed.")
        +" Stop, then retry capture.";
}
