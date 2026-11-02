# impulse-response-library Specification

## Purpose
Provide a small, reproducible and redistributable collection of distinct static impulse responses curated for reverse-reverb swells, with stable saved assignments and truthful musical descriptions.

## Requirements

### Requirement: Bundled responses cover distinct musical uses
The plugin SHALL ship Monumental, Intimate Plate, Vintage Dark Hall, Shimmer Cloud and Whisper in addition to unchanged Preverb Hall. Each new response SHALL have an audibly distinct reverse-swell character and a measured duration within its documented design range. Shimmer Cloud SHALL be described as static spectral coloration, not dynamic pitch shifting or modulation.

#### Scenario: Choose contrasting swell spaces
- **WHEN** the user compares the library on the same committed musical sources
- **THEN** Monumental provides a smooth 8–12 s cinematic response, Intimate Plate a controlled 0.7–1.5 s dense lift, Vintage Dark Hall a warm dark 3–5 s response, Shimmer Cloud a 5–9 s sparkling static texture, and Whisper a restrained 0.25–0.8 s response
- **AND** listening acceptance establishes useful differences after reverse convolution rather than accepting duration changes alone

### Requirement: Bundled audio has reproducible provenance
Each response SHALL include distributable audio, documented authorship/license, a deterministic generator and parameter manifest, and verified content checksums. Documentation SHALL identify generator version/seed, source format, exact duration, per-channel peak/RMS/energy and stereo compatibility. Assets SHALL not depend on proprietary reverb captures or unlicensed recordings.

#### Scenario: Reproduce a shipped response
- **WHEN** generation runs in its documented environment with the recorded version and parameters
- **THEN** the produced PCM and packaged audio match the published checksums and measured format, and redistribution terms are included

#### Scenario: Corrupt bundled data
- **WHEN** bundled content does not match its declared checksum or format
- **THEN** assignment reports the invalid response without publishing partial data or silently substituting another preset

### Requirement: Saved assignments retain immutable sound identities
Every catalogue release SHALL identify each response by stable preset identity, revision and content checksum. Assigned audio SHALL be retained as immutable managed content. Library additions, label changes or upgrades SHALL not reinterpret saved assignments. Existing Preverb Hall bytes and new-instance default behavior SHALL remain unchanged.

#### Scenario: Reopen after a library upgrade
- **WHEN** a saved session references an older assigned response and the installed catalogue has newer content or ordering
- **THEN** the session uses its original verified managed audio and label, without selecting by menu index or silently replacing sound

#### Scenario: Missing managed assignment
- **WHEN** saved audio cannot be verified
- **THEN** the affected swell shows actionable missing/invalid status and no newer response is silently accepted as equivalent

### Requirement: Authored levels are controlled without changing imported gain
Bundled responses SHALL be calibrated during authorship using documented reverse-convolution listening and objective measurements. Runtime preparation SHALL preserve asset gain, existing external-IR semantics and disclosed monitoring protection. Measurements SHALL distinguish IR levels, raw wet, shaped wet and protected playback; raw floating-point overs are not permission to clip or limit stored renders.

#### Scenario: Compare preset levels
- **WHEN** the same piano, pad, vocal and pluck fixtures are rendered at identical shape, routing and trim
- **THEN** reports include raw/shaped peaks and active-window levels, and acceptance checks for unexpected jumps, clipping, non-finite output and safety attenuation rather than only peak-normalizing each IR

### Requirement: Library preparation preserves full linear responses
Bundled responses SHALL follow the existing mono/stereo mapping, rate conversion and exact full reverse-convolution alignment contracts. Preparation SHALL run outside the real-time callback, retain complete long responses and reject unsupported resource requests explicitly. Longer wet lead-in SHALL not require captured dry pre-roll or imply playback before processed Song time.

#### Scenario: Long response at supported rates
- **WHEN** the longest stereo response is prepared at 44.1, 48 or 96 kHz with complete source input and admitted resources
- **THEN** output contains N+M-1 finite frames starting at P-(M-1), with correct channels, preserved rate-converted duration, negative lead-in and overlapping-swell timing

#### Scenario: Resource pressure
- **WHEN** resampling, rendering or replacement would exceed the supported working budget
- **THEN** the request remains explicit with a resource diagnostic and no silent shortening, fabricated coverage or real-time allocation/wait

### Requirement: Musical and operational evidence establishes library readiness
Completion SHALL include reproducible objective tests, reverse-swell listening acceptance and measured package/render/cache costs for the longest stereo response. Native UI and FL Studio checks SHALL exercise assignments, replacement, overlap, reopen and export. Measurements and listening results SHALL remain pending until actually performed.

#### Scenario: Deliver the expanded collection
- **WHEN** the change is presented for completion
- **THEN** evidence identifies exact asset/build revisions, test rates and channel policies, longest-response costs, approved musical examples and any unresolved limits without treating generated assets or compilation as listening approval
