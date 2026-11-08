# Issue #116 clocked Loop Learn

Local Windows x64 Release worktree on \`b55c493\` with local changes. The final repository-wrapper build was \`20261108-001440-75cd76da\`. The complete VST3 bundle remains in the worktree build tree. The binary SHA-256 is \`b2aee0a42c93daf7211cf933182192287218e6be7a7ec7b867b90476541a229a\`.

## Native verification

- All 12 focused Release CTest cases passed. Coverage includes the recorder, Learn UI, workflow processor and state, capture transaction and context, portable snapshot and recovery, Song export, UI standards and guide currency.
- Recorder checks use sample-offset clock and gate events and inspect the resulting stereo float WAV. They cover a mid-bar gate release, a release on the boundary, a fixed length of one bar and two beats, an off-grid Start, a missing beat after a pending Stop, cancellation and stale settlement. The accepted take and musical length survive restore. Callback allocation and deallocation checks passed. The pre-existing Song capture and gate-only project fixtures also passed.
- The native editor fixture exercised both capture method selections and the shared length control binding. Bounds checks and snapshots cover 900×600 at 100% and 125% user scale. Owner review of the Learn layout remains open.
- The initial broader CTest run was stopped during its later tests. It recorded an early UI standards failure for a per-screen wheel override, which was removed. The final checks used the control family's default and all selected tests passed. The final build has focused regression coverage. Live FL Studio, Stuart's full patch, and physical DPI were outside this run.

## Live Rack smoke

The final local VST3 loaded from its build tree in VCV Rack Free 2.6.6 with Host-FX 2.1.0, Impromptu CLKD 2.5.0 and Fundamental 2.6.4. The isolated 137 BPM fixture wired the actual CLKD master, /4 bar, /16 phrase and separate /16 gate through Host-FX. The four-beat bar came from that wiring. Both methods accepted a real, non-silent stereo WAV at 48 kHz.

| Learn selection | WAV frames | Displayed duration | Peak |
| --- | ---: | ---: | ---: |
| Follow capture gate | 252,263 | 5.255 s | 0.73105 |
| Fixed length, 1 bar and 0 beats | 84,088 | 1.752 s | 0.73105 |

The Follow waveform shows three bars. Switching capture method retained that take until the new one-bar capture finished. The one-bar result remained visible in the Rack plugin for review. The separate native fixture checks exact Stop and Start offsets. The live smoke checks Host-FX wiring, control interaction, real captured audio and the final WAV lengths.

- Gate take SHA-256 \`a8fdee67e58f11aed4c0fccc4bbf56e6fc52f30c1bf0497b7b44929f6adec3e2\`
- Fixed take SHA-256 \`4e41cabb20c14aae7de0b02480edd8b2a2a37c02f01dd90cff92e07af7493734\`

The fixture \`out/loop-mvp-spike/rack/clkd/ReverbArise-CLKD-Learn.vcv\` can be regenerated with \`make-learn-fixture.py\` in the same directory. It loads the complete Release bundle directly from the worktree build path.
