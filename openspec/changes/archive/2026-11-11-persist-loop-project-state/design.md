# Design

## Context

`PluginProcessor.cpp` already writes independent AcceptedLoopSource and NextCapture records in its host save and portable snapshot paths. Capture manifest schema 3 holds the Loop extent and capture-local timing. Associated state carries the manifest in its existing envelope. Restore already clears several capture controls and invalidates the Loop clock. The portability backend already supports exact dependency verification, Locate, Restore and explicit incomplete recovery. Existing processor and portable-recovery fixtures provide entry points for expanding coverage.

## Goals / Non-Goals

- Use the current records and existing Storage controls for fresh Rack and FL Studio projects. Preserve the accepted audio and identity through host save, restore and verified folder changes.
- Keep durable source and creative metadata separate from in-flight capture and live timing. Preserve existing Song behavior and plug-in identity.
- Legacy migration, historical session fixtures, a new storage format, and new recovery controls are outside this plan. The existing Song-oriented Re-learning flow stays outside this change.

## Decisions

1. **Use the accepted manifest as the source authority.** Audit both save paths and the load paths against the same kind, reference, revision, exclusive bounds and grid. Keep native PCM in managed assets. A new Loop-specific project payload would duplicate the asset identity already used for edit and render validation.
2. **Invalidate transient work at host restore and recovery adoption.** Reuse the capture generation/publication fence and Loop transport reset. Verify that late capture and render work from the prior generation is rejected and that a restored or held control cannot resume an action. Live Cue uses fresh host timing. Editor recreation alone should leave the active processor session intact.
3. **Reuse verified Storage association.** Inspect `pinPortableAssets`, `useWorkingFolder` and `reloadRecoveredAssets` for the Loop manifest and accepted identity at each handoff. Reuse snapshot verification and independent writer ownership. Report unresolved dry/render dependencies with the existing recovery surfaces. Automatically choosing a similarly named folder or a later manifest would risk binding edits to different audio.
4. **Verify the host round trips separately from native fixtures.** Use a new project in each host with a useful accepted Loop, an edited swell and known timing. Save, reopen, locate a moved folder and restore a verified backup using disposable project copies and storage destinations. Compare manifest identity, exact extent and asset checksums. Verify disarmed Learn, dry Standby and fresh Cue in both hosts. Use native fixtures for deterministic interruption, stale publication, held controls, multi-instance isolation and suspend/bypass cases.

## Risks / Trade-offs

- **Save during replacement** → pair the accepted source and creative revision under the existing snapshot fence and prove that a pending take does not leak into a saved project.
- **Invalid or incomplete recovery** → verify dependency identity before adoption, retain surviving state, and avoid touching original assets during manual checks.
- **Confusing captured timing with current sync** → assert that the local grid is usable after reopen even when wet playback is still waiting for a live reference.
- **Recovery work affects audio callbacks** → keep verification and file work on existing non-realtime paths and preserve bounded callback handoff.
