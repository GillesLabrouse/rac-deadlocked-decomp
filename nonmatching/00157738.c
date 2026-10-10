#include "common.h"

typedef struct SectionHeader {
    u32 address;
    u32 size;
    u32 unused;
    u32 entry;
} SectionHeader;

extern s32 D_0021DE10_[];
extern s32 D_0021DE10;

/* Draft of the section-stream loader. Not accepted until exact linking passes. */
u32 func_00157738(u8 *source, u32 limit) {
    u32 entry = 0;
    for (;;) {
        SectionHeader *section = (SectionHeader *)source;
        u32 destination = section->address;
        s32 index;
        source += sizeof(SectionHeader);
        if (entry == 0) {
            entry = section->entry;
            index = D_0021DE10_[0];
        } else {
            if (entry != section->entry) {
                return entry;
            }
            index = D_0021DE10;
        }
        if ((u32)(index - 1) >= 0x27 ||
            (destination < limit && destination + section->size < limit)) {
            u32 size = section->size;
            if (size != 0) {
                if (!(size & 7) && !((u32)source & 7) && !(destination & 7)) {
                    u64 *out = (u64 *)destination;
                    u64 *in = (u64 *)source;
                    u64 *end = (u64 *)(destination + size);
                    while (out != end) {
                        *out++ = *in++;
                    }
                } else if (!(size & 3) && !((u32)source & 3) && !(destination & 3)) {
                    u32 *in = (u32 *)source;
                    u32 *end = (u32 *)(destination + size);
                    while ((u32 *)destination != end) {
                        *(u32 *)destination = *in++;
                        destination += 4;
                    }
                } else {
                    u8 *out = (u8 *)destination;
                    u8 *in = source;
                    u8 *end = (u8 *)(destination + size);
                    while (out != end) {
                        *out++ = *in++;
                    }
                }
            }
        }
        source += (section->size + 3) & ~3;
    }
}
