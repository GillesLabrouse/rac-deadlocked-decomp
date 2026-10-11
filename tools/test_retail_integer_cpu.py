"""Synthetic ISA fixtures only: no retail instruction words or data."""
from pathlib import Path
import sys
import tempfile
import struct
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from retail_integer_cpu import (IntegerCPUError, IntegerMemory, RetailIntegerCPU,
                                MemoryAccessError, UnsupportedInstruction,
                                InstructionBudgetExceeded, IntegerTrap, MASK64)


def r(function, rs=0, rt=0, rd=0, shift=0):
    return (rs << 21) | (rt << 16) | (rd << 11) | (shift << 6) | function


def i(op, rs=0, rt=0, immediate=0):
    return (op << 26) | (rs << 21) | (rt << 16) | (immediate & 65535)


def j(op, address):
    return (op << 26) | (address >> 2)


class IntegerCPUTests(unittest.TestCase):
    def execute(self, instructions, initial=None, memory=()):
        words = dict(zip(range(0x1000, 0x1000 + len(instructions) * 4, 4), instructions))
        cpu = RetailIntegerCPU(words, IntegerMemory(memory))
        result = cpu.run(0x1000, initial_registers=initial)
        return cpu, result

    def test_mtsab_low_nibble_xor_and_no_branch(self):
        for low in range(16):
            for immediate in (*range(16),0xFFF0,0xFFFF,0x1234,-8):
                for source in (0,1):
                    raw=(0xFEDCBA98765432100123456789ABCDE0 | low)
                    initial={1:raw,2:0xAABBCCDDEEFF00112233445566778899}
                    cpu,result=self.execute([i(1,source,24,immediate),i(9,0,3,7),r(8,31),0],initial)
                    self.assertEqual(cpu.sa,(low if source else 0) ^ (immediate & 15))
                    self.assertEqual(cpu.registers[1],raw)
                    self.assertEqual(cpu.registers[2],initial[2])
                    self.assertEqual(cpu.registers[3],7)
                    self.assertEqual((cpu.hi,cpu.lo),(0,0))
                    self.assertEqual((result.steps,result.calls),(4,()))

    def test_qfsrv_byte_window_full_quad_aliases_and_zero_sources(self):
        a=bytes(range(0xA0,0xB0));b=bytes(range(0x10,0x20))
        inputs={1:int.from_bytes(a,'little'),2:int.from_bytes(b,'little')}
        for amount in range(16):
            for rs in (0,1,2):
                for rt in (0,1,2):
                    left=bytes(16) if not rs else (a if rs==1 else b)
                    right=bytes(16) if not rt else (a if rt==1 else b)
                    expected=int.from_bytes((right+left)[amount:amount+16],'little')
                    for destination in (0,1,2,3):
                        cpu,result=self.execute([i(1,0,24,amount),
                            (28<<26)|r(40,rs,rt,destination,27),r(8,31),0],inputs)
                        self.assertEqual(cpu.registers[destination],expected if destination else 0)
                        for register in (1,2):
                            if register!=destination:self.assertEqual(cpu.registers[register],inputs[register])
                        self.assertEqual((cpu.hi,cpu.lo,cpu.sa),(0,0,amount))
                        self.assertEqual((result.steps,result.calls),(4,()))

    def test_mtsab_qfsrv_preserve_initialized_accumulators_and_run_reset(self):
        a=bytes(range(16));b=bytes(range(16,32))
        products=(1,-2,3,-4,5,-6,7,-8)
        factors=int.from_bytes(struct.pack('<8h',*range(1,9)),'little')
        signs=int.from_bytes(struct.pack('<8h',1,-1,1,-1,1,-1,1,-1),'little')
        code=[(28<<26)|r(9,6,7,8,28),i(1,0,24,8),
              (28<<26)|r(40,1,2,1,27),r(8,31),0,
              (28<<26)|r(40,1,2,3,27),r(8,31),0]
        words=dict(zip(range(0x1000,0x1000+len(code)*4,4),code))
        cpu=RetailIntegerCPU(words,IntegerMemory(()))
        initial={1:int.from_bytes(a,'little'),2:int.from_bytes(b,'little'),6:factors,7:signs}
        cpu.run(0x1000,initial_registers=initial)
        self.assertEqual(cpu.registers[1],int.from_bytes((b+a)[8:24],'little'))
        self.assertEqual(cpu.lo,int.from_bytes(struct.pack('<4i',*(products[i] for i in (0,1,4,5))),'little'))
        self.assertEqual(cpu.hi,int.from_bytes(struct.pack('<4i',*(products[i] for i in (2,3,6,7))),'little'))
        cpu.run(0x1014,initial_registers=initial)
        self.assertEqual(cpu.sa,0)
        self.assertEqual(cpu.registers[3],initial[2])
        self.assertEqual((cpu.hi,cpu.lo),(0,0))

    def test_mtsah_is_still_an_unsupported_regimm_selector(self):
        with self.assertRaises(UnsupportedInstruction):
            self.execute([i(1,1,25,7),r(8,31),0],{1:4})

    def test_comparisons_use_full_signed_or_unsigned_64_bits(self):
        cpu, _ = self.execute([r(42, 1, 0, 4), r(43, 1, 0, 5), i(10, 1, 6, 1),
                               i(11, 1, 7, -1), r(42, 2, 0, 8), r(8, 31), 0],
                              {1: 1 << 32, 2: 1 << 63})
        self.assertEqual(cpu.registers[4:9], [0, 0, 0, 1, 1])

    def test_word_arithmetic_and_lui_sign_extend_low_half(self):
        top = 0xAABBCCDDEEFF0123 << 64
        cpu, _ = self.execute([i(9, 1, 1, 1), i(9, 2, 2, 1), i(15, 0, 3, 0x8000),
                               r(2, 0, 3, 4), r(8, 31), 0],
                              {1: top | 0xFFFFFFFF, 2: 0x7FFFFFFF})
        self.assertEqual(cpu.registers[1], top)
        self.assertEqual(cpu.registers[2], 0xFFFFFFFF80000000)
        self.assertEqual(cpu.registers[3], 0xFFFFFFFF80000000)
        self.assertEqual(cpu.registers[4], 0xFFFFFFFF80000000)

    def test_signed_loads_and_64_bit_store_preserve_register_upper_half(self):
        payload = bytes.fromhex("80 00 00 80 FF FF 00 00 11 22 33 44 55 66 77 88") + bytes(16)
        top = 0x123456789ABCDEF0 << 64
        cpu, _ = self.execute([i(32, 1, 2), i(35, 1, 3), i(33, 1, 4, 4), i(37, 1, 5, 4),
                               i(55, 1, 6, 8), i(63, 1, 6, 16), r(8, 31), 0],
                              {1: 0x2000, 6: top}, [(0x2000, payload)])
        self.assertEqual(cpu.registers[2], 0xFFFFFFFFFFFFFF80)
        self.assertEqual(cpu.registers[3], 0xFFFFFFFF80000080)
        self.assertEqual(cpu.registers[4], MASK64)
        self.assertEqual(cpu.registers[5], 65535)
        self.assertEqual(cpu.registers[6], top | 0x8877665544332211)
        self.assertEqual(cpu.memory.read(0x2010, 8), 0x8877665544332211)

    def test_unsigned_word_load_zero_extends_and_preserves_high_half(self):
        for bits in (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF):
            for destination in (0, 1, 2):
                top = 0xABCDEF0123456789 << 64
                cpu, _ = self.execute([i(39, 1, destination, -4), r(8, 31), 0],
                                      {1: top | 0x2004, 2: top | MASK64},
                                      [(0x2000, bits.to_bytes(4, 'little'))])
                self.assertEqual(cpu.registers[destination], 0 if not destination else top | bits)
        for address, data in ((0x2001, bytes(8)), (0x2000, bytes(3))):
            with self.assertRaises(MemoryAccessError):
                self.execute([i(39, 1, 0)], {1: address}, [(0x2000, data)])

    def test_nor_uses_low_64_bits_and_preserves_destination_high_half(self):
        for left, right in ((0, 0), (MASK64, 0), (0x80000000, 0),
                            (0x1234567890ABCDEF, 0xFEDCBA0987654321)):
            top = 0x9988776655443322 << 64
            for destination in (0, 1, 2, 3):
                cpu, _ = self.execute([r(39, 1, 2, destination), r(8, 31), 0],
                                      {1: top | left, 2: top | right, 3: top})
                self.assertEqual(cpu.registers[destination], 0 if not destination else
                                 top | (MASK64 ^ (left | right)))

    def test_dsub_signed_extremes_aliases_and_high_half_preservation(self):
        minimum, maximum = -(1 << 63), (1 << 63) - 1
        values = (minimum, minimum + 1, -1, 0, 1, maximum - 1, maximum)
        top = 0xABCDEF0123456789 << 64
        for left in values:
            for right in values:
                difference = left - right
                if minimum <= difference <= maximum:
                    for destination in (0, 1, 2, 3):
                        with self.subTest(left=left, right=right, destination=destination):
                            cpu, _ = self.execute([r(46, 1, 2, destination), r(8, 31), 0],
                                                  {1: top | (left & MASK64),
                                                   2: top | (right & MASK64), 3: top | 123})
                            self.assertEqual(cpu.registers[destination], 0 if not destination else
                                             top | (difference & MASK64))

    def test_dsub_overflow_preserves_destination_and_preceding_effects(self):
        cases = ((-(1 << 63), 1), ((1 << 63) - 1, -1),
                 (0, -(1 << 63)), ((1 << 63) - 1, -(1 << 63)),
                 (-(1 << 63), (1 << 63) - 1))
        top = 0x9876543210ABCDEF << 64
        for left, right in cases:
            for destination in (0, 1, 2, 3):
                initial = {1: top | (left & MASK64), 2: top | (right & MASK64),
                           3: top | 42, 4: 0x2000, 5: 0xDEADBEEF}
                words = {0x1000: i(43, 4, 5), 0x1004: r(46, 1, 2, destination)}
                cpu = RetailIntegerCPU(words, IntegerMemory([(0x2000, bytes(4))]))
                with self.subTest(left=left, right=right, destination=destination):
                    with self.assertRaisesRegex(IntegerCPUError, 'DSUB overflow at 00001004'):
                        cpu.run(0x1000, initial_registers=initial)
                    self.assertEqual(cpu.registers[destination], initial.get(destination, 0))
                    self.assertEqual(cpu.memory.read(0x2000, 4), 0xDEADBEEF)

    def test_ppacb_selects_low_halfword_bytes_rt_then_rs(self):
        left = int.from_bytes(bytes(range(0x10, 0x20)), 'little')
        right = int.from_bytes(bytes(range(0xE0, 0xF0)), 'little')
        expected = int.from_bytes(bytes.fromhex('E0 E2 E4 E6 E8 EA EC EE 10 12 14 16 18 1A 1C 1E'), 'little')
        for source_left, source_right, result in ((1, 2, expected), (0, 2, expected & MASK64),
                                                  (1, 0, expected & (MASK64 << 64)), (0, 0, 0)):
            for destination in (0, 1, 2, 3):
                cpu, _ = self.execute([(28 << 26) | r(8, source_left, source_right, destination, 27),
                                       r(8, 31), 0], {1: left, 2: right, 3: (1 << 128) - 1})
                self.assertEqual(cpu.registers[destination], result if destination else 0)

    def test_pcpyh_replicates_halfwords_zero_and_four_and_ignores_rs(self):
        right = int.from_bytes(bytes.fromhex('34 12 FF EE DD CC BB AA CD AB 11 22 33 44 55 66'), 'little')
        expected = int.from_bytes(bytes.fromhex('34 12 34 12 34 12 34 12 CD AB CD AB CD AB CD AB'), 'little')
        for source in (0, 2):
            for destination in (0, 1, 2, 3):
                cpu, _ = self.execute([(28 << 26) | r(41, 1, source, destination, 27), r(8, 31), 0],
                                      {1: (1 << 128) - 1, 2: right, 3: 123})
                self.assertEqual(cpu.registers[destination], expected if destination and source else 0)

    def test_pextuw_interleaves_upper_words_with_aliases_and_zero_sources(self):
        left = 0xFFFFFFFF800000002222222211111111
        right = 0x89ABCDEF7FFFFFFF4444444433333333
        fixtures = ((1, 2, 0xFFFFFFFF89ABCDEF800000007FFFFFFF),
                    (2, 1, 0x89ABCDEFFFFFFFFF7FFFFFFF80000000),
                    (0, 2, 0x0000000089ABCDEF000000007FFFFFFF),
                    (1, 0, 0xFFFFFFFF000000008000000000000000),
                    (0, 0, 0), (1, 1, 0xFFFFFFFFFFFFFFFF8000000080000000))
        for rs, rt, expected in fixtures:
            for destination in (0, 1, 2, 3):
                with self.subTest(rs=rs, rt=rt, destination=destination):
                    cpu, _ = self.execute([(28 << 26) | r(40, rs, rt, destination, 18),
                                           r(8, 31), 0], {1: left, 2: right, 3: (1 << 128) - 1})
                    self.assertEqual(cpu.registers[destination], expected if destination else 0)

    def test_paddw_wraps_without_carries_between_words(self):
        left = int.from_bytes(bytes.fromhex('FF FF FF FF 00 00 00 80 FF FF FF 7F 34 12 00 00'), 'little')
        right = int.from_bytes(bytes.fromhex('01 00 00 00 00 00 00 80 01 00 00 00 CC ED FF FF'), 'little')
        expected = 0x80000000 << 64
        for source_left, source_right, result in ((1, 2, expected), (0, 2, right),
                                                  (1, 0, left), (0, 0, 0),
                                                  (1, 1, int.from_bytes(bytes.fromhex(
                                                      'FE FF FF FF 00 00 00 00 FE FF FF FF 68 24 00 00'), 'little'))):
            for destination in (0, 1, 2, 3):
                cpu, _ = self.execute([(28 << 26) | r(8, source_left, source_right, destination, 0),
                                       r(8, 31), 0], {1: left, 2: right, 3: 123})
                self.assertEqual(cpu.registers[destination], result if destination else 0)

    def test_psubh_wraps_without_borrows_between_halfwords(self):
        left = int.from_bytes(bytes.fromhex('00 00 00 80 FF 7F FF FF 01 00 34 12 CD AB 00 00'), 'little')
        right = int.from_bytes(bytes.fromhex('01 00 01 00 FF FF 00 80 FF FF 34 12 CD AB 00 00'), 'little')
        expected = int.from_bytes(bytes.fromhex('FF FF FF 7F 00 80 FF 7F 02 00 00 00 00 00 00 00'), 'little')
        negative_right = int.from_bytes(bytes.fromhex('FF FF FF FF 01 00 00 80 01 00 CC ED 33 54 00 00'), 'little')
        for source_left, source_right, result in ((1, 2, expected), (0, 2, negative_right),
                                                  (1, 0, left), (0, 0, 0), (1, 1, 0)):
            for destination in (0, 1, 2, 3):
                cpu, _ = self.execute([(28 << 26) | r(8, source_left, source_right, destination, 5),
                                       r(8, 31), 0], {1: left, 2: right, 3: 123})
                self.assertEqual(cpu.registers[destination], result if destination else 0)

    def test_packed_word_shifts_all_counts_aliases_and_lane_signs(self):
        lanes = (0x89ABCDEF, 0x7FFFFFFF, 0x80000000, 1)
        source = sum(value << (lane * 32) for lane, value in enumerate(lanes))
        for function in (60, 62, 63):
            for shift in range(32):
                expected_words = []
                for value in lanes:
                    if function == 60:
                        expected_words.append((value * (2 ** shift)) % (2 ** 32))
                    elif function == 62:
                        expected_words.append(value // (2 ** shift))
                    else:
                        signed_value = value if value < 2 ** 31 else value - 2 ** 32
                        expected_words.append((signed_value // (2 ** shift)) % (2 ** 32))
                expected = int.from_bytes(b''.join(value.to_bytes(4, 'little') for value in expected_words), 'little')
                for rt in (0, 2):
                    for destination in (0, 1, 2, 3):
                        with self.subTest(function=function, shift=shift, rt=rt, destination=destination):
                            cpu, _ = self.execute([(28 << 26) | r(function, 1, rt, destination, shift),
                                                   r(8, 31), 0],
                                                  {1: (1 << 128) - 1, 2: source, 3: 123})
                            self.assertEqual(cpu.registers[destination], expected if destination and rt else 0)

    def test_psrah_all_counts_signed_lanes_aliases_zero_and_hilo(self):
        lanes = (0, 1, 32767, -32768, -1, -3, 0x1234, -0x1234)
        source = int.from_bytes(struct.pack('<8h', *lanes), 'little')
        factors = int.from_bytes(struct.pack('<8h', *range(1, 9)), 'little')
        signs = int.from_bytes(struct.pack('<8h', 1, -1, 1, -1, 1, -1, 1, -1), 'little')
        expected_lo = int.from_bytes(struct.pack('<4i', 1, -2, 5, -6), 'little')
        expected_hi = int.from_bytes(struct.pack('<4i', 3, -4, 7, -8), 'little')
        for shift in range(32):
            expected = int.from_bytes(b''.join(
                ((value // (2 ** (shift % 16))) % 65536).to_bytes(2, 'little')
                for value in lanes), 'little')
            for rt in (0, 2):
                for destination in (0, 1, 2, 3):
                    with self.subTest(shift=shift, rt=rt, destination=destination):
                        cpu, _ = self.execute([
                            (28 << 26) | r(9, 4, 5, 0, 28),
                            (28 << 26) | r(55, 1, rt, destination, shift),
                            r(8, 31), 0],
                            {1: (1 << 128) - 1, 2: source, 3: 123,
                             4: factors, 5: signs})
                        self.assertEqual(cpu.registers[destination],
                                         expected if destination and rt else 0)
                        self.assertEqual(cpu.registers[0], 0)
                        self.assertEqual(cpu.registers[2],
                                         (expected if rt else 0) if destination == 2 else source)
                        self.assertEqual((cpu.hi, cpu.lo), (expected_hi, expected_lo))

    def test_quad_copy_aligns_down_and_scalar_writes_retain_upper_64_bits(self):
        payload = bytes(range(16)) + bytes([0xCD] * 16)
        cpu, _ = self.execute([i(30, 1, 2, 3), i(9, 0, 2, -1), i(31, 1, 2, 31), r(8, 31), 0],
                              {1: 0x2000}, [(0x2000, payload)])
        backing, _ = cpu.memory.region(0x2010, 16)
        self.assertEqual(bytes(backing[16:]), bytes([255] * 8) + bytes(range(8, 16)))

    def test_plzcw_counts_both_words_and_retains_upper_half(self):
        plzcw = (28 << 26) | r(4, rs=1, rd=2)
        top = 0x11223344 << 64
        cpu, _ = self.execute([plzcw, r(8, 31), 0], {1: 0xFFFFFFFF00000001, 2: top})
        self.assertEqual(cpu.registers[2], top | (31 << 32) | 30)
        cpu, _ = self.execute([plzcw, r(8, 31), 0], {1: 0x7FFFFFFF80000000})
        self.assertEqual(cpu.registers[2], 0)

    def test_mult_has_ee_destination_and_signed_hi_lo(self):
        cpu, _ = self.execute([r(24, 1, 2, 3), r(8, 31), 0], {1: -3, 2: 7})
        self.assertEqual(cpu.registers[3], (-21) & MASK64)
        self.assertEqual(cpu.lo, (-21) & MASK64)
        self.assertEqual(cpu.hi, MASK64)

    def test_pabsw_saturates_signed_minimum_and_snapshots_aliases(self):
        source = 0xFFFFFFFF7FFFFFFF8000000000000000
        expected = 0x000000017FFFFFFF7FFFFFFF00000000
        for rt in (0, 1, 2):
            for destination in (0, 1, 2, 3):
                with self.subTest(rt=rt, destination=destination):
                    cpu, _ = self.execute([(28 << 26) | r(40, 3, rt, destination, 1), r(8, 31), 0],
                                          {1: source, 2: source, 3: (1 << 128) - 1})
                    self.assertEqual(cpu.registers[destination], expected if rt and destination else 0)

    def test_srlv_word_count_mask_aliases_and_upper_half_preservation(self):
        value = 0xFEDCBA98765432100123456789ABCDEF
        count_upper = 0x123456789ABCDEF0
        for count in range(64):
            for rs, rt in ((1, 2), (0, 2), (1, 0), (2, 2)):
                initial = {1: (count_upper << 64) | (0xFFFFFFFF00000000 + count),
                           2: value, 3: 0xCAFEBABE01234567 << 64}
                shift_count = 0 if rs == 0 else (initial[rs] % 32)
                low_word = 0 if rt == 0 else (initial[rt] % (2 ** 32))
                quotient = low_word // (2 ** shift_count)
                signed_result = quotient if quotient < 2 ** 31 else quotient - 2 ** 32
                for destination in (0, 1, 2, 3):
                    with self.subTest(count=count, rs=rs, rt=rt, destination=destination):
                        cpu, _ = self.execute([r(6, rs, rt, destination), r(8, 31), 0], initial)
                        expected = ((initial.get(destination, 0) >> 64) << 64) | (signed_result & MASK64)
                        self.assertEqual(cpu.registers[destination], expected if destination else 0)

    def test_pand_full_register_bits_aliases_and_zero_sources(self):
        full = (1 << 128) - 1
        fixtures = [(1, 2, 0xFF00FF00FF00FF00FF00FF00FF00FF00,
                     0xF0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0,
                     0xF000F000F000F000F000F000F000F000),
                    (0, 2, full, full, 0), (1, 0, full, full, 0),
                    (0, 0, full, full, 0)]
        for bit in range(128):
            single = 1 << bit
            fixtures.extend(((1, 2, single, full, single),
                             (1, 2, single, full-single, 0),
                             (1, 1, single, full, single)))
        for rs, rt, left, right, expected in fixtures:
            for destination in (0, 1, 2, 3):
                with self.subTest(rs=rs, rt=rt, left=left, destination=destination):
                    cpu, _ = self.execute([(28 << 26) | r(9, rs, rt, destination, 18),
                                           r(8, 31), 0], {1: left, 2: right, 3: full})
                    self.assertEqual(cpu.registers[destination], expected if destination else 0)
                    self.assertEqual((cpu.hi, cpu.lo), (0, 0))

    def test_pmulth_all_accumulator_lanes_even_destination_aliases_and_zero(self):
        left = int.from_bytes(struct.pack('<8h', -32768, -1, 0, 1, 32767, -123, 2345, -32768), 'little')
        right = int.from_bytes(struct.pack('<8h', -32768, -1, 32767, -2, 32767, 321, -777, 1), 'little')
        mixed = (1073741824, 1, 0, -2, 1073676289, -39483, -1822065, -32768)
        square = (1073741824, 1, 0, 1, 1073676289, 15129, 5499025, 1073741824)
        pack = lambda lanes: int.from_bytes(struct.pack('<4I', *(x & 0xFFFFFFFF for x in lanes)), 'little')
        for rs, rt, products in ((1, 2, mixed), (2, 1, mixed), (1, 1, square),
                                 (0, 2, (0,) * 8), (1, 0, (0,) * 8), (0, 0, (0,) * 8)):
            for destination in (0, 1, 2, 3):
                with self.subTest(rs=rs, rt=rt, destination=destination):
                    cpu, _ = self.execute([(28 << 26) | r(9, rs, rt, destination, 28), r(8, 31), 0],
                                          {1: left, 2: right, 3: (1 << 128) - 1})
                    self.assertEqual(cpu.lo, pack((products[0], products[1], products[4], products[5])))
                    self.assertEqual(cpu.hi, pack((products[2], products[3], products[6], products[7])))
                    self.assertEqual(cpu.registers[destination],
                                     pack((products[0], products[2], products[4], products[6])) if destination else 0)
        cpu, _ = self.execute([(28 << 26) | r(9, 1, 2, 0, 28),
                               (28 << 26) | r(9, 0, 0, 0, 28), r(8, 31), 0], {1: left, 2: right})
        self.assertEqual((cpu.hi, cpu.lo), (0, 0))

    def test_mtlo_copies_low_64_without_word_extension_and_preserves_hi(self):
        factors = int.from_bytes(struct.pack('<8h', *range(1, 9)), 'little')
        signs = int.from_bytes(struct.pack('<8h', 1, -1, 1, -1, 1, -1, 1, -1), 'little')
        expected_lo = int.from_bytes(struct.pack('<4i', 1, -2, 5, -6), 'little')
        expected_hi = int.from_bytes(struct.pack('<4i', 3, -4, 7, -8), 'little')
        for source in (0, 1):
            for value in (0, 0x80000000, 0x1234567887654321, MASK64, 1 << 63):
                cpu, _ = self.execute([(28 << 26) | r(9, 4, 5, 0, 28),
                                       r(19, source), r(8, 31), 0],
                                      {1: (0xFEDCBA9876543210 << 64) | value,
                                       4: factors, 5: signs})
                self.assertEqual(cpu.lo, (expected_lo & (MASK64 << 64)) | (value if source else 0))
                self.assertEqual(cpu.hi, expected_hi)
                self.assertEqual(cpu.registers[1], (0xFEDCBA9876543210 << 64) | value)

    def test_madd_wraps_signed_products_and_snapshots_aliased_sources(self):
        # Initialize HI/LO at the first instruction fetch, after run's reset.
        # No instruction implementation or expected-result helper is reused.
        class InitialAccumulators(dict):
            def __getitem__(words, address):
                if address == 0x1000:
                    words.cpu.hi, words.cpu.lo = words.initial_hi, words.initial_lo
                return super().__getitem__(address)
        cases = ((0, 0), (1, -1), (-1, -1), (-2147483648, -2147483648),
                 (2147483647, 2147483647), (-2147483648, 2147483647),
                 (123456789, -987654321), (17, 19))
        for left, right in cases:
            for accumulator in (0, (1 << 32) - 1, (1 << 63) - 1, MASK64):
                for rs, rt in ((1, 2), (1, 1), (0, 2), (1, 0)):
                    for destination in (0, 1, 2, 3):
                        a = left if rs else 0
                        b = right if rt == 2 else left if rt else 0
                        total = (accumulator + a * b) % (1 << 64)
                        low_word = total % (1 << 32)
                        high_word = total // (1 << 32)
                        low = (low_word if low_word < (1 << 31) else low_word - (1 << 32)) % (1 << 64)
                        high = (high_word if high_word < (1 << 31) else high_word - (1 << 32)) % (1 << 64)
                        # Irrelevant upper words in low64 must not feed the sum.
                        initial_lo = (0xABCDEF0123456789 << 64) | (0x12345678 << 32) | (accumulator % (1 << 32))
                        initial_hi = (0xFEDCBA9876543210 << 64) | (0x87654321 << 32) | (accumulator >> 32)
                        words = InitialAccumulators({0x1000: (28 << 26) | r(0, rs, rt, destination),
                                                     0x1004: r(8, 31), 0x1008: 0})
                        words.initial_hi, words.initial_lo = initial_hi, initial_lo
                        cpu = RetailIntegerCPU(words, IntegerMemory([]))
                        # Constructor deliberately copies words; bind the fixture afterward.
                        cpu.words = words; words.cpu = cpu
                        top_left, top_right, top_other = 0x13579BDF << 64, 0x2468ACE0 << 64, 0xDEADBEEF << 64
                        cpu.run(0x1000, initial_registers={1: top_left | (left % (1 << 64)),
                                                         2: top_right | (right % (1 << 64)), 3: top_other})
                        self.assertEqual(cpu.lo, (initial_lo & (MASK64 << 64)) | low)
                        self.assertEqual(cpu.hi, (initial_hi & (MASK64 << 64)) | high)
                        expected_top = {0: 0, 1: top_left, 2: top_right, 3: top_other}[destination]
                        self.assertEqual(cpu.registers[destination], expected_top | low if destination else 0)
                        self.assertEqual(cpu.registers[0], 0)

    def test_scalar_mult_and_div_preserve_packed_accumulator_upper_halves(self):
        halfwords = int.from_bytes(struct.pack('<8h', 1, 2, 3, 4, 5, 6, 7, 8), 'little')
        high_lo = (25 << 64) | (36 << 96)
        high_hi = (49 << 64) | (64 << 96)
        for function, low_lo, low_hi in ((24, -21, -1), (26, 0, -3)):
            cpu, _ = self.execute([(28 << 26) | r(9, 1, 1, 0, 28), r(function, 4, 5, 6),
                                   r(16, rd=7), r(8, 31), 0],
                                  {1: halfwords, 4: -3, 5: 7, 6: 0x123456789ABCDEF0 << 64,
                                   7: 0xFEDCBA9876543210 << 64})
            self.assertEqual(cpu.lo, high_lo | (low_lo & MASK64))
            self.assertEqual(cpu.hi, high_hi | (low_hi & MASK64))
            self.assertEqual(cpu.registers[7], (0xFEDCBA9876543210 << 64) | (low_hi & MASK64))
            if function == 24:
                self.assertEqual(cpu.registers[6], (0x123456789ABCDEF0 << 64) | (low_lo & MASK64))

    def test_doubleword_shifts_movz_and_word_signed_shift(self):
        cpu, _ = self.execute([r(56, rt=1, rd=4, shift=7), r(60, rt=1, rd=5, shift=5),
                               r(62, rt=2, rd=6, shift=5), r(3, rt=3, rd=7, shift=4),
                               r(10, 1, 0, 8), r(10, 2, 1, 8), r(8, 31), 0],
                              {1: 0x12345678, 2: 0xFEDCBA9876543210, 3: 0x80000000})
        self.assertEqual(cpu.registers[4], 0x91A2B3C00)
        self.assertEqual(cpu.registers[5], 0x468ACF0000000000)
        self.assertEqual(cpu.registers[6], 0x7F6E5D4)
        self.assertEqual(cpu.registers[7], 0xFFFFFFFFF8000000)
        self.assertEqual(cpu.registers[8], 0x12345678)

    def test_arithmetic_doubleword_shift32_and_destination_aliases(self):
        for value in (0, 1, 0x7FFFFFFFFFFFFFFF, 0x8000000000000000, MASK64):
            signed_value = value if value < (1 << 63) else value - (1 << 64)
            for shift in range(32):
                for destination in (0, 1, 2):
                    top = 0xFEDCBA9876543210 << 64
                    cpu, _ = self.execute([r(63, rt=1, rd=destination, shift=shift), r(8, 31), 0],
                                          {1: top | value, 2: top | 0x1234})
                    self.assertEqual(cpu.registers[destination], 0 if not destination else
                                     top | ((signed_value >> (shift + 32)) & MASK64))

    def test_signed_division_remainder_and_exceptional_inputs(self):
        upper = 0x123456789ABCDEF0 << 64
        cases = ((7,3,2,1), (-7,3,-2,-1), (7,-3,-2,1), (-7,-3,2,-1),
                 (0,3,0,0), (7,0,-1,7), (-7,0,1,-7), (0,0,-1,0),
                 (-2147483648,-1,-2147483648,0),
                 (-2147483648,1,-2147483648,0),
                 (-2147483648,0,1,-2147483648))
        for dividend,divisor,quotient,remainder in cases:
            with self.subTest(dividend=dividend,divisor=divisor):
                cpu, _ = self.execute([r(26,1,2),r(16,rd=3),r(16,rd=0),r(8,31),0],
                                      {1: dividend,2: divisor,3: upper})
                self.assertEqual(cpu.lo,quotient & MASK64)
                self.assertEqual(cpu.hi,remainder & MASK64)
                self.assertEqual(cpu.registers[3],upper | (remainder & MASK64))
                self.assertEqual(cpu.registers[0],0)

    def test_division_uses_low_words_and_mfhi_can_alias_an_operand(self):
        upper = 0xFEDCBA9876543210 << 64
        cpu, _ = self.execute([r(26,1,2),r(16,rd=1),r(8,31),0],
                              {1: upper | 0xAAAA0000FFFFFFF9,2: 0xBBBB000000000003})
        self.assertEqual(cpu.lo,(-2)&MASK64)
        self.assertEqual(cpu.registers[1],upper | MASK64)

    def test_partial_stores_preserve_neighbor_bytes(self):
        cpu, _ = self.execute([i(40, 1, 2, 1), i(41, 1, 2, 2), i(43, 1, 2, 4), r(8, 31), 0],
                              {1: 0x2000, 2: 0x123456789ABCDEF0}, [(0x2000, bytes([0xCD] * 12))])
        backing, _ = cpu.memory.region(0x2000, 12)
        self.assertEqual(bytes(backing), bytes.fromhex("CD F0 F0 DE F0 DE BC 9A CD CD CD CD"))

    def test_movn_uses_low_64_bit_condition_and_preserves_destination_upper_half(self):
        upper = 0x1122334455667788 << 64
        source = (0xFFEEDDCCBBAA9988 << 64) | 0xFEDCBA9876543210
        for condition, expected in ((0, 7), (1 << 64, 7), (1 << 32, source & MASK64),
                                    (1 << 63, source & MASK64)):
            with self.subTest(condition=condition):
                cpu, _ = self.execute([r(11, 1, 2, 3), r(11, 1, 2, 0), r(8, 31), 0],
                                      {1: source, 2: condition, 3: upper | 7})
                self.assertEqual(cpu.registers[3], upper | expected)
                self.assertEqual(cpu.registers[0], 0)
        cpu, _ = self.execute([r(11, 1, 2, 2), r(8, 31), 0],
                              {1: source, 2: upper | (1 << 32)})
        self.assertEqual(cpu.registers[2], upper | (source & MASK64))

    def test_xor_and_xori_use_low_half_and_zero_extend_immediate(self):
        upper = 0x13579BDF2468ACE0 << 64
        cpu, _ = self.execute([r(38, 1, 2, 3), i(14, 1, 4, 0x8000),
                               i(14, 1, 1, 0xFFFF), r(38, 1, 2, 0), r(8, 31), 0],
                              {1: upper | 0xFEDCBA9876543210, 2: 0x0123456789ABCDEF,
                               3: upper, 4: upper})
        self.assertEqual(cpu.registers[3], upper | MASK64)
        self.assertEqual(cpu.registers[4], upper | 0xFEDCBA987654B210)
        self.assertEqual(cpu.registers[1], upper | 0xFEDCBA987654CDEF)
        self.assertEqual(cpu.registers[0], 0)

    def test_dsrl_logical_shift_preserves_upper_half_with_aliased_registers(self):
        upper = 0xAABBCCDDEEFF0011 << 64
        for shift in range(32):
            with self.subTest(shift=shift):
                cpu, _ = self.execute([r(58, rt=1, rd=1, shift=shift),
                                       r(58, rt=1, rd=0, shift=shift), r(8, 31), 0],
                                      {1: upper | (1 << 63)})
                self.assertEqual(cpu.registers[1], upper | (1 << (63 - shift)))
                self.assertEqual(cpu.registers[0], 0)

    def test_signed_addition_sign_extends_words_and_preserves_upper_half(self):
        upper = 0xFEDCBA9876543210 << 64
        cpu, _ = self.execute([r(32, 1, 2, 3), i(8, 3, 4, -1), r(8, 31), 0],
                              {1: 0xABCD0000FFFFFFFE, 2: 1, 3: upper, 4: upper})
        self.assertEqual(cpu.registers[3], upper | MASK64)
        self.assertEqual(cpu.registers[4], upper | (MASK64 - 1))

    def test_signed_addition_overflow_fails_even_when_destination_is_zero(self):
        for operands in ((0x7FFFFFFF, 1), (0x80000000, 0xFFFFFFFF)):
            for destination in (0, 3):
                with self.assertRaisesRegex(IntegerCPUError, "Signed ADD overflow"):
                    self.execute([r(32, 1, 2, destination)], {1: operands[0], 2: operands[1]})
                with self.assertRaisesRegex(IntegerCPUError, "Signed ADDI overflow"):
                    self.execute([i(8, 1, destination, operands[1])], {1: operands[0]})

    def test_differential_comparison_rejects_byte_and_guard_mismatches(self):
        from check_native_class_differential import host_backing, compare_backing, DifferentialError
        segments = [(0x2000, bytearray([1, 2, 3, 4]))]
        memory = IntegerMemory(segments)
        arrays, _ = host_backing(segments)
        self.assertEqual(compare_backing(segments, arrays, memory, "synthetic"), 4)
        arrays[0][2] = 5
        with self.assertRaises(DifferentialError):
            compare_backing(segments, arrays, memory, "synthetic")
        arrays[0][2] = 2; arrays[0][0] = 0
        with self.assertRaises(DifferentialError):
            compare_backing(segments, arrays, memory, "synthetic")

    def test_build_manifest_is_required_before_reusing_a_dll(self):
        from check_native_class_differential import validate_build_manifest, DifferentialError
        with tempfile.TemporaryDirectory() as directory:
            dll = Path(directory) / "synthetic.dll"
            dll.write_bytes(b"synthetic DLL fixture")
            with self.assertRaisesRegex(DifferentialError, "Missing build-input manifest"):
                validate_build_manifest(dll, {"synthetic.c": "a"}, {"test": True}, "recipe")

    def test_validated_manifest_reuse_and_changed_inputs(self):
        from check_native_class_differential import validate_build_manifest, write_build_manifest, DifferentialError
        with tempfile.TemporaryDirectory() as directory:
            dll = Path(directory) / "synthetic.dll"
            dll.write_bytes(b"synthetic DLL fixture")
            inputs, config, recipe = {"synthetic.c": "a", "synthetic.h": "b"}, {"test": True}, "recipe"
            manifest = write_build_manifest(dll, inputs, config, recipe, True)
            self.assertEqual(validate_build_manifest(dll, inputs, config, recipe), manifest)
            with self.assertRaisesRegex(DifferentialError, "inputs differ"):
                validate_build_manifest(dll, {**inputs, "synthetic.h": "changed"}, config, recipe)
            with self.assertRaisesRegex(DifferentialError, "configuration or recipe differs"):
                validate_build_manifest(dll, inputs, {"test": False}, recipe)
            with self.assertRaisesRegex(DifferentialError, "configuration or recipe differs"):
                validate_build_manifest(dll, inputs, config, "changed recipe")
            dll.write_bytes(b"changed DLL fixture")
            with self.assertRaisesRegex(DifferentialError, "DLL hash differs"):
                validate_build_manifest(dll, inputs, config, recipe)
    def test_malformed_build_manifest_fails(self):
        from check_native_class_differential import validate_build_manifest, build_manifest_path, DifferentialError
        with tempfile.TemporaryDirectory() as directory:
            dll = Path(directory) / "synthetic.dll"
            dll.write_bytes(b"synthetic DLL fixture")
            path = build_manifest_path(dll)
            for payload in ("not JSON", "[]", '{"kind":"native-class-differential-build","version":2}'):
                path.write_text(payload, encoding="utf-8")
                with self.assertRaises(DifferentialError):
                    validate_build_manifest(dll, {"synthetic.c": "a"}, {"test": True}, "recipe")

    def test_real_call_executes_argument_and_return_delay_slots(self):
        words = {0x1000: r(45, 31, 0, 16), 0x1004: j(3, 0x1040), 0x1008: i(9, 0, 4, 7),
                 0x100C: i(43, 1, 2), 0x1010: r(8, 16), 0x1014: i(9, 0, 18, 3),
                 0x1040: i(9, 4, 2, 1), 0x1044: r(8, 31), 0x1048: i(9, 2, 2, 2)}
        cpu = RetailIntegerCPU(words, IntegerMemory([(0x2000, bytes(4))]))
        result = cpu.run(0x1000, initial_registers={1: 0x2000})
        self.assertEqual(result.calls, (0x1040,))
        self.assertEqual(result.steps, 9)
        self.assertEqual(cpu.memory.read(0x2000, 4), 10)
        self.assertEqual(cpu.registers[18], 3)
        self.assertEqual(cpu.registers[31], 0x100C)

    def test_branch_equality_ignores_upper_64_bits_and_executes_delay(self):
        cpu, _ = self.execute([i(4, 1, 2, 2), i(9, 0, 3, 7), i(9, 0, 3, 8), r(8, 31), 0],
                              {1: (0xAAAA << 64) | 5, 2: (0xBBBB << 64) | 5})
        self.assertEqual(cpu.registers[3], 7)

    def test_branch_sign_uses_64_bits(self):
        cpu, _ = self.execute([i(1, 1, 1, 2), i(9, 0, 3, 7), i(9, 0, 3, 8), r(8, 31), 0],
                              {1: 0x80000000})
        self.assertEqual(cpu.registers[3], 7)
        cpu, _ = self.execute([i(1, 1, 0, 2), i(9, 0, 3, 7), i(9, 0, 3, 8), r(8, 31), 0],
                              {1: 0xFFFFFFFF00000000})
        self.assertEqual(cpu.registers[3], 7)

    def test_not_taken_likely_annuls_even_an_unsupported_delay_word(self):
        cpu, result = self.execute([i(20, 1, 0, 2), 17 << 26, i(9, 0, 2, 9), r(8, 31), 0], {1: 1})
        self.assertEqual(cpu.registers[2], 9)
        self.assertEqual(result.steps, 4)

    def test_zero_register_is_immutable(self):
        cpu, _ = self.execute([i(9, 0, 0, 15), i(9, 0, 2, 1), r(8, 31), 0], {0: 99})
        self.assertEqual(cpu.registers[0], 0)
        self.assertEqual(cpu.registers[2], 1)

    def test_unsupported_instruction_and_outside_span_fail(self):
        with self.assertRaises(UnsupportedInstruction):
            self.execute([17 << 26])
        with self.assertRaises(IntegerCPUError):
            self.execute([0])

    def test_raw_cop1_words_preserve_all_payloads_and_separate_registers(self):
        payloads = (0, 0x80000000, 0x7FC12345, 0xFFFFFFFF, 0x00000001,
                    0x7F800000, 0xFF800000, 0x12345678)
        for register in range(32):
            for bits in payloads:
                with self.subTest(register=register, bits=bits):
                    raw = bits.to_bytes(4, "little") + bytes(4)
                    top = (0xAABBCCDDEEFF << 64) | 0x2000
                    cpu, _ = self.execute([i(49, 1, register),
                                           i(57, 1, register, 4), r(8, 31), 0],
                                          {1: top}, [(0x2000, raw)])
                    self.assertEqual(cpu.float_bits[register], bits)
                    self.assertEqual(cpu.memory.read(0x2004, 4), bits)
                    self.assertEqual(cpu.registers[1], top)

    def test_raw_cop1_signed_offsets_wrap_addresses_and_reset_per_run(self):
        raw = bytes.fromhex("45 23 C1 7F 00 00 00 00")
        cpu, _ = self.execute([i(49, 1, 0, -4), i(57, 2, 0, 4), r(8, 31), 0],
                              {1: 0x100000004, 2: 0}, [(0, raw)])
        self.assertEqual(cpu.memory.read(4, 4), 0x7FC12345)
        cpu.words = {0x1000: i(57, 0, 0, 4), 0x1004: r(8, 31), 0x1008: 0}
        cpu.run(0x1000)
        self.assertEqual(cpu.memory.read(4, 4), 0)

    def test_raw_cop1_memory_failures_and_arithmetic_rejection(self):
        for op in (49, 57):
            with self.assertRaises(MemoryAccessError):
                self.execute([i(op, 1, 0)], {1: 0x2001}, [(0x2000, bytes(8))])
            with self.assertRaises(MemoryAccessError):
                self.execute([i(op, 1, 0)], {1: 0x2000}, [(0x2000, bytes(3))])
        for function in (0, 1, 2, 3, 4, 5, 6, 7, 24, 28, 36, 50):
            with self.assertRaises(UnsupportedInstruction):
                self.execute([(17 << 26) | (16 << 21) | function])

    def test_pref_does_not_read_unmapped_memory_or_modify_registers(self):
        for hint in range(32):
            value = (0xABCDEF0123456789 << 64) | 0xFFFFFFFFFFFFFFFF
            cpu, result = self.execute([i(51, 1, hint, -32768), r(8, 31), 0],
                                       {1: value})
            self.assertEqual(cpu.registers[1], value)
            self.assertEqual(cpu.float_bits, [0] * 32)
            self.assertEqual(result.steps, 3)

    def test_variable_shifts_mask_counts_preserve_upper_half_and_aliases(self):
        top = 0x8877665544332211 << 64
        value = 0xFEDCBA9887654321
        for count in range(128):
            for function, width in ((4, 32), (20, 64), (22, 64)):
                amount = count % width
                expected = (value >> amount if function == 22 else value << amount)
                if width == 32:
                    expected &= 0xFFFFFFFF
                    if expected & 0x80000000:
                        expected |= MASK64 ^ 0xFFFFFFFF
                for destination in (0, 1, 2, 3):
                    cpu, _ = self.execute([r(function, 1, 2, destination), r(8, 31), 0],
                                          {1: top | (0xABCDEF00 << 32) | count,
                                           2: top | value, 3: top})
                    self.assertEqual(cpu.registers[destination],
                                     0 if not destination else top | (expected & MASK64))

    def test_signed_sub_overflow_and_zero_destination_are_explicit(self):
        cpu, _ = self.execute([r(34, 1, 2, 1), r(8, 31), 0],
                              {1: (0x1234 << 64) | 0xFFFFFFFF, 2: 1})
        self.assertEqual(cpu.registers[1], (0x1234 << 64) | (MASK64 - 1))
        for left, right in ((0x7FFFFFFF, 0xFFFFFFFF), (0x80000000, 1)):
            for destination in (0, 1, 3):
                with self.assertRaisesRegex(IntegerCPUError, "Signed SUB overflow"):
                    self.execute([r(34, 1, 2, destination)], {1: left, 2: right})

    def test_teq_uses_low_64_bits_and_retains_prior_effects(self):
        words = {0x1000: i(43, 3, 1), 0x1004: r(52, 1, 2),
                 0x1008: i(43, 3, 2, 4), 0x100C: r(8, 31), 0x1010: 0}
        cpu = RetailIntegerCPU(words, IntegerMemory([(0x2000, bytes(8))]))
        with self.assertRaisesRegex(IntegerTrap, "TEQ at 00001004"):
            cpu.run(0x1000, initial_registers={1: (123 << 64) | 7,
                                             2: (456 << 64) | 7, 3: 0x2000})
        self.assertEqual(cpu.memory.read(0x2000, 8), 7)
        cpu.run(0x1000, initial_registers={1: 7, 2: 8, 3: 0x2000})
        self.assertEqual(cpu.memory.read(0x2000, 8), (8 << 32) | 7)

    def test_packed_interleave_and_doubleword_copy_lane_order_and_aliases(self):
        left = bytes(range(0x80, 0x90))
        right = bytes(range(16))
        for function, operation in ((8, 26), (8, 22), (9, 14), (41, 14)):
            if operation in (26, 22):
                size = 1 if operation == 26 else 2
                expected = b''.join(right[n:n+size] + left[n:n+size]
                                    for n in range(0, 8, size))
            elif function == 9:
                expected = right[:8] + left[:8]
            else:
                expected = left[8:] + right[8:]
            for destination in (0, 1, 2, 3):
                instruction = (28 << 26) | r(function, 1, 2, destination, operation)
                cpu, _ = self.execute([instruction, r(8, 31), 0],
                                      {1: int.from_bytes(left, 'little'),
                                       2: int.from_bytes(right, 'little'), 3: (1 << 128) - 1})
                self.assertEqual(cpu.registers[destination],
                                 int.from_bytes(expected, 'little') if destination else 0)

    def test_packed_signed_byte_subtract_all_pairs_and_destination_aliases(self):
        for start in range(0, 65536, 16):
            pairs = [(n // 256 - 128, n % 256 - 128) for n in range(start, start + 16)]
            left = bytes(a & 255 for a, _ in pairs)
            right = bytes(b & 255 for _, b in pairs)
            expected = bytearray()
            for a, b in pairs:
                difference = a - b
                if difference > 127:
                    difference = 127
                elif difference < -128:
                    difference = -128
                expected.append(difference & 255)
            for destination in (0, 1, 2, 3):
                instruction = (28 << 26) | r(8, 1, 2, destination, 25)
                cpu, _ = self.execute([instruction, r(8, 31), 0],
                                      {1: int.from_bytes(left, 'little'),
                                       2: int.from_bytes(right, 'little')})
                self.assertEqual(cpu.registers[destination],
                                 int.from_bytes(expected, 'little') if destination else 0)

    def test_packed_signed_word_max_and_same_source_interleave(self):
        left = [0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, 0]
        right = [0xFFFFFFFF, 0x80000000, 0, 0x7FFFFFFF]
        pack = lambda lanes: int.from_bytes(b''.join(n.to_bytes(4, 'little') for n in lanes), 'little')
        for destination in (0, 1, 2, 3):
            cpu, _ = self.execute([(28 << 26) | r(8, 1, 2, destination, 3), r(8, 31), 0],
                                  {1: pack(left), 2: pack(right)})
            self.assertEqual(cpu.registers[destination],
                             pack([0xFFFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF]) if destination else 0)
        raw = bytes(range(16))
        for size, operation in ((1, 26), (2, 22)):
            expected = b''.join(raw[n:n+size] * 2 for n in range(0, 8, size))
            cpu, _ = self.execute([(28 << 26) | r(8, 1, 1, 1, operation), r(8, 31), 0],
                                  {1: int.from_bytes(raw, 'little')})
            self.assertEqual(cpu.registers[1], int.from_bytes(expected, 'little'))

    def test_psubw_wraps_each_word_without_saturation_or_lane_borrow(self):
        pack = lambda values: sum((word & 0xFFFFFFFF) << (lane * 32)
                                  for lane, word in enumerate(values))
        left = (0, 0x80000000, 0x7FFFFFFF, 0xFFFFFFFF)
        right = (1, 1, 0xFFFFFFFF, 0x80000000)
        expected = pack((0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, 0x7FFFFFFF))
        for destination in (0, 1, 2, 3):
            cpu, _ = self.execute([(28 << 26) | r(8, 1, 2, destination, 1),
                                   r(8, 31), 0], {1: pack(left), 2: pack(right)})
            self.assertEqual(cpu.registers[destination], expected if destination else 0)
        for source in (0, 1):
            cpu, _ = self.execute([(28 << 26) | r(8, source, source, 1, 1),
                                   r(8, 31), 0], {1: pack(left)})
            self.assertEqual(cpu.registers[1], 0)
        cpu, _ = self.execute([(28 << 26) | r(8, 0, 1, 1, 1), r(8, 31), 0],
                              {1: pack((1, 2, 3, 4))})
        self.assertEqual(cpu.registers[1], pack((-1, -2, -3, -4)))

    def test_ppach_keeps_even_halfwords_and_snapshots_aliased_sources(self):
        for bit in range(128):
            for rs, rt in ((1, 2), (1, 1), (0, 2), (1, 0), (0, 0)):
                values = {0: bytes(16), 1: (1 << bit).to_bytes(16, 'little'),
                          2: ((1 << 128) - 1).to_bytes(16, 'little')}
                expected = b''.join(values[source][offset:offset + 2]
                                    for source in (rt, rs) for offset in (0, 4, 8, 12))
                for destination in (0, 1, 2, 3):
                    cpu, _ = self.execute([(28 << 26) | r(8, rs, rt, destination, 23),
                                           r(8, 31), 0],
                                          {1: int.from_bytes(values[1], 'little'),
                                           2: int.from_bytes(values[2], 'little')})
                    self.assertEqual(cpu.registers[destination],
                                     int.from_bytes(expected, 'little') if destination else 0)

    def test_pminw_compares_signed_words_with_ties_aliases_and_zero(self):
        pack = lambda values: sum((word & 0xFFFFFFFF) << (lane * 32)
                                  for lane, word in enumerate(values))
        boundary = (-2147483648, -2147483647, -1, 0, 1, 2147483646, 2147483647)
        for a in boundary:
            for b in boundary:
                left = (a, b, a, 0)
                right = (b, a, a, b)
                for rs, rt in ((1, 2), (1, 1), (0, 2), (1, 0), (0, 0)):
                    sources = {0: (0, 0, 0, 0), 1: left, 2: right}
                    expected = pack(tuple(x if x < y else y
                                          for x, y in zip(sources[rs], sources[rt])))
                    for destination in (0, 1, 2, 3):
                        cpu, _ = self.execute([(28 << 26) | r(40, rs, rt, destination, 3),
                                               r(8, 31), 0], {1: pack(left), 2: pack(right)})
                        self.assertEqual(cpu.registers[destination], expected if destination else 0)

    def test_prot3w_raw_word_rotation_aliases_zero_and_ignored_rs(self):
        lanes = (0x80000000, 0x7FC12345, 0x01234567, 0xFEDCBA98)
        pack = lambda values: sum(word << (lane * 32) for lane, word in enumerate(values))
        source = pack(lanes)
        expected = pack((lanes[1], lanes[2], lanes[0], lanes[3]))
        for destination in (0, 1, 2, 3):
            for unused_source in (0, 1, 2):
                cpu, _ = self.execute([(28 << 26) | r(9, unused_source, 2, destination, 31),
                                       r(8, 31), 0], {1: 0xAA, 2: source, 3: (1 << 128)-1})
                self.assertEqual(cpu.registers[destination], expected if destination else 0)
                if destination != 2:
                    self.assertEqual(cpu.registers[2], source)
        cpu, _ = self.execute([(28 << 26) | r(9, 0, 0, 3, 31), r(8, 31), 0],
                              {3: source})
        self.assertEqual(cpu.registers[3], 0)

    def test_prot3w_three_rotations_restore_all_128_bits(self):
        operation = (28 << 26) | r(9, 0, 2, 2, 31)
        for bit in range(128):
            cpu, _ = self.execute([operation, operation, operation, r(8, 31), 0],
                                  {2: 1 << bit})
            self.assertEqual(cpu.registers[2], 1 << bit)

    def test_pextlw_low_two_words_interleave_aliases_and_zero(self):
        left = (0x80000000, 0xABCDEF01, 0x11111111, 0x22222222)
        right = (0xFFFFFFFF, 0x01234567, 0x33333333, 0x44444444)
        pack = lambda values: sum(word << (lane * 32) for lane, word in enumerate(values))
        for rs, rt in ((1, 2), (1, 1), (0, 2), (1, 0), (0, 0)):
            for destination in (0, 1, 2, 3):
                a = left if rs else (0, 0, 0, 0)
                b = right if rt == 2 else left if rt else (0, 0, 0, 0)
                expected = pack((b[0], a[0], b[1], a[1]))
                cpu, _ = self.execute([(28 << 26) | r(8, rs, rt, destination, 18),
                                       r(8, 31), 0], {1: pack(left), 2: pack(right)})
                self.assertEqual(cpu.registers[destination], expected if destination else 0)

    def test_por_uses_all_bits_with_aliases_and_zero(self):
        top = (1 << 128)-1
        for bit in range(128):
            left, right = 1 << bit, top ^ (1 << bit)
            for destination in (0, 1, 2, 3):
                cpu, _ = self.execute([(28 << 26) | r(41, 1, 2, destination, 18),
                                       r(8, 31), 0], {1: left, 2: right})
                self.assertEqual(cpu.registers[destination], top if destination else 0)
        cpu, _ = self.execute([(28 << 26) | r(41, 1, 0, 1, 18),
                               (28 << 26) | r(41, 0, 1, 3, 18), r(8, 31), 0], {1: 1 << 127})
        self.assertEqual(cpu.registers[1], 1 << 127)
        self.assertEqual(cpu.registers[3], 1 << 127)

    def test_only_noninterlocked_zero_to_vf0_transport_is_supported(self):
        instruction = (18 << 26) | (5 << 21)
        cpu, result = self.execute([instruction, r(8, 31), 0])
        self.assertEqual(result.steps, 3)
        self.assertEqual(cpu.registers[:31], [0] * 31)
        for other in (instruction | 1, instruction | (1 << 16), instruction | (1 << 11)):
            with self.assertRaises(UnsupportedInstruction):
                self.execute([other])

    def test_jalr_reads_old_target_and_publishes_link_before_delay_slot(self):
        for destination in (0, 1, 5, 31):
            words = {0x1000: r(9, 1, 0, destination),
                     0x1004: i(43, 2, destination),
                     0x2000: r(8, 7), 0x2004: i(9, 0, 3, 27)}
            memory = IntegerMemory([(0x3000, bytes(4))])
            cpu = RetailIntegerCPU(words, memory)
            result = cpu.run(0x1000, initial_registers={1: (0xABCDEF << 64) | 0x2000,
                2: 0x3000, 7: 0x0FFF0000, 5: 0x123456 << 64})
            self.assertEqual(result.calls, (0x2000,))
            self.assertEqual(memory.read(0x3000, 4), 0x1008 if destination else 0)
            self.assertEqual(cpu.registers[3], 27)
            if destination:
                self.assertEqual(cpu.registers[destination] & MASK64, 0x1008)
            if destination == 1:
                self.assertEqual(cpu.registers[1] >> 64, 0xABCDEF)
            if destination == 5:
                self.assertEqual(cpu.registers[5] >> 64, 0x123456)

    def test_jalr_target_uses_low_word_and_link_wraps_at_32_bits(self):
        cpu = RetailIntegerCPU({0xFFFFFFFC: r(9, 1, 0, 5), 0: i(9, 0, 3, 18),
                                0x2000: r(8, 31), 0x2004: 0}, IntegerMemory([]))
        result = cpu.run(0xFFFFFFFC, initial_registers={1: (1 << 80) | (0xABCD << 32) | 0x2000})
        self.assertEqual(result.calls, (0x2000,))
        self.assertEqual(cpu.registers[5], 4)
        self.assertEqual(cpu.registers[3], 18)

    def test_jalr_in_delay_slot_and_unmapped_target_fail(self):
        with self.assertRaises(IntegerCPUError):
            self.execute([r(9, 1, 0, 5), r(9, 1, 0, 6)], {1: 0x2000})
        cpu = RetailIntegerCPU({0x1000: r(9, 1, 0, 5), 0x1004: i(9, 0, 3, 18)},
                               IntegerMemory([]))
        with self.assertRaises(IntegerCPUError):
            cpu.run(0x1000, initial_registers={1: 0x2000})
        self.assertEqual(cpu.calls, [0x2000])
        self.assertEqual(cpu.registers[3], 18)
        self.assertEqual(cpu.registers[5], 0x1008)

    def test_instruction_budget_and_delay_control_fail(self):
        cpu = RetailIntegerCPU({0x1000: i(4, 0, 0, -1), 0x1004: 0}, IntegerMemory([]))
        with self.assertRaises(InstructionBudgetExceeded):
            cpu.run(0x1000, budget=7)
        self.assertEqual(cpu.steps, 7)
        with self.assertRaises(IntegerCPUError):
            self.execute([i(4, 0, 0, 2), r(8, 31)])

    def test_bad_memory_domains_fail(self):
        with self.assertRaises(MemoryAccessError):
            self.execute([i(33, 1, 2)], {1: 0x2001}, [(0x2000, bytes(8))])
        with self.assertRaises(MemoryAccessError):
            self.execute([i(55, 1, 2)], {1: 0x2000}, [(0x2000, bytes(4))])
        with self.assertRaises(MemoryAccessError):
            IntegerMemory([(0x2000, bytes(8)), (0x2004, bytes(4))])


if __name__ == "__main__":
    unittest.main()
