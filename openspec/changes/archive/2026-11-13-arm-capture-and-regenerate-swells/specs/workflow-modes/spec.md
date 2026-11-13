# Spec Delta

## ADDED Requirements

### Requirement: Learn capture requires a one-take arm
Song and Loop SHALL require an explicit Arm request in Learn before recording. The request SHALL pin the capture mode and replacement permission for one take. Fresh instances and restored projects SHALL start disarmed. The arm SHALL expire on settlement, cancellation or restore. Each capture mode SHALL retain its verified timing and ending rules.

#### Scenario: Open or return during transport
- **WHEN** a fresh instance is inserted, a saved project is restored or an idle instance returns to Learn during compatible playback
- **THEN** no recording starts until the user explicitly arms, including during FL Studio Pattern playback
- **AND** subsequent Play, seeks and repeats alone cannot grant capture

#### Scenario: Change the pending capture configuration
- **WHEN** a take is armed, recording or settling
- **THEN** the pinned mode and replacement permission cannot change and a duplicate arm cannot reserve a second take

#### Scenario: Cancel a waiting take
- **WHEN** the user disarms before recording
- **THEN** the next valid transport or timing observation cannot start that take and the accepted source remains available

## MODIFIED Requirements

### Requirement: Generation authority cannot outlive its workflow
The explicit Learn action and eligible Stop generation SHALL share readiness and cancellation guards with distinct replacement policies. Only an idle Learn request with matching final source/settings/timing/IR authority can publish. Capture arming, incompatible mode changes, restore, cancellation or changed dependencies SHALL invalidate pending publication and prevent partial definitions or stale Perform entry.

#### Scenario: Restart or switch while generation prepares
- **WHEN** a valid mode change, capture arm or restore supersedes a held generation request
- **THEN** no prepared definitions publish and no stale completion changes the accepted mode

#### Scenario: Manual navigation without generation
- **WHEN** an idle Learn user selects Perform or Export while candidate analysis is pending
- **THEN** existing readiness rules govern navigation, pending generation is invalidated and no automatic generation is implied

#### Scenario: Duplicate Stop or unsuccessful generation
- **WHEN** Stop repeats or generation is empty, protected-only, unchanged, failed or cancelled
- **THEN** no duplicate transaction commits, prior swells remain and the generation path stays Learn with its honest outcome
