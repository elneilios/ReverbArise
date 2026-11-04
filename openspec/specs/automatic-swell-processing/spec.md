# automatic-swell-processing Specification

## Purpose
Progressively turn committed dry Song captures into stable editable swells and verified wet renders without a manual capture, analysis or generation sequence.

## Requirements

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

### Requirement: New instances process captured Song audio automatically
New instances SHALL start in Capture with automatic processing and Auto markers enabled and Recapture off. Capture SHALL admit dry input only during realtime playback with usable absolute position and compatible timing. Only committed source SHALL own accepted targets/renders. After successful useful Stop validation, Live SHALL be selected independently of pending wet work.

#### Scenario: Fresh insert and playback
- **WHEN** a new instance captures musical input during supported DAW playback and the user stops
- **THEN** useful committed coverage causes automatic Live entry and targets/renders prepare without Capture, Suggest or Generate actions
- **AND** Capture playback itself emits no scheduled wet and background completion does not start internal audition

#### Scenario: Sparse capture and unsupported position
- **WHEN** playback visits bars 86-90 then 1-4, or usable absolute position becomes unavailable
- **THEN** admitted spans retain absolute coordinates and unmapped gaps, invalid timing is explained and no fabricated coordinates or partial invalid pass become committed

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
The system SHALL expose Waiting for source, Queued, Rendering, Ready, Stale/updating, Missing IR and Failed as applicable. It SHALL persist provenance, suppression, independent IR references and automation preferences. Restore SHALL validate assets/fresh timing and obey saved or migrated workflow mode. Captured content SHALL distinguish last committed source from provisional capture.

#### Scenario: Reopen edited automatic session
- **WHEN** a session with manual edits, suppressed targets and different per-swell IRs is reopened
- **THEN** identities and edits survive, verified results can be reused, missing assets remain explicit and no wet audition starts
- **AND** Recapture is off and no in-flight capture resumes

#### Scenario: Upstream edits while stopped
- **WHEN** source audio is changed upstream without explicit Capture/Recapture publication
- **THEN** the plugin does not claim to detect or adopt that change and retained source dependencies remain unchanged

#### Scenario: Disable automation
- **WHEN** automatic processing or marking is explicitly disabled and the project is saved/reopened
- **THEN** the preference is retained without deleting source or manual swells, and source admission remains controlled by workflow mode rather than enabling itself through automation

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
Analysis identity SHALL include detector policy, source dependencies and the applicable immutable pass or explicit reanalysis settings. Publication SHALL reject superseded requests without confusing later future-default edits with changes to an accepted pass. Rapid permitted Perform edits SHALL coalesce; failed/incomplete analysis SHALL NOT retire protected targets or erase manual work.

#### Scenario: Rapid sensitivity and enable changes
- **WHEN** sensitivity is dragged repeatedly in Perform and marking is disabled/re-enabled while a scan runs
- **THEN** only a complete permitted latest-request result publishes, work stays bounded and the UI/audio callback performs no PCM analysis

#### Scenario: Next-pass settings change during deferred generation
- **WHEN** an accepted earlier pass remains queued and the user changes allowed future defaults
- **THEN** its analysis retains the snapshotted settings rather than adopting or being silently relabelled with the next pass's settings

### Requirement: Automatic Swells does not grant source replacement
Automatic Swells SHALL govern automatic creation independently of Capture/Recapture permission. Off SHALL still permit dry capture and subsequent manual Live swells. Future defaults and manual/edited/frozen definitions SHALL survive toggling. Automation SHALL NOT capture source in Live/Finalise or publish accepted results from rejected provisional source.

#### Scenario: Dry-only learning
- **WHEN** useful dry is captured with Automatic Swells off and Stop validation succeeds
- **THEN** Live becomes usable for manual swell creation without requiring any automatic target or wet render

#### Scenario: Toggle without recapturing
- **WHEN** automatic marking is enabled or sensitivity changes in Live
- **THEN** permitted analysis uses committed source and cannot record upstream changes or overwrite mapped material

### Requirement: Source acceptance invalidates only real changed dependencies
A successful source transaction SHALL notify dependent work using its accepted revision and affected intervals. Identical accepted PCM SHALL reuse valid targets/renders/calibration; real changes SHALL invalidate only affected unfrozen dependencies. Rejected, duplicate or superseded transaction results SHALL NOT create targets or replace accepted renders.

#### Scenario: Atomic recapture with an unaffected neighbour
- **WHEN** one pass replaces source for one swell, preserves another identically and commits successfully
- **THEN** only genuinely changed dependencies refresh, protected edits remain and no provisional revision leaks into either swell

#### Scenario: Rendering continues after Live entry
- **WHEN** source validation completes before all corresponding wet jobs
- **THEN** Live shows truthful queued/rendering/ready states and admits only verified eligible wet while preparation continues

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
