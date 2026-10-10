"""Synthetic WAD packet regressions; no retail bytes or assets."""
import struct
import unittest

from unpack_wad import BLOCK, HEADER, decompress


def wad(packets):
    return b"WAD" + struct.pack("<I", HEADER + len(packets)) + bytes(9) + packets


class WadNoOpTests(unittest.TestCase):
    def setUp(self):
        self.seed = bytes(range(132))
        self.literal = bytes([0, 114]) + self.seed
        # EE/10 is a legal eight-byte match at distance132, followed by
        # the two squashed literals09/0A. EE must be decoded as an opcode.
        self.match = bytes([0xEE, 0x10, 9, 10])
        self.expected = self.seed + self.seed[:8] + bytes([9, 10])

    def test_noop_before_ee_match(self):
        packets = self.literal + bytes([0x11, 0, 0]) + self.match
        self.assertEqual(decompress(wad(packets), 0), self.expected)

    def test_noop_at_block_start_before_ee_match(self):
        packets = bytearray(self.literal + bytes([0x12, 0, 0]))
        packets.extend(bytes([0xEE]) * (BLOCK - len(packets)))
        self.assertEqual((HEADER + len(packets)) % BLOCK, HEADER)
        packets.extend(bytes([0x11, 0, 0]) + self.match)
        self.assertEqual(decompress(wad(packets), 0), self.expected)

    def test_padding_marker_still_skips_fill(self):
        for low_bits in range(4):
            with self.subTest(low_bits=low_bits):
                packets = bytearray(self.literal + bytes([0x12, low_bits, 0]))
                packets.extend(bytes([0xEE]) * (BLOCK - len(packets)))
                packets.extend(self.match)
                self.assertEqual(decompress(wad(packets), 0), self.expected)

    def test_squashed_literal_and_nonzero_wad_base(self):
        packets = self.literal + bytes([0x11, 1, 0, 0xEE])
        prefix = bytes(37)
        self.assertEqual(decompress(prefix + wad(packets), len(prefix)), self.seed + b"\xEE")


if __name__ == "__main__":
    unittest.main()
