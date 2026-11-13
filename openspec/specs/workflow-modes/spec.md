# workflow-modes Specification

## Purpose
Define the mutually exclusive Capture, Live and Finalise workflow, including safe host-driven capture settlement, transition prerequisites and observable action availability without coupling it to a particular workspace layout.

## Requirements

### Requirement: One workflow mode governs instance behavior
Each instance SHALL have exactly one Capture, Live or Finalise mode. Fresh instances SHALL start in Capture with Recapture off. Mode SHALL govern source capture, scheduled wet, creative editing and internal audition independently of saved automation, monitoring and live preferences.

#### Scenario: Fresh instance
- **WHEN** a fresh instance is inserted without useful captured source
- **THEN** Capture is selected, Recapture is off, and Live/Finalise are unavailable with explanations
- **AND** no generated wet or internal audition starts automatically

#### Scenario: Independent instances
- **WHEN** two instances use different modes and receive interleaved host callbacks
- **THEN** their mode, capture-pass ownership and output remain independent

### Requirement: Mode entry prerequisites reflect verified material
Live entry SHALL require useful verified compatible source or an eligible verified frozen wet render at compatible timing. Song Finalise entry SHALL require an enabled, unmuted, solo-eligible verified exportable wet render compatible with current rate, tempo and meter, independently of dry availability or mixdown readiness. Loop Finalise entry SHALL use verified accepted capture-local timing and eligible wet independently of current host timing or live synchronization; its file readiness SHALL follow the Loop export contract. Mapped silence alone SHALL NOT satisfy usefulness. Pending wet renders SHALL NOT prevent source-ready Live entry. Frozen recovery SHALL NOT fabricate dry or permit replacement.

#### Scenario: Manual workflow after dry capture
- **WHEN** useful source has committed with Automatic Swells off and no prepared swell
- **THEN** Live is available for manual editing and Finalise remains unavailable

#### Scenario: Silence or failed verification
- **WHEN** coverage is only silence, unavailable, incompatible or still validating
- **THEN** neither mode becomes available by treating that coverage as useful compatible source; an independently verified eligible wet render retains its explicit recovery eligibility
- **AND** valid stored material is retained without reinterpretation

#### Scenario: Finalise readiness
- **WHEN** useful compatible source and an enabled ready swell exist
- **THEN** Finalise can be selected without requiring an immediate export

#### Scenario: Ready render has incompatible timing
- **WHEN** useful source matches the host but a ready render has a different sample rate, tempo or meter in Song mode, or a Loop render is incompatible with its accepted capture-local timing
- **THEN** that render cannot qualify for Finalise or frozen-wet recovery; eligibility does not substitute for the later export workspace's full asset/output preparation

#### Scenario: Frozen wet recovery without source
- **WHEN** captured dry is missing/cleared but an enabled unmuted solo-eligible frozen render verifies at compatible timing
- **THEN** Live entry and wet editing/playback are available without recreating source
- **AND** unfrozen source-dependent swells remain unavailable, captured-dry audition still requires real dry, and no Recapture permission is granted

#### Scenario: Export with independently recoverable wet
- **WHEN** dry source is missing but an eligible frozen wet render verifies at compatible timing
- **THEN** Finalise entry is available and wet-only preparation requires no invented dry; dry-inclusive mixes use the explicitly warned-silence policy
- **AND** source-dependent unavailable renders do not become exportable merely because the mode changed

#### Scenario: Entry and current file readiness differ
- **WHEN** a verified wet render permits Finalise entry and the user selects 0% Wet or a new mix is still preparing
- **THEN** Finalise remains selected, its current Save/Drag/preview actions reflect actual mixdown readiness and dry-only source alone still cannot unlock initial entry

#### Scenario: An accepted Loop is ready to edit without live sync
- **WHEN** a useful verified Loop is accepted and capture is idle
- **THEN** Perform is available using that phrase's local coordinates even when Song transport, Loop phase or prepared wet is unavailable
- **AND** entering Perform starts in dry Standby without moving host transport
- **AND** Loop export availability follows its separately delivered prerequisites

#### Scenario: Loop Export is independent of Cue readiness
- **WHEN** capture is idle and an accepted Loop has useful verified capture-local material and eligible wet, but the live clock or Phrase reference is unavailable
- **THEN** Export entry remains available under the Loop export prerequisites
- **AND** Save, Drag and preview readiness reflect the current finite composition without enabling Cue or inventing live synchronization

### Requirement: Capture activity locks manual mode switching
Manual mode switching SHALL be disabled during active DAW Capture and while its Stop-triggered drain, commit or validation is settling. Rejection SHALL identify the reason, apply to every command path, and leave current mode/source unchanged. A stopped idle Capture SHALL permit transitions whose prerequisites hold.

#### Scenario: Attempt to leave recording Capture
- **WHEN** a user requests Live or Finalise during recording or delayed Stop validation
- **THEN** the request is rejected with a recording/settling explanation and no partial source is exposed

#### Scenario: Mapped-only playback
- **WHEN** DAW playback in Capture encounters only protected mapped positions
- **THEN** source remains unchanged and stopping leaves Capture idle without claiming new useful capture
- **AND** Live can be selected explicitly while stopped if its prerequisites hold

### Requirement: Validated useful Stop commits automatically enter Live
An explicit DAW Stop SHALL close the pass once and settle asynchronously. Useful source acceptance SHALL settle in Learn. Only enabled Auto-generate on Stop with effective Automatic Swells SHALL request the shared Generate transaction after matching final analysis; Perform entry SHALL require a nonempty changed generation commit, without waiting for wet renders. Empty, mapped-only, aborted, incompatible or failed passes, and sessions left without useful material, SHALL remain Capture with truthful results and prior valid data intact.

#### Scenario: Stop while wet work remains pending
- **WHEN** source validation and an eligible shared generation transaction succeed while wet jobs are still queued or rendering
- **THEN** Perform is selected atomically with the successful generation commit and reports unfinished wet preparation honestly

#### Scenario: Empty or failed pass
- **WHEN** Stop closes an empty/mapped-only pass, publication fails, or no useful material remains after a silence-only replacement
- **THEN** Capture remains selected, the real reason is shown and earlier committed source remains available

#### Scenario: Identical explicit recapture
- **WHEN** an explicitly armed pass receives useful mapped audio identical to the accepted source and validation succeeds
- **THEN** Recapture disarms, source identity is reused and Learn remains selected unless the opted-in shared transaction commits a genuine changed automatic result

#### Scenario: Recapture deliberately replaces a note with silence
- **WHEN** an explicitly armed pass successfully replaces mapped samples with silence and other useful committed source remains
- **THEN** Recapture disarms, including for identical already-mapped silence; unchanged PCM retains identity, and Perform entry still requires opted-in successful changed generation

### Requirement: Transport events cannot publish a capture pass twice
Capture settlement SHALL be idempotent and scoped to its originating instance/session/pass. Duplicate Stop reports, loops, seeks, restarts, restoration and superseded work SHALL NOT publish a pass twice, overwrite a newer revision or trigger a stale mode transition. Missing timing SHALL NOT be interpreted as zero position or a successful Stop.

#### Scenario: Repeated Stop and delayed completion
- **WHEN** repeated stopped callbacks arrive while one validation job is delayed
- **THEN** the pass is committed at most once and completion causes at most one accepted mode transition

#### Scenario: Loop and restart
- **WHEN** playing transport loops/seeks or restarts before a prior Stop settles
- **THEN** spans retain their absolute coordinates, loops do not falsely finalize the pass and new audio cannot enter the closing transaction

#### Scenario: Restore during validation
- **WHEN** a different saved session is restored while a pass completion is pending
- **THEN** that completion cannot publish into the restored session or change its mode

### Requirement: Mode changes preserve the working session
Entering Finalise SHALL preserve captured source, swell definitions, accepted IR/render versions and editing/monitoring preferences while locking creative edits. Returning to Live SHALL restore editing and saved live behavior without recapture, data loss, export requirements or implicit audition/host transport requests.

#### Scenario: Return without exporting
- **WHEN** a user enters Finalise and then returns to Live without creating any file
- **THEN** the same source, swells, edits and prepared assets remain and Live can be used indefinitely

#### Scenario: Existing exported assets
- **WHEN** the working session includes durable exported files produced by another capability and modes change
- **THEN** those files are not removed or overwritten by the mode transition

### Requirement: Workflow settlement remains outside realtime processing
Realtime host processing SHALL only perform bounded transport observation, capture enqueue and prepared mixing. Mode changes and Stop handling SHALL NOT introduce file access, asset validation, rendering, allocation/deallocation, mutex acquisition or waiting in that callback. Worker delay SHALL remain explicit without blocking audio.

#### Scenario: Slow storage on Stop
- **WHEN** source validation is delayed or storage fails while the host continues processing
- **THEN** dry audio continues under the mode contract, mode status remains settling/failed as appropriate and realtime processing performs no blocking storage work

### Requirement: Creative mode names are separate from durable identity
User-facing mode names SHALL be Learn, Perform and Export for the existing capture, live and finalise behaviors respectively. Visible mode guidance, rejection reasons and accessible text SHALL use those names consistently. Saved mode identifiers and plugin identities SHALL remain compatible; versioned generation preferences and revised optional Stop transitions SHALL follow their explicit migration contracts. Audio routing SHALL retain its existing mode semantics. Functional concepts such as source capture, Recapture and live wet SHALL retain their accurate meaning.

#### Scenario: Reopen saved workflow modes
- **WHEN** supported projects saved with Capture, Live or Finalise identifiers reopen
- **THEN** their corresponding Learn, Perform or Export mode is displayed with the same source, swells, edits, readiness rules and output behavior, without renumbering modes or implicit recording/audition; the separately versioned Auto-generate preference follows its legacy migration
- **AND** projects lacking saved mode metadata still enter Learn with Recapture off under the existing legacy policy

#### Scenario: Save and restore a newly labelled mode
- **WHEN** a valid mode is selected through the new navigation and the host saves/reopens the project
- **THEN** the original durable mode representation round-trips, the chosen display label returns, and a real mode change retains existing host save notification behavior

#### Scenario: Automatic transition while renders prepare
- **WHEN** the opted-in Stop route commits one or more new or genuinely regenerated automatic swells from useful accepted source
- **THEN** Perform becomes selected after the generation commit without waiting for wet jobs, with honest background preparation status
- **AND** mapped-only, fresh all-silent, failed and erased-last-useful-source outcomes retain their existing Learn behavior

#### Scenario: Existing eligibility and routing
- **WHEN** a user requests Perform or Export with silent-only, incompatible, frozen-recovery or ready-render material
- **THEN** the existing eligibility rules decide the request without being weakened by presentation changes
- **AND** Learn captures only under its existing policy, Perform schedules wet without capture, and Export leaves DAW output dry with only explicitly initiated supported preview

### Requirement: Optional automatic generation follows useful Stop only
Auto-generate on Stop SHALL admit generation only after successfully accepted useful Stop with effective Automatic Swells enabled, waiting for final matching analysis. Toggling ON, restoring state, editing settings or entering Learn SHALL NOT trigger generation. OFF SHALL cancel pending automatic intent before commit. Changed settings/source/default/timing or workflow authority SHALL invalidate the request without retargeting it to later work.

#### Scenario: Preference changes without capture
- **WHEN** Auto-generate is enabled, the project restores or Learn settings change without a newly accepted useful Stop
- **THEN** no automatic generation starts and existing swells remain unchanged

#### Scenario: Stop request awaits final analysis
- **WHEN** a useful accepted Stop admits an automatic request and its pinned identities change or Auto-generate is disabled before commit
- **THEN** that request is cancelled without generating against a later snapshot or changing the accepted mode

### Requirement: Generation authority cannot outlive its workflow
The explicit Learn action and eligible Stop generation SHALL share readiness and cancellation guards with distinct replacement policies. Only an idle Learn request with matching final source/settings/timing/IR authority can publish. Capture arming, incompatible mode changes, restore, cancellation or changed dependencies SHALL invalidate pending publication and prevent partial definitions or stale Perform entry.

#### Scenario: Restart or switch while generation prepares
- **WHEN** a valid mode change, capture arm or restore supersedes a held generation request
- **THEN** no prepared definitions publish and no stale completion changes the accepted mode

#### Scenario: Manual navigation without generation
- **WHEN** an idle Learn user selects Perform or Export while candidate analysis is pending
- **THEN** existing readiness rules govern navigation, pending generation is invalidated and no automatic generation is implied

#### Scenario: Duplicate Stop or unsuccessful generation
- **WHEN** Stop repeats or generation is empty, protected-only, unchanged, failed or cancelled
- **THEN** no duplicate transaction commits, prior swells remain and the generation path stays Learn with its honest outcome

### Requirement: Learn capture requires a one-take arm
Song and Loop SHALL require an explicit Arm request in Learn before recording. The request SHALL pin the capture mode and replacement permission for one take. Fresh instances and restored projects SHALL start disarmed. The arm SHALL expire on settlement, cancellation or restore. Each capture mode SHALL retain its verified timing and ending rules.

#### Scenario: Open or return during transport
- **WHEN** a fresh instance is inserted, a saved project is restored or an idle instance returns to Learn during compatible playback
- **THEN** no recording starts until the user explicitly arms, including during FL Studio Pattern playback
- **AND** subsequent Play, seeks and repeats alone cannot grant capture

#### Scenario: Change the pending capture configuration
- **WHEN** a take is armed, recording or settling
- **THEN** the pinned mode and replacement permission cannot change and a duplicate arm cannot reserve a second take

#### Scenario: Cancel a waiting take
- **WHEN** the user disarms before recording
- **THEN** the next valid transport or timing observation cannot start that take and the accepted source remains available
