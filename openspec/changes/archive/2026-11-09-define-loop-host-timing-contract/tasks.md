# Tasks

## 1. Close the gaps in Rack timing evidence

- [x] 1.1 Reuse the spike generators and traces to establish the intended CLKD/Host-FX wiring, or a reproducible equivalent. Verify all seven input roles, MIDI channel, sample offsets, and dry-to-reference alignment. Record the patch, versions, actual sample rate and block sizes, signed offsets, edge counts and any trace loss in the dated results matrix.
- [x] 1.2 Measure longer fractional-tempo operation and Stop, reset, missing or extra markers, cable disconnect, startup-high, suspend/resume and patch reload. Use the existing fixture where applicable and extend it only for missing cases. Record run length, observed interval and drift bounds, delivered lifecycle evidence, detectable failures, and unverified cases. Distinguish measured host runs from synthetic and replay checks.

## 2. Publish and verify the admission contract

- [x] 2.1 Specify the reference epoch, lock and loss bounds, invalid input behavior, and per-instance capability states and explanations in `docs/development/`. Exercise the contract in focused probe or replay cases for independent readiness, missing meter, a mismatched phrase length, valid adjacent integer periods, drift, missed markers, reset and fresh Cue. Include bounded callback work and invalid or overflowing evidence in the checks.
- [x] 2.2 Qualify capture start and end against the real gate trace. Verify held-high arming, same-sample and between-bar end requests, ambiguous disconnect and loss of callbacks before the boundary. Record which routes can establish an end request, how unavailable routes are explained, and what the user can do next. Identify FL Studio Loop MIDI/automation coverage as pending under #127.

## 3. Review the gate

- [x] 3.1 Verify compatibility with existing Song admission and sparse capture. Run the native Windows Release test wrapper and record the focused transport and capture regression results separately from the probe evidence. Validate the final OpenSpec delta and confirm that the technical guide links to the dated evidence.
- [x] 3.2 Present the evidence matrix and supported or withheld conditions to the owner. Record explicit go/no-go by configuration, the approved clock/reference/rate limits and unresolved coverage. A no-go leaves that route unavailable and blocks dependent work that needs it. Keep #123 open if the required timing contract cannot be approved.
