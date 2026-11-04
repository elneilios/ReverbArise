# Proposal

## Why

GitHub issue: [#53](https://github.com/elneilios/ReverbArise/issues/53). Some preserved FL Studio instances produce no Perform wet output during Song playback although the same accepted swell plays in Audition and Export. Existing diagnostics conflate incompatible mapping and preparation, preventing a proven, narrow repair of this release-blocking failure.

## What Changes

- Expose bounded, read-only developer evidence for callback admission, published/source mapping, exact rejection reasons, preparation revision/window and retry outcome in the existing Diagnostics surface.
- Establish the failing gate with preserved-instance observations or a deterministic injected fixture before selecting a behavioral fix; do not presume origin drift or relax timing checks without evidence.
- Repair the demonstrated defect and make recoverable preparation/publication failures retry safely at current Song time while unsupported timing remains explicitly unavailable.
- Add lifecycle, retry, position, source/render consistency and callback-safety regressions; retain independent Audition and Export behavior.
- Preserve saved projects, source and managed assets, persistent identities and capture exclusion in Perform. Keep #46 composition and unfinished worktree untouched.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `transport-swell-playback`: distinguish live admission failure reasons with coherent developer observations and require safe recovery from transient preparation/publication rejection without replaying elapsed wet or admitting incompatible assets.

## Impact

Expected areas are `Source/Core/TransportTime.h`, `Source/Playback/TransportPlayback.*`, `Source/DSP/SwellTransport.cpp`, SwellEngine snapshot/state and existing processor/editor diagnostic binding. The actual repair location depends on evidence. Extend transport, engine, processor and diagnostic component/binding tests and living technical guidance; record dated validation evidence. No new dependencies, persisted format, VST3 identity migration or new control family is proposed. Existing diagnostic text roles and shared controls are reused; any required visual exception or broader architectural change returns for owner review before rollout. Implementation and one brief final FL Studio acceptance remain separate from this planning phase.
