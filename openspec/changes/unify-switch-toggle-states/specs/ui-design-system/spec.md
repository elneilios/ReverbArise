# Spec Delta

## ADDED Requirements

### Requirement: Boolean controls use presentation suited to their layout
Learn capture options and visible Settings on/off controls SHALL use labeled switches where the layout has room. The Swell Inspector SHALL use inline checkboxes for Enabled, Solo and Freeze. Each control SHALL display its accepted state with a thumb position or checkmark in addition to colour.

#### Scenario: Review spacious settings and compact inspector flags
- **WHEN** a user views Learn, Settings or the Swell Inspector at a supported editor size and scale
- **THEN** Learn and Settings switches keep their labels close to the track, and Enabled, Solo and Freeze stay together on one inspector row
- **AND** labels, hit targets and focus decoration remain readable, contained and independently operable
- **AND** choosing the checkbox presentation for an inspector flag preserves its immediate effect

### Requirement: Switches and checkboxes share independent pointer and focus states
Both forms SHALL distinguish quiet hover and held feedback on the indicator surface. Keyboard and accessibility focus SHALL show thin aqua corner brackets with a small gap around the indicator. A pointer click SHALL hide the brackets while retaining actual focus and keyboard eligibility.

#### Scenario: Hover and press an enabled control
- **WHEN** the user hovers, holds or releases the pointer on a switch or checkbox
- **THEN** the indicator's border and background show distinct hover and held states without obscuring its accepted value
- **AND** the label and indicator activate the same control, and cancelling the press before activation leaves the value unchanged
- **AND** a pointer release can activate the control only after an eligible primary press from the same pointer

#### Scenario: Change focus input method
- **WHEN** the user tabs to a control or explicitly focuses it through accessibility
- **THEN** the brackets appear independently of pointer feedback and without changing the value
- **AND** pointer movement alone leaves those brackets visible
- **WHEN** the user clicks the control, including when it already has focus
- **THEN** the pointer feedback remains available and the brackets disappear

#### Scenario: An on or off control becomes unavailable
- **WHEN** an on/off control is unavailable, including Isolate dry source during wet-only audition
- **THEN** its disabled presentation keeps its accepted state readable and exposes the binding's reason through the applicable help and accessible description
- **AND** hover or press does not make it appear eligible and activation does not change its value

### Requirement: Boolean activation preserves binding authority
Migrated switches and checkboxes SHALL expose a stable accessible name and state. Eligible pointer, Space and accessibility activation SHALL issue at most one existing guarded request. Existing Return behavior SHALL be preserved per binding. Programmatic refresh SHALL silently display accepted state. Disabled, stale or destroyed contexts SHALL not mutate a model.

#### Scenario: A Learn or Settings invocation outlives its capture pass
- **WHEN** the workflow or capture pass changes between invocation and delivery
- **THEN** the existing guard rejects the stale request, and the displayed value reconciles to accepted state without issuing another request
- **AND** a disabled pre-invocation hook that is required by the binding does not make the request eligible

#### Scenario: A selected swell changes while a flag is being activated
- **WHEN** a queued Enabled, Solo or Freeze activation loses its selected swell or session
- **THEN** it cannot change a replacement swell
- **AND** valid flag changes retain their existing selection, audition and render behavior
