#pragma once
#include <UI/WorkspaceNotice.h>

inline void workspaceNoticeCases()
{
    using namespace m3;using namespace m3::ui;using controls::NoticeTone;
    CaptureSessionSnapshot capture;EditorSnapshot editor;workflow::Snapshot flow;
    {
        IRPreparationNotice progress;EditorSnapshot pending;pending.defaultIRPending=true;pending.requestedDefaultIRName="Monumental";
        REQUIRE(!progress.update(pending,1,0) && !progress.update(pending,1,599));
        pending.defaultIRPending=false;REQUIRE(!progress.update(pending,1,600)); // A quick completion never emits a progress/success flash.
        pending.defaultIRPending=true;REQUIRE(!progress.update(pending,1,1000));
        const auto slow=progress.update(pending,1,1600);REQUIRE(slow && slow->text=="Preparing IR: Monumental");
        REQUIRE(workspaceNotice(flow,capture,pending,slow)==slow);
        pending.defaultIRError="Cannot decode file";
        const auto error=workspaceNotice(flow,capture,pending,slow);REQUIRE(error->tone==NoticeTone::error && error->detail.contains("Cannot decode file") && error->text.contains("retry"));
        REQUIRE(workspaceNotice(flow,capture,pending,{} )==error); // Errors have no presentation timeout.
        pending.defaultIRError.clear();pending.requestedDefaultIRName="Whisper";REQUIRE(!progress.update(pending,1,1700));
        REQUIRE(!progress.update(pending,2,2300)); // Session supersession restarts the presentation delay.
        pending.defaultIRPending=false;REQUIRE(!progress.update(pending,2,3000));
        pending.defaultIRPending=true;REQUIRE(!progress.update(pending,2,3100));
        flow.phase=workflow::Phase::recording;const auto queued=progress.update(pending,2,3700);
        REQUIRE(workspaceNotice(flow,capture,pending,queued)->text.startsWith("Recording"));flow={};
    }
    REQUIRE(!workspaceNotice(flow,capture,editor));
    flow.mode=workflow::Mode::live;editor.message="Ordinary ready/progress text";
    REQUIRE(!workspaceNotice(flow,capture,editor));
    flow.mode=workflow::Mode::finalise;
    REQUIRE(workspaceNotice(flow,capture,editor)->text.contains("DAW wet is bypassed"));
    flow.mode=workflow::Mode::capture;flow.recapture=true;
    REQUIRE(workspaceNotice(flow,capture,editor)->tone==NoticeTone::warning);
    for(const auto phase:{workflow::Phase::recording,workflow::Phase::draining,workflow::Phase::validating})
    {flow.phase=phase;REQUIRE(workspaceNotice(flow,capture,editor)->tone==NoticeTone::information);}
    editor.liveStatus="Export warning: retained prepared sound";
    REQUIRE(workspaceNotice(flow,capture,editor)->tone==NoticeTone::warning);
    editor.defaultIRError="Default IR failed";
    REQUIRE(workspaceNotice(flow,capture,editor)->detail.contains(editor.defaultIRError) && workspaceNotice(flow,capture,editor)->text.contains("retry"));
    editor.actionError="Edit rejected";
    REQUIRE(workspaceNotice(flow,capture,editor)->text==editor.actionError);
    capture.unavailable=true;capture.status="Missing capture asset";
    REQUIRE(workspaceNotice(flow,capture,editor)->detail==capture.status);
    flow.phase=workflow::Phase::failed;
    REQUIRE(workspaceNotice(flow,capture,editor)->text.startsWith("Capture failed"));
    {
        CaptureSessionSnapshot failed;failed.failure.event.reason=CaptureFailureReason::queueFull;failed.failure.event.pass=flow.pass;
        const auto empty=workspaceNotice(flow,failed,editor);
        REQUIRE(empty->text.contains("no source accepted") && empty->text.contains("retry"));
        REQUIRE(empty->detail.contains("queue") && empty->detail.contains("No earlier source"));
        failed.manifest.ranges.push_back({0,64,CaptureCoverageKind::audio});failed.failure.hadCommittedSource=true;
        const auto retained=workspaceNotice(flow,failed,editor);
        REQUIRE(retained->text.contains("previous source retained") && retained->detail.contains("Previous source retained"));
        workflow::Snapshot retry;retry.mode=workflow::Mode::live;retry.pass=flow.pass+1;
        REQUIRE(!workspaceNotice(retry,failed,{})); // Historical evidence is not a current error.
        for(const auto width:{1100,1400,1600})
        {
            controls::NoticeBar bar;bar.setSize(width-40,controls::NoticeBar::preferredHeight);
            for(const auto message:{empty,retained})
            {
                REQUIRE(bar.display(message));juce::AttributedString line(message->text);line.setFont(fontFor(bar,bodySize));
                juce::TextLayout layout;layout.createLayout(line,static_cast<float>(bar.getWidth()-46));
                REQUIRE(layout.getNumLines()==1 && layout.getHeight()<=bar.getHeight());
            }
        }
    }
    flow={workflow::Mode::live};capture={};editor={};
    Swell swell;swell.id="failed-swell";swell.name="Swell Bar 2.1";swell.phase=Phase::error;swell.status="Cannot decode this IR";
    editor.session.swells.push_back(swell);editor.session.primary=swell.id;
    REQUIRE(workspaceNotice(flow,capture,editor)->detail==swell.status);
    editor.session.swells[0].phase=Phase::ready;REQUIRE(!workspaceNotice(flow,capture,editor));
    // One notice fits beneath the branding, clear of the mode description and divider.
    for(const auto size:{juce::Point<int>{1100,850},juce::Point<int>{1400,1000},juce::Point<int>{1600,1100}})
    {
        const juce::Rectangle<int> bounds(0,0,size.x,size.y);WorkspaceLayout notice(bounds);
        REQUIRE(notice.notice.getHeight()==32 && notice.header.contains(notice.notice));
        REQUIRE(notice.notice.getY()==notice.header.getY()+46);
        for(const auto region:{notice.header,notice.notice,notice.globals,notice.workspace,notice.audition,notice.overview,notice.info})
            REQUIRE(!region.isEmpty() && bounds.contains(region));
        REQUIRE(notice.notice.getBottom()<notice.globals.getY());
        REQUIRE(notice.header.getBottom()==notice.globals.getY());
        REQUIRE(notice.globals.getBottom()==notice.workspace.getY());
    }
}
