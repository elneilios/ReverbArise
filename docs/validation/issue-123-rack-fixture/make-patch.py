"""Generate a Rack patch using the #116 CLKD test-control VST3."""

import argparse
import json
import subprocess
from pathlib import Path


def make_patch(probe, control, output, mask_phrase):
    modules = []
    cables = []

    def module(plugin, model, version, x, row, params=(), **data):
        module_id = len(modules) + 1
        modules.append(dict(id=module_id, plugin=plugin, model=model, version=version,
                            pos=[x, row], params=[dict(id=i, value=value)
                                                 for i, value in enumerate(params)], data=data))
        return module_id

    def connect(source, source_port, target, target_port):
        cables.append(dict(id=len(cables) + 1, outputModuleId=source, outputId=source_port,
                           inputModuleId=target, inputId=target_port, color="#ffb437"))

    controller = module("VCV-Host", "Host-XL", "2.1.0", 0, 0,
                        parameterIds=[-1] * 24, subPluginId=0, blockSize=256,
                        bypass=False, worker=True, pluginPath=control.as_posix(),
                        pluginName="Rack CLKD Test Controls",
                        busChannelMappings=[[[0]], [[0, 1, 2, 3, 4, 5, 6, 7]]])
    notes = [14, 12, 19, 21, 22, 16, 17]
    for row, (tempo, host_buffer) in enumerate(((137, 256), (280, 512))):
        clock = module("ImpromptuModular", "Clocked-Clkd", "2.5.0", 26, row,
                       [-5, -17, -2, tempo], running=True,
                       trigOuts=[True, True, True, False], resetClockOutputsHigh=False)
        observer = module("VCV-Host", "Host-FX", "2.1.0", 52, row,
                          parameterIds=[note - 499 for note in notes] + [-1] * 9,
                          subPluginId=0, blockSize=host_buffer, bypass=False, worker=True,
                          pluginPath=probe.as_posix(), pluginName="Rack Host Admission Trace",
                          busChannelMappings=[[[0, 1]], [[0, 1]]])
        connect(controller, 0, clock, 0)
        connect(controller, 2, observer, 5)
        connect(controller, 3, observer, 6)
        for source_port, target_port in ((3, 0), (1, 2), (0, 3), (4, 4), (1, 17)):
            connect(clock, source_port, observer, target_port)
        if mask_phrase:
            mask = module("Fundamental", "VCA", "2.6.4", 40, row)
            connect(controller, 1, mask, 1)
            connect(clock, 2, mask, 2)
            connect(mask, 0, observer, 1)
        else:
            connect(clock, 2, observer, 1)
        connect(clock, 2, observer, 16)

    patch = dict(version="2.6.6", path="", zoom=0.8, gridOffset=[0, 0],
                 modules=modules, cables=cables)
    output.parent.mkdir(parents=True, exist_ok=True)
    patch_json = output.parent / "patch.json"
    patch_json.write_text(json.dumps(patch), encoding="utf-8")
    subprocess.run(["tar", "-c", "--zstd", "-f", str(output),
                    "-C", str(patch_json.parent), patch_json.name], check=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", type=Path, required=True)
    parser.add_argument("--control", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--mask-phrase", action="store_true")
    args = parser.parse_args()
    make_patch(args.probe.resolve(), args.control.resolve(strict=True),
               args.output.resolve(), args.mask_phrase)
