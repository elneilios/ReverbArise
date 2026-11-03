# Spec Delta

## MODIFIED Requirements

### Requirement: Legacy manual processing remains available
Perform SHALL retain manual suggestions and supported explicit generation/refresh policy without resetting manual edits. Learn with Automatic Swells off SHALL admit dry capture but no automatic pass analysis/convolution, regardless of legacy refresh or forced-Recapture policy. Auto markers off in Perform SHALL preserve existing targets and its enabled render-refresh policy; no control SHALL silently enable a saved-off master.

#### Scenario: Manual mode with refresh disabled
- **WHEN** changed source reaches a Perform swell while automatic processing and legacy auto-refresh are disabled
- **THEN** it becomes stale and waits for an explicit generation/edit request instead of starting automatic convolution

#### Scenario: Automatic mode takes responsibility for refresh
- **WHEN** permitted automatic processing is enabled and source changes with the legacy manual auto-refresh switch off
- **THEN** affected eligible swells refresh through the automatic pipeline while unrelated swells remain unchanged

#### Scenario: Diagnostic suggestions remain distinct
- **WHEN** the user requests manual onset suggestions in an allowed mode
- **THEN** unconfirmed suggestions remain separate from accepted targets and cannot independently own active swells

#### Scenario: Dry-only Recapture does not force generation
- **WHEN** Learn commits changed source with Automatic Swells off and a legacy refresh/forced-work preference was enabled
- **THEN** source commits normally, changed unfrozen dependencies become honestly stale, and that preference does not start automatic pass convolution
- **AND** saved definitions/edits and valid frozen results remain preserved for permitted later manual use

### Requirement: Future swells inherit a valid bundled IR
The plugin SHALL include a redistributable valid default IR. New automatic swells SHALL receive an independent immutable IR identity from their owning accepted pass context; new manual Perform swells SHALL use the currently accepted future default. Later default edits SHALL NOT change existing assignments or queued pass intent. Missing/invalid defaults or assigned IRs SHALL report errors without substitution or false Ready states.

#### Scenario: Independent assignments
- **WHEN** two swells inherit their pass default and one receives a different WAV/WavPack
- **THEN** only that swell changes its assignment/dependency; another future-default change does not alter either existing assignment

#### Scenario: Default cannot be loaded
- **WHEN** the bundled or assigned default cannot be verified
- **THEN** dry capture and allowed editing stay usable, affected generation reports the error and no incomplete result is Ready

#### Scenario: Two passes finish generation out of order
- **WHEN** pass A captured with IR A is still being analysed after pass B records with IR B
- **THEN** newly discovered targets in A retain IR A and targets in B retain IR B regardless of completion order or the currently selected default

#### Scenario: Add a manual swell after choosing a new default
- **WHEN** the user explicitly adds a manual target in Perform after accepting a new future default
- **THEN** the new manual target uses that current default without reassigning existing or deferred automatic targets

### Requirement: Marker-control changes invalidate obsolete analysis
Analysis identity SHALL include detector policy, source dependencies and the applicable immutable pass or explicit reanalysis settings. Publication SHALL reject superseded requests without confusing later future-default edits with changes to an accepted pass. Rapid permitted Perform edits SHALL coalesce; failed/incomplete analysis SHALL NOT retire protected targets or erase manual work.

#### Scenario: Rapid sensitivity and enable changes
- **WHEN** sensitivity is dragged repeatedly in Perform and marking is disabled/re-enabled while a scan runs
- **THEN** only a complete permitted latest-request result publishes, work stays bounded and the UI/audio callback performs no PCM analysis

#### Scenario: Next-pass settings change during deferred generation
- **WHEN** an accepted earlier pass remains queued and the user changes allowed future defaults
- **THEN** its analysis retains the snapshotted settings rather than adopting or being silently relabelled with the next pass's settings

## ADDED Requirements

### Requirement: A learning pass owns immutable generation defaults
At capture admission the system SHALL freeze effective Automatic Swells policy, detector controls and the accepted Default IR identity/label for that pass. Direct commands and late completions SHALL NOT change them before Stop settlement. Automatic work SHALL use the accepted source/context pair, independent of later selected preferences.

#### Scenario: Settings race DAW Play
- **WHEN** a generation-setting edit and pass start occur concurrently
- **THEN** the edit belongs wholly to the frozen pass or to a later pass, never a mixture of partially updated controls

#### Scenario: Import intent is still pending
- **WHEN** a custom/default import has not published successfully before Play
- **THEN** the pass pins the prior accepted default and the pending import cannot silently change its sound after admission

#### Scenario: A new attack follows a context boundary
- **WHEN** a target's arrival belongs to one accepted pass but confirmation/extraction extends into another
- **THEN** its IR/default ownership follows the arrival's context, with normal full-source-coverage checks retained

### Requirement: Automatic Swells off provides dry-only learning
Learn with Automatic Swells off SHALL admit original dry source under independent Recapture rules, without admitting automatic analysis, new targets or convolution for that pass. Obsolete automatic publication SHALL be fenced/cancelled. Hidden preferences, existing definitions and immutable assets SHALL survive. Automatic entry into Perform SHALL NOT itself start disabled-pass work.

#### Scenario: All four capture-policy combinations
- **WHEN** Recapture and Automatic Swells independently take each on/off combination
- **THEN** only Recapture grants mapped replacement and only effective Automatic Swells grants automatic pass generation
- **AND** useful accepted dry can enable Perform in every combination without requiring a wet render

#### Scenario: Turn automatic generation off with older work pending
- **WHEN** the idle Learn user disables Automatic Swells before another pass while automatic work remains pending
- **THEN** no new automatic work is admitted under Off, obsolete publication is cancelled/fenced and accepted definitions/assets are not deleted

#### Scenario: Manual work after dry-only learning
- **WHEN** dry-only capture has committed and the user explicitly creates/edits a swell in Perform
- **THEN** normal permitted manual preparation works without silently enabling automatic source replacement or automatic discovery

#### Scenario: Learn preferences target future passes
- **WHEN** the user changes Learn defaults after a dry-only pass
- **THEN** those future settings do not implicitly backfill/reassign old pass work; explicit permitted Perform reanalysis remains a separate user action

### Requirement: Generation assignment preserves protected work
Pass-based discovery/reconciliation SHALL preserve stable matched identities, manual/edited/frozen IR choices, deletion suppression and accepted transition/gain edits. Additive capture SHALL NOT replace protected source-context assignments. New default or context metadata alone SHALL NOT change audio fingerprints or reconvolve existing unchanged dependencies.

#### Scenario: Revisit a manually overridden automatic swell
- **WHEN** a new pass/default or permitted reanalysis revisits a manually edited or frozen target
- **THEN** its IR/edits/identity stay protected while genuinely new eligible targets receive their own accepted context

#### Scenario: Context-only update with identical audio
- **WHEN** a context metadata update is accepted while PCM and existing swell dependencies remain identical
- **THEN** existing renders/calibration are reused and only newly eligible generation resolves the new provenance
