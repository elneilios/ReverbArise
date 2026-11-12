# Spec Delta

## ADDED Requirements

### Requirement: IR replacement feedback reflects the accepted sound and effective shape

The Perform workspace SHALL distinguish a pending IR request from the accepted IR and present the accepted effective waveform, handles and inspector values together. A successful automatic fit SHALL be indicated as status with access to the reason for the adjustment.

#### Scenario: Pending then fitted
- **WHEN** a shorter valid response is requested for a swell with eligible accepted wet audio
- **THEN** status identifies the request and the retained response during preparation
- **AND** after acceptance the selector, waveform, handles and inspector agree with the effective shape, and status explains that timing was fitted to the response

#### Scenario: Superseded request
- **WHEN** an older IR request completes or fails after a newer selection supersedes it
- **THEN** the older result leaves current feedback and controls untouched

### Requirement: IR replacement errors explain the cause and recovery

If an IR replacement cannot be prepared, the workspace SHALL identify the requested IR and the specific failure in plain language, identify whether the previous sound remains available, and give a feasible next action where one exists. A fit that succeeds SHALL clear any obsolete error.

#### Scenario: Invalid or unreadable response
- **WHEN** the chosen file is missing, unreadable, corrupt or uses an unsupported format
- **THEN** the feedback identifies the applicable file problem and directs the user to find a readable copy, re-export to a supported format or choose another response as applicable

#### Scenario: Source or resource limit
- **WHEN** valid preparation fails because verified source coverage is unavailable or the render exceeds the working budget
- **THEN** the feedback explains the actual limit and gives the appropriate recovery, such as restoring or recapturing missing source or choosing a shorter response
- **AND** the workspace reports retained audio only where it remains available
