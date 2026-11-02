# Design

## Context

See proposal.md for motivation and scope. No active implementation change existed when this change was scaffolded. Current resources embed one deterministic stereo 48 kHz/16-bit, 86400-frame PreverbHall.wav; its WAV SHA-256 is b7275a27fc7629a9a1a7191abda4fbb3c8a46372dc9071e36dc1112e4839b458. DefaultImpulse hardcodes this checksum/format, the inspector hardcodes one bundled choice, and import jobs represent bundled selection with a boolean. Managed IR/render assets already carry content identity; saved version-7 sessions contain independent IR references and names.

The renderer reverses the IR and performs full linear convolution: N+M-1 frames at P-(M-1). Its current resampler preserves the origin and mono/stereo mapping. The working budget is 256 MiB, including occupied caches and a conservative source-decoding allowance. Current assignIR stops preview and invalidates that swell's live publication immediately. Per-swell prepared pages and processing-boundary transitions already exist; extend them rather than building another playback engine.

## Goals / Non-Goals

**Goals:** preserve existing projects and approved UI; author meaningful reverse-swell sounds; use one catalogue across packaging/decoding/UI; retain accepted output through IR-only work with bounded ownership; document actual sound and operational evidence.

**Non-Goals:** runtime algorithmic reverb, pitch shifting, modulation, new EQ/width/decay controls, new mode layouts, a global sound-preset system, changing external-IR gain, arbitrary memory-budget increases, or changing existing source-window policy. Issues #3 and #6 remain separate. Existing future-default file selection remains functional; adding a separate global catalogue browser is unnecessary here.

## Decisions

### 1. Catalogue entries describe immutable released content

Introduce a small catalogue descriptor containing stable string ID, revision, display name/description, resource bytes, WAV and decoded PCM hashes, channels/rate/frames and metadata manifest reference. Replace the boolean bundled discriminator with an explicit catalogue identity in background assignment commands; keep legacy Preverb Hall entry points forwarding to its exact original content. Dropdown item numbers are temporary UI handles mapped to IDs, never persisted identities.

Populate only verified actual resources. Reuse pinned swell ID, input/session generations, freeze gating, background import and immutable publication. Per-swell assignments must not change future defaults or unrelated swells. Preserve custom imported names and the adjacent file action. Reuse the existing status/tooltip to describe requested versus audible IR without widening or adding a panel.

Alternative: another hardcoded switch per preset is initially smaller but duplicates integrity, labels, defaults and menu mapping. A file-system factory library complicates installation and allows content replacement; embedded resources keep installation self-contained.

### 2. Author original deterministic static responses

New assets use stereo 48 kHz/24-bit PCM WAVs; leave the legacy file untouched. Record explicit PRNG algorithm/seed, generator revision, every synthesis/filter/envelope parameter, rounding/quantization, authoring gain, pinned generation environment and output hashes. Supply original audio under CC0-1.0 with provenance; reuse no third-party recordings or proprietary processors. A checked reproduction command compares regenerated PCM/WAV checksums. Cross-environment byte reproducibility must be measured rather than assumed from a fixed seed alone.

| ID (initial revision 1) | Working target | Static construction to prototype | Required distinction |
| --- | --- | --- | --- |
| monumental | 10 s | Gradually dense multi-band diffuse noise/reflections with long smooth decay | Long cinematic bloom rather than a stretched copy of Hall |
| intimate-plate | 1 s | Rapid dense bright diffusion, restrained low end and compact release | Controlled lift on short notes |
| vintage-dark-hall | 4 s | Warm diffuse response with progressively damped high bands | Dark tail without a harsh final arrival |
| shimmer-cloud | 7 s | Diffuse bed plus restrained distributed upper partials | Sparkling static spectral coloration, no claim of actual pitch-shifting input |
| whisper | 0.5 s | Short, soft, spectrally smooth diffusion with limited discrete reflections | Restrained transient anticipation |

Targets may be refined within the spec's ranges after listening. Build decorrelated stereo tails with coherent arrival/reflection policy; measure L/R energy, correlation, mono fold-down and resonant peaks. No required true-stereo crossfeed: retain left-to-left/right-to-right convolution. Avoid an unintended direct impulse; existing live dry routing remains responsible for dry audio. Use smooth authoring onset/end windows and verify quantized endpoints, without trimming/fading runtime raw convolution.

Alternative: recorded IRs introduce location/source/licensing variability; commercial captures need compatible permission. Deterministic original synthesis gives inspectable provenance and repeatable iteration. Time-varying shimmer is a separate effect, not representable faithfully by this static asset.

### 3. Calibrate authored gains using reverse-rendered material

Measure each asset's exact length, per-channel peak/RMS/sum-of-squares, DC, spectrum, energy-decay times and channel compatibility. Record sample duration separately from measured audible decay. Compare full-response reverse renders of deterministic piano-like chord, sustained pad, voice-like phrase and pluck fixtures, plus user-approved real material when available. Keep test-material licensing explicit.

Use Preverb Hall as the existing reference. Provisional authoring objective: the median active-window RMS difference over common fixtures is within +/-3 dB of the reference; publish per-fixture outliers and protection gains. Define the active-window gate/window in the measurement script so long quiet tails do not merely lower a whole-file RMS score. This is a calibration objective, not a guaranteed equal-loudness assertion: intentional differences require documented listening approval, not hidden runtime normalization. Compare full responses and a common supported crop, with equal source, gain, routing and trim.

No importer/resampler normalization, limiter or quiet boost is added. Raw floating-point convolution may exceed unity; verify unmodified finite samples and disclose existing wet/sum protection independently. Do not claim monitoring safety bounds the separate combined live dry signal. If an asset requires excessive routine attenuation or sounds inaudible at normal trim, revise its authored gain/energy distribution before shipping.

### 4. Persist sound by managed content, not catalogue position

Install each selected response into the normal immutable per-instance IR store. Existing content hash/format references plus saved label are sufficient to restore exact sound without introducing a mandatory catalogue field or schema bump. Catalogue revision is release/provenance identity; managed content remains the authoritative session dependency. Updated releases use new immutable content/revision and do not rewrite older assignments. Missing managed content remains actionable missing audio, not a reason to substitute the latest similarly named preset.

Future defaults remain Preverb Hall or the explicitly saved existing default. Reset-all continues to use that current future default. Library choices preserve names, source/arrival, placement, shape, activity and all unrelated preferences. Save/reopen during work persists only committed references; pending jobs do not autoplay or resume. A requested committed new IR with an unfinished wet render rebuilds under existing restoration rules; a transient retained live fallback is not a new persisted frozen state.

### 5. Retain an accepted snapshot only for IR-only replacements

Separate the requested definition/generation from one accepted audible snapshot per swell. Capture the accepted verified IR/render, source dependency, shape/placement revision and musical mapping before queuing replacement. Pin that accepted snapshot to existing prepared-page ownership while the requested asset imports/resamples/renders/shapes. Repeated B/C requests retain A, not a chain of intermediate versions. Only the latest valid request may replace A. Preserve file-IR behavior as well as catalogue choices through the same command path.

```text
Accepted A -- request B --> A audible / B preparing
                           | request C: supersede B, retain A
                           | failure: A audible / requested error
                           | source/shape/placement/mapping change: revoke A
                           v
                   C complete and verified
                           |
                    processing boundary
                           v
                   <=5 ms handover --> Accepted C
```

This exception applies only when a ready verified contribution existed and only the IR request changed. It cannot revive stale/missing audio or override freeze. Changed source, shape, placement or timing revokes retention and uses normal invalidation. Delete, disabled/muted/solo filters and live on/off still apply. Preflight frozen assignment rejection remains unchanged. Selection does not determine ownership.

Reuse current prepared-page transitions at absolute Song offsets. Both versions need admitted current-position pages for the <=5 ms transition; do not advance/restart independent cursors or replay elapsed lead-in. Use the existing protection model with the temporary aggregate represented correctly, avoiding an unprotected duplicate sum. Retire audio/page ownership on the worker/message side. No decoder, lock, allocation, object destruction or wait enters the live callback. Manual audition still stops on IR change and never auto-restarts.

For offline bounce, snapshot the accepted audible contribution at export entry, including eligible IR-only retention. A background completion must not alter that bounce midstream. New requested-but-unready audio is not fabricated. Pending choice is disclosed; once ready it participates in the next snapshot. Keep this bounded host export path separate from the unimplemented Finalise feature.

Alternative: immediate mute matches current conservative invalidation but violates the issue's audible continuity goal. Treating all stale renders as playable violates source integrity. Holding one narrowly verified snapshot makes the exception explicit and testable.

### 6. Preserve complete tails and enforce existing budgets

Automatic shape start already follows raw start; a long IR therefore extends genuine wet lead-in without additional dry capture before the event. Preserve the base extraction policy and completed-loop boundary handling. Existing explicit manual crops remain unchanged. If a shorter new IR cannot support a saved crop, show a preparation/shape-support error and retain eligible accepted output; do not clamp/reset the saved edit silently. The user can choose another response or explicitly reset/choose Full response.

Run actual 44.1/48/96 kHz preparation and long-tail/overlap tests, including sources at zero/late positions, sample rounding, mono IR to stereo and asymmetric stereo channel isolation. Memory estimates must include original/prepared IR, raw/shaped output, accepted plus pending audio, resident pages, FFT scratch and other caches. Make catalogue decode admission depend on actual descriptor length, replacing the current 3 MiB single-Hall assumption.

Planning estimates only: five 48 kHz stereo/24-bit assets at 10+1+4+7+0.5 s contain about 6.48 MB of PCM payload before embedding overhead. Existing estimator for a 10 s IR and 0.5 s stereo source reports about 95.1/95.7/127.4 MiB at 44.1/48/96 kHz before occupied cache costs. These are not measured peaks or performance guarantees. Measure elapsed render/resampling time, peak accounted/observed memory, first/repeated assignment latency, cache disk growth and Debug/Release bundle growth. Never enlarge budgets or truncate requests to make acceptance pass.

### 7. Evidence and listening gates precede completion

Keep a manifest and a requirement-to-test/listening checklist during implementation. Generate matched dry/forward-reference/reverse-wet examples so the user can assess the actual swells, not just the forward IR. First gate: approve five contrasting sounds and sensible levels before locking release checksums. Second gate: native dropdown/status verification followed by FL Studio live switching, overlaps, negative lead-in limits, two-instance offline bounce and reopen. Update task boxes as objective tests and user reports actually pass; no planning checkbox is a pass.

## Risks / Trade-offs

- [Long stereo assets exhaust memory, especially retained+pending at 96 kHz] -> descriptor-based admission, one accepted snapshot, bounded coalescing, measured longest-response stress and explicit failure retaining safe audio.
- [Equal IR peaks produce very different wet loudness] -> energy/spectrum/active-window measurement and reverse-render listening; authored gains only.
- [Shimmer description implies dynamic effects] -> static label/tooltip/documentation and spectral rather than pitch-shifting claims.
- [A generic stale fallback leaks changed source] -> IR-only ownership fence with source/shape/placement/mapping revocation tests.
- [Shorter response conflicts with a manual crop] -> preserve exact edit and accepted result, explain required explicit reset rather than silently clamping.
- [Catalogue upgrades change old projects] -> immutable managed audio, stable release manifest, old Hall byte check and no index-based persistence.
- [CPU/package growth is larger than expected] -> measure actual release size and worker costs; report limits before completion, not speculative optimization or silent quality reduction.

## Migration Plan

Add the catalogue and assets behind existing resource/managed storage APIs. Keep the original Hall resource, checksums, default and wrapper entry points intact. No host parameter/plugin identity or state-schema change is planned. Test old version-7 sessions with Hall, imported WAV/WavPack, frozen renders and nondefault shapes before/after. Additions do not delete cache assets. An earlier binary can read the same committed audio references even though it cannot offer new catalogue choices; verify this with a saved fixture. No capture migration or library-upgrade reassignment occurs.

Implementation is a separate apply action. This planning change creates no WAV assets, generator code, runtime changes, release measurements or listening approvals.
