# Spec Delta

## ADDED Requirements

### Requirement: C3 field feedback wraps within its associated input area
Fields adopting the C3 composition SHALL place captions above their inputs and use a single slot for small help or validation text directly below. The slot SHALL align with and wrap within the visible input body, or the combined span of the input bodies for a group, excluding adjacent actions. The layout SHALL make room for all rendered lines at the approved type size.

#### Scenario: A long message appears under a narrow field
- **WHEN** help or validation exceeds the input width at a supported size or user scale
- **THEN** it wraps to multiple lines within the input width without shrinking, clipping or overflowing into adjacent content
- **AND** the full message is available in the input's accessible description

#### Scenario: Feedback for a pair uses the input span
- **WHEN** Settings shows help or validation for Source range (bars)
- **THEN** Bar start and End excl. each have a caption above their input and share one message beneath the pair
- **AND** the message starts at the first input's left edge and wraps by the last input's right edge, excluding Set source from bars
- **AND** content below the pair reflows as the number of lines changes without overlap or loss of keyboard access

### Requirement: Source range feedback reflects the existing binding outcome
The source-range pair SHALL display an applicable rejection from its existing command in place of help, with red text and a red body border on the implicated input or inputs. Keyboard and accessibility focus brackets SHALL stay aqua and independent of the error. Presentation SHALL preserve the pair's existing input, eligibility and commit behavior and SHALL NOT add validation rules.

#### Scenario: Existing source-range command rejects a submitted pair
- **WHEN** the user submits a pair through Return or Set source from bars and the existing command rejects it
- **THEN** the local message explains the rejection, with an accessible association to the relevant inputs and a red border where the rejection applies
- **AND** a rejected range does not change the accepted source range
- **AND** a pointer click alone does not add aqua brackets

#### Scenario: A correction is accepted or the affected range context changes
- **WHEN** the binding accepts a corrected pair or the affected selection or session is replaced
- **THEN** the previous local error and red border clear, the current guidance is displayed and the accessible description no longer contains the stale error
- **AND** the change in feedback does not trigger another edit or a forced announcement

#### Scenario: The current source range cannot be applied
- **WHEN** the existing Settings action is unavailable, such as for a frozen swell
- **THEN** the explanation remains readable and associated with the pair while the action retains its existing unavailable behavior
- **AND** unavailability alone does not produce a red validation state
