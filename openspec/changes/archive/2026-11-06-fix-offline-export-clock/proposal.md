# Proposal

## Why

FL Studio rejects a constant-tempo DAW bounce because its export PPQ implies an origin exactly 16 samples different from the source's parked mapping. The prepared samples remain at the same absolute Song positions. The existing offline guard conflates source compatibility with export-run clock compatibility and omits the swells.

## What Changes

- Establish a per-bounce fixed-tempo host clock independently of the parked/source origin, retaining the existing one-sample tolerance within that bounce.
- Preserve absolute sample scheduling, prepared asset compatibility, real clock-change rejection, tails, routing and instance isolation.
- Retain precise offline failure timing and the owner-requested Copy diagnostics control without experimental output protection.
- Add regression coverage for the exact reported clock tuple, offset bounces, real drift, resets/seeks, block partitions, instance isolation and unchanged assets.
- Build and install a local test bundle from this dedicated main-based worktree.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `transport-swell-playback`: validate the offline run's own clock without moving accepted samples; retain failures and share diagnostics.

## Impact

SwellTransport offline preparation, per-instance ephemeral clock state, focused processor/unit tests, existing Diagnostics ActionButton binding, technical guidance and validation evidence. No plugin identity/schema change, output-protection stage, tempo-map implementation (#71), global tolerance relaxation or hard-coded offset correction. Keep this branch local.
