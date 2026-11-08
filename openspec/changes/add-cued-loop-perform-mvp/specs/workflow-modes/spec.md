# Spec Delta

## ADDED Requirements

### Requirement: Accepted Loop takes use phrase-local Perform readiness

An accepted Loop take SHALL select the Loop Perform view when requested. A verified useful Loop at the current processing rate SHALL permit editing without a DAW clock. Song keeps its existing readiness and routing. Loop Export SHALL remain unavailable until its dedicated export path exists.

#### Scenario: Next capture preference is changed

- **WHEN** the user changes Enable Loop, arms, cancels or completes an unsuccessful capture
- **THEN** Perform still follows the last accepted take and its verified source

#### Scenario: Song replaces the active Loop

- **WHEN** a useful Song take is accepted after the next capture preference returns to Song
- **THEN** Song Perform and Export readiness and routing apply with preserved Song data

#### Scenario: Saved Loop source is unavailable

- **WHEN** the accepted Loop cannot be verified or its rate is incompatible
- **THEN** Learn shows the reason and Perform does not substitute a previously accepted Song or replay stale Loop wet
