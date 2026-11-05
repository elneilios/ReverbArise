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

Readers and writers both acquire a non-waiting transaction lock; contention is
reported as `busy`, not as a missing profile. An initial busy read may be retried
until the first successful read or explicit user intent. This is initial restore
only, not live synchronization between editors. Explicit size/scale intent wins
over a delayed initial read, including explicitly selecting the default values.

Storage merges only changed fields against the latest disk record while holding
a non-waiting named interprocess lock. A busy writer causes a deferred retry.
Write a complete same-directory scratch file, flush/close it, then publish with
one native atomic replacement. Never truncate or remove the old target first.
An injected publisher permits failure checks before replacement. No filesystem
operations belong in the audio callback; disk work remains synchronous at this
boundary and must be invoked by the GUI/lifecycle owner with coalescing.

The per-storage thread gate also excludes overlapping calls within the same
process. On Windows, readers must join the transaction because native file
replacement can briefly make an uncoordinated pathname lookup unavailable.

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

`UI/EditorPresentationState.h` connects the per-editor desired preference to the
fit policy without depending on components or musical state. It supplies fitting
size/scale availability, explicit choice handling, physical-content-to-logical
manual resize conversion, initial-read retry and temporary display recovery.
Display changes resolve effective geometry without persistence. The component
owner must apply the effective state to its scaled content root and call
coalesced persistence on the GUI/lifecycle path; those real editor bindings are
still pending.

The standalone `preverb_presentation_policy_test` source covers field recovery,
coalescing, failed saves, explicit close/reopen, independent editor sessions,
atomic-publication faults, unsupported/oversized storage, concurrent processes,
work-area/chrome edges and temporary screen recovery. Native Debug and Release
fixture checks now pass, including repeated Release concurrency runs; exact
results, the initial reader race and its correction are in the linked validation
record. These are standalone policy checks, not full plugin or physical DPI
acceptance.

After policy verification, connect the content-root transform, shared Settings
controls, explicit size/scale actions and lifecycle persistence. Preserve active
entries, waveform zoom, popup anchoring and accepted musical state. Measure host
chrome and display changes, keep Settings/Diagnostics reachable, and prove
unsupported-combination feedback in the actual editor. These integration checks
remain separate from standalone policy coverage and the final merged native
900x600 acceptance gate. No production defaults or user preference files are
changed by adding these unused policy classes.
