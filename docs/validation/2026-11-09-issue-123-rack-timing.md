# Issue #123 Rack timing evidence

[Issue #123](https://github.com/elneilios/ReverbArise/issues/123) ·
[Admission contract](../development/loop-host-admission.md) ·
[Reproduction and analyzers](issue-123-rack-fixture/README.md)

Baseline `a35f156`. Windows x64, MSVC 19.51.36260, SDK 10.0.26100 and
JUCE 9.0.4 at `3fb5cbc`. The standalone observer built in Release.
The host runs used Rack Free 2.6.6, licensed Host-FX/Host-XL 2.1.0,
Impromptu CLKD 2.5.0 and Fundamental 2.6.4 at 48 kHz. The Host-FX worker
was enabled. Headless Rack ran the generated patch in a disposable
profile. There was no audio device in the fixture. The dry inputs mirrored
the electrical bar and phrase references. The earlier Rack cue spike is
the separate audio-path evidence.

## New host measurements

The alignment patch uses shared CLKD clock/reset, /4 bars and /16 phrases.
These windows contain the first 120 seconds of the new run. Both included
all seven roles on MIDI channel 1, valid callback-relative offsets and
zero trace losses. Alignment counts exclude ambiguous startup and trace
end samples.

| Tempo | Host setting / actual frames | Phrase rises aligned | Bar rises aligned | Dry minus MIDI |
| --- | --- | ---: | ---: | ---: |
| 137 BPM | 256 / 128 | 16 | 67 | 0 samples |
| 280 BPM | 512 / 256 | 34 | 139 | 0 samples |

Each lane received the same four Cue pulses, one Perform Stop and one
CLKD Reset. Capture gate, phrase, bar and beat were recurring signals.

| Role | Note | 137 BPM edges / offsets | 280 BPM edges / offsets |
| --- | ---: | --- | --- |
| Capture gate | 14 | 274 / 0–127 | 560 / 1–249 |
| Phrase | 12 | 36 / 1–123 | 72 / 1–233 |
| Bar | 19 | 138 / 0–127 | 282 / 1–233 |
| Beat | 21 | 550 / 0–127 | 1122 / 1–251 |
| Reset | 22 | 2 / 1–49 | 2 / 1–49 |
| Cue | 16 | 8 / 0–48 | 8 / 0–176 |
| Perform Stop | 17 | 2 / 0–48 | 2 / 0–48 |

Evidence SHA-256:

- Observer VST3 `ea76464fdd937079c83c1a42b7285a46c3b8858c9bd79c853434fe03e5dddecf`.
- Generated alignment patch `e2c0488b16d4025293b30bd8c8e07af8557506a560dc4bebbd7c66b03fbf95dd`.
- Raw 137 BPM window `1584b8f6ea64828cff74d7ccd3593a5a7ae74635269a769d6df62ef177a9fab3`.
- Raw 280 BPM window `aa15047941e8c89988b5e003bcaecc267964c28a860bd640cb27b274bf306ec9`.

### Continuous timing

The same run reached 177.472 seconds of engine time in both lanes in a
320-second wall-clock session. Both lanes had zero trace loss. The table
uses the complete post-reset dry intervals. Phase error is measured
relative to the first post-reset onset and the declared fractional
period. All matched dry/reference onsets, including the later portion
of the run, had zero sample offset.

| Tempo | Complete post-reset intervals (frames × count) | Cumulative phase error (samples) | Aligned phrase / bar rises |
| --- | --- | --- | --- |
| 137 BPM | 336,350 × 12 and 336,351 × 7 | −0.5547 to +0.4307 | 25 / 100 |
| 280 BPM | 164,571 × 23 and 164,572 × 17 | −0.4286 to +0.4286 | 51 / 207 |

Full traces: `timing-9de128ab854e4646959279ec54f11c1d.csv`
(`c8e03c7fdb550b0d7f7dc4a8ef2d73cabca9bfd1470b00a553b0f206d8568fad`)
and `timing-a17baf9fb59f4d35a7e9f1849f0ad828.csv`
(`2c83deae8f4e27ad151e69f0735c0cc8219eac490caea034f6c2d52e798d3814`).
The measured engine span limits the result. It does not establish
physical audio monitoring or endurance over a performance-length session.

### Missing reference, reset and reopen

The masked patch ran for 118.064 processed seconds per lane. The dry
reference continued across one missing phrase marker at 137 BPM and two
at 280 BPM. Bar audio and MIDI still aligned at zero offset over 66 and
137 rises respectively. The extra VCA put received phrase markers one
sample after the dry reference while the mask was open. There were no
trace drops. The corrected trace replay revoked phase once in each lane,
reacquired it and admitted a new Cue. At 137 BPM the earlier Cue arrived
before re-lock. At 280 BPM it was admitted and cleared by loss.

Reset and a phrase marker coincided in the direct patch. Using that
marker as the next epoch's anchor failed the cumulative phase check at
137 BPM, even though later integer intervals were valid. Excluding the
coincident marker let all 20 and 41 post-reset markers pass respectively,
with zero sync losses on the two continuous traces.

After closing Rack and reopening the same saved alignment patch in the
same test profile, both observers reported their first callback at
sample 0 and first phrase marker at sample 1. The 128-frame lane observed
three phrase rises and the 256-frame lane observed six. This checks
reopen across a host restart. It does not cover a live in-process patch
swap or production project restoration.

| Evidence | SHA-256 |
| --- | --- |
| Masked patch | `a87c30d99bce323456c05774b931b24d30fab89c969da2898989401232126529` |
| Masked 137 BPM trace `timing-079803bf97234d1db3430d467747426c.csv` | `adac27d9116fe4e15ae754515c55e6a5fa216b00f90eb4e000ff3c1463841f5e` |
| Masked 280 BPM trace `timing-0336764fafc94615ae4085f0c6720e7e.csv` | `b37be415f9fa3758fb8857b3ea2e289d11cbb08d2efdf86bda136c37ec92dde0` |
| Reopened 128-frame trace `timing-becc444680f14a75b4bacb9421f467e6.csv` | `860eea8ff112346e2ca3cc9972df18b57c00b47e3060e3cac52a6a949c5ee42c` |
| Reopened 256-frame trace `timing-0c7c40deb3bb4c298cd375d7cfbc3dd2.csv` | `e28e69672aef3aaef2c50633009e92651cbbed0851f4d457acf467d1be46e538` |

## Reference policy checks

`python -B check-admission.py` passed the focused scenarios for independent
readiness, missing meter, take/reference mismatch, adjacent intervals,
accumulated drift, extra and missing markers, reset, fresh Cue, invalid
offsets and the 128-event limit. Replay passed both continuous and both
masked trace lanes. Replay uses callback observations from the trace,
while the synthetic case checks the missing-marker deadline at sample
precision.

The PR review regression checks also passed. Cue at the last permitted
sample was admitted, and Cue one sample later was rejected before the
next heartbeat. At 137 BPM, both measured four-bar frame counts
(336,350 and 336,351) matched the reference with verified take timing.
A one-bar take, out-of-range frame counts and incompatible timing were
rejected. Both continuous and both masked traces passed replay after
these changes.

## Replayed spike observations

The [#116 evidence and sources](https://github.com/elneilios/ReverbArise/tree/47cbed2/docs/validation/issue-116-spike-fixtures)
are from separate host sessions. Replaying the four original CLKD capture
traces reproduced their saved JSON byte for byte. All 37 between-bar
requests closed on the next verified bar and all nine same-bar requests
closed on that bar. The ending sample stayed outside the take.

Replaying the earlier lifecycle trace reproduced a startup-high note-on
at sample 1, a disconnect note-off at sample 1,076,800 while the dry gate
stayed high, and a 21,673 ms callback gap during suspension. There were
zero processor lifecycle events after startup. That host action test used
the earlier synthetic clock fixture. It establishes that Host-FX can
emit these ambiguous observations. It cannot qualify a release on the
CLKD route by itself.

The earlier [clocked Learn host smoke](https://github.com/elneilios/ReverbArise/blob/47cbed2/docs/validation/2026-11-08-issue-116-clocked-learn.md)
captured 84,088 frames for one bar at 137 BPM and 252,263 frames in its
three-bar gate take. The gate take demonstrates completion on the
fixture's deliberate release. The unplug trace is why a raw release
cannot qualify as a successful end in the contract.

The reference cases reject arming on an already high input, qualify a
later verified rise after low, and require a distinct qualified end route.
They check same-bar and next-bar exclusive ends, missing callbacks before
the boundary and interruption before a later observed bar. The actual
suspend trace has a visible host-side pause, but no VST3 lifecycle signal.
No result here grants capture completion for an unobserved boundary or
for the first note-off from an unqualified gate.

The archived lifecycle raw CSV SHA-256 is
`ded4305e654306e560ac9e7d61bbe795427827d73ceb2cef92e7a5fffd362a98`.
Its action timestamps SHA-256 is
`b297872ecd05421b0a8d3a6b28d39a6bcaf983574af80010a0f0bd3d73b3e787`.

## Native Song regression

`scripts/build.ps1 -Configuration Release -Test` configured and built
successfully using pinned cached dependency sources. The focused native
workflow processor, capture transaction, capture context, scoped
recapture, transport time, transport playback and persistent capture
tests all passed. The complete CTest run passed 97 of 98 in 390.79
seconds. The wrapper exited 8 because `preverb_perform_readability_test`
timed out at the Windows foreground-focus predicate in
`PerformReadabilityCases.h:56`. The same failure is recorded in the
[earlier working folder validation](2026-11-06-issue-8-working-folder.md).
An isolated retry of the readability test reproduced the same timeout
in 31.39 seconds. Its log is `out/issue-123/readability-retry.log`.
The #123 diff contains the probe, reference checks, documentation and
OpenSpec artifacts. It does not change the production code or the
existing native tests.

Wrapper logs: `out/build-logs/Release/20261109-161716-29f58cad/`.
Strict OpenSpec validation and the local documentation links passed.

## Owner decision

**Go, approved by the owner on 9 November 2026.** The contract applies
to Rack Free 2.6.6, Host-FX 2.1.0 and CLKD 2.5.0 at 48 kHz in the
measured 137 BPM / 128-frame and 280 BPM / 256-frame lanes. Both require
shared clock and reset, declared 4/4 meter, and verified alignment
between the audible phrase and its reference.

| Route or condition | Approved disposition |
| --- | --- |
| Song timeline | Existing Song admission and guards. |
| CLKD phase, beat/bar/phrase, external Cue/Stop | Go for the measured stable-tempo lanes with matching phrase origin and fresh sync. |
| Duration capture | Go when a verified start and musical end are available. |
| Raw capture gate release | Withhold as a capture end. An independent request must be verified before advertising End signal. |
| Ordinary parameter automation as a sample-offset route | Withhold. The spike observed coalesced or missing short pulses and callback-boundary offsets. |
| Other rates, tempos, host versions or unverified wiring | Outside the current evidence. |
| FL Studio Loop MIDI and automation | Pending in [#127](https://github.com/elneilios/ReverbArise/issues/127). |
