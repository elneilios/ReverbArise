# Private compatibility diagnostic variant

Issue #111 adds a diagnostic companion for Stuart's VCV Host-FX investigation;
#60 consumes the same observations for Audacity. Neither host is claimed fixed.

## Explicit 4/4 experiment requested after diagnostic implementation

The owner subsequently requested hard-coded 4/4 for this private build. Build
with `scripts/build.ps1 -Configuration Release -CompatibilityMeterFourFour -Test`.
This implies the diagnostic variant and explicitly enables
`PREVERB_COMPATIBILITY_METER_4_4`. Without the extra switch, diagnostics keep the
original missing-meter guard. Ordinary wrapper builds disable both switches.
CMake rejects the meter experiment without the diagnostic companion.

Only a present PositionInfo with an absent time signature receives effective 4/4.
A host-supplied meter always wins, including invalid values that must still be
rejected. The coherent diagnostic getter supplies the same effective meter to
capture, UI and subsequent processing. The real host sample position, PPQ and
reported BPM remain untouched. Raw meter absence stays separately visible in the
companion/report. The experimental assumption is labelled prominently even before
callbacks arrive, and in the ZIP filename, README and manifest. No runtime meter
selector or saved-state schema change is introduced in this narrow build.

Existing `transportMappingMatches` still compares inferred sample origins across
observations (tolerance one sample) with fixed rate/tempo/meter. The native fixture
proves capture commits at 279.965 BPM with absent meter and consistent real sample/
PPQ fields, and rejects an inconsistent sample/PPQ observation while preserving
earlier source. Supplied 3/4 is retained. Other missing-field and capture guards
remain covered. These are controlled native results; VCV input reception, tempo
intent and mapping consistency remain unverified until Stuart tests the ZIP.

Build with `scripts/build.ps1 -Configuration Release -CompatibilityTest -Test`.
The switch explicitly configures `PREVERB_COMPATIBILITY_TEST=TRUE`; ordinary
wrapper invocations explicitly reset it to FALSE. The diagnostic build selects
the VST3, native companion tests, a disabled-variant compile fixture, workflow
processor tests and capture-failure tests. With `-Test` it runs the focused CTest
selection plus UI standards. It deliberately does not select full regression.
`-PluginOnly` still builds only the VST3. No plugin ID, session schema, capture
asset path or capture guard changes with this option.

The editor asynchronously opens one modeless **ReverbArise Compatibility Test**
window. Its SafePointer guards deferred creation; the editor owns and destroys
the window. Closing the companion hides it. The header's **Compatibility report**
button reopens the same window. Its timer updates without repeatedly bringing it
to the front. Shared ActionButton controls own Copy report and Clear report.

Raw stereo peaks, exact non-zero/nonfinite observations and callback count are
sampled before tone, monitoring and DSP, regardless of capture eligibility.
The initial boolean passed to workflow admission is observed without changing
it. Missing fields, mapping, busy state, offline mode, Learn mode and the existing
play-edge requirement are distinguished. Admission attempts, successful starts,
recording rejections and provisional versus committed capture remain distinct.
Seconds and raw PPQ loop bounds are observations only. JUCE does not expose
individual availability for PositionInfo's boolean play/loop fields; the report
states this limitation. No free-running song clock or timing fallback is added.

Stuart's Alpha 2 evidence reports callbacks/playback, samples, PPQ, 48 kHz and
279.965 BPM, but no meter (0/0), revision zero, no recording and zero dropped
blocks. The companion explicitly reports **Capture blocked: host time signature
unavailable** before a pass starts. Missing meter alone invalidates current
capture timing; it does not independently establish inconsistent sample/PPQ
mapping. Input reception remains unverified in Stuart's host until this independent
measurement runs. Reported tempo is shown with a request to compare it with the
intended tempo, never corrected automatically. A user-selected missing-meter
fallback was initially authorised for investigation; the subsequent explicit 4/4
request is implemented only by the optional experimental switch described above.

One callback producer transfers plain observations through a 256-slot SPSC queue
to one message-thread consumer. Release/acquire indices protect slot ownership;
full queues omit diagnostic observations and increment an atomic counter. The
callback performs no string construction, locks, disk/clipboard/UI operations or
allocation. State transitions publish immediately; unchanged observations are
coalesced with at most one periodic observation per second. The processor owns
the queue, latest observation and 128-entry history, so editor close/reopen does
not reset them. Truncation and queue loss are disclosed. Independent processors
share no diagnostic state. History timestamps are monotonic milliseconds, not
host song positions. Values retained in history allow inspection of advancement,
discontinuities and consistency; no host-specific cause is inferred.
The capture worker has a separate 64-slot SPSC confirmation queue and 32-entry
history, preserving successful/empty settlement even when callbacks stop before
the worker finishes. Its truncation and loss are also disclosed.

Copy report runs on the message thread and verifies clipboard readback before
showing success. It includes exact source/build identity, detected host/wrapper,
OS/configuration, timing field values or unavailability, worker outcomes and
bounded history. Audio, project identities and private paths are excluded.
Clear report drains the existing queue and resets history/baselines only; the
latest observation and explicitly labelled lifetime counters remain. It never
clears accepted source, swells, worker failure evidence or saved projects.

`scripts/package-compatibility-test.ps1` is an isolated private-only packaging
path for the owner's #111 fast-track. It requires committed tracked inputs,
matching configured build SHA and enabled diagnostic variant, takes the build
lock, verifies PE/version/plugin identity and reruns focused native checks.
It reuses existing release inventory, notices, toolchain, bundle and ZIP hash
verification helpers. It does not call or change the standard release gates or
publication path. Unrelated untracked planning files are disclosed; generated
build information truthfully retains its worktreeDirty flag. The output directory
contains the full source SHA and refuses reuse. README, build identity, licenses,
per-file inventory and ZIP checksum accompany the complete bundle.

The owner waived OpenSpec proposal/Apply gates, separate UI approval, full
regression and user-guide refresh for #111 only, and authorised private build
and Drive delivery before PR review. Standard release policy is unchanged.
Native tests are evidence for the component and simulated host fields, not
acceptance in VCV, Audacity or FL Studio.
