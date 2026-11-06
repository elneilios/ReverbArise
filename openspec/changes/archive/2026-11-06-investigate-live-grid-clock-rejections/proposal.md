# Proposal

## Why

GitHub issue: [#73](https://github.com/elneilios/ReverbArise/issues/73).
Constant-tempo Perform playback can show unavailable Snap and a live clock
rejection, but the existing observations do not establish their causal sequence.
The separately repaired offline bounce must not be mistaken for an unresolved
export failure or proof that live timing is safe.

## What Changes

- Add read-only fixed-grid rejection reasons and the exact accepted/observed
  timing used by the grid decision, independently of live playback admission.
- Extend bounded live evidence to retain the run-start, last admitted and first
  rejected callback observations, explicit run-origin deltas and lifecycle
  context, without changing clock admission or audio scheduling.
- Reproduce the recorded constant-tempo tuples in native tests, distinguish
  source-origin offsets from changes within a run, and compare actual positioned
  wet audio with an independent reference wherever admission is supported.
- Record build/source/render identity, causal findings, unresolved host behavior
  and a concrete recommendation before proposing any timing-policy repair.
- Reuse the existing local `fix-offline-export-clock` change and evidence at
  branch `codex/fix-offline-export-clock` (implementation `1532f37`, installation
  `09b5696`, owner result `a5b36d2`). Its 68/68 Debug and Release results and
  successful owner bounce are historical evidence, not validation of this change
  on current main. Do not duplicate or port that fix in this investigation.

This proposal authorizes a diagnostic investigation only after a fresh Apply
request. A behavioral repair requires causal evidence, revised planning artifacts
and explicit owner approval. Keep all work local: no push, PR, installation,
archive or issue closure is part of this scope.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `transport-swell-playback`: retain bounded evidence of the first live timing
  rejection and its preceding admitted/run-start observations.
- `swell-workspace`: explain fixed-grid availability using its actual inputs and
  predicate, separately from playback readiness.

## Impact

Expected affected areas are `HostGrid`, `LiveTransportClock`, existing bounded
callback observation publication, editor diagnostics/text bindings and focused
grid/transport/processor/UI tests. Evidence belongs in `docs/validation/` and
living collection guidance in `docs/development/`. Reuse existing Diagnostics
text, shared controls and production typography; no new control family, token or
layout variant is proposed. No new dependency, state schema, persistent mapping,
asset rewrite, tolerance relaxation, hard-coded compensation or transport rewrite.
Tempo maps (#71), Song/Pattern detection (#1), plugin Export gains (#51), output
protection and clipping/source-edge DSP remain separate.
