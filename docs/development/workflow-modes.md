# Workflow modes

Issue #10 introduced the foundation mode authority. Issue #16 adds the shared
Learn / Perform / Export header; the dedicated Learn/Export layouts and WAV
composition remain separate issues. This guide records the implemented contract;
new header host acceptance is recorded separately from the earlier foundation run.

## Display names and durable identity

The issue #16 presentation adapter maps the durable modes below to musician-facing
labels. The approved library selector is composed into one shared WorkflowHeader;
the former temporary dropdown is removed. Component and integrated appearance
acceptance are tracked separately in the dated header evidence.

Issue #35 adds cached semantic SVGs before Learn (waveform), Perform (three-handle
reverse swell) and Export (audio document/output), with the existing navigation
geometry and guarded requests. The lower row uses 14-pixel regular Inter in
`#C7CDD4` for both description and notice body. A 2×16-pixel aqua accent precedes
the description by an 8-pixel gap; notice severity colours remain independent.
The descriptions are “Capture audio and discover musical moments”, “Shape and
place reverse swells with precision” and “Preview your prepared swells”. See
[Issue #35 integration evidence](../validation/2026-11-04-issue-35-header-integration.md)
for owner approval and native/host/DPI coverage.
The implemented header contract is synced into `openspec/specs/swell-workspace/`;
its [archived change](../../openspec/changes/archive/2026-11-04-enhance-header-mode-navigation/proposal.md)
and [archive checks](../validation/2026-11-04-issue-41-openspec-archive.md) preserve
the planning history and validation limits.

| Durable XML value / internal mode | Display label | Role |
| --- | --- | --- |
| Capture / capture | Learn | Capture audio and discover musical moments |
| Live / live | Perform | Shape, edit and play swells; no source capture |
| Finalise / finalise | Export | DAW wet bypassed; existing explicit preview only |

`workflow::modeName()` and the `workflowMode` XML property remain serialization
identifiers. `UI/WorkflowPresentation.h` owns labels, concise subtitles/status and
human explanations; it uses `workflow::entryBlock()` from the same predicate chain
as mode admission. No second UI eligibility policy is introduced; the retired
string-returning guard adapter is removed after migrating its consumers.
Recording, drain/validation, missing source, incompatible timing and missing ready
wet have distinct explanations; source-ready Perform never waits for wet jobs.
The Export label does not add the deferred #6 WAV-delivery workspace. Source
capture, Recapture and live wet remain functional terms, not persisted mode renames.

## Foundation behavior

| Mode/phase | Source | Host-scheduled wet | Creative editing / internal preview |
| --- | --- | --- | --- |
| Learn idle | Can begin additive capture on valid DAW Play | Off, dry passes | Off / off |
| Learn recording | Protected additive or explicitly armed replacement pass | Off, dry passes | Off / off; mode selection locked |
| Learn draining/validating | Provisional; old committed source remains authoritative | Off, dry passes | Off / off; mode selection locked |
| Learn failed/empty | Prior source retained; reason visible | Off, dry passes | Off / off; stopped mode requests allowed |
| Perform | Never records | Existing enable/routing/gain at absolute positions | Existing validated editor / preview |
| Export | Never records | Off, dry passes | Off / explicit supported preview only |

Mode eligibility uses verified useful audio and compatible timing; silence is
valid coverage but alone does not enable Perform. User-approved compatibility:
verified enabled/unmuted/solo-eligible frozen wet also qualifies for Perform at its
compatible timing without dry source. Unfrozen work and captured-dry preview
still need real source; this exception grants no replacement permission.
Export entry requires an enabled, unmuted, solo-eligible compatible ready wet
render independently of dry availability; verified frozen-wet recovery therefore
also supports Export. Dry alone does not unlock Export, and its selected mix or
pending file preparation does not eject the mode. This is the implemented #6
entry refinement; dedicated WAV delivery remains in progress. A successful useful Stop commit selects Perform before wet
renders finish. Empty/failed passes remain Learn. Restored saved modes may
remain selected while dependency readiness is unavailable; unavailable controls
must explain the reason.

PR #18 review refinement: a successful explicit mapped replacement can enter
Perform using remaining useful committed material, even when that replacement is
silence. Erasing the last useful source stays Learn; fresh silence-only,
ordinary mapped-only and empty passes do not gain automatic Perform entry.
Export and frozen-wet admission share ready-render rate/tempo/meter checks.
This is foundation eligibility, not #6's later full export preflight.

`preverb_workflow_state_test` checks the eligibility matrix, all guarded
recording/settling transitions, successful/empty/failed Stop, duplicate and stale
completion rejection, restore/cancel disarming and independent instance state.
Processor and transaction fixtures exercise the actual host-observation and
storage paths; host-specific acceptance is recorded separately.
Old projects without mode metadata enter Learn (durable Capture), Recapture off, preserving
all creative state. No capture pass or preview resumes from saved transient work.

## Source transactions

Source capture uses the same `%LOCALAPPDATA%/K7MA/Preverb/Projects` managed store.
A pass stages bounded chunks into private immutable assets. Additive admission
subtracts mapped audio, mapped silence and earlier staged intervals. Explicit
Recapture permits replacement and resolves repeated positions using the last
received samples. The accepted map/save reference stays unchanged throughout.
Stop closes the pass, validates/writes an immutable candidate, then a short
ownership fence accepts it only for the same session/pass. Cancellation/restore
can supersede slow validation; an orphan candidate never becomes accepted source.
Failed passes keep the old reference; no destructive cleanup is implemented.

`preverb_capture_transaction_test` checks partial overlaps, sparse gaps, mapped
silence, loops, private source reads/save references, exact/changed recapture,
old snapshot/clone isolation, invalid data/rate/overflow and publication failure.
`preverb_workflow_processor_test` exercises actual host Stop, automatic Perform,
protected mapped playback, explicit recapture, Perform no-capture, Export dry-only
realtime/offline and old/new mode restore. Full UI/host acceptance remains open.

## State and recovery

The workflow uses processor XML envelope version 8; the existing swell JSON
remains version 8 with its original gain-policy revision semantics. Supported
processor envelopes 1-7 without workflow metadata always enter Learn (durable Capture),
Recapture off, even when they contain useful source or enabled old mapping/live
flags. No source, IR, raw render, creative edit or gain policy is reassigned.
For normal playback after opening an old project, select Perform once source or
verified frozen-wet recovery is ready. Missing unfrozen dependencies stay explicit.

New-format state saves its mode and last committed source reference; it never
stores an armed replacement permission, active voice or resumable pass. A save
during recording/validation can be reopened to the prior accepted revision.
Use a pre-upgrade FLP copy for rollback; older binaries reject the new processor
envelope instead of interpreting workflow metadata incorrectly. Existing
immutable assets/older saved snapshots remain retained; no root migration or
automatic cleanup is performed.

Persistent workflow, swell and Settings edits notify the host through JUCE's
non-parameter-state-changed flag. A coalesced AsyncUpdater delivers the notice
on the message thread, including accepted capture/automatic Perform changes when
the editor is closed. The callback only posts from non-realtime mutation paths;
it does not call host code under engine/publication locks. Unchanged settings,
selection, progress, playback and temporary Recapture arming do not request a
resave. Explicit project saving remains the host's responsibility.

Regeneration tolerance remains saved legacy metadata. It cannot authorize
source replacement and its former main control is retired. Marker sensitivity,
Automatic Swells and explicit Recapture remain separate. The old mapping
Settings switch now requests Learn/Perform through the same guards; it is not
an independent recording switch. Learn settings may select an idle instance
gain policy/future IR; selected-swell editing remains Perform-only.

New builds are named ReverbArise.vst3. The VST3 component/controller IDs,
com.k7ma.preverb bundle identity, K7Ma/Prv1 codes and empty parameter inventory
remain unchanged. An installed older Preverb Spike bundle is not removed by
building; host rescan/reload behavior requires explicit FL Studio verification.

## Learn generation settings

Learn's effective Automatic Swells policy is the conjunction of the saved
processing and marker switches. Explicit On enables both; Off disables
processing without resetting hidden IR/sensitivity/marker preferences. Idle
Learn edits configure future passes, rather than enqueuing reanalysis of old
source. Explicit Perform marking/reanalysis remains separate. The whole-pass
configuration pin and stale-input fences are documented in
[the Learn protocol](learn-source-context-protocol.md). Source/context
persistence and dry-only scheduling verification remain tracked implementation
work; these settings changes alone do not claim those tasks are complete.
