"""Deterministic FL API doubles + callback model. No real FL/VST3 or realtime validation."""
import math
import struct
import sys
import unittest
from types import SimpleNamespace
import device_reverbarise_context_probe as probe


class Receiver:
    """Model only: delivery setter != callback consumption; state never includes context."""
    def __init__(self):
        self.user_enable = True
        self.assets = ("source-accepted", "render-accepted", "saved-instance-id")
        self.restore()

    def restore(self):
        self.pending = None
        self.sequence = 0
        self.mode = probe.UNKNOWN
        self.received_at = None
        self.ack = 0.0

    def set_parameter(self, value):
        self.pending = value

    def block(self, now_ms, workflow="Perform", offline=False):
        if self.pending is not None:
            value, self.pending = self.pending, None
            sequence, mode = probe.decode(value)
            if sequence != self.sequence:
                self.sequence, self.mode, self.received_at = sequence, mode, now_ms
                self.ack = value  # Only callback, not setter, acknowledges consumption.
        fresh = self.received_at is not None and now_ms - self.received_at < 100
        # 100 ms is an arbitrary fixture lease, not a measured/recommended host timeout.
        context = self.mode if fresh else probe.UNKNOWN
        wet = (workflow == "Perform" and self.user_enable and not offline and context == probe.SONG)
        return context, wet  # Existing dry path/model assets are not modified.


class Plugin:
    def __init__(self, name=probe.TARGET, params=probe.PARAMS):
        self.name, self.params, self.receiver = name, params, Receiver()


class Host:
    def __init__(self):
        self.mode = 1
        self.slots = {(0, 0): Plugin(), (1, 3): Plugin(), (2, 0): Plugin("Other effect")}
        self.writes = []
        self.race = None
        self.api = SimpleNamespace(
            isValid=lambda t, s: (t, s) in self.slots,
            getPluginName=lambda t, s, user: self.slots[t, s].name,
            getParamCount=lambda t, s: len(self.slots[t, s].params),
            getParamName=lambda i, t, s: self.slots[t, s].params[i],
            getParamValue=lambda i, t, s: self.slots[t, s].receiver.ack,
            setParamValue=self.set_value)
        self.connector = probe.Connector(SimpleNamespace(getLoopMode=lambda: self.mode),
                                         self.api, SimpleNamespace(trackCount=lambda: 3))

    def set_value(self, value, index, track, slot, pickup):
        if self.race:
            action, self.race = self.race, None
            action()
        self.writes.append((track, slot, self.slots[track, slot].name, index, pickup))
        self.slots[track, slot].receiver.set_parameter(value)


class ConnectorTest(unittest.TestCase):
    def test_script_lifecycle_uses_direct_fl_api(self):
        host = Host()
        modules = {"transport": SimpleNamespace(getLoopMode=lambda: host.mode),
                   "plugins": host.api, "mixer": SimpleNamespace(trackCount=lambda: 3)}
        saved = {name: sys.modules.get(name) for name in modules}
        try:
            sys.modules.update(modules)
            probe.OnInit()
            r = host.slots[0, 0].receiver
            self.assertEqual(r.block(0), (probe.UNKNOWN, False))
            probe.OnIdle()
            self.assertEqual(r.block(1), (probe.SONG, True))
            probe.OnDeInit()
            self.assertEqual(r.block(2), (probe.UNKNOWN, False))
        finally:
            probe._connector = None
            for name, value in saved.items():
                if value is None:
                    sys.modules.pop(name, None)
                else:
                    sys.modules[name] = value

    def test_normalized_float32_packet_roundtrip(self):
        for seq in (1, 2, 255, 65535, probe.MAX_SEQUENCE):
            for mode in (probe.UNKNOWN, probe.PATTERN, probe.SONG):
                value = struct.unpack("f", struct.pack("f", probe.encode(seq, mode)))[0]
                self.assertEqual(probe.decode(value), (seq, mode))
        for invalid in (math.nan, math.inf, -1, 2, 0, 7 / probe.MAX_WORD):
            with self.assertRaises(ValueError):
                probe.decode(invalid)

    def test_all_instances_direct_api_and_ack_only_after_block(self):
        host = Host()
        host.connector.tick()
        self.assertEqual([(t, s) for t, s, *_ in host.writes], [(0, 0), (1, 3)])
        for key in ((0, 0), (1, 3)):
            receiver = host.slots[key].receiver
            self.assertEqual(receiver.ack, 0)
            self.assertEqual(receiver.block(0), (probe.SONG, True))
            self.assertNotEqual(receiver.ack, 0)
        self.assertTrue(all(index == 0 and pickup == 0 for _, _, _, index, pickup in host.writes))

    def test_move_duplicate_and_replacement_between_ticks(self):
        host = Host()
        host.connector.tick()
        host.slots[2, 5] = host.slots.pop((0, 0))
        host.slots[0, 0] = Plugin("Other effect")
        host.slots[1, 4] = Plugin()  # Duplicate gets its own receiver; no durable ID used.
        host.writes.clear()
        host.mode = 0
        host.connector.tick()
        self.assertEqual(set(host.connector.last), {(1, 3), (1, 4), (2, 5)})
        self.assertTrue(all(name == probe.TARGET for _, _, name, *_ in host.writes))
        for key in host.connector.last:
            self.assertEqual(host.slots[key].receiver.block(1), (probe.PATTERN, False))

    def test_incompatible_schema_or_invalid_mode(self):
        host = Host()
        host.slots[0, 0].params = ("Live enable", "Wet gain")
        host.mode = 99
        host.connector.tick()
        self.assertEqual(set(host.connector.last), {(1, 3)})
        self.assertEqual(host.slots[1, 3].receiver.block(0), (probe.UNKNOWN, False))

    def test_discovery_failure_does_not_write_or_starve_other_instance(self):
        host = Host()
        def name(track, slot, user):
            if (track, slot) == (0, 0):
                raise RuntimeError("removed during discovery")
            return host.slots[track, slot].name
        host.api.getPluginName = name
        host.connector.tick()
        self.assertEqual(host.connector.errors, 1)
        self.assertEqual(set(host.connector.last), {(1, 3)})

    def test_project_load_events_clear_targets(self):
        host = Host()
        probe._connector = host.connector
        try:
            host.connector.tick()
            probe.OnProjectLoad(0)
            host.writes.clear()
            host.connector.tick()
            self.assertEqual(host.writes, [])
            self.assertEqual(host.connector.last, {})
            host.slots = {(2, 6): Plugin()}
            probe.OnProjectLoad(101)
            host.connector.tick()
            self.assertEqual(host.writes, [])
            probe.OnProjectLoad(100)
            host.connector.tick()
            self.assertEqual(set(host.connector.last), {(2, 6)})
        finally:
            probe._connector = None

    def test_startup_disconnect_and_repeated_value_cannot_renew_lease(self):
        r = Receiver()
        self.assertEqual(r.block(0), (probe.UNKNOWN, False))
        packet = probe.encode(1, probe.SONG)
        r.set_parameter(packet)
        self.assertEqual(r.block(1), (probe.SONG, True))
        r.set_parameter(packet)
        self.assertEqual(r.block(101), (probe.UNKNOWN, False))
        r.set_parameter(probe.encode(2, probe.SONG))
        self.assertEqual(r.block(102), (probe.SONG, True))

    def test_enable_modes_assets_restore_and_offline_policy(self):
        r = Receiver()
        r.set_parameter(probe.encode(1, probe.SONG))
        r.block(0)
        assets = r.assets
        r.user_enable = False
        self.assertFalse(r.block(1)[1])
        r.user_enable = True
        for workflow in ("Learn", "Export"):
            self.assertFalse(r.block(2, workflow)[1])
        self.assertFalse(r.block(3, offline=True)[1])  # Model fail-closed choice only.
        r.restore()
        self.assertEqual(r.block(4), (probe.UNKNOWN, False))
        self.assertTrue(r.user_enable)
        self.assertEqual(r.assets, assets)

    def test_known_saved_parameter_replay_hole(self):
        r = Receiver()
        r.restore()
        r.set_parameter(probe.encode(1, probe.SONG))  # Host replays old value, no live sender.
        self.assertEqual(r.block(0), (probe.SONG, True))  # Demonstrates missing runtime challenge.

    def test_known_check_write_slot_race(self):
        host = Host()
        host.race = lambda: host.slots.__setitem__((0, 0), Plugin("Other effect"))
        host.connector.tick()
        self.assertEqual(host.writes[0][2], "Other effect")  # API is not compare-and-write.

    def test_delayed_delivery_leaks_even_with_correct_mode_query(self):
        # Virtual time, 48 kHz / 256 frames. Mode changes at 1 ms; script at 20 ms.
        r = Receiver()
        r.set_parameter(probe.encode(1, probe.SONG))
        r.block(0)
        wrong_callbacks = 0
        for sample in (256, 512, 768):
            wrong_callbacks += r.block(sample / 48)[1]
        r.set_parameter(probe.encode(2, probe.PATTERN))
        self.assertFalse(r.block(1024 / 48)[1])
        self.assertEqual(wrong_callbacks, 3)
        print("SIMULATED mid-play switch: 3 wet callbacks before Pattern at 21.333 ms")

    def test_immediately_before_play_and_frozen_offline_callbacks(self):
        r = Receiver()
        r.set_parameter(probe.encode(1, probe.SONG))
        r.block(-2)  # Correct stopped Song sample; host changes to Pattern at -1 ms.
        wrong = [r.block(sample / 48)[1] for sample in (0, 256, 512, 768)]
        self.assertEqual(sum(wrong), 4)  # First block is not guaranteed to see mode change.
        # This only proves the model's unconditional offline mute policy; it says
        # nothing about heartbeat expiry or actual host bounce behavior.
        self.assertTrue(all(not r.block(1, offline=True)[1] for _ in range(1000)))
        print("SIMULATED pre-Play switch: 4 wet callbacks; chosen offline mute policy rejects 1000 blocks (not expiry evidence)")


if __name__ == "__main__":
    unittest.main()
