#ifndef DEADLOCKED_DREADZONE_PROBE_REGISTERS_H
#define DEADLOCKED_DREADZONE_PROBE_REGISTERS_H
#include "common.h"

typedef struct DreadzoneProbeRegisters {
    u32 high64[2]; /* Raw a0 words2/3; no floating-point interpretation. */
    u32 last_writer_pc; /* Diagnostic: zero means preserved incoming state. */
} DreadzoneProbeRegisters;
#endif
