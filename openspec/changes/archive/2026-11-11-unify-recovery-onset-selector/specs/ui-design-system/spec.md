# Spec Delta

## ADDED Requirements

### Requirement: Recovery onset suggestions use deliberate sample selection

The Settings Recovery Onset suggestions field SHALL follow the shared selector presentation and interaction contract. An eligible selection SHALL choose the corresponding sample for the existing Confirm action. Selection and inspection SHALL leave onset confirmation under the existing command.

#### Scenario: Inspect, commit or dismiss onset suggestions
- **WHEN** the user opens the selector and inspects the suggestions by pointer or keyboard
- **THEN** shared quiet hover, keyboard inspection, accepted checkmark and focus return apply
- **AND** an explicit eligible commit chooses one sample, while Escape, Tab and wheel input do not change it
- **AND** Confirm remains a separate action

#### Scenario: Refresh or pick from the source lane
- **WHEN** the suggestion list refreshes or the source lane selects a sample
- **THEN** the selector silently reflects the current sample, preserving it while it remains available
- **AND** when the current sample disappears, the existing first-visible fallback applies or the field shows its readable empty guidance and Confirm is unavailable

#### Scenario: An open suggestion becomes stale
- **WHEN** an open popup outlives a change to its sample mapping, accepted choice or captured source context
- **THEN** a stale completion cannot choose a replacement sample or confirm an onset
- **AND** unchanged refresh leaves inspection available without publishing another choice
