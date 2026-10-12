#include "dreadzone_descriptor_rows.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define RAM_CAP ((size_t)0x02000000u)
#define GUEST_LIMIT UINT64_C(0x80000000)
#define ROW_SIZE ((size_t)32)
#define HEADER_SIZE ((size_t)16)
#define OUTPUT_BASE_CELL UINT32_C(0x00222990)
#define OUTPUT_COUNT_CELL UINT32_C(0x00222994)

static int ranges_overlap(uint64_t a, uint64_t an, uint64_t b, uint64_t bn) {
    return an != 0 && bn != 0 && a < b + bn && b < a + an;
}

static int span(const DreadzoneDescriptorRowsMemory *ram, uint32_t address, size_t length,
                  size_t *offset) {
    uint64_t begin = ram->guest_base;
    uint64_t map_end = begin + (uint64_t)ram->length;
    uint64_t request_begin = address;
    uint64_t request_end = request_begin + (uint64_t)length;
    uint64_t delta;
    if (request_begin < begin || request_end > map_end ||
        request_end > GUEST_LIMIT) {
        return 0;
    }
    delta = request_begin - begin;
    if (delta > (uint64_t)SIZE_MAX || length > SIZE_MAX - (size_t)delta) {
        return 0;
    }
    *offset = (size_t)delta;
    return 1;
}

static uint32_t read_le32_(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void write_le32_(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

int dreadzone_descriptor_rows_0057B498(const DreadzoneDescriptorRowsMemory *ram,
                                uint32_t source_guest,
                                uint32_t cursor_word_guest) {
    DreadzoneDescriptorRowsMemory snapshot;
    uintptr_t descriptor_begin, backing_begin;
    uint32_t count, cursor_value;
    uint64_t rows_size, source_size, output_end, advanced_cursor;
    uint64_t source_end, next_cursor;
    size_t count_offset, source_offset, cursor_offset;
    size_t output_offset = 0, base_global_offset, count_global_offset;
    uint8_t *staged_source = NULL, *staged_output = NULL;
    uint8_t *bytes;
    size_t row, word;
    static const uint8_t source_word_for_output[8] = {1, 2, 3, 0, 4, 5, 6, 7};

    if (ram == NULL) {
        return -1;
    }
    descriptor_begin = (uintptr_t)(const void *)ram;
    if (sizeof(*ram) > UINTPTR_MAX - descriptor_begin) {
        return -1;
    }
    /* The caller guarantees readable descriptor storage; do not probe pointers. */
    memcpy(&snapshot, ram, sizeof(snapshot));
    bytes = snapshot.bytes;
    if (bytes == NULL || snapshot.length == 0 || snapshot.length > RAM_CAP ||
        snapshot.guest_base >= GUEST_LIMIT ||
        (uint64_t)snapshot.length > GUEST_LIMIT - snapshot.guest_base ||
        (source_guest & 3u) != 0 || (cursor_word_guest & 3u) != 0) {
        return -1;
    }
    backing_begin = (uintptr_t)(void *)bytes;
    if (snapshot.length > UINTPTR_MAX - backing_begin ||
        ranges_overlap(descriptor_begin, sizeof(*ram), backing_begin,
                         snapshot.length)) {
        return -1;
    }
    if (!span(&snapshot, source_guest, HEADER_SIZE, &count_offset) ||
        !span(&snapshot, cursor_word_guest, 4, &cursor_offset) ||
        !span(&snapshot, OUTPUT_BASE_CELL, 4, &base_global_offset) ||
        !span(&snapshot, OUTPUT_COUNT_CELL, 4, &count_global_offset)) {
        return -1;
    }

    count = read_le32_(bytes + count_offset);
    if ((count & UINT32_C(0x80000000)) != 0 || count > RAM_CAP / ROW_SIZE) {
        return -1;
    }
    rows_size = (uint64_t)count * ROW_SIZE;
    source_size = HEADER_SIZE + rows_size;
    source_end = (uint64_t)source_guest + source_size;
    if (source_end > GUEST_LIMIT ||
        !span(&snapshot, source_guest, (size_t)source_size, &source_offset)) {
        return -1;
    }
    cursor_value = read_le32_(bytes + cursor_offset);
    if (cursor_value >= GUEST_LIMIT || (cursor_value & 3u) != 0) {
        return -1;
    }
    advanced_cursor = (uint64_t)cursor_value + rows_size;
    if (advanced_cursor >= GUEST_LIMIT) {
        return -1;
    }
    next_cursor = (advanced_cursor + UINT64_C(0x3F)) & ~UINT64_C(0x3F);
    if (next_cursor >= GUEST_LIMIT || next_cursor > UINT32_MAX) {
        return -1;
    }
    output_end = (uint64_t)cursor_value + rows_size;
    if (output_end > GUEST_LIMIT ||
        !span(&snapshot, cursor_value, (size_t)rows_size, &output_offset)) {
        return -1;
    }

    /* Refuse aliasing that would make the sequential retail zero/copy order ambiguous. */
    if (ranges_overlap(source_guest, source_size, cursor_word_guest, 4) ||
        ranges_overlap(source_guest, source_size, cursor_value, rows_size) ||
        ranges_overlap(source_guest, source_size, OUTPUT_BASE_CELL, 8) ||
        ranges_overlap(cursor_word_guest, 4, cursor_value, rows_size) ||
        ranges_overlap(cursor_word_guest, 4, OUTPUT_BASE_CELL, 8) ||
        ranges_overlap(cursor_value, rows_size, OUTPUT_BASE_CELL, 8)) {
        return -1;
    }

    if (rows_size != 0) {
        staged_source = (uint8_t *)malloc((size_t)rows_size);
        staged_output = (uint8_t *)malloc((size_t)rows_size);
        if (staged_source == NULL || staged_output == NULL) {
            free(staged_source);
            free(staged_output);
            return -1;
        }
        memcpy(staged_source, bytes + source_offset + HEADER_SIZE,
               (size_t)rows_size);
        for (row = 0; row < (size_t)count; ++row) {
            for (word = 0; word < 8; ++word) {
                memcpy(staged_output + row * ROW_SIZE + word * 4,
                       staged_source + row * ROW_SIZE +
                           source_word_for_output[word] * 4,
                       4);
            }
        }
    }

    /* Commit only after descriptor, source, output, cursor, globals and staging pass. */
    write_le32_(bytes + base_global_offset, cursor_value);
    write_le32_(bytes + count_global_offset, count);
    write_le32_(bytes + cursor_offset, (uint32_t)next_cursor);
    if (rows_size != 0) {
        memset(bytes + output_offset, 0, (size_t)rows_size);
        memcpy(bytes + output_offset, staged_output, (size_t)rows_size);
    }
    free(staged_source);
    free(staged_output);
    return 0;
}
