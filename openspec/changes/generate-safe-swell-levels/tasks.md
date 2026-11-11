# Tasks

## 1. Generated starting gain

- [x] 1.1 Store a separate generated starting attenuation with verified swell revisions and route it through shared shaping. Cover persistence, reset, and frozen audio with a round-trip fixture.
- [x] 1.2 Compute headroom on the worker for complete Song and Loop generation batches, including fixed-grid partial passes and variable-phase bounds. Admit only matching complete revisions and preserve retained swell gains. Cover correlated overlap, sparse peaks, cancellations, add/replace and no available headroom with focused fixtures.

## 2. Playback and export

- [x] 2.1 Make Song and Loop use accepted per-swell starting gains and explicit Live Wet, removing the shared playback attenuation and combined dry/wet guard. Cover an exact +12 dB edit, unchanged disjoint wet and unity dry in hot mixes and stop transitions.
- [x] 2.2 Use the accepted gains in Audition and export. Verify a boosted float sum survives playback and the WAV, and Loop Wet 100% uses the Perform wet reference.
- [x] 2.3 Bind measured overload to existing Diagnostics and monitoring feedback roles and update the musical gain staging guide. Verify status uses measured wet and output peaks without changing gain.

## 3. Integration

- [ ] 3.1 Verify the integrated changes at the repository review gates with the focused native regression set and one consolidated FL Studio smoke covering default headroom and deliberate overload. Record observed and pending checks separately.

### Validation on 2026-11-11

- Native MSVC Debug CTest passed all 10 focused cases, including the Level audio paths, Song and Loop playback, Audition, export and IR handover. The Audition fixture passed after correcting its expected sample to account for the existing edge ramp.
- The native Release VST3 built, was installed to the system VST3 folder and matched the built bundle. Full Release CTest was outside the focused test run.
- The owner confirmed in FL Studio that Swell 1 became louder at +12 dB and the other swells stayed at their previous levels. This was a listening check of the reported Loop regression. Default peak and deliberate overload were covered by native tests. No host peak measurement was reported.
- The standard regression set remains pending the PR readiness review.
