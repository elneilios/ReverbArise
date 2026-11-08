"""Compare CLKD marks, captured dry audio and emitted wet in two live traces."""
import json
import mmap
import struct
from bisect import bisect_left
from collections import Counter
from pathlib import Path

root = Path(__file__).resolve().parent
old = set((root/'previous-cue-traces.txt').read_text().splitlines())
paths = [path for path in (root.parent/'traces').glob('clkd-cue-*.f32') if path.name not in old]
assert len(paths) == 2, paths
frame = struct.Struct('<ffffI')
rate, lead = 48000, 2048
phrase, bar, cue, stop, reset, locked, queued, pre, down, lost, fading = (1 << n for n in range(11))


def verify(path):
    callback, period = map(int,path.with_suffix('.txt').read_text(encoding='utf-8').split())
    with path.open('rb') as stream, mmap.mmap(stream.fileno(),0,access=mmap.ACCESS_READ) as data:
        assert len(data) == 96*rate*frame.size
        events = {mask: [] for mask in (phrase, bar, cue, stop, reset, down, lost)}
        dry_onsets, preroll_starts = [], []
        previous = 0
        previous_audio = -1000
        for i,(left,right,out_left,out_right,flags) in enumerate(frame.iter_unpack(data)):
            assert right == out_right, i
            mask = flags & (phrase|bar|cue|stop|reset|down|lost)
            if mask:
                for bit, indices in events.items():
                    if mask & bit:
                        indices.append(i)
            if abs(right) > .01 and i-previous_audio > 100:
                dry_onsets.append(i)
            if abs(right) > .01:
                previous_audio = i
            if flags & pre and not previous & pre:
                preroll_starts.append(i)
            if not flags & (pre|fading) and not flags & down:
                # The 256-frame positive wet tail follows an actual downbeat.
                if not events[down] or i-events[down][-1] >= 256:
                    assert left == out_left, (i,left,out_left,flags)
            previous=flags

        common = sorted(set(events[phrase]) & set(events[bar]))
        source = common[0]
        assert common[1]-source == period
        assert events[reset] and events[lost] and len(events[cue]) >= 3 and len(events[stop]) == 1
        assert len(events[down]) >= 3, events
        read = lambda index: frame.unpack_from(data,index*frame.size)
        worst, alignment, padding = 0.0, Counter(), Counter()
        for marker in events[down]:
            assert marker in common, marker
            near = bisect_left(dry_onsets,marker-2)
            assert near<len(dry_onsets) and abs(dry_onsets[near]-marker)<=2, marker
            alignment[dry_onsets[near]-marker] += 1
            lead_start = preroll_starts[bisect_left(preroll_starts,marker)-1]
            length = marker-lead_start
            assert lead <= length <= lead+2, (marker, length)
            padding[length-lead] += 1
            for i in range(length):
                left,_,output,_,flags = read(lead_start+i)
                expected=read(source+lead-1-min(i,lead-1))[0]*.25
                assert flags & pre
                worst=max(worst,abs(output-left-expected))
            for i in range(256):
                left,_,output,_,_=read(marker+i)
                worst=max(worst,abs(output-left-read(source+i)[0]*.25))
        assert worst<7e-8, worst
        expected_losses=sorted(events[lost])
        injected_reset=events[reset][0]
        assert any(abs(at-injected_reset)<=1 for at in expected_losses), events
        assert any(at>53*rate for at in expected_losses), events
        for interrupted in expected_losses:
            next_cue=next((at for at in events[cue] if at>interrupted),96*rate)
            assert not any(interrupted<at<next_cue for at in events[down]), events
        return dict(file=path.name,callback_frames=callback,source_frames=period,
                    phrase_interval_counts=dict(sorted(Counter(b-a for a,b in zip(common,common[1:])
                        if abs(b-a-period)<=1).items())),
                    cue_samples=events[cue], reset_samples=events[reset], sync_loss_samples=events[lost],
                    downbeat_samples=events[down], downbeat_audio_minus_marker=dict(alignment),
                    extra_lead_hold_samples=dict(sorted(padding.items())), mix_error=worst,
                    stop_sample=events[stop][0])


results = sorted(map(verify,paths),key=lambda entry: entry['callback_frames'])
summary = json.dumps(results,indent=2)
(root/'cue-results.json').write_text(summary+'\n',encoding='utf-8')
print(summary)
