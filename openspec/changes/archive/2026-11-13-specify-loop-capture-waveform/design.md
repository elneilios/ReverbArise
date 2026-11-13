# Design

## Context

PR #186 implements the behavior described in the [Loop capture delta](specs/loop-phrase-capture/spec.md). CaptureWorker publishes preview telemetry for the active pass. Learn also reads the last accepted source for display and inspection.

## Goals / Non-Goals

**Goals**

- Specify which waveform and markers Learn displays while a Loop replacement waits, records, and settles.
- Preserve truthful waveform feedback when detailed coverage exceeds its display budget.

**Non-Goals**

- Changes to Loop timing, admission, settlement, persistence, or the existing Song capture contract.
- New controls, visual styles, or audio-thread work.

## Decisions

- Use the capture worker's pass-scoped telemetry for the provisional waveform. The UI displays a replacement in its own phrase-local coordinates, which keeps accepted peaks and markers from being attributed to the new take.
- Keep recorded extent and a bounded min/max summary alongside detailed coverage. The summary provides a continuous view of recorded progress when per-range detail fills up. Inspection can still report that the detail is unavailable.
- Reuse the existing cancellation, failure and restore checks to retire preview telemetry. The accepted waveform remains the source of truth after the provisional take is withdrawn or settles.

## Risks / Trade-offs

- Fragmented takes can outgrow detailed coverage. The summary preserves waveform peaks and extent, with reduced inspection detail.
- A stale preview could outlive its take. Pass identity and source-generation checks fence publication and display.
- The approval build verified live updates in FL Studio. The subsequent backward-seek and coverage-limit fixes were verified through Release fixtures at `38872d9`.
