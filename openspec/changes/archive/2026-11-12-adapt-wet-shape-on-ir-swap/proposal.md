# Proposal

## Why

In Perform, a shorter impulse response can render successfully yet be rejected because the current swell's explicit pre-roll extends beyond the new response. Routine IR browsing should preserve the user's swell and adapt the shape automatically, with actionable feedback when the requested sound genuinely cannot be prepared. [GitHub issue #183](https://github.com/elneilios/ReverbArise/issues/183)

## What Changes

- Fit the effective wet bounds and fades to the new response while preserving compatible shape edits, curve intent, gain, placement, arrival, source and activity settings.
- Recover the original shape where it fits when switching back to a longer response.
- Keep eligible accepted audio available through preparation. Publish the latest valid replacement as a coherent IR, render and effective shape for playback, editor, session state and export.
- Hand over active manual Audition at its current listening position when the replacement is ready.
- Show when shape timing was fitted. For genuine failures, identify the failed request, what remains available and a practical next step.
- Verify the reported bar 3.1 case and an independent comparison swell in the existing FL Studio project, using a working copy for edits.

## Capabilities

### New Capabilities

### Modified Capabilities

- `swell-editing` covers fitting and recovering shape intent.
- `swell-workspace` covers effective handle feedback and actionable assignment status.
- `swell-session` covers accepted assignment persistence and backward-compatible shape intent.
- `transport-swell-playback` covers retaining and handing over coherent wet audio when an IR swap also fits the shape.
- `reverse-convolution` covers preview validity while a replacement is pending and when it takes effect.

## Impact

Per-swell bundled, custom and managed IR assignment, background render and headroom preparation, Perform bindings, session encoding, audition and export consumers. Existing freeze and source validation policies, convolution behavior, host compatibility and real-time budgets continue to apply. Undo and redo are deferred to a separate product-wide history design.
