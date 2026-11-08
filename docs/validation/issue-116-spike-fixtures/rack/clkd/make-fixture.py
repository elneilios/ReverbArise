import json
import subprocess
from pathlib import Path

here = Path(__file__).resolve().parent
probe = here.parent / 'build/rack_clkd_trace_artefacts/Release/VST3/Rack CLKD Capture Trace.vst3'
modules = []
cables = []


def add(plugin, model, version, pos, params=(), data=None):
    module = {'id': len(modules) + 1, 'plugin': plugin, 'model': model,
              'version': version, 'pos': pos,
              'params': [{'id': i, 'value': value} for i, value in enumerate(params)]}
    if data is not None:
        module['data'] = data
    modules.append(module)
    return module['id']


def cable(source, output, dest, input_port, color='#0986ad'):
    cables.append(dict(id=len(cables)+1, outputModuleId=source, outputId=output,
                       inputModuleId=dest, inputId=input_port, color=color))


def lane(tempo, block, on_bar, row):
    clock = add('ImpromptuModular', 'Clocked-Clkd', '2.5.0', [0, row],
                [-5, -17, -2, tempo],
                {'running': True, 'trigOuts': [True]*4, 'resetClockOutputsHigh': False})
    osc = add('Fundamental', 'VCO', '2.6.4', [8, row])
    vca = add('Fundamental', 'VCA', '2.6.4', [18, row])
    host = add('VCV-Host', 'Host-FX', '2.1.0', [30, row], data={
        'parameterIds': [-485, -487, -483, -482] + [-1]*12,
        'subPluginId': 0, 'blockSize': block, 'bypass': False, 'worker': True,
        'pluginPath': probe.as_posix(), 'pluginName': 'Rack CLKD Capture Trace',
        'busChannelMappings': [[[0, 1]], [[0, 1]]],
    })
    # /4 and /16 clock two audible VCO lanes through the VCA. The controls
    # reach Host-FX directly, so the VCA adds one Rack engine sample to audio.
    for audio_port, divider, control in [(0, 1, 2), (1, 2, 1)]:
        cable(osc, 3, vca, [2, 5][audio_port])
        cable(clock, divider, vca, [1, 4][audio_port], '#ffb437')
        cable(clock, divider, host, control, '#ffb437')
        cable(vca, audio_port, host, 16 + audio_port)
    cable(clock, 1 if on_bar else 3, host, 3, '#c91847')
    reset = add('Fundamental', 'Push', '2.6.4', [45, row])
    cable(reset, 0, clock, 0, '#0c8e15')
    cable(clock, 4, host, 0, '#0c8e15')


lane(120, 256, False, 0)
lane(137, 256, False, 1)
lane(137, 512, True, 2)
lane(280, 512, False, 3)

patch = dict(version='2.6.6', path='', zoom=0.8, gridOffset=[0, 0],
             modules=modules, cables=cables)
(here / 'patch.json').write_text(json.dumps(patch), encoding='utf-8')
subprocess.run(['tar', '-c', '--zstd', '-f', str(here / 'clkd-bar-stop.vcv'),
                '-C', str(here), 'patch.json'], check=True)
