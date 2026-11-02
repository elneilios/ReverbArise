# Current UI access and acceptance

The header gear opens Settings. Its current grouped layout is accepted for this
iteration; a later Settings polish pass is a follow-up, not a reopened redesign.
Both sensitivity knobs remain global. Source/target/shape/placement, Enabled,
Solo and Freeze remain in Selected Swell. The current default is 1100x850.

## Diagnostics and action dispositions

| Original item | Current location and scope | Verification |
| --- | --- | --- |
| Sample/PPQ/tempo/meter/rate/block | Recovery > Open diagnostics > Timing; host footer is informational | Missing/stopped/seek UI fixtures, transport-time tests |
| Capture status/revision/drops/discontinuities | Diagnostics > Capture/assets, read-only | Capture/persistence/UI lifecycle tests |
| IR/source/raw bounds/progress/peaks | Diagnostics > Selection/render; bound to current ID | Engine/asset/UI identity and error fixtures |
| Exact source/wet positions | Diagnostics copyable read-only precision table; editable pairs in inspector | Sample/pixel, integer overflow and edit ownership tests |
| Live/export readiness, guards, block count | Diagnostics > Playback/export; important warnings also main status | Processor/transport/multiple-export tests |
| Marker/regeneration policy | One authoritative global knob each; diagnostic readout only | Independent-instance/restore tests |
| Duplicate sensitivity and mode buttons | Removed, including callbacks and hidden refresh | Source reference audit; continuous percentage tests |
| Duplicate gain/fade/tension/placement editors | Removed as writable diagnostic surfaces; inspector owns edits | Model/inspector round trips and deletion/selection tests |
| Unused fieldBounds, hidden curve labels, title override | Removed; supplied wordmark and actual curve presets retained | Reference audit and native geometry fixtures |
| Live on/off/routing/wet level | Settings > Playback | Independent live/preview tests |
| Mapping/master automatic/legacy auto-refresh/separation/default IR | Settings > Processing; serialized preferences retained | Automatic coordinator/migration/engine tests |
| Mute, curve presets, source-bar convenience, Full response | Settings > Swell | Shape/source and UI binding tests |
| Rename/Delete | Swell row menu; Delete also under Swell settings | Stable ID, scrolled-popup geometry; menu position confirmed by user in FL Studio |
| Preview wet/dry isolation, tail, trim | Settings > Audition; current trim and reset remain in band | Audition mix/voice/monitoring/UI tests |
| Regenerate selected/all, Suggest/candidate/Confirm | Settings > Recovery, explicit actions only | Analysis/engine/UI tests |
| Recapture/Cancel/range status | Settings > Recovery, explicit fresh coverage | Scoped recapture/processor tests |
| Clear map/deletion suppression reset | Settings > Recovery; no implicit window-lifecycle action | Capture and suppression tests |
| Asset path copy | Diagnostics > Copy asset info, clipboard only on click | Asset/engine fixtures and explicit callback audit |
| Scheduled 440 Hz tone and range | Settings > Recovery; armed indication and Off path in header | Scheduled-tone and processor silent-output/loop/off/state tests |

The user approved retained diagnostic UI on 2026-11-02. Close hides/reuses the
editor-owned content with no diagnostic refresh work; reopen immediately refreshes
current snapshots. Content/callbacks/windows are disposed on editor destruction.
Design decision 8 and its lifecycle scenario now reflect this approved policy.

Diagnostics formats only while visible, at most 5 Hz, and refreshes immediately
on reopen. Its precision observations are read-only; recovery remains explicit.
The editor owns the windows and destroys them before its LookAndFeel. Open,
Close, refresh and editor destruction do not start audition/tone, clear capture,
change preferences or queue rendering. Active callback/capture teardown is tested
in Debug and Release. No host parameters, state schema or managed asset migration
is introduced by this UI change.

## User acceptance for this build

The user approved all three groups below on 2026-11-02 with "all approved".
This is explicit user-reported acceptance of this build; it is separate from
automated native evidence and prior milestone results. No failures or scale
exceptions were reported. Detailed hardware measurements were not supplied.

Final native checkpoint C was approved on 2026-11-02: loaded UI, empty UI and
Diagnostics are accepted for FL Studio validation. Compact size/brightness and
row menu position retain their earlier individual approvals. Approved checks:

1. FL Studio editing: both sensitivities, different IRs including WavPack,
   Freeze/Solo, all knob/typed/canvas pairs, shaded source range, Reset wet and
   Reset All, naming, Overview navigation and Settings/Diagnostics lifecycle.
2. FL Studio playback/state: audition mix/isolation/tail/cursor; independent
   live routing/level; seek, looping/final swells, silent-input output;
   two-instance WAV export and save/reopen after edits.
3. Physical Windows/host scale: 100%, 125%, 150%, 200% where the monitor work
   area permits. Record actual host/audio settings and limitations; software
   scaled PNGs do not substitute for physical DPI acceptance.

Release/Debug binary identities, native image paths and actual test results
and the user acceptance record are in [the evidence log](ui-alignment-evidence.md).
All 45 tasks are checked; sync/archive remains a separate workflow action.
