# Spec Delta

## ADDED Requirements

### Requirement: IR replacement fits wet shape to the available response

Replacing the IR of an editable swell SHALL derive a valid effective shape from the user's shape intent and the verified response bounds. The swap SHALL preserve the swell identity, source interval, arrival, placement, activity, gain, curves and compatible timing edits. It SHALL recover the intended timing wherever supported by a subsequent response.

#### Scenario: Explicit pre-roll exceeds a shorter response
- **WHEN** a supported replacement produces wet audio with a later start than the requested explicit wet start
- **THEN** the swell accepts the IR and uses the earliest supported start while keeping any compatible end and fitting fade lengths to the usable interval
- **AND** a wet region with no overlap uses the available full response and a coherent fitted envelope, with the arrival marker fixed

#### Scenario: Return to a longer response
- **WHEN** a later replacement supports the original intended bounds and fades
- **THEN** those bounds and fades are restored, independent of intervening IR selections

#### Scenario: Edit after a fit
- **WHEN** the user changes a fitted timing control
- **THEN** the explicit edit becomes the new intent for that control, and unrelated timing intent remains available for future swaps

### Requirement: One edit undoes or redoes an accepted IR replacement

The plugin SHALL expose a single undo and redo action for an accepted per-swell IR replacement. Each action SHALL restore the corresponding IR, shape intent and effective shape together. Failed or superseded requests SHALL leave history unchanged.

#### Scenario: Undo and redo a fitted swap
- **WHEN** the user undoes and then redoes a completed long-to-short swap
- **THEN** undo restores the long IR and its prior shape, and redo restores the short IR and its fitted shape
- **AND** neither action changes a neighbouring swell or overwrites unrelated edits

#### Scenario: Freeze while browsing
- **WHEN** the user tries to replace the IR or undo or redo an IR swap on a frozen swell
- **THEN** the frozen render is preserved and the user is told to unfreeze before changing the response
- **AND** after unfreezing the edit is eligible under current source and timing checks

## MODIFIED Requirements

### Requirement: Selected-swell audition respects edits and activity controls

Manual audition SHALL anchor to the selected swell and apply edited shape/gain and protected levels. Membership, isolation, contextual dry and listening tails SHALL follow audition-monitoring. Disabled, muted or solo-excluded wet SHALL not audition. Selection or explicit audible-shape changes SHALL stop audition and require explicit restart. An accepted IR replacement and its fitted shape SHALL hand over active audition. Live input SHALL remain unchanged and preview SHALL never enter capture.

#### Scenario: Audition shaped selection
- **WHEN** a current enabled swell with cropped bounds and fades is auditioned
- **THEN** the prepared listening interval loops with those edits and the selected monitoring mode, without requesting host transport changes, and Stop ends only preview output

#### Scenario: Change selection or mute while playing
- **WHEN** the user selects another swell, explicitly changes shape, disables/mutes the active swell, or excludes it through solo
- **THEN** active audition stops without starting another swell automatically or freeing its audio on the callback

#### Scenario: Replace an IR while Audition is active
- **WHEN** an active audition contains the swell being updated and the latest compatible IR replacement completes
- **THEN** audition crossfades to the prepared replacement at the current listening position, wrapping into the new listening window if necessary
- **AND** neither a pending nor failed replacement interrupts the accepted mix
- **AND** a Stop or change of selection takes precedence over the handover
