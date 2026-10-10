"""Draft checks must use actual local boundaries and reject ambiguous ownership."""
import unittest
from unittest.mock import Mock, patch

import check_overlay_draft as draft


class OverlayDraftTests(unittest.TestCase):
    def test_actual_boundary_and_size_are_used(self):
        reference = [(0x500000, 16, ".text", b"retail bytes")]
        with patch.object(draft, "overlay_functions", return_value=[(0x500008, 8, b"code")]) as scan:
            sizes = draft.draft_sizes(reference, "L01")
        scan.assert_called_once_with(b"retail bytes", 0x500000)
        self.assertEqual(sizes, {"func_L01_00500008": (0x500008, 8)})

    def test_invalid_context_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "explicit level"):
            draft.draft_sizes([], "resident")

    def test_missing_or_duplicate_text_is_rejected(self):
        for reference in ([], [(0, 4, ".text", b"code")] * 2):
            with self.assertRaisesRegex(ValueError, "exactly one text"):
                draft.draft_sizes(reference, "L01")

    def symbols(self, name):
        elf = Mock()
        symbol = Mock()
        symbol.name = name
        elf.get_section_by_name.return_value.iter_symbols.return_value = [symbol]
        return elf

    def test_foreign_overlay_symbol_is_rejected(self):
        for symbol in ("func_L00_0066E0B8", "D_L00_0032B0E0"):
            with patch.object(draft.link_objects, "functions", return_value={"func_L01_00628020": None}):
                with self.assertRaisesRegex(ValueError, "independently resolved"):
                    draft.check_local_symbols(self.symbols(symbol), {"func_L01_00628020": (0x628020, 64)}, "L01")

    def test_wrong_entry_boundary_is_rejected(self):
        with patch.object(draft.link_objects, "functions", return_value={"func_L01_00628024": None}):
            with self.assertRaisesRegex(ValueError, "known local overlay boundary"):
                draft.check_local_symbols(self.symbols("D_001DFA70"), {"func_L01_00628020": (0x628020, 64)}, "L01")

    def test_empty_object_is_rejected(self):
        with patch.object(draft.link_objects, "functions", return_value={}):
            with self.assertRaisesRegex(ValueError, "no game functions"):
                draft.check_local_symbols(self.symbols("D_001DFA70"), {}, "L01")

    def test_verified_span_preserves_other_entries(self):
        sizes = {"func_L01_00500000": (0x500000, 8), "func_L01_00500008": (0x500008, 8)}
        reference = [(0x500000, 64, ".text", bytes(64))]
        result = draft.verified_spans(sizes, reference, "L01", ["00500000:20"])
        self.assertEqual(result["func_L01_00500000"], (0x500000, 32))
        self.assertEqual(result["func_L01_00500008"], sizes["func_L01_00500008"])
        self.assertEqual(sizes["func_L01_00500000"], (0x500000, 8))

    def test_invalid_extent_specs_are_rejected(self):
        sizes = {"func_L01_00500000": (0x500000, 8)}
        reference = [(0x500000, 64, ".text", bytes(64))]
        for span in ("bad", "500000:-4", "500000:0", "500000:A", "500000:100", "500004:8"):
            with self.assertRaises(ValueError):
                draft.verified_spans(sizes, reference, "L01", [span])

    def test_duplicate_verified_spans_are_rejected(self):
        with self.assertRaisesRegex(ValueError, "unique known entry"):
            draft.verified_spans({"func_L01_00500000": (0x500000, 8)},
                                 [(0x500000, 64, ".text", bytes(64))],
                                 "L01", ["500000:10", "500000:20"])

    def test_extent_cannot_use_data_section(self):
        with self.assertRaisesRegex(ValueError, "outside overlay text"):
            draft.verified_spans({"func_L01_00500000": (0x500000, 8)},
                                 [(0x500000, 64, ".data", bytes(64))], "L01", ["500000:10"])

    def test_explicit_unlisted_entry_is_draft_only(self):
        sizes = {"func_L01_00500000": (0x500000, 8)}
        result = draft.verified_spans(sizes, [(0x500000, 64, ".text", bytes(64))],
                                      "L01", ["500010:10"], new_entries=True)
        self.assertEqual(result["func_L01_00500010"], (0x500010, 16))
        self.assertNotIn("func_L01_00500010", sizes)
        with patch.object(draft.link_objects, "functions", return_value={"func_L01_00500010": None}):
            draft.check_local_symbols(self.symbols("D_002208A8"), result, "L01")

    def test_unlisted_entry_must_not_replace_known_boundaries(self):
        with self.assertRaisesRegex(ValueError, "unique unlisted entry"):
            draft.verified_spans({"func_L01_00500000": (0x500000, 8)},
                                 [(0x500000, 64, ".text", bytes(64))],
                                 "L01", ["500000:10"], new_entries=True)

    def test_unlisted_entry_rejects_invalid_extent(self):
        reference = [(0x500000, 64, ".text", bytes(64))]
        for span in ("500001:10", "500010:3", "500030:20", "600000:10"):
            with self.assertRaises(ValueError):
                draft.verified_spans({}, reference, "L01", [span], new_entries=True)

    def test_unlisted_entry_rejects_duplicate_declarations(self):
        with self.assertRaisesRegex(ValueError, "unique unlisted entry"):
            draft.verified_spans({}, [(0x500000, 64, ".text", bytes(64))],
                                 "L01", ["500010:10", "500010:20"], new_entries=True)


if __name__ == "__main__":
    unittest.main()
