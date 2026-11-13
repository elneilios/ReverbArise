# Learn workspace

## Shared Sound editor — issue #191

Learn Sound and Perform Sound use the same preset picker, contextual Custom IR
file row, Level layout and guarded binding. Learn edits the default used for
future Generate and manual creation. Sound remains reachable with detection off.
Descriptions follow the controls. A slow request covers the Sound viewport after
two seconds, while a failed import places actionable feedback below the file row.
Pass, mode and session guards apply to controls and asynchronous choosers.

Issue #11's first native composition passed checkpoint A on 2026-11-03.
It reuses the approved static-Inter header and reserved message row.
At 1100 × 850 and larger sizes, a 304-pixel left column holds Source Capture /
Recapture and Automatic Swells. The remaining area is one dry-source canvas.
Perform's list, inspector, audition band and compact Overview are hidden, with
its navigation and selection retained for return. Learn navigation never seeks
the DAW.

## Issue #38 preview and generation backend

### Current musician-focused native review

Learn retains the approved comfortable text/control roles. Detect events and
Event sensitivity describe detection; Generate Swells and Generate on Stop
describe creation. The selector is Default IR. Redundant switch descriptions are
removed; the manual hint still follows the action helper and stays visible with
detection OFF. No new panel, density profile or DSP setting is introduced.

The Audio view uses a width-proportional waveform band, at most half its width
subject to available height. Filled PCM min/max buckets cover their actual time
span at unchanged unity amplitude; quiet audio is not normalized. Musical mapping,
real gaps and stable dry extent remain intact. Neutral event guides yield to the
waveform; hover/pin uses the existing timing accent. Hollow heads still indicate
provisional/updating data. Emphasis never filters Generate or selects swells.

One caption combines capture state, duration and event count. The canvas readout
is musical time only. Exact samples, silence/coverage classifications, mapped
totals and inspection details remain in existing Diagnostics. This revised
composition/emphasis received scoped native owner approval on 2026-11-04.
That acceptance does not establish FL Studio, OS-DPI or screen-reader coverage.

Learn candidate snapshots are separate from saved swell definitions. Sensitivity
edits update read-only background analysis across accepted mapped source, retaining
previous marker positions marked updating. Progressive markers can use immutable
staged capture chunks, but remain provisional until Stop accepts source and a
matching final snapshot is ready. Abandoning a pass restores the prior settled
display; no provisional source can generate swells.

New projects default Auto-generate on Stop OFF and settle in Learn. Explicit
Generate and eligible opted-in Stop share one guarded replacement transaction;
only a changed viable automatic result enters Perform, before wet rendering
finishes. Current accepted Default IR applies to that request without changing
markers or protected assignments. Manual/edited/frozen and outside-scope work
remain intact. Returning to Learn or restoring state never generates by itself.
See the [source/context protocol](../development/learn-source-context-protocol.md)
and [in-progress validation](../validation/2026-11-04-issue-38-generation-workflow.md).
The Generate on Stop switch reuses `Switch` immediately beneath Detect events,
before the Default IR group. Generate Swells reuses the emphasized `ActionButton`
directly below sensitivity. Its contextual readiness helper sits
directly below the button; the persistent manual-placement hint follows last,
flowing naturally across the available width without a forced break after the
question. Both remain readable when generation is disabled. The manual hint
also remains visible beneath the enable-switch group when Automatic Swells is
OFF, including on editor reopen; generation controls and their contextual
helper stay hidden. The accessible
description also explains readiness. Invocations pin session, pass and
displayed candidate identities. Automatic Swells OFF hides these controls while
retaining the saved preference. Neutral heads/guides show events, with timing
emphasis for hover/pin; hollow heads identify provisional/updating positions. They are
distinct from the white inspection pin and solid DAW cursor. Scoped native owner
review was approved on 2026-11-04; host smoke validation remains pending.

## Approved #11 foundation

Checkpoint-A polish uses the existing Inter heading role for Source Capture,
Automatic Swells and Capture Timeline. Enable automatic swells is a separate
Medium field label; toggle, IR and sensitivity labels share the primary-white
14-pixel Medium role, with 14-pixel grey Regular descriptions 6 pixels beneath.
Balanced helper lines avoid isolated trailing words within the existing text
column, preserving switch alignment and panel proportions. Dropdown and slider
values share the explicit 13-pixel Regular control role; the shared invocation
slider preserves its value font when its text box is recreated.
Learn now uses the reusable `SliderControl` and `NumericInputControl`: wheel
adjustment is enabled, and the existing percentage field includes increase/
decrease buttons using the slider's range and interval. Dragging, typing and
stepping share invocation ownership and the same model binding. The numeric field
keeps its 74 × 28 dimensions; both step controls have focus and accessible names.
Ruler, caption and capture status
also use that readable minimum without changing header or Perform typography.
Groups stay top-aligned using shared
4-pixel tight, 12/16-pixel related and 24-pixel section spacing. Only one horizontal
rule separates the source and generation sections. The header's approved
geometry is unchanged. Timeline heading/caption/ruler/status occupy distinct,
aligned rows and edge ruler labels stay within the canvas width.
The secondary-grey caption and primary Medium ruler have a 6-pixel gap; reducing
the following clearance to 10 pixels preserves the waveform viewport area. The
empty-state icon, instruction and explanation form one vertically centred group
with tokenized 12/6-pixel internal gaps.

`UiGeometry.h` centralizes spacing and affected control/frame dimensions. Switch
tracks remain 44 × 24 within 56 × 36 components: the extra padding contains the
entire focus stroke. Both switches share their size and right edge; the folder
and Settings actions draw their focus strokes inside their own bounds. Native
focus/geometry and scaled-raster checks supplement the integrated preview;
scaled rasters do not claim operating-system DPI acceptance.
Joined mode-segment outlines are painted by the group after child fills, with
hover/press borders last. This prevents the next selected segment from erasing
the shared hover edge without changing child/accessibility order or hit areas.

The effective Automatic Swells switch is On only when both saved processing and
marker policies are enabled. Explicit On enables both. Off disables processing
and retains marker/IR/sensitivity preferences. Off hides the IR, file-import and
sensitivity controls. Recapture remains independent and transient.

The real factory/retained-import catalogue drives the Default IR dropdown and
adjacent folder action. It displays the accepted choice while a pending import
prepares, and explains a failed or superseded request. Successful imports remain
available when unused and after reopening.
The helper remains "Used by Generate." throughout selection,
preparation, success and failure; neither its text nor its geometry changes.
Editor-owned presentation logic suppresses IR progress for the first 600 ms.
Longer pending work uses the existing contextual message bar, without changing
loading or asynchronous acceptance. Completion removes that progress without a
success flash. Errors remain model-owned and persistent in the message bar, with
retry guidance and full details in its tooltip. Higher-priority capture/workflow
notices retain their existing precedence. No additional status row is introduced.
The horizontal Marker Sensitivity
slider has a typeable percentage; no Character controls appear. Generation
controls and their model commands lock from Play through Stop validation.

Shared switch, invocation slider/toggle and folder-icon controls live under
`Source/UI/Controls`. Existing Perform checkbox styling is preserved while its
affected generation bindings gain invocation ownership. The new switch/slider
look and feel require owner approval before wider rollout. The owner explicitly
approved the reusable numeric up/down control and the final stable IR-feedback
refinement on 2026-11-03; preserve them. Checkpoint A is complete. Broader coverage
display, persistence and checkpoint B remain separate unfinished tasks.

The empty canvas contains a centred waveform glyph, prominent instruction and
automatic/dry-only explanation. Primary capture instructions never occupy the
left configuration column. Recording and acceptance preserve real dry peaks;
concise phase-specific feedback lives in the combined caption. Any accepted coverage,
including silence or unavailable assets, suppresses the empty prompt.

Checkpoint A now brings forward the worker progress path from task 6.1: a separate
pass/epoch-fenced summary includes admitted input before storage batching. It is
display telemetry, not accepted source. Recording overlays use a restrained yellow
treatment; acceptance/cancel/failure/restore cannot promote stale telemetry into
saved or rendered source. Publication is capped at 10 Hz, with 4096 ranges/65536
peak bins below 4 MiB. Reaching the telemetry limit drops the overlay and discloses
the limit, without stopping valid capture. Exact coverage inspection, totals and
stable sparse fitting remain tasks 6.2–6.4 and checkpoint B; they are not complete
by approval of this composition. Source/context persistence and deterministic
generation remain separate backend tasks. No new FL Studio/DPI acceptance is
claimed by native screenshots or component tests.

## Final timeline behavior (checkpoint B candidate)

The approved typography and header remain unchanged. The musician-facing canvas
uses concise capture/event status; technical coverage labels and the separate
mapped/provisional/actual-extent summary are in Diagnostics. Exact-zero coverage alone is silence;
small audio peaks are never reclassified. Missing overview data is unavailable,
not invented silence. Hover inspects the absolute sample beneath the pointer,
using the current dry viewport; Y does not select a different channel or value.
Left-click pins that sample; another left-click relocates it. The pin survives
hover, refresh and keyboard focus moving elsewhere. Right-click neither pins nor
relocates it, and opens no menu. With canvas focus, Escape clears the pin;
Home/End inspect viewport boundaries and left/right inspect coverage boundaries.

The white dashed inspection line/dot and pale coverage wash are independent of
the aqua keyboard-focus outline and yellow DAW transport/provisional indicators.
Inspection remains available while recording or accepting capture. The existing
readout shows bar/beat and elapsed song time only with reliable compatible host
timing. Otherwise it says Timing unavailable. Exact samples, coverage classification
and full ranges remain available in Diagnostics. No timing is inferred from an
unavailable host position.

Inspection is UI-only: no seeking, editing, capture commits, generation or saved
state changes. Its absolute sample survives host seeks and extent growth. Mode
re-entry, project/instance replacement or session restore clears the pinned flag
and coordinate together; a new editor starts unpinned. Yellow provisional ranges
remain distinct from accepted source and disappear on failure/cancel/restore.
In Song, live peaks cover newly mapped audio and ranges admitted for Recapture.
In Loop, the live waveform grows from the start of the phrase. When a take
replaces the whole source, the canvas shows its provisional waveform during
recording and validation. A cancelled or failed take returns the canvas to the
previous accepted source.
If a Loop reaches the detailed coverage limit, Learn keeps displaying its
growing waveform and recorded extent. Inspection reports that coverage detail
is unavailable.

The Song viewport fits only committed/admitted dry bounds, anchored to song origin
with quantized bar headroom (seconds fallback). It expands at extent thresholds
and never shrinks for loops, backwards seeks, cancellation or shorter recapture
while open. Re-entry recomputes the fit; Perform navigation, selected wet bounds
and host seek position do not drive it. One-frame inferred-origin jitter is
absorbed. Negative source stays visible with a Song start marker; unavailable or
unreadable musical timing shows Time unavailable, with absolute samples in Diagnostics.

The owner approved checkpoint B on 2026-11-03, including the focused retry of
click/keyboard inspection. The owner also approved the refined persistent pin and timing readout in the
native preview on 2026-11-03. Preserve this accepted composition and interaction. Native raster scaling
checks are not OS-DPI/FL Studio acceptance. Production design-system density for
Perform remains a separate review.
