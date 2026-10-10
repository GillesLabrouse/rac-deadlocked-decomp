"""Synthetic ISA fixtures only: no retail instruction words or data."""
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from retail_integer_cpu import (IntegerCPUError, IntegerMemory, RetailIntegerCPU,
                                MemoryAccessError, UnsupportedInstruction,
                                InstructionBudgetExceeded, MASK64)


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
