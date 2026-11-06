# Design

## Context

See proposal.md for motivation. This is C1/P1 under the approved #107 inventory and contracts, standards revision 11. Base `717c890` contains merged #80, its shared `ActionButton::paintSurface`, stable icon ink and shared ConvolutionIrEditor/binding. No reimplementation of that correction is required.

`Source/UI/WorkspaceTheme.h` still tests names in drawButtonBackground/drawButtonText for audition, stopAudition, resetWetShape, resetSwellMenu and loadIR. The first four style actual legacy commands; loadIR now names a shared IconActionButton and its old TextButton branch is orphaned. `LatchingActionButton` delegates off-state painting but duplicates on-state surface/focus drawing. Legacy text button theme painting has no explicit contained focus treatment, and neutral disabled+hover precedence differs from the shared surface. Existing control/IR tests already exercise portions of command parity, keyboard, destruction and name invariance; use them as references rather than claiming complete coverage.

This is a cross-cutting family extraction spanning shared controls and main/auxiliary consumers, so a design and native review boundary are required. No code is implemented by proposal creation.

## Goals / Non-Goals

**Goals:** One working shared command state implementation, explicit semantic/content ownership, predictable eligible activation, and a finite consumer migration. Preserve existing command effects, layout intent, primary/reset/latch meaning and test discovery names.

**Non-Goals:** Active #108 Level range/KnobValue/input work; active #8 storage/recovery/engine work and its new window controls; selectors, checkbox toggles, segmented navigation, numeric steppers, row actions, Rename dialog internals or FileDragTile rendering. No new token, destructive command reclassification, DSP, persistence, capture, cache or asset work. Common editor source files may overlap other branches: edit only named command declarations/setup, with no whole-file reformatting or unrelated merge resolution.

## Decisions

### Current implementation status

The owner approved native family revision 4: “this is perfect and looks good”.
This satisfies the family gate and supersedes the pending-review wording in the
historical iterations below. Standards revision 12 records the accepted anatomy
and colours. The named production migration is implemented with unchanged
callbacks; automated integration and actual-editor acceptance are tracked
separately in tasks and dated evidence.

Main subsequently delivered #8 recovery/shared-engine work at `f98157e`. It is
retained as the new verification baseline, with no changes to its runtime/storage
source. The fixed 20-command migration and independent #108 scope remain unchanged.

During integrated review the owner accepted the styling apart from the IR import
button: its visible border had become shorter than its dropdown and crowded the
folder glyph. The shared IR composition now reserves the button's focus frame
around a full control-height body and aligns that body with the dropdown. This
uses existing geometry, preserves glyphs and feedback spacing, and remains at the
scoped integrated correction gate, now satisfied by explicit owner approval after
85/85 native checks. No knob or interaction change is involved.

### Owner revision at family review — hover, held press and focus

The first native candidate passed focused checks and 75/75 Release CTest, but
the owner requested clearer click feedback, destructive hover/press feedback,
and suggested tinting icons with the ring states. The revised candidate uses
existing tokens: muted `hover` outer outline, held neutral `separator` fill with
strong `aqua` outline, held destructive `danger` fill, and an independent inset
focus stroke. Primary/on-latch hover and press remain visibly distinct; focus
uses contrasting ink on filled surfaces. Neutral glyphs follow hover/press/focus
accent, while filled primary glyphs retain dark contrast. This supersedes the
initial candidate's stable-icon and combined single-ring choices, pending
owner approval of the revised native preview. It introduces no new token or
command behavior. FileDragTile's specialized painter is unchanged.

### Owner anatomy refinement — outer corner brackets

The owner subsequently selected thin aqua corner brackets, separated by a small
outside gap from the button border. The candidate reuses the knob's four-open-
corner visual pattern and existing 2-pixel focus stroke, but does not edit knobs.
The button component reserves stroke plus gap around its body so all focus ink
stays contained; preview frames reserve that space without shrinking the shown
34-pixel body height. Brackets are always aqua, independent of primary/destructive/
latched fill. Hover and held feedback remain on body border/background. This
supersedes the inset focus treatment above and remains at the native family gate.

### Owner colour refinement — retained focus, semantic body and stable glyphs

The owner explicitly liked the outer corner-bracket focus. Retain it unchanged.
They rejected aqua destructive hover and aqua icon hover. The current candidate
keeps the destructive outline red, increases its stroke on hover and uses the
existing red fill while held. Neutral icon glyphs stay off-white across hover,
press and focus; primary/filled semantic content retains contrasting ink. Mouse
feedback belongs to border/background, independent of the accepted aqua focus
brackets. This supersedes revision 2's suggested icon accents. No new token or
knob change is involved; full-family approval is still pending this preview.

### 1. Fix the consumer boundary before coding

The 20 persistent `juce::TextButton` members at this baseline are:

| Group | Exact members | Explicit role/content to retain |
| --- | --- | --- |
| Perform add/audition | `addButton`, `auditionButton`, `stopButton` | neutral add glyph, primary play glyph, neutral stop glyph; preserve titles/help and existing 24/34/40/50-width composition |
| Perform resets | `resetButton`, `resetMenu` | destructive text Reset / Reset All; `resetMenu` is a direct command, not a popup |
| Settings Swell | `removeButton`, `fullButton`, `setBarsButton` | neutral text Delete, Full response, Set source from bars; no new danger classification |
| Settings Processing | `defaultIRButton` | neutral default-IR command; retain route through Learn's shared IR binding |
| Settings Recovery | `generateButton`, `allButton`, `analyzeButton`, `confirmButton`, `recaptureButton`, `cancelButton`, `clearButton`, `suppressionButton`, `openDiagnosticsButton` | neutral text, existing labels/eligibility/effects; these baseline Settings commands are not #8's storage/recovery-window feature |
| Diagnostics | `assetInfoButton`, `timingButton` | neutral copy-assets command and timing disclosure; preserve clipboard content/disclosure semantics |

The two latch consumers are `snapButton` and `auditionWetOnly`. Already-shared `copyDiagnosticsButton`, `copyBuildInformation`, header Settings, Learn Generate/import, both shared IR import actions, TimelineZoomControls command children and Export play/stop/save/back remain regression references. They may consume the consolidated painter, but receive no new command behavior or layout redesign. FileDragTile inherits ActionButton but retains its explicit specialized override and drag policy.

Alternative: migrate every Button subclass. Rejected because boolean toggles, steppers, segments, row hit regions and native dialogs have different family contracts. Removing name-based styling does not require migrating their internals.

### 2. Extend the existing painter with explicit roles

Reuse `ActionButton::paintSurface` as the authority. Introduce a small explicit neutral/primary/destructive variant contract and accepted-latch input rather than another painter. Keep current bool-primary construction source-compatible or migrate its scoped calls with proven equivalence; do not break Export or #80 consumers to simplify this API. Make icon/text content explicit, retaining existing play/stop geometry and adding a typed add glyph/content mode for the narrow plus button. Names and labels remain metadata only.

Candidate state matrix uses existing tokens and contained geometry:

| Role | Idle / hover / pressed | Focus / disabled |
| --- | --- | --- |
| Neutral text or glyph | Neutral idle; muted aqua hover outline; held separator fill and strong aqua outline. Neutral glyph/text ink stays steady and readable | Independent outer aqua corner brackets with a clear gap; disabled body eligibility takes precedence over transients |
| Primary | Aqua idle / muted aqua hover / aqua held fill with stronger outline; dark content retains contrast | Outer aqua corner brackets independent of fill; dimmed disabled body |
| Destructive | Danger idle ink/outline, stronger red hover outline, held danger fill/strong outline with dark content | Unchanged outer aqua corner brackets; revised body feedback remains at owner review |
| Latch off/on | Off neutral; on aqua/dark content, with separate hover and held feedback | Same independent focus/disabled resolver; binding still owns accepted effective state |

No numerical color/spacing/alpha token is invented here. The destructive focus and corrected disabled-combination treatment are scoped family revisions, not already-approved appearance. Obtain owner native acceptance of the prepared candidate before production consumers adopt it. Do not rename Delete/Clear/Reset deleted targets into destructive variants based on visible text.

After consumer migration, remove all getName-based button branches and the orphaned folder glyph path from WorkspaceTheme. Retain a neutral generic fallback for unrelated JUCE TextButtons using shared neutral surface/font policy, without converting native dialogs or adding incidental label rules. Verify Rename dialog OK/Cancel remain usable as a fallback regression only.

Respect the existing include direction: ActionButton includes WorkspaceTheme, so WorkspaceTheme cannot include ActionButton to call its static painter. If the generic fallback needs that surface, extract the existing resolver/painter beside the theme's semantic tokens (before WorkspaceTheme's definition) and have both the theme and the public ActionButton::paintSurface adapter delegate to it. Keep one implementation and preserve the #80 adapter API; do not create an include cycle or duplicate neutral painting.

Alternative: set properties per screen or keep a name-to-role map. Rejected because it preserves implicit appearance ownership and scattered state policy. Alternative: new button framework. Rejected because the merged painter and native fixtures already work.

### 3. Preserve callbacks and eligibility across input paths

Keep existing onClick bodies, beforeInvocation ownership, enabled-state refresh and domain validation. Pointer, Return, Space and accessibility press must issue one eligible callback, with existing queued/lifetime behavior. Audit the actual JUCE accessibility press route; use a shared invocation mechanism only where needed so it cannot bypass capture, double-trigger on key release or read destroyed state after a callback. Do not add a new global command bus or model permission system.

Do not silently strengthen or weaken command semantics during migration. Commands currently querying accepted state on activation keep that behavior; commands with captured session/pass tokens retain them through dispatch. Tests must distinguish disabled input rejection, queued component lifetime and binding-specific stale-context rejection rather than claim every command already uses the same snapshot guard. Preserve Snap requested/effective distinction and audition listening session/pass. Programmatic latch display remains notification-free.

Alternative: change handlers while extracting UI. Rejected because command/session/audio preservation is a hard boundary of this change.

### 4. Verify the family before rollout, then integrate bounded groups

Extend `Tests/ControlLibraryCases.h`, #80 `ConvolutionIrCases.h` and appropriate UI harness with neutral text, neutral glyph, primary, destructive, latch-off/on specimens and actual callbacks. Add automated idle/hover/pressed/focus/disabled/combined-state rendering, contained strokes, name/relabel variant invariance, long-label/plus-glyph geometry, keyboard/accessibility activation, one-shot callbacks, disabled/queued/destruction and silent latch cases. Reuse pixel regions/semantic assertions rather than brittle full-image goldens.

Directly launch the prepared native preview for owner review before replacing legacy production members. Reuse existing #80 neutral glyph acceptance; show only revised destructive/focus/disabled/latch feel and representative narrow/text actions. Keep one preview window open through review. This gate is not met by the owner approving #107's documents.

After approval, migrate Perform commands/latches, then the listed Settings/Diagnostics commands with their existing callbacks. Run actual-editor binding checks for add, audition play/stop, resets, regeneration/default import, bar-range/Delete, diagnostics copy/disclosure and accepted latches; verify eligibility and protected-state invariance. Extend existing focused fixtures rather than introducing model/audio tests that merely mirror setters. Exercise existing persistence/audio oracles for untouched behavior where migration changes dispatch paths.

Supported review: 900x600/100%, 1000x700, 1100x850 and 1400x1000; 100%/125% user magnification, long labels and comfortable auxiliary windows. Distinguish actual Windows 100/125/150/200% DPI and FL Studio from logical/software-scale results. Run `scripts/build.ps1 -Configuration Release -Test` and `scripts/check-ui-standards.ps1`; preserve lock/cache and exact debt rules. Native owner review plus automated integration should avoid repeating prior host checks; request one brief host smoke only for a concrete unresolved command/focus-host behavior.

## Risks / Trade-offs

- [#108/#8 share editor files or framework ancestry] -> Use current-main baseline when Apply begins, limit diff to named command members and fixtures, preserve the explicit exclusion list; do not merge or change their branches/worktrees.
- [TextButton replacement changes narrow glyph layout or discovery] -> Keep names/titles and explicit content modes; test compact plus/play/stop and existing native query paths.
- [New keyboard/accessibility activation duplicates a command] -> Test real delivery and callback lifetime, not only keyPressed return values; preserve existing binding guards.
- [Generic theme fallback affects native Rename buttons] -> Keep neutral fallback and test OK/Cancel without migrating dialog internals.
- [Shared surface consolidation changes #80/Export variants] -> Run existing icon parity/IR/Export tests and retain specialized FileDragTile rendering and prior scoped acceptance.

## Migration Plan

1. Confirm baseline and exact residual consumers, implement/test the isolated shared candidate, and obtain scoped native family approval.
2. Migrate the listed production groups with unchanged bindings, eliminate name-based theme appearance and test fallback/native compatibility.
3. Update standards revision from the then-current approved revision, living contracts/inventory/debt and dated evidence together; review guide currency for any user-facing keyboard/focus changes.
4. Leave one owner-review PR with `Refs #107`. Roll back this presentation child independently if needed, never captures/assets/saved state. Archive/sync are later explicit workflow actions; no #107 closure or self-merge.
