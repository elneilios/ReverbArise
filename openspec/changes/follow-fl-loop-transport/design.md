# Design

## Context

The processor already receives FL Studio sample position, PPQ, tempo, meter and declared repeat bounds. It checks exact wraps for continuity before handing timing to Loop playback. The FL Studio host probe established that the effect cannot reliably distinguish Song and Pattern modes. Rack's verified inputs are Clock and Phrase, which supply relative phase. See [FL Studio probe](../../../docs/validation/2026-11-05-issue-1-spike-outcome.md) and [Loop host timing](../../../docs/development/loop-host-admission.md).

## Goals / Non-Goals

- Keep the captured source, prepared swell revisions and routing independent of how Perform schedules wet.
- Use existing realtime preparation, publication, gain and mixing limits across all three playback routes.
- Limit the first version to a single realtime Playlist placement per instance. Multiple placements, move-with-clip behavior, new FL MIDI commands and host offline rendering are outside this change.

## Decisions

### Select the playback route from independent evidence

On FL Studio, an active placement selects the absolute timeline route. Otherwise, a compatible declared repeat selects host repeat playback. Manual Cue is available when neither route is active and its timing is valid. Both Pattern and Playlist can expose repeat bounds, so use the bounds directly. An early seek discards continuity and recomputes the phase from the current verified host position. A verified wrap preserves the phase and overlap history. On Rack, the latest valid Phrase supplies the origin and verified Clock plus the accepted length predict later phrase starts. Clock-only and legacy Phrase-only configurations keep their existing admission and recovery rules. Reusing the next bar after a Phrase would allow intermediate bars to launch a multi-bar Loop.

### Save a musical position and schedule by intersection

Save the placement as host PPQ bound to the accepted Loop identity. Set Loop Start reads the reported position while stopped or playing. The owner confirmed that FL Studio supplies different positions when the parked playhead moves. The right-click or Shift+F10 menu offers manual entry and clearing. Reuse the current supported tempo, meter and rate checks and derive the sample origin for the current run. A command queued in the UI must validate the workflow and source identity before it writes placement state.

The first placed pass begins at its earliest prepared wet interval. For each block, intersect the current host position with the positioned contributions, including cross-pass leads. Repeat from the saved position until a stop command or transport interruption. A scheduled Stop lets the current pass and tail finish and suppresses later entries for that run. Seeking reevaluates the intersection. Reuse Loop wet preparation and bounded overlap accounting.

### Add a reusable Action Button + State Field

Compose the shared ActionButton input, command ink and focus painters with an EllipsisLabel or equivalent static accessible text. Use the existing controlLabel and controlValue roles and the raised field surface from SelectorPresentation. Paint a zero-gap join with a continuous outline, square inside corners, the existing 4-unit outer radius and a single seam. Keep the 1-unit field rails steady. Take the command's thicker held stroke inward and coordinate both halves at fractional scales. Keep the focus reserve and paint its brackets in the composite overlay around the action body. The action hit area ends at the seam and the field has no Tab stop. Keep the accepted value visible at full opacity when the command is disabled or a request is pending or rejected.

Use the host row's existing command allocation and body inset, including the 36-unit standard and 32-unit compact allocations. Measure action widths using the longest supported label and existing label inset. Preview a 124-unit default field width with an explicit per-instance override and the written reference's proposed 12-unit field padding. Cap the field to available layout width. Keep its geometry independent of the text. Fade clipped text at the edge. While the pointer remains over an overflowing field, wait briefly and scroll once to its end at a readable rate. Reset on exit or text change. Keep the complete value available in the fixed hover-help footer and through accessibility.

### Compose the Perform row

Use one aligned row. Both **Audition** and **Loop** use the standard section-heading text role at each width. Song and Loop share one Audition action, status field, mix control and command binding. The primary Play glyph becomes a destructive Stop glyph while Audition is playing. In Loop, show the divider and the **Loop** heading, then primary **Set Loop Start** joined to the accepted bar and beat and **Cue Loop / Cancel Loop / Stop Loop / Stop Now** joined to live status. Use the existing explicit neutral and destructive variants for the Loop playback command. The play/stop readiness and saved position are independent.

Use the existing fixed hover-help footer for “Set Loop beat one to the Playlist playhead.” For hosts without supported Playlist placement, keep **Set Loop Start** visible and disabled, and show “Requires a host song position” in the footer on hover and in the accessible description. Bind footer help at the composite boundary if the disabled button does not receive mouse events. Keep any accepted position legible and use explicit empty-state text when no position exists.

Launch a native preview of the reusable component and the three-pair row before integration. Check 900 × 600 and 1100 × 850, 100%, 125% and 150% scaling, the border and focus at the seam, text overflow, command states and disabled-host help. Review the full row in at least one native host/DPI combination. Obtain owner approval of the new family and any new token, variant or exception before wider use, then update the UI standards with the approved contract and tests.

## Risks / Trade-offs

- FL Studio does not supply a reliable mode flag. A matching Playlist repeat can autoplay just like a Pattern. A saved placement takes precedence and the status shows the selected route.
- FL Studio may omit the playhead in some configurations. Preserve the accepted position when the host position is unavailable and offer bar-and-beat entry through the context menu.
- Starting inside a lead-in can only play samples from the current transport position forward. Verify complete entry from before the first wet interval, then verify joins and wrap overlap at sample level.
- Stopping a transport-driven Loop suppresses its automatic re-entry for the current run. Clear the suppression on the next observed transport run and preserve any saved placement.

## Migration Plan

Load existing projects with no saved placement. Their matching FL Studio repeat ranges become eligible for automatic playback when Live playback is enabled. Save placement as optional versioned session state and leave captured assets and render data untouched. Projects without a usable placement keep access to manual Cue where eligible.
