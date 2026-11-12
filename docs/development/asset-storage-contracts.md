# Asset storage contract and rollback matrix — issue #8

Prepared 2026-11-06 against main `df765d95170caaaab10a646e43088ea3be66ce53`; phase-0 evidence is in [the measured audit](../validation/2026-11-06-issue-8-storage-audit.md). The owner approved these compatibility and backup rules on 2026-11-06 and authorized phase 1. This is not approval of portable-format readiness. The [original source audit](asset-storage-audit.md) remains pinned to its historical baseline. The matrix approval itself introduced no storage change. Phase-1 read-only adapters/inventory and the integrated Settings composition are described in [asset inventory](asset-inventory.md); writers, relocation and purge remain unchanged.

## Independent version domains

| Domain | Accepted / emitted | Existing migration and dependency contract | Native fixture |
| --- | --- | --- | --- |
| Processor `PreverbState` XML | Reads 1–8; writes 8 | v1 starts a fresh empty capture namespace. v2+ restores exact saved project/instance IDs, manifest reference and revision. v3+ decodes creative JSON; v3 receives legacy mapping/processing/provenance treatment. v8 restores saved internal Capture/Live/Finalise mode; older envelopes restore Learn with Recapture disarmed. | `workflow_processor_test.cpp` exercises envelope 1–7 and current v8; `processor_test.cpp` and `M3ProcessorCases.h` cover v1/v2/v3 and invalid newer state. |
| Creative SwellSession JSON | Reads 1–12, writes 12 | See introduction table below. Preserve future and per-swell Sound definitions and exact IR references, imported library entries, full renders, timing/calibration, selection, edits, freeze and generation fingerprints. Missing or unknown schema never authorizes deletion. | `storage_audit.cpp::compatibility` covers field-appropriate v1–12 states and transactional v0/v13 rejection. `swell_session_test.cpp` covers malformed-state and round-trip cases. |
| Capture manifests | Reads 1/2; workflow writes 2; contextless adapters may write 1 | v1 has unknown historical generation context. v2 context ownership covers exactly mapped source, including recorded silence, never gaps. Contexts and PCM references accept through one immutable manifest. Bounds: 4096 contexts, 65536 spans, 8 MiB context metadata. | `storage_audit.cpp` round-trips 1/2 and rejects 3 without replacing output; `capture_context_test.cpp`, `capture_transaction_test.cpp` and `generation_configuration_test.cpp` cover ownership, atomic acceptance and races. |
| Export metadata inside creative v10+ | Reads unversioned legacy / v2; writes 2 | Unversioned history retains complementary gains with gain law 1; migration resets current preferences to Dry 0/Wet 1 and clears desired/current references. v2 retains independent Dry/Wet and per-file law 1/2. Both retain historical path/checksum/composition/rate/origin/frames. | `song_export_test.cpp`: legacy/v2 decode and v3/nonfinite/invalid provenance rejection; engine/file tests verify immutable bytes and interrupted output. |
| Managed IR/render descriptors | Engine version 1, existing Asset codec | Store exact float32 WAV plus descriptor; render identity includes source/IR/timing/bounds. Musical sample hashing normalizes signed zero; file SHA-256 is required for exact-byte identity. Store write has a random suffix and does not look up identical content. | `swell_asset_test.cpp` verifies previous versions, malformed paths, IR imports, interrupted pairs and >unity float; measured audit exercises repeated writes and old hashes. |
| Portable backup backend | Store/snapshot/descriptor/receipt/journal and reader: independent v1 domains | Exact-byte copy with legacy aliases, bounded parsing and explicit locator verification. No production association or saved-state format change. Unknown dependency-bearing versions prevent complete-pack claims. | `portable_assets_test.cpp`, `portable_snapshot_test.cpp` and native preview bindings; [phase-2 evidence](../validation/2026-11-06-issue-8-portable-backend.md). Native approval and the scoped same-profile host smoke are recorded; isolated-profile restore remains open. |

### Creative-version introductions

| Version | Reader behavior introduced; older input gets conservative/default behavior |
| --- | --- |
| 1 | Base definitions, shapes, timing and selection; absent ownership decodes manualEdited; mapping and automatic processing OFF. |
| 2 | Mapping/automatic processing, audition/default IR, suppression and swell provenance/detector anchor. |
| 3 | Post-roll, support offset and curve tensions. |
| 4 | Markers/isolation and swell placement. |
| 5 | Live enable/routing/wet level. |
| 6 | Regeneration sensitivity and freeze. |
| 7 | Isolate Dry. |
| 8 | Gain policy/revision and calibration; prior versions retain legacy gain policy. |
| 9 | Retained `importedIRs` library. These are durable dependencies even when no current swell/default uses them. |
| 10 | Export preferences and immutable history. |
| 11 | Strict `autoGenerateOnStop` and optional per-swell `generationSource`. Fresh sessions default OFF; supported creative v1–10 and envelopes v1/v2 without creative metadata restore ON. Fingerprint records source identity, not an asset path or Ready-render claim. |
| 12 | Versioned future and per-swell Sound identities alongside their existing managed IR references, plus a future Level default. Migration matches bundled identities by exact PCM identity and format, preserves other managed IRs as Custom and leaves absent references unresolved. Legacy projects start the future Level at 0 dB without changing any saved swell gain. The 8 MiB creative limit and XML envelope version remain in place. |

The generated historical fixtures are synthetic codec inputs with later-introduced fields removed, not FLPs produced by every historic plugin binary. Existing native processor fixtures test the host-state envelope separately. Historical-binary/FL Studio rollback is not claimed from codec tests. Asset structural validation permits positive engine IDs, whereas `SwellAssetStore::verify` requires engine 1 and matching file/descriptor content. A successfully decoded session is therefore not proof of verified assets, supported engine readiness or complete pack closure.

## Dependency handoff and readiness

The worker-side dependency visitor must start with the exact accepted capture reference/revision and coherent accepted creative state. Include chunk WAVs/checksums, exact source positions and silence/gaps; all context IRs/origins/spans/settings; current default IR; every retained imported-library entry; each swell's assigned IR, completed full render, calibration and generation metadata; all saved Export-history WAVs and descriptors. Include bundled IR bytes actually accepted, not just a mutable preset identifier. Preserve disabled/frozen/stale references. A missing required file prevents a complete-pack claim; it is not permission to regenerate or drop that history.

Accepted historical context IRs are independent of current Default IR and swell assignments. A context can explicitly lack an IR without invalidating dry source; preserve that emptiness. Schema-1 history stays unknown. Learn candidates, provisional source, pending Stop intents, prepared generation/export jobs and active voices are excluded from durable authority. A relink/restore must not replay contexts or change auto-generation policy. Preserve current source/render fingerprints and accepted readiness checks; stored bytes alone do not establish compatible timing or output readiness.

Captured source roots are injected into the fixture. Production roots remain `%LOCALAPPDATA%\K7MA\Preverb\Projects\<projectId>\<instanceId>`. Managed export history is under `exports/` in that instance root. Separate delivered/Playlist WAVs belong to the user regardless of their directory and remain outside any collector. No complete graph of offline FLPs or external Playlist copies exists. Native duplicate-state tests demonstrate shared immutable directories with independent accepted maps, not discovery of a host's Save As identity.

## Backup and binary rollback

| Situation | Required recovery material / supported action | Gate or limit |
| --- | --- | --- |
| Same binary, supported saved state | Retain FLP plus its complete referenced legacy instance tree, including manifests, chunks, IRs, renders and exports. Preserve source/context and creative state together. | Current readers restore exact saved references; do not substitute newest files. No supported pack operation exists yet. |
| Missing/corrupt asset | Preserve all remaining files and FLP; use explicit unavailable diagnostics. Keep unaffected ranges/results. | Do not Clear, regenerate or delete history as a repair shortcut; a missing sole source may be unrecoverable. |
| Return to a pre-schema-2 / pre-v11 binary | Retain pre-upgrade FLP, original compatible binary and its untouched cache/IR/render files. Test on copies. | An older binary can reject newer creative/context state. No down-save or lossless reverse migration is provided. Native codec tests are not a historical-binary reopen test. |
| Future Pack/Relink/association | Exact-byte destination-only verification, all included instance receipts and complete dependencies, then explicit association and host-save request. Keep old FLP/store. | Not implemented or authorized by phase 0. Second-profile testing with originals unavailable remains mandatory before cutover. |
| Cleanup or dedup proposal | Indefinite protection for captured source, imported IRs, renders/history, exports and unknown references. Only separately reviewed journaled pack-copy scratch can eventually be eligible. | Phase 0 grants no cleanup, migration, compression or default-root change. Refcounts/age/duplicates never prove old-FLP unreachability. |

## Host project-path investigation

The [FL Studio manual](https://www.image-line.com/fl-studio-learning-content/fl-studio-online-manual/html/fformats_project.htm) excludes third-party-plugin samples from Zipped Projects. [VST3 IStreamAttributes](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/Change%2BHistory/3.6.0/IStreamAttributes.html), optional since 3.6.0, can carry project/preset context and a loaded-state path when supplied. This does not guarantee the current FLP location.

Inspected JUCE **9.0.4**, `modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp`: `setState(IBStream*)` reads the stream through its load helpers; the processor receives `setStateInformation(const void*, int)`. No `IStreamAttributes`/`kFilePathStringType` consumption was found in that wrapper. The ReverbArise processor has no host-path adapter. These observations support retaining the explicit-folder fallback; they do not establish that FL Studio never supplies metadata. The later owner-authorized [runtime spike](../validation/2026-11-06-issue-8-project-folder-spike.md) instrumented FL Studio 26.1.7 build 5654 with JUCE 9.0.4: all seven save/restore streams lacked the optional IStreamAttributes interface across unsaved, first-save, reopen, Save As and moved-project cases. No reliable project-folder signal was established on that route; other hosts/versions remain untested. No FLP path is inferred from CWD or names.

## Review boundary

Owner approval recorded 2026-11-06: "I approve the compatibility and backup rules. Proceed with phase 1 of portable-project-assets, preserve the safety gates, and update PR #31." This closes tasks 1.1/1.5 and authorizes read-only inventory/adapters and the phase-1 Settings preview. The later diagnostic host-path spike is recorded above; its negative result preserves explicit-folder/user-data fallback and grants no storage cutover authority. Pack/Relink, association, migration, cleanup and later phases remain separately gated.

Phase-2 authorization: the owner requested explicit-folder Pack/Relink with the user-data fallback. The [backend and native candidate](portable-asset-backups.md) are implemented; the approved controls are integrated, while association, missing-asset recovery and cleanup remain subject to their recorded gates. This supersedes the earlier spike-only implementation scope without changing compatibility or rollback rules.


## Phase-4 production association — 2026-11-07

PR #112 adds optional processor envelope v9 for explicitly associated stores.
Readers retain envelopes 1–8; legacy/default sessions still write v8. V9 embeds
exact accepted source metadata and a versioned store/snapshot/writer/registration
locator, with bounded recovery origins when applicable. It changes no creative,
capture or Export schema. Absolute locations remain local hints; writer ownership
is reacquired independently on every reopen. Host saves perform no filesystem I/O.
See [working-folder ownership](working-folder-association.md) for implemented
bindings and [integration evidence](../validation/2026-11-07-issue-8-phase4-settings.md)
for native, fresh-profile, FL Studio and rollback results. Old-binary rollback still requires
the retained pre-upgrade FLP and untouched legacy store; no v9 downconversion or
old-binary compatibility claim is made. Issue #8 and phases 5/6 remain open.
