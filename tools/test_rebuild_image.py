"""Fail-closed reconstruction checks using synthetic, non-retail sections."""
import struct
import unittest

from rebuild_image import reconstruct
from split_image import NAMES


def image():
    output = b""
    for index, name in enumerate(NAMES):
        address = 0x1000 + index * 0x100
        data = struct.pack("<II", 0x24020001, 0x03E00008) if name == "core.text" else bytes(8)
        output += struct.pack("<4I", address, len(data), 1, 0x1200) + data
    return output


def record(address=0x1200, size=8):
    return {"name": "func_00001200", "context": "resident", "address": address,
            "size": size, "exact": True, "binary": "synthetic.bin"}


class ReconstructionTests(unittest.TestCase):
    def test_exact_function_preserves_section_stream(self):
        code = struct.pack("<II", 0x24020001, 0x03E00008)
        rebuilt, inserted = reconstruct(image(), [record()], lambda _: code)
        self.assertEqual(rebuilt, image())
        self.assertEqual(inserted, ["func_00001200"])

    def test_report_cannot_authorize_changed_bytes(self):
        with self.assertRaisesRegex(ValueError, "differ"):
            reconstruct(image(), [record()], lambda _: bytes(8))

    def test_noncode_and_cross_section_ranges_rejected(self):
        for candidate in [record(address=0x1100), record(size=12)]:
            with self.assertRaises(ValueError):
                reconstruct(image(), [candidate], lambda _: bytes(candidate["size"]))

    def test_duplicate_and_empty_inputs_rejected(self):
        code = struct.pack("<II", 0x24020001, 0x03E00008)
        for records in [[], [record(), record()]]:
            with self.assertRaises(ValueError):
                reconstruct(image(), records, lambda _: code)

    def test_mismatch_report_rejected(self):
        candidate = record()
        candidate["exact"] = False
        with self.assertRaisesRegex(ValueError, "unmatched"):
            reconstruct(image(), [candidate], lambda _: bytes(8))


if __name__ == "__main__":
    unittest.main()
