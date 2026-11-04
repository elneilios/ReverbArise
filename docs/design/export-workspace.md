# Export workspace implementation reference

Issue #6 is implemented and its current native presentation is owner approved.
This document records the references, control ownership and deferred integration.
Final Release/FL Studio acceptance remains distinct from native visual approval.

The owner supplied two mockups directly in chat on 2026-11-04: a large dedicated
Finalise screen and a four-panel 0/50/100% mix comparison. Use the current **Export**
name and current approved header, including its existing information-row behavior.
The large example incorrectly retains white dry attacks under a 100% Wet label;
the comparison sheet and written issue resolve that as aqua wet only. Render real
samples, not the idealized identical illustrative events. Keep the current flat
charcoal, aqua, neutral dry and Inter typography instead of copying gradients or
obsolete labels from the mockup.

Below the unchanged header, the large left preview occupies roughly two-thirds;
the right action column uses the remaining third, divided by a thin rule. Align
heading, explanatory caption, original-song ruler, waveform/legend and internal
Play/Stop/time readout on the left. On the right keep exact Dry–Wet percentage,
large dashed aqua WAV tile, Save WAV and Return to Perform. There is no Export
Summary, modal overlay, inspector, list or duplicate Overview.

## Control ownership before integration

| Element | Existing owner / intended reuse | Remaining work |
| --- | --- | --- |
| Header modes, Settings and information | Current HeaderInformation, SegmentedSelector, IconActionButton and shared theme | Preserve approved header; bind real Export state only through existing API |
| Dry–Wet slider and typed/stepper percentage | Controls/SliderControl.h and NumericInputControl.h | Independent Export model binding and stale invocation guard |
| Play/Stop, Save and Return actions | Existing button/keyboard policies; IconActionButton currently owns only Settings/open-file icons | Extract reusable command/action family once; approve revised look and interaction |
| WAV tile | No existing external-file tile family | Shared keyboard/focus/disabled/drag behavior with callbacks; screen does not write files |
| Native Save chooser | Existing owned asynchronous chooser pattern | Shared revision/session-pinned invocation and explicit overwrite handling |
| Waveform, ruler, coverage and preview cursor | Existing bounded peak/timing conventions | Specialized read-only Export view; no creative handles or host seeking |
| Warnings and readiness | Current shared information/notice presentation | Stable, meaningful persistent feedback; no flashing helper messages or new summary panel |

The library owns generic input, accessibility, focus ink and styling; bindings own
percentage/revision/session identity and commands; worker code owns preparation,
verification and publication. The screen owns layout. Reuse existing 1100x850
opening/minimum size and larger supported layouts with shared spacing/font tokens.
Preserve complete focus outlines and the approved numeric stepper behavior.

0% shows neutral dry only, 50% shows weighted dry plus pre-arrival aqua wet, and
100% shows wet only (a subdued dry legend may remain). Waveform layers use one
disclosed amplitude scale and the actual gain/protection plan. Original song
coordinates remain visible even when preview fitting/start skips leading silence;
the WAV still begins at song origin. Missing dry remains visibly missing even
when the selected Export policy substitutes silence into the composition.

Native component checkpoint A approves only new/revised families before rollout.
Integrated checkpoint B reviews the dedicated reference-mix/pending/error states;
unchanged header/controls retain their prior approvals. Launch each native preview
directly. Consolidate real FL Studio checks at the end, recording actual scale
coverage separately from software raster fixtures. No new control approval is
claimed by this inventory.


## First integrated implementation (superseded checkpoint B evidence)

The Export workspace now replaces the Perform-shaped temporary view below the
unchanged header. Its dominant left waveform uses measured weighted dry/wet
peak summaries, absolute bar labels, a distinct preview cursor and one disclosed
scale. The right column uses the approved slider/stepper, WAV tile and command
families. Save and tile keyboard/click fallback share one safely owned chooser.
File checking/copying runs on the engine worker; screen code receives only ready
metadata/native delivery results. Old-file actions are disabled while a new
request is pending, including a workflow mode roundtrip.

Native fixtures at 1100x850 and supported larger sizes retain header geometry
and hide the list, inspector, Perform timeline/audition and Overview in Export.
Endpoint pixel oracles found 0 aqua / 8846 dry pixels at 0%, 3132 aqua / 3967 dry
at 50%, and 7210 aqua / 0 dry at 100% for the reference fixture. Supporting
pending/error/warned-dry/all-zero and 125/150/200% software-raster views are in
`out/issue6-integrated/`; these do not establish OS-DPI or real host drag behavior.
The first checkpoint B was refined after review. The revised acceptance is
recorded below; no FL Studio #6 acceptance is claimed by these original fixtures.

## Revised checkpoint B (owner approved 2026-11-04)

The owner's shorter nine-point feedback supersedes the earlier extended review.
The revised candidate preserves the header and 66/33 composition, shortens the
WAV tile to approximately 4:3 and groups its icon/text. This is a local layout
candidate, not a new universal geometry token. Aqua Wet swells and neutral Dry
source keys identify the actual weighted layers; zero-contribution keys say off.

Fit restores all audible contributions with margins. Wheel zoom anchors at the
pointer, Shift-left-drag pans, horizontal/Shift-wheel pans, +/- zooms, arrows pan and
Home/F or double-click restores Fit. The view remains read-only. New accepted
mixes fit once; ordinary refresh does not reset inspection. The transport readout
separates audible Preview duration from WAV duration at song origin.

Pending/error retains the last completed plot with scoped dimming and an explicit
previous-percentage label. The tile reports Preparing, Ready or Error; stale
Play/Save/drag remain disabled. The owner approved the presented layout,
mix-reactive waveform, tile, legend, readiness and pan/zoom/Fit interaction.
Reference mix, zoom/pan,
pending/error and software-raster evidence is in `out/issue6-refined-preview/`.
Software raster evidence is not actual Windows DPI or FL Studio acceptance.

## Deferred integration handover to issue #46 / Agent 2

This is approved current visual acceptance, not a failed or reopened checkpoint.
The owner separately approved two coordinated refinements: a fuller musical
timeline ruler above the waveform, and the shared Zoom − / slider / Zoom + / Fit
controls from #46. The current sparse bar labels are not claimed to satisfy that
future ruler. Export needs no Snap/grid or editing handles. Do not build a second
toolbar or refactor this viewer before coordinating the shared interface.

The integration should consume canonical absolute sample bounds and accepted
`Transport` timing, with `SwellViewport` as the view transform. Export supplies
its own Fit target: the complete accepted audible composition plus visual margins,
independent of selection and song-origin file duration. Controls should emit
navigation intents over that viewport, not own audio bounds or project state.
Preserve pointer-anchored zoom, pan gestures, once-per-accepted-mix Fit, manual
inspection during refresh, pending last-good presentation and exact current-file
readiness. Reuse the timing validity rules: show no invented musical positions.

`ExportWorkspace::viewport()` and `waveformBounds()` are existing test seams;
`Controls/TimelineNavigation.h` currently adapts gestures over `SwellViewport`.
The forthcoming shared navigation/ruler interface can replace that adapter once
agreed, without changing `SongMix`, export ownership, WAV alignment or playback.
Agent 2 is assessing composable waveform infrastructure; no extraction is part
of this validation pass. Merge and OpenSpec archive remain on hold until this
integration completes or the owner explicitly separates it into a follow-up issue.

After passing the FL Studio checks, the owner authorized click/drag preview
positioning immediately, before the shared #46 controls. Left-click/drag positions
the internal preview; Shift-drag preserves pan. Stopped seeks park without playing;
playing seeks continue after bounded refill. The immediate parked marker and
actual playing cursor share absolute mapping and existing preview ink; outgoing
fade-out cursors cannot overwrite the new target. Right-click does not seek.
Pending/failed previous plots are not seekable. No DAW seeking or audio editing
is introduced. This revised interaction needs its native feel check; prior
composition, file-delivery and FL Studio acceptance remain valid for unchanged
behavior. Coordinate shared control integration with Agent 2 against this seam.
