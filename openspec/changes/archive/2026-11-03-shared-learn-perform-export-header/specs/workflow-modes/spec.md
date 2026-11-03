## ADDED Requirements

### Requirement: Creative mode names are separate from durable identity
User-facing mode names SHALL be Learn, Perform and Export for the existing capture, live and finalise behaviors respectively. Visible mode guidance, rejection reasons and accessible text SHALL use those names consistently. Saved identifiers, state versions, plugin identities and mode/audio semantics SHALL remain compatible. Functional concepts such as source capture, Recapture and live wet SHALL retain their accurate meaning.

#### Scenario: Reopen saved workflow modes
- **WHEN** supported projects saved with Capture, Live or Finalise identifiers reopen
- **THEN** their corresponding Learn, Perform or Export mode is displayed with the same source, swells, edits, readiness rules and output behavior, without a state migration or implicit recording/audition
- **AND** projects lacking saved mode metadata still enter Learn with Recapture off under the existing legacy policy

#### Scenario: Save and restore a newly labelled mode
- **WHEN** a valid mode is selected through the new navigation and the host saves/reopens the project
- **THEN** the original durable mode representation round-trips, the chosen display label returns, and a real mode change retains existing host save notification behavior

#### Scenario: Automatic transition while renders prepare
- **WHEN** useful accepted source or the approved explicit replacement qualifies for automatic Live entry under the existing Stop policy
- **THEN** Perform becomes selected after source acceptance without waiting for wet jobs, with honest background preparation status
- **AND** mapped-only, fresh all-silent, failed and erased-last-useful-source outcomes retain their existing Learn behavior

#### Scenario: Existing eligibility and routing
- **WHEN** a user requests Perform or Export with silent-only, incompatible, frozen-recovery or ready-render material
- **THEN** the existing eligibility rules decide the request without being weakened by presentation changes
- **AND** Learn captures only under its existing policy, Perform schedules wet without capture, and Export leaves DAW output dry with only explicitly initiated supported preview
