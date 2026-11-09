# loop-host-admission Specification

## Purpose

Define how host observations establish usable timing and command authority for Loop Mode while preserving Song's absolute timeline contract.

## Requirements

### Requirement: Host capabilities have independent per-instance admission
The admission contract SHALL report the Song timeline, Loop phrase phase, each configured capture method, musical grid and external performance commands independently for each instance. Each result SHALL expose Unsupported, Not configured, Not verified, Sync lost or Ready, with a specific cause and recovery guidance when unavailable.

#### Scenario: No automatic fallback
- **WHEN** an instance using Song lacks a valid absolute host timeline and has verified Loop inputs
- **THEN** the Song timeline stays unavailable and the selected workflow does not change
- **AND** missing host samples, PPQ and meter are not invented

#### Scenario: External commands and local Cue have separate prerequisites
- **WHEN** phrase phase is Ready and no external command route is configured
- **THEN** the external command result is Not configured and that result does not block local Cue or Stop
- **AND** Loop performance still requires the applicable source and playback prerequisites

#### Scenario: Capture can have fewer authorities than playback
- **WHEN** an instance has verified capture edges but lacks a recurring phase reference
- **THEN** the edge result does not establish phase or a musical grid
- **AND** only a capture method with all its own timing prerequisites can be Ready
- **AND** another instance cannot inherit those observations

### Requirement: Musical admission requires explicit meter and origin
Bars, beats and musical grids SHALL require verified beat timing and explicit meter. A phrase authority SHALL identify an origin shared by the audible source and reference and a verified recurrence matching the intended phrase. Equal tempo alone SHALL NOT establish phase.

#### Scenario: Beat and bar observations without a declared meter
- **WHEN** periodic inputs arrive with no explicit meter configuration
- **THEN** musical duration and grid admission remain unavailable with a meter explanation

#### Scenario: Capture length differs from the recurring reference
- **WHEN** a one-bar take is verified and the only verified recurrence is four bars
- **THEN** the take remains valid and live Cue reports the missing matching phase reference

#### Scenario: Verified take length rounds to an adjacent frame count
- **WHEN** an accepted take matches the reference's verified sample rate, stable tempo, explicit meter, duration in meter beats and phrase origin
- **AND** its recorded frame count is the floor or ceiling of the nominal period derived from those timing fields
- **THEN** either frame count matches the recurrence without changing the recorded extent
- **AND** an out-of-range frame count or mismatched musical timing leaves Cue unavailable

### Requirement: Verified inputs retain their sample offsets and roles
The host input contract SHALL preserve valid callback-relative sample offsets and distinguish capture gate, phrase, bar, beat, reset, Cue and Perform Stop. Invalid offsets SHALL NOT be clamped into valid commands. Capture End signal SHALL require a verified request and, on a route with bar-quantised completion, a verified bar at or after the request.

#### Scenario: Valid events at the same sample
- **WHEN** a verified end request and bar marker share a sample
- **THEN** that sample is the exclusive capture end
- **AND** a phrase or bar marker by itself is neither a capture end request nor Perform Stop

#### Scenario: Host starts with a high capture gate
- **WHEN** capture is armed or restored with the gate high or of unknown state
- **THEN** a fresh verified low-to-high transition is required to establish a start

#### Scenario: Gate release cannot be attributed safely
- **WHEN** a disconnect is detected or its release cannot be distinguished from a deliberate request on the admitted route
- **THEN** ambiguous evidence cannot establish a successful capture end
- **AND** a route that cannot provide the necessary distinction requires a separately verified end request or reports that method unavailable

### Requirement: Phrase phase tolerates measured sample rounding
Stable-tempo phase authority SHALL accept the bounded adjacent integer intervals verified for its configuration and use observed markers as the boundary authority. It SHALL detect missing, inconsistent or drifting observations without silently stretching the accepted phrase or treating the one-sample rounding allowance as a tempo tolerance.

#### Scenario: Fractional samples per phrase
- **WHEN** an aligned reference alternates between the two verified adjacent integer intervals
- **THEN** phase remains Ready and each observed marker identifies its actual phrase boundary

#### Scenario: Reference slips away from its verified cadence
- **WHEN** a required marker misses its supported arrival window or successive intervals leave the verified cadence
- **THEN** phase becomes Sync lost with a timing explanation

#### Scenario: Cue arrives after the missing marker deadline
- **WHEN** Cue arrives after the last permitted sample for a required marker that has not arrived
- **THEN** the reference becomes Sync lost before Cue is evaluated, including between diagnostic heartbeats
- **AND** the command is rejected and a fresh Cue is required after re-lock

### Requirement: Discontinuities invalidate live authority
A reset, incompatible tempo or sample rate, or observed interruption of a required route SHALL revoke the affected live authority. Restore and reactivation SHALL require fresh evidence. An interrupted capture SHALL NOT finish from missing observations, and Loop performance SHALL require a fresh Cue after phase is regained.

#### Scenario: Callbacks stop before the requested end boundary
- **WHEN** callbacks cease while a bar-quantised end is pending
- **THEN** no completion is reported for the unobserved bar
- **AND** a detected interruption invalidates the pending request and preserves the prior accepted source

#### Scenario: Patch reload and reset
- **WHEN** a patch reloads or a verified reset arrives
- **THEN** prior timing and queued commands cannot grant readiness in the new reference
- **AND** recovery waits for fresh matching markers and a fresh Cue before admitting wet

### Requirement: Support is limited to approved measured configurations
The host contract SHALL identify approved combinations of route, versions, rate, callback conditions and wiring, with reproducible timing and lifecycle evidence. A configuration or route outside this evidence SHALL NOT be represented as verified by host name, parameter availability or analogy.

#### Scenario: A route remains unmeasured
- **WHEN** only Rack Note Gate has passed the applicable verification
- **THEN** neither ordinary automation nor FL Studio VST3 FX MIDI Out is advertised as verified
- **AND** the evidence identifies pending DAW coverage under #127
