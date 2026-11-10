# Spec Delta

## MODIFIED Requirements

### Requirement: Workspace reflects actual processing and selection
The workspace SHALL adapt to mode: Learn shows capture settings and one dry timeline for the accepted source; Perform shows the production three-column editor, audition and compact Overview; Export shows its dedicated preview and mix/file controls. Processing and selection SHALL remain identity-bound, truthful and mode-appropriate. A requested render or mode SHALL NOT be shown as already ready.

#### Scenario: Select while background work completes
- **WHEN** the user selects a swell in Perform while allowed analysis or another swell's rendering completes
- **THEN** its row, handles and inspector remain bound to that identity and show actual IR/state, with editing available only under mode guards

#### Scenario: Empty or unavailable selection
- **WHEN** the session is empty or a result is waiting, stale or failed
- **THEN** the active workspace explains its state and enables only valid mode/selection actions without false ready waveforms or current-WAV claims

#### Scenario: Stable empty and loaded composition
- **WHEN** the same editor size transitions through empty, mapped, selected and updating states within a mode
- **THEN** its region bounds/dividers remain coherent and no developer toolbar displaces them or promises a deferred export capability

#### Scenario: Return to Perform
- **WHEN** a prepared user leaves Learn for Perform
- **THEN** the working swell list, selected inspector, editor navigation, audition and compact Overview return without losing definitions, selection or edits

#### Scenario: Dedicated production Perform
- **WHEN** Perform is displayed at supported sizes
- **THEN** the chronological Swells list is left, real aligned dry/wet editor is central and Selected Swell inspector is right, with audition and one separate compact Overview below
- **AND** shared header, Settings, reserved notices, production Inter/semantic roles and existing editing capabilities remain usable without changing Learn or Export composition

### Requirement: Timeline navigation preserves exact edits
The editor SHALL align dry, wet, arrivals and handles on the accepted source's absolute Song or phrase-local Loop scale with bar references, zoom, pan and snap. The overview SHALL distinguish audio, captured silence, unmapped and unavailable coverage and support viewport navigation. Navigation SHALL not edit render inputs. Display-only peak scaling SHALL remain disclosed.

#### Scenario: Navigate distant sections
- **WHEN** the user clicks or drags the overview between distant captured sections
- **THEN** detail view and ruler follow the viewport without moving swells or changing source dependencies, and the gap remains visibly unmapped

#### Scenario: Loud wet waveform
- **WHEN** a selected raw render exceeds full scale
- **THEN** its envelope remains visible without flat clipping, its display scale is available, and its audio gain is unchanged by drawing

### Requirement: Timeline depiction reflects real timing and audio
Dry/wet lanes, ruler, handles and readouts SHALL share positions in the accepted source's time domain. The selected target SHALL use a narrow yellow dashed line across both lanes. Drawn wet data SHALL reflect real samples/shape and disclosed display scaling, without forcing its energy peak onto the target. Negative lead-in SHALL respect the scheduling contract of the accepted source.

#### Scenario: Shifted energy and negative endpoint
- **WHEN** an asymmetric IR produces an off-target energy peak and the wet endpoint is moved before dry arrival
- **THEN** actual energy/timing, signed gap and fixed dry event remain visible at different zoom levels without drawing a fictional symmetric swell

### Requirement: Overview has one truthful waveform viewport
Perform SHALL retain its compact Overview with ruler, real dry/wet summaries and one detail-viewport highlight. Learn SHALL use one large dry Overview for the accepted source instead of a duplicate strip, with origin-anchored extent, explicit coverage states and no wet-envelope editing. Both views SHALL use the accepted source's actual positions, disclose display scaling and avoid seeking the host through viewport navigation.

#### Scenario: Sparse map and viewport
- **WHEN** two captured sections with mapped silence are separated by an uncaptured gap and Perform detail navigation changes
- **THEN** the gap contains no invented waveform, silence remains recognizable, ruler/highlight agree with actual bounds and navigation affects only the editor

#### Scenario: Learn has one source view
- **WHEN** Learn displays a sparse capture
- **THEN** its large timeline is the full-song Overview, no second bottom Overview or wet lane appears, and all sections/gaps stay at their absolute positions

#### Scenario: Truly empty instance
- **WHEN** Perform has neither captured audio nor a selected swell
- **THEN** Swells (0), No swells yet, Press play to begin mapping your song, No swell selected and No audio mapped yet appear in its stable regions
- **AND** Learn uses its mode-appropriate DAW Play prompt; neither mode shows fake data or forced import/generation, and saved-off capture/generation policy is explained rather than silently enabled

## ADDED Requirements

### Requirement: Loop editing reuses the shared Perform workspace
Perform SHALL offer the shared multi-swell editing and Audition tools for an accepted Loop. The ruler, Overview, source controls and wet edits SHALL use the accepted phrase's local grid and exact extent. Source selections SHALL stay in bounds and wet placement SHALL retain supported negative offsets.

#### Scenario: Edit two swells around Bar 1
- **WHEN** a musician selects and edits two swells including wet that starts before Bar 1
- **THEN** the list, inspector exact fields, source and wet handles, zoom and Overview stay aligned to the selected swell and accepted phrase
- **AND** the existing IR, envelope, placement, reset, Freeze and audition bindings operate on the correct swell

#### Scenario: Next-capture preference changes
- **WHEN** the next-capture preference is changed or a replacement is pending
- **THEN** existing Perform edits stay bound to the current accepted identity until a useful replacement settles
- **AND** stale gestures and worker results cannot modify or publish against the replacement

#### Scenario: Add a target in Loop Perform
- **WHEN** the musician invokes Add while the Loop playhead is within the captured phrase
- **THEN** the target is added at the displayed phrase position
- **AND** when no in-phrase playhead is available, the action says it adds at the start of the visible loop
- **AND** a source or session change before insertion cancels the action

### Requirement: Loop playback controls report actual readiness
An accepted Loop SHALL show one playback command and separate status beside Audition in Perform. Cue Loop SHALL require verified phase and coherent prepared wet. Cancel Loop SHALL cancel a queued start. Stop Loop SHALL fade sounding Loop wet while dry continues. These commands SHALL act independently of Audition. Song SHALL retain its existing composition.

#### Scenario: Phase, preparation and playback change
- **WHEN** a Loop enters Perform with no reference, a mismatched reference, unverified phase, lost phase, unavailable preparation or disabled live output
- **THEN** Cue is disabled with the specific actionable reason and dry remains available
- **AND** verified phase and prepared wet enable Cue Loop automatically with Ready status
- **AND** the command becomes Cancel Loop with Waiting for phrase status when queued, then Stop Loop with Playing lead-in or Playing loop status when wet starts
- **AND** the command is disabled while Fading out and offers Cue Loop again when ready

#### Scenario: Lead-in wraps into the previous pass
- **WHEN** Loop playback repeats with a scheduled lead-in for the next pass
- **THEN** one yellow playhead follows the current phrase and wraps at the observed boundary
- **AND** the wet lane shows a faint waveform of the next pass's prepared wet mix near the end of the phrase, regardless of the selected swell

#### Scenario: Compact review layout
- **WHEN** Song and Loop Perform are viewed at 900×600 at 100% and 125%
- **THEN** the Loop command and status fit on the Audition row with a separator and the command remains keyboard accessible
- **AND** the rest of the shared editor stays usable and Song adds no Loop controls
