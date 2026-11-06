# Design

## Context

See proposal.md for scope. Planning started from #10's implementation at
`dda1a6c`; implementation now reconciles merged main `653f13a` via `dc003a7`,
including #10/#18 and #16/#24. The approved shared static-Inter header and its
reserved notice row are reused. The workflow word already locks capture/settlement,
stages source privately and publishes accepted source on explicit Stop. The
editor always lays out the eight-region Perform composition; its current
capture snapshot exposes committed coverage, not the incoming provisional
waveform. Capture batching can retain several seconds before staging a chunk.

`Session.defaultIR` and sensitivity are mutable instance preferences.
`reconcileTargets` reads the then-current default when creating a target, so
delayed analysis can assign a later choice. Explicit Recapture currently forces
dependent refresh even with automatic processing off. Selected-swell catalogue
assignment and the future-default file importer already manage immutable WAV/
WavPack assets; reuse their validation, not a second decoder/catalogue.

The source manifest is currently schema 1; processor envelope and swell JSON are
independently version 8. Source, IR and render assets use the existing managed
root and immutable references. Preserve the zero-parameter VST3 identity and
#10's coalesced host-save notifications.

## Goals / Non-Goals

**Goals:** a mode-specific, truthful learning surface; continuous provisional
feedback without accepting source early; coherent per-pass generation defaults;
dry-only learning with no automatic DSP jobs when disabled; focused visual and
functional verification.

**Non-Goals:** replacing the workflow state machine, changing audio calibration,
new Character DSP, destructive cache cleanup, or redesigning Perform/Export.
Use the delivered #16 display mapping and shared controls; do not implement that rename here.

## Decisions

### 1. Add a dedicated Learn body while preserving Perform geometry

Use a mode-specific layout branch/component beside the existing Perform layout,
sharing header branding, mode navigation, theme and settings ownership. At the
1100x850 opening size, budget approximately 280 logical pixels for the left
controls and give remaining space to the full-song timeline. Treat that width
as a checkpoint metric, not a raster-derived constant; controls may scroll
vertically at constrained sizes while the waveform retains its area.

```text
+------------------------------------------------------------------+
| Wordmark / tagline                Learn | Perform | Export   gear |
+------------------------+-----------------------------------------+
| Source Capture         | Captured source                         |
| Recapture [off/on]     | absolute ruler / yellow DAW playhead     |
|                        |                                         |
| Automatic Swells [on] | one full-song dry waveform               |
| Default IR / file      | audio / silence / gaps / provisional     |
| Sensitivity ---- value |                                         |
| concise state/help     | coverage key / timing / pass status      |
+------------------------+-----------------------------------------+
```

Hide the Perform list, inspector, audition strip and separate bottom Overview;
do not just shrink or disable them. Re-entering Perform restores its prior
selection, scroll/zoom and controls. Learn has an independent viewport and no
wet-envelope editing or audition affordance. Navigation does not seek the DAW.

Checkpoint-A polish keeps Source Capture and Automatic Swells as matching section
headings, with a separate regular-weight Enable automatic swells label beside its
switch. Field labels share one smaller typography role. Use shared 4-pixel tight,
12–16-pixel related and 24-pixel section spacing, with top-aligned groups rather
than height-filling distribution. Retain one restrained inter-section rule.
Switch component bounds include the entire focus stroke and padding; both
switches share dimensions and right alignment. Check other interactive outlines
and the timeline heading/caption/ruler/status rows for clipping or overlap while
preserving the approved header geometry and the dominant dry canvas.
The final checkpoint-A refinement keeps the header frozen, modestly widens the
settings column to 304 logical pixels, and uses 14-pixel secondary workspace text.
Restore the centred waveform glyph/instruction within an empty timeline, remove
left-column workflow instructions, and keep capture/acceptance feedback beneath
the waveform. Bring forward the existing worker-progress display from decision 2
so recording/acceptance show actual admitted data rather than an empty prompt.
This does not imply completion of coverage inspection, totals or sparse auto-fit.
The final focused polish uses an explicit bundled Inter Medium face for Learn
field labels, grey Regular helper text with a 6-pixel label/helper gap, balanced
description lines, and matching dropdown/numeric-value typography. Preserve the
304-pixel column, control dimensions and waveform viewport area. A single centred
empty-state group uses shared internal spacing rather than independent offsets.
These candidate Learn roles require owner approval before wider rollout; Perform
density will be reviewed separately before finalizing production design tokens.
The IR helper stays fixed in text and geometry across preparation/success/error
states. Suppress progress for near-instant operations; preparation still pending
after 600 ms uses the existing contextual message bar. Clear progress on completion
without a success flash, and retain actionable model-owned errors until retry or
resolution. This is presentation-only: preserve importer/loading/acceptance and
asynchronous ownership. Do not introduce a new status row or control. Adopt
the reusable wheel-enabled SliderControl/NumericInputControl family for Learn:
step buttons, wheel, dragging and typing share range, formatting, gesture and
invocation guards. The owner approved the numeric up/down family; preserve it.
Checkpoint A and the stable IR feedback refinement are approved. Preserve these
approved controls and composition while completing functional acceptance.
Paint joined mode-segment borders at group level, with hovered/pressed borders
after other borders, so neighbouring selected fills cannot clip a shared edge.
Retain square internal joins, rounded outer ends, stable accessibility order and
the existing selection/keyboard behaviour.

Follow the issue's approved mockups and flat style guide. Reuse the wordmark,
fonts, source grey, aqua accents and restrained yellow timing. The timeline is
the single Overview viewport; do not copy generated gradients or raised cards.
Omit Character controls completely until implemented algorithms exist; a static
Shimmer Cloud IR is not a Character algorithm. Reuse Settings/Diagnostics rather
than add developer controls to Learn.

### 2. Show committed data and provisional progress through separate views

Keep existing committed snapshots as the sole authority for save, render,
audition and playback. Add a bounded worker-owned progress snapshot tagged by
instance/session epoch and pass ID. It contains real admitted spans/peak bins,
coverage kind, current observed extent and phase, not another full-song PCM copy.

Publish display summaries at a capped cadence (target at most 10 Hz); coalesce
updates and downsample peaks within an explicit metadata budget. Include pending
admitted input so feedback does not wait for the current five-second asset batch.
Derive admission from the actual additive/Recapture transaction rules: protected
source and first-received provisional spans cannot be painted as newly replaced.
Provisional input may be displayed before its asset is durable, but is labelled
Recording, never committed. Do not increase durable write frequency to match UI
refresh or add callback allocations/locks/GUI work.

Show existing committed source throughout Recapture, with an explicitly distinct
provisional overlay. On acceptance swap to the coherent committed snapshot; on
cancel/failure remove the overlay and retain prior committed coverage. A pass/
epoch mismatch drops stale display updates just as it drops stale acceptance.
Display telemetry loss cannot invalidate valid audio capture, invent coverage,
or claim data that was never admitted.

### 3. Make coverage inspectable without interpreting waveform height

Draw audio from actual peaks, exact-zero captured silence as a distinct neutral
band/flat line, uncaptured intervals as the dark empty background with a subtle
gap treatment, and missing assets with the existing unavailable treatment.
Provisional capture uses a restrained progress accent/outline and an explicit
Recording label; aqua does not imply generated wet in this dry-only canvas.

Keep a small key with literal labels and an inspect/hover readout showing region
kind plus its absolute range. Include keyboard/focus or accessible text as an
alternative to colour/hover. A quiet release tail remains captured audio, not
silence merely because it appears flat. Use captured-duration as the union of
mapped spans, separately from total timeline extent; disclose provisional totals
separately. Do not imply a known song length, count an uncaptured seek as learned
audio, or present unavailable assets as silence.

#### Read-only interaction contract (owner refinement)

Map pointer X linearly to an absolute sample in the current dry viewport; Y does
not affect the position and there is no snapping. Unpinned hover updates the
readout. Left-click pins/relocates that absolute sample; subsequent hover and
focus movement to other controls must not replace or clear it. Draw the neutral
white dashed cursor/dot and inspected-coverage wash even when another control has
focus; aqua outline denotes waveform keyboard focus only. Solid yellow remains
the observed DAW transport position; yellow recording tint/stripe remains
provisional source. Right/middle click do not pin and have no new context action.

When the waveform owns focus, Escape clears coordinate and pin together; retain
Home/End viewport-edge and left/right coverage-boundary navigation. These keys
must not intercept another control's editing. Inspection stays enabled during
recording/drain/validation without issuing capture, commit, playback, generation
or persistent-state commands. Extent expansion/host seek retain the same pinned
absolute sample. Re-entering Learn or replacing/restoring/clearing the editor's
session/project scope clears coordinate and pin atomically; reopening restores
no inspection state. Ordinary capture acceptance keeps its established workflow.

Use the existing readout row: bar and beat (display precision up to three beat
fraction decimals), elapsed time from the reliably resolved song origin, exact
sample and coverage kind. Use only valid coherent fixed host timing and compatible
capture rate; otherwise show exact sample/coverage and explicit timing unavailable.
Do not invent origin, bars or elapsed time from an assumed zero. Negative source
may show negative elapsed time. Keep absolute coverage range and full readout in
tooltip/accessibility text. No extra rows or controls. Preserve actual coverage,
independent totals and threshold fitting, and keep all inspection ephemeral.

### 4. Auto-fit the dry extent independently of wet/selection state

Anchor the Learn viewport at song origin and include the highest committed or
currently admitted provisional position, with modest quantized headroom. Use
bar-sized headroom when timing is valid and seconds/sample fallback otherwise;
do not invent bar values when timing is unavailable. Existing negative-time
source, if any, must remain representable with origin clearly marked.

Only expand at a threshold, coalesced to the display cadence, rather than refit
every block. Do not shrink after a loop, backwards seek, cancellation or shorter
Recapture while the workspace is open; Fit/re-entry can recompute the accepted
extent. Repeated capture of the same extent does not change scale. Sparse seeks
expand the view but retain real gaps. Wet lead-in and selected swell bounds do
not stretch this dry-source view. Prefer a stable stepped fit over decorative
animation that obscures exact positions.

### 5. Freeze a generation context at pass admission

The confirmed UX locks Automatic Swells, Default IR/file assignment and Marker
Sensitivity from Play through draining/validation. Recapture follows #10's
existing arm/phase guards. Enforce this in model commands and async completion
admission, not only setEnabled; an old knob gesture or chooser cannot update a
locked pass. No-op/rejected attempts do not dirty the host project.

Prepare immutable generation configuration off the audio thread, including
effective automatic policy, detector controls and a stable accepted managed IR
identity/label. At successful begin, pin its revision/token through the same
bounded control-admission mechanism as the capture pass. Do not copy strings,
serialize settings or acquire the engine lock on the callback. A concurrent UI
edit resolves wholly before the pin or belongs to the next pass.

Pending IR import is not an accepted choice. Until successful publication, the
selector identifies the prior accepted default and separately shows pending or
failed intent. If Play wins the race, pin that accepted default; a late chooser/
import cannot silently replace the current pass's IR. Discard/defer that result
with a clear next-pass status and preserve the previous accepted value. Missing
default IR does not block dry capture; it produces honest generation status.

### 6. Publish source and generation provenance as one coherent record

Persist the context with the admitted source spans, not only a queued analysis
job or mutable session default. Use a versioned immutable capture commit record
that includes/references source coverage and immutable context metadata. Extend
manifest metadata compatibly if needed; do not change stored PCM or move roots.
Write and verify all referenced records before the existing single acceptance
fence. Saves must read one coherent accepted source/context snapshot, never a
new source pointer plus an old engine-side context table.

Ordinary additive capture retains each protected interval's context and tags only
newly admitted positions. Armed repeated positions use the last admitted source/
context pair within the pass. Coalesce adjacent equal assignments and reject
capacity overflow explicitly rather than drop ownership. Keep metadata within
documented bounds and account for it separately from the 256 MiB PCM budget.
Identical PCM and unchanged context reuse their identity; context-only updates
must not alter audio-content fingerprints or force existing swells to reconvolve.

Deferred analysis resolves the context owning a target's arrival; confirmation
or source-tail audio may cross a context boundary without changing that owner.
New swells receive an explicit immutable IR identity. Existing matched/manual/
edited/frozen assignments are never replaced by a new future default. Reopen
before analysis completes reconstructs the same assignments from accepted
provenance. Missing context/IR produces an error for generation without deleting
valid dry or silently taking the latest default. Manual Add in Perform keeps its
existing most-recent accepted default behaviour.

### 7. Separate dry-only learning from old marker/refresh switches

Expose one truthful Automatic Swells control in Learn. Migrate/bind the existing
master and marker preferences so a saved-off policy cannot appear On or silently
enable itself. Explicit On can enable the corresponding processing/marking
policies; Off preserves hidden preferences, all definitions and immutable assets.
Do not confuse this with Recapture or remove supported Perform marking actions.

With Learn Automatic Swells off, do not start automatic discovery, acceptance or
convolution for that pass, including legacy auto-refresh/forced-Recapture paths.
Fence/cancel obsolete automatic publication; preserve already accepted work and
allow manual creation/editing after entering Perform. Changed unfrozen existing
dependencies become honestly stale instead of playing mismatched audio; frozen
compatible results follow #10's exception. Explicit manual work remains possible
under Perform's guards. Do not silently defer disabled-pass convolution merely
to start it on automatic mode entry.

Learn default edits target future passes, not implicit backfill of old dry-only
captures. Existing deliberate Perform reanalysis remains available under its
current policy; distinguish that explicit request from ordinary delayed pass
work, preserving assigned IRs and protected edits. This avoids changing the sound
of old swells as a side effect of preparing the next pass.

### 8. Reuse catalogue, importer and host change notifications

List all six real bundled responses, every successfully accepted imported IR in
this instance, and Open from File in both Learn and Perform. Keep imports even
when no default or swell uses them, so auditioning alternatives does not lose
choices. Deduplicate by content identity; selecting an import reuses verified
managed bytes after the external file moves. Persist the bounded collection in
creative metadata v9; the host envelope remains v8. Restore/clone copies the
collection, with future additions independent. Legacy states seed it from their
accepted default and swell assignments; previously unused imports cannot be
reconstructed. Failed, cancelled or superseded imports do not enter the collection.
No directory scan, global library or automatic eviction is introduced. Reuse catalogue IDs/checksums and WAV/WavPack worker
validation. Add the default-target variant of existing assignment/import commands,
with instance/session/config-generation ownership and latest-request-wins guards.
Cancellation, decode failure, stale requests or deletion of the external file
cannot replace the prior valid default. Do not introduce a second importer.

Only accepted persistent settings/context/source changes notify the host using
#10's message-thread coalescing path. Progress, auto-fit, playhead, selection,
Recapture arming and rejected edits do not. Mode names use #16's display mapping;
internal values, bundle identifiers and historical records remain unchanged.

## Risks / Trade-offs

- [Provisional data mistaken for saved capture] -> separate display snapshot,
  literal Recording label, atomic source/context acceptance and cancellation tests.
- [Deferred work uses a later default] -> immutable arrival-owned context,
  two-pass/out-of-order/reopen fixtures and latest-job fences.
- [Large sparse captures grow metadata or UI cost] -> bounded/coalesced summaries,
  explicit limits, no full-song allocation, measured large-session fixtures.
- [Off still launches old forced work] -> test actual analysis/render counters
  across all Recapture/Automatic combinations, including pre-existing stale swells.
- [Hidden legacy flags contradict the main switch] -> effective-state migration,
  explicit enable semantics and save/reopen tests without resetting preferences.
- [New layout breaks Perform or host scaling] -> independent viewport/layout,
  native geometry tests and two user visual gates before consolidated host review.

## Migration Plan

Implement from the reconciled merged #10/#16 baseline recorded above.
Before code changes, pin the exact source-context envelope schema and bounds in
the implementation evidence. Read existing source schema 1 and supported state
versions; version only affected metadata domains. Legacy source has unknown
historical pass settings: preserve existing assignments and use a documented
legacy baseline from its saved preferences for future unassigned targets, without
claiming to recover the original capture defaults or requiring destructive recapture.

Persist current selected defaults/hidden values and accepted context references;
never resume provisional display, an armed pass or a voice from saved state.
Capture still saves only accepted source/context. Preserve earlier snapshots and
verify clone branches independently. Document older-binary rejection if a new
metadata version is necessary and retain pre-upgrade project copies for rollback.

Review checkpoint A: empty Learn, Automatic on/off, default picker and Recapture
warning at 1100x850 and a larger supported size. Review checkpoint B: real
audio/silence/gap/provisional/error coverage and stable sparse auto-fit; also show
unchanged Perform after returning. Stop for approval at each visual checkpoint.
Then use one short host checklist covering the user journey and custom default
save/reopen; group race/failure/large-data checks into automated evidence instead
of repeatedly asking for individual manual edge tests. Record actual DPI/host
results and limitations rather than inheriting #10's passes or waivers.
