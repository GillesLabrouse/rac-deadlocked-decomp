"""Authored regressions for integer and opt-in float immediate expansion."""
import unittest

from ps2eeas_dli import float_bits, rewrite
from ps2eeas_nops import move_sites


class ImmediateExpansionTests(unittest.TestCase):
    def test_float_expansion_is_opt_in(self):
        source = ['\tli.s\t$f12,1.10000002384185791016e0\n']
        self.assertEqual(rewrite(source), (source, 0, 0))

    def test_consumer_float_bits_and_registers(self):
        source = ['\tli.s\t$f12,1.10000002384185791016e0 # target\n',
                  '\tli.s\t$f13,5.00000044703483581543e-2\n']
        self.assertEqual(rewrite(source, True), ([
            '\t.set\tnoat\n',
            '\tlui\t$at,0x3f8c # target\n',
            '\tori\t$at,$at,0xcccd\n',
            '\tmtc1\t$at,$f12\n',
            '\t.set\tat\n',
            '\t.set\tnoat\n',
            '\tlui\t$at,0x3d4c\n',
            '\tori\t$at,$at,0xccce\n',
            '\tmtc1\t$at,$f13\n',
            '\t.set\tat\n'], 0, 2))

    def test_zero_low_half_and_negative_zero_are_unchanged(self):
        source = ['li.s $f0,0.25\n', 'li.s $f1,-0.0\n',
                  'li.s $f2,1024.0\n']
        self.assertEqual(rewrite(source, True), (source, 0, 0))
        self.assertEqual(float_bits('-0.0'), 0x80000000)

    def test_integer_expansion_is_unchanged(self):
        source = ['\tdli $4,0x8000000044\n']
        expected = (['\tori\t$4,$0,0x8000\n',
                     '\tdsll\t$4,$4,24\n',
                     '\tori\t$4,$4,0x44\n'], 1, 0)
        self.assertEqual(rewrite(source), expected)
        self.assertEqual(rewrite(source, True), expected)

    def test_expansion_exposes_the_true_move_hazard(self):
        source = ['li.s $f0,1.1\n', 'c.le.s $f0,$f1\n']
        output, _, _ = rewrite(source, True)
        self.assertEqual(move_sites(output, 0, len(output)), [5])

    def test_explicit_noat_mode_is_preserved(self):
        source = ['.set noat\n', 'li.s $f0,-1.1\n']
        output, _, floats = rewrite(source, True)
        self.assertEqual(output, ['.set noat\n', 'lui\t$at,0xbf8c\n',
                                  'ori\t$at,$at,0xcccd\n', 'mtc1\t$at,$f0\n'])
        self.assertEqual(floats, 1)

    def test_nonfinite_constants_fail_closed(self):
        for constant in ('nan', 'inf', '-inf'):
            with self.subTest(constant=constant):
                with self.assertRaises(ValueError):
                    rewrite(['li.s $f0,' + constant + '\n'], True)


if __name__ == '__main__':
    unittest.main()
