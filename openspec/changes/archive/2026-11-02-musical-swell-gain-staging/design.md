# Design

## Context

See proposal.md for the musical problem. The implementation currently has:

- `Shape::gainDb` default 0, validated and exposed from -60 to +12 dB; reset already means 0 dB with the short release.
- `prepareLiveSegments` computes the largest sum of overlapping members' whole-render peaks, then limits that bound to -6 dBFS. Those peaks need not coincide, so this can substantially underestimate the usable level. User gain is already represented in the peaks; boosting a protected lone swell can therefore be cancelled.
- `TransportPlayback::configure` and session validation restrict live wet level to -60..0 dB. `AuditionMix` separately scans actual samples, applies a -6 dBFS wet guard, blend and another sum guard. These are different protection contexts.
- Strict state versions 1..7; no gain-policy identity. The current library change preserves approved PCM and accepted-version handover through live and offline preparation. That work is a dependency, not something to replace.

## Goals / Non-Goals

**Goals:** A stronger audible default, useful Level travel before genuine safety limits, reproducible shared wet gain, and sample-tested preservation of old sessions. Keep 0 dB meaningful and reset simple.

**Non-Goals:** Continuous loudness riding, arbitrary full-scale normalization, compressors/limiters, dry processing, new latency or a redesigned editor. Short-window RMS is a practical source-relative intensity estimate, not a psychoacoustic loudness guarantee. A finite wet ceiling necessarily limits sufficiently large boosts; expose that fact instead of promising gain beyond the ceiling.

## Decisions

### 1. Separate calibration from the user's Level value

Add a session gain policy with immutable revision identity: legacy or a specifically identified musical method. The revised preferred candidate is local source-energy matching; fixed +6 dB remains the listening reference and peak matching remains a recorded prototype. Calibration is independent of `Shape::gainDb` and applied once to the nondestructive wet path. New automatically generated and manually added swells in a musical instance use the approved method at displayed 0 dB Level. Existing reset semantics remain 0 dB, rather than filling the knob with a hidden offset. Matching can require attenuation as well as boost, so its baseline is not necessarily positive for every IR/source pair.

Do not bake calibration into IRs or raw render assets. The fixed candidate preserves authored relative levels; the source-referenced candidate intentionally reduces recording-level differences while retaining timbre, decay, width and waveform identity. Both must be heard before approval. Neither matches arbitrary full scale. Gain/policy edits invalidate shaped/gain preparation only and reuse raw convolution, including frozen verified renders.

For the retained peak prototype, measure stereo-linked maximum absolute dry amplitude D in a local attack interval starting at the target arrival, bounded by verified source coverage, source end and the next confirmed target. Its existing 100 ms interval and creation-default wet peak W give `baselineDb = 20*log10(D/W) + T` at provisional T=0 dB. Retain those results for comparison without interpreting equal peaks as equal intensity.

For the revised energy candidate, use the same bounded local dry attack interval, initially up to 100 ms. Measure the maximum sliding-window RMS separately in each channel, then take the maximum across channels to obtain one stereo-linked dry amplitude D. Measure W with the identical RMS window and hop across the creation-default shaped wet support at neutral Level, zero placement, before calibration/protection; apply the same 2 ms wet-edge safety ramps used for audible wet preparation. Start with 20 ms rectangular energy windows and a 1 ms hop, rounded deterministically to frames, always including the final valid window. If either eligible interval is shorter, use the smaller common window length for both signals; never pad missing audio with zeroes or dilute one side with a longer denominator. A reference requires at least one verified frame. Retain dry/wet sample peaks separately for protection and diagnostics.

Set `baselineDb = 20*log10(D/W) + T` with initial target offset T=0 dB, then bound it. Do not average the whole reverb tail: appending silence must not raise the baseline. The strongest short energy window accommodates IR density and crest-factor differences better than a single peak, but spectral balance and duration still influence perception. Tune T against the user-preferred fixed +6 dB reference; do not assume zero RMS offset is the final musical default. Source amplitudes scaled by k with the same IR must retain k-scaled calibrated output; IR PCM scaled by k must receive compensating baseline gain and yield equivalent output, provided floors, caps and protection are inactive. Test positive/negative IR polarity, stereo asymmetry, sparse spikes, dense tails and zero-padding explicitly.

Use finite checks, source/wet RMS floors and a maximum upward calibration bound. Start energy trials with -60 dBFS RMS floors, a -60 dB lower baseline bound and +24 dB maximum boost; these, the window and T are provisional tuning values that must be recorded and pinned with the listening-approved revision. Silent/near-silent dry or wet does not trigger upward matching. Insufficient or unavailable local coverage makes matching explicitly unavailable rather than borrowing another event or fabricating silence. Either bound is disclosed as capped, not falsely reported as an exact match. Strong dry peaks and overlapping wet can still be constrained by final aggregate protection.

Persist the baseline and its measurement identity per swell: accepted raw/source/IR identities, actual dry reference interval, default-envelope revision, method/policy revision, measured dry/wet RMS and sample peaks, and effective window/hop frame counts. Hold it fixed through user Level, fades, tension, crop and wet placement edits; those edits must remain audible rather than be normalized away. Recompute only for genuinely changed source/IR, source extraction/support, target reference or explicit policy adoption, and publish it atomically with the corresponding accepted audio. Identical recapture, paging and repeated render checks do not remeasure or accumulate gain. Reset uses the same calibration when its measurement dependencies are unchanged; Full response is an envelope action, not a recalibration trigger.

Freeze preserves accepted calibration with the verified render, including when source is later absent or changed. Unfreezing permits normal dependency validation and recalculation. If explicit adoption requests source matching for an old frozen result with no measurable source or saved calibration, report that swell as needing reference audio; do not silently substitute a fixed baseline or manufacture an exact match. Existing legacy projects remain unaffected until adoption.

### 2. Calibrate with an explicit listening gate

The first review compared legacy, fixed +3/+6/+9 dB and peak matching. The user found fixed +6 dB consistently about right, then requested a source-intensity-relative algorithm for arbitrary imported IRs. This is a reference preference, not final approval of the energy method or ceiling. Produce a new production-path comparison of energy matching at documented target offsets against fixed +6 dB and the peak prototype, initially retaining the -1 dBFS wet sample-peak ceiling. Include quiet/nominal/hot sources, transient/chord/pad/busy material, lone swells, noncoincident and correlated overlaps, all six factory IRs, and sparse/dense imported responses at substantially different recording levels. No additional main-screen mode or calibration knob is assumed; Level supplies the personal adjustment around the baseline.

Measure active-window RMS, sample peak, wet-to-dry active level, branch and sum attenuation, effective gain and ceiling occupancy. For energy matching also report dry reference bounds, dry/wet RMS and sample peaks, window/hop, target offset, computed/capped baseline and any unavailable/protected result. Keep the dry reference and routing identical between comparisons. Show Wet Only and Dry + Wet; distinguish audition's 50% blend from live dry-plus-wet addition. Preserve all IR hashes. Obtain approval of the energy method/tuning or a named comparison candidate and ceiling, then pin exact parameters in the report and immutable policy revision. Do not introduce continuous dynamics during tuning.

### 3. Replace conservative peak bounds only in musical mode

Build a gain plan from the verified accepted eligible schedule: calibrated, shaped, positioned wet with selected-swell gain and the applicable path's wet output gain. Stream bounded chunks of its actual stereo sum across full support, including negative lead-in and real overlaps. Retain only the maximum absolute sample, not whole-song PCM. Member maxima can still provide conservative admission checks but must not determine musical attenuation when an exact sum fits.

For ceiling amplitude C and measured aggregate peak P, the fixed safety gain is `min(1, C/P)` for P>0, otherwise 1. Preserve relative members. This is sample-peak protection, not a true-peak or final dry-plus-wet ceiling. No dynamic limiter or clipping is introduced. Below C, Level changes have their requested dB ratio; once C is reached further overall gain is necessarily constrained and must be shown as protection, not an unexplained dead knob.

The plan key includes accepted asset/generation identities, per-swell saved calibration identities, eligibility, shape, placement, policy/revision and applicable wet gain. Its reference extent is the admitted members' full support, not the current page. Publish immutable plans with the corresponding accepted audio and calibration; page changes, seeks and block sizes do not recompute a different balance. Membership/edits create a new plan with the existing <=5 ms transition rules. Do not use a gain plan or measured baseline for a different accepted/requested IR version.

Perform scans off-callback under the shared 256 MiB reservation, cancellation and cooperative playback-prefetch rules. Cache by dependency key. No reconvolution for a pure gain edit, no allocation or file access on the live callback, and no extra latency. Measure preparation time on long overlaps. Resource failure must identify the unavailable plan rather than silently use mismatched gain or raise the budget.

Rejected alternatives: merely adding gain before the current bound (often cancelled), removing protection (unsafe wet overs), raising the ceiling alone (does not fix false overlap attenuation), and per-block normalization (pumping and export differences).

### 4. Share wet calculation while preserving routing

Use one prepared-wet gain contract for live, audition and offline. Musical live wet level expands to -60..+12 dB; selected Level retains -60..+12 dB. Retain the present -60..0 dB audition trim and linear blend semantics. Audition branch preparation uses its admitted membership, then explicit blend, policy-specific sum protection and trim. Live/offline add or route wet with unchanged dry behavior and no audition blend/trim.

For equal membership, support and corresponding unity controls, compare interior Wet Only samples across paths. Audition isolation can legitimately produce a different gain plan from the full live schedule; disclose the membership/protection context instead of pretending every blend/scope is identical. Monitoring edge/outer ramps remain explicitly separate.

Offline snapshots include the accepted gain policy, revision and plan alongside accepted audio. A pending IR replacement, gain edit or another instance must not mutate that bounce. Preserve available-member export behavior and warnings from the library/transport work.

### 5. Version state and make adoption explicit

Use strict session state version 8 with validated gain-policy/revision and per-swell measurement metadata. Give the energy method a distinct immutable revision; never reinterpret `candidate-source-peak-v1` as energy matching. Preserve supported prototype version-8 states under their original semantics, and require explicit adoption to recalculate with energy matching. Extend metadata with method-specific RMS/window/hop values; accept their absence only for revisions that did not require them. Read versions 1..7 as legacy and preserve numerical edits/output, including -6 dBFS protection and trim. A fresh instance selects the listening-approved musical revision. Save/reopen never multiplies calibration into Level or changes assets; frozen matching restores without source remeasurement. Unknown revisions, invalid method-specific measurements, mismatched identities and nonfinite/out-of-range values reject or invalidate preparation transactionally. Older binaries should reject unsupported gain revisions; retain pre-upgrade project copies for rollback.

Settings offers the instance gain policy, clearly distinguishing legacy and musical. Switching is an explicit audible action that preserves numerical settings and assets; switching back to legacy requires a valid legacy live level. If a positive musical live level is present, require the user to lower it first instead of silently clamping it. This prevents a round trip from losing an edit. Frozen gain stays editable under existing freeze semantics.

### 6. Reuse the approved UI and show effective gain

Keep the main inspector Level control, 0 dB neutral and typeable bounds. Update existing Settings live wet range by policy; do not add another main-screen gain control. Existing status and Diagnostics show policy, requested gain, calibration and protection separately when protection acts. Indicate that a requested boost is ceiling-constrained. Reset resets Level, not policy. Preview screenshots only if presentation materially changes, then ask for review under the existing visual-checkpoint preference.

## Risks / Trade-offs

Host-feedback display trial: replace independent shaped-wet normalisation with
one actual-amplitude scale shared by source and shaped wet. Add a typeable
display-only Wave zoom (-24..+48 dB, default/reset 0 dB) above the timeline.
Level changes remain visible; zoom does not follow waveform peaks or affect
audio, calibration, envelope geometry or output protection. Raw grey convolution
retains a subdued independent reference scale. Waveforms show prepared members
before path-specific aggregate/output gains, not a promise of equal peak and RMS.
This supersedes the earlier independent-scale UI trials without erasing their
test history. Obtain user trial feedback before final host acceptance.

- [Peak ceiling still constrains very hot material] -> Explicit protection indication, calibrated listening fixtures and measured ceiling occupancy; do not claim unrestricted gain at saturation.
- [Actual full-support scanning costs more than peak bounds] -> Bounded streaming, dependency cache, coalescing/cancellation, prefetch servicing and long-song timing evidence. Keep callback work unchanged.
- [A single extreme event affects a fixed schedule gain] -> Disclose the fixed protection context and test varied material. Continuous per-event riding is out of scope; seek user direction if neither baseline method passes listening acceptance.
- [New baseline changes old projects] -> Versioned legacy restoration and explicit adoption; old-state numerical output oracles remain mandatory.
- [Energy matching can still misjudge perceptual intensity or flatten intended IR loudness differences] -> Compare against the preferred fixed +6 dB reference, disclose RMS/peak/crest-factor changes and tune the target offset through listening. Preserve width/tone/decay; keep personal Level adjustment effective.
- [Matching amplifies noise or fights manual edits] -> Local-event reference, floor/cap safeguards and a saved baseline held through envelope/Level edits; freeze retains its measurement.
- [Gain and accepted IR versions diverge] -> Key and publish them together; snapshot both for offline and retain the correct plan during pending work.
- [Audition sounds different because its blend/scope differs] -> Matched Wet Only numerical tests and listening comparisons at explicitly stated controls.

## Migration Plan

1. Capture old-state output fixtures before gain implementation; retain library approval hashes.
2. Implement policy and candidate preparation behind explicit musical selection; keep production defaults provisional until the listening gate.
3. Obtain listening approval and pin immutable numerical policy values; enable that policy for fresh instances only.
4. Run full Debug/focused Release tests, compatibility and operational measurements; provide a Release host-validation build.
5. Record FL Studio playback/export/reopen reports before final acceptance/archive. Do not close the library change based solely on this change's results.
