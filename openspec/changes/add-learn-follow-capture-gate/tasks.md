# Tasks

## 1. Gate input and capture

- [x] 1.1 Add the VST3 MIDI note input and map MIDI note 14 to the Loop gate. Verify the VST3 identity, stereo buses and existing parameter IDs and test that the same MIDI events leave Song capture behavior intact.
- [x] 1.2 Adapt the bounded spike recorder for realtime stereo input, explicit arm and precise note offsets. Verify same-callback and split-callback boundaries, a held gate at arm, repeated edges, note-on with zero velocity, missing timing, cancellation, reset, rate change, stereo validity and the capacity limit.
- [x] 1.3 Bind next capture preference and arm ownership to Learn and current instance state. Verify that Song and Loop cannot start overlapping passes, unsupported or offline contexts cannot start a Loop take, and cancellation or state replacement prevents stale acceptance.

## 2. Take publication and Learn UI

- [x] 2.1 Publish and verify a unique stereo WAV outside the callback and persist the accepted Loop reference separately. Test silence, invalid samples, storage failure, missing or corrupt files, reopen, restore during settlement and older Song state with and without working folder association.
- [x] 2.2 Add Enable Loop, Follow capture gate, Arm/Cancel, and verified WAV access under Learn Source Capture using the shared controls. Verify that status follows processing state, Perform and Export follow the accepted take type, and rejected Loop captures leave prior Song access intact.
- [ ] 2.3 Run the UI standards check and focused native component/binding tests at 900×600 and the supported scale choices. Present the native preview directly for owner review under the repository UI policy before the candidate is delivered.
- [x] 2.4 Document the Rack mapping, gate priming, 15-second limit, WAV access, and expected failure/rollback behavior in current technical guidance. Verify those instructions against the native fixture and update the offline guide currency through its existing workflow.

## 3. Host validation and candidate readiness

- [ ] 3.1 Re-run the Rack boundary fixture with the ReverbArise VST3 at 48 kHz and verify WAV frame count and stereo samples. Capture an actual repeating synth phrase. Verify held-gate startup, cancel, editor reopen and host reset/disconnect behavior, and record the measured limitations.
- [ ] 3.2 Run a focused FL Studio Song smoke check against an earlier saved project and verify default capture, recapture, DAW Stop settlement and unchanged accepted source and edit identities. Verify rollback with a disposable copy.
- [ ] 3.3 Run native Windows Release and the required test gates. Record Rack and FL Studio evidence separately from fixtures, and author build-specific candidate notes against the last build shared with Stuart.

## Workflow follow-up

- Commit the intended source, evidence and release inputs on the issue branch. Open a PR with `Refs #116`.
- Package a clean committed candidate with the existing Windows packager. Verify the extracted bundle, manifest, notes, guide and ZIP hash. Provide the package and concise install, test and rollback instructions for Stuart.
- Keep #116 open for Stuart's acceptance and the later Loop workflow.
