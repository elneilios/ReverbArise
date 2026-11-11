# Design

## Context

See [proposal.md](proposal.md) for motivation and
[the delta](specs/ui-design-system/spec.md) for observable behavior. The baseline
is merged P3 PR #148 at `88bd5d0`, production standards revision 25.

`PluginSettings.cpp` has one anonymous `WorkspaceWindow` implementation for
Settings and Diagnostics. It already uses native title bars, comfortable
typography, `PresentationSurface`, `FocusViewport`, work-area bounds and one
user-scale transform. Closing hides the window; there is no shared Escape or
opener-focus policy. Settings pages retain their own scroll/selection state.

`PluginEditor.cpp` owns Rename and group-delete dialogs. They already reuse
`FieldTextEditor`, `FieldFeedbackLabel` and `ActionButton`; deletion captures
IDs, epoch and workflow pass and restores list focus. `PluginSettings.cpp`
owns the recovery alert and its session-guarded transition to Storage.
IR, Export and Storage owners already use asynchronous native choosers and
binding-specific lifetime/context guards. Portable and working-folder panels
request safe worker cancellation on hiding; that existing behavior is retained.

### Approved native reference and limits

The owner explicitly approved the design in this chat before requesting this
proposal. The native review was launched as
`ReverbArise - #107 P8 dialogs, choosers and windows review` from
`out/ui-review/p8-windows/build/Release/ReverbArise-P8-WindowReview.exe`.
Its `WindowReview.cpp` SHA-256 is
`676784fa5a95a2eec317cf17acb02f91e82df0adff50d10845a5e046c7e58269`.
The local ignored specimen is retained; this design is the durable record.

The specimen provides Rename, three-item confirmed deletion, a recovery warning,
real Windows file/folder/save pickers, Settings tabs with About last and long
Diagnostics content. It offers 100%/125% scale and normal/compact auxiliary
bounds, Escape and native close, safe initial focus and return to the launcher.
Its automated check passed layout and Name validation. Owner approval is design
acceptance, not proof of every production binding, host, DPI or accessibility
path. Picker selection in the specimen performs no asset operation.

Production keeps its actual labels and branches: Rename's existing OK action,
both recovery Locate and Restore actions, Continue, and the restricted metadata
failure warning. The specimen's sample labels, fixed sample count and simplified
recovery-to-folder demonstration do not replace those semantics. Production
settings, receipt-based restore and working-folder two-phase adoption require
integration coverage rather than another request to repeat the same design.

## Goals / Non-Goals

**Goals:** establish one small shared lifecycle policy for existing dialogs,
choosers and auxiliary windows; complete the consumer matrix below; retain
safe asynchronous completion and accepted presentation/field policies.

**Non-Goals:** custom file browsers, a new window-management framework, new
recovery actions, token redesign, freely resizable auxiliary-window preferences,
new edit/undo rules, DSP or asset-format changes. Keep the established single-row
delete behavior; group deletion still confirms. Export file preparation,
ready-WAV drag and cursor behavior remain owned by merged P8. Recovery candidate
selector debt, hidden compatibility controls, umbrella closure and unrelated
completed-change archives are separate work.

## Decisions

### Extend existing owners with a shared lifecycle helper

Extract or extend `WorkspaceWindow` and the working dialog mechanisms with a
small shared shell/completion/focus-return policy. Keep content and command
callbacks with their current owners. Reuse C1 commands, C3 text, C4 fields and
feedback, C7 connected tabs/scrolling, `PresentationSurface` and
`FocusVisibility`. Native dialog title-bar close follows cancellation. Reuse
reviewed metrics and colours; the new shell is not permission to restyle nested
controls or regenerate the standards debt inventory.

This concentrates Escape, one-shot completion, lifetime and focus return without
copying slightly different policies into every callback. A general modal
framework or a verbatim copy of the specimen would replace proven domain
bindings unnecessarily. An unchanged compliant consumer may be verified and
recorded rather than rewritten.

### Keep surface dismissal and command authority separate

Tab/Shift+Tab traverse actual eligible controls. Rename starts in the selected
Name field; group delete starts on Cancel; recovery starts on its safe Continue
action. Return/Space act through the focused control, never an unconditional
destructive default. Preserve explicit Rename submission and wrapped red field
errors. A duplicate dismissal cannot invoke completion twice.

Escape first honors the field's existing cancellation behavior and dismisses
the owning dialog/window without allowing a queued focus-loss callback to
publish the cancelled draft. Native picker Escape remains OS-owned. Closing a
nonmodal Settings page is not a new transaction rollback: accepted values stay
accepted, and ordinary close/focus-loss follows existing field bindings.
Storage visibility changes continue their current safe cancellation and guarded
handoff. Do not reinterpret hiding as a new Stop implementation or alter
copy/commit boundaries.

### Return focus after reconciliation to a live, visible owner

Capture a safe opening-control reference, a stable owner fallback and input
provenance at invocation. Restore only after the surface is dismissed and the
binding has reconciled visibility/eligibility. Keyboard and accessibility return
show brackets; pointer return does not. Shared fields retain caret/selection.

For virtualized rows use the existing stable Swell identity/list restoration
path, not a saved row-component address. For Diagnostics prefer its opening
Settings action; if that owner is hidden, choose the eligible visible editor
fallback without reopening Settings. Destroyed/hidden/ineligible controls cannot
receive focus. Defer native restoration until completion unwinds, and drop it
if its owner or activation context is obsolete. Background worker messages do
not raise a window or steal focus from another application.

### Preserve native picker ownership and captured results

Leave JUCE's Windows-native chooser mode, selection filters and overwrite
handling in place. Add only the missing owner-specific pending-invocation and
focus-return integration; do not globally serialize unrelated owners. Keep
current chooser test seams and submit continuations. Selection still goes
through domain eligibility, session/pass/ID/revision/epoch guards and lifetime
checks, never directly through a generic shell callback. Destruction safely
disposes pending UI and invalidates deferred restoration.

An empty result reconciles chooser state and focuses its surviving opener;
it performs no import, copy, save, adoption or asset recovery. A successful
native selection is not itself permission to accept a stale domain request.
Backup receipt files remain distinct from destination/original-asset folders;
working-folder selection still prepares before explicit adoption.

### Retain the existing auxiliary layout and state model

Keep the Settings and Diagnostics logical sizes, native chrome and comfortable
typography. Their existing presentation surface applies magnification once;
JUCE still owns Windows DPI. Bound the native frame to the work area and use
existing focus-aware scroll containers when content does not fit. Preserve P3
scroll positions during reflow, page selection, About-last ordering and current
field drafts/accepted values according to binding rules. Reuse an existing
window on repeated opening, and retain current close/reopen page state.

The specimen's compact bounds are a review fixture, not a new saved density or
window-size preference. Host resize chrome remains platform-owned; existing
editor size/scale behavior receives regression coverage, not redesign.

### Complete the actual consumer roster in this change

| Consumer and source | Adoption or verification | Preserved authority |
| --- | --- | --- |
| Rename and group Delete, `PluginEditor.cpp` | Shared shell, keyboard, dismissal and stable-list focus return | Captured swell/session; delete IDs/epoch/pass; explicit Name validation; single-delete behavior |
| Asset and metadata recovery alert, `PluginSettings.cpp` | Shared dialog controls, safe initial action, closing and chained-picker return | Existing warning text, permitted Locate/Restore/Continue branches and captured recovery input |
| `ConvolutionIrBinding::openFile` for Learn defaults, selected swell and Settings default-import route | Native picker lifecycle, pending-owner and focus return | Import serial, weak lifetime, default session/pass or selected ID/input generation; `*.wav;*.wv` filter |
| `ExportFileInvocations` Save and file-tile Save fallback | Verify native Save cancellation, lifetime and focus integration; adapt only missing lifecycle | Captured session/pass/file/version, choosing/save serial, readiness and overwrite rules; drag/cursor untouched |
| `PortableStoragePanel` Backup, Restore, Locate and original-asset recovery pickers | Native folder/receipt completion and return to visible action | Busy/choosing/reload guards, instance root/control epoch, exact receipt choice, hidden-owner rejection and worker cancellation |
| `WorkingFolderPanel` Choose, Locate and restore/recovery destination routes | Native folder completion, keyboard and focus return | Prepared candidate, explicit Use This Folder, source/session comparison, safe handoff and original assets |
| Settings `WorkspaceWindow` and its existing pages | Escape/native close, opener fallback, reuse, single scale, fit and focus reveal | Settings bindings, selected page, accepted values and P3 reflow/scroll behavior |
| Diagnostics `WorkspaceWindow` and existing diagnostic viewport | Nested return to opener, close-state reconciliation, fit and focus reveal | Diagnostics-open state, read-only precision/copy data and throttled refresh |

### Reuse the approval and verify integration

The approved preview is the scoped family checkpoint. During apply, preserve its
open review window and reuse its design; request new owner look/feel review only
for a material visual or interaction deviation. Production screenshots and
native tests verify adoption without inferring host/DPI passes from the specimen.
The production style guide, C8 contracts and inventory move together to the next
standards revision (26 at this baseline); introduce no unreviewed token.

## Risks / Trade-offs

- Escape, focus loss and async completion can race -> one-shot dismissal and
  guarded deferred focus; assert no accepted edit from a cancelled draft.
- A native picker can outlive a row, page or editor -> safe references, stable
  fallbacks and retained domain guards; exercise stale and destroyed contexts.
- Window hiding already affects Storage work -> retain visibility-driven safe
  cancellation and committed-handoff semantics; run existing recovery/persistence
  fixtures as well as UI cases.
- Native foreground focus differs from component focus -> assert native peer
  activation in a focused test window where required; record any environment
  limitation and keep actual FL Studio coverage separate.
- Nested scroll surfaces and scaling can clip focused controls -> verify both
  directions through actual Settings/Diagnostics descendants at 100%/125% and
  constrained bounds, including unchanged reflow reading positions.

## Migration Plan

Record the existing approval and baseline consumer matrix, add the shared
lifecycle support and focused tests, then adopt all named dialog, chooser and
auxiliary consumers in this change. Keep already compliant behavior by verified
reuse. Run supported native verification, standards checks, strict change
validation and guide-currency review; update living guidance and dated evidence.
Deliver one review PR with `Refs #107`; do not close the umbrella or archive
unrelated completed changes. A source revert is sufficient rollback because
this adds no state or asset migration.
