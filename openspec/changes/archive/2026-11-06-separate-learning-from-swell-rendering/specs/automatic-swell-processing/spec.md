# Spec Delta

## MODIFIED Requirements

### Requirement: Legacy manual processing remains available
Perform SHALL retain manual editing and explicit regeneration commands. Learn detection and marker-application controls SHALL govern new or reconciled marker definitions only. Existing unfrozen swells SHALL maintain current verified renders independently of those controls and legacy refresh preferences. No control SHALL silently enable detection or source replacement.

#### Scenario: Manual mode with refresh disabled
- **WHEN** changed source reaches a Perform swell while automatic processing and legacy auto-refresh are disabled
- **THEN** its changed material dependency queues bounded render maintenance while identities, edits and saved detection/legacy preferences remain unchanged

#### Scenario: Automatic mode takes responsibility for refresh
- **WHEN** accepted source changes with legacy manual auto-refresh off
- **THEN** affected eligible swells refresh through the automatic pipeline while unrelated swells remain unchanged

#### Scenario: Diagnostic suggestions remain distinct
- **WHEN** the user requests manual onset suggestions in an allowed mode
- **THEN** unconfirmed suggestions remain separate from accepted targets and cannot independently own active swells

#### Scenario: Dry-only Recapture does not force generation
- **WHEN** Learn commits changed source with Automatic Swells off and a legacy refresh/forced-work preference was enabled
- **THEN** source commits normally, no new marker analysis/application is admitted, and affected existing unfrozen swells reconcile through the common render pipeline
- **AND** saved definitions/edits and valid frozen results remain preserved for permitted later manual use

### Requirement: Automatic Swells off provides dry-only learning
Learn with detection off SHALL capture source under independent Recapture rules without detecting or applying new markers. Accepted source changes SHALL still trigger material-dependency maintenance for existing unfrozen swells. Detection changes SHALL fence obsolete marker work without cancelling valid existing-swell render work. Saved definitions, edits and assets SHALL survive.

#### Scenario: All four capture-policy combinations
- **WHEN** Recapture and Automatic Swells independently take each on/off combination
- **THEN** only Recapture grants mapped replacement; detection controls permit candidate analysis and marker application, and Auto-generate on Stop permits automatic marker application after useful Stop
- **AND** useful accepted dry can enable Perform in every combination without waiting for wet renders; existing unfrozen swells reconcile changed dependencies independently in all combinations

#### Scenario: Turn automatic generation off with older work pending
- **WHEN** the idle Learn user disables Automatic Swells before another pass while automatic work remains pending
- **THEN** no new detection or marker-application work is admitted under Off; obsolete marker publication is fenced while valid existing-swell maintenance remains eligible and definitions/assets are preserved

#### Scenario: Manual work after dry-only learning
- **WHEN** dry-only capture has committed and the user explicitly creates/edits a swell in Perform
- **THEN** normal permitted manual preparation works without silently enabling automatic source replacement or automatic discovery

#### Scenario: Learn preferences target future passes
- **WHEN** the user changes Learn defaults after a dry-only pass
- **THEN** future detection/default settings do not implicitly reassign existing IRs or commit marker definitions; current Learn settings can preview accepted coverage and explicit Generate can apply markers without recapture, independently of render maintenance

### Requirement: Explicit manual preparation is independent of automatic policy
Manual creation and render-affecting edits in Perform SHALL feed the same existing-swell dependency pipeline as accepted source and IR changes. Work SHALL remain independent of marker controls and legacy refresh preferences. Source, identity, edits, immutable assets and latest-request publication guards SHALL be preserved.

#### Scenario: Manual creation after dry-only capture
- **WHEN** short non-silent captured audio is committed with automatic processing and markers off and the user adds a manual target with complete source and a valid accepted IR
- **THEN** the target progresses through preparation to a verified Ready render and enables user-requested audible Perform Audition without enabling detection or starting playback automatically

#### Scenario: Edit manual dependencies
- **WHEN** the user accepts an arrival, source-interval or IR edit with all automatic policies off
- **THEN** only the affected eligible swell prepares against its latest accepted dependencies, preserves its identity and rejects obsolete completions
- **AND** shape-only, gain and placement changes retain their existing minimum-work behavior

#### Scenario: First snapped timeline insertion
- **WHEN** dry-only capture is accepted, no swell has yet pinned session timing, and the user double-clicks covered dry audio in Perform with Snap enabled
- **THEN** valid current fixed host timing and matching capture rate permit creation at the snapped arrival without a prior '+' insertion
- **AND** missing host position, invalid timing or incompatible established timing still blocks snapped insertion

#### Scenario: Capture alone remains dry-only
- **WHEN** dry-only Learn capture commits or Perform is entered without an explicit manual request
- **THEN** those events do not authorize marker discovery/application or change saved detection preferences; existing unfrozen swells still refresh only when verified material dependencies require it

#### Scenario: Imported WavPack IR
- **WHEN** a valid imported .wv IR is accepted as the manual target's assigned or future-default IR with sufficient source
- **THEN** manual preparation verifies and uses that immutable IR, yielding a usable render without substitution
