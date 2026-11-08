# Spec Delta

## Purpose

Let a musician prepare a reverse-reverb swell from a captured phrase and cue it against the same repeating part in a live patch. Loop playback uses an explicit phrase reference independent of Song transport.

## ADDED Requirements

### Requirement: Loop editing uses its accepted phrase

Loop Perform SHALL use the verified stereo take, its sample rate and its local interval from zero to the exclusive loop end. It SHALL let the user place, shape and audition a manual swell with its full negative lead-in. Editing SHALL remain available while an external reference is absent.

#### Scenario: Prepare before syncing

- **WHEN** a useful Loop take is verified and the patch supplies no phrase marker
- **THEN** the user can prepare and audition the swell and Perform explains that synchronized playback is waiting for a reference

#### Scenario: Source changes during a render

- **WHEN** recapture, restore or a newer edit supersedes an unfinished preparation
- **THEN** the old result cannot become playable for the new source or edit

### Requirement: Phrase reference establishes exact sample phase

Synchronized playback SHALL require a tested recurring phrase marker whose period matches the accepted take at the current sample rate. The first local MVP SHALL accept a stable integer-sample period. Capture Gate, Phrase Start, Cue and Stop SHALL have distinct meanings.

#### Scenario: Capture gate covers only part of the reference cycle

- **WHEN** consecutive phrase markers do not agree with the accepted take length
- **THEN** Perform explains the mismatch and stays in Standby with dry passthrough

#### Scenario: Clock is lost

- **WHEN** a marker is missing at its expected sample, arrives at a conflicting phase, or processing resets
- **THEN** queued playback is cancelled, playing wet stops with a bounded fade and a fresh matching reference is required

### Requirement: Cue preserves the full leading swell

With a verified prepared swell and a matching reference, Cue SHALL schedule the earliest future phrase boundary that leaves enough samples for the whole negative lead-in. Standby SHALL track the reference with dry passthrough. Cue SHALL leave capture and host transport unaffected.

#### Scenario: Late cue

- **WHEN** Cue arrives one sample after the full lead-in could begin for the next phrase boundary
- **THEN** playback waits one more complete phrase and starts the lead-in at its original local negative position

#### Scenario: Matching launch

- **WHEN** the queued lead-in finishes and the expected marker reaches Bar 1
- **THEN** the first non-negative wet sample coincides with the shared phrase start

### Requirement: Repeat reuses negative wet without duplication

Prepared Loop playback SHALL reuse the leading negative samples at the end of the preceding cycle and sum them with other wet contributions at the same positions. It SHALL preserve the original lead-in for the first cue and report unsupported span or resource limits before playback.

#### Scenario: Negative wet overlaps body wet at the wrap

- **WHEN** prepared negative and non-negative contributions meet before the next phrase begins
- **THEN** each sample contributes exactly once and the first launch still has its independent full lead-in

### Requirement: Stop cancels cue or fades wet

Stop SHALL cancel queued launch or fade sounding wet within 5 ms while preserving dry audio. Stop SHALL take precedence over Cue at the same sample. Capture gate transitions and ordinary recurring phrase markers SHALL never stop the performance.

#### Scenario: Stop before lead-in

- **WHEN** Stop is received while Cue is queued
- **THEN** no wet is emitted for that cue and the accepted take and prepared swell remain available

#### Scenario: Stop while sounding

- **WHEN** Stop is received during the lead-in or a repeat
- **THEN** wet stops within the bounded fade, dry continues and playback waits for another explicit Cue

### Requirement: Loop playback state stays ephemeral

Project restore SHALL retain the accepted take and its matching Loop edit and clear active playback and cue state. Only complete verified renders for the current take and edit SHALL enter the bounded real-time mix.

#### Scenario: Reopen during a performance

- **WHEN** a project is saved while queued or sounding and later reopened
- **THEN** it can restore and verify its edit, remains silent in Standby and waits for a fresh reference and Cue
