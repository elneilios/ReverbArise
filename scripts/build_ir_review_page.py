"""Build a local, dependency-free listening page and calibration evidence."""
import argparse
import html
import json
import math
from pathlib import Path
import statistics
import wave


def stereo(path):
    with wave.open(str(path), "rb") as f:
        assert f.getnchannels()==2 and f.getsampwidth()==3
        pcm=f.readframes(f.getnframes())
    ll=rr=lr=0.
    for i in range(0,len(pcm),6):
        l=int.from_bytes(pcm[i:i+3],"little",signed=True)/8388608.
        r=int.from_bytes(pcm[i+3:i+6],"little",signed=True)/8388608.
        ll+=l*l;rr+=r*r;lr+=l*r
    mid=(ll+rr+2*lr)/4;side=(ll+rr-2*lr)/4
    return dict(correlation=lr/math.sqrt(ll*rr),side_to_mid_db=10*math.log10(max(1e-30,side)/mid),
                mono_vs_mean_channel_db=10*math.log10(mid/((ll+rr)/2)))


def build(folder):
    report = json.loads((folder / "render-report.json").read_text(encoding="utf-8-sig"))
    baseline = report["responses"][0]["fixtures"]
    rows = []
    cards = []
    names = {"preverb-hall":"Preverb Hall (unchanged reference)","monumental":"Monumental — 10 s",
             "intimate-plate":"Intimate Plate — 1 s","vintage-dark-hall":"Vintage Dark Hall — 4 s",
             "shimmer-cloud":"Shimmer Cloud — 7 s, static coloration","whisper":"Whisper — 0.5 s"}
    for response in report["responses"]:
        row = dict(id=response["id"], fixtures=[])
        for index, fixture in enumerate(response["fixtures"]):
            entry = dict(fixture=fixture["fixture"],
                         full_active_delta_db=fixture["shaped"]["active_rms_dbfs"]-baseline[index]["shaped"]["active_rms_dbfs"],
                         common_crop_active_delta_db=fixture["common_250ms_preroll"]["active_rms_dbfs"]-baseline[index]["common_250ms_preroll"]["active_rms_dbfs"],
                         wet_guard_db=fixture["wet_attenuation_db"], sum_guard_db=fixture["sum_attenuation_db"],
                         raw_peak_dbfs=fixture["raw"]["peak_dbfs"],shaped_peak_dbfs=fixture["shaped"]["peak_dbfs"])
            entry["protected_wet_stereo"]=stereo(folder/response["id"]/(fixture["fixture"]+"-wet.wav"))
            row["fixtures"].append(entry)
        row["median_full_active_delta_db"] = statistics.median(v["full_active_delta_db"] for v in row["fixtures"])
        rows.append(row)
        ident=html.escape(response["id"],quote=True)
        cards.append(f'<article><h2>{html.escape(names[response["id"]])}</h2><audio controls preload="none" data-id="{ident}" src="{ident}/piano-mix.wav"></audio><p>Individual clip starts at its wet lead-in; dry event follows. <a href="{ident}-comparison.wav">Aligned four-source comparison (60 s)</a></p></article>')
        if response["id"] in ("monumental","vintage-dark-hall"):
            previous='../ir-library-listening-v3/'+ident
            cards.append(f'<details><summary>Compare with the previous revision of {html.escape(names[response["id"]])}</summary><audio controls preload="none" data-id="{previous}" src="{previous}/piano-mix.wav"></audio></details>')
    summary=dict(prototype=True, approval="Pending user listening", window=report["active_window"], monitoring=report["monitoring"],
                 notes="Level deltas are source-specific; median calibration does not imply equal perceived loudness. Common crop: 250 ms pre-roll / 20 ms release.", responses=rows)
    (folder / "calibration-summary.json").write_text(json.dumps(summary,indent=2)+"\n",encoding="utf-8")
    page='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>ReverbArise IR prototypes</title>
<style>body{font:16px system-ui;background:#0f1418;color:#f8fafc;max-width:900px;margin:32px auto;padding:16px}h1{color:#00efff}article{border-top:1px solid #4a5058;padding:12px 0}h2{font-size:18px}p{line-height:1.5}a{color:#00efff}audio{width:100%}select{padding:8px;font:inherit;background:#182127;color:#f8fafc;border:1px solid #6b737c;margin-right:12px}label{display:inline-block;margin-bottom:20px}</style>
<h1>IR prototype listening review — revision 4</h1><p>Review Monumental and Vintage Dark Hall. Monumental has a denser, stronger final build, more authored level and broad stereo diffusion. Vintage Dark Hall has low resonant warmth and a deliberately darker full response. Plate, Shimmer and Whisper are the exact sounds accepted in revision 3; their WAV hashes are unchanged.</p><p>Original synthetic sources, CC0. Compare each candidate with Hall on the same source. Production reverse convolution, 20 ms wet release, zero trim and disclosed monitoring protection. Monumental intentionally targets a more forceful level, not equal loudness with the subtle responses. Final approval of these two sounds is pending.</p>
<p>Listen for character, useful swell length, level and transition. Long/dark responses are stronger on pads; Plate/Whisper intentionally have less low-frequency sustain. Shimmer Cloud is static coloration, not pitch shifting. Individual clips have different lead-ins; aligned comparison files put dry events at 12, 27, 42 and 57 s.</p>
<label>Source <select id="fixture"><option value="piano">Piano-like chord</option><option value="pad">Pad</option><option value="voice-like">Synthetic voice-like phrase</option><option value="pluck">Pluck</option></select></label>
<label>Listen <select id="mode"><option value="mix">Dry + Wet (50%)</option><option value="wet">Wet only</option><option value="dry">Dry source</option><option value="forward">Forward reference (separate safety gain)</option></select></label>
'''+''.join(cards)+'''<p><a href="calibration-summary.json">Calibration summary</a> · <a href="render-report.json">Full production render measurements</a></p>
<script>const audio=[...document.querySelectorAll('audio')];function change(){for(const a of audio){a.pause();a.src=a.dataset.id+'/'+document.querySelector('#fixture').value+'-'+document.querySelector('#mode').value+'.wav';a.load()}}document.querySelectorAll('select').forEach(s=>s.onchange=change);audio.forEach(a=>a.onplay=()=>audio.filter(b=>b!==a).forEach(b=>b.pause()));</script></html>'''
    (folder / "index.html").write_text(page,encoding="utf-8")
    for row in rows:
        print(row["id"],"full delta dB",[round(v["full_active_delta_db"],1) for v in row["fixtures"]],"median",round(row["median_full_active_delta_db"],2))


if __name__ == "__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("folder",type=Path,nargs="?",default=Path("out/ir-library-listening-v4"))
    build(parser.parse_args().folder)
