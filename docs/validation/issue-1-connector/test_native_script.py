"""Script wiring checks supporting the real native receiver; not host delivery evidence."""
import importlib
import sys
import unittest
from types import SimpleNamespace
from unittest.mock import patch
import device_ra_context_native as script


class NativeScriptTest(unittest.TestCase):
    def test_refresh_mode_change_prompt_heartbeat_and_separate_discovery(self):
        module = importlib.reload(script)
        now = [10.0]
        mode = [1]
        calls, scans = [], []
        modules = {
            "transport": SimpleNamespace(getLoopMode=lambda: mode[0]),
            "plugins": SimpleNamespace(isValid=lambda t, s: True,
                getPluginName=lambda t, s, user: module.TARGET,
                getParamCount=lambda t, s: 4,
                getParamName=lambda i, t, s: module.PARAMS[i],
                getParamValue=lambda i, t, s: 0,
                setParamValue=lambda *args: calls.append(args)),
            "mixer": SimpleNamespace(trackCount=lambda: scans.append(1) or 1)}
        with patch.dict(sys.modules, modules), patch.object(module.time, "perf_counter", lambda: now[0]):
            module._targets = [(0, 0, (0, 1, 2, 3))]
            module._last_discovery = now[0]
            module.OnRefresh(256)
            self.assertEqual([call[1] for call in calls], [2, 3, 0])
            self.assertEqual(round(calls[-1][0]*module.MAX_WORD) & 3, 2)
            now[0] += .001
            module.OnRefresh(4096)
            self.assertEqual(len(calls), 3)  # Setter-related notification cannot create feedback flood.
            mode[0] = 0
            module.OnRefresh(256)
            self.assertEqual(len(calls), 6)
            self.assertEqual(round(calls[-1][0]*module.MAX_WORD) & 3, 1)
            self.assertEqual(scans, [])  # Frequent update does not enumerate the mixer.
            now[0] += .2
            module.OnIdle()
            self.assertEqual(len(calls), 9)
            self.assertEqual(scans, [])
            module.OnProjectLoad(0)
            module.OnIdle()
            self.assertEqual(module._targets, [])
            self.assertEqual(len(calls), 9)
            self.assertTrue(any(row[1] == "refresh" and row[2] == 256 for row in module._rows))

    def test_parameter_discovery_tolerates_extra_wrapper_parameters(self):
        module = importlib.reload(script)
        names = ["Bypass", *module.PARAMS, "Program"]
        plugins = SimpleNamespace(isValid=lambda t, s: True,
            getPluginName=lambda t, s, user: module.TARGET,
            getParamCount=lambda t, s: len(names),
            getParamName=lambda i, t, s: names[i])
        with patch.dict(sys.modules, {"plugins": plugins}):
            self.assertEqual(module._identify(0, 0), (1, 2, 3, 4))
            self.assertTrue(module._matches(0, 0, (1, 2, 3, 4)))


if __name__ == "__main__":
    unittest.main()
