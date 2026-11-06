# Design

## Context

See `proposal.md` for motivation and issue #80 scope. Planning baseline is merged main `dbfdb18`, including #58 compact composition and #51 Export levels. No matching active change or implementation PR was found. This document is planning, not implementation or approval evidence.

Observed implementation:

- `LearnWorkspace.h` owns IR labels, `IrSelector`, file action, catalogue rebuild, requested/default status and session/pass-pinned callbacks.
- `PluginEditor.*` independently owns selected-swell versions of the same UI and callbacks, pinned to ID/input generation/session. The Sound inspector also contains the existing Level knob/exact edit.
- `Controls/IrSelector.h` shares deliberate selector behavior only; it is not yet a shared IR editor. `SwellEngine::irChoices()` already supplies bundled and instance-retained managed choices with content deduplication.
- Existing `assignDefault*` and selected `assign*IR` APIs converge on existing import/worker mechanisms. `queueIR` guards creative permission, session, input generation and freeze; successful explicit IR editing retains the current automatic-to-manualEdited provenance. Default publication validates workflow/pass/request ownership and retains the prior accepted default on failure.
- `LearnGeneration.cpp` snapshots defaults in guarded generation transactions. Manual creation in `SwellEngine.cpp::addTarget` snapshots the accepted default. Neither needs replacement.
- #58 exposes Learn Capture & Generate / Sound and Perform Timing / Envelope / Sound, using `ResponsivePresentation.h`. The production guide places a Learn Level default outside #58; #80 also does not add one.
- The older `Learn exposes independent progressive capture controls` requirement still says IR hides with Automatic Swells off. Current responsive layout, the later Sound-access requirement and living production guide expose Default IR independently. The delta reconciles that specific wording with delivered behavior; it does not change capture/generation policy.

## Goals / Non-Goals

**Goals:** One actual reusable component with shared edit lifecycle, one focused command adapter with explicit context targets, and one authority for IR choice/validation. Prove that presentation reuse does not become settings or engine-state sharing.

**Non-Goals:** No universal parameter framework, new default model, schema version, audio algorithm, gain policy, cache rewrite or controls for hypothetical future stages. Retain adjacent Level editing rather than adding a gain field to Learn or pulling envelope logic into the IR family.

## Decisions

### 1. Compose the existing controls into one real family

Introduce a focused `Controls/ConvolutionIrEditor` (name may follow local conventions) containing the IR label/selector, file action and accepted/requested status presentation. Both workspaces instantiate that same class. It accepts a presentation snapshot and emits typed choice/import intents; it does not call the audio processor, import assets, render, or know workspace internals. Share catalogue-to-selection mapping, refresh suppression, deliberate no-wheel policy, accessible metadata, focus/help and status layout.

Reuse `IrSelector`/`DeliberateSelector`, `IconActionButton`, current text roles, flat theme, named spacing and responsive geometry. Retain full descriptions through existing accessibility/footer help. Do not introduce a family with mode flags that reaches into screen internals.

Alternative: extracting labels while leaving separate picker/import handlers in both screens would preserve the duplication #80 is intended to remove. A generic DSP-panel framework would exceed the foundation.

### 2. Use one binding path with two explicit target types

Add a focused binding/controller outside the control library. Its target is either FutureDefault(session epoch, workflow pass) or SelectedSwell(session epoch, persistent ID, input generation). Capture targets and choice identity at invocation; never resolve an asynchronous completion from current selection or a current row index. Catalogue menu invocations retain their offered entries/content identity.

The binding owns safe asynchronous file chooser lifetime, cancellation, shared command dispatch, and reconciliation to accepted model snapshots. Use JUCE safe component/lifetime ownership already established in the workspaces; destructed editors cannot receive completions. A menu/chooser for A may finish for still-valid A after selecting B, as already permitted, but never retarget B.

Dispatch both target types through one typed IR request path, then delegate to existing engine default/selected methods. Keep target-specific engine guards as policy, not duplicate validators in the screens. Catalogue descriptors/importer remain the IR format/content authority. No new numeric convolution parameter exists here; do not invent a parameter table of future fields. Shared validation is not permission to weaken existing default pass guards or selected revision/freeze guards.

Alternative: separate default/per-swell controllers copying chooser and validation logic would fail the common command-path acceptance. Replacing all engine APIs with a generic settings model would unnecessarily risk persistence and worker publication.

### 3. Share display mechanics, preserve distinct accepted states

Build presentation snapshots from accepted default state or the selected swell plus existing pending/retained information. Hosting screens identify context scope outside the shared field; the component has no scope-message API or scope child. This follows the owner's family-review correction. Missing selection clears old values; frozen or disallowed edits show existing unavailable reasons. A missing IR is unavailable data, not a fabricated ready assignment; retain any recovery choice permitted by current engine policy. Pending default and retained audible selected IR are different facts and must remain so even when drawn by the same class.

All refresh paths are notification-free. Remove the replaced catalogue/chooser/IR-status ownership from both screens after integration, including unreachable legacy layout branches that would otherwise reference removed widgets. Diagnostics' separate default-import recovery action may remain as an entry point routed through the shared binding; it must not grow a third independent editor or lose workflow guards. Inventory other consumers without migrating unrelated controls.

Alternative: normalizing both contexts to a single mutable settings object would create live inheritance and violate ownership. Showing the requested IR as accepted before worker publication would misrepresent sound.

### 4. Integrate within the accepted Sound regions

Keep Learn's dominant dry canvas and persistent Generate/readiness/manual guidance. Sound is accessible with detection off; its editor obeys pass locks. Perform keeps its single waveform/envelope editor, fixed Reset area and current exact Level editing beside the shared IR editor. Detection remains Learn-only; Export owns final Dry/Wet levels.

Use the #58 future Sound capacity specimen as a size/access constraint, not permission to ship its hypothetical Character/filter controls. Review real content at 900x600/100%, fitting 125% user magnification and representative larger sizes. Reuse supported scale-fit behavior; do not claim 900x600 at 125% fits every screen. Native review covers long imported names, descriptions, pending/failure, empty/frozen/unavailable states, keyboard/pointer access, exact Level and footer help.

Provide one prepared native family fixture in the existing UI test harness and launch it directly for owner look/feel review before wider integration. Launch the final integrated preview after both bindings are ready, avoiding duplicate windows and preserving the active review window. Existing primitives retain acceptance; the new composite and changed interaction receive scoped approval. No new visual tokens/variants are planned. Any necessary exception must be presented with scope/rationale/native evidence and approved before rollout, with standards revision/tests updated together.

### 5. Establish sound and state invariance through existing fixtures

Extend `tests/ir_choices_test.cpp`, `tests/ir_handover_test.cpp`, generation/provenance fixtures and native editor-binding tests rather than writing an alternative model. Component tests assert both screens contain the real shared class and exercise actual mouse/keyboard/file-intent routes. Test accepted/rejected edits at the binding boundary and eventual engine publication.

Cover target isolation; imported-content deduplication and save/reopen without the original file; session/selection/input/pass supersession; freeze/deletion; cancel/error/latest request; Generate/Stop/manual creation snapshots and protection; no-notification refresh. Preserve exact saved assignments, capture manifests/bytes, render references, gains and routing on inspection/reopen. Verify no unnecessary render-job increase for presentation-only activity and retained-output behavior on replacement failure.

Use deterministic source/IR fixtures to compare before/after accepted raw/shaped wet PCM and dry output under identical settings, and exercise existing audition/live/export/offline regression paths appropriate to the extraction. No new rendering math is required. Tests prove unchanged audio content while admission/routing policies remain separate. File work and UI allocations stay off the real-time callback.

## Risks / Trade-offs

- **Shared UI hides different target policies** -> explicit target variants, existing engine authority, negative binding tests and accepted-state reconciliation.
- **Late popup/chooser retargets another swell or session** -> capture all ownership at invocation, safely own async lifetime, test stale completion through the real binding.
- **Refactor loses requested-versus-audible feedback** -> snapshot both facts and test delayed/failing import with last-good output.
- **Compact composition crowds exact Level or Reset** -> bounded native family and integrated review using existing responsive tokens; do not shrink text or add another waveform.
- **Scope drifts into future processing** -> extend only this editor/binding when later audible features arrive. #30 can add real engine disclosure then; no enum/schema/cache placeholder is needed now.
- **Existing spec statements predate delivered UI** -> reconcile only the identified Sound-off visibility clause and preserve all its original scenarios; unrelated stale wording stays outside this change.

## Migration Plan

1. Apply only after a fresh owner request. First add and test the isolated family and target binding, then obtain native composite approval.
2. Replace both production IR editors together, preserving engine methods, persistence and assets. Extend focused tests and run the supported Windows wrapper/checks.
3. Obtain integrated native acceptance, maintain living guidance and offline-guide currency, then consolidate host testing into one brief FL Studio smoke after automated gates. Record untested DPI/host coverage without treating it as passed.
4. Deliver one issue PR for owner review. No self-merge or automatic archive. Rollback is reverting the UI/binding commit; no asset or saved-state conversion should need undoing.
