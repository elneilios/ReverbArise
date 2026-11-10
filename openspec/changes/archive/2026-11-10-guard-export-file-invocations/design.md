# Design

## Context

C8 owns the native file gesture and chooser boundary. Export's existing background worker verifies immutable published WAVs before it marks them ready. The old drag path queues a second integrity scan before handing off to the OS. The approved preview starts immediately, shows WAV and stop cursors based on target eligibility, and restores the dotted tile when the gesture ends.

## Goals / Non-Goals

**Goals**

- Keep gesture identity in the tile and delivery identity in the workspace binding. Keep actual file authority in the engine.
- Exercise the production continuation deterministically with controlled chooser and OS drag results, and review the revised gesture in one native fixture.

**Non-Goals**

- The scope is E2 and the Export paths in N3. P7 list and scroll work, Learn/Perform/Export waveform navigation, other chooser families and Settings/Diagnostics shells belong to their respective slices.
- Composition math, waveform paint work, managed file publication and DAW storage are outside this change.

## Decisions

1. **Capture the accepted file when input starts.** Reuse the shared command invocation hook to pin session, pass and the prepared file for Save and tile activation. Capture the original primary press for drag threshold detection. Use the same guarded continuation for the native chooser and focused tests. Check the captured identity before opening the chooser and pass it through to the existing engine guard at completion. A stale click should end quietly and allow a fresh invocation.
2. **Use the ready file at handoff.** Resolve the pinned revision, session and pass through the engine under its existing lock. Release the lock before touching the native drag surface. Check that the file is present and the original press is active when starting the OS gesture. Drag does not queue the delivery worker or reread WAV contents. Save uses its existing guarded worker and overwrite handling. Keyboard and accessibility Save are fresh commands.
3. **Use Windows target feedback for the cursor.** Offer the path through native copy-only file drag. Use the target's negotiated drop effect for the approved WAV and stop cursors. End or cancel the original press and clear solid pointer feedback when the native loop returns, including on failure. Preserve separate keyboard focus visibility and Save fallback. The accepted JUCE simulation provides the look and feel checkpoint. Native Windows payload and cursor negotiation are verified separately.

## Risks / Trade-offs

- A file removed after readiness must fail gracefully. The handoff uses the worker's prior integrity check and does not detect external edits to the ready WAV through a second full-file scan. Managed paths remain immutable under plugin ownership.
- The native drag loop can dispatch messages. Do not hold model locks across it, capture only immutable path data for the drag, and guard updates to the source UI if its owner closes during the loop.
- The accepted preview simulated drop results. Verify real Windows file payload and target feedback independently, and record actual FL Studio and physical DPI coverage separately.

## Migration Plan

Prove the gesture and binding in focused tests and show one native review before wiring the revised interaction into Export. Then verify the production workspace against the existing engine and update the C8 inventory and standards. Later P8 slices can audit the remaining IR/native surfaces and canvas and shell interactions independently. Reverting this slice requires no data migration.
