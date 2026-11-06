# Issue #111 private compatibility diagnostic validation

Windows x64, Visual Studio 18 2026 MSVC, Windows SDK 10.0.26100.0,
PowerShell 7.6.9; existing dependency/build cache reused. This is native evidence,
not VCV/Audacity acceptance. Exact delivery source, build/toolchain and payload
identity are recorded in the private ZIP's BUILD-IDENTITY.json and the adjacent
build-manifest.json. Unrelated pre-existing untracked OpenSpec planning files
were preserved and disclosed; tracked build inputs are committed.

`scripts/build.ps1 -Configuration Release -CompatibilityTest -Test` succeeded.
The 21:04:42 verification run selected four CTest tests: compatibility, UI
standards, workflow processor and capture failure. All four passed in 4.43 s.
The disabled-variant native compile fixture also passed; it statically verifies
the normal processor/editor/header have no diagnostic members or actions.
The committed delivery revision is rebuilt and the same focused tests are
rechecked before packaging; its output manifest is the delivery identity authority.

The compatibility test uses native JUCE processor/editor/windows and supplied
playhead fields. It covers:

- No callbacks versus silent input versus independently observed non-silent input.
- Missing host PositionInfo, samples, PPQ, tempo and meter before recording starts.
- Stuart's missing-meter case: samples/PPQ/playback/48 kHz/279.965 BPM present,
  no time signature; exact message **Capture blocked: host time signature
  unavailable**, idle workflow, revision zero and zero dropped blocks. The report
  asks for intended-tempo comparison and does not infer inconsistent sample/PPQ.
- Supplied seconds with missing samples remains blocked; no fallback is used.
- Successful source commit, timing-change rejection and preservation of earlier
  committed source. Worker confirmation survives the absence of later callbacks.
- Zero watched C++ allocations/deallocations in the exercised audio callbacks.
- Automatic modeless window creation, repeated show without duplicates, hide and
  header reopen, retained processor history, destruction of the owned companion
  and safe deferred callback after an editor is already destroyed.
- Native clipboard copy/readback equality, visible copy confirmation, Clear report
  feedback, reset history/counter baselines and preserved committed regions.
- Independent processor instances, callback repeat coalescing, 256-slot observation
  and 128-entry history bounds, overflow/truncation disclosure, separate 64-slot
  worker confirmation queue and 32-entry history bounds.

`preverb_compatibility_test.exe --snapshot` exercises the same native cases and
captures the missing-meter companion for direct layout inspection. The optional
image is local `out/compatibility-window.png`; default desktop DPI only. The
status/report panel scrolls, and the shared Copy/Clear buttons remain reachable.
The snapshot harness truncates an existing image before writing a new one.

The build-wrapper fixtures are reported separately from native MSVC/CTest.
They exercise fake tools and intentional subprocess failures, not a real compiler.
The direct in-process invocation left an intentional native failure's LASTEXITCODE
despite its 74 passing assertions; it was rerun in its own PowerShell process to
check the fixture process's actual exit status. The initial native configure was
blocked by sandbox SDK registry access; authorised escalation allowed the build.
An initial GUI test link failure was corrected by linking the harness directly
to JUCE audio-utils, consistent with the existing native UI harnesses.

Private packaging uses `scripts/package-compatibility-test.ps1`: tracked-clean
source and full SHA comparison, enabled diagnostic cache, build-directory lock,
native focused CTest, PE x64/file version and stable VST3 class identity checks,
complete bundle copy inventory equality, dependency notices, installation and
rollback notes, per-entry ZIP SHA-256 verification and whole-ZIP checksum.
Standard release tooling/gates were not weakened. No tag/public release, merge
or changes to existing Drive packages are authorised by this work.

Skipped by explicit issue-specific owner waiver: OpenSpec proposal/Apply approval
gates, separate UI approvals, full regression and user-guide refresh. Not tested:
VCV Host-FX, Audacity, FL Studio, alternate DPI, clean machine/VC runtime, signing,
real-time scheduler latency or clipboard-failure conditions in another host.
The fixture's non-silent input does not establish that Stuart's real patch reaches
the plugin. His observed missing meter alone explains timing validity being false;
sample/PPQ consistency and the intended tempo remain unverified.

The explicitly user-selected missing-meter fallback is authorised for a separate
investigation after diagnostic delivery. It is not implemented in this package.
