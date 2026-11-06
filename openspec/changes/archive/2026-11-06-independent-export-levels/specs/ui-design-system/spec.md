# Spec Delta

## ADDED Requirements

### Requirement: Shared notices do not force speech
Shared notices SHALL expose visible severity and accessible title, description and help text, with standard accessibility change events. They SHALL NOT explicitly invoke text-to-speech or post forced spoken announcements for notice changes, regardless of whether an automation or accessibility client is active. This SHALL NOT disable accessibility or change operating-system settings.

#### Scenario: Export readiness returns to routine guidance
- **WHEN** Export changes between preparation, warning and the DAW wet-bypass information notice
- **THEN** the visible and accessible notice updates without the plugin synthesizing speech
- **AND** assistive software can still obtain the notice through standard accessibility interfaces

#### Scenario: Unchanged and hidden notice refresh
- **WHEN** the same notice is refreshed, hidden or shown again
- **THEN** no forced speech occurs and unchanged refresh does not create redundant notice-change events
- **AND** cleared notices expose no stale title or description