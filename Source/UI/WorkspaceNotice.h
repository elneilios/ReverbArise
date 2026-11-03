#pragma once
#include <UI/Controls/NoticeBar.h>
#include <Core/WorkflowState.h>
#include <Capture/CaptureSession.h>
#include <DSP/SwellEngine.h>

namespace m3::ui
{
// Presentation only: choose one current notice, without changing model state.
inline std::optional<controls::NoticeMessage> workspaceNotice(workflow::Snapshot flow,
    const CaptureSessionSnapshot& capture,const EditorSnapshot& editor)
{
    using controls::NoticeMessage;using controls::NoticeTone;
    if(flow.phase==workflow::Phase::failed)
        return NoticeMessage{"Capture failed - previous source retained",capture.status,NoticeTone::error};
    if(capture.unavailable)
        return NoticeMessage{"Captured source is unavailable",capture.status,NoticeTone::error};
    if(editor.actionError.isNotEmpty())return NoticeMessage{editor.actionError,{},NoticeTone::error};
    if(editor.defaultIRError.isNotEmpty())return NoticeMessage{editor.defaultIRError,{},NoticeTone::error};
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
    return std::nullopt;
}
}
