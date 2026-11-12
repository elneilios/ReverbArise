# Workflow modes

The responsive editor opens fresh profiles at 900x600/100%. Learn separates
Capture & Generate from Sound without changing workflow admission; existing
Sound Preset and Level are available with detection off. Perform separates Timing, Envelope
and Sound while keeping Reset actions visible. Export retains its independent
mix and delivery guards. Settings Appearance controls workspace size and optional
100%/125% user magnification; these are local presentation preferences, not
musical session settings. See [editor presentation](editor-presentation.md).

Learn Sound and Perform Sound share the preset picker, contextual Custom IR
file controls and a scoped binding. Learn Level sets the future default, and
Perform Level edits the selected swell through the existing gain path. Accepted defaults enter explicit
Generate, eligible Generate on Stop and manual creation through the existing
snapshot transactions. Default edits never live-update existing swells. Explicit
per-swell IR changes retain existing manual-edited provenance; manual/frozen
ownership, asset identities and capture/render policy remain unchanged. Async
imports and menus retain their original session/pass or swell/input ownership.

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

Perform retains a warning-level fallback in the existing reserved header notice
row: **“Caution: In Perform, swells can remain audible during DAW playback, even
if the source is silent or muted.”** Existing errors, actionable warnings and
transient notices (including information-level debounced IR preparation) replace
it; it returns when they clear. Learn and Export keep their existing notice
policies. No timeout, dismissal preference or host-mode inference is added.

The full help/accessibility detail explains saved Song positions, lead-ins and
silence, Pattern playback and upstream Playlist/instrument mute. Explicit effect
bypass or mixer mute can stop the swells; mixer mute affects the whole path and
bypass can restore dry. The warning itself never changes playback enable,
capture eligibility, dry routing, audition, source/render identity or saved state.
Repeated identical refresh reuses NoticeBar's announcement suppression and
does not move the workspace or take focus.

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
entry refinement; dedicated WAV delivery remains in progress. Issue #38 separates
source settlement from Perform entry: a successful useful Stop settles idle
Learn, retaining dry readiness and consuming a successful Recapture arm. Only a
subsequent successful shared Generate transaction may automatically enter
Perform; source acceptance alone does not qualify. The persisted opt-in Stop
request waits for matching final candidate analysis and uses the same safe
generation transaction as the explicit button. Manual
Perform/Export navigation retains its existing readiness rules. Empty/failed
passes remain Learn. Restored saved modes may
remain selected while dependency readiness is unavailable; unavailable controls
must explain the reason.

PR #18's source usefulness refinement is retained: a successful explicit mapped
replacement can leave useful committed material even when the replacement is
silence. Under #38 this settles Learn and disarms Recapture; Perform entry still
requires a successful generation or an eligible manual mode request. Erasing the last useful source stays Learn; fresh silence-only,
ordinary mapped-only and empty passes do not gain automatic Perform entry.
Export and frozen-wet admission share ready-render rate/tempo/meter checks.
This is foundation eligibility, not #6's later full export preflight.

The workflow word has a transient generation-publication reservation, separate
from persisted mode/phase values. After background preparation and identity
checks, a bounded CAS reserves the exact idle Learn/pass/configuration snapshot;
the caller performs only a nonthrowing prepared-session swap and releases to
Perform. A losing Play/mode/settings request cannot observe partial publication.
Play admission never waits, locks or allocates. Off-callback reset/cancel orders
after the short swap; it cannot interrupt the swap and leave only half a commit.
Preparation itself holds no reservation, so user edits, capture restarts and
navigation can supersede it. Generation identities also fence source/settings
changes; the configuration slot alone is not an ABA-proof settings identity.

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
`preverb_workflow_processor_test` exercises actual host Stop, explicit navigation,
protected mapped playback, explicit recapture, Perform no-capture, Export dry-only
realtime/offline and old/new mode restore. Full UI/host acceptance remains open.

## State and recovery

The workflow uses processor XML envelope version 8. Creative swell JSON now
writes version 12 and preserves its original gain-policy revision semantics. Supported
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

The accepted default Sound and future Level are part of the guarded generation
configuration. A default edit marks host state dirty and invalidates obsolete
Generate/Stop work without changing existing swells or rerunning detection.
Genuinely new manual and automatic swells use the accepted Level, which starts
at 0 dB. When Generate matches an existing eligible ID, the existing response
replacement policy applies and the matched swell keeps its Level. Reset All
still restores its existing gain default.

Learn's effective Automatic Swells policy is the conjunction of the saved
processing and marker switches. Explicit On enables both; Off disables
processing without resetting hidden IR/sensitivity/marker preferences. Idle
Learn edits refresh read-only candidates across accepted source, including
earlier dry-only capture, without changing swell definitions or queuing wet
renders. Explicit Perform marking/reanalysis remains separate. The whole-pass
configuration pin and stale-input fences are documented in
[the Learn protocol](learn-source-context-protocol.md). Source/context
persistence, scheduling and verification are recorded in the issue #38 validation
report linked from that protocol.

## Initial Perform presentation

The first successful Generate commit offers one editor handoff, shared by explicit
Generate and opted-in Stop, even when the editor was closed during completion.
The editor selects the committed swell with the earliest arrival using the same
chronological ordering as the list, then uses canonical engine selection to bind
list, waveform/envelope and Selected Swell controls. The timeline reveals its
lead-in and arrival using definition/source/IR timing while wet rendering is
pending. Useful zoom is retained with minimal panning; an unsuitable span is fit.
Overview reads the same timeline viewport. Queued/rendering status remains honest.

A valid restored selection, an established nonempty Perform editing position,
and later mode returns retain the user's selection and navigation. Empty visits
do not establish an editing position. Empty/no-change/failure/cancellation never
offer a handoff. The offer is consumed once; later wet/raw publications refresh
content without fitting again, so a newer selection or viewport wins. This is
editor navigation only and does not seek the DAW, audition or change enablement.
Existing serialized mode values and navigation ownership remain unchanged.
The editor reads mode and session from one engine snapshot under the publication
lock, so a commit between separate reads cannot establish a Perform position
from pre-commit definitions or consume the new handoff prematurely.

With the approved Perform workspace, restore its optional `performUi` view/Snap
preferences before applying a new eligible handoff. The reveal uses the same
bounded navigation and persistence bindings, marks the initial fit complete and
keeps pending source/IR lead-in timing inside the navigation bounds. A real user
pan/zoom is persisted through that existing binding; a delayed render never
replays the initial fit. The optional Perform/audition preferences remain intact
when migrating an accepted v10 project to strict #38 v11 state.

### Explicit manual preparation in Perform

Adding a manual target or accepting an arrival, source-interval or IR edit requests
background preparation for that unfrozen target, including after dry-only Learn
capture with Automatic Swells, Auto markers and legacy auto-refresh off. This
does not enable automatic discovery, replace source or start Audition. A future
Default IR change still affects future targets only; selected IR import completion
prepares only its pinned target IDs after the existing session/edit checks.

The request uses the existing forced dependency-check queue and edit debounce.
It releases that target's deferred Learn dependency and records the current
source revision so a newly edited interval is not mistaken for a subsequent
dry-only source commit. Later capture changes retain the existing invalidation
and explicit-generation policy. Complete coverage, verified IR, compatible timing,
resource limits and immutable publication checks remain authoritative. Shape,
gain and placement edits keep their existing minimum-work paths.

Missing source or IR remains actionable and unavailable for current Audition;
invalid imports retain accepted assets and report the failed request. See
[issue #55 evidence](../validation/2026-11-04-issue-55-manual-preparation.md)
for native regressions and the separately recorded host acceptance status.
