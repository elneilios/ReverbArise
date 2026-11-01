# automatic-swell-processing Specification

## Purpose
Progressively turn committed dry Song captures into stable editable swells and verified wet renders without a manual capture, analysis or generation sequence.

## Requirements

### Requirement: Legacy manual processing remains available
Automatic processing SHALL govern dependent refresh and, while Auto markers is on, target acceptance. With the master disabled, manual suggestions SHALL remain unconfirmed and the saved optional auto-refresh policy SHALL apply. Auto markers off SHALL preserve existing targets and the enabled render-refresh policy. These controls SHALL NOT reset saved manual edits or implicitly enable an explicitly disabled master.

#### Scenario: Manual mode with refresh disabled
- **WHEN** ordinary recapture changes a swell while automatic processing and legacy auto-refresh are both disabled
- **THEN** the result becomes stale and waits for an explicit regeneration request

#### Scenario: Automatic mode takes responsibility for refresh
- **WHEN** automatic processing is enabled and source changes with the legacy manual-mode auto-refresh switch off
- **THEN** affected swells still refresh through the automatic pipeline while unrelated swells remain unchanged

#### Scenario: Diagnostic suggestions remain distinct
- **WHEN** the user requests manual onset suggestions
- **THEN** unconfirmed suggestions remain separate from accepted automatic or manual targets and cannot independently own active swells

### Requirement: New instances process captured Song audio automatically
New instances SHALL enable progressive mapping, automatic processing and Auto markers without a plugin action. Mapping SHALL capture only dry input while transport plays with usable absolute position and compatible timing. Unsupported position or rate SHALL pause mapping with an explanation. Ready swells SHALL not start playing automatically in this change.

#### Scenario: Fresh insert and playback
- **WHEN** a new instance receives musical input during supported Song playback
- **THEN** committed coverage, detected targets and complete wet renders appear without pressing Capture, Suggest or Generate, while generated swells remain silent until manually auditioned

#### Scenario: Sparse capture and unsupported position
- **WHEN** playback visits bars 86–90 then 1–4, or usable host position becomes unavailable
- **THEN** visited ranges remain at their absolute positions with an unmapped gap, and unavailable position pauses capture instead of inventing coordinates

### Requirement: Future swells inherit a valid bundled IR
The plugin SHALL include a redistributable valid default IR and assign an independent immutable IR identity to each new swell. Changing a future-swell default SHALL not alter existing assignments. Missing or invalid default and assigned IRs SHALL produce actionable errors without substitution or false Ready states.

#### Scenario: Independent assignments
- **WHEN** two swells inherit the default and the user assigns a different WAV or WavPack IR to one
- **THEN** only that swell changes its IR and render dependency, while a later default change affects only future swells

#### Scenario: Default cannot be loaded
- **WHEN** the bundled default fails validation
- **THEN** capture and editing remain usable, new rendering reports the default-IR error, and no incomplete result is Ready

### Requirement: Automatic targets have stable protected identities
The system SHALL distinguish suggestions, accepted automatic targets and manually created or edited targets. Repeated detection SHALL reconcile one-to-one within a documented sample tolerance, preserve matched IDs and selection, and avoid duplicate targets. Manual edits SHALL not be moved or deleted by detection; deleted automatic targets SHALL remain suppressed until explicitly reset.

#### Scenario: Repeated and boundary detection
- **WHEN** unchanged audio is revisited or a transient straddles capture commits
- **THEN** it retains one accepted target and swell identity rather than creating another at the boundary

#### Scenario: Manual intervention
- **WHEN** the user moves, edits or deletes an automatic target and its area is analyzed again
- **THEN** the edit and selected ID remain stable, and a deleted target does not immediately reappear

#### Scenario: Changed unedited target
- **WHEN** committed audio changes and an unedited automatic target is no longer supported by the detector
- **THEN** only that affected unprotected target is reconciled or retired, with obsolete jobs cancelled and unrelated targets preserved

### Requirement: Automatic acceptance distinguishes attacks from event decay
Default detection SHALL avoid accepting multiple swell targets from the decay, sustained modulation or release of one isolated event. Acceptance SHALL consider attack confidence as well as sensitivity and separation, while preserving distinct real attacks at supported spacing. Incremental and whole-run analysis SHALL converge without shifting arrivals to the later confirmation time.

#### Scenario: One decaying event
- **WHEN** an isolated pluck or note with a long, modulated or noisy decay is captured using default settings
- **THEN** its attack produces one accepted automatic target and its decay/release produces no additional accepted targets in the documented regression fixtures

#### Scenario: Genuine repeated and soft attacks
- **WHEN** soft attacks and distinct repeated notes at the supported minimum spacing are captured
- **THEN** eligible attacks remain detectable within the documented onset tolerance instead of being merged by an enlarged blind suppression interval

#### Scenario: Attack crosses a capture boundary
- **WHEN** an attack or its required confirmation context spans multiple commits or ends a partial pass
- **THEN** the system waits for sufficient real context, accepts at most one target at its onset position, and matches the equivalent whole-run result

#### Scenario: Refined policy revisits existing targets
- **WHEN** the detector policy or explicit detection settings change and valid reanalysis completes
- **THEN** obsolete analysis summaries are invalidated, unedited automatic false positives can be reconciled, and protected edits, suppression and stable matched identities remain intact without silently resetting saved sensitivity

### Requirement: Automatic source windows wait for complete coverage
Each target SHALL retain a defined extraction interval containing its arrival and wait for full coverage and a valid IR. Growing coverage or gaps SHALL NOT shorten it. An untouched automatic tail MAY fit an explicit loop end or observed completed backward pass as defined by transport-swell-playback, preserving manual edits. Unmapped input SHALL NOT become silence. Wet pre-roll SHALL NOT require captured dry pre-roll.

#### Scenario: Partial tail followed by capture
- **WHEN** an onset arrives before the remainder of its extraction window has been committed
- **THEN** its state is Waiting for source until coverage completes, then it queues automatically using the full interval

#### Scenario: Source gap and leading wet
- **WHEN** a source window crosses an uncaptured gap, or a complete source starts at sample zero
- **THEN** the gap blocks generation, while the complete source can produce negative-time wet lead-in without requiring negative-time capture

### Requirement: Dependency changes schedule bounded minimum work
Source changes and render-affecting edits SHALL refresh only dependent swells. Identical recapture SHALL preserve IDs/renders. Gain, activity, audition scope and pure wet placement SHALL not reconvolve; independent crop extension MAY require genuine extra input. Translation SHALL preserve extraction and detector anchors. Rapid requests SHALL coalesce, obsolete results SHALL NOT become current, and resource pressure SHALL report waiting/failure without truncation.

#### Scenario: Changed and identical recapture
- **WHEN** one swell's source changes and another source is recaptured identically
- **THEN** only the changed dependency becomes stale and reconvolves; unrelated and identical dependencies retain their renders

#### Scenario: Rapid edits and failure
- **WHEN** an IR or source field is edited repeatedly while rendering, or a job exceeds available resources
- **THEN** only the latest valid dependency can publish Ready, the queue remains bounded, and a failure retains edits with an explicit state

### Requirement: Processing states and restoration remain truthful
The system SHALL expose Waiting for source, Queued, Rendering, Ready, Stale/updating, Missing IR and Failed as applicable. It SHALL persist provenance, suppression, independent IR references and automation preferences. Reopen SHALL stop audition, validate assets and fresh host timing, and resume permitted mapping only during subsequent valid playback. Captured content SHALL be described as last captured.

#### Scenario: Reopen edited automatic session
- **WHEN** a session with manual edits, suppressed targets and different per-swell IRs is reopened
- **THEN** identities and edits survive, verified results can be reused, missing assets remain explicit, and no wet audition starts

#### Scenario: Upstream edits while stopped
- **WHEN** source audio is changed upstream without new playback through the plugin
- **THEN** the plugin does not claim to have detected that change and updates dependencies only after the affected source is recaptured

#### Scenario: Disable automation
- **WHEN** automatic processing or mapping is disabled in Diagnostics and the project is saved and reopened
- **THEN** the explicit preference is retained without deleting captured assets or manual swell definitions

### Requirement: Automatic creation applies transition defaults once
First creation of an automatic target SHALL use the shared new-swell 20 ms wet overlap/fade-out defaults with zero wet placement. Reconciliation and dependency refresh SHALL preserve previously stored transition/placement edits and manual ownership. A default endpoint beyond the base selection SHALL follow genuine support admission rather than shorten, pad or falsely declare Ready.

#### Scenario: First target and subsequent reanalysis
- **WHEN** a new automatic target is created and later analyzed or regenerated again
- **THEN** the initial transition defaults apply once, later edits remain intact and stable IDs/suppression are preserved

#### Scenario: Explicitly move an automatic swell
- **WHEN** the user translates an automatic swell's wet body
- **THEN** it becomes protected under manual-edit ownership, its original detector anchor and dry/source coordinates remain fixed, and subsequent detection does not undo placement or create a replacement at its old anchor

### Requirement: Marker sensitivity controls event selectivity
One canonical per-instance sensitivity SHALL govern automatic and diagnostic onset analysis, with lower values favouring stronger attacks and rejecting more small events in documented busy fixtures. It SHALL preserve saved sensitivity and use bounded coalesced background reanalysis when marking is enabled. It SHALL NOT depend solely on a longer blind separation or claim musical-key/chord recognition.

#### Scenario: Reduce minor markers in busy source
- **WHEN** sensitivity is lowered on a busy fixture containing strong attacks and weaker transients at unchanged separation
- **THEN** fewer minor automatic markers are accepted while the documented prominent fixture attacks remain detectable, with measured limits reported

#### Scenario: Reanalyse without losing manual work
- **WHEN** a sensitivity edit produces a complete valid reanalysis
- **THEN** only eligible unedited automatic targets are reconciled, matched IDs/manual edits/deletion suppression remain protected, and unchanged render dependencies are reused

### Requirement: Automatic marker acceptance can be paused independently
A saved Auto markers preference SHALL default on, independently of mapping and the automatic-processing master. Off SHALL pause automatic acceptance/reconciliation and reject in-flight publication without deleting existing targets or stopping capture, manual actions or enabled existing-render refresh. Effective marking SHALL require the master and marker switch; restoring the new default SHALL NOT enable a saved-off master.

#### Scenario: Turn marking off while capturing
- **WHEN** Auto markers is turned off during capture or automatic analysis
- **THEN** no automatic target additions, movement or retirement publish after the switch, capture continues, existing swells remain editable and their permitted dependency refresh still works

#### Scenario: Adjust while off and resume
- **WHEN** sensitivity changes with marking off and the user later turns marking on under an enabled master
- **THEN** the value is saved without interim target reconciliation, and latest-settings bounded reanalysis catches up from committed coverage while protecting manual edits and suppression

#### Scenario: Restore saved off or legacy manual mode
- **WHEN** saved state has Auto markers off, or a legacy state has its automatic-processing master off
- **THEN** the explicit off policy is retained, no automatic markers appear until permitted, saved sensitivity survives and no audition starts

### Requirement: Marker-control changes invalidate obsolete analysis
Automatic analysis SHALL include detector policy/controls in cache identity and marker-control generation in publication guards. Rapid knob edits SHALL coalesce bounded worker requests; Off/On or newer sensitivity SHALL prevent obsolete completions from becoming current. Incomplete/failed analysis SHALL NOT retire targets or erase protected edits.

#### Scenario: Rapid sensitivity and enable changes
- **WHEN** sensitivity is dragged repeatedly and marking is disabled/re-enabled while a scan runs
- **THEN** only a complete permitted latest-generation result can publish, work remains bounded and the UI/audio callback performs no PCM analysis
