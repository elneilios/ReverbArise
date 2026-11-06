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
- **THEN** that target remains committed during preview and is reconciled or retired only within a successful shared transaction producing a changed nonempty viable automatic collection; obsolete jobs are cancelled and unrelated targets preserved

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
- **THEN** only candidate positions change until Generate; committed targets, IDs/manual edits/deletion suppression and render dependencies remain unchanged by preview

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
Analysis identity SHALL include detector policy, accepted source/coverage revision, coherent host timing and current requested detector settings; provisional analysis SHALL additionally identify its originating pass and be ineligible for generation. Publication SHALL reject superseded requests and retain the last displayed candidates marked stale/updating until a matching result arrives. Default IR edits SHALL invalidate generation intent without rerunning detection. Rapid permitted Perform edits SHALL coalesce; failed/incomplete analysis SHALL NOT retire protected targets or erase manual work.

#### Scenario: Rapid sensitivity and enable changes
- **WHEN** sensitivity is dragged repeatedly in Perform and marking is disabled/re-enabled while a scan runs
- **THEN** only a complete permitted latest-request result publishes, work stays bounded and the UI/audio callback performs no PCM analysis

#### Scenario: Next-pass settings change during deferred generation
- **WHEN** an accepted earlier pass remains queued and the user changes allowed future defaults
- **THEN** the pending Generate request is cancelled on any source/settings/default mismatch; immutable capture metadata stays unchanged and only newly requested preview uses the current settings

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
A successful Generate SHALL atomically publish a nonempty viable automatic result when definitions change or eligible unmatched automatic swells retire, then enter Perform under the same authority. Unchanged survivors SHALL retain their renders. Empty or identical collections SHALL NOT count as success. Validation failure, cancellation and stale completion SHALL NOT partially mutate definitions. Last-good assets SHALL remain recoverable.

#### Scenario: Repeated Generate with and without change
- **WHEN** a user generates again from identical dependencies, then repeats with a different valid IR or source dependency
- **THEN** the identical run reports no changes and stays Learn; the changed run preserves matched IDs, replaces eligible definitions atomically and enters Perform without claiming ready wet audio

#### Scenario: Reduce sensitivity after a denser generation
- **WHEN** low, high and then low sensitivity are generated from the same source, leaving a nonempty viable set with unchanged surviving definitions
- **THEN** the final Generate removes unmatched eligible automatic swells, preserves protected work and survivor identities/renders, and enters Perform; repeating that result reports no change

#### Scenario: Empty or protected-only plan
- **WHEN** no viable eligible automatic result remains even though old automatic definitions could be retired
- **THEN** every prior definition remains, Learn stays selected and a nonfatal outcome explains empty or protected-only status

### Requirement: Regeneration supersedes pending automatic wet work
An admitted Generate SHALL cooperatively cancel superseded unfrozen automatic render work and take priority over queued renders. Completed render metadata SHALL NOT be mistaken for a newer musical edit or overwritten by an older request snapshot. Protected work and last-good assets SHALL remain intact. A failed/cancelled replacement SHALL leave interrupted accepted work eligible to resume; actual edit/settings/source/transport/restore changes SHALL still cancel publication.

#### Scenario: Reduce while high-density renders are pending
- **WHEN** the user requests a nonempty lower-density generation while an earlier automatic render is in flight
- **THEN** the earlier job yields without waiting for the collection to become Ready, the replacement commits, obsolete jobs cannot publish and surviving pending work remains honestly queued/rendering

#### Scenario: Protected render finishes after admission
- **WHEN** a protected render finishes after Generate admission but before replacement preparation starts
- **THEN** its completed render and calibration remain in the replacement, the request proceeds if musical authority is unchanged, and a genuine newer edit still cancels the request

### Requirement: Explicit manual preparation is independent of automatic policy
Accepted manual creation and render-affecting edits in Perform SHALL request background preparation of the affected unfrozen swell even when automatic processing, markers and legacy auto-refresh are off. Automatic preferences SHALL remain unchanged. Capture commits alone SHALL retain dry-only policy; manual preparation SHALL preserve source, identity, edits, immutable renders and publication guards.

#### Scenario: Manual creation after dry-only capture
- **WHEN** short non-silent captured audio is committed with automatic processing and markers off and the user adds a manual target with complete source and a valid accepted IR
- **THEN** the target progresses through preparation to a verified Ready render and enables user-requested audible Perform Audition without enabling detection or starting playback automatically

#### Scenario: Edit manual dependencies
- **WHEN** the user accepts an arrival, source-interval or IR edit with all automatic policies off
- **THEN** only the affected eligible swell prepares against its latest accepted dependencies, preserves its identity and rejects obsolete completions
- **AND** shape-only, gain and placement changes retain their existing minimum-work behavior

#### Scenario: First snapped timeline insertion
- **WHEN** dry-only capture is accepted, no swell has yet pinned session timing, and the user double-clicks covered dry audio in Perform with Snap enabled
- **THEN** valid current fixed host timing and matching capture rate permit creation at the snapped arrival without a prior '+' insertion
- **AND** missing host position, invalid timing or incompatible established timing still blocks snapped insertion

#### Scenario: Capture alone remains dry-only
- **WHEN** dry-only Learn capture commits or Perform is entered without an explicit manual request
- **THEN** those events do not authorize automatic discovery or disabled-pass convolution, and saved automatic preferences remain off

#### Scenario: Imported WavPack IR
- **WHEN** a valid imported .wv IR is accepted as the manual target's assigned or future-default IR with sufficient source
- **THEN** manual preparation verifies and uses that immutable IR, yielding a usable render without substitution

### Requirement: Manual preparation feedback identifies unavailable dependencies
Manual preparation SHALL expose pending, stale, missing or unsupported inputs, insufficient coverage and rendering failures with an actionable reason. Ready and audition availability SHALL require a verified current render. Derived wet timing SHALL use bounds belonging to the selected swell and valid timing, or show unavailable values rather than fabricated render bounds.

#### Scenario: Missing coverage or invalid IR
- **WHEN** a manual request lacks complete source coverage, usable timing or a supported verified IR
- **THEN** feedback identifies the blocking input or failure and the swell does not falsely become Ready or auditionable

#### Scenario: Late-song unprepared selection
- **WHEN** a short captured clip near bar 40 has a newly added stale target without verified wet bounds
- **THEN** displayed pre-roll and end offset derive from valid selected-swell bounds or remain unavailable, rather than treating song origin as an unprepared wet endpoint

#### Scenario: Selection or session changes during preparation
- **WHEN** selection changes, a target is deleted, or a session is replaced while manual work is pending
- **THEN** obsolete work cannot publish into a different target/session or supply another target's timing or audition readiness
