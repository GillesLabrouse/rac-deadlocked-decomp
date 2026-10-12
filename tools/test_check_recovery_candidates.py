"""Mapped libraries require review and must not reserve foreign overlays."""
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import check_recovery_candidates as checker


class CandidateInventoryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(dir=Path(__file__).resolve().parent)
        self.addCleanup(self.temp.cleanup)
        root = Path(self.temp.name)
        (root/'config').mkdir()
        (root/'config/libc.tsv').write_text('# synthetic mapping\nobject\tsetter\t0x00123400\t0x10\n')
        self.functions = {
            'resident': {'addr':0x123400, 'seg':'core_text'},
            'overlay': {'addr':0x123400, 'seg':'ovl_diff|L01'},
            'other': {'addr':0x124000, 'seg':'net_text'}}
        for target, value in [('ROOT', root), ('load_functions', lambda:self.functions),
                              ('source_functions', lambda:{})]:
            p = patch.object(checker.progress, target, value)
            p.start()
            self.addCleanup(p.stop)

    def test_library_hint_is_visible_and_requires_review(self):
        row = checker.inventory(['resident'])[0]
        self.assertEqual(row['status'], 'library_mapping_requires_review')
        self.assertEqual(row['library_hints'][0]['symbol'], 'setter')

    def test_resident_hint_does_not_claim_same_address_in_overlay(self):
        self.assertEqual(checker.inventory(['overlay'])[0]['status'], 'available')

    def test_unmapped_and_unknown_candidates(self):
        self.assertEqual([r['status']for r in checker.inventory(['other','missing'])],
                         ['available','unknown_canonical_name'])

    def test_accepted_source_and_reservation_take_priority(self):
        with patch.object(checker.progress, 'source_functions', return_value={'src/file.c':['resident']}):
            self.assertEqual(checker.inventory(['resident'])[0]['status'], 'already_recovered')
        self.assertEqual(checker.inventory(['resident'], owned=['resident'])[0]['status'],
                         'owned_by_another_batch')


if __name__ == '__main__':
    unittest.main()
