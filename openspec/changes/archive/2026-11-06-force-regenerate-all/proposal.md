# Proposal

## Why

Regenerate All silently skips Ready swells using current source-edge processing, while Regenerate Selected forces a fresh render. The owner reports that All appears broken and explicitly wants it to force regeneration.

## What Changes

- Make the explicit Regenerate All action request fresh renders for all eligible confirmed swells, including current Ready, disabled and muted definitions.
- Retain frozen-swell, IR-import, workflow and stale-publication safeguards. Preserve Regenerate Selected and automatic dependency-driven regeneration.
- Give visible, accessible feedback for accepted queued work and for requests that queue nothing, including skipped/blocked reasons. Do not claim successful rendering before jobs finish.
- Keep saved-project restore passive and retain prior complete assets on failed or superseded work.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-regeneration`: Explicit All forces fresh eligible renders and reports its outcome instead of acting as a silent update-outdated operation.

## Impact

Affected paths are the existing engine regeneration command, its Recovery/Diagnostics UI binding and helper feedback, source-edge/batch/component tests, source-edge guidance and offline guide currency. Reuse the current render queue, working-memory bound, shared buttons and helper roles. No DSP algorithm, source recapture, asset migration, new force toggle, automatic bulk regeneration or change to frozen ownership is proposed.

This is a separate follow-up to merged #58, on `codex/force-regenerate-all`. The current source-edge regression passes and explicitly asserts the old skip-current behavior; it must be revised together with the contract.
