# Design

## Context

See [proposal](proposal.md) for motivation and boundaries. Read-only audit at main `33cd54b` on 4 November 2026:

| Observed implementation | Consequence |
| --- | --- |
| `WorkspaceLayout` reserves header 82, globals 66, audition 82, Overview 142, info 44; list 180/210 and inspector 320/340 with two 20 gaps | At 1100x850 the central allocation is 528x402 before internal reductions; production roles need a deliberate spacing budget |
| `PluginEditor::resized` allocates Prev/Next/Zoom/Fit, bar-span selector and Snap in one fixed-width sequence | No horizontal slider or selectable snap division; overflow risk at minimum size |
| `SwellTimeline::resized` has a separate 24-high amplitude control, 42 top reservation, 32 lane gap and scrollbar | Horizontal zoom must remain independent of amplitude display; avoid silently deleting amplitude access |
| `SwellViewport` already owns pixel/sample transform, fit, zoom, paging and fixed quarter-beat snap | Extend this authority; do not create a toolbar-specific time mapping |
| `dragTo` snaps arrival/source bounds only; wet endpoints/fades use initial pointer delta | Extend absolute timing snap after pointer-delta conversion, retaining no-jump semantics and actual hit anchors |
| `SongTime` provides sample, PPQ, rate, tempo and meter; musical readouts define a beat as the meter denominator unit | Use the same beat definition. There is no historical tempo map or explicit bar-start PPQ in this snapshot |
| #4 merged `Controls/SwellList` and epoch-bound ID callbacks, with existing multiple selection | Reuse its sorting, selection and scroll rules; #9 still owns bulk inspector/actions |
| Editor retains Perform viewport across Learn visits but has no saved viewport/grid preferences | Add optional UI-only state without making navigation an audio parameter |

Visual references: [#3 approved Perform viewer](https://github.com/elneilios/ReverbArise/issues/3#issuecomment-6577860872), #46, production guide revision 2 and current controls. The mockup was inspected: actual two-lane editing, three columns, toolbar below detail, audition and single Overview. Its old labels, illustrative data, ambiguous division labels, surfaces and compact numbers do not override production standards.

## Goals / Non-Goals

**Goals:** retain existing editing and geometry authority, give the center more useful room, and prove coordinate, identity and lifecycle safety before rollout. Prepare an isolated native composition checkpoint before applying compact values to production.

**Non-Goals:** new sorting/selection models, bulk inspector or delete/undo workflows, another audition/mix engine, generation transitions, tempo-map capture, waveform caches, DSP or asset migrations. Existing multiple selection remains supported at its current level; no fictional mixed-value editing.

## Decisions

### 1. Isolate Perform composition and stage native approval

Keep `WorkflowHeader`, NoticeBar and Learn composition intact. Introduce a Perform-scoped layout calculation/composite rather than altering shared dimensions used by Export. Keep one editor, its existing handle painting/hit geometry and separate Overview. Preserve all supported inspector fields via vertical scrolling, not reduced font heights. Existing marker preferences stay in their current domain; migrate affected switch/value presentation without adopting #38's future generation behavior.

Candidate `performCompact` geometry below is **proposed logical-pixel arithmetic, not measured native evidence or approved tokens**. Begin with current 64-high list rows (three readable lines), shared 34-high controls, 36-high switch component, heading 30, labels 24, helper lines 20 and full focus padding. Do not shrink these roles.

| Proposed allocation | Below editor width 1200 | At/above 1200 | Rationale |
| --- | --- | --- | --- |
| Outer inset / header | existing 16 / 82 | existing 20 / 82 | Preserve shared baseline |
| List / inspector / column gaps | 180 / 304 / 16 each | 210 / 320 / 16 each | Recover center width; inspector scrolls |
| Globals / audition / Overview / info | 56 / 72 / 120 / 36 | same | Recover detail height without changing fonts |
| Section heading / related gap / group gap | 30 / 8 / 12 | same | Named Perform spacing only; approval required |
| Toolbar footprint / internal control | 48 / 36 | same | One row with focus clearance |

Derived budgets (outer content less header and bands):

| Editor size | Three-column height | Central width before internal inset | Detail component after 48 toolbar + 26 readout |
| --- | --- | --- | --- |
| 1100x850 | 452 | 552 | 378 |
| 1400x900 | 494 | 798 | 420 |
| 1400x1000 | 594 | 798 | 520 |
| 1600x1000 | 594 | 998 | 520 |

The native fixture must report actual dry/wet lane, ruler, scrollbar, toolbar, focus and inspector bounds and adjust these proposals before approval. A two-line 14-high helper must fit its proper line boxes. Test 1199/1200/1201 and adjacent header breakpoint behavior separately. Stop if the one-row toolbar or hit targets cannot fit; do not rescue it with smaller fonts or a second row.

Toolbar width starting budget inside the minimum center: minus 34, slider 96, plus 34, Fit 40, Snap toggle button 96, grid selector 136, five 8 gaps = 476, leaving 76 for insets/focus and responsive growth. Labels use explicit units and tooltips; text/focus containment determines final approved widths. Slider grows at larger sizes.

After a new Apply request, build the isolated native fixture using real component/data paths with empty, sparse, many/long-name, loading/error, selected and existing multiple-selection states. Launch it directly and preserve the review window. Record measured values, screenshots and explicit look/feel decisions. Only then roll approved metrics into production and update code, standards revision and tests together. This planning phase cannot create native fixture code; arithmetic above does not satisfy that checkpoint.

Alternative rejected: reusing legacy spacing as already approved, or globally shrinking Learn/header to solve Perform density.

### 2. Reuse controls with a bounded migration inventory

| Affected family | Reuse / extraction and binding ownership |
| --- | --- |
| List | Existing `SwellList`, `SwellListOrder`, immutable rows, primary/selected IDs and snapshot epoch; preserve row family appearance until separately approved |
| Snap toggle button | Reuse the shared button/theme pressed-state policy as a reusable latching text action, with effective-state feedback; no checkbox or switch-track presentation. Preview look/feel before wider adoption |
| Activity and marker toggles | `Switch` presentation with existing invocation/session guards; consolidate affected duplicate setup |
| Zoom and audition sliders / exact numeric values | `SliderControl`, `NumericInputControl`, `InvocationSlider`; domain bindings keep units, ranges, active-edit text and validation |
| Existing knobs/exact inspector controls | Extend working `KnobValue` with shared numeric/edit lifecycle; include corresponding affected Settings/Diagnostics consumers, keep independent domain steps |
| Grid/IR selectors | Shared selector/no-wheel invocation policy; extend existing `IrSelector`/invocation patterns for grid, not a new screen-local interaction policy |
| Zoom icon actions, Fit, Play/Stop and add | Existing `IconActionButton` and theme button policy; a missing reusable text-action primitive is an extraction from existing behavior, separately previewed before adoption |
| Headings, labels, help, empty states and notices | Named theme roles with embedded static Inter faces, semantic ink and reserved NoticeBar; no raw screen-local colour/font additions |
| Timeline/Overview | Existing specialized components; preserve painting/hit transforms, coverage truth and disclosed amplitude scaling |

Materially revised families receive a small native interaction checkpoint before wider adoption. Existing list/header acceptance is reused unless this change actually alters their appearance/behavior. Inventory untouched legacy controls explicitly in living guidance; no whole-plugin migration requirement. New profile tokens are the intended scoped exception; no unreviewed style variants. Never regenerate the standards debt inventory to absorb failures.

### 3. One bounded viewport, explicit Fit lifecycle

Use `SwellViewport` and timeline methods for every slider/button/wheel/Overview action. Slider is logarithmic in horizontal span; plus/minus change span by factor two. Minimum is one sample per logical lane pixel (or whole available context when shorter); maximum covers known capture and real swell extent with existing 4% Fit padding, never beyond checked representable bounds. Pointer wheel anchors at pointer; toolbar anchors visible selected target, else visible playhead, else center. No navigation changes audio/model coordinates.

Fit includes the selected swell's source context, requested audible pre-roll/end, placed raw render and actual tail/overlaps intersecting that context; a pending render uses truthful requested/source bounds. Without selection use accepted capture extent, retaining gaps and negative coordinates. Empty has a finite neutral view. Initial Perform entry Fits once when relevant source is ready; selection/background refresh does not repeatedly Fit. Explicit Fit resets context; return from Learn/Export restores prior user view. Overview viewport is derived from exactly the same span, never seeks the host. Keep display-amplitude control disclosed in the detail heading, outside the horizontal toolbar, using existing gain law.

Alternative rejected: sharing Learn auto-fit or introducing another ruler transform, which would cause view jumps or coordinate disagreement.

### 4. Snap absolute musical positions, validate mapping, preserve edit guards

Offer `1/16 beat`, `1/8 beat`, `1/4 beat`, `1 beat`, `1 bar`, `2 bars`, `4 bars`; Snap enable is independent and defaults to existing On with 1/4 beat for legacy projects. Per the owner's 4 November clarification, Snap is a latching toggle **button**, not a checkbox or switch track. Its saved requested state and transient effective state are distinct: `effective = requested && !ctrlHeld`. Holding Ctrl anywhere in the active editor temporarily shows the button off (including accessible pressed state), without changing the saved request or grid; release restores the requested state. If saved Off it stays Off on release. Tooltip/accessible description explains temporary bypass. Clicking the button still toggles the saved request; Ctrl masks its visual/effective result until released. Poll/handle modifier changes during gestures and idle focus, reconcile on focus loss/return so no stuck bypass survives, and do not dirty the host or persist held keys. Shift retains its existing fine-pointer-delta behavior independently; Ctrl+Shift combines unsnapped timing and fine movement. Existing Ctrl-wheel horizontal zoom and list selection shortcuts remain functional without changing the saved Snap preference. Releasing Ctrl changes the effective mode immediately but does not publish an edit until the next actual movement/nudge; do not move a handle merely because a key changed.

A beat is the meter denominator note, matching `MusicalPosition`; PPQ is quarter notes. Therefore grid step in PPQ is beat fraction × 4/denominator, or bars × numerator × 4/denominator. For coherent fixed mapping, origin in samples is `sample - ppq * rate * 60/bpm`. Convert absolute candidate to PPQ, round to nearest grid multiple relative to PPQ zero (half ties away from zero), then round once to integer sample. Reject non-finite/overflow results. Test negative and fractional PPQ explicitly.

Timing is usable only when sample/PPQ/rate/tempo/meter are finite, available, compatible with accepted capture, and coherent with its accepted mapping including origin. Readouts, grid and snapping must share this validity decision. Freeze a valid timing snapshot and identity/epoch at gesture begin; if mapping/mode/session/selection changes, cancel further gesture publication and report why. Ordinary seeks/loops retaining the same origin are valid. Existing accepted edits remain; no partial new intent reaches another ID.

The current engine has no historical tempo/meter map or bar-start authority. Never extrapolate the current tempo across a detected changed mapping or guess a new bar origin. With effective Snap On but untrustworthy mapping, disable horizontal snapped edits with a reason; allow switching Snap Off or holding Ctrl to edit exact sample coordinates. Zoom/Fit/vertical edits remain usable. Supporting snapping across an actual variable tempo/meter map requires a separate dependency/owner decision, not a heuristic in #46. This preserves correctness under supported fixed mapping and rejects unsupported timing changes truthfully.

Compute wet endpoint/fade candidates from initial anchor plus pointer delta, snap their **absolute displayed sample**, then convert through existing placement/offset setters. Snap horizontal arrival, source-bound and editable envelope-time handles; tension/amplitude remains continuous. Whole-wet translation snaps its translated audible-start anchor once and preserves all internal spacing. Preserve ordering/support rules: use a nearest valid grid point within bounded domains, or reject with previous accepted value when none exists. Do not clamp to an off-grid point while claiming snapped. Exact typed fields stay unsnapped. Ctrl temporarily bypasses all these horizontal snap paths; Shift independently enables fine movement; baseline/delta handling must avoid a modifier-induced jump. Source selection edits may trigger existing render work but never move/recapture captured PCM.

Alternative rejected: quantizing pixel distances or local durations, which breaks absolute alignment, placement and fractional PPQ.

### 5. Backward-compatible UI preferences, no audio authority changes

Store optional validated Perform view span/first, Snap requested flag and stable division key in the existing session JSON as a UI preference object; keep version 9 and processor envelope 8. Old readers ignore unknown properties; old states default to Fit, On, quarter beat. Invalid optional UI values reset only those preferences; never discard valid musical data. Retain per-instance preferences across editor close/reopen, project save/reopen and mode return; reset navigation ownership on session replacement. Coalesce UI preference publication outside the audio callback and follow host dirty policy. Do not persist active gestures, playhead, pending workers or inspection cursors. No host parameters or durable asset changes. Coordinate the additive object with #6 before either writer changes serialization.

### 6. Integration boundaries and verification

#4 is merged and consumed directly; its active OpenSpec delta remains unarchived, so do not duplicate or rewrite chronological-list requirements. Current canonical specs still say eight-region Perform and lack the toolbar details; this change refines those through a delta. Existing multi-selection is retained without #9 mixed inspector or new bulk actions. Preserve current audition position/mix APIs; #5 owns arrangement playback.

#6 local branch `feat/6-export-mixed-song-wav` was at `33f1871` with no divergence to inspect. This is not evidence of final APIs or authorization to edit another worktree. At Apply entry obtain owner-coordinated shared-file/timeline/preview/state ownership against its then-current revision. Until recorded, keep Perform fixture/composition isolated and stop shared refactors. #38 contract reviewed: accept source-ready Perform with pending wet, preserve manual/frozen IDs and return-to-Learn edits; do not change current Stop transition or Generate behavior. Reconcile canonical requirement wording with #6/#38 at final integration rather than reverting their approved behavior.

Use focused pure viewport/grid tests, actual component gestures and processor/editor bindings, then Release wrapper full CTest and `scripts/check-ui-standards.ps1`. Cover source/render/IR identity, no navigation-triggered jobs, no audio callback lock/allocation/I/O, native geometry and embedded typefaces. Save dated evidence under `docs/validation/`, update implemented design/development guides and standards together; canonical sync/archive is a separately requested ordinary lifecycle step. Owner-approved native fixture precedes one brief FL Studio smoke on the exact Release bundle, with usual scale, pending-render transition, editing/zoom/snap and save/reopen. Software scales and actual Windows DPI/host results remain separately labelled.

## Risks / Trade-offs

- [Minimum-width toolbar or inspector overflow] → native measured bounds and long text/focus tests before approval; revise scoped spacing, not typography.
- [Stale timing, fractional origin or variable mapping] → mapping/epoch guard and explicit unavailable snap state; no fabricated tempo map.
- [Shared composition/serialization conflicts with #6] → owner-coordinated file/API contract before shared mutations; recheck at final integration.
- [UI preference state causes audio work or save churn] → optional isolated object, no-op/coalesced updates and callback/job-count tests.
- [Large migration expands beyond #46] → consolidate only affected families/consumers; report required architectural expansion before proceeding.
- [Paint cost] → preserve current fidelity and interfaces; #25 rebaseline/cache remains separate, with no claimed speedup.

## Migration Plan

1. Review these planning artifacts; obtain a fresh Apply request.
2. Resolve #6 ownership, add focused timing oracles and isolated native dense/empty composition fixture; directly launch for compact/family approval.
3. Roll approved roles into Perform, implement viewport/grid gestures and optional UI preferences with meaningful regressions.
4. Run final native/standards/host gates, record exact tested binary and open/update one implementation PR without merging or auto-closing issues.
5. Rollback uses the prior binary; unknown UI preferences can be ignored while source, renders, musical definitions and external assets remain intact. No cleanup or rename is part of rollback.
