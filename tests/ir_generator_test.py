"""Independent format/measurement/reproduction checks for original IR prototypes."""
import hashlib
import importlib.util
import json
import math
import sys
from pathlib import Path
import tempfile
import unittest
import wave

ROOT = Path(__file__).resolve().parents[1]
sys.dont_write_bytecode = True
spec = importlib.util.spec_from_file_location("generator", ROOT / "scripts/generate_ir_library.py")
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)


class GeneratorTest(unittest.TestCase):
    def test_original_assets_reproduce_and_metrics_describe_real_pcm(self):
        with tempfile.TemporaryDirectory(prefix="preverb-ir-") as folder:
            first, second = Path(folder) / "first", Path(folder) / "second"
            generator.generate(first)
            generator.generate(second)
            manifest = json.loads((first / "manifest.json").read_text())
            self.assertEqual(len(manifest["entries"]), 5)
            self.assertEqual(manifest["license"], "CC0-1.0")
            self.assertTrue(manifest["prototypes_pending_listening"])
            self.assertEqual((first / "manifest.json").read_bytes(), (second / "manifest.json").read_bytes())
            for item in manifest["entries"]:
                low,high={"monumental":(8,12),"intimate-plate":(.7,1.5),"vintage-dark-hall":(3,5),"shimmer-cloud":(5,9),"whisper":(.25,.8)}[item["parameters"]["id"]]
                self.assertLessEqual(low,item["seconds"])
                self.assertLessEqual(item["seconds"],high)
                data = (first / item["file"]).read_bytes()
                self.assertEqual(data, (second / item["file"]).read_bytes())
                self.assertEqual(hashlib.sha256(data).hexdigest(), item["wav_sha256"])
                with wave.open(str(first / item["file"]), "rb") as w:
                    self.assertEqual((w.getnchannels(), w.getsampwidth(), w.getframerate(), w.getnframes()),
                                     (2, 3, 48000, item["frames"]))
                    pcm = w.readframes(w.getnframes())
                self.assertEqual(hashlib.sha256(pcm).hexdigest(), item["pcm_sha256"])
                for c in range(2):
                    values = []
                    for offset in range(c * 3, len(pcm), 6):
                        n = int.from_bytes(pcm[offset:offset + 3], "little", signed=True)
                        values.append(n / 8388608.)
                    self.assertEqual(values[0], 0)
                    self.assertEqual(values[-1], 0)
                    self.assertTrue(all(math.isfinite(x) and abs(x) < 1 for x in values))
                    actual = item["measurements"]["channels"][c]
                    self.assertAlmostEqual(sum(x*x for x in values), actual["energy"], places=10)
                    self.assertAlmostEqual(max(map(abs, values)), actual["peak"], places=12)
                    self.assertAlmostEqual(math.sqrt(sum(x*x for x in values)/len(values)), actual["rms"], places=12)
                self.assertGreater(item["measurements"]["mono_vs_mean_channel_db"], -6.)
            by_id = {e["parameters"]["id"]:e for e in manifest["entries"]}
            approved = {"intimate-plate":"fad9e920013101794af9050fbf88d784f65ccb7fd65523e963998a2853b10c1c",
                        "shimmer-cloud":"16d1ea58c5db5ae1bf110ceea30da6a88276092a40bb34f25745f72f7d9eee7f",
                        "whisper":"59f013118f8daccb87b93c9b125c385d620f50e72e710bede6eedcd93d5387c7"}
            for ident, checksum in approved.items():
                self.assertEqual(by_id[ident]["wav_sha256"],checksum,"Preserve the user's accepted sound")
            # Objective spatial distinction, separate from required subjective approval.
            self.assertGreater(by_id["monumental"]["measurements"]["side_to_mid_db"] -
                               by_id["intimate-plate"]["measurements"]["side_to_mid_db"], 10.)
            self.assertLess(by_id["monumental"]["measurements"]["correlation"], .2)
            self.assertGreater(by_id["whisper"]["measurements"]["correlation"], .8)
            # No content generation step may replace the existing default.
            self.assertEqual(hashlib.sha256((ROOT / "resources/PreverbHall.wav").read_bytes()).hexdigest(),
                             "b7275a27fc7629a9a1a7191abda4fbb3c8a46372dc9071e36dc1112e4839b458")


if __name__ == "__main__":
    unittest.main()
