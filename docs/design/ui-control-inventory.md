# UI control inventory and family roadmap — issue #107

Initial audit: `24cd287`; reconciled baseline: merged #80 at `717c890`,
6 November 2026. Status: **owner-approved audit and contracts**, integrated as
production standards revision 11 after #80's revisions 8–10. This document records
source inspection. The separate command-button child now records native family
revision-4 approval and production adoption at standards revision 12; its
[evidence](../validation/2026-11-06-issue-107-buttons.md) separates integrated
acceptance and automated results. Other production migrations remain
separate child changes; #107 remains open. Independent #108 Level and #8 recovery/engine
work are outside this delivery and the remaining-button proposal.

The selector child `unify-selector-states` adds the native-approved C2 family and
named consumer adoption at standards revision 14 (7 November 2026). Its
[evidence](../validation/2026-11-07-issue-107-selectors.md) records local field
feedback approval, binding checks and remaining host/DPI coverage. Action menus
remain the separate `unify-action-menu-states` child; Recovery candidateBox stays
with #8.

## How to read this inventory

Each row identifies the actual member(s), source/owning policy, surface, semantic
role, applicable state/interaction differences, approval reference and disposition.
Member lists are explicit; grouped rows do not imply screen-wide compliance.
Nested and dynamically created controls are included under their owning family.
The [contracts](../development/ui-control-contracts.md) define family IDs C1–C8;
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
This is not an immediate migration of the remaining C2–C8 renderers.

## Header and shared workspace feedback

Sources: [WorkflowHeader](../../Source/UI/WorkflowHeader.h),
[editor declarations](../../Source/Plugin/PluginEditor.h),
[editor composition/bindings](../../Source/Plugin/PluginEditor.cpp).
Approval baseline: production guide revisions 1/7 and issue #35/#58 evidence;
this audit adds no acceptance.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| H1 `workflowHeader.settings` | C1 IconActionButton, command | I/H/P/F/D; merged #80 shares ActionButton surface with stable glyph ink; explicit keyboard focus and Return/Space | Retained approved revision 9; P1 regression reference, no duplicate migration |
| H2 `workflowHeader.selector` and dynamically rebuilt `Segment` children | C7 SegmentedSelector, workflow navigation | I/H/P/F/D/S; arrows/Home/End inspect, Return/Space request; unavailable choice can be inspected; accepted selection separate from focus; shared edges painted by parent | P7, retain #35 policy; audit combined D/F states |

## Learn

Source: [LearnWorkspace](../../Source/UI/LearnWorkspace.h). Approval: Learn #11,
shared controls #16/#37, responsive #58 and merged #80 shared IR field.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| L1 `generate` | C1 ActionButton primary command | I/H/P/F/D/B; aqua primary fill with hover fill; beforeInvocation pins Generate provenance, accepted readiness disables | P1 retained reference, preserve generation serial/session/pass |
| L2 `irEditor.fileButton` | C1 IconActionButton import command inside ConvolutionIrEditor | I/H/P/F/D/B; shared command surface/stable glyph ink; async chooser is binding-owned | Retained #80/revision 9; P1 reference, no duplicate migration |
| L3 `irEditor.selector`, `irEditor`, `irBinding` | C2 IrSelector/ChoiceSelector and shared field/binding | Quiet independent hover/keyboard/selected states, outer aqua focus, small local feedback/red error border; no wheel reassignment; captured default catalogue/session/pass retained | Migrated in `unify-selector-states`; native family approved 2026-11-07; shared IR component/binding checks pass |
| L4 `recapture`, `automatic`, `autoStop` | C6 Switch/InvocationToggle | I/H/P/F/D/S; track/thumb, .4 disabled alpha; explicit outside track focus; unlike legacy checkbox geometry; Space activation inherited | P6; retain approved Learn shape and pass guards |
| L5 `sections` and generated segments | C7 SegmentedSelector, Capture & Generate/Sound navigation | I/H/P/F/D/S; same accepted-selection mechanism as H2, different responsive text role | P7; reuse H2 policy, #58 scope |

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| L6 `sensitivity`, embedded NumericInputControl, increment/decrement StepButton and temporary label TextEditor | C5/C4 SliderControl + InvocationSlider | I/H/P/F/D/E; repeatable step buttons, numeric entry excludes step strip; range/interval shared with slider; local slider Theme owns text-box creation | P4/P5, retain #37 path; review responsive font inheritance separately |
| L7 `captureTitle`, `captureGroup`, `recaptureLabel`, `recaptureHelp`, `automaticLabel`, `automaticToggleLabel`, `automaticHelp`, `sensitivityLabel`, `autoStopLabel`, `autoStopHelp`, `manualHelp`, `generateReason`, `soundScope`; nested `irEditor.label`, `irEditor.status` | C3 screen labels/roles plus shared IR EllipsisLabel/FieldMessage | I/D/B/overflow; IR feedback uses normal helper font, red actionable failure, readable disabled-input feedback and collapsed empty space; scope remains screen-owned | Retain #80 revisions 8/10; P3 reference for other fields |
| L8 painted `timelineHeading`, `timelineCaption`, `timelineRuler`, `timelineStatus`, empty prompt and interactive `chart` | C3/C8 LearnWorkspace specialized canvas | I/H/F/S/B; pin/inspect guides, Home/End/arrows/Escape; recorded/silence/gap/unavailable distinct; no generic input field | P8 retained #11/#58, review specialized accessibility rather than replacing canvas |

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
| P4 `enabledButton`, `soloButton`, `freezeButton` | C6 legacy ToggleButton | I/H/D/S; theme checkbox square/tick, no explicit focus painter and ignores pressed parameter; unlike Learn switches | P6, selected-swell guarded bindings retained |
| P5 `inspectorSections` and generated segments | C7 SegmentedSelector, Timing/Envelope/Sound | I/H/P/F/D/S; shares navigation mechanism, composition owns section choice and visible content | P7; retain #58 navigation |
| P6 `performZoom.minus`, `.plus`, `.fit`, `.slider` | C1/C5 TimelineZoomControls, presentation intents | I/H/P/F/D; ActionButton surface and parent-painted zoom-slider focus; no value text box; limit-specific disabled children | P1/P5 by nested family, C7 owns composition; retain #49 scope |

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| P7 `valueControls[0..10]`: Target, Source Start, Source End, Pre-roll, End Offset, Fade In, Fade Out, Level, Curve In, Curve Out, Move Wet | C5/C4 KnobValue, nested Dial/Caption/Entry | I/H/F/D/E/V; exact typed entry, gesture-aware refresh and accepted values; dial corner focus differs intentionally from command outlines; Return/focus-loss commit, Escape cancel | P4/P5, retain #46 reset/focus/precision and inspector identity guards |
| P8 `inspectorHeading`, `listEmpty`, `inspectorEmpty`, `auditionHeading`, `editorReadout`, `listCount`, `listLabel`, `inspectorGroups[0..2]`; `selectedStatus`, `monitorStatus`, `performSoundScope`; nested `irEditor.label`, `irEditor.status` | C3 roles/plain Labels, EllipsisLabel and IR FieldMessage | I/D/B/overflow; screen scope outside shared IR field; feedback collapse reflows Level while Reset stays fixed; remaining statuses ellipsize | P3 retain #80 revisions 8/10; do not duplicate field feedback |
| P9 `list`, generated `RowSurface`, inherited ListBox viewport/scrollbars | C7 SwellList specialized identity list | I/H/F/S/D/B; row Enabled and menu are painted hit regions, not Button children; RowSurface explicitly inaccessible, parent ListBox owns selection; refresh preserves identity/scroll; explicit list navigation reveals context | P7 retain #4/#93; verify accessible row actions before any broader compliance claim |
| P10 `inspectorViewport`, `inspectorContent` | C7 FocusViewport + screen layout | scroll/focus/overflow; reveal policy explicit, not implied by viewport type | P7 retain approved composition; check focus reveal enabled at each use |

## Export

Sources: [ExportWorkspace](../../Source/UI/ExportWorkspace.h),
[ExportLevelsControl](../../Source/UI/Controls/ExportLevelsControl.h),
[FileDragTile](../../Source/UI/Controls/FileDragTile.h). Approval: #6/#51/#58.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| E1 `play`, `stop`, `save`, `back` | C1 ActionButton; primary play, neutral commands | I/H/P/F/D/B; icon-only play/stop versus text+icon Save/Return; state/callback eligibility remains export-owned | P1 retained reference; no export command redesign |
| E2 `tile` | C1/C8 FileDragTile, native-file gesture variant | I/H/P/F/D/B; dashed aqua boundary becomes solid on focus, H/P changes border ink; click/Return saves, drag threshold suppresses subsequent click | P8 explicit approved #6 exception, do not flatten to ordinary button |
| E3 `mix.selector` | C2 ChoiceSelector, Character preset | Independent H/P/F/S/D; no wheel, exact canonical pair defines inspectable unavailable Custom; preset emits paired edit | Migrated in `unify-selector-states`; #51 atomic pair validation and silent refresh retained |
| E4 `zoom.minus`, `.plus`, `.fit`, `.slider` | C1/C5 TimelineZoomControls | I/H/P/F/D; same family as P6, export composition/session/pass guards differ | P1/P5, use same family tests and separate export binding cases |

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| E5 `mix.dry`, `mix.wet`, each embedded NumericInputControl/StepButtons/temporary editor; `characterLabel`, `dryLabel`, `wetLabel` | C5/C4/C3 ExportLevelsControl composition | I/H/P/F/D/E; exact pair accepted/Custom derivation, untouched level preserved on single fader edit; labels plain role-based | P4/P5, retain #51 isolated pair binding; no new mixer or preset defaults |
| E6 `navigation`; painted heading/caption/mixHeading/ruler/chart/legend/timeArea/statusArea | C8 TimelineNavigation and ExportWorkspace paint | I/H/F/D/B; left positions preview, right drag pans, wheel zoom/Shift-wheel pan, Home Fit; previous waveform/status distinct from accepted output | P8 retain #6/#51; no export asset or cache changes |

## Settings

Source: [PluginSettings](../../Source/Plugin/PluginSettings.cpp) moves existing
editor members into pages; these are not duplicate controls in Diagnostics.
Playback, Processing, Swell, Audition, Recovery, About plus conditional Appearance
are covered. About has #59 acceptance; Appearance/auxiliary limitations remain
as recorded in #58 evidence. Legacy other-page controls remain explicit debt.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| S1 `gainPolicyBox`, `liveRoutingBox` (Playback); `inCurveBox`, `outCurveBox` (Swell); `candidateBox` (Recovery) | C2 ChoiceSelector for four Playback/Swell choices; Recovery retains plain ComboBox/theme | Four migrated selectors use deliberate no-wheel inspection/commit and captured session/pass/selected-swell guards; gain prototype remains inspectable/unavailable | `unify-selector-states` adopts only the four named choices; Recovery candidateBox remains #8/P2 debt |
| S2 `defaultIRButton` (Processing); `fullButton`, `setBarsButton`, `removeButton` (Swell); `generateButton`, `allButton`, `analyzeButton`, `confirmButton`, `recaptureButton`, `cancelButton`, `clearButton`, `suppressionButton`, `openDiagnosticsButton` (Recovery) | C1 neutral ActionButton | I/H/P/F/D/B; shared mouse/focus states and eligible keyboard/accessibility path; Delete/Clear retain neutral classification | P1 implemented; callbacks and guards retained; default import retains shared binding; native integration approved |
| S3 `copyBuildInformation` (About) | C1 primary ActionButton | I/H/P/F/D; intentionally primary copy action already explicit, success feedback accessible event | P1 retain #59 scope |
| S4 `liveButton` (Playback), `mappingButton` (Processing), `muteButton` (Swell), `isolateButton`, `isolateDryButton` (Audition), `toneButton` (Recovery) | C6 plain ToggleButton/theme | I/H/D/S; checkbox presentation; model meanings differ from action latches | P6 legacy, no command or tone policy changes |
| S5 `automaticButton`, `autoButton` (Processing) | C6 InvocationToggle/theme | I/H/D/S; checkbox drawing plus captured pass; pre-invocation runs for keys before enabled check | P6 preserve tokens, test disabled hooks separately from mutation |
| S6 `presentationSizeBox`, `presentationScaleBox` (Appearance) | C2 ChoiceSelector | Independent H/P/F/S/D; no-wheel; explicit choice updates presentation profile, not project state; unavailable fit choices remain inspectable | Migrated in `unify-selector-states`; retain #58 composition/profile behavior |
| S7 `settingsTabs`, generated tab buttons, `settingsPages[0..6]` | C7 JUCE TabbedComponent + screen setup | I/H/P/F/D/S for tabs; fixed native tab mechanism unlike segmented main navigation | P7 legacy ownership; pages themselves are layout-only |

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| S8 `liveLevel` (Playback); `separationSlider` (Processing); `tailSlider`, `auditionSlider` (Audition); `toneStartSlider`, `toneEndSlider` (Recovery), with JUCE text-box labels/editors | C5/C4 plain Slider except invocation-aware separation | I/H/P/F/D/E; screen lambda configures style/range/text-box geometry; no shared stepper; wheel/fine policy not library-owned; audition double-click resets trim | P4/P5 explicit legacy, preserve units/ranges and beforeInvocation where present |
| S9 `barStartEditor`, `barEndEditor` and `barStartLabel`, `barEndLabel` | C4/C3 plain TextEditor and Label | I/F/D/E/V; digit restriction, Return calls applyBarRange; no explicit local Escape/focus-loss commit handlers or local error field | P4; preserve explicit button/Return commit, do not silently add focus-loss commit |
| S10 `settingsLabels[0..11]`, `settingsHelp[0..5]`, `gainPolicyLabel`, `liveLevelLabel`, `separationLabel`, `tailLabel`, `auditionLabel`, `toneStartLabel`, `toneEndLabel`, `recaptureStatus`; `regenerationFeedback` | C3 legacy label/help layout; feedback EllipsisLabel | I/D/B/overflow; most help built with raw body/secondary setup, About helper uses semantic role; regeneration feedback retains session; no universal association | P3 preserve copy and accepted feedback; link exact groups |
| S11 `aboutHeading`, `buildInformationLabels[0..2]`, `buildInformationValues[0..2]` | C3 roles + EllipsisLabel | I/overflow; full build value in description/tooltip; copy feedback uses standard accessible event | P3 retain #59 unchanged |
| S12 `presentationSizeLabel`, `presentationScaleLabel`, `presentationHelp` | C3 Appearance composition | I/B/overflow; fit recovery help distinct from requested profile and effective size | P3 retain #58 profile behavior |

## Diagnostics

Sources: editor composition and PluginSettings above. Members moved into Settings
are listed only under S rows. Explicitly retained hidden diagnostic seams are not
treated as production primary controls.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| D1 `timingButton`, `assetInfoButton` | C1 neutral ActionButton; timing disclosure/copy assets | I/H/P/F/D; shared semantic painter and activation; existing clipboard/disclosure callbacks | P1 implemented; family approved, native integration approved |
| D2 `copyDiagnosticsButton` | C1 ActionButton | I/H/P/F/D; shared neutral surface, explicit title/help | P1 retained reference |
| D3 `sampleUnitsButton`, `markerButton`, `recaptureExisting` | C6 retained checkbox/invocation seams | I/H/D/S if exposed; exact sample seam and old header controls hidden/zero bounds in current layout | P6 explicit legacy/hidden, no silent deletion or certification |

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| D4 `status`, `detail`, `captureDiagnostic`, `playbackDiagnostic`, `policyDiagnostic`, `liveDiagnostic` | C3 diagnostic text/labels | I/B/overflow; long technical content and throttled refresh; copyDiagnostics exposes full text | P3 retain diagnostic precision; not new product status controls |
| D5 `fields[name..fadeOut]`, `fieldLabels[name..fadeOut]`, `placementEditor`, `placementLabel` | C4/C3 retained exact/read-only diagnostic seams | I/F/D/overflow; Name hidden, numeric fields read-only; advanced timing exposes only existing seams; do not apply editable-field contract blindly | P4 read-only subtype; no removal or new edit path |
| D6 `markerKnob`, `regenerationKnob`, `markerLabel`, `markerState`, `regenerationLabel`, `headerStatus`, `spanLabel`, `helper` | C5/C3 retained legacy controls/labels | I/H/F/D/E for exposed sliders; old main sensitivity/header controls zero-sized or hidden in normal Perform; not same instance as Learn sensitivity | P3/P5 explicit legacy/hidden; preserve diagnostics and future-default semantics |
| D7 `diagnosticsViewport`, `diagnosticsContent` | C7 FocusViewport/FlatSurface | scroll/focus/overflow; diagnostic panel inside separately bounded auxiliary window | P7/P8; no independent duplicate model |

## Specialized interactions and native boundaries

| ID / actual consumers | Owner / semantics | States / differences | Disposition |
| --- | --- | --- | --- |
| N1 `showSwellMenu` popup Rename/Delete; dynamic rename AlertWindow editor `name`, OK and Cancel buttons | C2/C4/C1 screen-created JUCE menu/dialog | popup H/F/S/D and dialog E/F; Return OK, Escape Cancel; nonempty rename only; epoch/ID captured through async completion | P2/P4, retain identity guards and native dialog behavior pending scoped review |
| N2 `KnobValue` reset popup per value cell | C2/C5 specialized reset actions | I/H/F/D; action availability and target snapshot differ per parameter; stale reset closure guarded; inspector `resetMenu` is actually a direct Reset All command, not a popup | P2/P5, retain reset semantics, no generic destructive assumption |
| N3 Learn/selected ConvolutionIrBinding `chooser` (Settings default import routes through Learn binding), Export save `chooser`, native external file drag and editor host resize chrome | C8 platform surfaces; application owns launch/completion | OS-managed focus/selection/cancel; async lifetime/epoch/pass/file identity stays binding-owned | Retain native; P8 invocation ownership only, no themed chooser replacement |

| ID / actual consumers | Owner / semantics | States / differences | Disposition |
| --- | --- | --- | --- |
| N4 `timeline`, source/wet handles, ruler, internal overview, `scrollbar`, `waveZoom`, `waveZoomLabel`; external `songOverview` and `.heading` | C8 SwellTimeline/SwellOverview, C5 display-only height slider | I/H/F/S/D/drag/overflow; sample/grid/gesture identity and frozen rules; overview drag navigates; height reset changes display only, never gain | P8/P5, preserve coordinate/hit and Snap policies; no generic slider rewrite |
| N5 `settingsWindow`, `diagnosticsWindow` WorkspaceWindow; nested `surface`, `viewport`, `comfortableTheme`; editor `presentationSurface` and host resize affordance | C8/C7 PresentationSurface, FocusViewport, native DocumentWindow chrome | logical size versus single magnification transform; comfortable auxiliary typography; bounded work-area scroll and focus reveal; native close behavior | P8 retain #58, actual host/DPI coverage separate |
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

### Merged-main addendum

Main `f98157e` subsequently delivered the independently owned #8 recovery/storage
work. It is preserved when integrating this button child; the original 20-command
migration boundary does not expand. The newly present shared commands are recorded
here without claiming a new storage/recovery audit or repeating that delivery's approval.

| ID / actual members | Owner / semantics | States and concrete differences | Disposition |
| --- | --- | --- | --- |
| X1 `PortableStoragePanel.backupButton`, `restoreButton`, `locateButton`, `cancelButton`; `StorageInventoryPanel.cancelButton` | C1 existing neutral ActionButton; #8 owns chooser, worker, reload and scan bindings | I/H/P/F/D/B; existing idle/busy/chooser/reload guards and conditional stop actions; inherits the reviewed common command painter | Retained main declarations/bindings, no migration in this child. Native storage regressions remain part of the combined suite; recovery semantics retain their separate #8 evidence |

### Delivery packets

The following packets are candidates for separate child OpenSpec changes. No
packet is authorization to change production code or create a GitHub issue.
An explicit member group may be subdivided into named consumer lists when it
would otherwise create an oversized review, without fragmenting contract ownership.

| Packet / proposed order | Exact scope and reusable ownership | Dependencies, guards and disposition |
| --- | --- | --- |
| P1 Commands, glyph actions and latches | H1, L1/L2, P1/P2/P6 command children, E1/E4 command children, S2/S3, D1/D2; C1 shared command family | Implemented by `unify-command-button-states`; native family revision 4 approved. Explicit roles replace all 20 persistent legacy TextButtons; generic Rename fallback remains neutral. Callbacks, captured ownership and effective latch state retained. Native integration and the IR alignment correction are approved; PR #114 remains open for owner review. |
| P2 Selectors and menus | L3, P3, E3, four S1 selectors and S6 adopted by `unify-selector-states`; N1 popup and N2 reset popup planned separately in `unify-action-menu-states` | Reuses #109 IR field/binding; selectors preserve catalogue/ID/epoch/input/pass and approved deliberate no-wheel policy. Action-menu implementation waits for the selector adapter delivery, retaining reset closures and anchoring. Recovery candidateBox stays with #8; rename dialog input stays P4. |
| P3 Text, context help and feedback | H3, L7/L8 text, P8, E5 labels/E6 painted text, S9 labels/S10/S11/S12, D4/D5 labels/D6 labels, N4 labels; C3 semantic roles/EllipsisLabel/NoticeBar/HeaderInformation/readout | First shared caption/help/validation composition; reuse scoped #109 feedback and screen-owned context prose. Preserve notice precedence/no speech, transport fixed geometry and full diagnostic copy. No new color or shrinking exception. Contract-level accessibility associations need concrete child component tests. |
| P4 Numeric and text fields | L6 numeric subtree, P7 Entry, E5 numeric subtrees, S8 text-box subtrees/S9 input, D5 read-only input, N1 rename input/OK/Cancel; C4 existing numeric/editor lifecycles | Depends on accepted C3 feedback contract/P3 reusable presentation when needed. Preserve per-subtype Return/Escape/focus-loss, parsing/clamping, exact precision, session/selection guards and explicit bar-range pair commit. Read-only/hidden seams remain explicit; no forced deletion. No universal new editor. |
| P5 Knobs and sliders | L6 fader, P6/E4 zoom slider, P7 Dial, E5 Dry/Wet faders, S8 sliders, D6 knobs, N4 display height; C5 KnobValue/SliderControl/InvocationSlider | Coordinate P4 nested fields; own gesture/reset/fine/wheel defaults once. Preserve #46 corner focus/Level steps, grid/anchor rebase, display-only versus audio values and #51 pair edits. Audit local slider Theme responsive metrics before declaring equivalence. |
| P6 Switches and toggles | L4, P4, S4/S5, D3; C6 Switch/InvocationToggle/legacy checkbox | Boolean settings remain distinct from P1 latches. Preserve accepted state/pass hooks, freeze/mute/solo/capture/tone meanings. Candidate checkbox-versus-track/focus/disabled policy must receive scoped owner approval; no implied global alpha change. |
| P7 Navigation, lists and scrolling | H2/L5/P5 segments, P9/P10, S7, D7 viewport, N5 scroll viewport; C7 SegmentedSelector/SwellList/FocusViewport/native tabs | Preserve accepted versus inspected option, row identity/scroll/multiselection and #93 contextual reveal. Review row keyboard/accessibility gaps and explicit focus-reveal use. Named native-tab difference allowed with evidence; no whole-workspace layout redesign. |
| P8 Specialized interactions and auxiliary shells | L8 canvas, E2/E6 navigation/canvas, N3/N4/N5; C8 existing timeline/overview/navigation/file/window mechanisms | No generic replacement of domain geometry or asset lifecycle. Preserve timing/Snap/frozen/selection and source/render/session identities, native chooser cancellation and file drag/save guards, single transform and comfortable auxiliary type. C1/C5 nested controls remain owned there. A source audit gap alone does not authorize DSP or persistence changes. |

Each packet requires its own proposal/design/tasks and a consumer-specific diff,
tests, review evidence and documentation update. The exact child change names are
chosen when proposed. No child has been created or implemented here. Every row
has a packet or explicit retained-native disposition; retained shared references
require equivalence evidence in the child, not compulsory replacement.

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
| P4 | ControlLibrary, [PerformKnobCases](../../Tests/PerformKnobCases.h), PerformKnobReset, [ExportLevelsCases](../../Tests/ExportLevelsCases.h), real bar-range/rename bindings: subtype lifecycle, exact/range/invalid draft, stale/active refresh, silent display, keyboard/accessibility steps and read-only behavior | `--review-workspace`, `--review-export-levels`, field-family extension when needed; exact typing, long help/error, disabled input with readable reason |
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
Remaining name-based theme consumers and other legacy families stay explicit debt.
No production code is changed by this audit.

The first child is proposed as
[`unify-command-button-states`](../../openspec/changes/archive/2026-11-06-unify-command-button-states/proposal.md).
Its [design](../../openspec/changes/archive/2026-11-06-unify-command-button-states/design.md) fixes the
20 remaining persistent legacy command consumers and two latch consumers, reuses
merged #80 and retains scoped native approval before migration. It does not start
P2–P8 or modify active #108/#8 work.
