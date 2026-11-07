# Issue #8 storage efficiency experiments

## Store-local exact-byte candidates

Regrouped the measured [phase 0 inventory](2026-11-06-issue-8-storage-audit.json) using only entries under `Projects/`. For each distinct project/instance folder, group by logical byte count and SHA-256. Sum all but one member per group. The inventory uses independent physical files and the measurement excludes the fixture inputs, host-state files and matches across separate instances.

| Category | Exact-byte groups | Duplicate files beyond first | Duplicate logical bytes |
| --- | ---: | ---: | ---: |
| IR WAV | 6 | 54 | 82,949,616 |
| Capture WAV | 2 | 2 | 1,920,208 |
| Total | 8 | 56 | 84,869,824 |

The managed subset contains 660 files and 1,106,147,121 logical bytes. The measured duplicates account for 7.67% of that subset. The 54 IR copies came from repeated external imports of six four-second IRs. The audit recorded no new files for forty shape, name, gain and trim edits. These numbers describe the fixture. Every observed duplicate remains protected.

Decoded identities answer a different question. The native inventory fixture verifies that signed-zero-distinct WAVs have different byte hashes while their normalized decoded samples can compare equal. Portable Backup already addresses byte objects by SHA-256. It preserves relative aliases and validates descriptors separately from the shared bytes. The production working store keeps its existing write and read formats.

## Independent codec results

The [benchmark](../../scripts/asset-compression-benchmark.py) used Python 3.10, WavPack 5.9.0 built as a standalone x64 MSVC Release DLL from `5803634a030e2a11dba602ba057b89cc34486c67`, and Python's bundled zlib 1.2.12 at level 6. DLL SHA-256 was `708f3e4153a6c91ec0de6be899beed7810a08ada4537b8f94c58fb53aa0dea2a`. It generated deterministic five-second stereo float32 chunks at 44.1, 48 and 96 kHz by constructing IEEE-754 words. Each codec and fixture ran in a fresh process. No plugin writer, reader, callback or real project participated.

| Signal | WavPack bytes as % of raw float32 | DEFLATE bytes as % of raw float32 |
| --- | ---: | ---: |
| Dense | 79.36 | 87.41 |
| Sparse | 2.64 | 2.11 |
| Low-level | 79.36 | 87.41 |
| Over unity | 79.36 | 87.42 |
| Subnormal | 76.06 | 87.41 |
| Signed zero | 6.32 | 6.56 |

These are encoded stream bytes divided by raw sample bytes, aggregated across rates. A production wrapper, descriptors and checksums would add their own overhead. Pure recorded silence already has metadata-only coverage, and gaps have no audio file. The synthetic sparse and zero results therefore describe the codecs, with limited implications for savings on current capture workloads.

All 36 codec/rate/signal combinations reproduced every sample bit and SHA-256. Format and length were validated, including the float32 exponent and exact WavPack seek values. Per-case ratio inputs, CPU and wall time, peak process working set, and warm open/first-frame and open/seek/middle-frame timings are in the [measured results](2026-11-07-issue-8-compression.json). WavPack opened and sought within its encoded stream. DEFLATE decoded one bounded chunk for each read. Timings include local cached file operations and Python overhead, and the CPU timer is coarse for these short runs. The process peak includes interpreter and fixture memory. Timing is single-run synthetic evidence.

The disposable fault worker rejected corrupt, truncated, oversized-input and oversized-output cases for both codecs and rejected hybrid WavPack without correction data. Decode validates expected geometry before allocating the output buffer. The expected exact hash is checked on full decode. Each worker has an 8 MiB encoded input limit, a maximum five-second stereo output at 96 kHz, and a 30-second process timeout. This is focused malformed-input coverage. Production resource admission and compatibility still need their own review if a format change is proposed.

To reproduce, build the pinned dependency as a shared library with programs disabled in a standalone build directory. Then run this command from the repository root with a destination in ignored output.

```powershell
python scripts/asset-compression-benchmark.py <path-to-wavpackdll.dll> --output out/compression.json
```

## Adoption and rollback proposal

Keep WAV as the storage and rollback baseline. WavPack is the stronger candidate for a future codec review because it preserved every tested bit, had the better dense ratio, and supported quick seeks. Before adopting a codec, measure end-to-end growth, open/seek and CPU on representative permitted music through the real asynchronous capture and render paths. Require a new explicit format version, compatible verified readers, bitwise checks, and a verified rollback copy in WAV before any project refers to a compressed object. Keep the earlier project, binary and WAV store together. The experiment introduces no codec dependency into production.

The 82,949,616 bytes of repeated IR WAV data identify duplicate-write avoidance as the first candidate for separate review. Test an exact-byte or exact serialization lookup in one managed store before publishing an identical IR. Pin and verify the existing object under worker coordination and fall back to the current immutable write if the lookup is incomplete or uncertain. Preserve distinct import provenance, dependency descriptors, historical aliases and signed-zero-distinct bytes. Measure avoidable writes and verification cost on repeated and different imports. Existing alias files stay available for earlier project saves. The fixture found few capture duplicates and no exact wet-render duplicate candidates in the selected scope, so their storage format remains a separate decision.

### Proposed bounded cache scope

| Class | Eligibility and reconstruction requirement |
| --- | --- |
| Future explicitly registered runtime-only intermediate | Consider only under a new versioned private namespace that saved states, manifests, backup receipts and exports cannot address. Require pinned exact durable inputs, engine version and configuration, validated regeneration off the callback, and an expired or released job lease. |
| Captured source, abandoned recordings, imported IRs | Protected. These may be the only exact source and have no substitute reconstruction proof. |
| Saved or historical wet renders, frozen output, prior snapshots | Protected. The current arrangement cannot enumerate older FLP references or establish reproducible frozen audio. |
| Managed or external exports, unknown files and legacy scratch | Protected. Ownership or the DAW's external references may be unknown. |
| Already approved cancelled Backup scratch | Use the separate quarantine and explicit final-removal workflow. This remains outside any future automatic cache policy. |

A future budget applies only to the explicitly versioned runtime-only namespace. Require a finite per-store byte and entry cap before admission. At the cap, skip admitting a new cache entry until an eligible unleased entry can be safely rebuilt. Unknown dependencies, index corruption, offline inputs and active jobs veto eviction. The product can always disable this cache and read its durable inputs using existing readers. Swell deletion, regeneration and absence from the current state confer no eligibility. The proposal requires separate owner review before any collector, eviction or storage format rollout.
