"""Tests for the bounded Dreadzone scene-WAD audit parser."""

import struct
import tempfile
import unittest
from pathlib import Path

import audit_dreadzone_scene_wad as audit


class SceneWadIndexTests(unittest.TestCase):
    def test_reads_provisional_chunk_resource_entries(self):
        decoded = bytearray(0x100)
        struct.pack_into("<4I", decoded, 0x50, 0x80, 0, 0, 0)
        struct.pack_into("<2I", decoded, 0x80, 9207, 0xC0)
        self.assertEqual(
            audit.parse_chunk_resource_entries(decoded, 20, 5),
            [{
                "descriptor_offset": 0x80,
                "resource_id": 9207,
                "payload_offset": 0xC0,
            }],
        )

    def test_summarizes_repeated_raw_rows_without_interpreting_them(self):
        rows = bytes.fromhex(
            "0102030405060708090a0b0c0d0e0f10"
            "0102030405060708090a0b0c0d0e0f10"
            "1112131415161718191a1b1c1d1e1f20"
        )
        summary = audit.summarize_candidate_payload(rows, {"data_offset": 0})
        self.assertEqual(summary["row_size"], 16)
        self.assertEqual(summary["available_row_count"], 3)
        self.assertEqual(summary["unique_row_count"], 2)
        self.assertEqual(summary["repeated_first_row_count"], 2)
        self.assertEqual(summary["rows_after_repeated_prefix"], 1)
        self.assertEqual(summary["first_row_hex"], rows[:16].hex())

    def test_records_next_candidate_descriptor_distance_without_linking_assets(self):
        candidates = [
            {"descriptor_offset": 0x60, "data_offset": 0x80},
            {"descriptor_offset": 0x140, "data_offset": 0x180},
        ]
        audit.annotate_next_candidate_descriptors(candidates)
        by_data_offset = {candidate["data_offset"]: candidate for candidate in candidates}
        self.assertEqual(
            by_data_offset[0x80]["next_candidate_descriptor_offset"], 0x140
        )
        self.assertEqual(
            by_data_offset[0x80]["rows_before_next_candidate_descriptor"], 12
        )
        self.assertEqual(
            by_data_offset[0x180]["next_candidate_descriptor_offset"], None
        )
        self.assertEqual(
            by_data_offset[0x180]["rows_before_next_candidate_descriptor"], None
        )

    def test_rejects_chunk_resource_payload_outside_stream(self):
        decoded = bytearray(0x100)
        struct.pack_into("<4I", decoded, 0x50, 0x80, 0, 0, 0)
        struct.pack_into("<2I", decoded, 0x80, 9207, 0x100)
        with self.assertRaisesRegex(ValueError, "invalid resource entry"):
            audit.parse_chunk_resource_entries(decoded, 20, 5)

    def test_finds_nested_zero_reserved_resource_candidate(self):
        decoded = bytearray(0x200)
        struct.pack_into("<4I", decoded, 0x80, 9207, 0xC0, 0, 0)
        struct.pack_into("<4I", decoded, 0xC0, 9227, 0x180, 0, 0)
        self.assertEqual(
            audit.scan_nested_resource_candidates(decoded, 20, 5, {0x80}),
            [{
                "descriptor_offset": 0xC0,
                "resource_id": 9227,
                "data_offset": 0x180,
            }],
        )

    def test_collects_next_sector_boundaries_from_all_scene_records(self):
        data = bytearray(10000)
        record = audit.SCENE_HEADER_OFFSET
        struct.pack_into("<I", data, record, 2)
        struct.pack_into("<I", data, record + 0x30, 3)
        struct.pack_into("<I", data, record + audit.SCENE_CHUNK_TABLE_OFFSET, 4)
        self.assertEqual(audit.scene_sector_starts(data), [2, 3, 4, 5])

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
        repo_root = Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory(dir=repo_root) as directory:
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

    def test_reports_exact_core_header_matches_in_scene_chunks(self):
        repo_root = Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory(dir=repo_root) as directory:
            core = Path(directory) / "class" / "core.bin"
            core.parent.mkdir()
            signature = bytes(range(64))
            core.write_bytes(signature + b"core payload")
            report = audit.scan_exact_core_headers_in_chunks(
                [Path(directory)], [(20, 5, b"prefix" + signature)]
            )
        self.assertEqual(report["global_and_mission_core_files_scanned"], 1)
        self.assertEqual(report["exact_first_64_byte_core_header_matches"], [{
            "core_path": str(core.resolve()),
            "scene_index": 20,
            "chunk_index": 5,
            "offset": 6,
        }])


if __name__ == "__main__":
    unittest.main()
