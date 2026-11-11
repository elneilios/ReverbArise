# Issue #107 P8 dialogs, choosers and auxiliary windows

## Approved baseline

The owner approved the native design in this chat before requesting
`unify-dialogs-choosers-and-windows`, then explicitly authorized its application.
The baseline is merged P3 PR #148 (`88bd5d0`) plus planning commit `ce07be8`.
The complete consumer roster and reference are recorded in the change's design.
The retained specimen is `out/ui-review/p8-windows/WindowReview.cpp`, SHA-256
`676784fa5a95a2eec317cf17acb02f91e82df0adff50d10845a5e046c7e58269`.
The specimen covered sample dialogs, native pickers, Settings/Diagnostics-style
windows, normal/compact bounds and 100%/125% software scale. This is family
approval, not production binding, host, physical DPI or live-reader evidence.

Production retains Rename OK/Cancel, confirmed captured-group deletion,
Locate/Restore/Continue recovery branches and restricted metadata-failure
messages. Existing session/pass/ID/input-generation/serial guards, Storage
receipt-versus-folder selection, two-stage working-folder adoption and
visibility-driven safe cancellation remain authoritative. Export ready-WAV
drag and cursor behavior are outside this lifecycle revision.

## Shared mechanism verification

`SurfaceFocusReturn` restores a surviving eligible opener or visible fallback
after dismissal, retaining input provenance and checking foreground ownership.
`SurfaceInvocation` rejects competing and repeated completion and invalidates
deferred restoration after replacement or owner destruction. `SurfaceEscape`
preserves a focused editor's cancellation hook before auxiliary close.
`OwnedAlertWindow` retains measured AlertWindow content/custom controls with
the approved native close chrome and shared theme.

The isolated native/component fixture reuses the warm Release libraries and
builds separately under `out/ui-review/p8-lifecycle-tests/`. Its initial
probe caught the base AlertWindow theme resetting native-title-bar policy;
the owned dialog theme now explicitly owns that native chrome. Component and
native close checks pass before consumer adoption. These fixture checks are
separate from the supported MSVC/CTest integration gate, still to be run.

## Integrated consumer dispositions

| Consumer | Delivered policy / retained authority |
| --- | --- |
| Rename and group Delete | Owned native dialog chrome, shared Name/feedback/actions, safe initial focus and stable row/list return; existing targets, IDs, epoch/pass and single-delete behavior retained |
| Asset/metadata warnings | Shared measured alert content and neutral actions; safe Continue focus, native close/Escape; existing Locate/Restore and metadata-only restrictions retained |
| Learn/selected/Settings-default IR import | Native WAV/WV picker, one pending owner, cancellation/duplicate completion and weak lifetime; delegated Settings route captures its actual opening button; existing import serial/session/pass/ID/input guards retained |
| Export Save and tile fallback | Shared completion/return policy; session/pass/file/version, overwrite and readiness guards retained; immediate ready-WAV drag and cursor ownership unchanged |
| Portable Storage Backup/Restore/Locate | Native folders or exact receipt files according to the existing route; original assets, root/epoch guards, reload and safe hide cancellation retained |
| Associated/recovery working folder | Native location/recovery/restore destinations, including chained receipt-to-folder selection; original initiating action propagated, preparation and explicit adoption/guarded handoff retained |
| Settings and Diagnostics | Shared WorkspaceWindow, Escape/native close and nested opener/fallback return; existing size/scale, comfortable text, tabs, page state, focus reveal and P3 reflow position retained |

The initial supported Release build passed, followed by all seven focused native
CTest cases in 29.50 seconds. Logs: `out/build-logs/Release/20261111-001149-39cdd0de/`
and `out/build-logs/p8-lifecycle-focused.log`. A subsequent build caught a name
collision in the added IR test (`retained`); the test variable was renamed without
changing the assertion. Additional cases cover duplicate/cancelled results,
destroyed picker owners, nested auxiliary return, hidden-parent fallback and
reverse focus reveal. Final results follow below.

The final visual reconciliation explicitly resolves owned alert title/message
fonts and background/text/outline through existing approved roles, matching the
specimen instead of inheriting unrelated default alert styling. No new token
or numerical type role is introduced. This presentation-only refinement follows
the initial full-run compilation and requires a final rebuilt focused rerun.

## Broad verification and final reconciliation

The supported Release `-Test` wrapper configured and built successfully, then
passed 98/100 native CTest cases in 441.97 seconds. Logs are retained at
`out/build-logs/Release/20261111-002103-43c113b3/`. The failures were the existing
Reset fixture's Windows foreground gate (`peer=1`, foreground false) and a new
auxiliary-return assertion whose main editor had no visible native owner.
The focus-return policy correctly refused that invisible target; the fixture
now hosts the real editor in `NativeEditorPeer` before checking window return.

Final reconciliation also applies the approved alert colour/text roles, blocks
return to an unrelated active native peer, invalidates pending chooser tickets
before Storage controls are torn down, and suppresses intermediate focus return
while a chained recovery destination picker is occupied. Existing operation
checks and safe worker cancellation/handoff are unchanged. A compile-time name
collision between the alert's text member and the named text colour was fixed
by qualifying the role. Final rebuilt focused results are recorded below;
the broad run is not represented as a second full-suite pass at the final tree.

The final production rebuild passed (`scripts/build.ps1 -Configuration Release`,
`out/build-logs/Release/20261111-003419-adb27293/`). All seven affected native
CTest cases passed in 72.12 seconds: Swell UI (including Rename, list deletion,
IR and Export), shared surface lifecycle, responsive scale, Storage Settings,
portable Storage, working folder and recovery. The focused log is
`out/build-logs/p8-lifecycle-final-focused.log`.

The unchanged Reset case passed in its visible native validation window
(`out/build-logs/p8-native-reset.log`). The auxiliary fixture was then rebuilt
with a test-only foreground requirement and passed in its visible native owner
with `PREVERB_REQUIRE_NATIVE_AUX_FOCUS=1 --test-responsive-scale`. This required
foreground peer activation and verified return to the Settings gear, the nested
Diagnostics opener and the editor fallback with Settings hidden. Build log:
`out/build-logs/Release/20261111-004315-188352a2/`; native result:
`out/build-logs/p8-native-aux-focus.log`.

The final recovery fixture additionally cancelled the folder picker opened
through the real dialog's keyboard Locate action, retained the unavailable
capture/reference, then exercised the existing Locate and Restore successes.
Its test-only rebuild and rerun passed (`out/build-logs/Release/20261111-004800-82872be8/`,
`out/build-logs/p8-recovery-cancel-test.log`, 4.01 seconds total). These final
focused checks supplement the original full run; they are not a repeated
100-case run. All builds reused the dependency cache and recompiled affected
objects. UI standards (88 files, 48 existing bounded occurrences), strict
OpenSpec, guide currency and diff checks passed. Actual FL Studio hosting,
physical Windows DPI changes and a live screen reader were not retested.

## Review delivery

[PR #149](https://github.com/elneilios/ReverbArise/pull/149) is open for owner
review. The final diff is limited to the approved P8 UI lifecycle, tests,
standards, user guide and change artifacts. The canonical specs remain pending
normal approval/sync/archive. Task-owned Release objects were cleaned after
validation and delivery; the approved specimen, review executables, dependency
caches and all logs/evidence were retained. See the local cleanup record at
`out/build-logs/p8-lifecycle-cleanup.json`.

## Review correction F1

After pointer focus in a field, Escape could close the auxiliary window before
the shared observer recorded keyboard input. `SurfaceEscape` now records it
before cancelling the draft or dismissing the window. The regression reproduced
the missing keyboard provenance before the fix and checks the returned focus
brackets when the native peer owns foreground focus.

The initial focused run also exposed a Storage fixture race between polling
and observing worker completion. The fixture now polls once more after the
worker finishes, before asserting that the prepared folder is ready. The
Release build and all three focused CTest cases passed on the final run.
Logs are retained in `out/build-logs/p8-escape-review-retest.log`.

## OpenSpec closeout

After owner approval of PR #149, the six approved requirements were added to
the canonical UI design system spec and the change was archived. All 20
canonical specs passed standard validation, and the completed change passed
strict validation before archival. Strict validation of the canonical UI spec
reports the existing length warning on the density requirement. This closeout
changes documentation only and uses the native validation recorded above.
