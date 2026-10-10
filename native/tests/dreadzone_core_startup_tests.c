#include "dreadzone_core_startup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

_Static_assert(sizeof(DreadzoneCoreStartupBlock) == 0x1688, "Accessed block extent changed");
_Static_assert(sizeof(((DreadzoneCoreStartupContext *)0)->state_0021DFE8) == 4, "Guest state must be u32");
_Static_assert(sizeof(((DreadzoneCoreStartupContext *)0)->callback_0021DB94) == 4, "Guest callback must be u32");
static unsigned checks;
static void check(int condition, const char *message) {
    checks++;
    if (!condition) { fprintf(stderr, "Core startup check failed: %s\n", message); exit(1); }
}

/* The oracle uses retail absolute addresses and little-endian stores. It is
 * independent of the implementation's union word accesses and loop bases. */
static void oracle_word(u8 *bytes, u32 address, u32 value) {
    u32 i = address - 0x00172160;
    bytes[i] = (u8)value; bytes[i + 1] = (u8)(value >> 8);
    bytes[i + 2] = (u8)(value >> 16); bytes[i + 3] = (u8)(value >> 24);
}
static void oracle_A170(u8 *bytes, s32 state) {
    unsigned i;
    bytes[0x1737E1 - 0x172160] = 0;
    if (state >= 7) return;
    for (i = 0; i < 10; i++) {
        oracle_word(bytes, 0x1727A4 + 4 * i, 0xFFFFFFFFU);
        bytes[0x172959 - 0x172160 + i] = 0;
        bytes[0x172963 - 0x172160 + 16 * i] = 0;
        bytes[0x172A03 - 0x172160 + i] = 255;
        oracle_word(bytes, 0x172A10 + 4 * i, 0xFFFFFFFFU);
    }
    oracle_word(bytes, 0x172168, 0);
    oracle_word(bytes, 0x172160, 7);
}
static void oracle_AA58(u8 *bytes, s32 state, s32 level) {
    unsigned i;
    if (state >= 10) return;
    bytes[0x172958 - 0x172160] = 0;
    bytes[0x172828 - 0x172160] = 0;
    for (i = 0; i < 10; i++) {
        oracle_word(bytes, 0x172800 + 4 * i, 0);
        oracle_word(bytes, 0x17292C + 4 * i, 0);
    }
    oracle_word(bytes, 0x172954, 0);
    oracle_word(bytes, 0x172160, 10);
    if (level == 0) bytes[0x1723C5 - 0x172160] = 1;
    oracle_word(bytes, 0x172170, 0);
    oracle_word(bytes, 0x1723B4, 0);
    bytes[0x1723B0 - 0x172160] = 1;
    memcpy(bytes + 0x220, bytes + 0x218, 4);
}
static void compare_bytes(const u8 *actual, const u8 *expected) {
    unsigned i;
    for (i = 0; i < 0x1688; i++) check(actual[i] == expected[i], "complete block byte oracle");
}
static DreadzoneCoreStartupContext context_for(DreadzoneCoreStartupBlock *block) {
    DreadzoneCoreStartupContext context;
    memset(&context, 0, sizeof(context));
    context.block = block;
    context.level_0021DE10 = 1;
    context.state_0021DFE8 = 0x00223E40;
    context.selected_002120F4 = 12345;
    context.flag_0021DFA4 = 0xA5;
    return context;
}
static void test_real_level_one_sequence(void) {
    static const s32 states[] = {INT_MIN, -1, 0, 6, 7, 8, 9, 10, INT_MAX};
    static const u8 patterns[] = {0, 0x39, 0xA5, 255};
    struct { u8 before[16]; DreadzoneCoreStartupBlock block; u8 after[16]; } guarded;
    u8 expected[0x1688];
    unsigned i, j, k;
    s32 current;
    DreadzoneCoreStartupContext context;
    for (i = 0; i < sizeof(states) / sizeof(states[0]); i++) {
        for (j = 0; j < sizeof(patterns); j++) {
            memset(&guarded, patterns[j], sizeof(guarded));
            guarded.block.words[0] = (u32)states[i];
            guarded.block.words[0x258 / 4] = 1;
            context = context_for(&guarded.block);
            memcpy(expected, guarded.block.bytes, sizeof(expected));
            oracle_A170(expected, states[i]);
            check(dreadzone_core_0015A170_slice(&context) == 0, "actual level1 A170 path");
            compare_bytes(guarded.block.bytes, expected);
            check(context.selected_002120F4 == 12345 && context.flag_0021DFA4 == 0xA5,
                  "A170 leaves other globals unchanged");
            current = states[i] < 7 ? 7 : states[i];
            oracle_AA58(expected, current, 1);
            check(dreadzone_core_0015AA58_slice(&context) == 0, "actual level1 AA58/B498/F4A0 path");
            compare_bytes(guarded.block.bytes, expected);
            check(context.selected_002120F4 == (current < 10 ? 0 : 12345), "selected global value");
            check(context.flag_0021DFA4 == (current < 10 ? 0 : 0xA5), "global flag changed only on executed path");
            check(context.state_0021DFE8 == 0x223E40 && context.callback_0021DB94 == 0 &&
                  context.level_0021DE10 == 1, "existing-state branch preserves guest globals");
            for (k = 0; k < 16; k++) {
                check(guarded.before[k] == patterns[j], "before guard preserved");
                check(guarded.after[k] == patterns[j], "after guard preserved");
            }
        }
    }
}
static void test_standalone_paths(void) {
    static const u32 values[] = {0, 1, 0xFFFFFFFFU, 0x80000000U, 0x76543210U};
    DreadzoneCoreStartupBlock block;
    DreadzoneCoreStartupContext context;
    u8 expected[0x1688];
    unsigned i;
    s32 retail_result;
    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        memset(&block, 0x39, sizeof(block));
        block.words[0x258 / 4] = 1;
        block.words[0x10 / 4] = values[i];
        block.words[0x218 / 4] = ~values[i];
        context = context_for(&block);
        context.state_0021DFE8 = values[i] ? values[i] : 0x223E40;
        memcpy(expected, block.bytes, sizeof(expected));
        oracle_word(expected, 0x1723B4, values[i]);
        expected[0x250] = 1;
        oracle_word(expected, 0x172380, ~values[i]);
        check(dreadzone_core_0015B498_slice(&context) == 0, "B498 actual scalar propagation");
        compare_bytes(block.bytes, expected);
        check((u32)context.selected_002120F4 == values[i], "B498 selected global preserves all bits");
        retail_result = -99;
        check(dreadzone_core_0015F4A0_existing_slice(&context, &retail_result) == 0 && retail_result == 1,
              "real F4A0 existing-state early return");
    }
    /* Standalone AA58 has a real level0 mode1 branch; A170 level0 remains
     * unresolved and this is not presented as an executable level0 boot. */
    memset(&block, 0x39, sizeof(block));
    block.words[0] = 8; block.words[0x258 / 4] = 1;
    context = context_for(&block); context.level_0021DE10 = 0;
    memcpy(expected, block.bytes, sizeof(expected));
    oracle_AA58(expected, 8, 0);
    check(dreadzone_core_0015AA58_slice(&context) == 0, "standalone AA58 level0 flag branch");
    compare_bytes(block.bytes, expected);
}
static void test_explicit_boundaries(void) {
    DreadzoneCoreStartupBlock block;
    DreadzoneCoreStartupContext context;
    u8 expected[0x1688];
    s32 result = 77;
    unsigned which;
    for (which = 0; which < 4; which++) {
        memset(&block, 0xA5, sizeof(block));
        block.words[0] = 0; block.words[0x258 / 4] = 1;
        context = context_for(&block);
        if (which == 0) context.level_0021DE10 = 0;
        if (which == 1) block.words[0x258 / 4] = 0;
        if (which == 2) context.state_0021DFE8 = 0;
        if (which == 3) context.callback_0021DB94 = 0x00600000;
        memcpy(expected, block.bytes, sizeof(expected));
        if (which < 2) {
            expected[0x1681] = 0;
            check(dreadzone_core_0015A170_slice(&context) == (which == 0 ?
                  DREADZONE_CORE_STARTUP_NEEDS_LEVEL_ZERO : DREADZONE_CORE_STARTUP_NEEDS_NETWORK),
                  "A170 unresolved boundary retains mandatory clear");
            if (which == 1) {
                check(dreadzone_core_0015AA58_slice(&context) == DREADZONE_CORE_STARTUP_NEEDS_NETWORK,
                      "AA58 mode0 unresolved boundary");
                check(dreadzone_core_0015B498_slice(&context) == DREADZONE_CORE_STARTUP_NEEDS_NETWORK,
                      "B498 mode0 unresolved boundary");
            }
        } else {
            int wanted = which == 2 ? DREADZONE_CORE_STARTUP_NEEDS_NETWORK : DREADZONE_CORE_STARTUP_NEEDS_CALLBACK;
            check(dreadzone_core_0015AA58_slice(&context) == wanted, "AA58 explicit boundary");
            check(dreadzone_core_0015B498_slice(&context) == wanted, "B498 explicit boundary");
            check(context.selected_002120F4 == 12345 && context.flag_0021DFA4 == 0xA5,
                  "boundary does not publish fake transition");
        }
        compare_bytes(block.bytes, expected);
    }
    memset(&block, 0xA5, sizeof(block));
    block.words[0] = 10; block.words[0x258 / 4] = 0;
    context = context_for(&block);
    context.level_0021DE10 = 0;
    context.state_0021DFE8 = 0;
    context.callback_0021DB94 = 0xFFFFFFFFU;
    memcpy(expected, block.bytes, sizeof(expected));
    expected[0x1681] = 0;
    check(dreadzone_core_0015A170_slice(&context) == 0, "A170 early exit precedes all unresolved domains");
    check(dreadzone_core_0015AA58_slice(&context) == 0, "AA58 early exit precedes all unresolved domains");
    compare_bytes(block.bytes, expected);
    context.state_0021DFE8 = 0;
    check(dreadzone_core_0015F4A0_existing_slice(&context, &result) == DREADZONE_CORE_STARTUP_NEEDS_NETWORK && result == 77,
          "F4A0 unresolved branch does not invent return value");
    check(dreadzone_core_0015F4A0_existing_slice(NULL, &result) == DREADZONE_CORE_STARTUP_INVALID_STORAGE,
          "F4A0 null context");
    check(dreadzone_core_0015F4A0_existing_slice(&context, NULL) == DREADZONE_CORE_STARTUP_INVALID_STORAGE,
          "F4A0 null result");
    check(dreadzone_core_0015A170_slice(NULL) == DREADZONE_CORE_STARTUP_INVALID_STORAGE, "A170 null context");
    check(dreadzone_core_0015AA58_slice(NULL) == DREADZONE_CORE_STARTUP_INVALID_STORAGE, "AA58 null context");
    check(dreadzone_core_0015B498_slice(NULL) == DREADZONE_CORE_STARTUP_INVALID_STORAGE, "B498 null context");
    context.block = NULL;
    check(dreadzone_core_0015A170_slice(&context) == DREADZONE_CORE_STARTUP_INVALID_STORAGE, "A170 null block");
    check(dreadzone_core_0015AA58_slice(&context) == DREADZONE_CORE_STARTUP_INVALID_STORAGE, "AA58 null block");
    check(dreadzone_core_0015B498_slice(&context) == DREADZONE_CORE_STARTUP_INVALID_STORAGE, "B498 null block");
}
int main(void) {
    test_real_level_one_sequence();
    test_standalone_paths();
    test_explicit_boundaries();
    printf("Dreadzone core startup: %u checks passed; mode1 slices, no network or callback substitutes\n", checks);
    return 0;
}
