#pragma once
#include <UI/WorkspaceNotice.h>

inline void workspaceNoticeCases()
{
    using namespace m3;using namespace m3::ui;using controls::NoticeTone;
    CaptureSessionSnapshot capture;EditorSnapshot editor;workflow::Snapshot flow;
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
    REQUIRE(workspaceNotice(flow,capture,editor)->text==editor.defaultIRError);
    editor.actionError="Edit rejected";
    REQUIRE(workspaceNotice(flow,capture,editor)->text==editor.actionError);
    capture.unavailable=true;capture.status="Missing capture asset";
    REQUIRE(workspaceNotice(flow,capture,editor)->detail==capture.status);
    flow.phase=workflow::Phase::failed;
    REQUIRE(workspaceNotice(flow,capture,editor)->text.startsWith("Capture failed"));
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
