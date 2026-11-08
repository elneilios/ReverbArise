import json
import subprocess
from pathlib import Path

here = Path(__file__).resolve().parent
build = here.parent / 'build'
modules, cables = [], []


def module(plugin, model, version, pos, params=(), data=None):
    result = len(modules)+1
    modules.append({'id': result, 'plugin': plugin, 'model': model,
                    'version': version, 'pos': pos,
                    'params': [dict(id=i,value=v) for i,v in enumerate(params)]})
    if data is not None:
        modules[-1]['data'] = data
    return result


def cable(source, out, target, port, color='#0986ad'):
    cables.append(dict(id=len(cables)+1, outputModuleId=source, outputId=out,
                       inputModuleId=target,inputId=port,color=color))


def host(model, target, name, row, x, block, parameters, buses):
    bundle=build / f'{target}_artefacts/Release/VST3/{name}.vst3'
    return module('VCV-Host',model,'2.1.0',[x,row],data=dict(
        parameterIds=parameters, subPluginId=0, blockSize=block,
        bypass=False, worker=True, pluginPath=bundle.as_posix(),
        pluginName=name, busChannelMappings=buses))


control=host('Host-XL','rack_clkd_control','Rack CLKD Test Controls',
             0,0,256,[-1]*24,[[[0]],[[0,1,2,3,4,5,6,7]]])

for row,(tempo,block) in enumerate([(137,256),(280,512)]):
    clock=module('ImpromptuModular','Clocked-Clkd','2.5.0',[26,row],[-5,-17,-2,tempo],
                 dict(running=True,trigOuts=[True,False,True,True],resetClockOutputsHigh=False))
    osc=module('Fundamental','VCO','2.6.4',[34,row])
    audio=module('Fundamental','VCA','2.6.4',[46,row])
    markers=module('Fundamental','VCA','2.6.4',[56,row])
    observer=host('Host-FX','rack_clkd_cue','Rack CLKD Cue Trace',row,66,block,
                  [-487,-480,-483,-482,-477]+[-1]*11,[[[0,1]],[[0,1]]])
    cable(control,0,clock,0,'#0c8e15')
    cable(clock,4,observer,4,'#0c8e15')
    cable(control,2,observer,2,'#c91847')
    cable(control,3,observer,3,'#c91847')
    cable(clock,2,markers,2,'#ffb437')
    cable(control,1,markers,1,'#ffb437')
    cable(clock,1,markers,5,'#ffb437')
    cable(markers,0,observer,0,'#ffb437')
    cable(markers,1,observer,1,'#ffb437')
    for i in range(2):
        cable(osc,3,audio,[2,5][i])
        cable(clock,i+1,audio,[1,4][i],'#ffb437')
        cable(audio,i,observer,16+i)

patch=dict(version='2.6.6',path='',zoom=0.8,gridOffset=[0,0],modules=modules,cables=cables)
folder=here/'cue-patch'
folder.mkdir(exist_ok=True)
(folder/'patch.json').write_text(json.dumps(patch),encoding='utf-8')
subprocess.run(['tar','-c','--zstd','-f',str(here/'clkd-cued-playback.vcv'),
                '-C',str(folder),'patch.json'],check=True)
