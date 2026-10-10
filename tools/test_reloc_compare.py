"""Regression cases for constants that the old opcode-wide audit masked."""
import struct
import unittest

from reloc_compare import relocated_equal


def words(*values):
    return struct.pack("<%dI" % len(values), *values)


class RelocationComparisonTests(unittest.TestCase):
    def test_pointer_stride_is_not_a_relocation(self):
        self.assertFalse(relocated_equal(words(0x260401E0), words(0x26040078), []))

    def test_only_relocated_immediate_is_masked(self):
        self.assertTrue(relocated_equal(words(0x3C040000, 0x24840000),
                                        words(0x3C040022, 0x24841234), [(0, 5), (4, 6)]))
        self.assertFalse(relocated_equal(words(0x3C050000), words(0x3C040022), [(0, 5)]))

    def test_call_target_is_masked_only_at_relocation(self):
        self.assertTrue(relocated_equal(words(0x0C000000), words(0x0C123456), [(0, 4)]))
        self.assertFalse(relocated_equal(words(0x0C000000), words(0x0C123456), []))
        self.assertFalse(relocated_equal(words(0x08000000), words(0x0C123456), [(0, 4)]))

    def test_unknown_or_invalid_relocation_fails_closed(self):
        for relocation in [(0, 99), (2, 5), (4, 5)]:
            with self.assertRaises(ValueError):
                relocated_equal(words(0), words(0), [relocation])

    def test_size_mismatch(self):
        self.assertFalse(relocated_equal(words(0), words(0, 0), []))


if __name__ == "__main__":
    unittest.main()
