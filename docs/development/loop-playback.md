# Loop playback scheduler

The headless Loop route uses the last accepted source identity and exact recorded extent. The next-capture preference controls Learn. The captured grid describes the source, while a separate live reference verifies the shared phrase origin and recurring phase. See [Loop host admission](loop-host-admission.md) for the timing and command contracts.

The live adapter checks phase in dry Standby. Cue pins one complete prepared revision and chooses the first future verified window that can fit the full wet lead-in. The wet downbeat follows the observed marker. Perform Stop clears queued work or starts an immediate wet fade of at most 5 ms. Loss of sync clears pending work, fades active wet, and requires fresh phase and Cue. Capture commands retain their own roles.

The swell worker reads the same shaped render assets used by Song playback. It publishes and retires Loop plans outside the audio callback and charges the current and held plans against the shared 256 MiB PCM working budget. Preparation checks a bounded range of at most 64 cycle candidates and conservatively bounds simultaneous cycle contributions at 64. The bound includes rounding and lead correction. Exceeding either resource limit makes the prepared plan unavailable and leaves dry audio available. The mixer sums every intersecting contribution from the first scheduled cycle forward.

The headless processor entry point accepts an explicit live reference setup and exposes phase, preparation and scheduler states separately. Editor bindings follow in #126 and FL Studio host routing in #127.
