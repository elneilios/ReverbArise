# Design

## Context

See proposal.md. Installed diagnostic evidence gives prepared sample 3399344,
PPQ 144, and failing export sample 3399344, PPQ 143.999322222222219 at 48 kHz,
122 BPM, 4/4: origin delta exactly 16 samples. This identifies the rejected
predicate, not the cause of FL Studio's different clocks or its stability.
Current offline admission and TransportPlayback compare the host to the source
origin. LiveTransportClock already separates fixed asset compatibility from a
runtime origin through LiveClockSnapshot::accepts.

## Goals / Non-Goals

**Goals:** repair constant-tempo offline scheduling using the tested runtime-clock
validator, retain strict drift detection and absolute sample audio, preserve
accepted IR handover, failures, routing, memory budgets, isolation and tails.

**Non-Goals:** #71 tempo-map evidence/persistence, Snap/live behavior changes,
hard-coded 16-sample correction, a global tolerance increase, output protection,
retiming assets or changing plugin IDs/schema.

## Decisions

- Keep one callback-owned LiveClockSnapshot for each offline run. Pin its
  reference at the first positive-frame callback of a new offline snapshot,
  independent of host Playing. Reuse its finite fixed-rate/BPM/meter asset gate
  and existing one-sample within-run origin check. No source mapping is rewritten.
- Apply the same clock to initial accepted-IR fallback selection, initial asset
  admission, subsequent wet-required callbacks and TransportPlayback::process.
  Passing only the outer gate would leave the inner mixer rejecting the audio.
- Retain existing run boundaries: endOffline, prepare/release and supported
  backward-position snapshot restart. Preserve sticky failure within a run;
  invalid/zero callbacks cannot establish useful admission. A later mapping
  reset after all wet intervals ended retains the existing tail exception.
- Keep publications' accepted asset timing metadata. The explicit validated
  runtime clock supplies the ephemeral admission authority to the mixer.
- Carry only diagnostic timing text and Copy diagnostics from the A/B work,
  excluding its output stage, latency, settings and test constructor seam.
  Reuse Controls/ActionButton in the existing Diagnostics row; no new token,
  control family, styling exception or production standards change.
- Reuse main's processor/export fixtures, adding offset-vs-zero sample equality,
  exact owner timing validation, drift rejection and per-instance offset cases.
  Verify byte-identical saved state/asset identities for ready renders. Run full
  Debug/Release wrappers because this changes audio admission, plus UI guard.

## Risks / Trade-offs

- A stable origin shift is not proof that a later edited tempo map preserves
  alignment. This remains the existing fixed-tempo sample-domain contract;
  genuine within-run changes fail, and #71 owns map revisions/precision.
- Missing one gate can produce silent output despite successful preparation.
  Compare actual nonzero positioned audio against independent original fixtures.
- Carrying diagnostic UI across branches can pull in experimental state.
  Inspect the final diff and confirm zero host latency/no protection controls.

## Migration Plan

Work from origin/main in the separate codex/fix-offline-export-clock worktree.
Record native results and host limits in dated evidence, install via the existing
test installer after FL Studio closes, retain the complete previous bundle and
verify inventory. Keep branch local; no push/PR. Owner checks only the previously
failing bounce and shares copied diagnostics if it still fails.
