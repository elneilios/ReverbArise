# swell-workspace Specification

## Purpose
Provide a usable single-screen workspace for inspecting captured audio and editing individual swells without exposing an engineering workflow by default.

## Requirements

### Requirement: Workspace reflects actual processing and selection
The foundation workspace SHALL retain the existing eight-region Live composition while adding mode selection/status without a dedicated Capture/Finalise redesign. Capture/Finalise SHALL show mode-appropriate read-only content and lock unavailable creative actions. Selection/processing SHALL remain identity-bound and SHALL NOT imply a requested render or mode is already ready.

#### Scenario: Select while background work completes
- **WHEN** the user selects a swell while allowed analysis or another swell's rendering completes
- **THEN** its row, handles and inspector remain bound to that identity and show actual IR/state, with editing available only under mode guards

#### Scenario: Empty or unavailable selection
- **WHEN** the session is empty or a result is waiting, stale or failed
- **THEN** the workspace explains its state and enables only valid mode/selection actions without false ready waveforms

#### Scenario: Stable empty and loaded composition
- **WHEN** the same editor size transitions through empty, mapped, selected and updating states in this foundation
- **THEN** existing region bounds/dividers remain coherent, no developer toolbar displaces them and mode/readiness text does not promise the deferred workspace/export implementation

### Requirement: Timeline navigation preserves exact edits
The editor SHALL align dry, wet, arrivals and handles on one absolute Song scale with bar references, zoom, pan and snap. The overview SHALL distinguish audio, captured silence, unmapped and unavailable coverage and support viewport navigation. Navigation SHALL not edit render inputs. Display-only peak scaling SHALL remain disclosed.

#### Scenario: Navigate distant sections
- **WHEN** the user clicks or drags the overview between distant captured sections
- **THEN** detail view and ruler follow the viewport without moving swells or changing source dependencies, and the gap remains visibly unmapped

#### Scenario: Loud wet waveform
- **WHEN** a selected raw render exceeds full scale
- **THEN** its envelope remains visible without flat clipping, its display scale is available, and its audio gain is unchanged by drawing

### Requirement: Inspector exposes supported musical editing
The inspector SHALL show selected name/enabled state, Freeze, Solo, own IR dropdown/file loader and knob/exact entries for Pre-roll, signed End Offset, Fade In, Fade Out, Level, Curve In and Curve Out. Every waveform edit SHALL have a corresponding inspector knob/exact entry, including source bounds, target and whole-wet placement. Settings SHALL retain presets, mute and reset. Units SHALL reflect validated domains; no duplicate envelope graph or unsupported processing control SHALL appear.

#### Scenario: Type and drag a precise edit
- **WHEN** a user drags a fade or bound and then types an exact value
- **THEN** the handle, field and canonical edit agree without unintended whole-bar rounding or unnecessary convolution

#### Scenario: Invalid timing or crop
- **WHEN** timing is unavailable or an edit creates an invalid crop
- **THEN** the inspector retains the prior valid value or shows the explicit invalid edit state, without silently changing unrelated swells

#### Scenario: Relative timing fixture
- **GIVEN** compatible timing at 48000 Hz and 120 BPM, with target sample 96000 and sufficient real wet support
- **WHEN** Pre-roll is 1.00 s and End Offset is +0.35 s
- **THEN** audible bounds are [48000,112800), duration is 64800 samples or 1.35 s, and the drawing spans two beats before target and 0.7 beat after it
- **AND** nonzero wet placement is included in displayed bounds without moving the dry target or source

#### Scenario: Independent curve controls
- **WHEN** Curve In or Curve Out is changed by knob, typed value or corresponding canvas tension handle
- **THEN** only that existing fade tension changes in its -1 to +1 domain, its preset is preserved, and zero-duration fades do not display fictitious tension handles

#### Scenario: Per-swell IR and activity
- **WHEN** selection moves between swells with different IRs, activity flags or missing dependencies
- **THEN** every inspector value and IR choice follows the new ID, old values are cleared when unselected, and frozen or unavailable actions are honestly gated

### Requirement: Envelope handles follow the visible curve
Outlined circular endpoint, fade and tension handles SHALL share painting/hit geometry with generous hit areas and hover/focus feedback. Start/End SHALL retain centre-line anchors; fade/tension controls SHALL follow the curve. Only genuine time-and-amplitude coincidences SHALL get small labelled connected targets. Fine edits, numeric/keyboard alternatives and immediate accepted feedback SHALL remain available.

#### Scenario: Grab and drag a fade
- **WHEN** the user grabs a visible fade handle and drags between grid lines
- **THEN** the intended fade changes smoothly, the handle remains on its curve, values update coherently and unrelated timing or selection does not jump

#### Scenario: Co-located or zero-length controls
- **WHEN** an endpoint and fade boundary coincide or a fade has zero duration
- **THEN** hit selection is deterministic with clear feedback and both endpoint and fade-duration controls remain directly grabbable at their anchors or, only for genuine time-and-amplitude coincidence, through small labelled connected targets, and zero duration does not show a fictitious active tension curve

### Requirement: Fade tension is continuously editable
Each nonzero fade segment SHALL provide a continuous tension handle and precise value in addition to its curve preset. The displayed envelope and prepared wet audio SHALL use the same tension law, retain monotonic endpoints and preserve the existing preset at neutral tension. Tension changes SHALL not reconvolve source audio or modify dry audio.

#### Scenario: Bend a fade
- **WHEN** the user moves a fade's tension point vertically and then types a precise value
- **THEN** curve, numeric value and shaped wet agree continuously without changing fade endpoints or raw render identity

#### Scenario: Restore a previous project
- **WHEN** saved state has no tension settings
- **THEN** neutral tension preserves its existing curve presets and audible shape without resetting other edits

### Requirement: Wet endpoint controls gap and overlap independently of fade
The user SHALL be able to drag the wet endpoint before or beyond the fixed dry arrival. Its relative position SHALL determine the displayed gap or overlap. Wet fade-out duration and tension SHALL remain independently editable. Endpoint edits SHALL leave dry timing/level and other swells unchanged, retain requested bounds while support is prepared, and disclose any additional render-source context.

#### Scenario: Intentional gap
- **WHEN** the wet endpoint is placed 200 ms before dry arrival
- **THEN** wet ends at that endpoint, a 200 ms gap is shown, and dry remains at its original arrival with no transition-driven gain change

#### Scenario: Overlap with independent fade-out
- **WHEN** the wet endpoint is placed 300 ms beyond dry arrival and the wet fade-out is set to 100 ms
- **THEN** wet overlaps the first 300 ms of the source event and fades over its final 100 ms, with the dry event unshifted and unfaded by transition controls

#### Scenario: Extend beyond current support
- **WHEN** the requested endpoint exceeds the current wet render's support
- **THEN** the UI preserves that requested endpoint, highlights required render-source extension and shows preparation or missing coverage instead of silently clamping the handle to the old render

### Requirement: Engineering tools remain accessible outside the main workflow
Useful developer observations/actions SHALL remain accessible in a dedicated settings-opened Diagnostics window. Musical preferences and precise edits SHALL remain available in normal contextual UI. Automatic first use SHALL work with Diagnostics closed. Live Song playback and manual audition SHALL be clearly distinguished; no deferred-transport claim or unverified host-control action SHALL be shown.

#### Scenario: Closed diagnostics
- **WHEN** a user inserts the plugin, plays input, selects a prepared swell and edits it
- **THEN** capture, target creation, rendering and editing work without opening Diagnostics or pressing a generation button

#### Scenario: Recovery and deferred transport
- **WHEN** the user opens Diagnostics or asks how to hear prepared swells
- **THEN** manual recovery tools remain functional and audition is clearly distinguished from implemented Song-synchronized playback

#### Scenario: Diagnostics lifecycle during playback
- **WHEN** Diagnostics opens, refreshes, hides, closes or reopens while playback and capture continue
- **THEN** it shows current snapshots without changing parameters, starting voices/test signals, clearing capture or queuing renders, and it does not resize the main workspace
- **AND** hidden/closed diagnostics cease their own periodic work while their UI may remain editor-owned for reuse; reopening immediately refreshes current data, and editor destruction safely releases their content, callbacks and window ownership before LookAndFeel/model teardown

#### Scenario: Explicit developer action
- **WHEN** a user explicitly enables the retained scheduled tone or invokes a recovery action
- **THEN** scope, availability and active test state are visible with an explicit return to normal, while merely opening the window performs no action

#### Scenario: Deliberate cleanup
- **WHEN** the diagnostics revision is reviewed
- **THEN** each original item has a disposition with consumers and verification, duplicate sensitivity/status is consolidated, proven-unused bindings/helpers are removed, and tested tone/capture/scheduling capabilities and saved identities remain intact

### Requirement: Supported editor sizes remain usable
The eight-region workspace SHALL remain readable at a 1100x850 logical minimum and opening default, and larger supported sizes, including 1400x900, 1400x1000 and 1600x1000. List scrolling and contextual windows SHALL not hide the main workflow. Supported Windows scales SHALL retain usable text/hit targets without overlap; numerical and keyboard alternatives SHALL remain available.

#### Scenario: Resize and inspect many swells
- **WHEN** the editor is resized across supported sizes with hundreds of swells and Diagnostics open or closed
- **THEN** controls remain readable and reachable, the selected row is identifiable, and essential actions do not overlap

#### Scenario: DPI and constrained window
- **WHEN** minimum and default sizes are evaluated at 100%, 125%, 150% and 200% Windows scale
- **THEN** logical region geometry stays coherent, values/units fit and controls remain operable
- **AND** any in-host scale or screen-fit limitation is explicitly recorded rather than inferred from an offscreen raster

### Requirement: Zero fades remain directly editable on the canvas
Fade-duration handles SHALL remain grabbable at zero duration and independently from centre-line endpoints. Painting/hit geometry SHALL retain true anchors; only genuine time-and-amplitude coincidences SHALL get small labelled collision targets. Dragging SHALL use initial value and pointer delta without a displacement jump. Distinct times SHALL stack at low zoom and separate on zoom-in; exact entry remains available.

#### Scenario: Open an initial zero fade
- **WHEN** a user grabs an initial zero-length fade-in or fade-out control and drags it
- **THEN** only that duration opens smoothly, the audible bound and dry arrival stay fixed, and endpoint and fade controls remain distinguishable

#### Scenario: Zoom merges neighbouring handles
- **WHEN** zoom or a short fade makes hit areas overlap
- **THEN** controls stack in true-time paint order with topmost hit priority and separate naturally on zoom-in; only genuine coincidences use labelled connected targets, with consistent hover/active feedback

### Requirement: Whole-swell movement translates wet audio and envelope together
The selected wet body SHALL provide a move gesture and a precise placement alternative that translate wet PCM and its envelope by the same signed offset. Duration, fade durations, presets, tensions, gain and logical crop SHALL remain unchanged. Dry arrival, base source selection, detector anchor and unrelated swells SHALL stay fixed. Placement SHALL be saved, default zero for legacy state, and use checked arithmetic. Movement alone SHALL not reconvolve or rewrite raw assets.

#### Scenario: Move a shaped swell across the dry arrival
- **WHEN** the user drags the whole wet body earlier or later
- **THEN** waveform and every envelope point translate together with unchanged spacing, fades and tensions, while gap/overlap readouts update around the fixed dry event

#### Scenario: Precise move and invalid placement
- **WHEN** the user types or nudges placement, or requests a position that overflows representable bounds
- **THEN** valid movement preserves off-grid internal spacing and stable viewport/selection, while invalid movement is rejected transactionally without clamping individual points

#### Scenario: Reopen a moved swell
- **WHEN** a project with nonzero wet placement reopens
- **THEN** placement and shape survive, prior states use zero placement, raw assets remain reusable and audition does not start

### Requirement: Audition scope and monitoring safety are visible
The full-width audition band SHALL provide Play/Stop and one continuous Dry-to-Wet slider with exact percentage. Secondary audition options SHALL retain saved wet isolation, dry-source isolation, listening tail and visible/resettable nonzero trim. Scope/member availability, cursor and disclosed safety protection SHALL remain understandable without duplicate mode buttons or live-routing controls in that band.

#### Scenario: Choose overlapping or isolated listening
- **WHEN** the user toggles Isolate selected with overlapping swells in view
- **THEN** scope and member count reflect the prepared audible group, the continuous mix value and live routing remain independent, and no render is regenerated solely for that toggle

### Requirement: Marker controls are prominent and instance scoped
The main workspace SHALL expose one prominent rotary Marker sensitivity knob with a typeable 0-100% value and adjacent Auto markers On/Off switch, accessible with Diagnostics closed. Low SHALL mean more selective marking and the switch SHALL default on. Both SHALL bind to saved per-instance state independently of swell selection; any diagnostic sensitivity display SHALL share that value.

#### Scenario: Tune automatic markers from the main view
- **WHEN** a user turns or types the knob while listening to busy source
- **THEN** the value and effective marking/analyzing status update coherently, target-count feedback is available, and no selected-swell envelope, IR, blend or monitoring preference is reset

#### Scenario: Disable marking without losing the workflow
- **WHEN** Auto markers is off
- **THEN** the main view clearly shows Off, the stored sensitivity remains visible/editable, existing swells remain usable and capture/manual edits/render refresh are not presented as disabled by that switch

#### Scenario: An explicit master policy prevents marking
- **WHEN** Auto markers is on but automatic processing is explicitly disabled
- **THEN** the UI identifies marking as paused by that master policy and does not silently enable it or show an active marking state

#### Scenario: Resize and restore marker controls
- **WHEN** the editor is resized across 1100x850, 1400x900 and 1600x1000, Diagnostics is toggled, or a saved instance reopens
- **THEN** the knob, typeable value and switch stay readable/reachable, saved values remain synchronized and changing selected swells does not change these global instance preferences

### Requirement: Visual system is flat and consistent
The UI SHALL use one uniform #0F1418 background, the documented baseline aqua/text/separator tokens, neutral dry data and restrained yellow timing. Thin rules SHALL divide bands/columns; only the Overview waveform viewport SHALL have a major full rounded outline. Gradients, textures, glow, shadows, raised panels and speculative controls SHALL be absent.

#### Scenario: Inspect surfaces and borders
- **WHEN** empty, loaded and error states are captured from the native UI
- **THEN** unoccupied surfaces use the same flat background; list, editor, inspector and global controls have no large enclosing rectangles
- **AND** allowed control outlines and restrained row/viewport highlights do not become boxed panels or broad aqua fills

### Requirement: Brand and typography follow approved direction
The header SHALL display ReverbArise with light Reverb, aqua Arise and the approved waveform-shaped A integrated into the word, plus the italic tagline Reverse Reverb for Creative Swells. Typography SHALL use legally available consistent sans-serif roles and stroke icons. Unsupported undo/redo or presets SHALL be omitted; no separate conceptual app icon SHALL replace the approved mark.

#### Scenario: Brand asset approval
- **WHEN** the production wordmark and font policy are reviewed
- **THEN** the supplied resources/logo dark wordmark is reused with its integrated A, transparency and proportions intact; no whole prototype screenshot or contact sheet is embedded as UI, and any packaged font retains its required notice

### Requirement: Timeline depiction reflects real timing and audio
Dry/wet lanes, ruler, handles and readouts SHALL share actual absolute positions. The selected target SHALL use a narrow yellow dashed line across both lanes. Drawn wet data SHALL reflect real samples/shape and disclosed display scaling, without forcing its energy peak onto the target. Negative lead-in SHALL not imply output before DAW playback starts.

#### Scenario: Shifted energy and negative endpoint
- **WHEN** an asymmetric IR produces an off-target energy peak and the wet endpoint is moved before dry arrival
- **THEN** actual energy/timing, signed gap and fixed dry event remain visible at different zoom levels without drawing a fictional symmetric swell

### Requirement: Overview has one truthful waveform viewport
The heading SHALL read Overview, with a real timeline ruler above one rounded waveform viewport. It SHALL show full captured extent, lead-in as appropriate, distinct audio/silence/unmapped/unavailable states, actual swell positions and one restrained detail-viewport highlight. Overview navigation SHALL agree with the detail transform and SHALL NOT seek the host.

#### Scenario: Sparse map and viewport
- **WHEN** two captured sections with covered silence are separated by an unmapped gap and detail zoom/scroll changes
- **THEN** the gap contains no invented waveform, silence remains recognizably captured, ruler/highlight agree with actual bounds and overview interaction only navigates the editor

#### Scenario: Truly empty instance
- **WHEN** there is neither captured audio nor a selected swell
- **THEN** Swells (0), No swells yet, Press play to begin mapping your song, No swell selected and No audio mapped yet appear in stable regions
- **AND** the instruction identifies DAW Play, no fake data or forced import/generation is shown, and saved disabled mapping is explained rather than silently reenabled

### Requirement: Secondary product controls remain accessible
The header Settings menu SHALL preserve live enable/routing/level, automatic-processing/mapping preferences, future-default IR, mute, rename/delete, curve presets, Full response, audition isolation/tail/trim and recovery. Freeze/Solo SHALL remain in Selected Swell; both sensitivity knobs SHALL remain global. Scope, nondefault modes and important failures SHALL remain explicit without extra main toolbars or invented DSP.

#### Scenario: Independent live and preview choices
- **WHEN** live routing/level or preview mix/isolation is changed from Settings or its approved main control
- **THEN** only the intended saved preference changes and its current state is visible, with no implicit host transport request

### Requirement: Host information is informational and truthful
The bottom band SHALL show available host musical position, BPM and meter, with unavailable indications for unknown values. Processing/capture/readiness and important failures SHALL remain concisely visible in normal UI. No host Play/Stop strip or unverified seek/play/stop action SHALL be introduced.

#### Scenario: Unknown or stopped transport
- **WHEN** host position or musical mapping is unavailable, stops or seeks
- **THEN** each readout reflects its actual availability and state without prototype constants or fabricated beat positions

### Requirement: Exact edits preserve model and session compatibility
Knobs, fields and canvas gestures SHALL update one canonical selected-swell value, preserve independent parameters and reject invalid input transactionally. Refresh SHALL not overwrite an active edit. Existing plugin/parameter/state/asset identities and saved preferences SHALL survive; UI-only controls SHALL NOT silently become host automation parameters.

#### Scenario: Invalid edit and selection change
- **WHEN** a field receives non-finite, overflowing or invalid timing, is cancelled, or selection changes while a field is active
- **THEN** no partial edit reaches another swell, accepted values remain coherent, and focus/validation feedback identifies the result

#### Scenario: Restore legacy and current sessions
- **WHEN** supported legacy states and current version-7 sessions reopen
- **THEN** stable IDs, source/render/IR references, exact shape/placement, activity/freeze, routing and preferences are retained without autoplay or asset rewriting

### Requirement: Native visual evidence establishes readiness
Completion SHALL include matched native empty/mapped/selected screenshots and rule/geometry checks at documented sizes/scales, plus separately recorded FL Studio acceptance. Compiling or matching a generated raster SHALL NOT alone establish compliance. Unresolved native appearance acceptance, host checks or supported-scale checks SHALL remain explicitly open.

#### Scenario: Review evidence
- **WHEN** implementation is presented for acceptance
- **THEN** actual native screenshots, binding/coordinate assertions and regression results are traceable to requirements, and unavailable host or DPI checks are not marked passed

### Requirement: Both sensitivity controls remain global
Marker sensitivity SHALL remain a saved instance preference independent of selection and Recapture. The prior Regeneration sensitivity value SHALL be preserved for compatibility but its implicit source-overwrite control SHALL be disabled/retired with an explanation. Mode/Recapture SHALL exclusively govern source admission; no replacement behavior SHALL be hidden behind either sensitivity.

#### Scenario: Two distinct sensitivities
- **WHEN** an older project restores either sensitivity value and the user changes marker sensitivity in an allowed mode
- **THEN** marker selection policy remains independent, the saved tolerance value is retained without authorizing source overwrite, and selected-swell edits remain unchanged

### Requirement: IR choices are real and independently assigned
The selected-swell IR dropdown SHALL list verified bundled catalogue responses and its current imported assignment, with an adjacent Load file action. Selection SHALL affect only the intended swell, preserve other assignments/future defaults and edits, honor freeze and report async failures. Pending replacement SHALL distinguish requested IR from retained audible IR. No fictional entries or new DSP controls SHALL appear.

#### Scenario: Bundled and imported choices
- **WHEN** a user chooses the bundled Preverb Hall or loads a mono/stereo WAV/WavPack file for one swell
- **THEN** its real managed IR identity and label update after validated background preparation, while another swell and the future default remain unchanged

#### Scenario: Selection changes during file choice
- **WHEN** a file chooser opens for swell A and selection changes to swell B before completion
- **THEN** the assignment is pinned to A or explicitly cancelled, never silently applied to B; deleted/frozen/obsolete targets reject the completion safely

#### Scenario: Select another library response
- **WHEN** one swell receives Monumental, Intimate Plate, Vintage Dark Hall, Shimmer Cloud or Whisper through the existing dropdown
- **THEN** the real requested catalogue identity is prepared for that swell alone without resetting source, arrival, envelope, placement, name, activity or unrelated preferences
- **AND** the current custom response remains identifiable when it is not a catalogue entry, and a short description explains the chosen sound including Shimmer Cloud's static limitation

#### Scenario: Requested and audible responses differ
- **WHEN** an eligible IR-only replacement is preparing or fails while the old verified live contribution remains available
- **THEN** concise status identifies the requested response, retained audible response and preparation/failure state, rather than claiming that the requested IR is already playing

#### Scenario: Rapid choices and restored identity
- **WHEN** selection, session restoration or repeated catalogue choices supersede an asynchronous assignment
- **THEN** only the latest permitted target/session request can publish and catalogue ordering does not determine saved assignment identity

### Requirement: Every waveform edit has a paired inspector control
Each movable source, target or wet control SHALL have a corresponding knob and typeable value in Selected Swell: source start/end, arrival, wet start/pre-roll, wet end/offset, fade durations, independent tensions and whole-wet placement. Both directions SHALL use identical canonical validation. Grouping and inspector scrolling SHALL preserve access without shrinking waveform lanes or hiding these edits in Settings.

#### Scenario: Exercise every waveform edit
- **WHEN** each supported canvas edit is dragged and its paired inspector knob/field is then changed
- **THEN** source range, target, wet bounds, fades, tensions and placement each agree with the model within sample precision, unrelated settings are retained and selection cannot receive another swell's pending edit

### Requirement: Source selection boundaries and range are clear
Source start and exclusive end SHALL each have a matching full-height vertical boundary and grabbable handle in the source lane, with subtle uniform shading between them. The range SHALL remain distinct from the yellow arrival target and wet bounds, preserve underlying dry data and sparse-coverage indications, and synchronize with source-bound inspector knobs/exact entries.

#### Scenario: Inspect and resize a source range
- **WHEN** a user selects a swell and drags either source boundary or types its paired value
- **THEN** both boundary lines and the shaded span update at actual source coordinates without moving the dry audio, falsely filling capture gaps or confusing the arrival with the source start
- **AND** the dependent render follows existing invalidation/preparation rules while unrelated swells remain unchanged

### Requirement: Mode selector explains eligibility and transition locks
The existing editor SHALL provide Capture, Live and Finalise selection with concise roles/status and unavailable reasons. Recording/Stop settlement SHALL lock mode selection. Capture readiness SHALL be distinguished from pending wet work; no export implementation or unverified host Play/Stop action SHALL be exposed by this foundation.

#### Scenario: Stop is settling
- **WHEN** a capture pass is draining or validating
- **THEN** the mode control explains the lock, source remains provisional and no manual switch bypasses it
- **AND** successful useful source acceptance automatically selects Live even with wet jobs still pending

#### Scenario: Insufficient material
- **WHEN** there is no useful compatible source or no enabled prepared swell
- **THEN** Live/Finalise availability reflects the distinct prerequisites and explains what is missing

### Requirement: Mode guards apply below disabled UI
Capture SHALL reject creative editing and manual audition; Finalise SHALL reject creative editing while allowing only separately initiated supported preview. Live SHALL permit the current editor/audition capabilities under existing validity rules. Canvas, numeric fields, keyboard, Settings, Diagnostics and async chooser completions SHALL all obey the same mode authority.

#### Scenario: Delayed file chooser changes mode
- **WHEN** an IR chooser opened in Live completes after Finalise is entered
- **THEN** the edit is rejected/cancelled with preserved definitions/assets rather than bypassing the read-only mode

#### Scenario: Keyboard or recovery bypass attempt
- **WHEN** an edit, generation or legacy mapping command is requested through a secondary path in a disallowing mode
- **THEN** the model rejects the disallowed action, preserves state and provides the same reason as the visible control

#### Scenario: Explicit preview versus host output
- **WHEN** supported internal preview is explicitly started in Finalise
- **THEN** it is clearly identified separately from dry-only DAW output and does not enable capture or host-scheduled wet

### Requirement: Recapture status is independent and honest
An explicitly labelled Recapture existing audio control SHALL show its armed/off/settling/failure state independently of Automatic Swells. Legacy mapping controls SHALL route through workflow guards or be unavailable with an explanation. Saved mode and toggles SHALL NOT silently overwrite source or enable another policy.

#### Scenario: Return to Capture
- **WHEN** the user returns from Live to Capture with existing mapped material
- **THEN** protected coverage remains visible, Recapture remains off unless explicitly armed and the next ordinary playback fills unmapped positions only

#### Scenario: Successful recapture finishes
- **WHEN** a useful armed pass commits successfully on Stop
- **THEN** Recapture shows off, Live is selected and background rendering status remains visible without claiming all swells ready

### Requirement: Foundation visuals require native and host review
Mode selection, guarded controls and source/settlement status SHALL remain usable at 1100x850 and existing larger supported sizes. Native evidence SHALL exercise empty, recording, settling, Live, Finalise and failure states. User visual review SHALL precede final FL Studio acceptance; untested host/DPI results SHALL remain open.

#### Scenario: Review the minimal mode integration
- **WHEN** the foundation UI is ready for review
- **THEN** actual native captures and binding/guard tests show readable controls and no overlap, with a user checkpoint before broader host validation
