# Issue #116 live Rack CLKD capture timing

## Fixture

Windows x64, VCV Rack Free 2.6.6, VCV Host-FX 2.1.0, Impromptu Modular 2.5.0 and Fundamental 2.6.4. The CLKD panel matches the module visible in Stuart's screenshot. The fixture used its own settings and wiring. The official CLKD package was loaded into a separate copy of the existing isolated Rack profile.

Each CLKD drove the left audio accent and the MIDI Bar input with its /4 output, and the right audio accent and MIDI Phrase input with its /16 output. These divisions define a four-beat bar and a four-bar phrase for the fixture. Each audio accent passed a Fundamental VCO square through a VCA. The VCA adds a Rack engine sample on the audio path. The test probe auto-armed on a phrase downbeat after observing a complete bar. Its Capture Stop came from CLKD /2 for the between-bar cases and CLKD /4 for the same-bar case. CLKD continued running after the request.

The native MSVC Release VST3 probe inspected the actual Host-FX MIDI offsets and audio inputs at 48 kHz. It counted the included and non-silent samples within the test capture window. Capture end excluded the boundary sample. Trace writes ran on the message thread. This fixture measures the prototype capture window and leaves WAV storage and product integration to subsequent work.

## Measured results

| Nominal BPM | Actual callback frames | Stop placement | Completed capture windows | Bar intervals in samples | Four-bar phrase intervals in samples |
| ---: | ---: | --- | ---: | --- | --- |
| 120 | 128 | Mid-bar | 8 | 96,000 | 384,000 |
| 137 | 128 | Mid-bar | 9 | 84,087–84,088 | 336,350–336,351 |
| 137 | 256 | On the bar | 9 | 84,087–84,088 | 336,350–336,351 |
| 280 | 256 | Mid-bar | 20 | 41,142–41,143 | 164,571–164,572 |

- All 194 checked bar onsets and 44 phrase onsets arrived one sample after their matching MIDI markers, consistent with the extra VCA in the audio path. Every complete phrase interval spanned four bars. There was no measured onset drift during the run.
- All 37 mid-bar requests closed the capture on the following Bar marker. All nine requests on a Bar marker closed on that marker. Capture length matched end minus start for every take, and each contained 49 non-silent frames from the gated tone.
- At 137 and 280 BPM, consecutive intervals can differ by one sample even though the audio and clock edges stay in phase. The existing fixed-sample-period playback lock cannot be assumed to work at those tempos. A marker-based playback policy for fractional periods needs separate evidence before production use.
- 280 BPM is a fixture setting near the diagnostic reading of 279.965 BPM. Stuart's intended tempo, full patch, reset and missed-pulse behavior, Cue and Perform Stop, Capture Gate falling-edge behavior, physical audio output and FL Studio were outside this run.

## Reproduction

In the issue worktree, `out/loop-mvp-spike/rack/clkd/` contains the generated `clkd-bar-stop.vcv`, `make-fixture.py`, `ClkdTrace.cpp`, `check.py`, and `results.json`. Build the standalone `rack_clkd_trace_VST3` target in the Rack spike CMake project in Release. Open the patch in an isolated Rack profile with the listed modules, let it run for at least 75 seconds, and run the analyzer with the four new CSV traces. `previous-traces.txt` records the trace names that predate the run. The results JSON names the four checked traces in the adjacent `traces` directory.

- CLKD package SHA-256 `446cfb063e757592cd5b0a422634cc1e5a5e7db5d826bbc6e684124c2cf3f254`
- Fixture SHA-256 `d5bfe12e8e06e482042e9a5fd64858a81df876167d6a493ad692094ae44e4792`
- Probe VST3 SHA-256 `e5efbaacfaa30f96de79ad9f9b89b8b1078cff118346f594816319f659a5020c`
