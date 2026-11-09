"""Build local manual, sequenced and trace patches without bundling Rack licenses."""

import argparse
import json
import subprocess
from pathlib import Path
from tempfile import TemporaryDirectory


def create_patch(bundle, output, variant, audio_device=None):
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

    manual = variant == "manual"
    probe = variant == "probe"
    clock = module("ImpromptuModular", "Clocked-Clkd", "2.5.0", 10, [(3, 137)],
                   running=manual or probe, trigOuts=[False] * 4, resetClockOutputsHigh=False)
    source, beat = clock, 0
    if not manual:
        sequencer = module("Fundamental", "SEQ3", "2.6.4", 22, [(3, 4)], running=True,
                           clockPassthrough=True)
        connect(clock, 0, sequencer, 1)
        source, beat = sequencer, 13

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

    notes = [23] if manual else [23, 24, 25]
    if probe:
        notes += [21, 17]
    host = module("VCV-Host", "Host-FX", "2.1.0", 10,
                  parameterIds=[n - 499 for n in notes] + [-1] * (16 - len(notes)),
                  subPluginId=0, blockSize=256, bypass=False, worker=True,
                  pluginPath=bundle.as_posix(), pluginName=bundle.stem,
                  busChannelMappings=[[[0, 1]], [[0, 1]]])
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
            "In Learn, turn on Loop Mode and select End Signal. Click Arm to start. "
            "Click Arm off while recording to finish at the end of the bar."
            if manual else
            "In Learn, turn on Loop Mode and select End Signal, then press Run on Clkd. SEQ3 Step 1 arms the take. "
            "Step 4 requests finish. The next Step 1 closes the bar."
        )
        module("Core", "Notes", "2.6.6", 16, text=(
            f"REVERBARISE LOOP / {variant.upper()}\n\n"
            "Select your speakers in Audio 2 if needed. Click the Host-FX eye to open Learn.\n\n"
            f"{workflow}\n\n"
            "Recapture Audio off protects an accepted Loop. Switch it on to replace the Loop. "
            "If the clock stops before the closing beat, the incomplete take is discarded."
        ))
    patch = dict(version="2.6.6", path="", zoom=0.85, gridOffset=[0, 0], modules=modules, cables=cables)
    output.parent.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(dir=output.parent) as temp:
        (Path(temp) / "patch.json").write_text(json.dumps(patch, indent=2), encoding="utf-8")
        subprocess.run(["tar", "-c", "--zstd", "-f", str(output), "-C", temp, "patch.json"], check=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("variant", choices=["manual", "sequenced", "probe"])
    parser.add_argument("--bundle", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--asio-device")
    args = parser.parse_args()
    create_patch(args.bundle.resolve(strict=True), args.output.resolve(), args.variant, args.asio_device)
