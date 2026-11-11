# Issue #129 current-build Loop project recovery

The 2026-11-11 desktop run used disposable copies of the Rack clock-only patch and the FL Studio Playlist fixture. Both projects started with an empty ReverbArise Loop and captured new audio. FL Studio used the Playlist dry track and an imported copy of the bundled Preverb Hall impulse response.

Recovery used the Release VST3 built from the #129 worktree at `3aea131` with uncommitted changes. The installed module SHA-256 was `F642559351B4C5A3A2B60DE69D51388534C7A899F46231194F55923478693055`. The Rack take was recorded with the preceding #129 candidate and saved, reopened and recovered with the updated module.

| Host | Accepted capture at 48 kHz | Save, reopen and recovery |
| --- | --- | --- |
| Rack 2.6.6, Host-FX 2.1.0 | 4 bars, 137 BPM, 4/4, 336,351 frames | Passed Locate and Restore with the original folder unavailable. A save and reopen after each recovery retained the accepted revision and local grid. All four swells and the frozen edit survived. All 13 recovered asset paths matched by SHA-256. |
| FL Studio 26.1.7.5654, Playlist | 2 bars, 120 BPM, 4/4, 192,000 frames, stereo | Passed Locate and Restore with the original folder unavailable. A full host restart after each recovery confirmed the saved association and waveform. The before, located and restored saves contained identical project, instance, manifest and revision identifiers and all four serialized swell records. All 15 recovered asset paths matched by SHA-256. |

In both hosts, Learn reopened disarmed with Recapture Audio off. Perform remained idle when timing returned. A fresh Cue entered playback and advanced the Loop playhead. This run checked transport and playback state visually and compared the persisted assets programmatically. It did not measure host audio output.

The initial Rack check exposed a Storage handoff rejection while CLKD was stopped. Host-FX still reported transport playing without a valid Song timeline. The updated candidate uses the Loop playback state to protect active playback on that host and retains the host transport check when a valid Song timeline is available. The previously rejected working-folder handoff succeeded on the updated candidate.

In FL Studio, the desktop automation tool lost focus when the missing-assets notice opened the chooser directly. The successful route was **Continue**, then **Settings → Storage → Locate assets / Restore backup**. Both operations used the plug-in’s native file choosers.

Host projects and checksum reports are retained under `out/issue-129/host-automation/`. The projects reference disposable local asset folders listed in its `storage-root.txt`.

## Release verification after review

- The focused Windows MSVC Release CTest run passed 7 of 7 tests. It covered Loop playback, both Rack clock replays, asset inventory, working-folder association, portable snapshots and portable recovery. The recovery and playback tests included the manifest preflight and concurrent Cue cases added after review.
- `./scripts/build.ps1 -Configuration Release -PrRegression` passed 15 of 15 tests. The first gate exposed a fixture that skipped JUCE's editor-deletion notification. A focused recovery run also exposed a fixture that hashed assets before the engine had adopted the restored location. The corrected fixtures passed both gates.
- The final runs used `c8650e5` plus the two fixture corrections. Local logs are in `out/issue-129/release-focused.log` and `out/build-logs/Release/20261111-131744-01dceb45/`. The desktop Rack and FL Studio results above used the earlier installed candidate.
