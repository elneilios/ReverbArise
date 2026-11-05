# Spec Delta

## ADDED Requirements

### Requirement: Failed capture retains actionable diagnostic evidence
The system SHALL retain bounded first-failure evidence independently of provisional waveform data: reason, pass/epoch, accepted source identity/revision, attempted frame range, queued/dropped block counts and available host timing. Missing timing SHALL remain explicitly unavailable. Later symptoms SHALL NOT overwrite the original failure. Evidence SHALL survive settlement and retry until another failure or explicit session reset supersedes it.

#### Scenario: Queue pressure after visible recording progress
- **GIVEN** a pass has displayed provisional progress and staged multiple chunks
- **WHEN** capture admission fails because its queue is full
- **THEN** diagnostics retain the queue-full reason and failure-time context after the provisional waveform is discarded
- **AND** previously accepted references, source/context and immutable history remain unchanged

#### Scenario: Distinct failure reasons
- **WHEN** invalid host timing, incompatible rate, unsupported block size, invalid samples, storage/validation failure or a recoverable worker exception rejects a pass
- **THEN** evidence identifies that cause and its available context rather than substituting a generic queue failure
- **AND** later drops or settlement errors do not replace the first recorded failure

#### Scenario: Retry and session ownership
- **WHEN** a failed pass is followed by a successful retry
- **THEN** the earlier failure remains inspectable with its original pass identity and is not presented as a current failure
- **AND WHEN** restore or explicit session reset supersedes that session
- **THEN** stale completion cannot attach its evidence to the newly active session

### Requirement: Failed provisional capture feedback identifies durable coverage
After capture failure, user feedback SHALL identify that the in-progress pass was not accepted, distinguish discarded provisional progress from retained committed source, and provide a readable cause and retry guidance. If no source had been committed, feedback SHALL say so rather than imply retained audio. Display and diagnostics SHALL NOT grant access to rejected source as accepted audio.

#### Scenario: First pass fails
- **GIVEN** no accepted source exists and Learn displays provisional audio
- **WHEN** that pass fails
- **THEN** provisional audio disappears and feedback explains that no source from the pass was accepted and no earlier committed source exists

#### Scenario: Failure with prior committed coverage
- **GIVEN** earlier accepted source exists
- **WHEN** an additive or Recapture pass fails
- **THEN** its provisional overlay is discarded, earlier committed coverage remains visible and readable, and feedback identifies the retained source separately from the rejected pass

### Requirement: Failure reporting preserves bounded realtime processing
Capture failure observation SHALL use bounded metadata without callback allocation/deallocation, locks, waiting, file access or per-block logging. Formatting, storage inspection and publication SHALL occur outside realtime processing. Diagnostic capacity exhaustion SHALL NOT corrupt accepted source or publish an incomplete pass.

#### Scenario: Repeated failures under delayed worker pressure
- **WHEN** the capture worker is delayed and callbacks repeatedly encounter failure
- **THEN** callback failure reporting remains bounded and nonblocking, dry passthrough continues and the first failure is retained coherently
- **AND** stopping and retrying after the pressure is removed can accept a new complete pass without changing older immutable snapshots
