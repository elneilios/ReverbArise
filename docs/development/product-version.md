# Product version and release identity

The only authoritative numeric product version is the literal
`project(Preverb VERSION 0.1.0 LANGUAGES C CXX)` declaration in root CMakeLists.txt.
CMake passes PROJECT_VERSION explicitly to JUCE. Generated definitions, Windows
resources, VST3 module/class versions and `out/build/windows-msvc/product-identity.json`
are derived outputs, not files to edit when bumping a version.

Use three decimal components X.Y.Z without leading zeroes or suffixes. Components
are limited to 0..255 to keep JUCE's packed numeric representation unambiguous.
This is a numeric SemVer-style product version, not full SemVer prerelease syntax.

## Cheap verification and the packaging interface

### Running build information

CMake generates `generated/BuildInformationData.h` and `build-information.json`
from the product version, explicit release tag/candidate and full Git SHA/status.
Settings → About displays that compiled identity; Copy build information copies
the full version/build/commit/source-status report. It does not read Git, touch
project state or perform work on the audio callback. No build clock, random ID or
absolute workspace path enters these generated identity values.

The supported wrapper passes both identity inputs on every configure. An ordinary
`scripts/build.ps1 -Configuration Release` clears prior candidate cache values and
identifies `vX.Y.Z-local`. The release script explicitly forwards its existing
`-ReleaseTag` / `-Candidate` inputs and rejects mismatched generated identity.
When building/installing an identified candidate directly, pass those same
optional arguments to `build.ps1` or `install-test-vst3.ps1`; defaults deliberately
produce local builds. Missing Git identity is shown as unavailable and is not
accepted for packaging. Numeric version, plugin IDs and storage schemas remain unchanged.

Run from the repository root; use the installed CMake supporting the existing VS
2026 generator for native builds. Script-only checks need CMake 3.22+, no compiler,
JUCE download, Git tag lookup or plugin configure:

```powershell
$cmake = "$env:ProgramFiles\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cmake -P scripts/verify-version.cmake
& $cmake '-DRELEASE_TAG=v0.1.0' '-DOUTPUT_FILE=out/release-identity.json' -P scripts/verify-version.cmake
if ($LASTEXITCODE -ne 0) { throw 'Version verification failed' }
```

Quote each `-DNAME=value` argument in PowerShell, especially dotted values. The
script resolves its own source root, so absolute-path invocation also works from
other working directories. SOURCE_DIR can explicitly select another source root.

RELEASE_TAG, when supplied, must equal `v` plus the exact current numeric version.
Bare 0.1.0, v00.1.0, v0.2.0 and v0.1.0-rc.1 fail against 0.1.0. To label a candidate,
use the separate `'-DCANDIDATE=rc.1'` argument, optionally together with the matching
base RELEASE_TAG. Labels contain alphanumeric components separated by dots/hyphens.
The candidate's releaseIdentity is v0.1.0-rc.1, while its plugin version remains 0.1.0.
Candidates must remain explicitly labelled by the packager; this verifier
neither changes tags nor creates, renames, signs, installs or publishes packages.

OUTPUT_FILE is optional. On success its JSON contains:

| Field | Meaning |
| --- | --- |
| version | Authoritative numeric X.Y.Z read from CMake |
| expectedTag | Exact vX.Y.Z for that version |
| requestedTag | Supplied tag, or empty when none was requested |
| tagVerified | True only if a supplied tag matched |
| candidate | Explicit candidate label, or empty |
| releaseIdentity | vX.Y.Z, with -label only for explicit candidates |
| builtVerified | True only if requested built metadata checks succeeded |

Consumers such as scripts/release.ps1 must check the process exit code before
reading JSON. A failure does not write successful output, but an old output file
may still exist: never use it after a nonzero exit. Source-only queries do not
claim tag or binary validation. Stable releases must request tag and built checks;
explicit candidate packages must keep their candidate label. No packaging workflow
is implemented by Issue #20.

## Built verification

```powershell
./scripts/build.ps1 -Configuration Release -Test
& $cmake '-DRELEASE_TAG=v0.1.0' '-DBINARY_DIR=out/build/windows-msvc' '-DCONFIGURATION=Release' '-DOUTPUT_FILE=out/release-identity.json' -P scripts/verify-version.cmake
if ($LASTEXITCODE -ne 0) { throw 'Built identity verification failed' }
```

The check compares the source version, configure-derived identity and the built
VST3 module/class metadata, requiring the binary and manifest to exist. Stale outputs
after a version edit fail until rebuilt. CTest includes a compiled assertion of
JUCE's real version/identity definitions, plus compiler-free negative fixtures and
the built metadata check. The fixtures use dummy binaries and are not native-build
or DLL-authenticity validation. Neither a manifest check nor CTest replaces manual
FL Studio acceptance or signing/provenance verification.

## Manual bump procedure and compatibility

1. Owner chooses the next numeric version; edit only the root project VERSION.
   Do not edit generated JSON/resources or use Git commits/merges to bump it.
2. Run the cheap verifier with the intended exact vX.Y.Z tag; a mismatch must stop
   release work. Use separate candidate metadata when needed.
3. Rebuild through scripts/build.ps1, run matching CTest and built verification.
   Inspect actual binary/file and VST3 metadata before any future packaging.
4. Review the change and evidence in an owner-reviewed PR. Tagging, packaging and
   publishing remain separate authorized actions.

Product versions are independent of plugin/swell saved-state versions (currently 8),
capture schema (currently 1), managed-asset engines and IR/render manifest formats.
Do not bump these with releases or add product-version gates to state loading.
Preserve manufacturer code K7Ma, product code Prv1, bundle ID com.k7ma.preverb,
ReverbArise product name, VST3 component/controller CIDs and
JUCE_VST3_CAN_REPLACE_VST2=0. No version bump or candidate label migrates saved FL
Studio sessions, captured source or assets. Changes to storage formats or host
identity need separately scoped compatibility work.

## Packaging integration

The release packager must validate actual Git refs against the intended source
revision and verify worktree cleanliness before packaging; matching a supplied tag
string here does not prove that a Git ref exists or identifies this checkout.
After any failed verification it must reject previously generated release-identity
JSON, even if the file still exists. These packaging checks belong to Issue #21;
the [local release workflow](releases.md) now supplies those checks.
