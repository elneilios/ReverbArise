## ADDED Requirements

### Requirement: Perform audition loops its finite window
Manual Perform audition SHALL repeat its prepared selected listening window until
Stop or existing selection/session/dependency/timing/workflow/DAW-priority guards
stop playback. Loops SHALL retain boundary ramps, sample alignment and bounded
allocation/lock/I/O-free audio processing, without republishing or accumulating
buffer ownership on each repeat. Restore SHALL NOT start playback. This SHALL NOT
extend the window or implement whole-song arrangement audition.

#### Scenario: Repeat and stop
- **WHEN** manual audition crosses the prepared window end
- **THEN** playback resumes at its start, including multiple wraps within one block and tiny buffers, with bounded position and unchanged stored audio
- **AND WHEN** Stop or an incompatible dependency interrupts playback
- **THEN** existing smooth-stop or immediate-invalidation behavior terminates it and releases buffers outside the callback

### Requirement: Perform audition exposes a Wet only toggle
Perform SHALL replace its blend slider and row Trim reset with a latching Wet only
button. New instances SHALL default Off: unity captured dry plus saved Live Wet
gain. On SHALL suppress captured dry and retain that wet gain. Existing wet/sum
protection and replay boundary ramps SHALL remain disclosed. Export controls and
live routing SHALL remain independent. Status SHALL truthfully identify Ready,
Looping, Preparing or Unavailable, with full reasons available without shrinking text.

#### Scenario: Toggle live-level listening
- **WHEN** Wet only is toggled with an unchanged valid invocation
- **THEN** accepted pressed state, prepared sound and status agree without autoplay, convolution, asset edits or Export mix changes
- **AND** stale/disabled invocations cannot change a different session or workflow

#### Scenario: Restore old and new preferences
- **WHEN** legacy mode/blend/trim or new live-level monitoring preferences restore
- **THEN** their saved behavior is retained without playback; legacy switches to live-level behavior only after explicit toggle
- **AND** missing optional metadata defaults to legacy behavior, malformed metadata is handled transactionally and current round trips preserve mode/identity

#### Scenario: Monitoring protection and unavailable source
- **WHEN** live-level monitoring needs protection or requested captured dry is unavailable
- **THEN** attenuation is disclosed, and unavailable dry is not padded or silently replaced by Wet only
- **AND** saved legacy trim remains available in Settings for legacy monitoring, while live-level mode ignores it explicitly

## REMOVED Requirements

### Requirement: Audition blend has explicit endpoints
**Reason**: The owner replaces continuous audition percentage with Wet only and live-level nominal balance. Saved legacy values remain compatible.
**Migration**: Restore legacy mode/blend/trim until explicit toggle. New instances use live-level Dry + wet. Export retains its independent continuous mix.

### Requirement: Audition percentage and geometry agree
**Reason**: The audition percentage control is removed.
**Migration**: Use the approved latching action family for accepted Wet only state; preserve existing preview/DAW-priority guards.

## MODIFIED Requirements

### Requirement: Monitoring protection and trim remain transparent
The aggregate wet branch SHALL follow the saved gain policy: legacy -6 dBFS protection without upward normalization, or the shared musical calibration/protection contract. The prepared audition sum SHALL receive additional attenuation only when needed to stay within that policy's ceiling before trim. Neither guard SHALL alter saved renders. Legacy monitoring SHALL apply saved -60 to 0 dB trim after the sum, defaulting to 0 dB when absent, visible/resettable in Settings. Live-level monitoring SHALL retain but ignore stored legacy trim and use Live Wet gain before wet protection, unity captured dry in mixed mode and disclosed combined-sum protection.

#### Scenario: Hot correlated blend
- **WHEN** protected wet and captured dry combine above the audition ceiling
- **THEN** only the manual audition contribution is attenuated to the saved policy's ceiling before trim, and Diagnostics identifies branch and blend attenuation separately

#### Scenario: Existing low trim
- **WHEN** a saved legacy session contains -24 dB trim
- **THEN** it restores unchanged with a visible Settings trim value and a double-click reset-to-0 action, without auto-starting audition

#### Scenario: Level rises below protection onset
- **WHEN** musical swell Level increases by 6 dB during a unity-trim Wet Only comparison with sufficient headroom
- **THEN** its prepared wet amplitude increases accordingly without a compensating hidden guard change
- **AND** if the actual ceiling is reached, requested gain and protection attenuation remain separately visible

### Requirement: Group monitoring preserves dry and relative wet levels
Overlapping wet SHALL retain each member's saved fixed or source-matched baseline, user gain/envelope and positioned offsets before shared aggregate protection. Captured dry SHALL be mixed once. Legacy Dry=1-wet and wet=wet blend SHALL remain compatible. Live-level monitoring SHALL use dry=1 and wet=Live Wet gain (or dry=0 for Wet only). Policy ceiling, combined-sum protection and legacy trim SHALL remain separate from transition editing. No additional per-member normalization beyond the saved calibration or dry duplication SHALL occur.

#### Scenario: Correlated overlapping wet and one dry branch
- **WHEN** two hot wet contributions overlap with captured dry at legacy 0%, 50% and 100% Wet, or live-level Dry + wet/Wet only
- **THEN** a sample oracle matches the aligned wet sum and single dry branch with disclosed policy-specific aggregate/sum attenuation, raw assets and live dry remain unchanged, and relative member gains are preserved

#### Scenario: Audition and export compare equal membership
- **WHEN** the same musical-policy membership is monitored at 100% Wet and exported Wet Only with corresponding unity output controls
- **THEN** the wet gain plan and interior samples agree within declared tolerance, with monitoring boundary ramps identified separately
- **AND** switching isolation or blend does not rewrite the musical baseline or saved live preferences

#### Scenario: Matching stays fixed across audition scope
- **WHEN** a source-matched swell is auditioned with different isolation, blend, listening-tail or trim settings
- **THEN** its measured baseline is reused without remeasuring against the audition dry window or counteracting envelope edits
- **AND** only the explicit monitoring factors and aggregate protection context change

#### Scenario: Energy calibration does not follow monitoring loudness
- **WHEN** an energy-calibrated swell is monitored Wet Only or Dry + Wet with changed blend, trim, isolation or tail
- **THEN** its accepted dry/default-wet RMS measurement and baseline remain fixed, Level remains relative to that baseline, and only the explicit monitoring/protection factors change

### Requirement: Manual playback boundaries ramp safely
Manual replay SHALL use disclosed 5 ms outer start/end ramps and smooth Stop, shortened for tiny buffers and reaching zero at boundary samples without changing interval length. Loop wraps SHALL restart the same finite window without callback allocation or buffer republishing. Explicit Stop and existing invalidation/priority guards SHALL terminate looping. Replacement SHALL preserve sequential cursor and smooth handoff. Live dry and stored PCM SHALL remain unchanged; unsafe invalidation MAY silence immediately. The callback SHALL remain allocation/lock/file-I/O free with retirement outside it.

#### Scenario: Natural completion and explicit stop
- **WHEN** audition starts, reaches a nonzero final stored sample, or receives Stop and is replayed
- **THEN** short monitoring ramps avoid an abrupt boundary step, replay restarts at the prepared window start, and live dry remains untouched; the outer ramp applies only to replay contribution (including recorded dry at window edges), with its interior unchanged

#### Scenario: Very short buffer and restoration
- **WHEN** an audition interval is shorter than the nominal ramps or an old zero-fade session reopens
- **THEN** ramp arithmetic stays finite/bounded, saved edits retain their exact values, boundary protection is disclosed separately and playback never starts on restore

