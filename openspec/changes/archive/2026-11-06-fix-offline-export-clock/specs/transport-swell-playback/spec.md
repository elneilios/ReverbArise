# Spec Delta

## ADDED Requirements

### Requirement: Offline bounces validate their own runtime mapping
Each fresh host offline bounce SHALL establish a valid fixed-tempo runtime mapping independently of parked/source PPQ origin. Assets SHALL retain compatible rate, tempo and meter. Subsequent wet-required callbacks SHALL match the established bounce mapping within one sample. Stored source/render positions and identities SHALL remain unchanged.

#### Scenario: Export origin differs from prepared origin
- **GIVEN** compatible ready wet at absolute sample positions
- **WHEN** a fresh offline run supplies a valid origin offset, including the observed 16-sample difference at 48 kHz and 122 BPM
- **THEN** available swells render at their original absolute sample offsets with either Playing flag value and irregular blocks
- **AND** unchanged source/render assets, calibration and gain plans are retained without compensating sample shifts

#### Scenario: Genuine drift during required wet
- **WHEN** an established offline run changes origin beyond one sample, rate, tempo or meter while wet remains current or due
- **THEN** export fails visibly with precise failing timing and does not silently replace the baseline
- **AND** the existing completed-wet tail exception remains in force

#### Scenario: Lifecycle and separate instances
- **WHEN** offline processing is entered anew, preparation restarts, or an existing supported backward export restart occurs
- **THEN** the instance establishes a fresh mapping independently of other instances
- **AND** zero-length/invalid callbacks cannot admit wet, while accepted pending IR contributions retain their original snapshot rules

### Requirement: Offline timing failures are shareable
Offline timing errors SHALL retain the prepared, bounce-reference and failing host timing values. Diagnostics SHALL offer a keyboard-accessible Copy diagnostics action that copies the full current report and retained error without changing saved session state.

#### Scenario: Copy a failed bounce report
- **WHEN** the owner invokes Copy diagnostics after offline timing failure
- **THEN** the clipboard includes precise timing/error information, current capture/gain observations, Snap/grid availability and live/worker observations
- **AND** the action changes no source, swell, routing, transport or saved preference
