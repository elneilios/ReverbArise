# Spec Delta

## MODIFIED Requirements

### Requirement: IR choices are real and independently assigned
The selected-swell IR dropdown SHALL list verified bundled catalogue responses and its current imported assignment, with an adjacent Load file action. Selection SHALL affect only the intended swell, preserve other assignments/future defaults and edits, honor freeze and report async failures. Pending replacement SHALL distinguish requested IR from retained audible IR. No fictional entries or new DSP controls SHALL appear.

#### Scenario: Bundled and imported choices
- **WHEN** a user chooses the bundled Preverb Hall or loads a mono/stereo WAV/WavPack file for one swell
- **THEN** its real managed IR identity and label update after validated background preparation, while another swell and the future default remain unchanged

#### Scenario: Selection changes during file choice
- **WHEN** a file chooser opens for swell A and selection changes to swell B before completion
- **THEN** the assignment is pinned to A or explicitly cancelled, never silently applied to B; deleted/frozen/obsolete targets reject the completion safely

#### Scenario: Select another library response
- **WHEN** one swell receives Monumental, Intimate Plate, Vintage Dark Hall, Shimmer Cloud or Whisper through the existing dropdown
- **THEN** the real requested catalogue identity is prepared for that swell alone without resetting source, arrival, envelope, placement, name, activity or unrelated preferences
- **AND** the current custom response remains identifiable when it is not a catalogue entry, and a short description explains the chosen sound including Shimmer Cloud's static limitation

#### Scenario: Requested and audible responses differ
- **WHEN** an eligible IR-only replacement is preparing or fails while the old verified live contribution remains available
- **THEN** concise status identifies the requested response, retained audible response and preparation/failure state, rather than claiming that the requested IR is already playing

#### Scenario: Rapid choices and restored identity
- **WHEN** selection, session restoration or repeated catalogue choices supersede an asynchronous assignment
- **THEN** only the latest permitted target/session request can publish and catalogue ordering does not determine saved assignment identity
