# FL Studio Loop transport and Perform review

12 November 2026 · Windows x64 · MSVC Debug

- The owner approved the native Action Button + State Field and integrated Loop Perform layout. At 900 × 600, all three pairs fit on one row. The native review showed the disabled Set Loop Start help in the fixed footer, the matched disabled outline and the Audition button switching to a red Stop action.
- The native review checks both heading roles at 900 × 600 and 1100 × 850. The Debug VST3 and native preview built with `scripts/build.ps1` on the feature worktree. Strict OpenSpec validation passed.
- The native review uses the Rack timing fixture. FL Studio repeat playback, parked playhead freshness and the saved Playlist placement still need the FL Studio desktop smoke test. Stopped placement currently opens explicit bar-and-beat entry.
- Processor, sample and component regression cases were added. Automated execution follows the owner approval gate in the contribution workflow. Fractional native DPI coverage is pending.
