#include "dreadzone_callback_table.h"
#include <string.h>

int dreadzone_callback_table_006C1C00(const DreadzoneCallbackTableMemory *ram) {
    DreadzoneCallbackTableMemory view;
    uintptr_t descriptor, backing;
    size_t offset, i;
    static const struct { uint8_t offset; uint32_t code; } writes[] = {
        {0x00, 0x006C1E28u}, {0x44, 0x006C24E0u}, {0x48, 0x006C26C0u},
        {0x04, 0x006C1EB8u}, {0x0C, 0x006C2320u}, {0x10, 0x006C23E0u},
        {0x14, 0x006C2670u}, {0x24, 0x006C2098u}, {0x28, 0x006C2170u},
        {0x2C, 0x006C2200u}, {0x38, 0x006C2278u}, {0x30, 0x006C26A0u},
        {0x3C, 0x006C2410u}, {0x40, 0x006C2498u}
    };
    if (ram == NULL) return -1;
    descriptor = (uintptr_t)(const void *)ram;
    if (sizeof(*ram) > UINTPTR_MAX - descriptor) return -1;
    memcpy(&view, ram, sizeof(view));
    if (view.bytes == NULL || view.length == 0 || view.length > 0x02000000u ||
        view.guest_base > 0x00212148u ||
        (uint64_t)view.guest_base + view.length > UINT64_C(0x80000000)) return -1;
    backing = (uintptr_t)(void *)view.bytes;
    if (view.length > UINTPTR_MAX - backing ||
        (descriptor < backing + view.length && backing < descriptor + sizeof(*ram))) return -1;
    offset = (size_t)(0x00212148u - view.guest_base);
    if (offset > view.length || 0x4Cu > view.length - offset) return -1;
    for (i = 0; i < sizeof(writes)/sizeof(writes[0]); ++i) {
        uint8_t *destination = view.bytes + offset + writes[i].offset;
        uint32_t value = writes[i].code;
        destination[0] = (uint8_t)value;
        destination[1] = (uint8_t)(value >> 8);
        destination[2] = (uint8_t)(value >> 16);
        destination[3] = (uint8_t)(value >> 24);
    }
    return 0;
}
