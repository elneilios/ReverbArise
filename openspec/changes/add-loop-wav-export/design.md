# Design

## Context

See proposal.md for motivation and the two capability deltas for behavior. `PluginProcessor::getWorkflowReadiness` currently returns no Finalise eligibility for Loop captures. `SongExportPlan` pins source, timing, members, gain and absolute bounds; `SongMix` reads bounded pages and supplies measured waveform/preview/file audio. `SwellExport` owns generation/session/pass checks, verification and safe delivery. `ExportWorkspace` binds that authority to existing preview/navigation, level controls, FileDragTile and Save.

Song export deliberately clips negative audio at song origin. `LoopWetPlan` already enumerates contributing past/future cycles for live wrap overlap, but its publication holds resident render audio. Export must reuse that cycle/composition authority without copying an entire long repeated WAV into memory. Existing `song_export_test`, `SongMixCases` and `song_export_engine_test` provide sample, metadata, ownership and preview/WAV comparison coverage to extend.

## Goals / Non-Goals

**Goals:** Keep composition coordinates deterministic and share cycle membership between live and offline paths, while sharing the established bounded export infrastructure. Separate capture-local export authority from current host playback admission.

**Non-Goals:** Altering capture or Cue/Stop semantics, baking the live output guard into Float32 files, broadly migrating Song state, or implementing #129 recovery and #130 integrated qualification. No alternative screen or navigation flow. The owner's native-review decision supersedes the original wet-only scope: the shared independent Dry/Wet pair now applies to both Preview and WAV.

## Decisions

### 1. Add a Loop branch to the existing export plan and worker pipeline

Add an explicit composition kind and immutable Loop metadata to the existing plan authority, retaining the Song branch's current identities and arithmetic. Pin accepted source/render revisions, shape/placement/calibration and the Dry/Wet pair before preparing. Adapt `SongMix` layer reads to consume finite Loop cycle contributions and repeated accepted dry source, so waveform, measurements, preview and files consume one plan. Keep ExportFiles and SwellExport's atomic publication and ownership checks.

Alternative rejected: an independent Loop writer/preview path, which would duplicate file safety and make parity harder to prove. Do not eagerly expand every repeat into resident PCM or copy the live resident publication wholesale.

### 2. Share cycle membership and coordinate conversion with Loop playback

Factor the smallest reusable pure cycle-coordinate/contribution enumeration from `LoopWetPlan` and the scheduler's exact-boundary rules. Give export a finite cycle interval [0, repeatCount), while live playback retains its existing scheduler limits. Read verified wet assets in bounded pages using that shared enumeration. A reference oracle independently sums scheduled render contributions, rather than calling the compositor being tested.

Let P be the accepted exact period and C(k) its shared absolute sample conversion for cycle k. Derive C(k) from k and the fixed accepted anchor, never repeatedly add a rounded beat period. Negative audio for cycle zero is retained. Internal tails and negative audio are counted once per scheduled cycle, and cycle repeatCount never contributes. End is at least the final scheduled Loop boundary and extends through that last scheduled pass's eligible tail; silent periodic material still preserves the requested musical extent.

When Dry is nonzero, read capture-local dry into each scheduled pass interval exactly once with the same absolute boundary authority. Keep dry zero before cycle zero and after the final pass; wet lead-in/tail enumeration does not cause extra dry reads. Reuse `ExportSourceReader`'s verified coverage and warned-silence policy in bounded pages. Dry at zero reads no dry; Wet at zero requires no wet sample reads or wet lead-in/tail extension. Add independent dry impulse/ramp oracles at shared boundaries, including fractional-period conversion, to catch duplicate or dropped source frames.

Alternative rejected: copying one cycle's audio N times, which omits wrap overlap, duplicates initial lead-in and can drift at fractional periods. Regression tests must show the extraction preserves live Loop playback.

### 3. Compute the lead-in origin from accepted musical coordinates

Use the accepted capture-local beat/meter mapping, not the current host snapshot. For earliest included wet coordinate E and selected grid G, origin is zero when E is nonnegative; otherwise floor E outward to the grid, then convert using checked accepted sample mapping. A dry-only mix has no negative wet lead-in and starts at Loop Bar 1. Handle negative floor semantics explicitly. Reserve leading silence as zero spans. Label file origin and Loop Bar 1 separately and derive the placement offset from this same plan.

Repeat count defaults to one; a one-bar lead-in grid is the initial preference when valid. The native approval gate can refine presentation/defaults without changing composition semantics. Offer verified beat/bar choices using existing selector conventions; unsupported bar choices explain missing meter. Do not substitute host meter or pretend unsupported musical alignment exists. Preserve accepted extent for non-whole-bar Duration as well as End on Signal and End on repeat takes.

Alternative rejected: quantizing to the live host grid or the nearest rather than preceding boundary, which can move Loop Bar 1 or trim initial wet.

### 4. Keep offline gain transparent and readiness independent

Loop mixes use the existing independent Dry/Wet export law: output = Dry level * repeated captured dry + Wet level * accepted calibrated wet. Both levels range from zero to unity; 100% Dry means unity, and presets change only this pair. A fresh Loop mix defaults to Wet Layer (0%, 100%). Reuse existing preset pairs and Custom matching, without introducing new sound algorithms or a second monitoring mix. Perform Live Wet, monitoring trim and the live combined output guard are excluded. Preview monitoring boundary ramps remain separate. Preserve above-unity Float32 sums and measured peak warnings as Song export already does.

At 0/0 disable current preview/Save/Drag without invalidating immutable history. Existing initial Export entry still requires eligible wet; adjusting an entered composition to dry-only does not create a new source-only entry path. A zero branch has no audio-read dependency. Loop readiness uses accepted capture-local material and verified members; live clock/Phrase availability is not an export dependency. Preview still requires an output device at a compatible rate and explains unavailable monitoring without falsely invalidating file delivery. Wet-only delivery is labelled for layering; dry-inclusive delivery tells the musician to mute/replace the original dry to avoid doubling it.

Alternative rejected: conditioning export on Cue readiness or capturing protected live output, which couples a deterministic file to changing host state and gain history.

### 5. Reuse the approved Export composition and shared control families

Use `ExportWorkspace`, ActionButton, FileDragTile, TimelineNavigation/Zoom and the existing musical ruler. Compose Repeat and Lead-in grid fields from existing numeric/selector controls with scoped bindings. Reuse `ExportLevelsControl` for Mix Character and the independent Dry/Wet faders in Loop as well as Song; remove the preview's duplicate standalone Wet slider rather than introducing another mix-control family. Preserve Song's behavior and preferences. Add WAV start/Loop Bar 1 markers and concise placement/dry-doubling guidance next to delivery. Use the current production style guide, responsive tokens and footer context-help policy; no new visual token or variant is pre-approved by this plan.

Reconcile the native preview with the [approved epic mockups](https://github.com/elneilios/ReverbArise/issues/116#issuecomment-6707861945), current production Export layout and the owner's explicit Dry/Wet revision. The original Wet-only preview passed component checks but requires revision; its look-good feedback does not approve omission of the shared mix controls. Reopen task 3.1 and launch one revised native preview with Mix Character and both faders at 900x600 and an expanded size. Use the existing responsive layout conventions to keep all controls and delivery guidance reachable without shrinking library typography or inventing compact control tokens. Test components/bindings, disabled reasons, keyboard focus and stale intents. Any necessary new family/exception must have scope, rationale and evidence approved, and standards/tests updated together, before rollout. Owner approval is a required task, not inferred from this proposal.

### 6. Add metadata narrowly and preserve immutable history

Add validated optional Loop export preferences/descriptors with explicit composition revision/kind, count, grid, independent Dry/Wet pair, exact extent and origin identity. Persist the pair, not a preset name or a separate preview-only mix. Defaults apply only when absent; malformed values fail safely. Keep Song preference values and existing file descriptors/paths/checksums intact; never reuse a Song revision identity for a Loop file. Pin every asynchronous request to session/pass/generation and plan identity. Restoration cannot autoplay or publish stale work. Broader accepted-Loop asset recovery remains #129.

## Risks / Trade-offs

- Shared compositor extraction could affect live overlap → independent sample oracles plus existing live scheduler/compositor tests and Song export regression coverage.
- Very long lead-ins, tails or repeat counts can overflow or exhaust resources → checked preflight of rate, bounds, RIFF size, disk and working budget; bounded pages and explicit failure, never truncation.
- Fractional periods and negative rounding can shift placement → absolute anchor conversion tests at 44.1/48 kHz, nonintegral tempos and multi-repeat stress cases.
- Repeated dry could double at boundaries or in wet tails → exact pass-interval source-read tests, independent dry/wet endpoint oracles and dry-doubling guidance in the approved native UI and user guide.
- Export remains intentionally unclamped unlike protected live output → preserve Float32 semantics and show truthful peak/overload guidance; no new settings.
- Host import behavior differs from component simulation → one focused FL Studio placement/preview/Drag/Save smoke using an export from a proven Clock-only Rack take; record actual host versions and separate automated evidence from host results. Physical DPI matrix remains #130.

## Migration Plan

Implement and test on the issue branch from `feature/loop-mode`. Obtain native UI approval before rollout; validate additive metadata with legacy fixtures and stale restore/write cases. Update living export guidance and user-guide appendices, recording dated evidence. Run UI standards and full Release gates, then open a PR against `feature/loop-mode` and leave it open for review. Rollback is branch/revert plus the existing retained test VST3 bundle if installed; immutable user files and durable assets are never deleted as rollback.
