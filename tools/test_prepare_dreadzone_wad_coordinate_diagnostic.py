import struct
import unittest
from unittest.mock import patch

import prepare_dreadzone_wad_coordinate_diagnostic as diagnostic


class CoordinateDiagnosticTests(unittest.TestCase):
    def report(self):
        return {
            "scene_chunk_inventory": {
                "chunks": [{
                    "scene_index": 20,
                    "chunk_index": 5,
                    "sector": 0,
                    "resource_entries": [
                        {"resource_id": 9228, "payload_offset": 16,
                         "descriptor_offset": 64},
                        {"resource_id": 9999, "payload_offset": 80,
                         "descriptor_offset": 80},
                    ],
                }],
                "nested_candidates": [{
                    "scene_index": 20, "chunk_index": 5,
                    "descriptor_offset": 48,
                }],
            },
        }

    def test_extracts_rows_only_until_next_direct_or_nested_descriptor(self):
        decoded = bytearray(128)
        struct.pack_into("<4f", decoded, 16, 1, 2, 3, 0)
        struct.pack_into("<4f", decoded, 32, 4, 5, 6, 0)
        struct.pack_into("<4f", decoded, 48, 7, 8, 9, 0)
        with patch.object(diagnostic, "decompress", return_value=decoded):
            rows = diagnostic.extract_rows(
                self.report(), b"wad", 20, 5, 5, 9228)
        self.assertEqual(rows, [(5, 0, (1.0, 2.0, 3.0)),
                                (5, 1, (4.0, 5.0, 6.0))])

    def test_filters_scene_and_chunk_range(self):
        with patch.object(diagnostic, "decompress") as decompress:
            rows = diagnostic.extract_rows(
                self.report(), b"wad", 19, 5, 5, 9228)
        self.assertEqual(rows, [])
        decompress.assert_not_called()

    def test_rejects_nonzero_fourth_component(self):
        decoded = bytearray(128)
        struct.pack_into("<4f", decoded, 16, 1, 2, 3, 1)
        with patch.object(diagnostic, "decompress", return_value=decoded):
            with self.assertRaisesRegex(ValueError, "Invalid coordinate row"):
                diagnostic.extract_rows(self.report(), b"wad", 20, 5, 5,
                                        9228)

    def test_marker_vertices_preserve_point_centers(self):
        rows = [(5, 0, (1.0, 2.0, 3.0))]
        vertices = diagnostic.point_markers(rows, 0.1)
        self.assertEqual(len(vertices), 24)
        self.assertEqual(vertices[0][:3], (1.1, 2.0, 3.0))
        self.assertEqual(vertices[0][5:], (0.15, 0.85, 0.95, 1.0))


if __name__ == "__main__":
    unittest.main()
