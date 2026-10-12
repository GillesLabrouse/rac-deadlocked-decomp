#include "dreadzone_client_history_lookup.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TABLE_BYTES 0x2100U
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "history lookup check failed line%d\n", __LINE__); return 1; } } while (0)
static void key(uint8_t *ram, uint32_t slot, uint16_t value)
{
    ram[slot * 0xB0U + 0x86U] = (uint8_t)value;
    ram[slot * 0xB0U + 0x87U] = (uint8_t)(value >> 8);
}
int main(void)
{
    uint8_t *allocation = (uint8_t *)malloc(TABLE_BYTES + 26);
    uint8_t *before = (uint8_t *)malloc(TABLE_BYTES + 26), *ram;
    uint32_t selected, index, slot, table;
    CHECK(allocation && before);
    ram = allocation + 13;
    memset(allocation, 0xA5, TABLE_BYTES + 26);
    for (index = 0; index < 4; index++) {
        table = 0x231AD0U + index * TABLE_BYTES;
        for (slot = 0; slot < 48; slot++) key(ram, slot, 0xFFFFU);
        key(ram, 17, 0x8000U); key(ram, 47, 0x8000U);
        memcpy(before, allocation, TABLE_BYTES + 26);
        CHECK(dreadzone_client_history_lookup_005353C8(ram, table, TABLE_BYTES, index, -32768, &selected) == 1);
        CHECK(selected == table + 17U * 0xB0U);
        CHECK(!memcmp(before, allocation, TABLE_BYTES + 26));
        key(ram, 17, 0xFFFFU);
        CHECK(dreadzone_client_history_lookup_005353C8(ram, table, TABLE_BYTES, index, -32768, &selected) == 1);
        CHECK(selected == table + 47U * 0xB0U);
        CHECK(dreadzone_client_history_lookup_005353C8(ram, table, TABLE_BYTES, index, 65536, &selected) == 1 && selected == 0);
        CHECK(dreadzone_client_history_lookup_005353C8(ram, table, TABLE_BYTES, index, INT32_MIN, &selected) == 1 && selected == 0);
    }
    table = 0x231AD0U;
    selected = 0xDEADBEEFU; memcpy(before, allocation, TABLE_BYTES + 26);
    CHECK(dreadzone_client_history_lookup_005353C8(ram, table, TABLE_BYTES - 1, 0, 0, &selected) == -1);
    CHECK(dreadzone_client_history_lookup_005353C8(ram, table + 1, TABLE_BYTES, 0, 0, &selected) == -1);
    CHECK(dreadzone_client_history_lookup_005353C8(ram, table, TABLE_BYTES, 4, 0, &selected) == -2);
    CHECK(dreadzone_client_history_lookup_005353C8(ram, table, TABLE_BYTES, UINT32_MAX, 0, &selected) == -2);
    CHECK(dreadzone_client_history_lookup_005353C8(NULL, table, TABLE_BYTES, 0, 0, &selected) == -1);
    CHECK(dreadzone_client_history_lookup_005353C8(ram, table, TABLE_BYTES, 0, 0, NULL) == -1);
    CHECK(dreadzone_client_history_lookup_005353C8(ram, table, TABLE_BYTES, 0, 0, (uint32_t *)(allocation + 16)) == -1);
    CHECK(dreadzone_client_history_lookup_005353C8(ram, table, 0x1000001U, 0, 0, &selected) == -1);
    CHECK(selected == 0xDEADBEEFU && !memcmp(before, allocation, TABLE_BYTES + 26));
    free(before); free(allocation);
    puts("PASS client history signed lookup, first/last match, unchanged RAM and atomic refusals");
    return 0;
}
