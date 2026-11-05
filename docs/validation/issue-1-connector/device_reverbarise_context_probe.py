# name=ReverbArise Context Probe (disposable)
"""Explore-only direct FL parameter experiment; NOT an installer or production connector.

Only addresses a hypothetical diagnostic plugin with the exact name/schema below.
Current ReverbArise has no such parameters. No IPC, controller setup or file writes.
Known limitations (deliberately tested): slot check/write race, saved parameter replay,
unbounded host delivery delay, expensive full discovery. Do not deploy this script.
"""
import math

TARGET = "ReverbArise FL Context Probe"
PARAMS = ("RA Probe Context v1", "RA Probe Audio Ack v1")
MAX_WORD = (1 << 24) - 1
MAX_SEQUENCE = (1 << 22) - 1
UNKNOWN, PATTERN, SONG = 0, 1, 2


def encode(sequence, mode):
    if not 1 <= sequence <= MAX_SEQUENCE or mode not in (UNKNOWN, PATTERN, SONG):
        raise ValueError("invalid context packet")
    return ((sequence << 2) | mode) / MAX_WORD


def decode(value):
    if not math.isfinite(value) or not 0 <= value <= 1:
        raise ValueError("invalid normalized value")
    word = round(value * MAX_WORD)
    sequence, mode = word >> 2, word & 3
    if sequence == 0 or mode == 3:
        raise ValueError("invalid packet")
    return sequence, mode


class Connector:
    def __init__(self, transport_api, plugins_api, mixer_api):
        self.transport, self.plugins, self.mixer = transport_api, plugins_api, mixer_api
        self.sequence = 0
        self.paused = False
        self.last = {}  # Latest bounded scan only; no saved slot/instance identities.
        self.errors = 0

    def matches(self, track, slot):
        p = self.plugins
        return (p.isValid(track, slot)
                and p.getPluginName(track, slot, 0) == TARGET
                and p.getParamCount(track, slot) == len(PARAMS)
                and tuple(p.getParamName(i, track, slot) for i in range(len(PARAMS))) == PARAMS)

    def tick(self, override=None):
        self.last = {}
        if self.paused:
            return
        count = self.mixer.trackCount()  # Includes Master and Current per FL API.
        if not 0 <= count <= 512:
            self.errors += 1
            return
        for track in range(count):
            for slot in range(10):  # FL mixer effect slots; no Channel Rack targeting.
                try:
                    if not self.matches(track, slot):
                        continue
                    raw = self.transport.getLoopMode() if override is None else None
                    mode = override if override is not None else {0: PATTERN, 1: SONG}.get(raw, UNKNOWN)
                    self.sequence = self.sequence % MAX_SEQUENCE + 1
                    packet = encode(self.sequence, mode)
                    # A recent name/schema check reduces stale-index mistakes, but
                    # FL exposes no atomic compare-plugin-and-write operation here.
                    if not self.matches(track, slot):
                        continue
                    ack = self.plugins.getParamValue(1, track, slot)
                    self.plugins.setParamValue(packet, 0, track, slot, 0)  # PIM_None
                    self.last[(track, slot)] = (self.sequence, mode, ack)
                except Exception:
                    self.errors += 1  # Discovery races/unavailable slots stay explicit.


_connector = None


def OnInit():
    global _connector
    import transport
    import plugins
    import mixer
    _connector = Connector(transport, plugins, mixer)
    _connector.tick(UNKNOWN)


def OnIdle():
    if _connector is not None:
        _connector.tick()


def OnProjectLoad(status):
    if _connector is not None:
        _connector.last = {}
        _connector.paused = status != 100  # PL_LoadOk; start/error do not send into new project.


def OnDeInit():
    global _connector
    if _connector is not None:
        _connector.tick(UNKNOWN)  # Best effort only; abrupt disconnect needs receiver expiry.
    _connector = None
