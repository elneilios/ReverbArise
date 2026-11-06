# Spec Delta

## MODIFIED Requirements

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

### Requirement: Learn offers one explicit Generate action
Generate Swells SHALL be the primary Automatic Swells action below Marker Sensitivity and supported conditional parameters, or in a persistent action area below those settings at compact heights. Its labelled action and readiness helper SHALL remain directly discoverable when settings overflow. It SHALL reuse ActionButton, retain its label across runs and expose accessible readiness/progress reasons. Auto-generate on Stop SHALL reuse Switch below Enable automatic swells, before IR controls. The contextual readiness helper SHALL sit directly below Generate.

#### Scenario: Ready or disabled action
- **WHEN** Learn is ready, capturing, settling, analysing, generating or missing usable source/IR
- **THEN** Generate reflects canonical readiness, keyboard/pointer/secondary routes share the same guards and disabled reasons remain readable without a rejected click

#### Scenario: Cancel a waiting automatic request
- **WHEN** the user disables Auto-generate on Stop while its request awaits final analysis or preparation
- **THEN** the not-yet-committed request is cancelled and no lock hides the cancellation path

## ADDED Requirements

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
