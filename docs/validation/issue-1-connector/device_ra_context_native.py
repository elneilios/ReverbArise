# name=ReverbArise Native Context Probe v1
"""Disposable native spike. Assign explicitly to a test MIDI input; never production.
No MIDI handlers are overridden. Script discovery does not activate a device.
"""
import csv
import os
import sys
import time

TARGET = "ReverbArise FL Context Probe"
PARAMS = ("RA Probe Context v1", "RA Probe Audio Ack v1",
          "RA Probe Query Clock v1", "RA Probe Write Clock v1")
MAX_WORD = 0xFFFFFF
EVIDENCE_DIRECTORY = os.path.join(os.path.dirname(__file__), "evidence")
_targets = []
_sequence = 0
_last_mode = None
_last_send = -1.0
_last_discovery = -1.0
_paused = False
_sending = False
_rows = []
_dropped = 0
_log_file = None


def _record(event, flags=0, track=-1, slot=-1, seq=0, mode=-1, ack=-1, detail=""):
    global _dropped
    if len(_rows) < 4096:
        _rows.append((time.perf_counter(), event, flags, track, slot, seq, mode, ack, detail))
    else:
        _dropped += 1


def _flush():
    global _rows
    if _log_file and _rows:
        with open(_log_file, "a", newline="") as output:
            csv.writer(output).writerows(_rows)
        _rows = []


def _identify(track, slot):
    import plugins
    if not plugins.isValid(track, slot) or plugins.getPluginName(track, slot, 0) != TARGET:
        return None
    count = plugins.getParamCount(track, slot)
    if not 4 <= count <= 512:
        _record("schema-error", track=track, slot=slot, detail="parameter count %s" % count)
        return None
    names = [plugins.getParamName(i, track, slot) for i in range(count)]
    if any(names.count(name) != 1 for name in PARAMS):
        _record("schema-error", track=track, slot=slot, detail=str(names))
        print("RA probe parameter names differ:", names)
        return None
    # Wrapper bypass/program parameters may be present; never assume numeric indexes.
    return tuple(names.index(name) for name in PARAMS)


def _matches(track, slot, indices):
    import plugins
    return (plugins.isValid(track, slot)
            and plugins.getPluginName(track, slot, 0) == TARGET
            and tuple(plugins.getParamName(i, track, slot) for i in indices) == PARAMS)


def _discover():
    global _targets, _last_discovery
    import mixer
    found = []
    count = mixer.trackCount()
    if not 0 <= count <= 512:
        raise RuntimeError("Unexpected mixer size")
    for track in range(count):
        for slot in range(10):
            try:
                indices = _identify(track, slot)
                if indices is not None:
                    found.append((track, slot, indices))
            except Exception as error:
                _record("discover-error", track=track, slot=slot, detail=str(error))
    if found != _targets:
        print("RA probe targets:", found, "(zero means not discovered; check exact name/parameters)")
    _targets, _last_discovery = found, time.perf_counter()
    _record("discover", detail=str(found))


def _clock_value():
    return (int(time.perf_counter() * 8000) & MAX_WORD) / MAX_WORD


def _update(event, flags=0, force_unknown=False):
    global _sequence, _last_mode, _last_send, _sending
    import plugins
    import transport
    if _paused or _sending:
        return
    now = time.perf_counter()
    raw = transport.getLoopMode()
    mode = 0 if force_unknown else {0: 1, 1: 2}.get(raw, 0)
    _record(event, flags=flags, mode=mode)
    if mode == _last_mode and now - _last_send < 0.1 and not force_unknown:
        return
    _sending = True
    try:
        for track, slot, indices in list(_targets):
            try:
                if not _matches(track, slot, indices):
                    _record("target-changed", track=track, slot=slot)
                    continue
                # Query again for each receiver. These are QPC-derived timestamps,
                # not audio sample times. Packet is written LAST as commit marker.
                query_seconds = time.perf_counter()
                query_stamp = (int(query_seconds * 8000) & MAX_WORD) / MAX_WORD
                raw = transport.getLoopMode()
                sampled = 0 if force_unknown else {0: 1, 1: 2}.get(raw, 0)
                _sequence = _sequence % ((1 << 22) - 1) + 1
                context_index, ack_index, query_index, write_index = indices
                ack = plugins.getParamValue(ack_index, track, slot)
                plugins.setParamValue(query_stamp, query_index, track, slot, 0)
                write_seconds = time.perf_counter()
                plugins.setParamValue((int(write_seconds * 8000) & MAX_WORD) / MAX_WORD, write_index, track, slot, 0)
                plugins.setParamValue(((_sequence << 2) | sampled) / MAX_WORD, context_index, track, slot, 0)
                _record("write", flags, track, slot, _sequence, sampled, ack,
                        "query=%.9f;write=%.9f" % (query_seconds, write_seconds))
            except Exception as error:
                _record("write-error", flags, track, slot, detail=str(error))
        if mode != _last_mode:
            print("RA probe mode:", ("Unknown", "Pattern", "Song")[mode], "via", event, "flags", flags)
        _last_mode, _last_send = mode, now
    finally:
        _sending = False


def OnInit():
    global _paused, _log_file, _last_mode, _last_send, _last_discovery
    _paused, _last_mode, _last_send, _last_discovery = False, None, -1.0, -1.0
    os.makedirs(EVIDENCE_DIRECTORY, exist_ok=True)
    _log_file = os.path.join(EVIDENCE_DIRECTORY, "script-%d.csv" % time.time_ns())
    with open(_log_file, "w", newline="") as output:
        csv.writer(output).writerow(("qpc_seconds", "event", "flags", "track", "slot", "sequence", "mode", "ack_readback", "detail"))
    _discover()
    _update("init", force_unknown=True)
    _flush()
    print("RA native probe init OK; log:", _log_file, "Python:", sys.version,
          "clock:", time.get_clock_info("perf_counter"))


def OnIdle():
    if not _paused:
        if time.perf_counter() - _last_discovery >= 1.0:
            _discover()
        _update("idle")
    _flush()


def OnRefresh(flags):
    # Observe ALL flags, including HW_Dirty_LEDs (256), instead of assuming a
    # notification implies Pat/Song. Poll the explicit query on each event.
    _update("refresh", flags)


def OnDoFullRefresh():
    global _last_discovery
    _last_discovery = -1.0
    _update("full-refresh")


def OnProjectLoad(status):
    global _paused, _targets, _last_discovery, _last_mode
    _targets, _last_discovery, _last_mode = [], -1.0, None
    _paused = status != 100
    _record("project", flags=status)
    if not _paused:
        _discover()
        _update("project-ready", force_unknown=True)


def OnDeInit():
    _update("deinit", force_unknown=True)
    _record("shutdown", detail="dropped-script-rows=%d" % _dropped)
    _flush()
