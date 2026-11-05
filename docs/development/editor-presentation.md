# Editor presentation implementation — issue #58

This work is in flight on `codex/58-compact-responsive-ui`. The normal plugin
still uses the existing production presentation. The interim 900x600 layout is
owner-approved; final upstream integration and the combined native review are
pending. See the [canonical work/validation record](../validation/2026-11-05-issue-58-compact-ui.md).

## Preference boundary

`UI/PresentationPreferences.h` defines the UI-only record, injectable storage and
one per-editor preference session. It has no processor, audio, capture, render or
musical session dependency. It is not yet connected to editor Settings or its
lifecycle. Current desired content bounds are 900–2000 by 600–1400 logical units;
accepted scale values in this policy are 100 and 125 percent. No sub-100-percent
choice is enabled by the policy.

The version-1 JSON fields are `version`, `contentWidth`, `contentHeight` and
`scalePercent`. Validate integer fields independently, preserving valid fields
while replacing invalid ones with 900/600/100 defaults. Missing/corrupt text
recovers defaults; unsupported schema versions are not overwritten. Reads are
bounded to 16 KiB. `FilePresentationStorage::userFile()` locates
`%LOCALAPPDATA%/ReverbArise/presentation-v1.json`, with the JUCE user-application
data directory as fallback. The function does not create or write a file.

Each open editor loads once. Only explicit preference changes update its desired
values. `saveIfDue()` coalesces edits for 750 ms; a failed write retains the
in-memory value and pending patch for a later retry. Editor-close integration
must explicitly flush outside the audio callback. Another editor's save cannot
change an already-open editor; a subsequent opening loads the latest record.

Storage merges only changed fields against the latest disk record while holding
a non-waiting named interprocess lock. A busy writer causes a deferred retry.
Write a complete same-directory scratch file, flush/close it, then publish with
one native atomic replacement. Never truncate or remove the old target first.
An injected publisher permits failure checks before replacement. No filesystem
operations belong in the audio callback; disk work remains synchronous at this
boundary and must be invoked by the GUI/lifecycle owner with coalescing.

## Screen-fit boundary

`UI/PresentationFit.h` consumes a work-area extent and separately measured host
chrome, both already in host/desktop logical units. Windows/host scale must not
be multiplied again. User magnification applies once to content: 900x600 at
125 percent requires 1125x750 before external chrome.

Fit checks supply choice availability. Recovery first tries a smaller logical
workspace at the preferred magnification; if that cannot fit the baseline, try
100 percent. It never automatically shrinks text below 100 percent. If even
900x600 at 100 percent cannot fit, return an unsupported result for the editor
to expose through reachable recovery UI. The result is an effective temporary
presentation; it must never be persisted over the desired preference. Returning
to a larger display should resolve the original desired value again.

## Integration and verification still required

The standalone `preverb_presentation_policy_test` source covers field recovery,
coalescing, failed saves, explicit close/reopen, independent editor sessions,
atomic-publication faults, unsupported/oversized storage, concurrent processes,
work-area/chrome edges and temporary screen recovery. Native execution is queued
until #73 releases its quiet validation window; written tests are not a pass.

After policy verification, connect the content-root transform, shared Settings
controls, explicit size/scale actions and lifecycle persistence. Preserve active
entries, waveform zoom, popup anchoring and accepted musical state. Measure host
chrome and display changes, keep Settings/Diagnostics reachable, and prove
unsupported-combination feedback in the actual editor. These integration checks
remain separate from standalone policy coverage and the final merged native
900x600 acceptance gate. No production defaults or user preference files are
changed by adding these unused policy classes.
