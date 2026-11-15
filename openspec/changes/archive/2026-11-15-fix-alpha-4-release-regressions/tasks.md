# Tasks

## 1. Render handover

- [x] 1.1 Fix progress when a managed IR assignment supersedes an in-flight render and Regenerate All queues other swells. Use the source-edge fixture to prove the accepted render remains available during preparation and the replacement and other jobs settle. Check related IR handover and swell engine cases for cancellation, Freeze and stale publication.

## 2. Shipped workflow fixtures

- [x] 2.1 Update the processor fixture to Arm explicitly and verify the captured interval contains the expected dry PCM.
- [x] 2.2 Update the Perform readability fixture to traverse Timing, Envelope and Sound, retaining field visibility, precision, focus and fixed Reset coverage.
- [x] 2.3 Update the storage compatibility fixture for the supported creative state versions and verify a genuinely unsupported version preserves the original state.
- [x] 2.4 Diagnose the recapture wait and verify accepted source and render changes, preserved selection identities and frozen render, and analysis dispatch only where applicable. Run the broad Swell UI case.
- [x] 2.5 Diagnose the Export native pointer sequence and verify release cancels an unstarted drag and a subsequent Save fires exactly once. Run the broad Swell UI case and related Export binding checks.

## 3. Integration

- [x] 3.1 Review the repair against the offline guide and verify the currency record. After the owner approves moving the draft to ready, run the targeted Windows Release tests and all 15 standard PR regressions and record results on the tested revision.

## Workflow follow-up

- Sync and archive the completed change through the repository workflow.
- Integrate the reviewed repairs into the alpha.4 release branch. Run the full existing packaging gate, verify the extracted ZIP, and record the consolidated exact-package FL Studio smoke separately.
