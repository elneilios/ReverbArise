# Proposal

## Why

GitHub issue: [#80](https://github.com/elneilios/ReverbArise/issues/80), under #79. Learn and Perform currently duplicate their IR picker, import lifecycle and feedback despite sharing the catalogue and engine. One reusable editor will keep both contexts consistent and give later individual DSP features a small extension point without changing sound.

## What Changes

- Extract one real convolution/IR editor using production controls, consumed by Learn Sound and Perform Sound through two explicit target bindings and a shared command/validation path.
- Keep Learn edits scoped to accepted future-generation defaults and Perform edits scoped to the invocation's selected swell. Preserve Generate snapshots, manual creation, provenance, frozen ownership and stale-command guards.
- Consolidate catalogue presentation, deliberate no-wheel choice, safe file chooser lifecycle, accepted/requested feedback, availability and context help. Retain engine validation as authority rather than duplicating it in each screen.
- Fit the delivered #58 labelled Sound access, responsive typography, keyboard focus and exact-value controls. Perform Level retains its current control and binding; no Learn Level default is introduced.
- Preserve existing managed assignments, saved format, render/cache identities, retained output, source PCM and audio/routing. No blanket regeneration, migration or new sonic defaults.
- Reconcile the older progressive-control wording with #58's delivered Sound access when Detect events is off; generation/detection controls remain conditional and Learn-only.
- Require native component/binding, persistence and baseline-audio evidence, scoped family approval, living guidance and offline-guide currency review.

No Tone/HP/LP/Stretch/Profile/Character widgets, new algorithms, generic DSP schema, routing graph, bulk editing, Export mixer changes or framework-wide migration are included. #30 owns the first working processing Character selector.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-workspace`: consistent shared IR editing, explicit scope/availability and compact access in Learn and Perform, preserving independent accepted assignments and existing creation transactions.

## Impact

Expected changes touch `Source/UI/Controls/`, a focused UI binding/adapter beside the shared editor, `Source/UI/LearnWorkspace.h` and `Source/Plugin/PluginEditor.*`. Reuse `SwellEngine::irChoices`, existing default/selected assignment commands, importer and worker validation; any engine adapter must be minimal. Extend existing IR-choice/handover, generation, native UI and editor-binding tests. Update UI ownership guidance, relevant Learn/Perform documentation, dated validation evidence and guide currency after implementation. No new dependency, host parameter or durable state field is planned.
