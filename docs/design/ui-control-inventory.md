# UI control inventory and family roadmap â€” issue #107

Initial audit: `24cd287`; reconciled baseline: merged #80 at `717c890`,
6 November 2026. Status: **owner-approved audit and contracts**, integrated as
production standards revision 11 after #80's revisions 8â€“10. This document records
source inspection. The separate command-button child now records native family
revision-4 approval and production adoption at standards revision 12; its
[evidence](../validation/2026-11-06-issue-107-buttons.md) separates integrated
acceptance and automated results. The family rollout shipped in the child changes
listed below. Independent #108 Level and #8 recovery/engine work retains its own
delivery and evidence.

The selector child `unify-selector-states` adds the native-approved C2 family and
named consumer adoption at standards revision 14 (7 November 2026). Its
[evidence](../validation/2026-11-07-issue-107-selectors.md) records local field
feedback approval, binding checks and remaining host/DPI coverage. Action menus
shipped in `unify-action-menu-states`. The Recovery selector deferred during #8
adopts C2 in `unify-recovery-onset-selector`.

## How to read this inventory

Each row identifies the actual member(s), source/owning policy, surface, semantic
role, applicable state/interaction differences, approval reference and disposition.
Member lists are explicit; grouped rows do not imply screen-wide compliance.
Nested and dynamically created controls are included under their owning family.
The [contracts](../development/ui-control-contracts.md) define family IDs C1â€“C8;
the packets below identify review and verification required before migration.

State key: I idle, H hover, P pressed, F keyboard focus, D disabled,
S selected/latched, E editing, V invalid/rejected, B busy/unavailable.
Noninteractive text has I/D/overflow, not press or activation states. Graphs have
their own hover/drag/selection semantics. "Legacy" means explicit remaining debt;
"retained" means reuse existing policy, not newly certified compliance.

All shared C1 command consumers now inherit the revision-12 painter/input policy,
including previously shared header, Learn, IR, timeline and Export references.
Their earlier approvals continue to describe their command/binding scope; the
button evidence covers the revised common states. FileDragTile retains its
specialized painter, and generic Rename buttons retain the neutral fallback.

Revision 13 adds keyboard/accessibility-only focus decoration to the shared
command family, with actual focus and mouse feedback retained. The owner directed
the same policy for other applicable controls during their own design-system
jobs; see the [decision and boundaries](../development/ui-control-contracts.md#focus-visibility-policy).
This is not an immediate migration of the remaining C2â€“C8 renderers.

## Header and shared workspace feedback

Sources: [WorkflowHeader](../../Source/UI/WorkflowHeader.h),
[editor declarations](../../Source/Plugin/PluginEditor.h),
[editor composition/bindings](../../Source/Plugin/PluginEditor.cpp).
Approval baseline: production guide revisions 1/7 and issue #35/#58 evidence;
this audit adds no acceptance.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| H1 `workflowHeader.settings` | C1 IconActionButton, command | I/H/P/F/D; merged #80 shares ActionButton surface with stable glyph ink; explicit keyboard focus and Return/Space | Retained approved revision 9; P1 regression reference, no duplicate migration |
| H2 `workflowHeader.selector` and dynamically rebuilt `Segment` children | C7 connected SegmentedSelector, workflow navigation | I/H/P/F/D/S; arrows/Home/End inspect, Return/Space request; unavailable choice can be inspected; accepted selection separate from focus; shared edges painted by parent | P7 tabs adopted; #35 guards retained and unavailable inspection covered in ControlLibrary and WorkflowHeader checks |

## Learn

Source: [LearnWorkspace](../../Source/UI/LearnWorkspace.h). Approval: Learn #11,
shared controls #16/#37, responsive #58 and merged #80 shared IR field.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| L1 `generate` | C1 ActionButton primary command | I/H/P/F/D/B; aqua primary fill with hover fill; beforeInvocation pins Generate provenance, accepted readiness disables | P1 retained reference, preserve generation serial/session/pass |
| L2 `irEditor.fileButton` | C1 IconActionButton import command inside ConvolutionIrEditor | I/H/P/F/D/B; shared command surface/stable glyph ink; async chooser is binding-owned | Retained #80/revision 9; P1 reference, no duplicate migration |
| L3 `irEditor.selector`, `irEditor`, `irBinding` | C2 IrSelector/ChoiceSelector and shared field/binding | Quiet independent hover/keyboard/selected states, outer aqua focus, small local feedback/red error border; no wheel reassignment; captured default catalogue/session/pass retained | Migrated in `unify-selector-states`; native family approved 2026-11-07; shared IR component/binding checks pass |
| L4 `recapture`, `automatic`, `autoStop` | C6 Switch | I/H/P/F/D/S; trailing track, clickable label and keyboard/accessibility brackets | P6 adopted with Learn pass guards |
| L5 `sections` and generated segments | C7 connected SegmentedSelector, Capture & Generate/Sound navigation | I/H/P/F/D/S; same accepted-selection mechanism as H2, different responsive text role | P7 tabs adopted; Learn retains #58 section and responsive layout checks |

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| L6 `sensitivity`, embedded FieldNumericInput and steppers | C5/C4 SliderControl + InvocationSlider | I/H/P/F/D/E. Shared percent entry and track. Plain/Ctrl+wheel moves 1/0.1 percentage points on the track. Learn pass remains pinned. | P4 field and P5 track adopted. |
| L7 `captureTitle`, `captureGroup`, `recaptureLabel`, `recaptureHelp`, `automaticLabel`, `automaticToggleLabel`, `automaticHelp`, `sensitivityLabel`, `autoStopLabel`, `autoStopHelp`, `manualHelp`, `generateReason`, `soundScope`; nested `irEditor.label`, `irEditor.status` | C3 screen labels/roles plus shared IR EllipsisLabel/FieldMessage | I/D/B/overflow; IR feedback uses normal helper font, red actionable failure, readable disabled-input feedback and collapsed empty space; scope remains screen-owned | Retain #80 revisions 8/10; P3 measured wrapping adopted for screen prose; existing IR feedback retained |
| L8 painted `timelineHeading`, `timelineCaption`, `timelineRuler`, `timelineStatus`, empty prompt and interactive `chart` | C3/C8 LearnWorkspace specialized canvas | I/H/F/S/B; left-drag pin, wheel zoom, middle-drag pan, Home/End/arrows/Escape; recorded/silence/gap/unavailable distinct; no generic input field | P8 waveform gestures and shared focus adopted, retaining #11/#58 inspection |

## Perform

Sources: editor declarations/composition above,
[inspector binding](../../Source/Plugin/PluginInspector.cpp),
[timeline](../../Source/UI/SwellTimeline.h). No binding is replaced.
Approval: scoped #46/#49/#4/#68/#93/#58 evidence, not blanket legacy compliance.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| P1 `addButton`, `auditionButton`, `stopButton`, `resetButton`, `resetMenu`; `irEditor.fileButton` retained shared reference | C1 ActionButton plus shared icon command | I/H/P/F/D/B; explicit add/play/stop glyphs and primary/destructive roles; shared outer corner focus and semantic mouse states; names are metadata | P1 implemented; native family revision 4 approved, native integration approved; unchanged callbacks/reset guards and IR binding |
| P2 `snapButton`, `auditionWetOnly` | C1 LatchingActionButton, accepted effective latch | I/H/P/F/D/S; shared semantic painter for off/on; requested/effective Snap and audition session/pass remain distinct | P1 implemented; silent accepted-state refresh and existing bindings retained; family approved, native integration approved |
| P3 `gridBox`, `irEditor.selector`, `irEditor`, `irBinding` | C2 ChoiceSelector and shared ConvolutionIrEditor/binding | Quiet inspection, explicit commit, no wheel; IR retains captured selected ID/input/epoch; grid retains rebase/cancel/pass guards | Migrated in `unify-selector-states`; native-approved family; component and actual-editor binding coverage |
| P4 `enabledButton`, `soloButton`, `freezeButton` | C6 BooleanToggle checkbox | I/H/P/F/D/S; compact inline indicators and shared focus | P6 adopted with captured selected-swell scope |
| P5 `inspectorSections` and generated segments | C7 connected SegmentedSelector, Timing/Envelope/Sound | I/H/P/F/D/S; shares navigation mechanism, composition owns section choice and visible content | P7 tabs adopted; Perform retains #58 section and responsive layout checks |
| P6 `performZoom.minus`, `.plus`, `.fit`, `.slider` | C1/C5 TimelineZoomControls, presentation intents | I/H/P/F/D. Shared track focus and wheel steps without a value box. Command buttons retain limit-specific states. | P1/P5 adopted. Keep #49 navigation and Fit behavior. |

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| P7 `valueControls[0..10]`: Target, Source Start, Source End, Pre-roll, End Offset, Fade In, Fade Out, Level, Curve In, Curve Out, Move Wet | C5/C4 KnobValue, nested Dial/Caption/FieldTextEditor | I/H/F/D/E/V. Shared dial state, domain wheel steps, optional 0 dB Level detent, guarded Reset and exact entry. | P4 entries and P5 dials adopted. |
| P8 `inspectorHeading`, `listEmpty`, `inspectorEmpty`, `auditionHeading`, `editorReadout`, `listCount`, `listLabel`, `inspectorGroups[0..2]`; `selectedStatus`, `monitorStatus`, `performSoundScope`; nested `irEditor.label`, `irEditor.status` | C3 roles/plain Labels, EllipsisLabel and IR FieldMessage | I/D/B/overflow; screen scope outside shared IR field; feedback collapse reflows Level while Reset stays fixed; remaining statuses ellipsize | P3 shared roles and explicit single-line/wrapped labels adopted; existing field feedback retained |
| P9 `list`, generated `RowSurface`, inherited ListBox viewport/scrollbars | C7 SwellList specialized identity list | I/H/P/F/S/D/B; real Enabled and More controls expose row actions, with Tab traversal for the primary row; ListBox owns selection and refresh preserves identity and scroll | P7 adopted at revision 23, with #4 and #93 behavior retained |
| P10 `inspectorViewport`, `inspectorContent` | C7 FocusViewport + screen layout | scroll/focus/overflow; reveal policy explicit, not implied by viewport type | P7 verified with existing focus reveal and pinned Reset |

## Export

Sources: [ExportWorkspace](../../Source/UI/ExportWorkspace.h),
[ExportLevelsControl](../../Source/UI/Controls/ExportLevelsControl.h),
[FileDragTile](../../Source/UI/Controls/FileDragTile.h). Approval: #6/#51/#58.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| E1 `play`, `stop`, `save`, `back` | C1 ActionButton; primary play, neutral commands | I/H/P/F/D/B; icon-only play/stop versus text+icon Save/Return; state/callback eligibility remains export-owned | P1 retained reference; no export command redesign |
| E2 `tile` | C1/C8 FileDragTile, native-file gesture variant | I/H/P/F/D/B; dotted aqua pointer-idle boundary, solid active drag or visible keyboard focus; threshold starts one ready-file drag and suppresses that press's Save; click/Return saves | P8 Export file slice adopted at revision 22; retained #6 file tile variant |
| E3 `mix.selector` | C2 ChoiceSelector, Character preset | Independent H/P/F/S/D; no wheel, exact canonical pair defines inspectable unavailable Custom; preset emits paired edit | Migrated in `unify-selector-states`; #51 atomic pair validation and silent refresh retained |
| E4 `zoom.minus`, `.plus`, `.fit`, `.slider` | C1/C5 TimelineZoomControls | I/H/P/F/D. Shared track and command states. Export retains its own zoom scope and pass guards. | P1/P5 adopted. |

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| E5 `mix.dry`, `mix.wet`, embedded FieldNumericInput and steppers | C5/C4/C3 ExportLevelsControl | I/H/P/F/D/E. Shared tracks use 1/0.1 percentage-point wheel steps. The untouched level and accepted Character are preserved. | P4 fields and P5 tracks adopted. |
| E6 `navigation`; painted heading/caption/mixHeading/ruler/chart/legend/timeArea/statusArea | C8 TimelineNavigation and ExportWorkspace paint | I/H/F/D/B; left positions ready preview, middle-drag pans, wheel zooms, Home fits; previous waveform/status distinct from accepted output | P8 waveform gestures and shared focus adopted, retaining #6/#51 preview and file ownership |

## Settings

Source: [PluginSettings](../../Source/Plugin/PluginSettings.cpp) moves existing
editor members into pages; these are not duplicate controls in Diagnostics.
Playback, Processing, Swell, Audition, Recovery, About plus conditional Appearance
are covered. About has #59 acceptance; Appearance/auxiliary limitations remain
as recorded in #58 evidence.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| S1 `gainPolicyBox`, `liveRoutingBox` (Playback); `inCurveBox`, `outCurveBox` (Swell); `candidateBox` (Recovery) | C2 ChoiceSelector | Deliberate no-wheel inspection/commit with captured binding context; gain prototype remains inspectable/unavailable. Recovery refreshes by sample identity and keeps Confirm separate. | `unify-selector-states` covers Playback/Swell. `unify-recovery-onset-selector` covers Recovery. |
| S2 `defaultIRButton` (Processing); `fullButton`, `setBarsButton`, `removeButton` (Swell); `generateButton`, `allButton`, `analyzeButton`, `confirmButton`, `recaptureButton`, `cancelButton`, `clearButton`, `suppressionButton`, `openDiagnosticsButton` (Recovery) | C1 neutral ActionButton | I/H/P/F/D/B; shared mouse/focus states and eligible keyboard/accessibility path; Delete/Clear retain neutral classification | P1 implemented; callbacks and guards retained; default import retains shared binding; native integration approved |
| S3 `copyBuildInformation` (About) | C1 primary ActionButton | I/H/P/F/D; intentionally primary copy action already explicit, success feedback accessible event | P1 retain #59 scope |
| S4 `liveButton` (Playback), `mappingButton` (Processing), `muteButton` (Swell), `isolateButton`, `isolateDryButton` (Audition), `toneButton` (Recovery) | C6 Switch | I/H/P/F/D/S; labeled track and thumb, retained unavailable on state | P6 adopted with existing playback, edit and tone rules |
| S5 `automaticButton`, `autoButton` (Processing) | C6 Switch | I/H/P/F/D/S; labeled track and thumb with captured pass | P6 adopted with guarded activation and silent reconciliation |
| S6 `presentationSizeBox`, `presentationScaleBox` (Appearance) | C2 ChoiceSelector | Independent H/P/F/S/D; no-wheel; explicit choice updates presentation profile, not project state; unavailable fit choices remain inspectable | Migrated in `unify-selector-states`; retain #58 composition/profile behavior |
| S7 `settingsTabs`, generated tab buttons, existing pages and Storage viewport | C7 SettingsTabs and connected SegmentedSelector | I/H/P/F/D/S; stable page bindings, Appearance when available, About last | P7 tabs adopted; Storage, recovery, About and responsive tests cover page identity and geometry |

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| S8 `liveLevel` (Playback), `separationSlider` (Processing), `tailSlider` and `auditionSlider` (Audition), `toneStartSlider` and `toneEndSlider` (Recovery) | C5/C4 SliderControl with compact FieldNumericInput | I/H/P/F/D/E. Shared tracks use domain wheel steps. Compact boxes keep their suffix, range and reset rules, and Separation keeps its captured pass. | P4 fields and P5 tracks adopted. |
| S9 `barStartEditor`, `barEndEditor` and their captions | C4 FieldTextEditor and C3 FieldFeedbackLabel | I/F/D/E/V. Digits only, shared wrapping feedback. Focus loss checks the draft. Return and Set source from bars submit to the bound swell and session. Context changes clear the pair. | P3 presentation retained and verified in P4. |
| S10 `settingsLabels[0..11]`, `settingsHelp[0..5]`, `gainPolicyLabel`, `liveLevelLabel`, `separationLabel`, `tailLabel`, `auditionLabel`, `toneStartLabel`, `toneEndLabel`, `recaptureStatus`; `regenerationFeedback` | C3 shared captions, WrappedLabel help and EllipsisLabel feedback | I/D/B/overflow; shared semantic roles and measured prose; regeneration feedback retains session | P3 adopted with measured Settings help and scrollable page content |
| S11 `aboutHeading`, `buildInformationLabels[0..2]`, `buildInformationValues[0..2]` | C3 roles + EllipsisLabel | I/overflow; full build value in description/tooltip; copy feedback uses standard accessible event | P3 verified existing About roles, full build values and copy authority |
| S12 `presentationSizeLabel`, `presentationScaleLabel`, `presentationHelp` | C3 Appearance composition | I/B/overflow; fit recovery help distinct from requested profile and effective size | P3 shared labels with existing #58 profile behavior |

## Diagnostics

Sources: editor composition and PluginSettings above. Members moved into Settings
are listed only under S rows. Explicitly retained hidden diagnostic seams are not
treated as production primary controls.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| D1 `timingButton`, `assetInfoButton` | C1 neutral ActionButton; timing disclosure/copy assets | I/H/P/F/D; shared semantic painter and activation; existing clipboard/disclosure callbacks | P1 implemented; family approved, native integration approved |
| D2 `copyDiagnosticsButton` | C1 ActionButton | I/H/P/F/D; shared neutral surface, explicit title/help | P1 retained reference |
| D3 `sampleUnitsButton`, `markerButton`, `recaptureExisting` | C6 retained checkbox/invocation seams | I/H/D/S if exposed; exact sample seam and old header controls hidden/zero bounds in current layout | P6 verified as retained compatibility seams |

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| D4 `status`, `detail`, `captureDiagnostic`, `playbackDiagnostic`, `policyDiagnostic`, `liveDiagnostic` | C3 diagnostic text/labels | I/B/overflow; long technical content and throttled refresh; copyDiagnostics exposes full text | P3 measured multiline labels adopted, retaining diagnostic precision and full copy |
| D5 `fields[name..fadeOut]` and `placementEditor`, with their captions | C4 shared read-only FieldTextEditor | I/F/D/overflow. Accessible precision and copying, existing timing visibility, and hidden Name. | P4 read-only adoption. |
| D6 `markerKnob`, `regenerationKnob`, `markerLabel`, `markerState`, `regenerationLabel`, `headerStatus`, `spanLabel`, `helper` | C5/C3 retained legacy controls and labels | Hidden or zero-size sensitivity boxes and their saved compatibility values remain in place. | P4 and P5 audited and retained as compatibility seams. |
| D7 `diagnosticsViewport`, `diagnosticsContent` | C7 FocusViewport/FlatSurface | scroll/focus/overflow; diagnostic panel inside separately bounded auxiliary window | P7/P8; no independent duplicate model |

## Specialized interactions and native boundaries

| ID / actual consumers | Owner / semantics | States / differences | Disposition |
| --- | --- | --- | --- |
| N1 `showSwellMenu` Rename/Delete, modal Name field, and group confirmation | C2 ChoicePopup; C8 OwnedAlertWindow with C4 FieldTextEditor and C1 ActionButton | Right-click or three dots opens the same menu. Rename is unavailable during multiselect. Group Delete confirms the captured IDs with Cancel initially focused. Escape dismisses without an edit. | Revision 23 popup/row integration retained; revision 26 adds native dialog close, shared dismissal and stable-list focus return. Rename keeps P4 validation. |
| N2 `KnobValue` reset popup per value cell | C2 `ChoicePopup` neutral action and C5 reset provider | Right-click anchor, provider label/eligibility and captured Reset closure. Pointer return preserves focus with brackets suppressed. Inspector `resetMenu` is the separate Reset All command | Popup adopted at revision 16 on the merged #108 Level baseline |
| N3 Learn/selected ConvolutionIrBinding `chooser` (Settings default import routes through Learn binding), Export save `chooser`, native external file drag and editor host resize chrome | C8 platform surfaces; application owns launch/completion | OS-managed chooser focus, overwrite and cancel. Export captures session/pass/file; Windows copy drag shows WAV or stop feedback from the target's drop effect. | Export drag remains at revision 22; revision 26 adds the shared native-picker completion/focus lifecycle to IR and verifies Save. Host resize chrome keeps platform ownership and responsive checks. |

| ID / actual consumers | Owner / semantics | States / differences | Disposition |
| --- | --- | --- | --- |
| N4 `timeline`, source/wet handles, ruler, overview, scrollbar, `waveZoom` and its caption | C8 SwellTimeline/SwellOverview, C5/C4 display-only Height | I/H/F/S/D/drag/overflow. Height uses the compact field and shared 3/1 dB wheel track with a 0 dB reset. | P4/P5 Height and P8 waveform gestures adopted. Detail left places the local playhead or edits a handle. Compact Overview left-drag pans. |
| N5 `settingsWindow`, `diagnosticsWindow` WorkspaceWindow; nested `surface`, `viewport`, `comfortableTheme`; editor `presentationSurface` and host resize affordance | C8/C7 PresentationSurface, FocusViewport, native DocumentWindow chrome | logical size versus single magnification transform; comfortable auxiliary typography; bounded work-area scroll and focus reveal; native close behavior | P8 revision 26 close/Escape and guarded opener return; retained #58 scale/fit and P3 scroll policy. Actual host/DPI coverage separate |
| H3 `workflowHeader.wordmark`, `.tagline`, `.role`; editor `hoverHelp`, `noticeBar`, `hostInfo` | C3 Wordmark/Label/HeaderInformation/EllipsisLabel/NoticeBar/TransportReadout | I/B/overflow; wordmark SVG; compact hides tagline, role wraps; bottom hoverHelp full description, notice severity/precedence, transport fixed slots and unavailable text | P3 retained #35/#46/#68/#58 and no-forced-speech notice policy |

### Source and acceptance references

The [production guide](ReverbArise_UI_Layout_and_Style_Guide.md) is the numerical
authority. Current acceptance/debt evidence includes
[#37 standards](../validation/2026-11-04-issue-37-ui-standards.md),
[#46 integration](../validation/2026-11-04-issue-46-integration.md),
[#46 knob audit](../validation/2026-11-04-issue-46-knob-audit.md),
[#51 levels](../validation/2026-11-05-issue-51-independent-levels.md),
[#58 integration](../validation/2026-11-06-issue-58-integration.md),
[#59 copy feedback](../validation/2026-11-05-issue-59-copy-feedback.md),
[#68 readout](../validation/2026-11-05-issue-68-transport-readout.md) and
[#93 reveal](../validation/2026-11-05-issue-93-selection-reveal.md).
Prior acceptance is scoped to those records; this audit does not extend it.

Sources for nested mechanisms:
[KnobValue](../../Source/UI/KnobValue.h),
[SliderControl](../../Source/UI/Controls/SliderControl.h),
[NumericInputControl](../../Source/UI/Controls/NumericInputControl.h),
[SwellList](../../Source/UI/Controls/SwellList.h),
[TimelineZoomControls](../../Source/UI/Controls/TimelineZoomControls.h),
[Presentation settings](../../Source/Plugin/PluginPresentation.cpp),
[FocusViewport](../../Source/UI/Controls/FocusViewport.h).
Popup items, text-box editors, row hit regions, tabs and native windows are included
even when they have no persistent Button/Label member of their own.

## Family roadmap and delivery packets

The P3 rollout accounts for every visible text group, including the closed #8
Storage/Recovery surfaces. Its [consumer dispositions and evidence](../validation/2026-11-10-issue-107-text-help.md)
identify the planned shared implementation, verified existing policy or retained
native presentation for each group.

### Merged-main addendum

Main `f98157e` subsequently delivered the independently owned #8 recovery/storage
work. It is preserved when integrating this button child; the original 20-command
migration boundary does not expand. The newly present shared commands are recorded
here without claiming a new storage/recovery audit or repeating that delivery's approval.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| X1 `PortableStoragePanel.backupButton`, `restoreButton`, `locateButton`, `cancelButton`; `StorageInventoryPanel.cancelButton` | C1 existing neutral ActionButton; #8 owns chooser, worker, reload and scan bindings | I/H/P/F/D/B; existing idle/busy/chooser/reload guards and conditional stop actions; inherits the reviewed common command painter | Retained main declarations/bindings, no migration in this child. Native storage regressions remain part of the combined suite; recovery semantics retain their separate #8 evidence |
| X2 PortableStoragePanel and WorkingFolderPanel native folder/receipt routes | C8 SurfaceInvocation and guarded binding callbacks | One pending picker, native modes/filters, cancellation, delegated opener and chained receipt-to-folder return; session/root/epoch and safe handoff remain binding-owned | Revision 26 P8 lifecycle adoption; full route and verification roster in the [P8 evidence](../validation/2026-11-11-issue-107-dialogs-windows.md). C1 commands and Storage operation semantics retained |

### Delivery packets

The following packets are candidates for separate child OpenSpec changes. No
packet is authorization to change production code or create a GitHub issue.
An explicit member group may be subdivided into named consumer lists when it
would otherwise create an oversized review, without fragmenting contract ownership.
Each family plan lists all consumers across the plugin and records adoption,
existing shared coverage or an owner-approved specialized variant for each one.
Keep the family open until every consumer has a recorded disposition.

| Packet / proposed order | Exact scope and reusable ownership | Dependencies, guards and disposition |
| --- | --- | --- |
| P1 Commands, glyph actions and latches | H1, L1/L2, P1/P2/P6 command children, E1/E4 command children, S2/S3, D1/D2; C1 shared command family | Delivered in PR #114. Native family revision 4 and the IR alignment correction were approved. Explicit roles replace all 20 persistent legacy TextButtons. Callbacks, captured ownership and effective latch state are preserved. |
| P2 Selectors and menus | L3, P3, E3, S1 and S6 use the shared selector. N1 and N2 use the shared action presenter. | Selectors preserve catalogue/ID/epoch/input/pass and deliberate no-wheel policy. Swells and Reset retain captured actions and pointer anchoring. The Recovery follow-up uses the approved C2 presentation and preserves the existing sample and Confirm behavior. Rename dialog input is covered by P4. |
| P3 Text, context help and feedback | Shared measured wrapping and full-description labels across H3, L7/L8, P8, E5/E6, S10/S11/S12, D4/D5/D6 and N4, including Storage/Recovery from closed #8. S9 source-range feedback shipped separately in #122. | Revision 25 adoption. Existing field feedback, notice precedence, transport geometry and full diagnostic copying retain their approved specialized policies. See the P3 consumer evidence and [S9 validation](../validation/2026-11-07-issue-107-field-feedback.md). |
| P4 Numeric and text fields | L6, P7, E5, S8/S9, D5, N1 and N4 adopted or verified by `unify-numeric-text-fields`. D6 audited as retained hidden seams. | Shared text and numeric surfaces, wrapped feedback where it has a position, exact and explicit edit lifecycles, read-only copying and configured wheel steps. |
| P5 Knobs and sliders | L6, P6/E4, P7, E5, S8 and N4 adopted by `unify-knob-slider-interactions`. D6 retained as hidden compatibility seams. | Shared dial/track feedback and Ctrl+wheel, Level home detent, preserved gesture guards, Reset, independent pair edits and display-only values. |
| P6 Switches and toggles | L4, P4 and S4/S5 adopted by `unify-switch-toggle-states`. D3 retained hidden. | Shared toggle states. Labeled switches in Learn and Settings, inline checkboxes in the Swell Inspector, and preserved pass and binding authority. |
| P7 Connected tabs | H2/L5/P5 and S7 adopted by `unify-connected-tabs`. | One connected strip and deliberate selection contract, with About last in Settings. |
| P7 Lists and scrolling | P9/P10, D7 viewport and N5 scroll viewport covered by `unify-swells-list-and-scrolling`. | Row actions, confirmed group Delete, disabled popup feedback and existing focus reveal. See the [validation record](../validation/2026-11-10-issue-107-swells-list-and-scrolling.md). |
| P8 Export file gestures | E2 and Export N3 Save WAV, tile Save fallback and ready-file drag | Adopted at revision 22. See the [Export validation record](../validation/2026-11-10-issue-107-export-file-invocations.md). |
| P8 Waveform gestures | L8, E6 and N4. Shared main-canvas gestures and focus with a left-drag-only Perform Overview. | See [waveform gesture evidence](../validation/2026-11-10-issue-107-waveform-gestures.md). |
| P8 Dialogs, choosers and auxiliary shells | N1 dialogs, N3 native pickers, N5 Settings/Diagnostics and Storage/working-folder picker routes | Revision 26 adoption; [full consumer evidence](../validation/2026-11-11-issue-107-dialogs-windows.md). Shared dismissal/focus lifetime, retained native modes/filters and operation guards, single transform and comfortable typography. Host chrome, waveform and drag interactions retain their owners. |

The final coverage review reconciles these rows against the delivered P1–P8
children and the Recovery selector follow-up. Visible consumers are accounted
for by the shared families or their documented specialized owners. In
particular, native pickers and host chrome retain platform ownership, while
waveform, file drag and virtualized list interactions retain their specialized
policies. The hidden diagnostic compatibility controls in D3 and D6 retain
their existing bindings and stay outside the visible control surface.

### Common native and compatibility gate for every packet

Review affected consumers at 900x600/100%, representative 1000x700, 1100x850 and
1400x1000; exercise supported 100%/125% user magnification where the work area
fits. Cover empty/dense, long text, disabled/unavailable, focused and error states
where applicable. Include both main and comfortable Settings/Diagnostics consumers
of the family. A material family revision must have its prepared native preview
opened directly for owner look/feel review before wider adoption; retain the
single review window and prior unchanged scoped approvals.

Record actual Windows 100/125/150/200% DPI, monitor/host work-area fit and FL Studio
as separate pass/fail/unsupported/untested results. Software render scale is not
OS-DPI proof. Run `scripts/build.ps1 -Configuration Release -Test` for child native
changes and `scripts/check-ui-standards.ps1`; extend focused existing tests as
listed below, never raw concurrent CMake/MSBuild or baseline regeneration. Commands,
guards and model/audio behavior remain invariant: changing their semantics needs
separate owner scope, not a styling exception. Consolidate any concrete unresolved
host check into one brief final smoke rather than repeating automated edge cases.

### Packet verification matrix

All packet tests below are **required future checks**, not results from this audit.
The common gate above applies to every row.

| Packet | Automated/component and binding proof | Native preview starting point / approval focus |
| --- | --- | --- |
| P1 | Extend [ControlLibraryCases](../../Tests/ControlLibraryCases.h), [ExportControlCases](../../Tests/ExportControlCases.h), [PerformAuditionStripCases](../../Tests/PerformAuditionStripCases.h): I/H/P/F/D plus selected/latch combinations, name/relabel role invariance, one pointer/Return/Space/accessibility command, disabled/queued lifetime and captured invocation guards, silent display | `--review-controls`, `--review-export-controls`, integrated `--review-workspace`; compare text/glyph/primary/destructive/latch surfaces, preserve drag-tile variant |
| P2 | [UiStandardsCases](../../Tests/UiStandardsCases.h), ControlLibrary, [PerformKnobResetCases](../../Tests/PerformKnobResetCases.h), #109 IR tests: no-wheel, key/popup/cancel/focus, accessible unavailable reason, reorder/stale/selection/deletion/pass rejection, silent selection | `--review-controls`, `--review-workspace`; #109 `--review-ir-editor`, long selected values, menu anchors and focus return |
| P3 | UiStandards, [WorkspaceNoticeCases](../../Tests/WorkspaceNoticeCases.h), [WorkspaceFeedbackCases](../../Tests/WorkspaceFeedbackCases.h), [WorkflowHeaderCases](../../Tests/WorkflowHeaderCases.h): actual font/overflow, description/association, cleared/unchanged feedback, error precedence, no forced speech | `--review-messages`, `--review-workspace`; reuse #109 grouped feedback, inspect both local errors and footer help without waveform overlays |
| P4 | [PerformKnobCases](../../tests/PerformKnobCases.h), [ExportLevelsCases](../../tests/ExportLevelsCases.h), the native numeric and text fixture and real bar-range/Rename bindings cover precision, feedback, ownership, wheel steps and read-only values. | `preverb_selector_popup_test --review-numeric-text-fields` covers exact typing, wrapped feedback, unavailable states and compact steppers. |
| P5 | PerformKnob/Reset, ControlLibrary, ExportLevels, [PerformNavigationStateCases](../../Tests/PerformNavigationStateCases.h): coarse/fine wheel/drag/key/reset, anchor rebase/rejection, display/value agreement, range/units, no unintended notification or audio-gain effect | `--review-workspace`, `--review-export-levels`; preserve approved corner focus and Level steps; compare global/selected same-family feel |
| P6 | UiStandards, ControlLibrary, [LearnPassPolicyCases](../../Tests/LearnPassPolicyCases.h) and real editor binding cases: pointer/key/accessibility accepted toggles, D/F/S combinations, stale pass, silent refresh and no mutation through disabled paths | `--review-learn`, `--review-workspace`; explicit checkbox/track review, caption association and focus containment |
| P7 | ControlLibrary, [SwellListBindingCases](../../Tests/SwellListBindingCases.h), PerformNavigationState, [ResponsiveEditorCases](../../Tests/ResponsiveEditorCases.h), [ResponsiveScaleCases](../../Tests/ResponsiveScaleCases.h): adjacent hover/selected borders, unavailable inspection, accessible row actions, identity/scroll/reveal and focus reachability | `--review-controls`, `--review-workspace`, `--review-responsive`; keyboard through hidden/scrolling pages, selected/focused distinction, long names |
| P8 | [LearnTimelineCases](../../Tests/LearnTimelineCases.h), PerformNavigationState, ExportControl, ResponsiveEditor/Scale and affected timeline/guard tests: hit/drag/key geometry, frozen/stale gesture rejection, file drag/cancel/stale save, transform and auxiliary focus reveal, retained data invariance | `--review-learn`, `--review-workspace`, `--review-responsive`; domain interaction and native/auxiliary boundaries, actual host/DPI limits separately |

Preview switches must be confirmed in the integrated test harness before launch;
some belong to an upstream family branch. Missing family-state coverage is added
in that child rather than claimed from an existing generic preview. No automated
test, native approval or host pass is inferred from the table.

At the audit baseline, `--review-controls`, `--review-messages`,
`--review-export-controls`, `--review-export-levels`, `--review-learn-empty`,
`--review-learn` and `--review-workspace` are on `preverb_swell_ui_test`.
`--review-responsive` is on **`preverb_perform_composition_preview`**, not that
executable. Its `--review-integrated`, `--review-audition-strip` and
`--review-selection` cover the actual production editor composition. Merged #109
adds `--review-ir-editor` on `preverb_swell_ui_test`. Source authorities:
[swell UI harness](../../Tests/swell_ui_test.cpp),
[composition harness](../../Tests/perform_composition_preview.cpp) and
[build targets](../../CMakeLists.txt). These are reusable launch routes, not
new previews required for the documentation-only umbrella.

## Reconciliation with issue #80

[PR #109](https://github.com/elneilios/ReverbArise/pull/109) is **merged**, at
`717c890c4121cdfc84e20c988dedf7a6da0bc811`; this branch is fast-forwarded to that
baseline. Its [icon source](../../Source/UI/Controls/IconActionButton.h) calls
`ActionButton::paintSurface`, retaining neutral glyph ink on hover. Its shared
`ConvolutionIrEditor`/binding replaces standalone Learn/Perform IR setup, groups
caption/input/import/actionable red feedback and collapses empty feedback; hosts
keep context explanations. The [validation record](../validation/2026-11-06-issue-80-shared-ir.md)
records scoped owner approval, 75/75 Release CTest and FL Studio smoke. Those are
upstream results, not tests performed by this documentation audit.

P1 reuses the delivered shared surface; P3/P4 reuse the approved IR feedback
composition. Rows H1/L2/L3/L7/P1/P3/P8/N3 reflect merged ownership. The legacy
`loadIR` theme painter is orphaned; the actual IR file action is already shared.
The initial audit identified remaining name-based theme consumers. P1 replaced
those command consumers with typed roles, and the current fallback painter
uses button state independently of names and labels.

The first child is proposed as
[`unify-command-button-states`](../../openspec/changes/archive/2026-11-06-unify-command-button-states/proposal.md).
Its [design](../../openspec/changes/archive/2026-11-06-unify-command-button-states/design.md) fixes the
20 remaining persistent legacy command consumers and two latch consumers, reuses
merged #80 and retains scoped native approval before migration. It does not start
P2â€“P8 or modify active #108/#8 work.
