"""Compare instructions while masking only fields with actual ELF relocations.

The broad masking in retail.normalise is useful for grouping overlay copies,
but cannot validate C constants. Here an ordinary immediate stays significant.
"""
import struct

# MIPS ELF relocation numbers; REL addends are stored in the instruction.
MASKS = {0: 0, 1: 0xFFFF, 2: 0xFFFFFFFF, 4: 0x03FFFFFF,
         5: 0xFFFF, 6: 0xFFFF, 7: 0xFFFF}


def function_relocations(elf, symbol):
    result = []
    start, size = symbol["st_value"], symbol["st_size"]
    for section in elf.iter_sections():
        if section["sh_type"] == "SHT_REL" and section["sh_info"] == symbol["st_shndx"]:
            result.extend((r["r_offset"] - start, r["r_info_type"])
                          for r in section.iter_relocations() if start <= r["r_offset"] < start + size)
    return result


def relocated_equal(code, expected, relocations):
    if len(code) != len(expected) or len(code) % 4:
        return False
    left = list(struct.unpack("<%dI" % (len(code) // 4), code))
    right = list(struct.unpack("<%dI" % (len(expected) // 4), expected))
    for offset, kind in relocations:
        if kind not in MASKS:
            raise ValueError("unsupported MIPS relocation type %d" % kind)
        if offset % 4 or not 0 <= offset < len(code):
            raise ValueError("relocation outside function")
        mask = ~MASKS[kind] & 0xFFFFFFFF
        left[offset // 4] &= mask
        right[offset // 4] &= mask
    return left == right
