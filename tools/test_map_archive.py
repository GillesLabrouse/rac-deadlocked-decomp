"""Archive hints must preserve unrelocated structure offsets and ambiguity."""
import struct
import unittest

from map_archive import unique_candidate


def words(*values):
    return struct.pack('<%dI' % len(values), *values)


class ArchiveMappingTests(unittest.TestCase):
    def setUp(self):
        self.code = words(0x3C040000, 0x8C840000, 0x03E00008, 0xAC850058)
        self.reference = words(0x3C040019, 0x8C842340, 0x03E00008, 0xAC850058)
        self.relocations = [(0, 5), (4, 6)]

    def test_real_relocations_allow_address_changes(self):
        self.assertEqual(unique_candidate(self.code, self.relocations,
                         [(0x1000, 'one', self.reference)]), (0x1000, 'one', 16))

    def test_nonrelocated_field_difference_rejects_false_alias(self):
        wrong = self.reference[:-4] + words(0xAC850010)
        self.assertIsNone(unique_candidate(self.code, self.relocations, [(0x1000, 'one', wrong)]))

    def test_padding_must_be_short_and_zero(self):
        self.assertEqual(unique_candidate(self.code, self.relocations,
                         [(0x1000, 'one', self.reference+words(0))]), (0x1000, 'one', 20))
        for padding in (words(1), words(0,0)):
            self.assertIsNone(unique_candidate(self.code, self.relocations,
                              [(0x1000, 'one', self.reference+padding)]))

    def test_ambiguity_is_not_resolved_by_inventory_order(self):
        candidates = [(0x1000, 'one', self.reference), (0x2000, 'two', self.reference)]
        self.assertIsNone(unique_candidate(self.code, self.relocations, candidates))
        self.assertIsNone(unique_candidate(self.code, self.relocations, candidates[::-1]))
        self.assertIsNone(unique_candidate(self.code, self.relocations, candidates, {'one'}))
        self.assertIsNone(unique_candidate(self.code, self.relocations, candidates[:1], {'one'}))

    def test_unknown_relocation_cannot_justify_hint(self):
        with self.assertRaises(ValueError):
            unique_candidate(self.code, [(0,99)], [(0x1000, 'one', self.reference)])


if __name__ == '__main__':
    unittest.main()
