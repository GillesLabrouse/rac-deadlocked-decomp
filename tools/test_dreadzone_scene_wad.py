"""Tests for the bounded Dreadzone scene-WAD audit parser."""

import struct
import tempfile
import unittest
from pathlib import Path

import audit_dreadzone_scene_wad as audit


class SceneWadIndexTests(unittest.TestCase):
    def test_reads_records_until_all_ones_sentinel(self):
        decoded = (
            struct.pack("<4I", 9300, 0x10, 0x20, 0)
            + struct.pack("<4I", 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
        )
        self.assertEqual(
            audit.parse_index(decoded, 3),
            [{"record_id": 9300, "start": 0x10, "end": 0x20}],
        )

    def test_rejects_out_of_range_record(self):
        decoded = struct.pack("<4I", 9300, 0x20, 0x102, 0) + bytes(0x101 - 16)
        with self.assertRaisesRegex(ValueError, "invalid index record"):
            audit.parse_index(decoded, 3)

    def test_rejects_missing_sentinel(self):
        decoded = struct.pack("<4I", 9300, 0x1, 0x10, 0)
        with self.assertRaisesRegex(ValueError, "no complete index sentinel"):
            audit.parse_index(decoded, 3)

    def test_reads_pif2_header_without_decoding_payload(self):
        header = struct.pack("<8I", 0x50494632, 0x1234, 128, 64, 0x13, 0, 0, 4)
        result = audit.parse_pif_headers(b"prefix" + header + b"payload", 4)
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0]["offset"], 6)
        self.assertEqual(result[0]["width"], 128)
        self.assertEqual(result[0]["height"], 64)
        self.assertEqual(result[0]["mip_levels"], 4)

    def test_reports_exact_exported_core_header_matches(self):
        build_directory = Path(__file__).resolve().parents[1] / "build"
        with tempfile.TemporaryDirectory(dir=build_directory) as directory:
            core = Path(directory) / "class" / "core.bin"
            core.parent.mkdir()
            core.write_bytes(bytes(range(64)) + b"core payload")
            report = audit.scan_exact_core_headers(
                directory, [(7, b"unrelated"), (9, b"prefix" + bytes(range(64)))]
            )
        self.assertEqual(report["global_core_files_scanned"], 1)
        self.assertEqual(report["exact_first_64_byte_core_header_matches"], [{
            "core_path": str(core), "scene_index": 9, "offset": 6,
        }])


if __name__ == "__main__":
    unittest.main()
