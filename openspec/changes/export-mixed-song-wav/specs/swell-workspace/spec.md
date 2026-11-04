# swell-workspace Spec Delta

## MODIFIED Requirements

### Requirement: Workspace reflects actual processing and selection
The workspace SHALL adapt to mode: Learn shows capture settings and one full-song dry timeline; Perform retains its eight-region editor; Export shows its dedicated preview and mix/file controls. Processing and selection SHALL remain identity-bound, truthful and mode-appropriate. A requested render or mode SHALL NOT be shown as already ready.

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

## ADDED Requirements

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
Export SHALL draw actual accepted dry/wet data with a shared disclosed amplitude scale and the composition gain plan. Dry SHALL use neutral white/grey and wet aqua; zero-weight layers SHALL be invisible. Actual wet pre-arrival rise, placement, crop and post-arrival tail SHALL be preserved. Ruler, preview cursor and coverage indications SHALL remain truthful and visually distinguishable.

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
Unmodified left-click/drag SHALL position the ready internal preview using the
same absolute mapping as waveform/ruler/cursor. Stopped clicks SHALL not autoplay;
playing seeks SHALL resume after bounded preparation. Shift-drag SHALL retain
panning, right-click SHALL not seek, and pending previous plots SHALL not seek.
Position SHALL be ephemeral and SHALL not change DAW transport or export audio.

#### Scenario: Positioning while zoomed or panned
- **WHEN** a ready waveform is clicked or dragged after viewport navigation
- **THEN** cursor/readout identify the mapped clamped sample and Play starts there without changing view, mix or WAV alignment
- **AND** Shift-drag still pans without changing preview position
