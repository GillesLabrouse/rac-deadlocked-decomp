"""Bounded integer instruction oracle for native class/geometry validation.

This is original test tooling, not a console emulator. It executes only the
integer instructions needed by the scoped workload and rejects everything else.
No hardware, DMA, VU, floating arithmetic, syscalls, BIOS or captured callees exist.
LWC1/SWC1 transport raw words only; PREF has no observable cache result here.
Only the non-interlocked zero-to-immutable-VF0 transfer is accepted from COP2;
it has no register/memory result in this domain, which excludes concurrent VU
work. All VU computations and other COP2 instructions are rejected.
Instruction words must be supplied locally; this module contains no retail code.

Registers retain 128 bits for LQ/SQ. Scalar operations replace their low 64 bits;
word arithmetic and signed word loads sign-extend into that low half. Comparisons
and ordinary branches use 64 bits. PLZCW counts redundant sign bits separately
in the two low words. LQ/SQ round addresses down to 16; other accesses require
their natural alignment. These details were independently cross-checked against
the PCSX2 project's R5900OpcodeImpl.cpp and MMI.cpp; no code was copied from them.
Raw COP1 load/store semantics were checked against its FPU.cpp. The oracle
models memory effects only, not cache timing or floating control/status state.
"""
from dataclasses import dataclass
from typing import Mapping, Sequence

MASK32 = (1 << 32) - 1
MASK64 = (1 << 64) - 1
MASK128 = (1 << 128) - 1


def signed(value: int, bits: int) -> int:
    value &= (1 << bits) - 1
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


class IntegerCPUError(RuntimeError):
    """The requested instruction run cannot be verified in this bounded domain."""


class UnsupportedInstruction(IntegerCPUError):
    pass


class InstructionBudgetExceeded(IntegerCPUError):
    pass


class IntegerTrap(IntegerCPUError):
    """Executed TEQ: preceding effects are retained; no exception handler runs."""


class MemoryAccessError(IntegerCPUError):
    pass


@dataclass
class MemoryRange:
    base: int
    data: bytearray


class IntegerMemory:
    """Disjoint guest32 byte ranges. Input backing is copied, never borrowed."""

    def __init__(self, ranges: Sequence[tuple[int, bytes | bytearray]]):
        self.ranges = []
        for base, data in ranges:
            if not 0 <= base <= MASK32 or not data or base + len(data) > 1 << 32:
                raise MemoryAccessError("Invalid guest range")
            if any(base < r.base + len(r.data) and r.base < base + len(data)
                   for r in self.ranges):
                raise MemoryAccessError("Overlapping guest ranges")
            self.ranges.append(MemoryRange(base, bytearray(data)))

    def region(self, address: int, size: int) -> tuple[bytearray, int]:
        if size <= 0 or not 0 <= address <= MASK32 or address + size > 1 << 32:
            raise MemoryAccessError(f"Invalid access at {address:08X}, {size} bytes")
        for r in self.ranges:
            offset = address - r.base
            if 0 <= offset and offset + size <= len(r.data):
                return r.data, offset
        raise MemoryAccessError(f"Unmapped access at {address:08X}, {size} bytes")

    def read(self, address: int, size: int) -> int:
        data, offset = self.region(address, size)
        return int.from_bytes(data[offset:offset + size], "little")

    def write(self, address: int, value: int, size: int) -> None:
        data, offset = self.region(address, size)
        data[offset:offset + size] = (value & ((1 << (size * 8)) - 1)).to_bytes(size, "little")


@dataclass(frozen=True)
class IntegerExecution:
    steps: int
    calls: tuple[int, ...]


class RetailIntegerCPU:
    """No fallback: unsupported executed instructions and bad memory are errors."""

    def __init__(self, words: Mapping[int, int], memory: IntegerMemory):
        if any(a & 3 or not 0 <= a <= MASK32 or not 0 <= w <= MASK32
               for a, w in words.items()):
            raise IntegerCPUError("Invalid instruction address or word")
        self.words = dict(words)
        self.memory = memory
        self.registers = [0] * 32
        self.float_bits = [0] * 32
        self.hi = self.lo = 0
        self.steps = 0
        self.calls: list[int] = []

    def _low(self, register: int) -> int:
        return self.registers[register] & MASK64

    def _put(self, register: int, value: int) -> None:
        if register:
            self.registers[register] = (self.registers[register] & (MASK128 ^ MASK64)) | (value & MASK64)

    def _put_word(self, register: int, value: int) -> None:
        self._put(register, signed(value, 32))

    def _put_quad(self, register: int, value: int) -> None:
        if register:
            self.registers[register] = value & MASK128

    def run(self, entry: int, arguments: Sequence[int] = (), *,
            initial_registers: Mapping[int, int] | None = None,
            stop_address: int = 0x0FFF0000, gp: int = 0, sp: int = 0,
            budget: int = 10000) -> IntegerExecution:
        if budget <= 0 or len(arguments) > 8 or entry & 3 or stop_address & 3:
            raise IntegerCPUError("Invalid run parameters")
        self.registers = [0] * 32
        self.float_bits = [0] * 32
        self.hi = self.lo = self.steps = 0
        self.calls = []
        for r, value in (initial_registers or {}).items():
            if not 0 <= r < 32:
                raise IntegerCPUError("Invalid initial register")
            if r:
                self.registers[r] = value & MASK128
        self._put(28, gp)
        self._put(29, sp)
        self._put(31, stop_address)
        for r, value in enumerate(arguments, 4):
            self._put(r, value)
        pc, pending = entry, None
        while self.steps < budget:
            if pc not in self.words:
                raise IntegerCPUError(f"Instruction outside verified spans at {pc:08X}")
            word = self.words[pc]
            op = word >> 26
            rs, rt, rd, shift = (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31, (word >> 6) & 31
            immediate = word & 0xFFFF
            displacement = signed(immediate, 16)
            a, b = self._low(rs), self._low(rt)
            target, annul = None, False
            self.steps += 1
            if op == 0:
                function = word & 63
                if function == 0:
                    self._put_word(rd, b << shift)
                elif function == 2:
                    self._put_word(rd, (b & MASK32) >> shift)
                elif function == 3:
                    self._put_word(rd, signed(b, 32) >> shift)
                elif function == 4:
                    self._put_word(rd, b << (a & 31))
                elif function == 8:
                    target = a & MASK32
                elif function == 9:
                    # Resolve old rs before publishing the link (rd may alias).
                    target = a & MASK32
                    self._put(rd, (pc + 8) & MASK32)
                    self.calls.append(target)
                elif function == 10:
                    if b == 0:
                        self._put(rd, a)
                elif function == 11:
                    if b != 0:
                        self._put(rd, a)
                elif function == 16:
                    self._put(rd, self.hi)
                elif function == 20:
                    self._put(rd, b << (a & 63))
                elif function == 22:
                    self._put(rd, b >> (a & 63))
                elif function == 24:
                    product = signed(a, 32) * signed(b, 32)
                    self.lo, self.hi = signed(product, 32) & MASK64, signed(product >> 32, 32) & MASK64
                    self._put(rd, self.lo)
                elif function == 26:
                    dividend, divisor = signed(a, 32), signed(b, 32)
                    if not divisor:
                        quotient, remainder = (1 if dividend < 0 else -1), dividend
                    elif dividend == -(1 << 31) and divisor == -1:
                        quotient, remainder = -(1 << 31), 0
                    else:
                        quotient = abs(dividend) // abs(divisor)
                        if (dividend < 0) != (divisor < 0):
                            quotient = -quotient
                        remainder = dividend - quotient * divisor
                    self.lo, self.hi = signed(quotient, 32) & MASK64, signed(remainder, 32) & MASK64
                elif function == 32:
                    total = signed(a, 32) + signed(b, 32)
                    if not -(1 << 31) <= total < (1 << 31):
                        raise IntegerCPUError(f"Signed ADD overflow at {pc:08X}; exception handlers are outside this oracle")
                    self._put_word(rd, total)
                elif function == 33:
                    self._put_word(rd, a + b)
                elif function == 34:
                    difference = signed(a, 32) - signed(b, 32)
                    if not -(1 << 31) <= difference < (1 << 31):
                        raise IntegerCPUError(f"Signed SUB overflow at {pc:08X}; exception handlers are outside this oracle")
                    self._put_word(rd, difference)
                elif function == 35:
                    self._put_word(rd, a - b)
                elif function == 36:
                    self._put(rd, a & b)
                elif function == 37:
                    self._put(rd, a | b)
                elif function == 38:
                    self._put(rd, a ^ b)
                elif function == 39:
                    self._put(rd, ~(a | b))
                elif function == 42:
                    self._put(rd, int(signed(a, 64) < signed(b, 64)))
                elif function == 43:
                    self._put(rd, int(a < b))
                elif function == 45:
                    self._put(rd, a + b)
                elif function == 52:
                    if a == b:
                        raise IntegerTrap(f"TEQ at {pc:08X}")
                elif function == 56:
                    self._put(rd, b << shift)
                elif function == 58:
                    self._put(rd, b >> shift)
                elif function == 60:
                    self._put(rd, b << (shift + 32))
                elif function == 62:
                    self._put(rd, b >> (shift + 32))
                elif function == 63:
                    self._put(rd, signed(b, 64) >> (shift + 32))
                else:
                    raise UnsupportedInstruction(f"Unsupported SPECIAL function {function:02X} at {pc:08X}")
            elif op == 8:
                total = signed(a, 32) + displacement
                if not -(1 << 31) <= total < (1 << 31):
                    raise IntegerCPUError(f"Signed ADDI overflow at {pc:08X}; exception handlers are outside this oracle")
                self._put_word(rt, total)
            elif op == 9:
                self._put_word(rt, a + displacement)
            elif op == 10:
                self._put(rt, int(signed(a, 64) < displacement))
            elif op == 11:
                self._put(rt, int(a < (displacement & MASK64)))
            elif op == 12:
                self._put(rt, a & immediate)
            elif op == 13:
                self._put(rt, a | immediate)
            elif op == 14:
                self._put(rt, a ^ immediate)
            elif op == 15:
                self._put_word(rt, immediate << 16)
            elif op in (2, 3):
                target = ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
                if op == 3:
                    self._put(31, pc + 8)
                    self.calls.append(target)
            elif op == 1 or op in (4, 5, 6, 7, 20, 21, 22, 23):
                if op == 1:
                    if rt not in (0, 1, 2, 3):
                        raise UnsupportedInstruction(f"Unsupported REGIMM selector {rt:02X} at {pc:08X}")
                    condition = signed(a, 64) < 0 if rt in (0, 2) else signed(a, 64) >= 0
                    likely = rt in (2, 3)
                else:
                    kind = op - 16 if op >= 20 else op
                    condition = {4: a == b, 5: a != b, 6: signed(a, 64) <= 0, 7: signed(a, 64) > 0}[kind]
                    likely = op >= 20
                annul = likely and not condition
                target = (pc + 4 + displacement * 4) & MASK32 if condition else (pc + 8) & MASK32
            elif word == ((18 << 26) | (5 << 21)):
                # Only non-interlocked QMTC2 zero -> immutable VF0. No VU
                # calculation or concurrent VU work is part of this domain.
                pass
            elif op == 51:
                # Cache state/timing is outside this memory-effect oracle.
                pass
            elif op in (49, 57):
                address = (a + displacement) & MASK32
                if address & 3:
                    raise MemoryAccessError(f"Unaligned COP1 word at {address:08X}")
                if op == 49:
                    self.float_bits[rt] = self.memory.read(address, 4)
                else:
                    self.memory.write(address, self.float_bits[rt], 4)
            elif op == 28 and (word & 63) == 4 and rt == 0 and shift == 0:
                result = 0
                for i in range(2):
                    bits = (a >> (i * 32)) & MASK32
                    if bits & (1 << 31):
                        bits ^= MASK32
                    result |= (31 - bits.bit_length()) << (i * 32)
                self._put(rd, result)
            elif op == 28 and (word & 63, shift) in (
                    (8, 26), (8, 22), (8, 18), (8, 25), (8, 3),
                    (9, 14), (9, 31), (41, 14), (41, 18)):
                left, right = self.registers[rs], self.registers[rt]
                function = word & 63
                if function == 8 and shift in (26, 22, 18):
                    width = {26: 8, 22: 16, 18: 32}[shift]
                    mask = (1 << width) - 1
                    result = sum(((right >> (lane * width)) & mask) << (lane * 2 * width)
                                 | ((left >> (lane * width)) & mask) << ((lane * 2 + 1) * width)
                                 for lane in range(64 // width))
                elif function == 8 and shift == 25:
                    result = sum((max(-128, min(127, signed(left >> (lane * 8), 8)
                                               - signed(right >> (lane * 8), 8))) & 255)
                                 << (lane * 8) for lane in range(16))
                elif function == 8:
                    result = sum((max(signed(left >> (lane * 32), 32),
                                      signed(right >> (lane * 32), 32)) & MASK32)
                                 << (lane * 32) for lane in range(4))
                elif function == 9 and shift == 31:
                    # PROT3W rotates words xyz left, retaining w; raw bits.
                    result = sum(((right >> (source * 32)) & MASK32) << (lane * 32)
                                 for lane, source in enumerate((1, 2, 0, 3)))
                elif function == 9:
                    result = (right & MASK64) | ((left & MASK64) << 64)
                elif shift == 18:
                    result = left | right
                else:
                    result = (left >> 64) | ((right >> 64) << 64)
                self._put_quad(rd, result)
            elif op in (30, 31, 32, 33, 35, 36, 37, 39, 40, 41, 43, 55, 63):
                size = {30: 16, 31: 16, 32: 1, 33: 2, 35: 4, 36: 1, 37: 2, 39: 4, 40: 1, 41: 2, 43: 4, 55: 8, 63: 8}[op]
                address = (a + displacement) & MASK32
                if op in (30, 31):
                    address &= ~15
                elif address & (size - 1):
                    raise MemoryAccessError(f"Unaligned access at {address:08X}, {size} bytes")
                if op in (31, 40, 41, 43, 63):
                    self.memory.write(address, self.registers[rt] if op == 31 else b, size)
                else:
                    value = self.memory.read(address, size)
                    if op == 30:
                        if rt:
                            self.registers[rt] = value
                    else:
                        self._put(rt, signed(value, size * 8) if op in (32, 33, 35) else value)
            else:
                raise UnsupportedInstruction(f"Unsupported opcode {op:02X} at {pc:08X}")
            if pending is not None:
                if target is not None:
                    raise IntegerCPUError(f"Control transfer in a delay slot at {pc:08X}")
                pc, pending = pending, None
                if pc == stop_address:
                    return IntegerExecution(self.steps, tuple(self.calls))
            elif annul:
                pc = target
            else:
                pc, pending = (pc + 4) & MASK32, target
        raise InstructionBudgetExceeded(f"Instruction budget {budget} exhausted at {pc:08X}")
