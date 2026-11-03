# local-windows-validation Specification

## Purpose

Provide developers and local agents with repeatable Windows builds and trustworthy local test outcomes while preserving incremental build state and avoiding hosted runner costs.

## Requirements

### Requirement: Local configuration selection
The repository SHALL provide `scripts/build.ps1 -Configuration Debug|Release`, defaulting to Debug, with optional `-Test`. Both configurations SHALL preserve the existing Windows x64 MSVC generator, CMake settings and dependency pins. Execution SHALL require no hosted CI, remote build service or paid runner.

#### Scenario: Build either configuration
- **WHEN** the command runs with Debug or Release on the supported local toolchain
- **THEN** it configures as needed and builds the selected configuration, including the VST3 and enabled test targets
- **AND** the existing Debug preset remains available and corresponding Release build and test presets are available

#### Scenario: Default configuration
- **WHEN** the command runs without a configuration argument
- **THEN** it selects Debug

### Requirement: Matching test execution
With `-Test`, the command SHALL run the registered CTest suite for the selected configuration only after a successful build. Without `-Test`, it SHALL report testing as not requested. An empty test suite SHALL fail validation.

#### Scenario: Release validation
- **WHEN** a Release build succeeds and `-Test` is supplied
- **THEN** CTest executes the Release test binaries and the command reports their actual result

#### Scenario: Debug validation
- **WHEN** a Debug build succeeds and `-Test` is supplied
- **THEN** CTest executes the Debug test binaries and the command reports their actual result

#### Scenario: Build without tests
- **WHEN** a build succeeds without `-Test`
- **THEN** no CTest run occurs and the summary does not claim tests passed

#### Scenario: No tests discovered
- **WHEN** requested validation discovers zero tests
- **THEN** the command exits nonzero and does not report validation success

### Requirement: Reliable failure and outcome reporting
Invalid input, unavailable prerequisites, configure failure, build failure and test failure SHALL return nonzero process exit status. Later stages SHALL stop after failure. Valid invocations SHALL end with a concise summary identifying configuration, Git revision or explicit unavailability, stage outcomes and diagnostic log locations. Useful failure output SHALL remain accessible without routinely printing full successful build logs.

#### Scenario: Invalid configuration
- **WHEN** a configuration other than Debug or Release is supplied
- **THEN** argument validation exits nonzero before configure or build

#### Scenario: Missing local tools
- **WHEN** the required Windows toolchain or compatible CMake/CTest cannot be found
- **THEN** execution exits nonzero with an actionable prerequisite diagnostic and does not claim build or test success

#### Scenario: Native stage fails
- **WHEN** configure, build or requested tests fail
- **THEN** the command exits nonzero, identifies the failed stage and preserves useful error output
- **AND** stages following that failure are reported as not run

### Requirement: Location independence and safe paths
The command SHALL resolve the repository from its own location, safely pass Windows paths containing spaces and restore the caller's working directory on success or failure.

#### Scenario: Invocation outside the checkout
- **WHEN** a caller invokes the script by absolute path from another directory and the checkout path contains spaces
- **THEN** configure, build and tests use that checkout and the caller's directory is restored

### Requirement: Incremental state preservation
Repeated invocations and configuration switches SHALL reuse the existing shared build tree and available dependency sources. Default execution SHALL NOT delete caches, clean build outputs, force dependency updates or require a fresh build directory.

#### Scenario: Repeat build and switch configuration
- **WHEN** a developer repeats a build or switches from Debug to Release in a compatible configured checkout
- **THEN** existing dependency downloads and configuration state are reused, with only necessary incremental compilation and configuration work

#### Scenario: Incompatible existing cache
- **WHEN** a cache belongs to a different checkout or incompatible generator
- **THEN** execution fails with recovery guidance instead of silently deleting or replacing it

#### Scenario: Incomplete existing cache
- **WHEN** an existing cache lacks source directory, generator or platform identity
- **THEN** preflight fails with a diagnostic naming the missing entries before configure, build or tests run
- **AND** the cache remains unchanged and no automatic repair occurs

### Requirement: Reproducible documentation and evidence
Living development guidance SHALL describe prerequisites, both configurations, optional testing, output locations and failure behavior, with a short AGENTS.md pointer. Validation evidence SHALL identify commands, environment, revision and actual outcomes, distinguishing unavailable checks from passes and automated CTest from manual DAW acceptance.

#### Scenario: Supported local verification
- **WHEN** the implementation is validated locally
- **THEN** dated evidence records actual Debug and Release commands, CTest outcomes, failure-path checks and incremental reuse observations

#### Scenario: Environment cannot run MSVC
- **WHEN** a cloud or other environment lacks the supported Windows toolchain
- **THEN** native build and test validation is explicitly reported as not run, with the missing prerequisite recorded

### Requirement: Build directory exclusion
The local command SHALL hold an exclusive lock for its selected CMake binary directory across configure, build and requested tests. A simultaneous invocation targeting the same directory SHALL fail clearly before those stages. Independent worktrees with separate binary directories SHALL remain able to build concurrently.

#### Scenario: Same tree contention
- **WHEN** another invocation owns the binary directory lock, even for a different configuration
- **THEN** the command exits nonzero with a busy-directory diagnostic and does not configure, build or test that tree

#### Scenario: Independent tree and lock release
- **WHEN** an invocation uses a different binary directory while the first is locked
- **THEN** it can proceed independently
- **AND** each invocation releases its own lock on success or failure
