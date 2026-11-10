# Spec Delta

## MODIFIED Requirements

### Requirement: Drag and Save deliver the same current revision
Drag WAV SHALL offer the current, background-verified ready revision to the OS as soon as the original pointer crosses the drag threshold. Save WAV SHALL deliver the same composition through a native chooser with explicit overwrite confirmation and safe completion ownership. Cancelled or unsupported dragging SHALL retain the durable file and leave Save available. A failed save or stale chooser SHALL NOT replace another session's output or report success.

#### Scenario: Drag into FL Studio
- **WHEN** the user drags a ready WAV into the target Windows FL Studio Playlist
- **THEN** a real complete file is offered, placement at song origin aligns it with source and the original file survives drag cancellation or later edits

#### Scenario: Save fallback
- **WHEN** external dragging is unsupported or cancelled and Save WAV is used
- **THEN** the same composition can be saved to a chosen path, an existing file requires explicit overwrite approval and a cancelled chooser changes no published file

#### Scenario: A new revision becomes ready during a pointer gesture
- **WHEN** a user presses on a ready Save or drag surface and the ready WAV, Export pass or session changes before the click or drag threshold is reached
- **THEN** the gesture cannot open a chooser, request delivery or drag a replacement WAV
- **AND** a new eligible interaction can deliver the new ready revision

#### Scenario: A pending native save becomes stale
- **WHEN** a save was invoked by pointer, keyboard or accessibility and the native chooser returns after its original WAV, Export pass, session or workspace becomes stale
- **THEN** the completion cannot copy, overwrite or report success for either the original or replacement WAV
- **AND** cancellation or teardown leaves committed files intact

#### Scenario: A drag handoff outlives the original gesture
- **WHEN** the queued native handoff runs after the original pointer gesture was released, cancelled, superseded or invalidated by a revision, Export pass or session change
- **THEN** no OS drag starts even if a new pointer press is active
- **AND** a drag attempt cannot fall through to Save on release
- **AND** a fresh eligible Save or drag remains available

#### Scenario: Drag a ready WAV
- **WHEN** the original primary gesture crosses the threshold and its prepared WAV is still current and present
- **THEN** Export offers that verified file to the operating system at most once for the gesture without a second full-file integrity scan
- **AND** a missing ready file cannot be offered as a valid drag

#### Scenario: Move between Windows drop targets
- **WHEN** an active drag moves between a target that accepts the WAV and one that rejects it
- **THEN** the cursor shows the WAV badge on the accepting target and the stop sign on the rejecting target
- **AND** the normal cursor and dotted drag affordance return after drop or cancellation
