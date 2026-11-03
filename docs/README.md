# ReverbArise documentation

This directory covers product identity, the existing implementation, development guidance and historical records.

| Location | Purpose | Maintenance |
| --- | --- | --- |
| [Product Vision](product-vision.md) | Concise target product identity and creative workflow | Replace for major direction changes; archive the previous edition |
| [Development](development/) | Build, DSP, capture, persistence and transport technical guides | Update in place; Git records revisions |
| [Design](design/) | UI system, current implemented layouts and diagnostics guidance | Update in place as work is implemented |
| [Validation](validation/) | Dated native and FL Studio test evidence | Preserve test scope and actual results |
| [Archive](archive/) | Superseded visions, handoffs and early visual concepts | Historical context only |

## Recent engineering references

- [Local Windows releases](development/releases.md) — validated ZIPs, immutable outputs, manual publication and redistribution gate.

- [Product version and release identity](development/product-version.md) — CMake authority, exact tag checks, candidate labels and schema/host compatibility.

- [Development setup and local validation](development/development.md) — Windows prerequisites, Debug/Release builds, matching CTest and build-directory locking.
- [UI development standards](development/ui-development-standards.md) — shared controls, incremental consolidation, interaction tests and visual checkpoints for UI work.
- [Bundled IR library](development/ir-library.md) and [musical gain staging](development/musical-gain-staging.md) — living technical references.
- [IR library evidence](validation/ir-library-evidence.md) and [gain-staging evidence](validation/gain-staging-evidence.md) — recorded validation and measurement evidence. Related JSON reports are kept alongside the IR evidence.

## Documentation policy

- **GitHub Issues and approved issue mockups** describe the future roadmap and requirements.
- **Canonical OpenSpec specifications** describe the system as currently implemented; OpenSpec changes evolve them as features are built.
- **Living technical and agent guidance** is edited in place and kept current. Git history is the safety net.
- **Major superseded product visions and design proposals** are archived as historical snapshots, not rewritten to match a later vision.
- **Validation reports** preserve what was actually tested at the time, even if product behaviour subsequently changes.
- A difference between approved future design and current implementation documentation is intentional, not necessarily a contradiction.

Detailed feature acceptance criteria belong in their GitHub issues, not this documentation index or the vision.
