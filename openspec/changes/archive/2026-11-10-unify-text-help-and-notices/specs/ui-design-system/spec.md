# Spec Delta

## ADDED Requirements

### Requirement: Shared text stays readable within its allocated region

Captions, explanations, empty states, status and diagnostic text SHALL use the
approved semantic text roles throughout the plugin and its owned windows.
Multiline prose SHALL wrap within its region and keep later content reachable.
Single-line labels SHALL ellipsize with full accessible text. Overflow SHALL
retain the role's font size, subject to the existing bounded notice policy.

#### Scenario: Long text in a compact or scaled view
- **WHEN** long help, an explanation, an empty-state message or diagnostic detail is displayed at a supported size or 100%/125% user scale
- **THEN** text wraps within its allocated width and following controls remain accessible through layout or scrolling
- **AND** an unbroken filename or path fits through wrapping or the region's declared ellipsis policy without crossing into neighbouring content

#### Scenario: Inspect a shortened value
- **WHEN** a long name, path or build value exceeds a single-line label
- **THEN** an ellipsis identifies the shortened display and the complete value is available through the accessible description and existing contextual or copy access
- **AND** refreshing the same value does not publish an edit or repeatedly announce it

### Requirement: Footer context help uses the approved aqua information colour

The shared hover-help footer SHALL render its contextual message in softer aqua
at the approved helper text size. It SHALL expose the complete message through
its accessible description even when the visible line ellipsizes. Hovering a
nested control SHALL use the nearest applicable contextual description.

#### Scenario: Hover a control or nested child
- **WHEN** the pointer moves over a control with help or its nested child in the editor, Settings or Diagnostics
- **THEN** the footer shows the applicable message in the approved softer aqua colour #00C2CC
- **AND** ordinary screen captions and permanent helper prose retain their own semantic text roles

#### Scenario: Change or clear the help source
- **WHEN** the pointer leaves its applicable region or the owning component hides, changes context or is destroyed
- **THEN** footer text and accessible description reconcile together without retaining obsolete help
- **AND** hover changes leave focus and accepted values under their existing owners

### Requirement: Persistent messages keep their local and workspace meaning

Persistent feedback SHALL remain discoverable in its owning field, group,
status region or notice. A field error SHALL take precedence over its routine
help with the existing red message and affected border. Workspace notices
SHALL retain their established severity and precedence. Footer help SHALL
supplement the persistent message.

#### Scenario: Hover elsewhere while a field has an error
- **WHEN** a rejected field edit has an active local explanation and the pointer moves to another control
- **THEN** the red local explanation remains visible and accessible while aqua footer help follows the newly hovered control
- **AND** an accepted correction or context replacement clears the obsolete local error under the existing binding rules

#### Scenario: Unavailable or empty state
- **WHEN** a control is unavailable, content is empty or a persistent operation status changes
- **THEN** its applicable explanation remains readable and accessible with the accepted state
- **AND** unavailability alone creates no validation error, and unchanged refresh produces no redundant notice event or forced speech
