# Spec Delta

## MODIFIED Requirements

### Requirement: Density preserves invariant visual and interaction roles
Semantic colour, legibility, focus and control interaction language SHALL remain consistent across density profiles. Typography SHALL use the owner-approved responsive compact/expanded roles, rather than fixed heights across all sizes. The approved Learn spacing SHALL be documented as comfortable. Compact geometry SHALL require scoped native approval across its affected Learn, Perform, Export and header consumers before production rollout; it SHALL NOT be achieved by arbitrary text reduction or unreviewed token values. The 900x600 baseline SHALL work at 100% UI scale, independently of optional reviewed proportional scaling.

#### Scenario: Prepare Perform work
- **WHEN** a contributor plans a denser three-column Perform workspace
- **THEN** the plan reuses invariant roles and proposes spacing changes through a named profile review, without treating current legacy spacing or unvalidated compact numbers as approved

#### Scenario: Adopt a compact profile
- **WHEN** a compact proposal is assessed at supported sizes with representative dense content
- **THEN** approval covers readability, overflow, waveform room, hit targets, keyboard and focus before its values enter production, and changes to comfortable consumers are limited to their explicitly reviewed responsive composition using the explicitly approved responsive text roles while preserving focus and interaction policies

#### Scenario: Review the compact baseline across modes
- **WHEN** 900x600 composition and optional proportional scales are presented for acceptance
- **THEN** 100% UI scale independently passes readability, useful waveform space, hit-target, keyboard and overflow review in all three modes
- **AND** any compact header treatment, inspector section family or new metric receives named scoped approval before production adoption

### Requirement: Component and native evidence establish practical UI confidence
Automated checks SHALL cover resolved typography, clipping, focus containment, layout/overflow and keyboard/edit interaction for affected patterns. Reusable native fixtures SHALL support visual review at the 900x600 baseline at 100% UI scale, intermediate sizes, responsive breakpoints, 1100x850 and documented larger sizes. Evidence SHALL separate automated proof, native owner approval, actual Windows DPI and FL Studio results, recording untested coverage honestly.

#### Scenario: Review supported geometry and scale
- **WHEN** Learn/header and shared patterns are validated
- **THEN** results include normal/focused/disabled/error and long-content cases across supported sizes and software scales, while Windows 100/125/150/200% checks are separately passed, failed or untested

#### Scenario: Approve a material family revision
- **WHEN** a new or materially revised control family is ready for rollout
- **THEN** its prepared native preview is opened for owner look and interaction review before further consumers adopt it; passing automated cases and prior approvals are not repeated without a concrete reason

#### Scenario: Primary 1080p acceptance and complete control access
- **WHEN** compact implementation is validated on the owner's 1920x1080 display
- **THEN** actual Windows scaling and FL Studio work-area fit are recorded, Reset remains visible without scrolling and every supported control has demonstrated mouse/keyboard access
- **AND** separate results cover Windows 100/125/150% where available, with 200% and any non-fitting combinations explicitly passed, failed, unsupported or untested rather than inferred from software screenshots
