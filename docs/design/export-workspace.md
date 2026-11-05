# Export workspace implementation reference

Issue #51 replaces the right-column crossfade with the owner-reviewed Mix Character
selector and two shared editable Dry/Wet rows. Each level ranges from 0–100%;
100% is unity for that branch. Presets modify only the pair; unmatched pairs show
Custom. The white dry/aqua wet waveform, legend, retained-preview caption, status
and immutable filenames describe the accepted independent pair. The original left
waveform/ruler, Zoom/Fit, transport and right delivery actions retain their layout.
At 0/0 the canvas reports Silent and preview/seek/Save/drag are disabled. Overload
uses the existing warning role; Float32 samples are not attenuated automatically.
Earlier single-crossfade descriptions below record the #6 reference baseline.

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
pointer, Right-drag pans, horizontal/Shift-wheel pans, +/- zooms, arrows pan and
Home/F or left-double-click restores Fit. The view remains read-only. New accepted
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

**Current status:** PR #49 is merged and its shared row/ruler are implemented in
Export. The new integrated native checkpoint remains open. Original playhead
feel is owner approved and its behavior is retained. The historical handover
below is now being fulfilled; no new waveform infrastructure was extracted.
Both Debug and Release integration suites pass 46/46; see the
[integration evidence](../validation/2026-11-04-export-shared-timeline-integration.md)
for source/binary identity, native images and approval limits.

Following owner placement feedback, a single row beneath the waveform holds
`TimelineZoomControls` on the left and the compact Wet/Dry percentage legend on the right.
The numerical shared-scale caption is omitted; both layers still use the same
amplitude scale. Related spacing separates this row from Preview and provides
additional bottom clearance. Only meaningful delivery/silent-mix feedback adds
a status line beneath the row. Dry-inclusive mute-source guidance is part of
the tile's Ready status, alongside percentage and song alignment. The compact
single-line caption leaves related spacing before the ruler. The header notice
retains Export playback safety guidance. The row retains its approved control geometry; the
waveform keeps its width/ruler alignment and yields the footer's required height.
The right column and header remain unchanged. The slider uses the existing adapter's accepted limits logarithmically;
buttons, slider, wheel and keyboard reflect the same clamped viewport. Fit still
shows the complete audible export. `paintMusicalTimelineRuler` uses the same
absolute viewport and accepted timing, including denominator-aware beat ticks.
Unknown timing is explicitly unavailable rather than fabricated. Pending
last-good navigation remains usable; stale epoch/pass/composition callbacks are
rejected. No Snap/grid, source editing, DSP or saved viewport state is added.

The owner approved the completed shared row/ruler integration and right-drag pan
on 2026-11-04. Export uses canonical absolute sample bounds and accepted timing,
with SwellViewport as the transform. Fit remains the complete audible composition
plus margins, independent of Perform selection and WAV lead-in. Controls emit
intents, with the workspace retaining audio bounds and ownership. Ruler timing
is never fabricated. Ordinary refresh preserves manual inspection; current-file
readiness remains revision-bound. No new waveform infrastructure was extracted.
Agent 2's final Perform editor/serialization integration follows #6's merge,
with the current interface and schema boundary recorded in the shared handover.
Merge and OpenSpec sync/archive remain owner-controlled.
After passing the FL Studio checks, the owner authorized click/drag preview
positioning immediately, before the shared #46 controls. Left-click/drag positions
the internal preview; Right-drag pans. Stopped seeks park without playing;
playing seeks continue after bounded refill. The immediate parked marker and
actual playing cursor share absolute mapping and existing preview ink; outgoing
fade-out cursors cannot overwrite the new target. Right-click does not seek.
Pending/failed previous plots are not seekable. No DAW seeking or audio editing
is introduced. The revised interaction has native owner approval; prior
composition, file-delivery and FL Studio acceptance remain valid for unchanged
behavior. Coordinate shared control integration with Agent 2 against this seam.
