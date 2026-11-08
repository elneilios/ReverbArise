# Spec Delta

## ADDED Requirements

### Requirement: Accepted takes share the production Perform workspace

Perform SHALL use the same Swells list, timeline, Overview, inspector, Settings and Audition controls for accepted Song and Loop takes. Loop Perform SHALL provide Cue Loop, Stop Loop and reference status beside Audition in the existing row. The approved compact composition SHALL fit at 900×600 and at supported user scales.

#### Scenario: Open Perform after accepting a Loop

- **WHEN** the user opens Perform with a verified accepted Loop
- **THEN** the standard Perform editor SHALL show its usual controls alongside the Loop Playback controls
- **AND** the ruler and Overview SHALL show the phrase, its exclusive end and any negative wet lead-in

#### Scenario: Open Perform after accepting a Song

- **WHEN** the user opens Perform with an accepted Song
- **THEN** Perform SHALL show the existing Song controls and transport readouts

### Requirement: Loop takes support the full manual swell workflow

An accepted Loop SHALL support the same manual swell operations, control semantics and selection behavior as Song. Edits SHALL use the verified phrase time domain. Musical labels and rhythmic snapping SHALL require verified musical timing. Phrase-local time and exact edits SHALL remain available with a phrase-start pulse alone.

#### Scenario: Edit several swells on one Loop

- **WHEN** the user adds, selects, renames, moves, adjusts or removes swells on an accepted Loop
- **THEN** the same list, timeline and inspector actions SHALL apply to the selected identities
- **AND** independent source bounds, target, placement, pre-roll, wet end, fades, curves, tension, level, IR, activity, Freeze and reset SHALL retain their established meaning

#### Scenario: Open Sound or Audition for a Loop swell

- **WHEN** the user assigns an IR or auditions selected material from the shared editor
- **THEN** the supported bundled and imported IR choices, audition mix, listening scope and protection SHALL follow the existing Perform controls
