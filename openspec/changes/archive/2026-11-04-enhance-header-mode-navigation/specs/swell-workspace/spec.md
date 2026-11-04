# Spec Delta

## MODIFIED Requirements

### Requirement: Shared header preserves approved composition
All modes SHALL share the approved wordmark, italic tagline, Semibold mode labels, regular mode description with an aqua accent, and Settings action. Logo, tagline and controls SHALL retain their shared vertical center and existing dimensions. The description and contextual-message rows SHALL fit without clipping or overlap. Routine Perform status SHALL remain absent; important notices SHALL use the conditional inline bar. Descriptions SHALL NOT imply deferred WAV delivery exists.

#### Scenario: Minimum and larger editor sizes
- **WHEN** empty, ready, recording, settling and failure states are displayed at 1100x850 and supported larger logical sizes
- **THEN** all segments, Settings, brand and concise status remain readable and operable, with existing workspace controls reachable and no redundant mode dropdown

#### Scenario: Export remains a foundation workspace
- **WHEN** Export is selected before its dedicated delivery capability is implemented
- **THEN** the header reports dry DAW passthrough and explicitly initiated supported preview honestly, with no false ready-WAV or drag/export action

## ADDED Requirements

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
