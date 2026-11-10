# Design

## Context

The base is `feature/loop-mode` at `db9c57e`, which includes #133. `PreverbAudioProcessor::processBlock` currently disables Song timing for an accepted Loop. `SwellEngine` owns shared render preparation and transport mixing. The capture `PhraseClock` validates the legacy marker route against a fixed 16-beat phrase. The #116 playback spike established a timing reference for a single swell, including measured Rack runs at 137 and 280 BPM with adjacent phrase periods.

## Goals / Non-Goals

**Goals:** Give the processor a headless scheduler with separately observable reference, prepared audio and command readiness. Use a deterministic wet compositor that can also support #128. Exercise the production path with sample-level tests and measured Rack inputs.

**Non-Goals:** New Learn or Perform controls, FL Studio routing certification, WAV export, new persistent fields, or changes to the capture and Song paths. Those boundaries keep this change usable as a dependency of #126–#129.

## Decisions

### Separate the stored grid, live reference and command route

Use the accepted Loop identity and exact exclusive extent as immutable source data. Feed an independent per-instance live reference from the verified marker route using the existing sample-offset MIDI roles and reset. Require a configured phrase length and shared origin for this route. Reuse the #123 recurring phrase-marker cadence rules for live use. Check the adjacent phrase intervals and the fixed one-sample epoch bound. Intermediate capture beat observations do not grant or revoke playback phase. Capture Clock/Start/Stop carry no Perform intent. Existing Perform Cue and Stop remain distinct positive-edge roles. Local headless Cue and Stop use the same scheduler regardless of whether external commands are configured.

Verify two complete matching phrase intervals. Match sample rate, explicit meter and beat length, and require a recorded frame count that is one of the adjacent integer lengths at the verified live tempo. Account for the capture tempo's measurement resolution when comparing it with the live tempo. For example, the accepted Rack one-bar take stores about 136.99933 BPM for 84,088 frames at 48 kHz on the verified 137 BPM route. Compare the estimated period and the independently verified live period against the recorded integer extent. Use the declared or measured live tempo to check live markers, with the fixed-epoch drift bound and no added BPM tolerance. If rate, meter, origin, length or timing cannot be verified, report the missing authority and stay dry. Check the missing-marker deadline before Cue and throughout Standby, lead-in and repeat.

### Schedule from absolute callback samples and observed boundaries

Keep the runtime epoch and prepared revision separate. For a Cue at sample `c`, choose the first future matching boundary whose earliest allowed arrival minus the longest negative lead is at or after `c`. Run the lead-in from that deadline. Process invalidation first, verify the marker, then apply Stop ahead of Cue at a shared offset. Only a verified observed marker anchors the wet downbeat. Use the earliest-to-latest arrival window from the stable epoch and allow only the bounded endpoint correction exercised in the CLKD reference. Retain each original lead sample, with a short hold of the final lead sample when needed, then anchor non-negative positions at the actual marker. An arrival outside the window revokes sync. Expose the correction in validation and keep the prepared PCM at its native rate.

Use dry Standby, queued, lead-in, repeating and stopping as scheduler states. A Stop during a queued launch clears it immediately. An active Stop or loss of authority starts a wet fade at the event sample using the existing click-prevention duration ceiling. Reset, seek, processor or session interruption, incompatible timing and workflow exit invalidate the relevant epoch. A recovered reference requires a fresh Cue. A prepared revision alone never triggers playback.

### Reuse prepared wet and bound periodic work

Prepare a coherent immutable set of eligible wet entries from the shared render and cache owner. Preserve placement, IR, envelope, gain, mute and solo semantics. Reuse the transport publication and retirement pattern so that callback leases do not allocate, free or own disk-backed data. Count pinned publications against the shared PCM budget. The processor routes accepted Loop live wet through this adapter and keeps the Song call path and capture ordering intact. Use a real accepted source and shared preparation in integration fixtures while editor creation and control bindings are pending in #126.

Give each contribution a cycle ordinal and source-relative half-open interval. Sum all intersecting contributions from the first scheduled cycle forward, including leads for future cycles and tails from earlier scheduled cycles. Use the same interval intersection and mix logic for repeat and for later finite export. Precompute the lookup data and an upper bound on simultaneous contributions across the allowed phase windows. Reserve bounded active state and reject plans beyond the existing overlap and PCM budgets before Cue. Preserve a pinned coherent revision while it is valid. If edits or source replacement revoke its dependencies, cancel or fade it and let the next Cue select a complete current publication. Reuse the existing eligible IR-only retention rule while its dependencies remain valid.

## Risks / Trade-offs

- Negative lead-in begins before the marker can be observed. A missing marker can invalidate wet already started for its expected window. The deadline stops further playback with the bounded fade. The tests cover missed markers after lead-in begins.
- A single-swell proof does not establish overlap, long-tail or resource safety. Compare the production compositor against an independent summation oracle with mixed positive and negative offsets, tails beyond multiple cycles, both measured fractional periods and varying callback partitions.
- Matching a captured extent alone could mistake another clock for the source clock. Require independently verified live recurrence and a shared phrase origin, and test absent and mismatched references separately from valid capture.

## Migration Plan

Use the existing accepted Loop metadata and sample-relative positions. Reopen starts from Standby and acquires a fresh live epoch. Keep new runtime state ephemeral. Place measured Rack replay and supported-profile evidence beside the existing #123 and #133 evidence. The production UI and broader supported-host matrix follow in their assigned issues.
