# Spec Delta

## MODIFIED Requirements

### Requirement: Project state preserves accepted and requested capture separately
Saved projects SHALL retain verified accepted-source metadata and the next-capture preference separately, using managed audio references. Older Song projects SHALL restore as Song with existing identities. Restore SHALL invalidate pending attempts and require a new explicit arm for Loop.

#### Scenario: Save during a pending replacement
- **WHEN** a project is saved while a new Loop attempt is armed, recording or settling
- **THEN** reopen restores the previously accepted source and saved preference without resuming that attempt
- **AND** a late completion from an invalidated attempt cannot replace the restored take or its dependent edits

#### Scenario: Open a project with an accepted Loop
- **WHEN** a saved Loop project is reopened with a verified managed source
- **THEN** the same accepted identity, revision, sample rate, extent and musical timing are restored
- **AND** the exact referenced audio, swell edits and compatible verified asset references are retained
- **AND** missing or invalid referenced material is reported as unavailable

#### Scenario: Instance isolation
- **WHEN** one instance replaces or cancels a take while another instance has a source or pending attempt
- **THEN** the second instance retains its own accepted and pending state

#### Scenario: Reopen protected and in Standby
- **WHEN** a current-build project with an accepted Loop reopens after a save with Recapture Audio enabled or Loop playback queued or active
- **THEN** Learn starts disarmed with Recapture Audio off, and neither UI nor external Start/Arm can replace the accepted Loop until permission is enabled
- **AND** live output starts in dry Standby, and wet playback requires a fresh Cue and verified live timing
- **AND** the saved capture-local grid and a held-high control at restore cannot grant live timing, Arm or Cue
