# Learn capture workspace — issue #11



## Baseline reconciliation — 2026-11-03



Implementation continues on `docs/11-learn-capture-workspace`, the original

issue-specific planning branch. Merge `dc003a7` brings in `origin/main` at

`653f13a`, including merged #10/#18, knob-wheel fix, shared-control development

standards, #16/#24's approved static Inter header, #26's build wrapper and #27's

product-version checks. The original planning commit `c5d94d8` remains intact.

There was no existing PR for this branch at implementation start.



Reuse `WorkflowHeader`, `WorkflowPresentation`, `NoticeBar` and their reserved

82-pixel header/notice geometry. Learn/Perform/Export are presentation labels;

internal capture/live/finalise enums and saved Capture/Live/Finalise strings

remain unchanged. No rename is reimplemented here. Static Inter faces, approved

colors, 1100x850 default/minimum and supported larger sizes remain the baseline.



Issue #11 comment 6578923411's two approved images and issue #9 comment

6579018037's default-IR image were retrieved through the authenticated GitHub

connector and inspected. Their older mode labels are superseded by #16, and

unimplemented Character algorithms are omitted as agreed. Native control-family

review and Learn checkpoints A/B remain required; detailed automated coverage

will precede one consolidated final host check.



Baseline native validation used `./scripts/build.ps1 -Configuration Release -Test`:

**35/35 passed, 35.48 seconds**, exit 0, all wrapper stages pass, Git `dc003a7`.

Logs: `out/build-logs/Release/20261103-012418-37c3b4e3/`. This includes workflow,

transactional capture, default/library/custom IR, automatic coordination, native

UI, session migration/frozen recovery and transport/export tests. This is native

automated baseline coverage, not new owner host acceptance.



Baseline Release binary SHA-256:

`4bb22ede30d01e3b79b3fc9f4d6c82974aff81b6571ae86ff823d07647e00295`.

Bundled IR manifest SHA-256:

`7d78ef0fc2d6012922f50fbd7fb0dd22fdfb1abaa673aefe02e1061d78ae786a`.

Processor envelope and swell JSON are v8; source manifest schema is 1. Product

version is 0.1.0, zero host parameters, component/controller IDs are unchanged

from #16. The source/context protocol and explicit metadata bounds are pinned in

[the implementation contract](../development/learn-source-context-protocol.md)

before code changes; persistence itself is not implemented yet.



Representative legacy/current, frozen and custom-IR fixture definitions are

retained under `out/issue11-baseline/fixtures/`, with exact hashes in

`out/issue11-baseline/fixture-identities.json`, and remain recoverable at baseline

Git `dc003a7`. These are automated fixture sources, not captured user FL projects.



## Shared importer and retained instance choices — 2026-11-03



Task 2.1 is implemented. The owner refined membership from currently used IRs to

every successful accepted import, retained independently of assignment and saved

per instance. Creative JSON v9 persists the bounded collection; the processor

envelope remains v8. The Perform picker now uses the shared catalogue; Learn's

new picker/layout remains task 3.2. Pass-admission atomicity remains task 2.2.



`scripts/build.ps1 -Configuration Debug -Test` passed all wrapper stages and

**36/36 CTest tests in 108.24 seconds**, exit 0. Logs:

`out/build-logs/Debug/20261103-014357-724021fe/`. The new

`preverb_ir_choices_test` covers unused-import retention, content deduplication,

managed reuse after original-file deletion, save/restore and legacy seeding,

independent instance additions, invalid/cancelled imports, in-flight supersession,

restored-session fencing and invalid/duplicate/over-capacity collection metadata.

Existing UI, IR handover, WAV/WavPack, gain, capture, state, workflow, transport,

allocation and product-version regressions also pass.



The initial Debug run built successfully but had two outdated test expectations

(legacy seeding includes the known factory assignment, and custom choices no

longer disappear on switching). Those assertions were corrected to the approved

retention policy before the passing rebuilt run. Release validation of this

implementation and the dedicated Learn UI review are still pending; no new FL

Studio results are claimed.



## Pass admission and first Learn composition — 2026-11-03



Tasks 2.2, 2.3, 3.1 and 3.2 are verified. The workflow atomically pins one of two

off-callback immutable generation slots. Core tests exercise 200 concurrent

Play/edit races, recording/drain/validation locks, stale edit/import tokens,

clear/restore lifetime and an asynchronously unavailable factory IR. All four

legacy master/marker combinations retain their effective saved policy. Explicit

Learn On/Off and hidden-value persistence use existing session fields, with

coalesced host notifications and no rejected/no-op notifications.



The native Learn body uses the approved header above a 280-pixel settings column

and one independent dry timeline. Tests verify hidden Perform panels and restored

Perform bounds/navigation at 1100 × 850 and 1400 × 1000. Shared switch/slider,

numeric editor, invocation-toggle and folder icon behavior is covered; real

percentage entry is tested by editing the JUCE numeric editor and committing it.

Native empty/on/off/default-selector/Recapture and mode-return images are in

`out/issue11-checkpoint-a/`.



Full wrapper run `scripts/build.ps1 -Configuration Debug -Test` passed **37/37**

tests, 122.20 seconds, exit 0, at

`out/build-logs/Debug/20261103-022034-78866972/`. This includes the real percentage

entry oracle. A subsequent final run adds explicit native pending/error IR label

assertions; its result is recorded below when complete. Earlier iteration logs

retain compiler integration failures, one MSVC COMDAT linker failure resolved by

recompiling affected objects, and fixture failures corrected before the passes.

Factory-dependent lifecycle/loop fixtures now wait for their intended setup IR;

the separate configuration test covers the missing/late-factory case. Native

button keyboard tests explicitly pump JUCE's asynchronous message delivery.

Focused loop-capture, performance-export and complete native UI fixture runs

also passed independently; these are separate fixture checks, not extra CTests.



Checkpoint A's interactive native preview is open from

`out/component-previews/issue11-learn-a/preverb_swell_ui_test.exe --review-learn`

with isolated test audio/assets. Owner approval is pending: task 3.3 stays

unchecked. The preview is intentionally retained open for review. No new FL

Studio or operating-system DPI acceptance is claimed. Durable source/context

publication, automatic scheduling and progressive/inspectable coverage are still

pending tasks, not implied by this layout approval request. Release validation

remains pending for the completed implementation.



Final checkpoint run: `scripts/build.ps1 -Configuration Debug -Test` passed all

wrapper stages and **37/37 tests in 123.57 seconds**, exit 0. Logs:

`out/build-logs/Debug/20261103-022322-e8b893e3/`. The native fixture now also proves

that pending IR intent keeps the prior accepted selector value, a failed import

shows feedback while retaining that value, and successful assignment updates it.

OpenSpec progress is **7/25**; checkpoint A approval remains pending at task 3.3.



The Debug VST3 SHA-256 is

`78198cef6c2551ded9b77f4ce0e45371122f3ff14a1466142b1a6e914de2c153`.

The separate open review executable SHA-256 is

`27d386a8ceeef49f0cf3f0da5e73a8c65e5bbcdaf8c17c4c8fa1beae8d44c579`.

The review copy predates only an IR-status component name added for test access;

its visuals and interaction behavior match the checkpoint. Changed source/test

identities are retained in `out/issue11-checkpoint-a/source-identities.json`

(SHA-256 `cc73a20859f1c18bd0f639ef0bc9c7317ef9c8d5c5d9442a70ec9ade2042a9d1`).

Source manifests remain schema 1 at this checkpoint, creative JSON is v9, the

processor envelope remains v8, and product/VST3 identities and IR assets remain

unchanged. The future schema-2 source/context implementation is not delivered yet.



## Checkpoint A layout/focus/hover refinement — 2026-11-03



Owner feedback was folded into existing task 3.3; no new change or task was

created. Matching Inter section headings now sit above consistent regular field

labels, including the separate Enable automatic swells row. Shared geometry

uses 4-pixel tight, 12/16-pixel related and 24-pixel section spacing. Toggle

label/help text occupies a left text column, avoiding overlap with the switches'

hit/focus regions. The canvas heading aligns with Source Capture, and caption,

ruler and status have distinct nonoverlapping rows. The approved header geometry,

charcoal/aqua palette, thin rules and workflow behavior remain unchanged.



The shared switch preserves its 44 × 24 track in a 56 × 36 component, including

focus stroke and fractional-scale clearance. Geometry checks and actual focused

rasters pass at 100%, 125%, 150% and 200%. An initial strict edge-raster check

caught coverage at 125%; clearance was increased before the passes. The shared

folder/Settings icon now paints its complete focus stroke inside its bounds.

Slider end-thumb clearance and unchanged header alignment are also checked.

These raster checks are not operating-system DPI or FL Studio acceptance.



The owner additionally reported a neighbouring-hover seam in the three-way mode

selector. Its joined child buttons retain their one-pixel overlap and square

internal joins, but the group now paints borders after all child fills, with

hover/press borders last. Native tests cover every adjacent selected/hovered pair,

full shared/top/bottom edges, unchanged selection/focus and stable accessibility

child order. The control gallery reproduces selected Export with hovered Perform.



Final command: `scripts/build.ps1 -Configuration Debug -Test` — all wrapper stages

passed, **37/37 tests in 130.74 seconds**, exit 0. Logs:

`out/build-logs/Debug/20261103-085023-0d80d288/`.

The preceding compact-layout run passed 37/37 in 117.49 seconds before the added

selector-hover fix. Focused native fixture checks passed independently too;

those do not count as extra CTests. Updated empty/on/off/default/Recapture and

focused-control screenshots are in `out/issue11-checkpoint-a-polish/`, including

1100 × 850 and 1400 × 1000 views and `control-library-states.png`.



The updated isolated native preview is open from

`out/component-previews/issue11-learn-a-polish/preverb_swell_ui_test.exe --review-learn`.

Review executable SHA-256:

`cf6ec45d5abc327ca4e7a7fbc4685f3c3ece92db275284d664fd9f47d273639b`.

Debug VST3 SHA-256:

`6ed0b7fa2ab05909084eab23e01553432cc6200ecb4f814e463144496e153d64`.

Checkpoint A remains awaiting explicit owner approval; task 3.3 stays unchecked

and overall progress stays 7/25. No further visual work, Release validation,

new FL Studio acceptance, source-context completion or archive is claimed.



## 2026-11-03 — final checkpoint-A workspace refinement



The owner requested restoration of the original centred timeline empty state,

larger secondary text, compact configuration-only grouping and truthful recording/

acceptance feedback. The approved header remains untouched: the top 114 rows of

the 1100 × 850 empty native render compare pixel-identically with the preceding

checkpoint. The settings column is now 304 logical pixels; Learn helper, ruler,

caption and bottom-status text use a 14-pixel Inter role. Primary workflow

instructions appear inside the empty timeline rather than beneath sensitivity.



The existing task-6.1 progress path was brought forward for this feedback: the

worker builds separate pass/epoch-fenced admitted-span/peak telemetry before the

five-second storage batch, publishing at most 10 Hz. Its explicit limits are 4096

ranges and 65536 peaks, statically bounded below 4 MiB of summary metadata. Source

save/render/playback readers still see only accepted commits. Native tests cover

protected silence, first-received additive input, last-received replacement,

metadata saturation, real pre-batch feedback, preservation through validation and

discard after acceptance/cancel/restore. Audio callback allocation guards remain

enabled. Coverage inspection/totals, sparse fit and the full task-6.1 acceptance

matrix remain unfinished; no additional task is marked complete by partial work.



`scripts/build.ps1 -Configuration Debug -Test` passed **37/37 CTests in 110.87 s**,

exit 0. Logs: `out/build-logs/Debug/20261103-094415-64bac596/`.

The first run had 36/37 passes with a UI-fixture stack failure after adding an

extra stack-resident editor; moving that fixture editor to heap ownership fixed

it. The focused UI fixture then passed independently, followed by the full passing

wrapper run. The final screenshot fixture also exited 0; it is not an extra CTest.

Strict validation of `learn-capture-workspace` and `git diff --check` pass.



Native screenshots: `out/issue11-workspace-final-polish/`, including

`fresh-1100.png`, `learn-on.png`, `learn-recording-real.png`,

`learn-accepting-real.png` and `learn-accepted-real.png`. Minimum/larger-size

renders and 125/150/200% scaled rasters supplement the existing focused-control

raster/geometry checks. These are not OS-DPI or FL Studio acceptance results.



The isolated native preview is open from

`out/component-previews/issue11-learn-a-final-polish/preverb_swell_ui_test.exe --review-learn-empty`.

Executable SHA-256:

`375925c8e49e100e766840624a47d0bd47ecb42d7d3956cf8a17c8175589de31`.

Checkpoint A still awaits owner approval; task 3.3 stays unchecked, progress 7/25.

No Release run, host acceptance, commit, archive or merge is claimed here.



## 2026-11-03 — typography consistency and reusable value controls



Owner feedback requested a focused visual pass, stable IR selection feedback and

standard reusable wheel/stepper controls. Learn labels now use explicit Inter

Medium 500 at 14 pixels, with grey Regular descriptions and a shared 6-pixel

label/help gap. Medium is an unmodified face from the already pinned official

Inter 4.1 archive; SHA-256:

`97ad806f526e41546d46365bb3a393145f75b7b1568913db74549ad8b8dba872`.

The existing OFL notice covers this added embedded face. No installed font or

synthetic weight is used. Header faces and existing Perform typography are retained.



Helper line breaks are balanced within the unchanged text column. The empty

state uses one centred group with 12/6-pixel internal gaps. Metadata uses a grey

Regular caption, a primary Medium ruler and revised internal spacing, retaining

the waveform viewport area, 304-pixel panel and existing control dimensions.

Dropdown and numeric values use matching 13-pixel Regular faces.



`SliderControl` / `NumericInputControl` are reusable library components, first

integrated in Learn. They support wheel input over track/value, typed values and

keyboard-accessible increment/decrement buttons, sharing range/interval,

formatting, gestures and invocation ownership. Tests exercise clamping, disabled

input, keyboard activation, typed synchronization, wheel/model synchronization,

button gestures and queued-step rejection across a workflow-mode round trip.

The slider owns its small specialized theme on the heap rather than enlarging

stack-resident editor fixtures. Other plugin controls are not migrated prematurely.



The IR feedback row has fixed 20-pixel geometry for normal, pending, accepted and

failed states; success text lasts 2.5 seconds before returning to helper text.

Long messages ellipsize without shrinking and remain fully available in the

tooltip. UI tests assert stable feedback/sensitivity bounds during pending,

success and failure, plus readable success retention.



Final command: `scripts/build.ps1 -Configuration Debug -Test`, **37/37 CTests

passed in 107.11 seconds**, exit 0. Logs:

`out/build-logs/Debug/20261103-101540-1a35650c/`.

Earlier incremental compile/link issues were corrected before this pass; coherent

recompilation cleared MSVC's COMDAT failure. The final independent native UI

screenshot fixture exited 0 and is not counted as an extra CTest. Strict change

validation and `git diff --check` pass.



Screenshots: `out/issue11-learn-control-polish/`, including empty/loaded states,

IR selection feedback, real recording/acceptance and minimum/larger sizes with

125/150/200% raster variants. The first 114 rows of the minimum empty render are

pixel-identical to the preceding approved-header baseline. Raster and geometry

checks do not claim OS-DPI or FL Studio acceptance.



The native owner preview is open from

`out/component-previews/issue11-learn-control-polish/preverb_swell_ui_test.exe --review-learn-empty`.

Review executable SHA-256:

`a1493af3036b4397d1fdf80111af1f298539d810fbced2b746020ce863631364`.

Debug VST3 SHA-256:

`fa2457544485b75e5de7fa3819908abc1cd60e07b3a64b2e27a9a31fa136f5c9`.

Workspace and revised control-family approval remain pending. Task 3.3 is still

unchecked; overall implementation remains 7/25. No Release, new host acceptance,

commit, archive or merge is claimed.



## 2026-11-03 — checkpoint A accepted; stable IR helper



The owner explicitly approved the numeric up/down component, then approved the

focused IR-feedback refinement and instructed continuation. Task 3.3 is checked;

the refreshed CLI confirms **8/25 complete**. Preserve the accepted components,

header and Learn composition; this does not complete backend/context/coverage or

host-validation tasks, nor finalize Perform's denser production styling.



Investigation confirmed model-owned `defaultIRPending` / `defaultIRError` signals

and latest-request/session/pass-fenced asynchronous acceptance. The UI had been

cycling helper text independently. The helper is now constant across all states.

An editor-owned presenter debounces pending feedback for 600 ms, then uses the

existing contextual bar. Completion emits no success flash. Persistent errors

show retry guidance and retain exact diagnostics in the tooltip. No importer,

DSP, state, numeric-control or header geometry behavior changed.



Deterministic presentation tests cover fast completion suppression, slow pending,

request-name/session supersession, capture-notice priority and nonexpiring errors.

Real held-import UI tests check delayed progress, constant helper/geometry,

accepted-old-IR retention, completion and persistent failed-import notices.

Screenshots are in `out/issue11-ir-feedback-refinement/`, including

`learn-ir-preparing.png` and `learn-ir-error.png`. The independent screenshot

fixture exited 0.



Final wrapper: `scripts/build.ps1 -Configuration Debug -Test`, **37/37 passed in

103.73 seconds**, exit 0. Logs:

`out/build-logs/Debug/20261103-104035-ef66b892/`.

An earlier full run caught streaming-page starvation at sample 767744; its

unchanged fixture passed in isolation and in subsequent suites. DSP was not

changed to mask it. Another run exposed a native UI fixture stack overflow

(Windows exit `0xc00000fd`), reproducible in CTest's working directory despite

three successful repository-directory runs. All remaining full editor fixtures

were moved from the oversized test stack to heap ownership; the final CTest run

then passed. These earlier failures remain recorded, not counted as passes.



The approved visible preview remains open from

`out/component-previews/issue11-stable-ir-helper/preverb_swell_ui_test.exe`.

Its SHA-256 is `4511681a01f99a8ec757935a32f6ebd8ffc413157e5a8f53bd363fc002a47ec1`;

the subsequent test-fixture ownership fix does not change the approved visuals.

Debug VST3 SHA-256:

`8bcb4650f21141fb0d4f27dd781e1752db5b0b97972a77e02a3142c837a8b16e`.

No new host/DPI acceptance, Release run, commit, archive or merge is claimed.



## 2026-11-03 — task 4.1 source/context codec



The explicit schema-2 context codec and metadata bounds are implemented before

runtime pass wiring. Schema-1 serialization remains byte-equivalent and accepted;

default runtime writers still produce schema 1 at this stage. The shared Asset

definition moved to a neutral header without changing its fields or codec.

No plugin identity, gain policy, envelope/creative version or PCM format changed.



The new native `preverb_capture_context_test` covers schema-1/2 round trips,

deterministic legacy provenance, accepted IR references, explicit missing IR,

pass/legacy boundaries, signed negative positions, exact source-union ownership,

unknown/duplicate identities, overlaps/gaps, noncanonical neighbours, unsafe and

non-IR paths, invalid enums/nonfinite/out-of-range controls, strict field types,

unknown records, capacity limits and >8 MiB metadata in both encode and parse.

Rejected decoding/migration leaves the destination intact. Independent oracles

compare legacy source JSON, session assignment JSON and retained source samples

before/after migration.



Final command: `scripts/build.ps1 -Configuration Debug -Test`, **38/38 passed in

126.59 seconds**, exit 0. Logs:

`out/build-logs/Debug/20261103-105432-c45df141/`.

The initial focused context fixture also exited 0. Task 4.1 is checked and the

refreshed CLI reports **9/25 complete**. Atomic context/source pass acceptance,

recovery/generation policy and remaining Learn coverage work are still pending.



## 2026-11-03 — task 4.2 atomic pass-context publication



Production workflow capture now consumes the pinned configuration once on the

capture worker, outside the command fence/callback. Staging owns an independent

copy; accepted source and exact context ownership publish together at Stop in

one schema-2 manifest. Compatibility adapters without a context retain schema 1.

Context-only updates preserve PCM files, range revisions and fingerprints and

do not invoke source-audio dirty callbacks. Clear removes ownership metadata

together with mapped source while preserving managed assets/immutable old records.



Exact interval tests cover first-received additive audio/protected silence,

sparse positions, last-received replacement, mixed context boundaries, a mutated

configuration after pinning, source/context invisibility during preparation and

validation, metadata-only commits, unchanged PCM-file counts/fingerprints,

reference reuse for identical updates and context discard on abort. A real

processor test verifies the admitted automatic policy and accepted IR metadata;

its audio-callback allocation guards remain enabled. A worker fixture proves

one provider copy per pass and no extra audio invalidation for context-only work.



The first full run exposed an actual integration gap: gain-compatibility changes

advanced the job epoch without updating the admission slot. The snapshot now

publishes the future epoch through the guarded CAS before the gain edit. Targeted

configuration and the previously failing performance/export fixtures both passed.

No gain algorithm or saved gain semantics changed. Existing valid unnamed IR

labels also remain accepted without fabricating or renaming historical labels.



Final wrapper: `scripts/build.ps1 -Configuration Debug -Test`, **38/38 passed in

117.16 seconds**, exit 0. Logs:

`out/build-logs/Debug/20261103-112504-5ce27709/`.

Strict change validation and diff checks pass. Task 4.2 is checked; refreshed

progress is **10/25 complete**. Fault/race coverage, deferred generation ownership,

dry-only scheduling and the remaining visual/integration acceptance are pending.



## 2026-11-03 — task 4.3 fault/race isolation



The source/context fixture now injects source-asset and manifest-directory write

failures in disposable roots, verifies failed commits retain the old context and

PCM, checks old manifest hashes, rejects prepared work after restore, proves copied

session cursors cannot mutate one another, and rejects contextless writes into an

owned schema-2 map. Context-only work also preserves the actual source dependency

key, independently of the PCM fingerprint/file-count checks.



A worker race blocks context preparation outside the command fence, restores the

previous accepted snapshot, then releases the stale producer. Restore returns

without waiting on that producer; stale work cannot publish source or context,

trigger audio invalidation or create an accepted-state notification. Earlier abort

and held-validation fixtures retain their paired source/context assertions.



Native MSVC wrapper build passed, exit 0:

`out/build-logs/Debug/20261103-113625-ba92da76/`.

Focused `preverb_capture_context_test` and `preverb_capture_transaction_test` both

exited 0; logs are `out/issue11-context-faults.log` and

`out/issue11-context-restore-race.log`. These are fixture runs, not additional

CTest passes. Production code is unchanged from the preceding 38/38 Debug run;

the new test cases will also run in final integration validation. Task 4.3 is

checked and refreshed progress is **11/25 complete**. No host acceptance is implied.



### 2026-11-03 — storage/session guidance (task 4.5)



Updated development setup and the living source/context protocol with schema-2

and creative-v9 compatibility, exact fixture interval examples, metadata-only

identity rules, the shared Stop acceptance reference, provisional-data exclusion,

legacy unknown-context labels and rollback copies. Examples were checked against

`capture_context_test.cpp` and `capture_transaction_test.cpp`'s native results

recorded above. Root relocation, portable packaging and cleanup remain outside

this change. Deferred generation acceptance remains unchecked under 4.4/5.x.



### 2026-11-03 — deferred pass ownership and recovery (tasks 4.4 / 5.1)



Native `scripts/build.ps1 -Configuration Debug -Test` passed **38/38** CTest

cases in 111.32 s, wrapper exit 0:

`out/build-logs/Debug/20261103-121003-fd5a119c/`.



`PassOwnedGenerationCases.h` captures chronological A at a later source position,

then B earlier, serializes/reopens before analysis, and discovers B before A using

their accepted contexts despite a selected B default and saved-off master. It

removes both original custom files before generation. Reopen reuses exact IR,

render, shape and calibration identities (no new convolution), including a frozen

A result. Two restored engines retain independent edits; a new manual target

inherits current B. Existing legacy coordinator tests still pass.



`automatic_targets_test` covers an A arrival whose extraction extends into B,

matched identities, protected manual edits, frozen identity/shape/source/IR,

deleted-target suppression and an honest missing-context error without current-

default substitution. Frozen automatic definitions are excluded from automatic

window resizing, arrival movement and retirement.



The first build found an accidental callback argument (fixed). The first full

run exposed an invalid frozen test fixture with no render and a processor fixture

starting before its accepted default existed. The corrected frozen metadata and

an explicit valid-default admission wait preserve those tests' intended oracles;

missing accepted IR remains separately tested as an error. Subsequent full run

passed above. Dry-only scheduling changes made after this run are separately

under validation and are not included in this result.


### 2026-11-03 — dry-only/Perform and timeline acceptance (tasks 5.2–6.4)

Native Debug build passed (wrapper exit 0):
`out/build-logs/Debug/20261103-124218-84718ffb/`.
The complete `preverb_processor_test` fixture exited **0** after that build
(`out/issue11-processor-final.log`). This includes all processor, automatic,
live/resource, loop, performance and multi-instance/export sample oracles.
`preverb_workflow_processor_test` also exited **0**
(`out/issue11-progress-final.log`). These are focused native fixture runs, not
an additional full CTest pass.

- `LearnPassPolicyCases` checks all four Recapture/Automatic combinations with
  real capture, analysis/render counters and independent original/new source
  samples. Off retains accepted renders/definitions, changed dependencies become
  stale, forced Recapture and saved legacy refresh cannot start convolution, and
  future-only Learn On plus Perform entry does not backfill Off work. Explicit
  Generate, new manual targets and source edits still prepare in Perform.
- Existing rapid marker edits/off/on cancellation and stable IR/manual/frozen/
  suppression cases pass. Per-pass immutable settings remain separate from
  explicit reanalysis. Shared live/export/audition preparation remains intact.
- Two legacy Recapture fixtures now explicitly expect stale source under Off
  before requesting Generate; they retain their original neighbour and audio
  sample oracles. The synthetic Off-learned automatic-loop/export fixture also
  explicitly requests its changed extraction window. Exact-input missing-render
  recovery remains supported; changed Off dependencies do not qualify.
- Real worker feedback arrives before storage batching and stays outside saved
  source. Additive protects silence/first-received input; Recapture uses latest
  input. Restore, failure and explicit cancellation discard the overlay. The
  closed-editor processor fixture exercises the same path. Callback allocation/
  deallocation guards stay zero. Published telemetry's measured maximum logical
  metadata ceiling is **2,195,512 bytes**, below 4 MiB, with no retained PCM.
  Both range and peak capacity pressure drop telemetry honestly, not source.
- Current native UI/model tests passed in
  `out/build-logs/Debug/20261103-123724-f2baa118/` (the full run was **37/38**;
  its processor fixture failed on the now-updated loop/export expectation, then
  passed in the focused complete rerun above). The separate UI snapshot fixture
  exited 0: `out/issue11-checkpoint-b.log`.
- `LearnTimelineCases` verifies tiny-but-audio tails, exact silence, unavailable
  assets, unfilled sparse gaps, provisional precedence and independent totals.
  Native pixels distinguish silence/gap/unavailable; keyboard readouts expose
  literal kinds and absolute ranges. The fit tests cover 86–90 then 1–4,
  same-extent recapture, one-frame origin jitter, loops/cancellation, unavailable
  timing, negative source and a trillion-sample sparse span. No wet/selection or
  host-seek input is present in this model.
- Existing native editor tests retain Perform bounds/navigation/selection and
  live/audition preferences across mode changes. Progress/no-op/rejected edits
  do not generate saved-state notifications; accepted settings/commits notify on
  the message thread. The approved header/controls are unchanged.

Checkpoint B is open in native preview PID 45948, with supporting captures under
`out/issue11-checkpoint-b/`. Owner requested detailed review instructions; no B
approval is recorded yet. Minimum/larger native layouts and 100/125/150/200%
software-raster captures are supporting evidence, **not** OS-DPI/FL Studio passes.
Full final Debug/Release validation and host smoke acceptance remain unchecked.

### 2026-11-03 — final Debug sweep and checkpoint B feedback

`scripts/build.ps1 -Configuration Debug -Test` passed **38/38 CTest**, 118.16 s,
wrapper exit 0: `out/build-logs/Debug/20261103-124615-e91c7657/`. This includes
the cancellation screenshot/assertion and the corrected complete processor suite.
Release and host acceptance are still pending; this is not task 7.1 completion.

Owner checkpoint B results: loaded view **Pass**, mixed coverage **Pass**,
recording/acceptance **Pass**, Learn/Perform return **Pass**. Hover inspection
passes; click/keyboard inspection **failed** and remains open. No repeat of the
four passing checks is requested. Reopened task 6.2 pending the focused fix.

A native-peer test replaces sole reliance on direct `keyPressed` handler calls.
The first probe incorrectly made its test window fully transparent and could
not obtain focus. An ordinary visible native peer then passes hit-tested click,
Home/right forwarding and refresh retention (`out/issue11-native-focus-repro.log`,
fixture exit 0; Debug build `20261103-125459-f4b33f05`). Actual desktop automation
could not start: both runtimes reported `windows sandbox failed:
helper_unknown_error: setup refresh had errors`. No OS SendInput success is
claimed.

The focused revision pins clicked/keyboard inspection, supplies a visible focus
outline/inspection cursor, prevents hover from replacing it, and uses Escape to
return to hover. Native-peer regression checks cover the mouse/key route and
pin retention. Those revised checks and owner acceptance are pending below; the
approved header, capture behavior and other four B passes remain unchanged.

Focused inspection revision: Debug wrapper build
`out/build-logs/Debug/20261103-125746-7fde6191/` exited 0. The complete UI/native-
peer fixture exited **0** (`out/issue11-keyboard-inspection.log`), exercising
native hit-tested click-to-focus, Home/right forwarding, refresh retention,
pinned hover immunity and Escape release. Screenshot:
`out/issue11-keyboard-inspection/learn-keyboard-inspection.png`; inspected for a
visible inset focus border, dashed inspection cursor and unchanged layout.
Task 6.2 is rechecked on this automated evidence; owner point-3 acceptance stays
pending under 6.5. Only that failed check is being repeated. Final full Debug/
Release must include this subsequent UI revision; the earlier 38/38 result is
not claimed for the later source snapshot.

### 2026-11-03 — checkpoint B approved

Owner retried only the failed click/keyboard check in the revised native preview
and reported **Point 3 passes**. The other four B checks already passed and were
not repeated: loaded display, mixed coverage, recording/acceptance and mode
return. Checkpoint B is now approved. The approved numeric control and header
remain unchanged. No FL Studio or OS-DPI acceptance is inferred from this native
review. Final Debug/Release validation and one grouped host smoke check remain.

### 2026-11-03 — final Debug/Release validation (task 7.1)

Final `scripts/build.ps1 -Configuration Debug -Test`: **38/38**, 118.75 s,
exit 0, `out/build-logs/Debug/20261103-134638-74846905/`.
Final `scripts/build.ps1 -Configuration Release -Test`: **38/38**, 48.31 s,
exit 0, `out/build-logs/Release/20261103-135047-ff3a309d/`.
Product-version, shared-header/control, native-input, context/capture/storage,
IR, automatic, gain, streaming, transport and export regressions all pass.
Strict all-spec validation: **15/15**. `git diff --check`: pass.

The preceding Debug sweep (`20261103-133740-6bb5d5e8`) was **37/38**, with native
focus left on the selector during test setup. The test now settles native peer
activation before injecting the click, checks focus and hit target, and retains
all original behavior assertions. No approved production UI behavior changed.
Both focused runs from CTest's working directory exited 0, followed by the full
Debug/Release results above. This is synchronization of setup, not a skipped
assertion, retry-until-pass production check or owner UAT substitute.

Exact artifact/source/schema/asset identities, measured resource usage and the
acceptance crosswalk are recorded in
[final acceptance](2026-11-03-learn-capture-acceptance.md), with full hashes in
`out/issue11-final-identities.json`. Only validation documentation/task tracking
changed after these builds. Host task 7.2 and final review 7.3 remain unchecked;
the owner's interaction-contract review and scope-reset observation remain open.


### Owner host acceptance — 2026-11-03

The owner reported “All three pass — usual setup” for the consolidated checklist
in `2026-11-03-learn-capture-acceptance.md`: dry-only/protected replay, automatic
IR/Recapture/Stop/Perform flow, and save/reopen. Tested Release SHA-256:
`a1ee81786a429ac1d4f4eadde62fb91c0093830d01dd68e04aac8f346d54a9b2`.
FL Studio 2026 v26.1.7 build 5653, FL Studio ASIO 48 kHz / 512 samples; no
configuration changes reported. Full DPI coverage remains untested. This does
not claim a host pass of the later read-only inspection refinements.


### Read-only inspection refinement — focused native pass

The expanded native fixture passed (exit 0), using Debug build-only wrapper
`20261103-145645-f33abe49`; screenshots are under
`out/issue11-inspection-contract/`. Exact readout model tests cover nonzero song
origin, compound meter, negative samples, beat rounding, hours, missing timing,
rate mismatch and large-coordinate fallback. Component/hit-tested mouse events
and native-peer keyboard routing cover persistent focus-independent left pin,
right-click no-op, focus-scoped Escape, real recording/held acceptance, scope and
same-ID session restore, mode roundtrip between refreshes, fresh editor state,
unchanged saved-state bytes/source references and zero dirty/job notifications.
These are deterministic native component tests, not OS-injected pointer or host
claims. The white dashed pin remains visible with focus on the Recapture switch.

During refinement, real Stop/Perform/Learn between UI refreshes exposed a reset
case, now covered by workflow-generation detection. The first full Debug rerun
(`20261103-145309-0dec3cb7`) passed 37/38: a later fixture reused a stale Recapture
invocation after its intentional restore. Using a fresh pointer invocation retains
the production stale-event protection; the complete focused UI fixture now passes.
Current-source full wrapper checks and focused owner preview approval remain
pending. No DSP or serialized-state changes were introduced by this refinement.


### Inspection approval and full Debug validation

The owner approved the revised native preview: “Approve inspection refinements”.
All earlier visual and grouped host passes are retained; no repeated FL Studio
checks were requested. `scripts/build.ps1 -Configuration Debug -Test` passed
38/38 CTests, 132.83 seconds, exit 0, wrapper logs
`out/build-logs/Debug/20261103-145813-af692d06/`.
`openspec validate --all --strict --no-interactive` passed all 15 items;
`git diff --check` passed. Final Release validation is running separately.


### Final current-source Release validation

`scripts/build.ps1 -Configuration Release -Test` passed **38/38**, 44.69 s,
exit 0; logs `out/build-logs/Release/20261103-150104-e555666c/`.
The inspection preview was explicitly approved. Current/baseline identities,
all issue criteria, retained host passes and untested full OS-DPI coverage are
cross-referenced in `2026-11-03-learn-capture-acceptance.md`.
All requested implementation and acceptance are complete; no archival or Git
publication action is implied.
