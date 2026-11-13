# Spec Delta

## MODIFIED Requirements

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

## ADDED Requirements

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
