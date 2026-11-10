"""Build local capture, Perform and trace patches without bundling Rack licenses."""

import argparse
import json
import subprocess
from pathlib import Path
from tempfile import TemporaryDirectory


def create_patch(bundle, output, variant, audio_device=None, steps=16):
    modules = []
    cables = []
    position = 0

    def module(plugin, model, version, width, params=(), **data):
        nonlocal position
        identifier = len(modules) + 1
        modules.append(dict(id=identifier, plugin=plugin, model=model, version=version,
                            pos=[position, 0], params=[dict(id=i, value=v) for i, v in params], data=data))
        position += width
        return identifier

    def connect(source, outlet, target, inlet, timing=True):
        cables.append(dict(id=len(cables) + 1, outputModuleId=source, outputId=outlet,
                           inputModuleId=target, inputId=inlet,
                           color="#ffb437" if timing else "#1ebc8b"))

    clocked_end_signal = variant == "clocked-end-signal"
    clocked_repeat = variant == "clocked-end-on-repeat"
    clocked = clocked_end_signal or clocked_repeat
    perform_sequence = variant == "perform-sequenced"
    perform = variant == "perform" or perform_sequence
    manual = variant == "manual" or perform or clocked
    probe = variant == "probe"
    if clocked:
        # Master at 137 BPM, CLK 1 and 2 divided by 8, CLK 3 divided by 4.
        # CLK 2's last delay setting offsets Stop by three quarters of its cycle.
        clock_params = [(0, 137), (1, -9), (2, -9), (3, -5), (16, 7)]
        clock = module("ImpromptuModular", "Clocked", "2.5.0", 20, clock_params,
                       running=False, resetClockOutputsHigh=False, displayDelayNoteMode=False)
    else:
        clock_params = [(1, -17), (2, -5), (3, 137)] if variant == "perform" else [(3, 137)]
        clock = module("ImpromptuModular", "Clocked-Clkd", "2.5.0", 10, clock_params,
                       running=variant in ("manual", "probe"),
                       trigOuts=[False, False, True, False] if variant == "perform" else [False] * 4,
                       resetClockOutputsHigh=False)
    source, beat = clock, 0
    if not manual:
        sequencer = module("Fundamental", "SEQ3", "2.6.4", 22, [(3, 4)], running=True,
                           clockPassthrough=True)
        connect(clock, 0, sequencer, 1)
        source, beat = sequencer, 13

    if perform_sequence:
        # GateSeq64 in 4x16 mode. At one pulse per step, enabled gates
        # follow the input clock, so all four Host-FX inputs align.
        default_gate, enabled_gate = 50, 50 | 0x200
        attributes = [default_gate] * 64
        for track, active_steps in enumerate(([0], [steps - 1], range(steps), range(0, steps, 4))):
            for step in active_steps:
                attributes[track * 16 + step] = enabled_gate
        sequencer = module("ImpromptuModular", "Gate-Seq-64", "2.5.0", 24,
                           [(66, 0), (71, 1)], running=True, pulsesPerStep=1,
                           sequences=[steps], attributes2=attributes)
        connect(clock, 0, sequencer, 0)
    if perform or clocked:
        noise = module("Fundamental", "Noise", "2.6.4", 5)
        envelope = module("Fundamental", "ADSR", "2.6.4", 12,
                          [(0, 0.0), (1, 0.42), (2, 0.0), (3, 0.2)])
        level = module("Fundamental", "VCA", "2.6.4", 5, [(0, 0.42)])
        # A bar gate gives the snare envelope time to decay.
        connect(sequencer if perform_sequence else clock, 3, envelope, 4)
        connect(envelope, 0, level, 1)
        connect(noise, 0, level, 2, False)
    else:
        if not probe:
            oscillator = module("Fundamental", "VCO", "2.6.4", 9, [(2, -12)])
        level = module("Fundamental", "VCA", "2.6.4", 5, [(0, 0.24), (1, 1)])
        connect(source, beat, level, 5)
        if not probe:
            connect(oscillator, 0, level, 2, False)
            connect(source, beat, level, 1)
    if not manual:
        gates = module("Fundamental", "VCA", "2.6.4", 5)
        # Step gates must coincide with clock high, including at patch load.
        for step, signal, cv in [(4, 2, 1), (7, 5, 4)]:
            connect(sequencer, step, gates, signal)
            connect(source, beat, gates, cv)

    if perform_sequence:
        notes = [23, 24, 25, 12]
    elif clocked_repeat:
        notes = [23, 12]
    elif clocked_end_signal:
        notes = [23, 24, 25]
    elif perform:
        notes = [23]
    else:
        notes = [23] if manual else [23, 24, 25]
    if probe:
        notes += [21, 17]
    host = module("VCV-Host", "Host-FX", "2.1.0", 10,
                  parameterIds=[n - 499 for n in notes] + [-1] * (16 - len(notes)),
                  subPluginId=0, blockSize=256, bypass=False, worker=True,
                  pluginPath=bundle.as_posix(), pluginName=bundle.stem,
                  busChannelMappings=[[[0, 1]], [[0, 1]]])
    if perform_sequence:
        for inlet, track in enumerate((2, 0, 1, 0)):
            connect(sequencer, track, host, inlet)
    elif clocked:
        for channel in range(2 if clocked_repeat else 3):
            connect(clock, channel, host, channel)
    elif perform:
        connect(clock, 0, host, 0)  # Clock supplies the beat in Learn and Perform.
    else:
        connect(level, 1, host, 0)
    if not manual:
        connect(gates, 0, host, 1)
        connect(gates, 1, host, 2)

    if probe:
        connect(level, 1, host, 3)
        connect(gates, 1, host, 4)
        connect(gates, 0, host, 16, False)
        connect(gates, 1, host, 17, False)
    else:
        audio = {"driver": 6, "inputOffset": 0, "outputOffset": 0}
        if audio_device:
            audio = dict(driver=7, deviceName=audio_device, sampleRate=48000, blockSize=256,
                         inputOffset=0, outputOffset=0)
        output_module = module("Core", "AudioInterface2", "2.6.6", 5, audio=audio, dcFilter=True)
        for channel in (0, 1):
            connect(level, 0, host, 16 + channel, False)
            connect(host, channel, output_module, channel, False)
        workflow = (
            "In Learn, turn on Loop Mode and select End on Signal. Click Arm to start. "
            "Click Arm off while recording to finish at the end of the bar."
            if manual else
            "In Learn, turn on Loop Mode and select End on Signal, then press Run on Clkd. SEQ3 Step 1 arms the take. "
            "Step 4 requests finish. The next Step 1 closes the bar."
        )
        instructions = (
            f"REVERBARISE LOOP / {variant.upper()}\n\n"
            "Select your speakers in Audio 2 if needed. Click the Host-FX eye to open Learn.\n\n"
            "Host-FX Note Gate (MIDI channel 1)\n"
            "B0 (23) = Clock\nC1 (24) = Start/Arm\nC#1 (25) = Stop\n\n"
            f"{workflow}\n\n"
            "Recapture Audio off protects an accepted Loop. Switch it on to replace the Loop. "
            "If the clock stops before the closing beat, the incomplete take is discarded."
        )
        if clocked_repeat:
            instructions = (
                "REVERBARISE / END ON REPEAT\n\n"
                "Select your output in Audio 2 at 48 kHz. Open the Host-FX eye. "
                "In Learn, turn on Loop Mode, select End on repeat, and click Arm.\n\n"
                "Press Run on Clocked. Master goes to B0 (Clock). "
                "CLK 1 /8 goes to C0 (Phrase) and marks each two-bar repeat. "
                "The next Phrase pulse closes the take. CLK 3 plays the snare once per bar."
            )
        elif clocked_end_signal:
            instructions = (
                "REVERBARISE / TWO-BAR CLOCKED END SIGNAL\n\n"
                "Select your output in Audio 2 at 48 kHz. Open the Host-FX eye. "
                "In Learn, turn on Loop Mode, select End on Signal, and leave Arm off.\n\n"
                "Press Run on Clocked and keep it running through the take. "
                "Master goes to B0 (Clock), CLK 1 /8 goes to C1 (Start/Arm), "
                "and CLK 2 /8 with 3/4 delay goes to C#1 (Stop). Beat 9 closes the take. "
                "CLK 3 plays the snare once per bar.\n\n"
                "Leave Recapture Audio off to keep the first accepted take."
            )
        elif perform_sequence:
            instructions = (
                f"REVERBARISE / {steps}-STEP END SIGNAL\n\n"
                "Select 48 kHz audio. Open the Host-FX eye. In Learn, turn on Loop Mode "
                "and choose End on Signal if needed.\n\n"
                "Press RUN on Clkd and wait for Step 1. The sequencer does the arming. "
                "The Arm button in Learn starts a manual take on the next beat.\n\n"
                f"Step 1 arms and sets Phrase. Step {steps} requests Stop. "
                f"The next Step 1 closes the {steps // 4}-bar take. "
                "Recapture Audio off keeps the accepted take on later cycles.\n\n"
                "Keep Clkd running while you generate swells in Learn. "
                "Go to Perform and click Cue Loop when Ready."
            )
        elif perform:
            instructions = (
                "REVERBARISE / LOOP PERFORM\n\n"
                "Use 48 kHz and leave Clkd at 137 BPM. Select your output device in Audio 2. "
                "You should hear a short snare-like hit once per bar. Open ReverbArise with the Host-FX eye.\n\n"
                "Learn: turn on Loop Mode, set Duration to 4 Bars, click Arm and press Run on Clkd. "
                "After Loop accepted, review the detected markers and click Generate Swells.\n\n"
                "Perform: wait for Ready, then click Cue Loop. While waiting you can click Cancel Loop. "
                "Stop Loop finishes the current pass. Click Stop Now for an immediate fade.\n\n"
                "Host-FX input 1: Clock (B0). Green cables carry audio."
            )
        module("Core", "Notes", "2.6.6", 16, text=instructions)
    patch = dict(version="2.6.6", path="", zoom=0.85, gridOffset=[0, 0], modules=modules, cables=cables)
    output.parent.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(dir=output.parent) as temp:
        (Path(temp) / "patch.json").write_text(json.dumps(patch, indent=2), encoding="utf-8")
        subprocess.run(["tar", "-c", "--zstd", "-f", str(output), "-C", temp, "patch.json"], check=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("variant", choices=["manual", "sequenced", "probe", "perform", "perform-sequenced",
                                            "clocked-end-signal", "clocked-end-on-repeat"])
    parser.add_argument("--bundle", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--asio-device")
    parser.add_argument("--steps", type=int, choices=[4, 8, 16], default=16)
    args = parser.parse_args()
    create_patch(args.bundle.resolve(strict=True), args.output.resolve(), args.variant, args.asio_device, args.steps)
