# sound-presets Specification

## Purpose

Define durable Sound assignments for swell creation and editing, with versioned preset settings and a verified impulse response for playback and project restoration.

## Requirements

### Requirement: Sound assignments identify the definition and exact response

An accepted Sound assignment SHALL identify a stable preset, its definition version, supported typed values and the exact managed response reference. The catalogue SHALL include Stereo Ping Pong Crescendo alongside the six shipped bundled responses and managed Custom IR. Catalogue order and labels SHALL have no authority over saved assignments.

#### Scenario: Bundled or custom choice is accepted
- **WHEN** a shipped response or managed import is accepted
- **THEN** the accepted assignment uses the stable bundled definition or Custom IR definition, respectively, with the exact immutable response
- **AND** names, parameter defaults and format validation remain independent of catalogue position

#### Scenario: Previously assigned bytes differ from the installed catalogue
- **WHEN** a stored response is unavailable or differs from a current catalogue revision
- **THEN** restoration retains the saved assignment and reports unavailable dependencies through existing recovery behavior
- **AND** no currently installed response is silently substituted

#### Scenario: Ping Pong is accepted or restored
- **WHEN** a Ping Pong setting is accepted or a saved project is restored
- **THEN** the assignment carries the exact managed response and versioned creative settings used to produce it
- **AND** reopen preserves accepted audio and settings across host timing or catalogue changes until an eligible replacement is prepared and accepted

### Requirement: Accepted Learn defaults affect newly created swells

The instance SHALL retain a versioned future Sound assignment and a Level default of 0 dB for new or migrated projects with no saved Learn Level. New automatic and manual swells SHALL copy the accepted settings at their existing guarded creation boundary. Editing defaults SHALL leave existing definitions and renders intact.

#### Scenario: Create swells after changing a default
- **WHEN** an eligible default change is accepted and a new automatic or manual swell is created
- **THEN** the new swell uses the accepted preset, response and Level from that creation request
- **AND** existing swells keep their assigned settings and gain

#### Scenario: A queued creation request loses authority
- **WHEN** the accepted defaults, session or guarded capture context change before publication
- **THEN** stale work cannot publish a swell assembled from mixed defaults or transfer authority to another session

#### Scenario: Generate matches an existing eligible automatic swell
- **WHEN** a successful Generate applies the current default response to an eligible automatic swell under the existing replacement policy
- **THEN** the matched swell retains its ID and existing Level
- **AND** the new Learn Level default is applied only to newly created swells

### Requirement: Sound replacement publishes one coherent accepted edit

A response-changing edit SHALL reuse bounded background preparation and commit the assignment, effective wet shape and matching render as one accepted edit. Failed, stale and superseded requests SHALL leave the prior accepted state intact. The realtime callback SHALL perform no new preparation, I/O or waits.

#### Scenario: A shorter response needs fitted bounds during active playback
- **WHEN** a valid shorter response is requested on an eligible verified swell
- **THEN** the existing contribution remains eligible until the verified replacement and fitted shape publish together
- **AND** the target, gain and unrelated swells stay intact, and timing intent remains available for a longer response

#### Scenario: A replacement fails, loses authority or targets a frozen swell
- **WHEN** preparation fails, a newer request supersedes it, its session or target authority changes, or Freeze prevents acceptance
- **THEN** the stale or invalid edit cannot publish or change the saved sound, fitted shape or render
- **AND** a failure of the current request remains actionable, while a discarded request cannot overwrite newer status

#### Scenario: Export or host bounce starts with a replacement pending
- **WHEN** a pending request completes after a composition or host offline snapshot is taken
- **THEN** that snapshot uses the coherent accepted assignment and render for its duration
- **AND** the next eligible snapshot can use the new accepted revision

#### Scenario: Level or Envelope changes independently
- **WHEN** an eligible Level or Envelope edit is accepted without a response change
- **THEN** it follows its existing gain or shape update path and does not rebuild the full convolution response

### Requirement: Ping Pong retimes alternating hits around the swell arrival

Stereo Ping Pong Crescendo SHALL position discrete stereo hits at the selected musical subdivision in audible reverse-convolution time. With Tempo Sync on and compatible timing available, hit anchors SHALL align to the ruler's grid measured from Beat 1. With Sync off, the final hit SHALL retain its authored offset from the swell arrival.

#### Scenario: Change echo spacing
- **WHEN** the same sound is rendered at different supported BPMs or subdivisions
- **THEN** quarter, eighth, dotted eighth, sixteenth, dotted sixteenth and thirty-second spacing are respectively one, one half, three quarters, one quarter, three eighths and one eighth of a quarter-note beat
- **AND** hit positions are rounded independently to the output sample grid, preserve hit character and leave the swell arrival fixed

#### Scenario: Fewer complete hits fit
- **WHEN** a wider spacing fits fewer hits inside the available response duration
- **THEN** the earliest audible hits are omitted first and the last eligible hit fits the available response support

#### Scenario: Edit a synced swell in Perform
- **WHEN** the user commits a Target, Move Wet, source-bound, crop or Envelope edit with a compatible ruler grid
- **THEN** accepted synced hit anchors stay on that grid regardless of Perform Snap, and the user can keep an off-grid arrival
- **AND** crop and Envelope can hide or attenuate hits without moving their anchors
- **AND** placement and response changes that require realignment publish together, with the last coherent accepted state retained while pending or if the edit cannot be accepted

### Requirement: Ping Pong resolves a stable tempo for preparation

Tempo Sync SHALL use compatible fixed Song timing or committed Loop tempo. With Sync off, Echo Rate SHALL use the authored reference tempo. A pending or unavailable timing change SHALL leave a previously accepted response in effect.

#### Scenario: Prepare with no compatible timing
- **WHEN** Sync is on and the host cannot supply compatible timing for a new assignment
- **THEN** preparation uses the authored reference tempo if there is no accepted Ping Pong response to retain for that assignment, and the Sound editor identifies the fallback
- **AND** an ineligible transport observation cannot silently replace an accepted synced response

#### Scenario: Prepare after a stable timing change
- **WHEN** a new compatible fixed tempo becomes authoritative for an eligible assignment
- **THEN** the response is prepared in the background and accepted only if the timing and assignment are still current
- **AND** host offline bounce uses the coherent accepted snapshot under existing timing admission

### Requirement: Ping Pong character controls preserve independent gain and shaping

Stereo Spread SHALL control the separation of the alternating hits. Build Intensity SHALL control their gain progression toward the arrival, from even response peaks at 0% through the authored build at 75% to a stronger build at 100%. Musical gain matching SHALL preserve the intended change in late-hit level, subject to existing headroom protection. Level and Envelope SHALL retain their existing independent behavior.

#### Scenario: Edit spread or build
- **WHEN** an eligible Spread or Build setting is changed
- **THEN** the prepared response reflects the requested channel separation or gain progression while retaining the echo timing
- **AND** a Level or Envelope edit follows the existing gain or shaping path without regenerating the response

### Requirement: Gate Amount controls overlap between Ping Pong echoes

Gate Amount SHALL default to 0% and blend between original stereo excitation and the shared, tapered excitation used at 100%. The blend SHALL leave captured dry audio and the echo grid unchanged.

#### Scenario: Render from a panned sustained source
- **WHEN** the current preset renders at 100% Gate Amount with nonzero Stereo Spread from sustained material panned to one side
- **THEN** both wet channels are driven from the shared excitation tapered over one echo interval

#### Scenario: Restore a saved Ping Pong assignment
- **WHEN** a project reopens with an accepted Ping Pong assignment
- **THEN** the assignment keeps its accepted audio and saved Gate Amount
- **AND** saved revisions 1–3 display 0% and revision 4 displays 100%

### Requirement: Damping reduces Ping Pong burst resonance

Damping SHALL default to 0% and reduce ringing within each prepared burst as it increases. The burst peaks, timing and channel assignments SHALL be preserved. Saved assignments SHALL retain their accepted audio and saved Damping value. Assignments saved before Damping SHALL reopen at 0%.

#### Scenario: Tame a resonant echo
- **WHEN** Damping increases on an editable Ping Pong assignment
- **THEN** the updated response reduces ringing around each hit while keeping its timing and stereo assignment
- **AND** returning to 0% restores the original response at the current settings

### Requirement: Every Sound assignment supports shared shaping

Every available preset and Custom IR SHALL support Level, Damping, High Pass and Low Pass. New assignments SHALL default to 0 dB Level, 0% Damping, and Off for each cut unless accepted Learn defaults specify other values.

#### Scenario: Set the full creative range
- **WHEN** an eligible assignment is edited
- **THEN** Damping accepts 0–100%, and each cut accepts Off or a cutoff within 20 Hz–20 kHz
- **AND** Level retains its existing per-context range and gain behavior

#### Scenario: Apply source cuts to the wet response
- **WHEN** a High Pass or Low Pass is enabled
- **THEN** the wet render reflects the corresponding high-pass or low-pass filtering of the captured source before convolution
- **AND** dry playback and captured PCM remain unchanged

#### Scenario: Shorten a ringing response
- **WHEN** Damping increases from 0% on a full response
- **THEN** progressively less of the IR tail contributes to the wet swell, spanning the original decay through a short accent at the top of the range
- **AND** the selected arrival stays fixed and Envelope stays independently editable

#### Scenario: Dampen a rhythmic preset
- **WHEN** Stereo Ping Pong Crescendo uses its shared Damping dial
- **THEN** Damping shortens the ring within each hit while preserving its echo grid, stereo assignment and saved Damping behavior

### Requirement: Shared shaping preserves accepted sound on reopen

Existing swells SHALL retain their settings, immutable source and IR references, and accepted audio when they reopen. The neutral shared processing SHALL bypass the new filters and decay shaping exactly.

#### Scenario: Load a project from before shared shaping
- **WHEN** a project lacks the new shared values
- **THEN** absent values read as neutral and any previously saved Level or preset-specific Damping retains its existing value and sound
- **AND** opening the project does not queue regeneration just to add shared controls

#### Scenario: Adopt shared values for new swells
- **WHEN** Learn accepts a shared shaping edit
- **THEN** new automatic and manual swells inherit the accepted settings
- **AND** no existing swell acquires those values solely because the defaults changed

#### Scenario: Save an accepted shaped assignment
- **WHEN** a project with a non-neutral shared setting is saved and reopened
- **THEN** its canonical setting and accepted render reopen together
- **AND** a failed, stale, superseded or frozen edit cannot replace that accepted pair
