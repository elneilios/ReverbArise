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

Keep `WorkflowHeader`, NoticeBar and Learn composition intact. Introduce a Perform-scoped layout calculation/composite rather than altering shared dimensions used by Export. Keep one editor, its existing handle painting/hit geometry and separate Overview. Preserve all supported inspector fields via vertical scrolling, not reduced font heights. The owner's divider reference review clarifies that the old Perform auto-marker tooling is to be removed: omit its band, preserve saved instance detection preferences and Learn's controls, and do not adopt #38's generation behavior.

The geometry below was measured in native revision 4 and explicitly approved by the owner on 4 November 2026. `PerformLayout.h::performGeometry` and standards revision 3 now record its scoped values; production editor integration follows #50's merged baseline `9b8e70f` under the confirmed ownership contract. Retain current 64-high list rows (three readable lines), shared 34-high controls, 36-high switch component, heading 30, labels 24, helper lines 20 and full focus padding. Do not shrink these roles.

| Approved allocation | Below editor width 1200 | At/above 1200 | Rationale |
| --- | --- | --- | --- |
| Outer inset / header | existing 16 / 82 | existing 20 / 82 | Preserve shared baseline |
| List / inspector / column gaps | 180 / 304 / 16 each | 210 / 320 / 16 each | Recover center width; inspector scrolls |
| Perform globals / audition / Overview / info | none / 72 / 120 / 36 | same | Omit obsolete marker tooling; retain the other bands |
| Column content inset from horizontal rules | 12 top/bottom for list/inspector; 12 top for timeline | same | Preserve full-width rules after opaque child painting |
| Section heading / related gap / group gap | 30 / 8 / 12 | same | Scoped Perform spacing; native composition approved |
| Toolbar footprint / internal control | 48 / 36 | same | One row with focus clearance |

Derived budgets (outer content less header and bands):

| Editor size | Three-column height | Central width before internal inset | Detail after toolbar/readout and 12 top inset |
| --- | --- | --- | --- |
| 1100x850 | 508 | 552 | 422 |
| 1400x900 | 550 | 798 | 464 |
| 1400x1000 | 650 | 798 | 564 |
| 1600x1000 | 650 | 998 | 564 |

The reference uses full-width horizontal separators, with vertical rules aligned
to inset column content rather than joining the band rules. An opaque timeline
placed on the header divider erased its middle in the first previews. Test the
finished composite pixels across every horizontal rule, not just the parent paint
or the gaps at vertical endpoints. Audition and Overview retain 8-pixel vertical
insets. These scoped values have native approval; production adopts them after the owner confirmed #50 merged.

The native fixture must report actual dry/wet lane, ruler, scrollbar, toolbar, focus and inspector bounds and adjust these proposals before approval. A two-line 14-high helper must fit its proper line boxes. Test 1199/1200/1201 and adjacent header breakpoint behavior separately. Stop if the one-row toolbar or hit targets cannot fit; do not rescue it with smaller fonts or a second row.

The owner-approved shared row from merged PR #49 supersedes the initial toolbar arithmetic. In the minimum native fixture, the row receives 292 pixels: minus/plus 34 each, Fit 74, three shared 12-pixel gaps and a 114-pixel slider. Snap 96 and grid 136 retain two external 8-pixel gaps inside the 540-pixel inset toolbar. Slider grows at larger sizes. This measured composition received compact-layout approval and is adopted by the production editor.

The authorized Apply phase built the isolated native fixture using real component/data paths with empty, sparse, many/long-name, loading/error, selected and existing multiple-selection states. Launch it directly and preserve the review window. Record measured values, screenshots and explicit look/feel decisions. Only then roll approved metrics into production and update code, standards revision and tests together. This planning phase cannot create native fixture code; arithmetic above does not satisfy that checkpoint.

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
| Zoom actions and Fit | Approved shared `TimelineZoomControls` / `ActionButton` from merged PR #49; workspace owns limits, Fit and accepted state |
| Play/Stop and add | Existing action mechanisms; consolidate only affected consumers without extending audition scope |
| Headings, labels, help, empty states and notices | Named theme roles with embedded static Inter faces, semantic ink and reserved NoticeBar; no raw screen-local colour/font additions |
| Timeline/Overview | Existing specialized components; preserve painting/hit transforms, coverage truth and disclosed amplitude scaling |

Materially revised families receive a small native interaction checkpoint before wider adoption. Existing list/header acceptance is reused unless this change actually alters their appearance/behavior. Inventory untouched legacy controls explicitly in living guidance; no whole-plugin migration requirement. New profile tokens are the intended scoped exception; no unreviewed style variants. Never regenerate the standards debt inventory to absorb failures.

### 3. One bounded viewport, explicit Fit lifecycle

Use `SwellViewport` and timeline methods for every slider/button/wheel/Overview action. Slider is logarithmic in horizontal span; plus/minus use the owner-approved shared row's .8/1.25 span factors. Minimum is one sample per logical lane pixel (or whole available context when shorter); maximum covers known capture and real swell extent with existing 4% Fit padding, never beyond checked representable bounds. Pointer wheel anchors at pointer; toolbar anchors visible selected target, else visible playhead, else center. No navigation changes audio/model coordinates.

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

Store optional validated Perform view span/first, Snap requested flag and stable division key in the existing session JSON as a UI preference object. Planning main used session version 9/envelope 8; the subsequent read-only Export audit found its unmerged session version 10/envelope 8. Integrate into the then-current agreed merged decoder without downgrading Export or incrementing versions merely for optional UI fields. Readers of that same schema ignore unknown properties; compatibility with older schema versions remains governed by their actual decoder, not a claim of universal backward readability. Old states default to Fit, On, quarter beat. Invalid optional UI values reset only those preferences; never discard valid musical data. Retain per-instance preferences across editor close/reopen, project save/reopen and mode return; reset navigation ownership on session replacement. Coalesce UI preference publication outside the audio callback and follow host dirty policy. Do not persist active gestures, playhead, pending workers or inspection cursors. No host parameters or durable asset changes. Coordinate the additive object with #6 before either writer changes serialization.

### 6. Integration boundaries and verification

#4 is merged and consumed directly; its active OpenSpec delta remains unarchived, so do not duplicate or rewrite chronological-list requirements. Current canonical specs still say eight-region Perform and lack the toolbar details; this change refines those through a delta. Existing multi-selection is retained without #9 mixed inspector or new bulk actions. Preserve current audition position/mix APIs; #5 owns arrangement playback.

The planning audit saw #6 at `33f1871` without final APIs. Agent 1 subsequently supplied an explicit handover; final integration follows owner-confirmed merged #50 at `9b8e70f`. No other worktree is modified. #38 contract reviewed: accept source-ready Perform with pending wet, preserve manual/frozen IDs and return-to-Learn edits; do not change current Stop transition or Generate behavior. The final delta preserves merged Export composition and current-WAV truth rather than reverting its requirements.

Use focused pure viewport/grid tests, actual component gestures and processor/editor bindings, then Release wrapper full CTest and `scripts/check-ui-standards.ps1`. Cover source/render/IR identity, no navigation-triggered jobs, no audio callback lock/allocation/I/O, native geometry and embedded typefaces. Save dated evidence under `docs/validation/`, update implemented design/development guides and standards together; canonical sync/archive is a separately requested ordinary lifecycle step. Owner-approved native fixture precedes one brief FL Studio smoke on the exact Release bundle, with usual scale, pending-render transition, editing/zoom/snap and save/reopen. Software scales and actual Windows DPI/host results remain separately labelled.

### 7. Shared waveform composition proposal — owner steering, 4 November

The original interface proposal below was delivered independently in PR #49,
approved by the owner and merged into main at `12182f2` on 4 November. Its shared
Zoom/Fit row, ActionButton and musical ruler are now available. This approval does
not transfer Export implementation/state ownership. The owner subsequently approved
Perform compact revision 4 separately. The owner requested small composable boundaries across Learn/Perform/Export
while retaining the existing native composition checkpoint and #6 ownership gate.

**Safe existing shared foundation:** `SwellViewport` remains the sample/pixel and
viewport arithmetic authority; `makeSongBarRuler` remains the musical tick authority.
Named theme roles and focus/accessibility conventions remain shared. Learn keeps
`LearnDryView`'s stable coverage-driven extent policy; no manual navigation or new
toolbar is added to Learn as part of #46.

**Proposed small presentation interface:** a processor-independent
`TimelineZoomControls` under `UI/Controls`, composing existing action/slider
families, with no source/render data, viewport copy, snap logic or playback state:

```cpp
struct ZoomControlState
{
    double fraction; // Validated [0,1] position in the owner's zoom domain.
    bool canZoomIn, canZoomOut, canFit;
};
void display(const ZoomControlState&); // Silent accepted-state refresh.
std::function<void(double factor)> onZoomStep;
std::function<void(double fraction)> onZoomFraction;
std::function<void()> onFit;
```

Callbacks express UI intent, never host seek or model edits. The binding supplies
the logarithmic fraction/span conversion and validated limits; only the workspace's
existing viewport owner accepts a change. The component owns minus/slider/plus/Fit
layout, names, focus, keyboard/wheel policy and accessibility. It does not own Fit
bounds or automatically fit on data refresh. Fit remains Learn dry coverage,
Perform selected source/pre-roll/arrival/tail, or Export audible mix extent.
No workspace enum or collection of feature flags belongs in this component.

Agent 1's accepted `Controls/ActionButton.h`, merged through #49/#50, provides the generic
command family (focus, keyboard activation, invocation hook and named text roles).
The shared Zoom/Fit row consumes that family unchanged. Its native acceptance and
measured font/padding/bounds are retained. Snap is a separate Perform-owned latching action,
not a new Export flag or a reason to change the generic command family's semantics.

Read-only inspection of Agent 1's `TimelineNavigation.h`, now merged through #50, found an existing
`viewport()`, `setRange(audibleFirst,audibleLast,fileFirst,fileLast,rate,reset)`,
`zoom(factor,anchor)`, `pan(fraction)`, `fit()` and `onChange` API. Export currently
owns one view through that component, including its sample-rate-derived minimum
span, read-only pan/wheel/key gestures and detail requests. A binding could feed
the shared toolbar's intents into that API and report accepted zoom state back;
Agent 1 added accepted-state/min/max accessors for the slider fraction in #50.
Do not replace its ownership with a second view or silently
impose Perform's minimum-span/gesture policy. Perform retains editable gestures
and Ctrl snap bypass, with shared named conventions only where they agree.

**Small later extractions:** a ruler paint helper can consume existing ticks,
absolute viewport, bounds and named text/ink roles once two call sites demonstrate
identical drawing requirements. Cursor/coverage overlays can share simple drawing
primitives only where semantics and geometry match. No extraction of a monolithic
waveform renderer, decoded data model, worker cache or generalized feature-flag
viewer is proposed. Real peak/layer differences and invalidation remain local.

Before adopting shared production changes, agree the interface and shared-file
ownership with Agent 1 and record the then-current revision. Direct chat delivery
was rejected twice with an active-writer error. The owner subsequently supplied
[Agent 1's handover](https://github.com/elneilios/ReverbArise/issues/46#issuecomment-6629874432),
which agrees canonical absolute bounds, accepted timing, `SwellViewport` and
navigation intents, with a workspace-owned Export Fit target. Export's current
visual acceptance is retained; only the fuller musical ruler and shared zoom row
are coordinated refinements. Preserve pointer zoom/pan, manual view across refresh,
once-per-accepted-mix Fit, last-good pending display and current-revision delivery.
Agent 1 retains Export/mix/voice/state ownership. Merged #49/#50 and owner
acceptance resolved the original merge/archive hold and supplied the accepted
adapter/accessors. The isolated fixture remains independent. Component tests cover
silent display, bounds/focus/accessibility and intent callbacks; independent
Perform/Export binding tests prove one viewport, distinct Fit policy and no host
seeking, audio jobs or serialization side effects from navigation. Wider extraction is deferred
where it would destabilize the workspaces. This proposal does not absorb #6, #38,
waveform performance work or DSP.

## Risks / Trade-offs

Current #6 ownership contract: [Agent 1's explicit handover](https://github.com/elneilios/ReverbArise/issues/46#issuecomment-6632200424)
at Export baseline `13ec5bec36112540ec1eaa42d3e788413367d637` releases isolated
SwellTimeline/SwellViewport and Snap/grid work with existing APIs preserved.
The owner confirmed #50 merged at `9b8e70f`, releasing final PluginEditor and SwellSession integration; do not
absorb unmerged Export implementation. Preserve special Export visibility,
creative schema-10 fields/migrations and processor envelope 8. Export retains
Fit/seek/pan/file ownership; this replaces earlier unresolved-coordination status.

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
