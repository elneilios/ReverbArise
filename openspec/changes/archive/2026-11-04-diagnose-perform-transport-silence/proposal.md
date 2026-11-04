# Proposal

## Why

GitHub issue: [#53](https://github.com/elneilios/ReverbArise/issues/53). Some preserved FL Studio instances produce no Perform wet output during Song playback although the same accepted swell plays in Audition and Export. Owner diagnostics now establish a stable approximately 16-sample origin difference during Play versus Stop that makes both live admission and preparation reject otherwise ready assets.

## What Changes

- Expose bounded, read-only developer evidence for callback admission, published/source mapping, exact rejection reasons, preparation revision/window and retry outcome in the existing Diagnostics surface.
- Preserve the completed causal evidence and native replay: the recorded playing clock differs from the parked/source clock, while rate, tempo, meter, live configuration and callback range remain valid.
- Separate live transport clock validation from stored absolute source/render coordinates. Establish a runtime mapping for each valid playing run and keep the existing one-sample origin check within that run; a different parked/source origin alone must not suppress live wet.
- Revalidate worker preparation/publication against the current live clock and dependencies, retry transient failures, and join current Song time without moving saved audio or replaying elapsed lead-in.
- Add lifecycle, retry, position, source/render consistency and callback-safety regressions; retain independent Audition and Export behavior.
- Preserve saved projects, source and managed assets, persistent identities and capture exclusion in Perform. Keep #46 composition and unfinished worktree untouched.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `transport-swell-playback`: distinguish live admission failure reasons with coherent developer observations and define independent ephemeral playing-clock validation, retain absolute asset coordinates and explicit incompatibility within a run, and recover from transient preparation/publication rejection safely.

## Impact

Expected areas are `Source/Core/TransportTime.h`, `Source/Playback/TransportPlayback.*`, `Source/DSP/SwellTransport.cpp`, SwellEngine snapshot/state and existing processor/editor diagnostic binding. The live callback and worker must share coherent runtime clock identity; strict shared mapping helpers and Capture, Audition, Export and offline timing behavior remain unchanged. Existing retained-IR eligibility must use the same live asset/clock separation. Extend transport, engine, processor and diagnostic component/binding tests and living technical guidance; record dated validation evidence. No new dependencies, persisted format, VST3 identity migration or new control family is proposed. Existing diagnostic text roles and shared controls are reused; any required visual exception or broader architectural change returns for owner review before rollout. Implementation and one brief final FL Studio acceptance remain separate from this planning phase.

## Owner decision

On 4 November 2026 the owner approved revising these planning artifacts to validate the playing clock separately while preserving all saved sample positions. This supersedes the initial exclusion of a live mapping-contract change; it does not authorize a global tolerance increase, a hard-coded 16-sample correction or a persistent migration. The native replay and recording are linked in the dated issue #53 evidence.
