"""Regressions for pairing authored compiler FP source with GNU as output."""
import struct
import unittest

from ps2eeas_nops import move_sites, scan


class FloatMovePairingTests(unittest.TestCase):
    def test_literal_pool_load_is_not_a_move_hazard(self):
        # li.s f0,1.1 -> lwc1 f0,lit4(gp); c.le.s f0,f1.
        source = ['li.s $f0,1.10000002384185791016e0\n',
                  '#nop\n', 'c.le.s $f0,$f1\n']
        code = struct.pack('<2I', 0xC7800000, 0x46010036)
        self.assertEqual(move_sites(source, 0, len(source)), [])
        self.assertEqual(scan(0, len(code), code)[2], [])

    def test_immediate_constant_requires_move_padding(self):
        # li.s f0,0.25 -> lui at,3e80; mtc1 at,f0; c.lt.s f1,f0.
        source = ['li.s $f0,2.50000000000000000000e-1\n',
                  '#nop\n', 'c.lt.s $f1,$f0\n']
        code = struct.pack('<3I', 0x3C013E80, 0x44810000, 0x46000834)
        self.assertEqual(move_sites(source, 0, len(source)), [2])
        self.assertEqual(scan(0, len(code), code)[2], [8])

    def test_explicit_move_and_noreorder(self):
        source = ['mtc1 $zero,$f0\n', 'add.s $f1,$f0,$f2\n',
                  '.set noreorder\n', 'li.s $f0,1.0\n',
                  'c.le.s $f0,$f1\n']
        self.assertEqual(move_sites(source, 0, len(source)), [1])


if __name__ == '__main__':
    unittest.main()
