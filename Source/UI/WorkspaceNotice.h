#pragma once
#include <UI/Controls/NoticeBar.h>
#include <Core/WorkflowState.h>
#include <Capture/CaptureSession.h>
#include <DSP/SwellEngine.h>

namespace m3::ui
{
// Editor-owned presentation debounce. Pending/error flags remain model-owned;
// this never delays, cancels or completes an import. Session/name changes reset
// the delay, so a previous instance/request cannot flash stale progress.
class IRPreparationNotice
{
public:
    static constexpr double delayMilliseconds=600;
    std::optional<controls::NoticeMessage> update(const EditorSnapshot& editor,uint64_t epoch,double now)
    {
        if(owner!=epoch || name!=editor.requestedDefaultIRName)
        {owner=epoch;name=editor.requestedDefaultIRName;since.reset();}
        if(!editor.defaultIRPending){since.reset();return {};}
        if(!since)since=now;
        if(now-*since<delayMilliseconds)return {};
        return controls::NoticeMessage{"Preparing IR: "+name,"Your current IR is retained until preparation succeeds.",controls::NoticeTone::information};
    }
private:
    uint64_t owner=UINT64_MAX;juce::String name;std::optional<double> since;
};

// Presentation only: choose one current notice, without changing model state.
inline std::optional<controls::NoticeMessage> workspaceNotice(workflow::Snapshot flow,
    const CaptureSessionSnapshot& capture,const EditorSnapshot& editor,
    std::optional<controls::NoticeMessage> irPreparation={})
{
    using controls::NoticeMessage;using controls::NoticeTone;
    if(flow.mode==workflow::Mode::capture && flow.admissionFailed)
        return NoticeMessage{"Capture could not start - stop and play again","Settings changed during admission; no partial pass was accepted.",NoticeTone::warning};
    if(flow.phase==workflow::Phase::failed)
        return NoticeMessage{"Capture failed - previous source retained",capture.status,NoticeTone::error};
    if(capture.unavailable)
        return NoticeMessage{"Captured source is unavailable",capture.status,NoticeTone::error};
    if(editor.actionError.isNotEmpty())return NoticeMessage{editor.actionError,{},NoticeTone::error};
    if(editor.defaultIRError.isNotEmpty())return NoticeMessage{"IR could not be loaded - choose another or retry",
        editor.defaultIRError+"\nUse the IR dropdown or folder button to retry. Your previous IR is retained.",NoticeTone::error};
    if(const auto* swell=find(editor.session,editor.session.primary);swell && swell->phase==Phase::error)
        return NoticeMessage{"Unable to prepare " + swell->name,swell->status,NoticeTone::error};
    // These are the existing offline-result prefixes, also used by the editor's
    // playback diagnostics. Do not interpret arbitrary progress/message prose.
    if(editor.liveStatus.startsWith("Export incomplete") || editor.liveStatus.startsWith("Export warning"))
        return NoticeMessage{editor.liveStatus,editor.offlineStatus,NoticeTone::warning};
    switch(flow.phase)
    {
        case workflow::Phase::recording:return NoticeMessage{"Recording - stop your DAW when finished",{},NoticeTone::information};
        case workflow::Phase::draining:return NoticeMessage{"Committing captured source",{},NoticeTone::information};
        case workflow::Phase::validating:return NoticeMessage{"Validating captured source",{},NoticeTone::information};
        default:break;
    }
    if(flow.mode==workflow::Mode::capture && flow.recapture)
        return NoticeMessage{"Recapture is armed - existing audio will be replaced","Turn Recapture off to protect existing source.",NoticeTone::warning};
    if(flow.mode==workflow::Mode::finalise)
        return NoticeMessage{"DAW wet is bypassed - use Audition to preview","Export delivery controls are not available in this workspace yet.",NoticeTone::information};
    return irPreparation;
}
}
