import json
import subprocess
from pathlib import Path

here = Path(__file__).resolve().parent
worktree = here.parents[3]
bundle = worktree/'out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3'
assert bundle.is_dir(), bundle
baseline = json.loads((here/'patch.json').read_text())
patch = {**baseline, 'modules': baseline['modules'][:5],
         'cables': [c for c in baseline['cables']
                    if c['inputModuleId'] <= 5 and c['outputModuleId'] <= 5]}
clock, osc, vca, host, reset = patch['modules']
for item in clock['params']:
    if item['id'] == 3:
        item['value'] = 137
    if item['id'] == 2:
        item['value'] = -17
clock['data']['trigOuts'] = [True,False,True,False]
host['data'].update(pluginPath=bundle.as_posix(),pluginName='ReverbArise',
                    parameterIds=[-485,-487,-480,-478,-477]+[-1]*11)
for cable in patch['cables']:
    if cable['inputModuleId']==host['id']:
        note_port = cable['inputId']
        if note_port==0:
            cable['outputModuleId'],cable['outputId']=clock['id'],3
        if note_port==1:
            cable['outputModuleId'],cable['outputId']=clock['id'],2
        if note_port==2:
            cable['outputModuleId'],cable['outputId']=clock['id'],1
        if note_port==3:
            cable['outputModuleId'],cable['outputId']=clock['id'],0
# The initial timing probe used note 14 for reset. Reassign that cable to
# the dedicated reset note, then add the actual gate on the first port.
for cable in patch['cables']:
    if cable['inputModuleId']==host['id'] and cable['inputId']==0:
        cable['inputId'],cable['outputId']=4,4
        break
patch['cables'].append(dict(id=max(c['id'] for c in patch['cables'])+1,
    outputModuleId=clock['id'],outputId=3,inputModuleId=host['id'],inputId=0,color='#c91847'))
folder=here/'learn-patch'
folder.mkdir(exist_ok=True)
(folder/'patch.json').write_text(json.dumps(patch),encoding='utf-8')
subprocess.run(['tar','-c','--zstd','-f',str(here/'ReverbArise-CLKD-Learn.vcv'),
                '-C',str(folder),'patch.json'],check=True)
