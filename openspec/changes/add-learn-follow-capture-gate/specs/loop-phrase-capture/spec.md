# Spec Delta

## Purpose

Capture one stereo phrase from a live patch using a musician-selected gate and retain a verified recording for later use.

## ADDED Requirements

### Requirement: Learn selects the next capture type

Learn SHALL expose Enable Loop and Follow capture gate. Song SHALL remain the default for existing and new instances. The next capture preference SHALL remain separate from the last accepted take. Learn SHALL show which take is accepted and whether the next capture is waiting, recording, settling, cancelled or rejected.

#### Scenario: Arm a loop over an accepted Song take

- **GIVEN** an instance has accepted Song source and the user selects Enable Loop
- **WHEN** the user arms a gate capture
- **THEN** Learn waits for the gate and retains the accepted Song source and its edits
- **AND** the Song workflow stays associated with the accepted source until the new Loop take is verified

#### Scenario: Song uses the existing host route

- **WHEN** Enable Loop is off and the DAW starts or stops
- **THEN** existing Song capture admission, absolute coordinates, recapture, settlement and generated assets behave as before
- **AND** MIDI capture notes have no effect on Song capture

### Requirement: A fresh capture gate controls one take

An explicitly armed Learn instance SHALL capture the stereo interval from the sample of the next fresh gate rise up to the sample immediately before its fall. The gate SHALL use the verified Host-FX Note Gate route. The take SHALL use local sample positions and the prepared audio rate.

#### Scenario: Gate edges inside callbacks

- **GIVEN** Host-FX delivers the armed rise and fall at nonzero offsets within audio callbacks
- **WHEN** the take settles
- **THEN** the WAV contains exactly the stereo frames between those two offsets
- **AND** unavailable host PPQ, time signature or transport fields do not change the take boundaries

#### Scenario: Gate is already high or its state is unknown

- **WHEN** the user arms while the gate is high or its level is unknown
- **THEN** Learn shows that it is waiting for low
- **AND** a fall followed by a fresh rise starts the take

#### Scenario: Further cycles or duplicate edges

- **WHEN** the same gate cycles again without a new arm
- **THEN** the instance does not publish another take
- **AND** duplicate highs and lows cannot start or settle the same take twice

### Requirement: Take publication preserves accepted audio

The instance SHALL publish only a complete, verified stereo WAV at the capture rate. It SHALL preserve the prior accepted take through cancellation, invalid input, bounds failures and stale completions. Capture shall use bounded realtime work and SHALL settle the WAV outside the callback.

#### Scenario: Cancel an active or waiting take

- **WHEN** the user cancels or the host resets processing before the take is complete
- **THEN** the attempt is discarded and Learn explains the outcome
- **AND** the earlier accepted WAV, Song source and edits remain available

#### Scenario: Invalid or interrupted input

- **WHEN** capture reaches its duration limit, changes rate, loses a required input channel or fails validation or storage
- **THEN** the instance reports the specific failure and retains the last accepted take
- **AND** a missing gate fall cannot silently complete the take

#### Scenario: Capture contains no usable signal

- **WHEN** a complete gate span contains only silence
- **THEN** Learn reports the result and retains the last useful accepted take

#### Scenario: Restore while a take is settling

- **WHEN** the instance restores a different saved session during an in-flight capture or publication
- **THEN** that capture cannot attach a take to the restored session

### Requirement: A verified loop is accessible in Learn

Learn SHALL identify a verified Loop take using its frames, rate and duration, and expose its saved WAV. The candidate SHALL retain a verified take reference across editor reopen and supported session save and restore. An accepted Loop take SHALL select the Loop workflow. The capture candidate SHALL explain that the recorded WAV is available in Learn while Loop Perform and Loop Export await implementation.

#### Scenario: Reopen a completed take

- **WHEN** the editor closes and reopens or a supported saved session with an accepted Loop take is restored
- **THEN** Learn shows the verified take and enables access to its WAV
- **AND** the instance is idle until explicitly armed

#### Scenario: Restore a missing or damaged take file

- **WHEN** the saved Loop WAV fails verification
- **THEN** Learn explains that the file is unavailable and disables WAV access
- **AND** the instance preserves the saved reference and existing Song source and edits
