# Spec Delta

## MODIFIED Requirements

### Requirement: New instances process captured Song audio automatically
New instances SHALL start in Capture with automatic processing and Auto markers enabled and Recapture off. Capture SHALL admit dry input only during realtime playback with usable absolute position and compatible timing. Only committed source SHALL own accepted targets/renders. After useful Stop validation, Learn SHALL remain selected unless the opted-in shared generation transaction succeeds; only that commit can enter Perform, independently of pending wet work.

#### Scenario: Fresh insert and playback
- **WHEN** a new instance captures musical input during supported DAW playback and the user stops
- **THEN** useful committed coverage produces reviewable candidates; new projects have Auto-generate on Stop OFF and remain Learn until a successful explicit Generate
- **AND** Capture playback itself emits no scheduled wet and background completion does not start internal audition

#### Scenario: Sparse capture and unsupported position
- **WHEN** playback visits bars 86-90 then 1-4, or usable absolute position becomes unavailable
- **THEN** admitted spans retain absolute coordinates and unmapped gaps, invalid timing is explained and no fabricated coordinates or partial invalid pass become committed

### Requirement: Future swells inherit a valid bundled IR
The plugin SHALL include a redistributable valid default IR. New or eligible regenerated automatic swells SHALL use the current accepted Default IR frozen by the shared Generate request; new manual Perform swells SHALL use the currently accepted future default. Later default edits SHALL NOT mutate committed assignments; they invalidate pending generation, and apply only to a subsequent valid Generate request. Missing/invalid defaults or assigned IRs SHALL report errors without substitution or false Ready states.

#### Scenario: Independent assignments
- **WHEN** two swells inherit the accepted Generate default and one receives a different WAV/WavPack
- **THEN** only that swell changes its assignment/dependency; a future-default change alone does not alter existing assignments, while a later Generate may replace eligible automatic assignments

#### Scenario: Default cannot be loaded
- **WHEN** the bundled or assigned default cannot be verified
- **THEN** dry capture and allowed editing stay usable, affected generation reports the error and no incomplete result is Ready

#### Scenario: Two passes finish generation out of order
- **WHEN** pass A captured with IR A is still being analysed after pass B records with IR B
- **THEN** no analysis completion assigns an IR or commits a target; the shared Generate request freezes the then-current accepted default across the eligible full arrangement, preserving both capture contexts as history

#### Scenario: Add a manual swell after choosing a new default
- **WHEN** the user explicitly adds a manual target in Perform after accepting a new future default
- **THEN** the new manual target uses that current default without reassigning existing targets or implicitly running pending generation

### Requirement: Automatic targets have stable protected identities
The system SHALL distinguish suggestions, accepted automatic targets and manually created or edited targets. Successful shared generation SHALL reconcile one-to-one within a documented sample tolerance, preserve matched IDs and selection, and avoid duplicate targets. Manual edits SHALL not be moved or deleted by detection; deleted automatic targets SHALL remain suppressed until explicitly reset.

#### Scenario: Repeated and boundary detection
- **WHEN** unchanged audio is revisited or a transient straddles capture commits
- **THEN** preview creates no definitions; subsequent successful Generate retains one accepted matched identity rather than duplicating it at a boundary

#### Scenario: Manual intervention
- **WHEN** the user moves, edits or deletes an automatic target and its area is analyzed again
- **THEN** the edit and selected ID remain stable, and a deleted target does not immediately reappear

#### Scenario: Changed unedited target
- **WHEN** committed audio changes and an unedited automatic target is no longer supported by the detector
- **THEN** that target remains committed during preview and is reconciled or retired only within a successful shared transaction producing at least one changed viable automatic result; obsolete jobs are cancelled and unrelated targets preserved

### Requirement: Marker sensitivity controls event selectivity
One canonical per-instance sensitivity SHALL govern automatic and diagnostic onset analysis, with lower values favouring stronger attacks and rejecting more small events in documented busy fixtures. It SHALL preserve saved sensitivity and use bounded coalesced background reanalysis when marking is enabled. It SHALL NOT depend solely on a longer blind separation or claim musical-key/chord recognition.

#### Scenario: Reduce minor markers in busy source
- **WHEN** sensitivity is lowered on a busy fixture containing strong attacks and weaker transients at unchanged separation
- **THEN** fewer minor automatic markers are accepted while the documented prominent fixture attacks remain detectable, with measured limits reported

#### Scenario: Reanalyse without losing manual work
- **WHEN** a sensitivity edit produces a complete valid reanalysis
- **THEN** only candidate positions change until Generate; committed targets, IDs/manual edits/deletion suppression and render dependencies remain unchanged by preview

### Requirement: Marker-control changes invalidate obsolete analysis
Analysis identity SHALL include detector policy, accepted source/coverage revision, coherent host timing and current requested detector settings; provisional analysis SHALL additionally identify its originating pass and be ineligible for generation. Publication SHALL reject superseded requests and retain the last displayed candidates marked stale/updating until a matching result arrives. Default IR edits SHALL invalidate generation intent without rerunning detection. Rapid permitted Perform edits SHALL coalesce; failed/incomplete analysis SHALL NOT retire protected targets or erase manual work.

#### Scenario: Rapid sensitivity and enable changes
- **WHEN** sensitivity is dragged repeatedly in Perform and marking is disabled/re-enabled while a scan runs
- **THEN** only a complete permitted latest-request result publishes, work stays bounded and the UI/audio callback performs no PCM analysis

#### Scenario: Next-pass settings change during deferred generation
- **WHEN** an accepted earlier pass remains queued and the user changes allowed future defaults
- **THEN** the pending Generate request is cancelled on any source/settings/default mismatch; immutable capture metadata stays unchanged and only newly requested preview uses the current settings

### Requirement: Source acceptance invalidates only real changed dependencies
A successful source transaction SHALL notify dependent work using its accepted revision and affected intervals. Identical accepted PCM SHALL reuse valid targets/renders/calibration; real changes SHALL invalidate only affected unfrozen dependencies. Rejected, duplicate or superseded transaction results SHALL NOT create targets or replace accepted renders.

#### Scenario: Atomic recapture with an unaffected neighbour
- **WHEN** one pass replaces source for one swell, preserves another identically and commits successfully
- **THEN** only genuinely changed dependencies refresh, protected edits remain and no provisional revision leaks into either swell

#### Scenario: Rendering continues after Live entry
- **WHEN** source validation completes before all corresponding wet jobs
- **THEN** Perform entry occurs only after successful shared generation, then shows truthful queued/rendering/ready states and admits only verified eligible wet

### Requirement: A learning pass owns immutable generation defaults
At capture admission the system SHALL freeze effective Automatic Swells policy, detector controls and the accepted Default IR identity/label for that pass. Direct commands and late completions SHALL NOT change them before Stop settlement. Capture history SHALL retain that source/context pair. Preview and shared Generate SHALL explicitly use their own current accepted source/settings/default snapshots without rewriting capture history or silently replaying deferred pass-generation intent.

#### Scenario: Settings race DAW Play
- **WHEN** a generation-setting edit and pass start occur concurrently
- **THEN** the edit belongs wholly to the frozen pass or to a later pass, never a mixture of partially updated controls

#### Scenario: Import intent is still pending
- **WHEN** a custom/default import has not published successfully before Play
- **THEN** the pass pins the prior accepted default and the pending import cannot silently change its sound after admission

#### Scenario: A new attack follows a context boundary
- **WHEN** a target's arrival belongs to one accepted pass but confirmation/extraction extends into another
- **THEN** its capture provenance remains attributable to the arrival context, while Generate uses its explicitly frozen current default with normal full-source-coverage checks retained

### Requirement: Automatic Swells off provides dry-only learning
Learn with Automatic Swells off SHALL admit original dry source under independent Recapture rules, without admitting automatic analysis, new targets or convolution for that pass. Obsolete automatic publication SHALL be fenced/cancelled. Hidden preferences, existing definitions and immutable assets SHALL survive. Entering Perform SHALL NOT itself start new discovery from a disabled pass; explicit later Generate from accepted coverage uses the then-current effective Automatic Swells policy.

#### Scenario: All four capture-policy combinations
- **WHEN** Recapture and Automatic Swells independently take each on/off combination
- **THEN** only Recapture grants mapped replacement and effective Automatic Swells permits candidate analysis and Generate, while only enabled Auto-generate on Stop admits automatic generation after useful Stop
- **AND** useful accepted dry can enable Perform in every combination without requiring a wet render

#### Scenario: Turn automatic generation off with older work pending
- **WHEN** the idle Learn user disables Automatic Swells before another pass while automatic work remains pending
- **THEN** no new automatic detection or Generate work is admitted under Off, obsolete publication is cancelled/fenced and accepted definitions/assets are not deleted

#### Scenario: Manual work after dry-only learning
- **WHEN** dry-only capture has committed and the user explicitly creates/edits a swell in Perform
- **THEN** normal permitted manual preparation works without silently enabling automatic source replacement or automatic discovery

#### Scenario: Learn preferences target future passes
- **WHEN** the user changes Learn defaults after a dry-only pass
- **THEN** those future settings do not implicitly backfill/reassign old pass work; current Learn settings can preview all committed coverage and explicit Generate can use it without recapture; no setting edit itself commits definitions

### Requirement: Generation assignment preserves protected work
Shared generation SHALL preserve stable matched identities, manual/edited/frozen IR choices, deletion suppression and accepted transition/gain edits. Additive capture SHALL NOT replace protected source-context assignments. New default or context metadata alone SHALL NOT change audio fingerprints or reconvolve existing unchanged dependencies; only a successful Generate can assign the current default to eligible automatic definitions.

#### Scenario: Revisit a manually overridden automatic swell
- **WHEN** a new pass/default or permitted reanalysis revisits a manually edited or frozen target
- **THEN** its IR/edits/identity stay protected while genuinely new eligible targets use the request's accepted current Default IR with source provenance retained independently

#### Scenario: Context-only update with identical audio
- **WHEN** a context metadata update is accepted while PCM and existing swell dependencies remain identical
- **THEN** existing renders/calibration are reused and only an explicitly admitted shared generation transaction can create or replace automatic definitions

## ADDED Requirements

### Requirement: Learn candidates are read-only and versioned
Learn SHALL progressively display bounded asynchronous provisional or accepted candidate snapshots from real source coverage. Settled candidates SHALL match accepted source, detector settings and timing. Detection, sensitivity edits, inspection and overlay refresh SHALL NOT mutate definitions, render audio or write assets. Stale work SHALL NOT replace newer candidates; cancelled refresh SHALL retain the prior displayed set with truthful stale status.

#### Scenario: First capture and final acceptance
- **WHEN** safe capture summaries become available and Stop later accepts or rejects the pass
- **THEN** provisional candidates are visibly labelled and cannot generate; only a matching final accepted snapshot becomes eligible, with rejected provisional authority discarded

#### Scenario: Refine an existing dense arrangement
- **WHEN** a user returns to Learn without recapturing and rapidly changes sensitivity on a dense multi-run song
- **THEN** background scans coalesce over all committed mapped coverage, previous markers remain visibly updating until the latest complete snapshot replaces them, and definitions/render-job counts remain unchanged

### Requirement: Shared Generate freezes current accepted intent
Explicit Generate and eligible automatic Stop SHALL use one guarded transaction with the same final displayed candidates, accepted source/settings/timing and current verified Default IR. Capturing, source settlement/validation, stale analysis, invalid source/IR or another Generate SHALL prevent admission. Superseded identities SHALL prevent publication. No provisional source SHALL generate definitions.

#### Scenario: Change only Default IR
- **WHEN** the default changes after candidates settle
- **THEN** marker positions stay valid without convolution, pending generation is cancelled and a subsequent Generate uses the new accepted IR

#### Scenario: Invalid dependencies or preparation failure
- **WHEN** source validation, IR verification, resource limits or replacement preparation fails
- **THEN** no partial replacement occurs, last-good definitions/render references remain, and Learn reports a retryable failure

### Requirement: Replacement protects work across the complete arrangement
Generate SHALL prepare one replacement over the union of eligible committed mapped runs, preserving manual/edited/frozen, unknown-origin and outside-scope swells. Protected targets and deletion suppression SHALL win collisions within the documented detector tolerance. Eligible automatic matches SHALL retain IDs, names, selection and activity flags, including disabled state. Empty, protected-only or identical results SHALL preserve the whole collection.

#### Scenario: Mixed protected and automatic targets
- **WHEN** candidates collide with protected targets, include duplicate positions and match disabled unfrozen automatic definitions
- **THEN** protected targets retain every saved field/asset, equal candidates deduplicate, matches retain identity/activity flags and only eligible automatic dependencies adopt the request's default

#### Scenario: Sparse source and incomplete windows
- **WHEN** committed coverage contains gaps, silence, unavailable ranges or incomplete extraction tails
- **THEN** generation uses real eligible coverage, never invents source or retires definitions merely because they fall inside a bounding interval across gaps

### Requirement: Replacement success is nonempty and atomic
A successful Generate SHALL atomically publish at least one new or genuinely regenerated automatic definition and any eligible retirements, then enter Perform under the same workflow authority. Retirement alone or unchanged definitions SHALL NOT count as success. No validation failure, cancellation or stale completion SHALL partially mutate definitions. Immutable last-good assets SHALL remain recoverable while wet renders prepare asynchronously.

#### Scenario: Repeated Generate with and without change
- **WHEN** a user generates again from identical dependencies, then repeats with a different valid IR or source dependency
- **THEN** the identical run reports no changes and stays Learn; the changed run preserves matched IDs, replaces eligible definitions atomically and enters Perform without claiming ready wet audio

#### Scenario: Empty or removal-only plan
- **WHEN** no viable changed generated result exists even though old automatic definitions could be retired
- **THEN** every prior definition remains, Learn stays selected and a nonfatal outcome explains empty, protected-only or no-change status
