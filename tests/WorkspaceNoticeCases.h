#pragma once
#include <UI/WorkspaceNotice.h>

inline void workspaceNoticeCases()
{
    {
        using namespace m3::ui::controls;NoticeBar bar;
        REQUIRE(bar.display(NoticeMessage{"DAW wet is bypassed in Export","Preview uses accepted layers",NoticeTone::information}));
        REQUIRE(bar.getTitle()=="Information: DAW wet is bypassed in Export" && bar.getDescription()=="Preview uses accepted layers");
        REQUIRE(!bar.display(NoticeMessage{"DAW wet is bypassed in Export","Preview uses accepted layers",NoticeTone::information}));
        REQUIRE(bar.display(NoticeMessage{"Preparation failed","Prior files retained",NoticeTone::error}));
        REQUIRE(bar.getTitle()=="Error: Preparation failed" && bar.getHelpText()=="Prior files retained");
        REQUIRE(bar.display({}) && bar.getTitle().isEmpty() && bar.getDescription().isEmpty() && !bar.isVisible());
    }
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
    const auto perform=workspaceNotice(flow,capture,editor);
    REQUIRE(perform && perform->tone==NoticeTone::warning);
    REQUIRE(perform->text=="Caution: In Perform, swells can remain audible during DAW playback, even if the source is silent or muted.");
    REQUIRE(perform->detail.contains("saved Song positions") && perform->detail.contains("Pattern playback") && perform->detail.contains("mute its mixer track"));
    for(const auto enabled:{false,true})
    {
        editor.session.liveEnabled=enabled;
        REQUIRE(workspaceNotice(flow,capture,editor)==perform); // Independent of playback enable, readiness and empty source.
    }
    {
        const controls::NoticeMessage preparing{"Preparing IR: Monumental","Previous valid IR retained",NoticeTone::information};
        REQUIRE(workspaceNotice(flow,capture,editor,preparing)==preparing);
        REQUIRE(workspaceNotice(flow,capture,editor)==perform);
        auto pending=editor;pending.actionError="Edit rejected";
        REQUIRE(workspaceNotice(flow,capture,pending,preparing)->text==pending.actionError);
        pending=editor;pending.defaultIRError="Cannot decode IR";
        REQUIRE(workspaceNotice(flow,capture,pending)->tone==NoticeTone::error);
        auto unavailable=capture;unavailable.unavailable=true;unavailable.status="Missing source";
        REQUIRE(workspaceNotice(flow,unavailable,editor)->text=="Captured source is unavailable");
        for(const auto status:{"Export incomplete: missing wet","Export warning: retained sound"})
        {pending=editor;pending.liveStatus=status;REQUIRE(workspaceNotice(flow,capture,pending)->text==status);}
        for(const auto phase:{workflow::Phase::recording,workflow::Phase::draining,workflow::Phase::validating,workflow::Phase::failed})
        {auto busy=flow;busy.phase=phase;REQUIRE(workspaceNotice(busy,capture,editor)!=perform);}
        REQUIRE(workspaceNotice(flow,capture,editor)==perform); // No timer, dismissal or mode transition needed to restore.
    }
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
        auto stillRecording=flow;stillRecording.phase=workflow::Phase::recording;
        REQUIRE(workspaceNotice(stillRecording,failed,editor)->tone==NoticeTone::error);
        failed.manifest.ranges.push_back({0,64,CaptureCoverageKind::audio});failed.failure.hadCommittedSource=true;
        const auto retained=workspaceNotice(flow,failed,editor);
        REQUIRE(retained->text.contains("previous source retained") && retained->detail.contains("Previous source retained"));
        workflow::Snapshot retry;retry.mode=workflow::Mode::live;retry.pass=flow.pass+1;
        REQUIRE(workspaceNotice(retry,failed,{})==perform); // Historical evidence cannot replace current Perform guidance.
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
    editor.session.swells[0].phase=Phase::ready;REQUIRE(workspaceNotice(flow,capture,editor)==perform);
    // One notice fits beneath the branding, clear of the mode description and divider.
    for(const auto size:{juce::Point<int>{1100,850},juce::Point<int>{1400,900},juce::Point<int>{1400,1000},juce::Point<int>{1600,1000}})
    {
        const juce::Rectangle<int> bounds(0,0,size.x,size.y);WorkspaceLayout notice(bounds);
        REQUIRE(notice.notice.getHeight()==32 && notice.header.contains(notice.notice));
        REQUIRE(notice.notice.getY()==notice.header.getY()+46);
        for(const auto region:{notice.header,notice.notice,notice.globals,notice.workspace,notice.audition,notice.overview,notice.info})
            REQUIRE(!region.isEmpty() && bounds.contains(region));
        REQUIRE(notice.notice.getBottom()<notice.globals.getY());
        REQUIRE(notice.header.getBottom()==notice.globals.getY());
        REQUIRE(notice.globals.getBottom()==notice.workspace.getY());
        WorkspaceTheme theme;controls::NoticeBar bar;bar.setLookAndFeel(&theme);bar.setHeaderAppearance(true);bar.setBounds(notice.notice);
        bar.addToDesktop(0);
        REQUIRE(bar.display(perform) && bar.isVisible());
        REQUIRE(!bar.display(workspaceNotice(flow,capture,editor))); // Identical refresh does not re-announce.
        const auto accessible=bar.getAccessibilityHandler();
        REQUIRE(accessible && accessible->getTitle()=="Warning: "+perform->text && accessible->getDescription()==perform->detail);
        REQUIRE(bar.getTooltip().contains(perform->text) && bar.getTooltip().contains(perform->detail));
        const auto width=juce::GlyphArrangement::getStringWidth(fontFor(bar,bodySize),perform->text);
        std::cout<<"Perform warning width "<<width<<" / "<<bar.getWidth()-46<<" at "<<size.x<<'x'<<size.y<<'\n';
        REQUIRE(width<=bar.getWidth()-46);
        const auto before=bar.getBounds();
        const controls::NoticeMessage progress{"Preparing IR: Whisper","Current IR retained",NoticeTone::information};
        REQUIRE(bar.display(workspaceNotice(flow,capture,editor,progress)));
        REQUIRE(bar.currentMessage()==progress && bar.getBounds()==before);
        REQUIRE(bar.display(workspaceNotice(flow,capture,editor)) && bar.currentMessage()==perform && bar.getBounds()==before);
        bar.removeFromDesktop();bar.setLookAndFeel(nullptr);
    }
    flow.mode=workflow::Mode::capture;REQUIRE(!workspaceNotice(flow,capture,editor));
    flow.mode=workflow::Mode::finalise;REQUIRE(workspaceNotice(flow,capture,editor)->tone==NoticeTone::information);
    flow.mode=workflow::Mode::live;REQUIRE(workspaceNotice(flow,capture,editor)==perform); // Mode return/editor recomputation.
}
