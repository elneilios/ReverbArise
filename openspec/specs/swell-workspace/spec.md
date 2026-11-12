# swell-workspace Specification

## Purpose
Provide a usable single-screen workspace for inspecting captured audio and editing individual swells without exposing an engineering workflow by default.

## Requirements

### Requirement: Workspace reflects actual processing and selection
The workspace SHALL adapt to mode: Learn shows capture settings and one dry timeline for the accepted source; Perform shows the production three-column editor, audition and compact Overview; Export shows its dedicated preview and mix/file controls. Processing and selection SHALL remain identity-bound, truthful and mode-appropriate. A requested render or mode SHALL NOT be shown as already ready.

#### Scenario: Select while background work completes
- **WHEN** the user selects a swell in Perform while allowed analysis or another swell's rendering completes
- **THEN** its row, handles and inspector remain bound to that identity and show actual IR/state, with editing available only under mode guards

#### Scenario: Empty or unavailable selection
- **WHEN** the session is empty or a result is waiting, stale or failed
- **THEN** the active workspace explains its state and enables only valid mode/selection actions without false ready waveforms or current-WAV claims

#### Scenario: Stable empty and loaded composition
- **WHEN** the same editor size transitions through empty, mapped, selected and updating states within a mode
- **THEN** its region bounds/dividers remain coherent and no developer toolbar displaces them or promises a deferred export capability

#### Scenario: Return to Perform
- **WHEN** a prepared user leaves Learn for Perform
- **THEN** the working swell list, selected inspector, editor navigation, audition and compact Overview return without losing definitions, selection or edits

#### Scenario: Dedicated production Perform
- **WHEN** Perform is displayed at supported sizes
- **THEN** the chronological Swells list is left, real aligned dry/wet editor is central and Selected Swell inspector is right, with audition and one separate compact Overview below
- **AND** shared header, Settings, reserved notices, production Inter/semantic roles and existing editing capabilities remain usable without changing Learn or Export composition

### Requirement: Timeline navigation preserves exact edits

The editor SHALL align dry, wet, arrivals and handles on the accepted source's absolute Song or phrase-local Loop scale
with bar references, zoom, pan and snap. The Overview SHALL distinguish audio,
captured silence, unmapped and unavailable coverage. Left-drag on the Overview
SHALL pan the detail viewport at the full-source scale without changing its span.
A click without dragging, wheel and other mouse buttons SHALL leave it unchanged.
Navigation SHALL not edit render inputs. Display-only peak scaling SHALL remain
disclosed.

#### Scenario: Navigate distant sections
- **WHEN** the user left-drags the Perform Overview between distant captured sections
- **THEN** the viewport highlight follows the drag direction within bounds and the detail view and ruler follow at the same zoom
- **AND** playheads, swells and source dependencies are unchanged and the gap remains visibly unmapped

#### Scenario: Use another gesture on the Overview
- **WHEN** the user clicks without dragging, wheels, middle-drags or right-drags over the Overview
- **THEN** the viewport, playheads and handles stay in place

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
- **WHEN** a user inserts the plugin, captures input, generates through Learn's explicit action or opted-in Generate on Stop, selects a prepared swell and edits it
- **THEN** capture, candidate review, target creation, rendering and editing work without opening Diagnostics

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
The mode-appropriate workspace SHALL remain readable at a 900x600 logical minimum and fresh opening default at 100% UI scale, and larger supported sizes, including 1400x900, 1400x1000 and 1600x1000. List scrolling and contextual windows SHALL not hide the main workflow. Supported Windows scales SHALL retain usable text/hit targets without overlap; numerical and keyboard alternatives SHALL remain available.

#### Scenario: Resize and inspect many swells
- **WHEN** the editor is resized across supported sizes with hundreds of swells and Diagnostics open or closed
- **THEN** controls remain readable and reachable, the selected row is identifiable, and essential actions do not overlap

#### Scenario: DPI and constrained window
- **WHEN** minimum and default sizes are evaluated at 100%, 125%, 150% and 200% Windows scale
- **THEN** logical region geometry stays coherent, values/units fit and controls remain operable
- **AND** any in-host scale or screen-fit limitation is explicitly recorded rather than inferred from an offscreen raster

#### Scenario: Compact baseline and responsive expansion
- **WHEN** Learn, Perform and Export are exercised at 900x600 and 100% UI scale, then resized through intermediate and larger sizes
- **THEN** each complete composition remains readable with labelled access to all supported controls and useful waveform space
- **AND** expansion adds working area without changing source, render, selection or playback state

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
Marker sensitivity SHALL remain one saved instance value independent of selection. Learn SHALL expose a horizontal slider and typeable 0-100% value within enabled Automatic Swells settings. Perform SHALL omit the obsolete Auto markers and sensitivity tooling. Low means more selective marking. Saved-off policy SHALL remain effective and truthful; changing presentation SHALL NOT reset preferences.

#### Scenario: Tune automatic markers from the main view
- **WHEN** the user adjusts/types the idle Learn slider
- **THEN** the canonical percentage updates coherently without resetting selected envelopes, IRs, blend or monitoring preferences
- **AND** idle Learn sensitivity changes refresh read-only candidates over accepted coverage without changing committed swells until an explicit or eligible Stop generation succeeds

#### Scenario: Disable marking without losing the workflow
- **WHEN** Automatic Swells is off in Learn
- **THEN** generation controls are hidden, dry capture remains available and automatic pass generation is disabled
- **AND WHEN** the user enters Perform
- **THEN** existing swells and manual editing remain usable without a marker-control band or changes to saved marking preferences

#### Scenario: An explicit master policy prevents marking
- **WHEN** legacy saved master/marker preferences disagree or automatic processing is disabled
- **THEN** Learn does not falsely show effective automatic generation as enabled or silently enable a saved-off policy
- **AND** an explicit user enable action has a documented consistent effect on the underlying marking/processing policy

#### Scenario: Resize and restore marker controls
- **WHEN** the editor is resized across supported sizes, Diagnostics is toggled or saved state reopens
- **THEN** Learn's visible sensitivity control remains reachable and synchronized with saved state, Perform retains no obsolete marker band, and selection does not change the instance value

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
Dry/wet lanes, ruler, handles and readouts SHALL share positions in the accepted source's time domain. The selected target SHALL use a narrow yellow dashed line across both lanes. Drawn wet data SHALL reflect real samples/shape and disclosed display scaling, without forcing its energy peak onto the target. Negative lead-in SHALL respect the scheduling contract of the accepted source.

#### Scenario: Shifted energy and negative endpoint
- **WHEN** an asymmetric IR produces an off-target energy peak and the wet endpoint is moved before dry arrival
- **THEN** actual energy/timing, signed gap and fixed dry event remain visible at different zoom levels without drawing a fictional symmetric swell

### Requirement: Overview has one truthful waveform viewport

Perform SHALL retain its compact Overview with ruler, real dry/wet summaries and
one detail-viewport highlight. Learn SHALL use one large dry waveform with an
origin-anchored full-source fit, bounded inspection zoom and pan, explicit coverage
states and no wet-envelope editing. Both views SHALL use the accepted source's actual
positions, disclose display scaling and avoid seeking the host through viewport
navigation.

#### Scenario: Sparse map and viewport
- **WHEN** two captured sections with mapped silence are separated by an uncaptured gap and Perform detail navigation changes
- **THEN** the gap contains no invented waveform, silence remains recognizable, ruler/highlight agree with actual bounds and navigation affects only the editor

#### Scenario: Learn has one source view
- **WHEN** Learn displays a sparse capture
- **THEN** its single large timeline starts fitted to the full-song extent, no second bottom Overview or wet lane appears, and all sections/gaps stay at their absolute positions as the user zooms and pans

#### Scenario: Truly empty instance
- **WHEN** Perform has neither captured audio nor a selected swell
- **THEN** Swells (0), No swells yet, Press play to begin mapping your song, No swell selected and No audio mapped yet appear in its stable regions
- **AND** Learn uses its mode-appropriate DAW Play prompt; neither mode shows fake data or forced import/generation, and saved-off capture/generation policy is explained rather than silently enabled

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

#### Scenario: Presentation remains read-only
- **WHEN** transport values update or the footer is resized, inspected or reopened
- **THEN** only its presentation changes, without host transport requests, timing changes, audio processing changes or saved-state edits

### Requirement: Perform transport fields retain stable geometry
Perform SHALL reserve independent Bar, Beat, BPM and meter slots with consistently aligned numeric values. At a fixed editor size and scale, field boundaries, separators and the complete status group's bounds SHALL remain fixed across value and availability updates. Geometry SHALL depend on available space and the display policy rather than current text width.

#### Scenario: Playback and seeking cross digit boundaries
- **WHEN** playback or seeking crosses bars 9 to 10 and 99 to 100, or beat and fractional-beat boundaries in either direction
- **THEN** all field bounds, separator positions and enclosing status bounds remain identical, with existing musical rounding preserved

#### Scenario: Tempo, meter and availability change
- **WHEN** tempo changes between integral and fractional displays, meter changes including 4/4 to 6/8 and 12/8, or any host field becomes unavailable and returns
- **THEN** neighbouring slots and separators stay anchored and each field reports its actual value or explicit unavailability

### Requirement: Stable transport information remains readable
The footer SHALL retain existing typography, semantic ink and dark styling at supported logical sizes and scales. Its slots SHALL accommodate realistic multi-digit bars, fractional beats, fractional tempos and meters without truncation or overlap. Oversized values SHALL remain inspectable without moving neighbouring slots or silently changing host values or displayed precision.

#### Scenario: Supported size and scale matrix
- **WHEN** the footer is shown at 1100x850, 1400x900, 1400x1000 and 1600x1000 with 100%, 125%, 150% and 200% scale coverage
- **THEN** ordinary and multi-digit values, units, separators and unavailable indications remain legible and contained above context help, with no automatic font shrinking
- **AND** native software-scale evidence and physical Windows/host coverage are recorded separately

#### Scenario: Large positions and unusual host values
- **WHEN** display values include negative lead-in, six-digit bars, three-digit beats with fractional display, four-digit tempos with two decimals or three-digit meter parts
- **THEN** they fit without truncation and maintain the same slot geometry as ordinary values
- **AND** still larger representable values retain a full accessible/help representation and a clear bounded overflow indication without overlapping adjacent fields

#### Scenario: Scoped presentation acceptance
- **WHEN** the revised read-only footer is ready for production integration
- **THEN** its native preview is launched directly for owner look/feel review, including changing and unavailable values at minimum size
- **AND** new tokens or visual exceptions receive explicit scoped approval before rollout, with standards and regression coverage updated together

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
The shared header SHALL show prominent Learn, Perform and Export segments with concise roles/status and accessible unavailable reasons. Active, available and disabled states SHALL differ. Recording/Stop settlement SHALL lock selection and keep source provisional. Source readiness SHALL differ from pending wet work. Existing read-only workspaces SHALL remain until replaced; no unfinished export delivery or unverified host transport action SHALL be exposed.

#### Scenario: Insufficient material
- **WHEN** a fresh, silent-only or incompatible instance has no eligible source or no enabled prepared swell
- **THEN** Perform/Export availability reflects the distinct existing prerequisites and explains what is missing without treating a pending render as required for source-ready Perform

#### Scenario: Stop is settling
- **WHEN** Learn records or its Stop-triggered source transaction is draining or validating
- **THEN** no segment interaction can switch modes, the active mode remains visible, and status identifies recording or source settlement
- **AND** successful useful source acceptance selects Perform without waiting for queued wet rendering; empty/failed passes retain the foundation policy

### Requirement: Mode guards apply below disabled UI
Capture SHALL reject ordinary creative editing and manual audition, with the explicit guarded Learn Generate transaction as its sole new definition-commit exception; Finalise SHALL reject creative editing while allowing only separately initiated supported preview. Live SHALL permit the current editor/audition capabilities under existing validity rules. Canvas, numeric fields, keyboard, Settings, Diagnostics and async chooser completions SHALL all obey the same mode authority.

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

### Requirement: Header navigation is accessible and truthful
The segmented selector SHALL support pointer and keyboard activation with visible focus, accessible names and selected/unavailable state. Disabled reasons SHALL be obtainable without attempting a transition. Selection SHALL reflect accepted model state, never an optimistic click. Refresh, repeated active selection and incidental wheel movement SHALL NOT initiate mode changes.

#### Scenario: Keyboard selection and stale availability
- **WHEN** a user focuses a mode and activates it with the keyboard while readiness changes
- **THEN** the same guarded request used by pointer activation determines acceptance, failure retains the accepted mode, and focus and a meaningful reason remain available

#### Scenario: Read unavailable reason
- **WHEN** a mode is unavailable because of recording, settlement, missing source or incompatible wet timing
- **THEN** its accessible description or equivalent focusable help exposes the actual reason even though activation is disabled

### Requirement: Shared header preserves approved composition
All modes SHALL share the approved wordmark, italic tagline, Semibold mode labels, regular mode description with an aqua accent, and Settings action. Brand and controls SHALL retain coherent alignment and readable production roles. A reviewed compact header treatment SHALL keep mode navigation, Settings and reserved notices visible; the compact approved header SHALL omit the decorative tagline in favour of the mode description beside the optically aligned wordmark and a full-width reserved message row. The description and contextual-message rows SHALL fit without clipping or overlap. Routine Perform status SHALL remain absent; important notices SHALL use the conditional inline bar. Descriptions SHALL NOT imply deferred WAV delivery exists.

#### Scenario: Minimum and larger editor sizes
- **WHEN** empty, ready, recording, settling and failure states are displayed at 1100x850 and supported larger logical sizes
- **THEN** all segments, Settings, brand and concise status remain readable and operable, with existing workspace controls reachable and no redundant mode dropdown

#### Scenario: Export remains a foundation workspace
- **WHEN** Export is selected before its dedicated delivery capability is implemented
- **THEN** the header reports dry DAW passthrough and explicitly initiated supported preview honestly, with no false ready-WAV or drag/export action

#### Scenario: Compact header with a notice
- **WHEN** the 900x600 editor displays a long mode description or actionable notice
- **THEN** mode labels, Settings, the wordmark and message access remain legible without collision, and appearing notices do not shift controls
- **AND** the full normal Perform warning fits without ellipsis, message adaptation is bounded, and the original wordmark artwork/proportions remain intact

### Requirement: Header controls receive incremental owner acceptance
New or materially revised header control families SHALL receive explicit owner approval of appearance and interaction feel before wider rollout. Representative native/host examples SHALL cover focus, activation, disabled feedback and scale; appearance approval SHALL NOT imply untried interaction or host/DPI acceptance. Integrated header approval SHALL precede final host sign-off.

#### Scenario: Component checkpoint before integration
- **WHEN** the segmented selector and Settings action are ready in a representative control fixture
- **THEN** the owner is asked for look-and-feel feedback, requested adjustments are resolved, and approval or untested behavior is recorded before dependent integration proceeds

#### Scenario: Integrated checkpoint
- **WHEN** the approved controls are integrated into the shared header across modes
- **THEN** the owner reviews composition and interaction at supported sizes before final FL Studio acceptance, with remaining gaps explicitly open

#### Scenario: Minimal host acceptance
- **WHEN** automated component/state/processor/persistence checks and native owner checkpoints pass
- **THEN** FL Studio UAT is consolidated into one brief final control-feel, mode-journey and save/reopen smoke test; detailed edge cases rely on automation, with extra host checks requested only for failures or specific unresolved host behavior

### Requirement: Important notices use a conditional inline message bar
Important notices SHALL use a nonmodal reserved row beneath the branding, with readable severity and accessible detail. One message SHALL NOT shift the workspace, cover controls or steal focus. The global-settings divider SHALL remain visible. Routine idle/ready information SHALL be absent; relevant notices SHALL remain until resolution. Perform SHALL retain its safety fallback when no other notice is active. Identical refresh SHALL NOT repeat announcements.

#### Scenario: Clean idle workspace
- **WHEN** Perform is idle with no other notice, or Learn is idle with Recapture off and no relevant error
- **THEN** Perform shows its safety warning while Learn retains its clean message-free state, and the normal compact layout is retained in both

#### Scenario: Recording and failure
- **WHEN** recording, source validation, Recapture arming or a relevant failure requires attention
- **THEN** the highest-priority current notice is visible beneath the branding, its text and severity are accessible, and all existing controls remain reachable at the minimum window size
- **AND** the divider above global settings stays visible below the reserved row in both notice and clean states

#### Scenario: Notice resolves
- **WHEN** the underlying higher-priority notice condition clears
- **THEN** Perform restores its safety fallback while other modes follow their existing notice policy, without changing layout or parameters, moving audio or interrupting an active control edit

### Requirement: Perform explains independent prepared playback
Perform SHALL show a persistent warning-level fallback explaining that prepared swells can remain audible during DAW playback independently of source audibility. Other current actionable, error and transient progress notices SHALL take precedence, regardless of severity. The fallback SHALL resume when they clear and provide accessible/help detail about saved Song timing, Pattern playback and explicit output controls, without implying detected host state.

#### Scenario: Remain in Perform
- **WHEN** Perform is selected with no higher-priority notice, including while stopped, playing, empty or with live playback disabled
- **THEN** the reserved message bar shows “Caution: In Perform, swells can remain audible during DAW playback, even if the source is silent or muted.” at warning severity without a timeout or dismiss preference
- **AND** its full help/accessibility detail explains that saved Song-positioned swells can precede notes and continue through silence, Pattern playback or upstream Playlist/instrument mute may not stop them, and effect bypass or mixer mute can stop the swells

#### Scenario: Temporary message takes precedence
- **WHEN** a current error, actionable warning or transient preparation notice becomes applicable in Perform
- **THEN** that notice replaces the fallback according to existing notice priority, including information-level IR preparation after its existing debounce
- **AND WHEN** the higher-priority notice clears
- **THEN** the warning returns without a mode switch, focus change, control interruption or layout movement

#### Scenario: Switch modes or reopen
- **WHEN** the user enters Learn or Export, returns to Perform, or reopens the editor in Perform
- **THEN** Learn and Export retain their existing notices, and Perform recomputes its current warning/override without persisting a new preference

#### Scenario: Refresh and inspect safely
- **WHEN** the warning is repeatedly refreshed, resized or inspected with accessibility or context help
- **THEN** its warning semantics and full explanatory detail remain available without repeated identical announcements, clipped controls or additional layout space
- **AND** rendering, capture eligibility, Learn Pattern capture, dry routing, live/audition enable, source/render identities and saved state remain unchanged by presentation

### Requirement: Learn provides the approved two-column capture composition
Learn SHALL show the existing branded header/mode navigation, narrow left capture settings and one large right dry timeline. The Swells list, selected-swell inspector, audition band and duplicate Overview SHALL be hidden. Layout SHALL follow approved mockups and the flat visual system at the existing opening/minimum size and supported larger sizes.

#### Scenario: Fresh learning workspace
- **WHEN** a fresh instance opens in Learn
- **THEN** the timeline prompts DAW Play, the two independent control groups are visible and unrelated editing/export controls do not occupy the workspace

#### Scenario: Return with source already present
- **WHEN** a user returns to Learn from Perform
- **THEN** accepted source and full-song context remain visible, Recapture is off unless explicitly armed, and the Perform editor state is retained for return

#### Scenario: Compact settings and keyboard focus
- **WHEN** Learn is shown at the supported minimum or larger size and the user tabs through its controls
- **THEN** matching section headings, smaller consistent field labels and tightly grouped descriptions remain top-aligned; both right-aligned switches and all other interactive controls retain complete focus outlines inside their bounds without touching the column separator
- **AND** the timeline heading, caption, ruler and status remain aligned without overlap, preserving the approved header and dominant dry canvas

#### Scenario: Hover beside a selected workflow mode
- **WHEN** a mode segment is selected and the pointer hovers an adjacent segment
- **THEN** its entire hover border remains visible at the shared edge without changing selection, square joins, rounded outer ends, keyboard focus or accessibility order

### Requirement: Learn exposes independent progressive capture controls
Learn SHALL separate Source Capture/Recapture from Automatic Swells. Automatic Swells off SHALL hide sensitivity, Auto-generate on Stop and Generate controls while retaining their values and explaining dry-only learning. Sound SHALL retain access to the accepted Default IR independently of detection. On SHALL expose real supported generation settings and explicit Generate, with a retained Auto-generate on Stop choice. Unsupported Character algorithms/parameters SHALL be absent rather than selectable placeholders.

#### Scenario: Dry-only controls
- **WHEN** Automatic Swells is switched off while Learn is idle
- **THEN** dry capture and Recapture stay independently available, generation preferences hide, and switching on again restores their accepted values
- **AND** Sound remains accessible for the accepted future default without enabling detection or generating swells

#### Scenario: Character processing is unavailable
- **WHEN** the build has no implemented Character processing
- **THEN** no inactive/fictitious Character choices or parameter controls appear, and a bundled static IR is not presented as a Character algorithm

### Requirement: Learn generation controls are locked for a whole pass
Automatic Swells, Default IR/file assignment and Marker Sensitivity SHALL be immutable from valid DAW Play through Stop drain/validation. Visible controls and every command/completion path SHALL enforce the same pass lock, with an explanation. After settlement, later detector edits SHALL asynchronously update candidates over accepted coverage without generating; current Default IR SHALL apply to the next explicit or eligible Stop generation snapshot without rewriting accepted pass context.

#### Scenario: Active gesture or chooser crosses Play
- **WHEN** a gesture, typed edit, preset action, chooser or import completes after the pass lock begins
- **THEN** it cannot alter that pass's settings, cannot bypass disabled controls and reports cancellation/deferred next-pass intent truthfully

#### Scenario: A later default is chosen while old generation is pending
- **WHEN** a user changes an allowed future default after source acceptance while earlier wet work remains queued
- **THEN** the new choice is shown, the earlier capture context remains unchanged, any pending Generate with the old default is cancelled and the next Generate uses the new accepted default

### Requirement: Learn Default IR choices use accepted managed identities
The Learn Default IR selector SHALL offer all real bundled responses, its current custom assignment and Open from File using the supported importer. Pending intent SHALL be distinguished from the accepted default. Cancelled, failed or obsolete imports SHALL preserve the prior default; assignments to existing swells SHALL remain unchanged.

#### Scenario: Choose a default before playback
- **WHEN** the user selects a bundled response or successfully imports a mono/stereo WAV/WavPack while idle
- **THEN** it becomes the accepted future default with its stable managed identity and is used for new eligible swells from the next pass

#### Scenario: Cancel or fail a file import
- **WHEN** the picker is cancelled or decoding/validation fails
- **THEN** the prior accepted default remains selected/usable, failure is explained, and no existing assignment changes

#### Scenario: Playback begins during a pending import
- **WHEN** DAW Play locks the pass before a requested default is accepted
- **THEN** the pass uses the displayed previously accepted default, late completion cannot replace it, and any deferred/discarded request is disclosed

### Requirement: IR pickers share the instance's retained imported choices
Learn and Perform IR pickers SHALL include bundled IRs and every successfully accepted imported managed IR in this instance, deduplicated by content identity. Imported choices SHALL remain available when unused or deselected and across save/reopen. Reuse SHALL verify managed assets without requiring the original external file. Failed, cancelled and superseded imports SHALL NOT enter the collection. The collection SHALL be instance-local rather than a scanned or global library.

#### Scenario: Reuse an unselected learning import in Perform
- **WHEN** a custom learning IR is successfully imported, subsequently deselected everywhere, and the user edits another swell's IR
- **THEN** that custom IR remains selectable and selecting it changes only the intended assignment

#### Scenario: Reopen or copy an instance
- **WHEN** the instance is saved/restored or copied with an unused successful import
- **THEN** the imported choice survives without duplicates and subsequent imports into one instance do not change another instance's choices

### Requirement: Learn coverage can be understood without waveform-height guessing
Learn SHALL explicitly distinguish captured audio, exact-zero silence, uncaptured gaps, unavailable assets and provisional recording. A concise key and inspectable range/status text SHALL supplement colour and waveform height. Capture totals SHALL distinguish mapped duration, observed extent and provisional progress without inventing a known song length.

#### Scenario: Quiet audio beside silence and a gap
- **WHEN** source includes a quiet release tail, exact-zero stored samples, an uncaptured interval and an unavailable range
- **THEN** each has the correct labelled/inspectable state; a flat-looking tail is not relabelled silence and unavailable data is not relabelled empty

#### Scenario: Provisional recapture is cancelled
- **WHEN** incoming replacement peaks are shown and the pass is cancelled or fails
- **THEN** the provisional indication disappears, prior accepted source stays visible and no recording progress is represented as committed coverage

### Requirement: Learn auto-fit retains full dry-source context stably
The Learn viewport SHALL include song origin and the highest accepted/admitted provisional source position, with coalesced threshold-based expansion and modest headroom. Sparse seeks SHALL retain actual gaps. Loops, selection, wet lead-in and same-extent Recapture SHALL NOT cause repeated rescaling; Learn and Perform navigation state SHALL remain independent.

#### Scenario: Capture late and early sections
- **WHEN** playback captures bars 86-90, then seeks to bars 1-4
- **THEN** the Learn view includes origin through the late section with a real gap, and the backward seek does not continually shrink/refit it

#### Scenario: Recapture without extending coverage
- **WHEN** an armed pass revisits already known extent
- **THEN** the scale stays stable while the real provisional/accepted content changes

#### Scenario: Timing is unavailable
- **WHEN** musical timing or asset compatibility is unavailable
- **THEN** the ruler/status uses truthful available coordinates and does not invent bars, stretch old source or hide unavailable regions

### Requirement: Learn inspection is read-only, persistent across focus and ephemeral

Learn SHALL provide read-only absolute-sample inspection. Left-click or drag
SHALL pin or relocate across focus changes. Waveform-focused Escape SHALL clear
coordinate and pin together. Inspection SHALL remain available during capture,
stay ephemeral and leave capture, generation and Perform unchanged. Project/session
reset or editor reopen SHALL clear it. Inspection, transport and provisional
indicators SHALL be distinguishable.

#### Scenario: Relocate and retain a pin
- **WHEN** a left-click or drag pins a position and focus moves, the pointer leaves, capture advances or the user navigates the viewport
- **THEN** the same absolute inspection position remains and its marker is visible whenever the sample is in view
- **AND** another left-click or drag relocates it, right-click does not relocate it and Escape outside waveform focus does not clear it

#### Scenario: Inspect an active or settling pass
- **WHEN** hover, pin relocation or waveform keyboard inspection occurs while recording/draining/validating
- **THEN** inspection remains responsive without changing capture authority, accepted source/context, job counts, saved state or Perform selection

#### Scenario: Reset inspection consistently
- **WHEN** Learn is re-entered or the editor's project/instance/session is replaced, restored or cleared
- **THEN** coordinate and pinned flag clear together, hover resumes and no marker is restored from project state
- **AND** ordinary auto-fit expansion and DAW seeks preserve the absolute pin without changing manual navigation or seeking the host

### Requirement: Learn inspection readout uses reliable musical timing
The existing inspection row SHALL show exact sample and coverage classification, plus bar/beat and elapsed time from song origin only with reliable coherent host timing and compatible capture rate. Missing/incompatible timing SHALL be explicitly unavailable. Full coverage ranges SHALL remain accessible without new controls or fabricated coordinates.

#### Scenario: Inspect with and without reliable timing
- **WHEN** an inspected sample has valid coherent host timing and a compatible capture rate
- **THEN** its bar/beat and elapsed time from song origin are shown with exact sample and truthful audio/silence/gap/unavailable/provisional classification
- **AND WHEN** that timing is unavailable or incompatible
- **THEN** exact sample/coverage remain available and musical/elapsed timing is explicitly unavailable rather than fabricated

### Requirement: Mode labels have semantic vector icons
Learn, Perform and Export SHALL display consistent resolution-independent outline icons immediately before their labels. Learn SHALL use varying-height waveform bars; Perform SHALL show a smooth reverse-swell rise, short release and three outlined handles; Export SHALL use an audio-document/output motif. Icon and label SHALL use off-white inactive ink and dark active ink on aqua. Icons SHALL preserve the established segment geometry and navigation behaviour.

#### Scenario: Inspect mode icon states
- **WHEN** each mode is active, inactive, hovered, disabled or keyboard focused
- **THEN** icon and label remain optically aligned and readable with consistent spacing and stroke weight
- **AND** complete borders and focus outlines remain visible with the concurrent clipping corrections preserved

#### Scenario: Navigation remains model owned
- **WHEN** pointer, keyboard or accessibility activation requests an available or unavailable mode
- **THEN** the existing guarded intent determines acceptance, accessible names remain Learn, Perform and Export, and decorative icons create no additional focus or action targets

#### Scenario: Consistent small-size artwork
- **WHEN** the canonical SVG icons are displayed at 14 and 16 logical pixels and scaled for supported DPI
- **THEN** they share a 16-unit canvas, a proportionally scaling 1.6-unit rounded stroke and transparent fill, with three distinct outlined Perform handles and no clipped stroke or external image/font dependency
- **AND** the nominal navigation uses a 16 px icon canvas and 8 px icon-to-label gap without changing segment or focus geometry

### Requirement: Header information shares typography without losing severity
The mode description SHALL have a 2 px aqua vertical accent, an 8–10 px text gap, regular text and no icon, fill or enclosing border. Description and contextual-message body text SHALL share font family, size, regular weight, #C7CDD4 ink and comparable vertical alignment. Contextual accents and icons SHALL preserve severity colours. Both columns SHALL retain reserved bounds and existing header dimensions without changing branding or main-workspace placement.

#### Scenario: Information warning and error messages
- **WHEN** different-length contextual messages appear or disappear beside each selected mode description
- **THEN** severity remains distinguishable, both columns remain within their own bounds and no controls or workspace regions move
- **AND** overlong contextual text retains full tooltip/accessibility detail while its visible presentation stays within the message column

#### Scenario: Supported scale and size
- **WHEN** the header is shown at existing supported logical sizes and Windows scale factors
- **THEN** vector icons, text, accents and focus outlines remain complete and legible with no collision between the contextual message and mode description

#### Scenario: Information-row metrics
- **WHEN** a mode description and contextual message share the existing 32 px information row
- **THEN** both body texts use 14 px regular Inter and the same vertical text alignment, while the description's 2×16 px aqua accent is vertically centred with an 8 px gap before text
- **AND** contextual severity markers retain their existing geometry and colours, and header-specific body styling does not change unrelated controls

### Requirement: Export provides a dedicated waveform-to-controls workspace
Export SHALL reuse the approved header above an approximately 66% full-height preview and 33% controls column. The preview SHALL include ruler, legend and internal Play/Stop; controls SHALL include exact Dry–Wet percentage, Drag WAV, Save WAV and Return to Perform. Perform's list/inspector, duplicate Overview and Export Summary SHALL be absent. Shared controls/fonts, accessibility and supported sizes SHALL be preserved.

#### Scenario: Dedicated minimum-size composition
- **WHEN** a ready session enters Export at 1100x850 or supported larger sizes
- **THEN** waveform dominates the left, export actions remain reachable on the right and no modal/dimmed Perform editor or duplicated summary appears
- **AND** keyboard focus, wheel, typed percentage and any numeric steppers use the approved reusable behavior without clipping

#### Scenario: Pending or failed composition
- **WHEN** the current mix is preparing, unavailable or failed
- **THEN** current readiness/reason is concise and accessible, Save/Drag availability follows that revision and the layout does not flash transient helper text or shift controls

### Requirement: Export waveform depicts the audible mix at absolute positions
Export SHALL draw actual accepted dry/wet data with a shared amplitude scale and the composition gain plan. Dry SHALL use neutral white/grey and wet aqua; zero-weight layers SHALL be invisible. Actual wet pre-arrival rise, placement, crop and post-arrival tail SHALL be preserved. Ruler, preview cursor and coverage indications SHALL remain truthful and visually distinguishable.

#### Scenario: Compact navigation and mix guidance
- **WHEN** an accepted mix is displayed
- **THEN** the compact colour legend shows each accepted layer percentage and is right-aligned on the shared zoom toolbar row beneath the viewer without a numerical scale caption
- **AND** Ready dry-inclusive WAVs show mute-source guidance in the WAV tile rather than below the waveform
#### Scenario: Compare 0 50 and 100 percent
- **WHEN** the same asymmetric source/wet fixture is shown at 0%, 50% and 100% Wet
- **THEN** dry alone, scaled dry plus rising wet, and wet alone respectively match the audible composition, including real tails and overlap
- **AND** no conventional reverb-decay illustration or unweighted grey dry waveform implies an exported contribution at 100% Wet

#### Scenario: Fit audible region while retaining song alignment
- **WHEN** Export is entered or its mix changes with long leading silence
- **THEN** initial fitting and explicit preview start use the current audible extent, the ruler retains original song coordinates and a clear alignment indication explains that WAV frame zero is song origin
- **AND** an empty/all-zero result shows an honest empty state without fabricated peaks or trimming exported lead-in

#### Scenario: Inspect missing dry
- **WHEN** dry-inclusive composition substitutes silence for gaps or unreadable source
- **THEN** coverage/readout and a persistent warning retain the missing-data distinction without invented dry peaks, new waveform editing or DAW seeking

### Requirement: Export navigation is read only and fits the complete audible mix
Export SHALL fit the complete accepted audible mix with visual margins, independent of Perform selection and WAV lead-in. Read-only Fit, bounded zoom/pan and keyboard navigation SHALL reuse existing mapping, preserve manual inspection on refresh and remain ephemeral. A newly accepted mix may refit once.

#### Scenario: Complete fit and inspection
- **WHEN** separated/overlapping swells, early pre-roll and late quiet tails are fitted, zoomed and panned
- **THEN** Fit includes the entire audible mix with margins, ruler/cursor remain aligned and ordinary refresh preserves manual inspection
- **AND** file origin, samples, captured/render assets and creative settings remain unchanged

### Requirement: Previous waveform remains truthful during mix preparation
Pending/failed requests SHALL retain a dimmed, percentage-labelled previous waveform and legible status. Only a fully published matching WAV SHALL enable current Play/Save/Drag. The proportionate tile and colour legend SHALL identify readiness and actual layer contributions without implying zero-weight audio.

#### Scenario: Rapid mix changes and failure
- **WHEN** multiple percentages supersede one another or current preparation fails
- **THEN** previous accepted data remains labelled, stale actions stay disabled, only the latest matching success replaces it and the viewport does not oscillate while pending

#### Scenario: Audible duration versus WAV duration
- **WHEN** wet-only audible extent is shorter than the dry-inclusive extent
- **THEN** Preview explicitly describes audible position/duration while WAV retains its declared song-origin range and complete contributions

### Requirement: Export waveform positions internal preview only

Left-click and left-drag SHALL position the ready internal preview using the same
absolute mapping as waveform, ruler and cursor. Stopped clicks SHALL not autoplay.
Playing seeks SHALL resume after bounded preparation. A pending previous
waveform SHALL support navigation while positioning remains disabled. Position
SHALL be ephemeral and SHALL not change DAW transport or export audio.

#### Scenario: Positioning while zoomed or panned
- **WHEN** a ready waveform is clicked or dragged after viewport navigation
- **THEN** cursor/readout identify the mapped clamped sample and Play starts there without changing view, mix or WAV alignment
- **AND** middle-drag pans without changing preview position

#### Scenario: Navigate the previous waveform while pending
- **WHEN** a previous waveform is displayed while a replacement mix is pending
- **THEN** wheel zoom and middle-drag pan remain available, while left-click and left-drag leave preview position and readiness unchanged

### Requirement: Export consumes the shared timeline controls
Export SHALL use the shared Zoom − / slider / Zoom + / Fit row and musical ruler.
Its binding SHALL retain accepted navigation limits, logarithmic zoom, absolute
sample/timing mapping and complete-audible Fit. Ordinary refresh SHALL preserve
manual inspection and reflect the actual clamped view without issuing intents.
Stale session/pass/composition callbacks SHALL not modify a replacement view.

#### Scenario: Controls and pointer navigation agree
- **WHEN** the user zooms using buttons, slider, wheel or keyboard and returns to Fit
- **THEN** the row, ruler, waveform and preview cursor agree on one accepted viewport, all audible contributions fit, and WAV/mix/source/render state remains unchanged
- **AND** previous-waveform navigation remains available while current preparation gates playback/delivery, without adding Snap or editing handles

### Requirement: Swells list follows canonical target chronology
The Swells list SHALL display ascending canonical absolute target/arrival position, with persistent swell ID as the deterministic ascending tie-breaker. Names, insertion order, rendered bounds, wet placement and formatted bar/beat text SHALL NOT affect order. Any position label SHALL derive from canonical position using available timing, never from parsing names.

#### Scenario: Capture an earlier section after a later section
- **WHEN** a later captured section already has swells and a new swell is accepted earlier in the song
- **THEN** the earlier swell appears before later targets on the next normal UI refresh without manual reload
- **AND** targets corresponding to bar 2 sort before bar 10 regardless of their names or creation order

#### Scenario: Rename or change cosmetic and wet settings
- **WHEN** a swell receives an arbitrary name or changes status, IR label, envelope or whole-wet placement without moving its target
- **THEN** its ordering relative to unchanged targets stays the same and the user's name remains intact

#### Scenario: Multiple targets share a position
- **WHEN** multiple swells have equal target positions
- **THEN** their persistent IDs determine a stable order across refresh, reanalysis and save/reopen regardless of stored definition order

### Requirement: Chronological rows retain swell ownership
Selection, primary selection, row actions and delayed row commands SHALL follow persistent swell identity across reordering. Presentation refresh SHALL NOT submit selection edits or alter swell settings, source, assets, generation or audio scheduling. Commands from obsolete sessions SHALL be rejected or cancelled under existing ownership guards.

#### Scenario: Move the selected target across other rows
- **WHEN** the selected swell's target moves earlier or later through a supported canvas or inspector edit
- **THEN** its displayed row moves to the chronological position and the same primary and selected identities remain selected
- **AND** the inspector and subsequent row actions address those identities

#### Scenario: Reorder with multiple selection and a pending menu
- **WHEN** several swells are selected and a background update reorders rows while a row menu or rename dialog is open
- **THEN** surviving selected identities and primary are retained, the delayed action addresses its original identity or is safely cancelled, and no action reaches the replacement occupant of a row
- **AND** enabled, mute, solo, freeze, IR, shape, manual/pinned-marker metadata and unrelated swell settings remain attached to their original identities

#### Scenario: Capture reconciliation and restoration
- **WHEN** noncontiguous capture, seeks/loops, recapture, reanalysis or a supported project restore publishes accepted swell definitions
- **THEN** the list reflects current target chronology, surviving selection follows IDs and restored settings/assets retain their identities
- **AND** presentation itself neither moves targets nor changes playback placement or regenerates audio

### Requirement: Chronological updates preserve list scroll context
Routine refresh and background insertion/reordering SHALL preserve the visible list context by surviving swell identity and relative row offset where possible. Explicit primary selection and movement of a visible primary SHALL keep that primary visible with minimal scrolling. Refresh SHALL NOT repeatedly pull the user back to an offscreen selection. Missing anchors SHALL use a bounded nearby fallback.

#### Scenario: Insert above a scrolled viewport
- **WHEN** the user has scrolled through a long list and background capture inserts earlier targets above the viewport
- **THEN** the surviving top visible swell retains its relative vertical offset where list bounds permit, without jumping to the first row or an offscreen primary

#### Scenario: Follow an actively moved or newly selected primary
- **WHEN** the user explicitly selects a primary or moves a currently visible primary across rows
- **THEN** the primary stays visible using the least necessary scroll adjustment while other selected IDs remain selected

#### Scenario: Unchanged refresh and disappearing anchor
- **WHEN** identical snapshots, cosmetic-only changes or deletion of the visible anchor are refreshed
- **THEN** unchanged rows retain their scroll position and a missing anchor falls back to the nearest surviving visible row or a clamped prior viewport position
- **AND** empty lists and returning from Learn remain valid without stale row actions or out-of-range scrolling

### Requirement: Perform compact composition requires native acceptance
Perform SHALL use owner-approved scoped compact spacing with approved responsive production typography and consistent semantic ink, focus and usable hit targets. Dense, sparse, empty, pending and failed states SHALL remain readable and bounded. New or materially revised control families SHALL receive native look/feel approval before wider rollout.

#### Scenario: Review compact spacing
- **WHEN** a compact Perform composition is proposed
- **THEN** a directly launched native preview supplies actual row, panel, gap, lane and focus measurements for owner approval before production adoption
- **AND** Learn/header changes remain limited to the separately reviewed compact baseline and otherwise retain their accepted geometry

#### Scenario: Long content and resizing
- **WHEN** many swells, long names/IRs, existing multiple selection and empty/loading/error states are shown at 900x600, 1000x700, 1100x850, 1400x900, 1400x1000, 1600x1000 and width breakpoints at software 100/125/150/200%
- **THEN** labels, exact fields, focus and actions do not overlap or clip; supported truncation provides full tooltip/accessibility detail and inspector scrolling preserves every supported edit
- **AND** native owner acceptance, software scale, actual Windows DPI and FL Studio coverage are recorded separately

#### Scenario: Composite divider continuity
- **WHEN** the complete native Perform composition is painted with its actual child components
- **THEN** horizontal rules remain continuous across the available width at the header, audition, Overview and information boundaries, with column content inset and vertical rules separated from those horizontal rules

### Requirement: Perform toolbar provides horizontal Zoom and contextual Fit
Perform SHALL provide grouped Zoom minus, horizontal zoom slider, Zoom plus, Fit, Snap toggle button and rhythmic-grid selector, replacing Prev/Next and page-span controls. These groups SHALL share one inline row when they fit and reflow into two usable rows at compact widths without shrinking control targets or losing any navigation action. Zoom SHALL change only horizontal view scale and synchronize the real dry/wet, ruler, handles and Overview viewport. Display-amplitude scaling SHALL remain separately disclosed.

#### Scenario: Zoom extremes and anchors
- **WHEN** buttons, slider or pointer-wheel navigation reach either zoom extreme
- **THEN** the viewport remains finite and bounded, with stable pointer/selected-target/playhead context and all time layers aligned
- **AND** source PCM, swell positions, selection, gains, render identities and job counts remain unchanged

#### Scenario: Fit selected and unselected material
- **WHEN** Fit is activated with a selected swell, including pending wet or overlap
- **THEN** the view includes its actual source, requested pre-roll/arrival/end and available render-tail context without fabricated audio
- **AND WHEN** no swell is selected
- **THEN** Fit shows available captured extent with truthful sparse gaps, or a finite empty view

#### Scenario: Initial entry and return
- **WHEN** Perform first receives usable source without a saved view
- **THEN** it Fits once without waiting for all wet renders
- **AND WHEN** background publication, incidental selection refresh or mode return occurs after user navigation
- **THEN** the user view is retained until explicit Fit or navigation, including explicit Swells-list selection that applies contextual Fit, without host seeking or repeated auto-fit

#### Scenario: Rendering completes after list navigation
- **WHEN** a pending selected render completes after the user has selected another swell or manually panned or zoomed
- **THEN** matching waveform content refreshes without replaying list reveal, refitting or overriding the user's newer view

#### Scenario: Compact navigation reflow
- **WHEN** the centre column cannot accommodate both navigation groups in one row
- **THEN** Zoom/Fit and Snap/Grid occupy separate rows with complete focus and hit areas, and no navigation action is hidden
- **AND** the detail canvas remains usable and all existing zoom, Fit, snap and temporary-modifier behavior is preserved

### Requirement: Snap preference and temporary bypass are independent
Snap SHALL be a latching toggle button, not a checkbox. Its remembered On/Off preference SHALL be independent of the remembered rhythmic grid. Ctrl SHALL temporarily bypass snap and show the button off without changing either preference. Shift SHALL independently enable existing fine-drag; Ctrl+Shift SHALL combine both behaviors.

#### Scenario: Hold and release Ctrl
- **WHEN** Snap is remembered On and Ctrl is pressed with or without an active drag
- **THEN** effective snapping and the toggle button's visible/accessibility pressed state become Off while held
- **AND WHEN** Ctrl is released
- **THEN** both return On without a saved-preference change, host dirty notification or key-only handle movement

#### Scenario: Saved Off and combined modifiers
- **WHEN** Snap is remembered Off and Ctrl is pressed/released
- **THEN** it remains Off and the grid choice is retained
- **AND WHEN** Ctrl+Shift is held during a timing drag
- **THEN** movement is unsnapped and fine; Shift alone remains fine with the remembered snap policy, and Ctrl alone bypasses snap with normal drag precision

#### Scenario: Modifier lifecycle and existing shortcuts
- **WHEN** modifier keys change during a gesture, editor focus leaves/returns, or Ctrl-wheel zoom or existing list-selection shortcuts are used
- **THEN** button state and effective snap reconcile with actual Ctrl state without a stuck bypass, another target's edit, an unintended saved toggle or broken existing shortcuts
- **AND** key transitions alone do not move handles; subsequent pointer movement uses the effective policy without an artificial drag-baseline jump

### Requirement: Rhythmic snapping uses trustworthy host coordinates
Snap SHALL offer explicit 1/16 beat, 1/8 beat, 1/4 beat, 1 beat and 1/2/4 bar divisions. Beat units SHALL follow the reported meter denominator and bars its numerator. Quantization SHALL use validated absolute host PPQ origin with nearest-grid rounding and checked sample conversion, never display names, pixels or a fabricated tempo map. The accepted captured grid SHALL remain the editing coordinate system when an independently validated fixed-tempo playback run has a different origin.

#### Scenario: Meter and fractional PPQ
- **WHEN** trustworthy compatible timing reports 4/4, 3/4 or 6/8 with fractional PPQ or negative lead-in
- **THEN** each division quantizes to its absolute host grid with deterministic half ties away from PPQ zero and at most integer-sample rounding error
- **AND** unchanged-origin seeks/loops retain that mapping without changing captured positions

#### Scenario: Missing or changed mapping
- **WHEN** host timing is absent, non-finite, overflowing, capture-rate incompatible, has incompatible tempo/meter, or its origin matches neither the accepted mapping nor an independently validated current fixed-tempo playback run
- **THEN** effective Snap On prevents horizontal snapped publication and explains unavailable timing rather than guessing
- **AND** Snap Off or Ctrl bypass permits valid unsnapped sample edits while navigation and vertical editing remain usable
- **AND** a known rejected current run SHALL NOT become available through a fallback to the saved origin, even if the latest observation happens to match that saved origin

#### Scenario: Timing changes during a drag
- **WHEN** a drag's session, selection, mode or validated mapping changes
- **THEN** further publication from that obsolete gesture is cancelled with prior accepted musical/source state preserved and an understandable reason

#### Scenario: Stable fixed-tempo playback offset
- **GIVEN** accepted capture timing and an independently validated current playback run have compatible rate, tempo and meter
- **WHEN** the host remains consistent with that run although its origin differs from the accepted capture origin, including the recorded approximately +16-sample case
- **THEN** Snap remains available and quantizes to the same accepted captured-grid sample positions as before Play
- **AND** positive, negative and zero stable offsets use the same validation policy without a hard-coded offset or expanded drift tolerance
- **AND** Play, Stop, seeks and loops alone do not rewrite accepted timing, source, swell positions or render assets

#### Scenario: Actual drift after an admitted offset
- **GIVEN** a shifted fixed-tempo run previously made the captured grid available
- **WHEN** subsequent host timing violates that run's existing one-sample mapping guard or its rate, tempo or meter compatibility
- **THEN** snapped publication becomes unavailable and obsolete gestures cannot continue under the previous admission
- **AND** diagnostics distinguish the run rejection from an informational source-to-run offset

#### Scenario: Shared editing and readiness
- **WHEN** the shifted run is valid and the user uses canvas timing gestures, inspector knobs/wheel, snapped keyboard actions or target insertion
- **THEN** each path uses the same accepted grid and availability decision with existing canonical bounds, modifiers, exact-entry and identity guards
- **AND** grid availability does not require an audible swell, prepared render, selected IR, enabled wet output or an open Diagnostics window

#### Scenario: Stopped empty and restored sessions
- **WHEN** the transport is stopped, an empty session bootstraps timing, or a saved session is reopened
- **THEN** existing trustworthy stopped/empty behavior remains and a retired or unavailable run cannot authorize another session's grid
- **AND** unchanged source/render identities and saved Snap/grid preferences survive with no autoplay or persistent clock migration

#### Scenario: Tempo automation remains outside fixed-grid coverage
- **WHEN** instantaneous BPM changes or timing does not establish a compatible fixed-tempo mapping
- **THEN** this fixed-tempo repair does not fabricate a global grid or infer exact tempo-ramp conversions
- **AND** exact sample editing and existing temporary bypass remain available under their normal model guards, with truthful timing feedback

### Requirement: Horizontal handle snapping preserves exact musical edits
Effective Snap SHALL quantize horizontal target, source-bound and editable envelope-time gestures to one absolute grid. Vertical amplitude/tension SHALL remain continuous. Whole-wet translation SHALL preserve internal spacing. Exact numeric entry SHALL remain unsnapped. Every accepted edit SHALL preserve ordering, source PCM, unrelated identities and existing model guards.

#### Scenario: Endpoints and fades with wet placement
- **WHEN** an arrival, source boundary, wet start/end or fade boundary is dragged with nonzero placement and Snap On
- **THEN** its absolute displayed timing lands on a valid grid point and agrees with the exact inspector value through canonical validation
- **AND** a domain with no valid grid point rejects the edit instead of claiming an off-grid clamp is snapped

#### Scenario: Vertical and translated edits
- **WHEN** a tension/amplitude handle is moved vertically or the wet body is translated horizontally
- **THEN** vertical values remain continuous, while translated snap anchors move the complete wet shape without quantizing its internal durations or moving dry PCM

#### Scenario: Fine and exact alternatives
- **WHEN** Ctrl bypass, Shift fine-drag, combined Ctrl+Shift, exact numeric input or keyboard alternatives are used with overlapping handles or zero fades
- **THEN** the effective timing policy is consistent, hit anchors and accepted values agree, and no unrelated parameter, captured source or selected identity changes

#### Scenario: Inspector timing knobs follow the grid
- **WHEN** Target, Source Start/End, Move Wet, Pre-roll, End Offset or Fade In/Out is adjusted by knob drag or wheel with effective Snap On
- **THEN** the corresponding absolute timeline anchor snaps through the same validated host grid and canonical bounds as the canvas, with Move Wet preserving internal spacing
- **AND** Ctrl bypasses snapping, Shift retains fine drag and Ctrl+Shift combines both without key-only movement or modifier-induced jumps
- **AND** unavailable timing or a domain without a valid grid point rejects snapped publication with a reason; accepted dial/value/canvas feedback agrees
- **AND** typed values remain exact and unsnapped, curve and level knobs remain continuous, and stale session/selection/mode/source/timing intents cannot publish
- **AND** unsnapped wheel/arrow increments are independent of zoom: timing 1 ms, level 1 dB, curves 0.01; Shift divides by ten with internal sample rounding, while snapped actions advance one grid point
- **AND** user-facing labels expose musical position or time units, seconds display six decimals, milliseconds three, level two and curves three, without reducing valid typed/saved precision

### Requirement: Knob reset is explicit and identity guarded
Knobs SHALL support double-click reset and a right-click reset action using existing model defaults or automatic wet start. Target and Source Start/End SHALL restore positions first loaded for that swell in the current editor. Reset SHALL bypass Snap, preserve canonical validity and unrelated musical/source/IR identities, and SHALL NOT invoke whole-swell reset or persist transient initial positions.

#### Scenario: Reset one knob
- **WHEN** an enabled knob is double-clicked or its reset menu action is selected
- **THEN** its guarded canonical default/initial operation runs, dial/text/canvas show accepted state and invalid bounds leave the prior state intact
- **AND** resetting an already-default value does not schedule work or dirty state, and the right-click menu names default versus initial/automatic semantics truthfully

#### Scenario: Reset menu becomes stale
- **WHEN** selection, source, fixed timing, workflow/session or component lifetime changes before a queued reset is delivered
- **THEN** the obsolete action cannot edit another swell/session or revive a destroyed component

### Requirement: Perform navigation preferences survive reopen safely
Perform SHALL retain valid view, requested Snap state and grid across editor reopen, mode return and project save/reopen, independently per instance. Legacy or malformed optional navigation values SHALL fall back safely without discarding valid musical state. Held modifiers and active gestures SHALL remain ephemeral; restore SHALL not autoplay, recapture or regenerate audio.

#### Scenario: Reopen current and legacy state
- **WHEN** a project with user zoom and Snap Off is reopened, or a legacy state lacks navigation preferences
- **THEN** valid preferences return, while legacy uses contextual Fit, Snap On and quarter-beat grid without changing source/render/IR references, IDs or existing parameters

#### Scenario: Save during temporary bypass
- **WHEN** a project is saved while Ctrl temporarily masks remembered Snap On
- **THEN** reopen retains the remembered On/grid preferences and has no stuck bypass or restored gesture

#### Scenario: Invalid preferences and copied instances
- **WHEN** optional view/grid values are malformed or an instance is copied/restored
- **THEN** invalid UI fields alone default safely, valid musical content survives, and subsequent navigation does not change another instance

### Requirement: Perform presentation supports precise musical inspection
Perform SHALL retain its three columns while presenting a shared musical ruler
aligned to both editable lanes, concise lane labels with explanatory bottom context help,
full Source labels, grouped timing/envelope/level controls and a readable musical
footer. Preview metrics SHALL receive native owner acceptance before rollout.
Compact values SHALL NOT reduce stored precision or change exact typed editing.
Overview and Audition headings SHALL share their role and text-box margin policy.

#### Scenario: Inspect and edit at minimum size
- **WHEN** Perform is shown at 900x600 and 100% UI scale, and larger supported sizes, with selected/dense or empty data
- **THEN** ruler/lanes/handles agree after zoom/pan, group labels and Source captions remain readable, and labelled section access or scrolling reaches every inspector field without clipping while Reset and reset options remain fixed and visible
- **AND** missing timing remains explicit and essential edit warnings remain visible

#### Scenario: Compact readout preserves precision
- **WHEN** a compact value receives focus and Return without a user change
- **THEN** its full edit precision is exposed without mutating state, jobs or durable source/render identities
- **AND** explicit typed values retain existing canonical validation and precision while musical display rounding carries across beat/bar boundaries correctly

### Requirement: Hover context uses the plugin bottom footer
All control and graph hover help SHALL appear in one full-width bottom row below
the position readout, using normal helper typography, without popup tooltips or a
graph overlay. Existing help metadata and control behavior SHALL be reused.

#### Scenario: Hover any control or graph region
- **WHEN** the pointer enters a control, nested value field, graph region or owned Settings/Diagnostics control
- **THEN** the footer shows its help in normal-sized text without resizing or covering the work area, intercepting input, mutating state or scheduling audio work
- **AND** leaving clears help, reading the footer retains it, dynamic control state updates its text and destroyed sources cannot leave unsafe callbacks

#### Scenario: Change workspace or resize
- **WHEN** Learn, Perform or Export is shown at a supported size
- **THEN** the shared help footer remains visible at the plugin bottom and contained clear of workspace content and the position readout

### Requirement: Initial generated Perform entry selects and reveals the earliest swell
The first successful Generate-to-Perform handoff SHALL select the earliest committed swell by chronological arrival and reveal its lead-in and arrival through shared editor navigation. Explicit and automatic triggers SHALL behave identically. List, waveform/envelope, inspector and Overview SHALL agree. Current zoom SHALL be retained where practical. Empty visits and unsuccessful generation SHALL NOT consume initial-handoff eligibility.

#### Scenario: Creation order differs from song order
- **WHEN** initial explicit or automatic generation commits multiple swells stored in a different creation order
- **THEN** the earliest arrival in the committed collection is selected, its editor/Selected Swell controls appear and its lead-in/arrival are meaningfully visible without transport, audition or enablement changes

#### Scenario: Restored or established editing position
- **WHEN** an existing project opens with valid restored selection/navigation, or the user later returns from Learn to an established nonempty Perform workspace
- **THEN** that selection and viewport remain instead of jumping to the first swell or replaying a generation handoff

#### Scenario: Empty visits and unsuccessful generation
- **WHEN** Perform was previously empty or a Generate request is failed, cancelled, empty or unchanged
- **THEN** the empty visit does not consume initial eligibility, unsuccessful requests preserve their existing Learn outcomes and no selection or viewport is changed by those outcomes

### Requirement: Initial framing does not wait for or follow delayed renders
Initial Perform selection/reveal SHALL use available definition/source timing while reporting pending rendering honestly. Definition commit SHALL remain sufficient for Perform entry. Later render completion SHALL update content without overriding a newer selection or user-adjusted viewport. The handoff SHALL affect editor selection/navigation only and preserve serialized modes and existing capture/DSP behavior.

#### Scenario: Wet rendering is delayed
- **WHEN** the initial selected swell is still queued/rendering and the user subsequently selects another swell or pans/zooms before its render arrives
- **THEN** the initial handoff uses available lead-in/arrival timing immediately, and delayed completion updates only matching content/status without stealing selection or moving the user's viewport

### Requirement: Learn offers one explicit Generate action
Generate Swells SHALL be the primary Automatic Swells action below Marker Sensitivity and supported conditional parameters, or in a persistent action area below those settings at compact heights. Its labelled action and readiness helper SHALL remain directly discoverable when settings overflow. It SHALL reuse ActionButton, retain its label across runs and expose accessible readiness/progress reasons. Auto-generate on Stop SHALL reuse Switch below Enable automatic swells, before IR controls. The contextual readiness helper SHALL sit directly below Generate.

#### Scenario: Ready or disabled action
- **WHEN** Learn is ready, capturing, settling, analysing, generating or missing usable source/IR
- **THEN** Generate reflects canonical readiness, keyboard/pointer/secondary routes share the same guards and disabled reasons remain readable without a rejected click

#### Scenario: Cancel a waiting automatic request
- **WHEN** the user disables Auto-generate on Stop while its request awaits final analysis or preparation
- **THEN** the not-yet-committed request is cancelled and no lock hides the cancellation path

### Requirement: Manual placement guidance remains readable
A subdued helper SHALL read “Want to place swells yourself? Add manual swells in Perform.” It SHALL follow Generate's contextual helper when Automatic Swells is ON and remain below the enable-switch group when OFF. It SHALL use existing Inter helper typography, wrap naturally without a forced break after the question and remain readable with Generate disabled at minimum/larger sizes.

#### Scenario: Automatic Swells off and editor reopen
- **WHEN** Automatic Swells is disabled in an open editor or before reopening it
- **THEN** manual-placement guidance remains visible with valid layout while generation controls and their contextual helper stay hidden

#### Scenario: Minimum-size busy workspace
- **WHEN** Automatic Swells is ON at 1100x850 and generation is disabled or busy
- **THEN** the full helper remains legible beneath Generate, all settings remain reachable and NoticeBar does not move the workspace

### Requirement: Candidate markers have truthful visual identity
The Learn timeline SHALL show sample-aligned events as restrained neutral guides, with stronger timing-accent emphasis for hovered or pinned events. Hollow heads SHALL identify provisional/updating data. Event emphasis SHALL reuse inspection without editing swells or filtering Generate. The white inspection pin and solid DAW cursor SHALL remain distinct. Refresh SHALL be read-only and bounded at display resolution.

#### Scenario: Hover and pin an event
- **WHEN** the musician hovers or pins near a preview event
- **THEN** that event is emphasized without changing definitions, source, generation scope, transport or audition

### Requirement: Learn presents a concise truthful waveform preview
Learn SHALL use a shorter width-proportional waveform band with unchanged time mapping and unscaled PCM min/max representation. It SHALL distinguish Detect events from Generate Swells, label the future assignment Default IR and consolidate capture state, duration and event count. Exact samples, coverage and silence classifications SHALL remain in existing Diagnostics rather than primary status. The manual hint SHALL remain visible with detection off.

#### Scenario: Inspect audio and diagnostics
- **WHEN** Learn is resized or inspected during capture, settled preview or detection OFF
- **THEN** waveform amplitude ratios, gaps and musical timing remain accurate, concise status stays readable and existing Diagnostics retains exact technical inspection

#### Scenario: Inspect sparse and dense source
- **WHEN** markers are viewed at full-song scale, resized, zoomed or software-scaled with silence, gaps, unavailable ranges and negative pre-roll
- **THEN** marker coordinates remain absolute and truthful, cursor roles stay distinguishable and provisional/stale markers cannot imply settled generation readiness

#### Scenario: Return from Perform
- **WHEN** the user returns to Learn with accepted source and committed swells
- **THEN** current candidate status appears without duplicate definitions, recapture, DSP work from inspection or implicit generation

### Requirement: Generation outcomes use existing feedback and review gates
Candidate counts and progress SHALL use restrained Learn status text; actionable empty/protected-only/no-change/failure outcomes SHALL use the reserved accessible NoticeBar without layout movement. Revised marker presentation and any new control variant SHALL receive scoped native owner review before rollout. UI standards, component/binding checks and actual host/DPI coverage SHALL be recorded separately.

#### Scenario: Unsuccessful Generate
- **WHEN** Generate cannot publish a changed viable result
- **THEN** Learn remains selected, the preserved-work outcome is explained without claiming success and repeated refresh does not repeatedly announce the same notice

#### Scenario: Native acceptance
- **WHEN** the integrated controls and overlay are ready for owner review
- **THEN** one prepared native preview is launched directly with keyboard, disabled, busy and dense-marker states; any required token exception is reviewed before rollout and untested host/DPI coverage remains explicit

### Requirement: Explicit Swells-list navigation fits the accepted primary
Explicit list selection in Perform SHALL frame the accepted primary using the same pan, zoom and complete-context policy as Fit. Both waveform lanes, target, envelope handles, ruler, toolbar and Overview SHALL share that viewport. Repeating selection of an already-fitted swell SHALL remain visually stable. Selection SHALL follow persistent identity and canonical absolute position.

#### Scenario: Navigate distant swells
- **WHEN** the user clicks an offscreen swell and then another distant swell
- **THEN** each accepted primary immediately receives the same view as explicit Fit, including available source, requested envelope, placed wet and overlap context
- **AND** the view changes zoom as needed to show that complete region with existing Fit margins

#### Scenario: Already visible and repeated selection
- **WHEN** a clicked swell is already visible at a manually chosen pan or zoom
- **THEN** selection still applies contextual Fit to show its complete region
- **AND WHEN** the same selected row is clicked again after panning or zooming away
- **THEN** its complete fitted region returns without requiring a different selection first
- **AND** repeating selection on the already-fitted region causes no visual viewport movement

#### Scenario: Pending rendering and timeline boundaries
- **WHEN** the selected render is pending, or its context includes negative lead-in or reaches known timeline boundaries
- **THEN** automatic framing matches explicit Fit's finite bounded view using available timing and does not wait for rendering or invent waveform content

#### Scenario: Stable identities and multiple selection
- **WHEN** rows reorder or names change before explicit selection, including Ctrl/Shift selection
- **THEN** automatic Fit follows the accepted primary's persistent identity and canonical absolute timing while preserving existing selected-ID and modifier semantics
- **AND** empty or rejected obsolete-session selection does not fit another row's swell

#### Scenario: Navigation has no creative or audio side effects
- **WHEN** list selection automatically fits a swell
- **THEN** only selection and navigation change; captured PCM, timing, shape, enablement, IR/render assets and generation inputs remain unchanged
- **AND** selection does not start playback, seek the host or request rendering

### Requirement: Fixed-grid diagnostics explain the actual availability decision
Diagnostics SHALL identify the actual fixed-grid availability result, rejected timing dimension, accepted mapping, observed mapping, capture rate and sample-position availability. Valid origins and their delta SHALL be shown with enough precision to assess the existing one-sample allowance. Missing inputs SHALL remain unavailable. Grid evidence SHALL be labelled independently from live callback admission and SHALL NOT change Snap, editing or playback policy.

#### Scenario: Playback offset disables the accepted source grid
- **GIVEN** accepted timing at sample 3399344, PPQ 144, 48000 Hz, 122 BPM and 4/4
- **WHEN** grid evaluation uses host sample 3399344 and PPQ 143.999322222222219 with the same rate, tempo and meter under source-only validation (no playing-run context)
- **THEN** diagnostics identifies the approximately 16-sample accepted-to-host origin difference as a grid-origin rejection under the existing policy
- **AND** it does not infer live rejection from that grid result or claim the existing offline fix repairs Snap

#### Scenario: Stop restores grid compatibility
- **WHEN** stopped host timing again matches the accepted source mapping after a grid rejection
- **THEN** current grid evidence reflects that compatibility and labels its evaluation context
- **AND** restored grid availability is not presented as proof of preceding wet audibility or successful live preparation

#### Scenario: Other grid prerequisites fail
- **WHEN** sample position is unavailable, timing is invalid or nonfinite, capture rate is incompatible, tempo or meter differs, or grid conversion cannot be represented
- **THEN** diagnostics identifies the failing prerequisite without inventing an origin or reducing every failure to an origin mismatch
- **AND** the existing Snap On rejection, Ctrl bypass, Snap Off, exact-entry and stale-gesture protections remain unchanged

#### Scenario: UI and callback observations have different provenance
- **WHEN** a grid decision and a live callback or worker observation come from different snapshots
- **THEN** diagnostics labels those contexts separately and exposes the exact inputs used for the grid evaluation without claiming callback-level simultaneity
- **AND** unavailable evidence cannot be substituted with a later stopped observation

#### Scenario: Read diagnostic detail without editing
- **WHEN** grid evidence is refreshed, inspected or the editor is reopened
- **THEN** it uses the existing Diagnostics presentation and accessible full text without changing saved navigation, Snap preference, source/render identities, job counts or host transport
- **AND** the main workspace retains its existing concise unavailable message and supported editing alternatives

### Requirement: Inspector reset actions remain visible
The selected-swell Reset and reset-options controls SHALL occupy a fixed inspector action area outside scrolling or sectioned parameter content. They SHALL remain visible at 900x600 and larger sizes and retain existing labels, reset semantics, availability, keyboard access and selection/session guards.

#### Scenario: Discover Reset without scrolling
- **WHEN** a swell is selected and the user views any inspector section or scroll position at a supported size
- **THEN** Reset and reset options remain visible without scrolling, uncovering another panel or reducing text size
- **AND** empty, unavailable or invalid selections retain honest disabled state without hiding where the actions belong

#### Scenario: Reset while an invocation becomes stale
- **WHEN** a reset-options menu is open and selection, source, mode or session changes before completion
- **THEN** the established ownership guards reject or cancel obsolete operations without resetting a replacement swell
- **AND** ordinary Reset preserves its existing wet-shape scope, while other options keep their existing distinct meanings

### Requirement: Inspector sections disclose every supported edit
The inspector SHALL use persistent labelled Timing, Envelope and Sound section selectors that remain visible and expose the complete supported control set without moving it into Settings. Switching sections SHALL change presentation only and preserve accepted values, identity-bound edits and access to the fixed reset actions.

#### Scenario: Find timing envelope and sound controls
- **WHEN** the user inspects a swell at the compact baseline
- **THEN** each timing/source, envelope and existing sound/IR/level control is visible or has an obvious labelled section entry
- **AND** the same complete capabilities remain reachable with keyboard navigation and at larger sizes, without speculative DSP widgets

### Requirement: Learn separates capture and sound access
Learn SHALL retain its dry canvas and left settings column while exposing Capture & Generate and Sound tabs. Capture policies and sensitivity SHALL stay in the first tab; current Default IR and delivered shared sound settings SHALL occupy Sound. Generate, readiness and manual-placement guidance SHALL remain visible below either tab. Unsupported future DSP widgets SHALL remain absent.

#### Scenario: Inspect defaults and return to capture
- **WHEN** the user switches Learn tabs during allowed or locked workflow states
- **THEN** the same accepted defaults and guards remain authoritative, Generate availability stays truthful, and switching presentation alone does not capture, generate or modify existing swells

### Requirement: Sound editing has consistent context-bound interactions
Learn Sound and Perform Sound SHALL expose the same IR choice, import, accepted/requested feedback and deliberate-selection interactions. Scope SHALL identify future-generation defaults in Learn and the selected swell in Perform. Refresh or switching context SHALL NOT submit edits, inherit settings into existing swells or regenerate audio.

#### Scenario: Edit two independent contexts
- **WHEN** a user changes Learn's accepted default and then opens Perform Sound for an existing swell
- **THEN** Perform retains that swell's assignment and Learn retains the new default
- **AND** changing that swell's IR changes neither the default nor any other swell or instance

#### Scenario: Refresh and incidental input
- **WHEN** accepted state refreshes, Sound tabs switch, catalogue order changes or the pointer wheel crosses an IR selector
- **THEN** no assignment is submitted and selection follows managed content identity rather than row position
- **AND** pending requests remain distinguishable from accepted defaults and retained audible responses

### Requirement: Sound intents retain their original scope
IR commands and asynchronous completions SHALL retain their invocation's session and context ownership. Learn SHALL enforce workflow-pass locks; Perform SHALL enforce selected-swell identity, input revision, mode and freeze guards. Stale, invalid, cancelled or unavailable commands SHALL preserve accepted assignments and retained output rather than edit a replacement target.

#### Scenario: Selection or session changes during a choice
- **WHEN** a menu or file chooser opened for swell A completes after selecting B, deleting A or restoring a session
- **THEN** it addresses the original valid A or is cancelled; it never addresses B or a replacement session
- **AND** freezing A or superseding its input invalidates the obsolete command

#### Scenario: Pass and mode guards apply to every route
- **WHEN** a default request crosses DAW Play, Stop settlement or a workflow-pass change, or a selected-swell request crosses into a disallowing mode
- **THEN** the existing command guards reject obsolete publication even if activation bypasses disabled UI
- **AND** the editor reconciles to accepted values with a meaningful unavailable or failure reason

#### Scenario: Import is pending or fails
- **WHEN** import is cancelled, decoding fails or a newer request supersedes pending work
- **THEN** accepted identities, instance-local retained choices and last-good output follow existing retention rules
- **AND** feedback never claims an unaccepted request is the current audible response

### Requirement: Shared Sound preserves creation and audio ownership
Sound defaults SHALL enter new swells only through existing accepted Generate or manual-creation transactions. Existing automatic, manually edited, manually created and frozen swells SHALL retain their ownership rules. Sharing the editor SHALL NOT change source PCM, saved assignments, render/cache identities, gain calibration or routing, or introduce a migration or blanket regeneration.

#### Scenario: Generate snapshots an accepted default
- **WHEN** explicit or eligible Generate on Stop commits after a default is accepted
- **THEN** eligible automatic definitions use the transaction's accepted default snapshot with existing cancellation and provenance rules
- **AND** manual, edited and frozen definitions remain protected and a later default edit does not live-update committed swells

#### Scenario: Manual creation and subsequent editing
- **WHEN** a user creates a manual swell and later changes the future default or edits an automatic swell's IR
- **THEN** the manual swell retains its creation-time assignment and manual ownership
- **AND** the automatic swell's explicit edit follows the existing manual-edited provenance policy

#### Scenario: Save reopen and unchanged audio
- **WHEN** an existing project opens and both Sound contexts are inspected without an edit
- **THEN** captured bytes, saved IR/render references, exact envelope/Level, durable identities and accepted audio remain unchanged
- **AND** audition, host playback, plugin WAV and offline output retain their existing content and distinct admission policies

### Requirement: Sound access remains readable and limited to delivered processing
Both Sound contexts SHALL retain discoverable labelled access, readable production typography, keyboard focus, accessible scope/status and bottom context help at supported compact and larger sizes/scales. Existing exact-value edits SHALL remain available. Detection SHALL stay Learn-only and final Dry/Wet mixing Export-owned. Unsupported processing controls SHALL remain absent.

#### Scenario: Compact and scaled native review
- **WHEN** Learn Sound and Perform Sound are reviewed at 900x600/100% and supported fitting 125% presentation plus larger sizes
- **THEN** IR choice/import/status, full focus outlines and Perform's existing exact Level edit remain reachable without clipping or hiding Reset
- **AND** long labels, empty selection, unavailable assets, frozen and busy states remain understandable without additional waveform editors

#### Scenario: Review the real control family
- **WHEN** the shared family and integrated composition are ready
- **THEN** a prepared native preview is launched directly for scoped owner look/feel approval before wider rollout
- **AND** component/binding evidence, actual host/DPI coverage and guide currency are recorded separately

#### Scenario: Future processing remains future work
- **WHEN** the convolution foundation is displayed
- **THEN** no Character, Tone, HP, LP, Stretch or Profile placeholder appears and no new Learn Level default is introduced
- **AND** IR identity remains distinct from processing Character and Export Mix Character

### Requirement: Loop editing reuses the shared Perform workspace
Perform SHALL offer the shared multi-swell editing and Audition tools for an accepted Loop. The ruler, Overview, source controls and wet edits SHALL use the accepted phrase's local grid and exact extent. Source selections SHALL stay in bounds and wet placement SHALL retain supported negative offsets.

#### Scenario: Edit two swells around Bar 1
- **WHEN** a musician selects and edits two swells including wet that starts before Bar 1
- **THEN** the list, inspector exact fields, source and wet handles, zoom and Overview stay aligned to the selected swell and accepted phrase
- **AND** the existing IR, envelope, placement, reset, Freeze and audition bindings operate on the correct swell

#### Scenario: Next-capture preference changes
- **WHEN** the next-capture preference is changed or a replacement is pending
- **THEN** existing Perform edits stay bound to the current accepted identity until a useful replacement settles
- **AND** stale gestures and worker results cannot modify or publish against the replacement

#### Scenario: Add a target in Loop Perform
- **WHEN** the musician invokes Add while the Loop playhead is within the captured phrase
- **THEN** the target is added at the displayed phrase position
- **AND** when no in-phrase playhead is available, the action says it adds at the start of the visible loop
- **AND** a source or session change before insertion cancels the action

### Requirement: Loop playback controls report actual readiness

An accepted Loop SHALL show one playback command and separate status beside Audition in Perform. Cue Loop SHALL require valid live timing and coherent prepared wet. Cancel Loop SHALL cancel a queued start. Stop Loop SHALL finish the current pass and suppress the next lead-in. Stop Now SHALL fade wet immediately while dry continues. These commands SHALL act independently of Audition. Song SHALL retain its existing composition.

#### Scenario: Phase, preparation and playback change

- **WHEN** a Loop enters Perform with unavailable live timing, unavailable preparation or disabled live output
- **THEN** Cue is disabled with the specific actionable reason and dry remains available
- **AND** valid live timing and prepared wet enable Cue Loop automatically with Ready status
- **AND** the command becomes Cancel Loop with Waiting for bar status when queued, then Stop Loop with Playing lead-in or Playing loop status when wet starts
- **AND** Stop Loop becomes Stop Now with Finishing loop status until completion or an immediate stop
- **AND** the command is disabled while Fading out and offers Cue Loop again when ready

#### Scenario: Lead-in wraps into the previous pass

- **WHEN** Loop playback repeats with a scheduled lead-in for the next pass
- **THEN** one yellow playhead follows the current phrase and wraps at the observed boundary
- **AND** the wet lane shows a faint waveform of the next pass's prepared wet mix near the end of the phrase, regardless of the selected swell
- **AND** a pending Stop hides the suppressed next lead-in

#### Scenario: Compact review layout

- **WHEN** Song and Loop Perform are viewed at 900×600 at 100% and 125%
- **THEN** the Loop command and status fit on the Audition row with a separator and the command remains keyboard accessible
- **AND** the rest of the shared editor stays usable and Song adds no Loop controls

#### Scenario: Perform reads the current Loop position

- **WHEN** an accepted Loop is in Perform
- **THEN** the existing footer reports its captured tempo and meter
- **AND** during the pass Bar and Beat follow the Loop playhead, returning to Bar 1, Beat 1 each time the Loop repeats, including in Rack without host song position
- **AND** position is unavailable until beat one and after playback ends

#### Scenario: Playback controls explain why they are disabled

- **WHEN** Audition or Cue Loop is unavailable in Perform
- **THEN** the control is disabled and its hover help explains the current prerequisite
- **AND** if the host rate differs from the accepted Loop's rate, both controls report the rate required to play the captured audio

### Requirement: Swells row actions support pointer, keyboard and accessibility
Swells rows SHALL expose named Enabled and More actions for their swell. The Enabled indicator SHALL be square. Keyboard and accessibility focus SHALL use the shared aqua brackets. Row action navigation and activation SHALL preserve the selected identities unless the requested action itself changes them.

#### Scenario: Traverse a long list and compact inspector
- **WHEN** a user navigates the list by keyboard and Tabs through the primary swell's Enabled and More actions
- **THEN** Up/Down retains list selection behavior, Tab reaches those two actions and continues into the inspector, and Return or Space activates the focused action
- **AND** an off-screen inspector field scrolls into view with its focus treatment without editing its value
- **AND** explicit list selection keeps the existing contextual waveform reveal while Tab traversal and background refresh do not replay it

#### Scenario: Invoke a row action by identity
- **WHEN** a user operates the Enabled checkbox or opens the same row menu by its three dots, right-click, keyboard or accessibility
- **THEN** the action names the swell it targets and remains bound to its captured identity across reorder or row recycling
- **AND** pointer focus keeps its native ownership with brackets suppressed
- **AND** presentation and focus changes alone issue no command or selection edit

### Requirement: Swells menu confirms deletion of a captured selection
Delete from a selected row with multiple swells selected SHALL target the entire captured selection and require confirmation. Delete from an unselected row SHALL target only that row. Rename SHALL be unavailable while multiple swells are selected. Deletion SHALL keep the existing session and edit guards and SHALL either remove the confirmed group or leave it intact.

#### Scenario: Confirm or cancel a group delete
- **WHEN** the user opens the menu on a selected row and chooses Delete for multiple swells
- **THEN** the menu and confirmation show the count, the confirmation opens with Cancel focused, and only explicit confirmation removes the captured swells
- **AND** Cancel, Escape and closing the confirmation leave the swells, selection and scroll context intact

#### Scenario: Open the menu outside the selection
- **WHEN** the user opens the menu on an unselected row while other rows are selected
- **THEN** Delete targets only the invoked row and the existing selection is preserved
- **AND** Rename remains disabled while the multiselect is active

#### Scenario: The captured group becomes invalid
- **WHEN** the list reorders or the selection changes while a menu or confirmation is open
- **THEN** the action can only delete the originally captured IDs
- **AND** a missing target, obsolete session, invalid edit context or deletion capacity failure rejects the whole group without mutating any swell

### Requirement: Main waveforms share mouse navigation

The main Learn, Export and Perform waveform plots SHALL zoom at the pointer with
the wheel and pan horizontally with middle-button drag. Navigation SHALL respect
each view's bounds and SHALL NOT move a marker, edit a handle or seek the DAW.
Right-click and right-drag SHALL leave the view and its content unchanged.

#### Scenario: Navigate a zoomed waveform
- **WHEN** the user wheels over a main waveform and then middle-drags it
- **THEN** zoom retains the sample under the pointer where bounds permit, and the waveform follows the drag at the detail scale
- **AND** ruler, markers and any linked overview or zoom controls stay aligned to the clamped view

#### Scenario: Right button and gesture isolation
- **WHEN** the user right-clicks or right-drags, or releases a middle drag and then moves the pointer
- **THEN** the right button opens no menu and changes no viewport, marker or edit, and the released drag stops panning

### Requirement: Perform positions a local playhead and gives handles priority

On the Perform detail waveform, left-click and left-drag SHALL place an ephemeral
local playhead when the press starts on open waveform. A press on a handle SHALL
route to its existing inspection and edit rules. The local playhead SHALL remain
distinct from host transport, audition and editable swell markers.

#### Scenario: Position on open waveform
- **WHEN** the user clicks or drags on open waveform after zooming or panning
- **THEN** the local playhead identifies the mapped absolute sample in the aligned lanes and leaves the DAW, audition and swell values unchanged
- **AND** subsequent pan and zoom preserve the playhead's sample

#### Scenario: Start on a handle
- **WHEN** the user left-drags from a timing or envelope handle
- **THEN** the handle owns the gesture under its existing Snap, modifier, frozen and stale-edit guards
- **AND** a frozen or otherwise guarded handle remains uneditable and the press does not fall through to the playhead

### Requirement: Canvas focus identifies keyboard and accessibility navigation

The main waveform controls SHALL use aqua corner brackets for keyboard or
accessibility focus. A pointer press that takes focus SHALL suppress the brackets
while preserving focus, keyboard commands and independent interaction feedback.

#### Scenario: Switch between pointer and keyboard
- **WHEN** the user positions or edits by pointer and then uses the focused canvas by keyboard
- **THEN** the pointer gesture leaves the brackets hidden and keyboard use restores them without changing the meaning of the existing command

### Requirement: Waveform repaint work follows display resolution
Unchanged waveform layers SHALL be reusable during cursor-only refresh. Detail waveform drawing SHALL restrict work to visible data and display resolution. Full-song overview drawing SHALL use display-resolution summaries while preserving actual extrema, sparse coverage and swell positions. Hidden inline overview content SHALL cause no waveform drawing.

#### Scenario: Long song with hundreds of prepared swells
- **GIVEN** a five-minute capture with 200 prepared swell summaries at the supported editor size
- **WHEN** the user views the full song or zooms and scrolls through it
- **THEN** waveform drawing is bounded by displayed summaries instead of repeatedly drawing every subpixel peak
- **AND** audio, captured silence, unmapped and unavailable coverage remain distinct and no control or host position changes merely from navigation

#### Scenario: Cursor-only refresh
- **WHEN** transport or audition cursor position changes without waveform, viewport, size or display-setting changes
- **THEN** unchanged waveform layers are reused while cursors, selection and handles remain current

### Requirement: Display summaries follow accepted edits and data revisions
Display summaries SHALL reflect accepted source, render, shape, gain, placement, availability, viewport, size and scaling changes without showing another selection's data. Summarisation SHALL preserve exact model coordinates and SHALL NOT change audio, queue convolution, write assets or alter saved state. Interaction geometry SHALL remain derived from exact coordinates.

#### Scenario: Edit while a new render arrives
- **WHEN** a selected swell is shaped or moved while a background render or source revision is published
- **THEN** the next presented summary reflects the accepted identities and settings, with matching inspector values and handles and no stale waveform attributed to another swell

#### Scenario: Narrow impulse and sparse gap
- **WHEN** one-sample extrema, negative lead-in and an unmapped gap are viewed at full-song and close zoom
- **THEN** real extrema remain represented within their pixel columns, the gap contains no invented waveform and timing/selection hit geometry remains sample-derived

### Requirement: Waveform performance claims have matched native evidence
Waveform optimisation acceptance SHALL include matched native baseline and candidate paint distributions, visual/component and binding checks, and an editor-refresh measurement. Native software fixture results SHALL remain distinct from interactive host/DPI acceptance. Unperformed checks SHALL remain explicitly open.

#### Scenario: Review measured improvement
- **WHEN** the waveform change is presented for owner review
- **THEN** the audit's 3/5/20-minute and 10/100/200-swell scenarios report p50, p95 and maximum paint/refresh times on the same machine, build and dimensions
- **AND** cursor-only summary reuse and faithful zoom/scroll/edit invalidation are demonstrated with automated checks before one consolidated host smoke test

### Requirement: IR replacement feedback reflects the accepted sound and effective shape

The Perform workspace SHALL distinguish a pending IR request from the accepted IR and present the accepted effective waveform, handles and inspector values together. A successful automatic fit SHALL be indicated as status with access to the reason for the adjustment.

#### Scenario: Pending then fitted
- **WHEN** a shorter valid response is requested for a swell with eligible accepted wet audio
- **THEN** status identifies the request and the retained response during preparation
- **AND** after acceptance the selector, waveform, handles and inspector agree with the effective shape, and status explains that timing was fitted to the response

#### Scenario: Superseded request
- **WHEN** an older IR request completes or fails after a newer selection supersedes it
- **THEN** the older result leaves current feedback and controls untouched

### Requirement: IR replacement errors explain the cause and recovery

If an IR replacement cannot be prepared, the workspace SHALL identify the requested IR and the specific failure in plain language, identify whether the previous sound remains available, and give a feasible next action where one exists. A fit that succeeds SHALL clear any obsolete error.

#### Scenario: Invalid or unreadable response
- **WHEN** the chosen file is missing, unreadable, corrupt or uses an unsupported format
- **THEN** the feedback identifies the applicable file problem and directs the user to find a readable copy, re-export to a supported format or choose another response as applicable

#### Scenario: Source or resource limit
- **WHEN** valid preparation fails because verified source coverage is unavailable or the render exceeds the working budget
- **THEN** the feedback explains the actual limit and gives the appropriate recovery, such as restoring or recapturing missing source or choosing a shorter response
- **AND** the workspace reports retained audio only where it remains available
