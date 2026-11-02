# Workflow modes

Issue #10 introduces a foundation mode authority; the dedicated Capture and
Finalise layouts and export composition are separate issues. This guide records
the implemented model and processor contract. FL Studio acceptance remains open.

| Mode/phase | Source | Host-scheduled wet | Creative editing / internal preview |
| --- | --- | --- | --- |
| Capture idle | Can begin additive capture on valid DAW Play | Off, dry passes | Off / off |
| Capture recording | Protected additive or explicitly armed replacement pass | Off, dry passes | Off / off; mode selection locked |
| Capture draining/validating | Provisional; old committed source remains authoritative | Off, dry passes | Off / off; mode selection locked |
| Capture failed/empty | Prior source retained; reason visible | Off, dry passes | Off / off; stopped mode requests allowed |
| Live | Never records | Existing enable/routing/gain at absolute positions | Existing validated editor / preview |
| Finalise | Never records | Off, dry passes | Off / explicit supported preview only |

Mode eligibility uses verified useful audio and compatible timing; silence is
valid coverage but alone does not enable Live. User-approved compatibility:
verified enabled/unmuted/solo-eligible frozen wet also qualifies for Live at its
compatible timing without dry source. Unfrozen work and captured-dry preview
still need real source; this exception grants no replacement permission.
Finalise entry requires useful captured source and additionally
an enabled ready swell. A successful useful Stop commit selects Live before wet
renders finish. Empty/failed passes remain Capture. Restored saved modes may
remain selected while dependency readiness is unavailable; unavailable controls
must explain the reason.

`preverb_workflow_state_test` checks the eligibility matrix, all guarded
recording/settling transitions, successful/empty/failed Stop, duplicate and stale
completion rejection, restore/cancel disarming and independent instance state.
Processor and transaction fixtures exercise the actual host-observation and
storage paths; host-specific acceptance is recorded separately.
Old projects without mode metadata migrate to Capture, Recapture off, preserving
all creative state. No capture pass or preview resumes from saved transient work.

## Source transactions

Capture uses the same `%LOCALAPPDATA%/K7MA/Preverb/Projects` managed store.
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
`preverb_workflow_processor_test` exercises actual host Stop, automatic Live,
protected mapped playback, explicit recapture, Live no-capture, Finalise dry-only
realtime/offline and old/new mode restore. Full UI/host acceptance remains open.

## State and recovery

The workflow uses processor XML envelope version 8; the existing swell JSON
remains version 8 with its original gain-policy revision semantics. Supported
processor envelopes 1-7 without workflow metadata always migrate to Capture,
Recapture off, even when they contain useful source or enabled old mapping/live
flags. No source, IR, raw render, creative edit or gain policy is reassigned.
For normal playback after opening an old project, select Live once source or
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
on the message thread, including accepted capture/automatic Live changes when
the editor is closed. The callback only posts from non-realtime mutation paths;
it does not call host code under engine/publication locks. Unchanged settings,
selection, progress, playback and temporary Recapture arming do not request a
resave. Explicit project saving remains the host's responsibility.

Regeneration tolerance remains saved legacy metadata. It cannot authorize
source replacement and its former main control is retired. Marker sensitivity,
Automatic Swells and explicit Recapture remain separate. The old mapping
Settings switch now requests Capture/Live through the same guards; it is not
an independent recording switch. Capture settings may select an idle instance
gain policy/future IR; selected-swell editing remains Live-only.

New builds are named ReverbArise.vst3. The VST3 component/controller IDs,
com.k7ma.preverb bundle identity, K7Ma/Prv1 codes and empty parameter inventory
remain unchanged. An installed older Preverb Spike bundle is not removed by
building; host rescan/reload behavior requires explicit FL Studio verification.
