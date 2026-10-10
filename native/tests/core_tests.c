#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(sizeof(s32) == 4, "Game words must be 32 bits");
_Static_assert(sizeof(s64) == 8, "Callback contexts must be 64 bits");
_Static_assert(sizeof(u64) == 8, "Unsigned game doublewords must be 64 bits");
_Static_assert(sizeof(f32) == 4 && sizeof(f64) == 8, "Game float widths must be preserved");

typedef void (*Callback)(s32, s64);
typedef struct CommandPayload { s32 values[6]; } CommandPayload;
_Static_assert(sizeof(CommandPayload) == 24, "Command records must retain their game layout");

extern u32 func_00139A58(u32);
extern s32 func_001585C8(s32, Callback, s64);
extern s32 func_00158630(s32, CommandPayload *);
extern s32 func_001586F0(s32, s32, s32);
extern s32 func_00158780(s32, s32, s32, s32, s32, s32, Callback, s64);
extern s32 func_00158988(s32, s32, s32, s32, s32, s32);
extern s32 func_001589A8(s32, s32, s32, s32, s32, s32, Callback, s64);

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "Check failed at line %d: %s\n", __LINE__, #condition); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

/* Test transport only: no PS2 driver or game runtime is provided here. */
static s32 command;
static s32 payload_size;
static s32 words[7];
static Callback callback;
static s64 context;
static unsigned callback_calls;
static s64 delivered_context;

s32 func_00158C60(s32 code, s32 size, void *payload, Callback completion, s64 value) {
    CHECK(size >= 0 && size <= (s32)sizeof(words));
    command = code;
    payload_size = size;
    callback = completion;
    context = value;
    if (size != 0) {
        CHECK(payload != NULL);
        /* Copy the representation; absent optional payload tails are unspecified. */
        memcpy(words, payload, (size_t)size);
    }
    return -73;
}

static void completed(s32 result, s64 value) {
    CHECK(result == -73);
    callback_calls++;
    delivered_context = value;
}

static void check_six_words(void) {
    const s32 expected[6] = { INT32_MIN, INT32_MAX, -1, 0, 17, -29 };
    CHECK(payload_size == 24);
    CHECK(memcmp(words, expected, sizeof(expected)) == 0);
}

int main(void) {
    const s64 positive_context = INT64_C(0x1234567887654321);
    const s64 negative_context = -INT64_C(0x1234567887654321);
    CommandPayload optional = {{ INT32_MIN, INT32_MAX, -1, 0, 17, -29 }};

    CHECK(func_001585C8(INT32_MIN, completed, positive_context) == -73);
    CHECK(command == 6 && payload_size == 4 && words[0] == INT32_MIN);
    CHECK(callback == completed && context == positive_context);
    callback(-73, context);
    CHECK(callback_calls == 1 && delivered_context == positive_context);

    CHECK(func_00158780(INT32_MIN, INT32_MAX, -1, 0, 17, -29,
                       completed, negative_context) == -73);
    CHECK(command == 0x11 && callback == completed && context == negative_context);
    check_six_words();
    callback(-73, context);
    CHECK(callback_calls == 2 && delivered_context == negative_context);

    CHECK(func_001589A8(INT32_MIN, INT32_MAX, -1, 0, 17, -29,
                       completed, positive_context) == -73);
    CHECK(command == 0x21 && callback == completed && context == positive_context);
    check_six_words();

    CHECK(func_00158988(INT32_MIN, INT32_MAX, -1, 0, 17, -29) == -73);
    CHECK(command == 0x21 && callback == NULL && context == 0);
    check_six_words();

    CHECK(func_001586F0(INT32_MIN, INT32_MAX, -1) == -73);
    CHECK(command == 0x4E && payload_size == 12 && callback == NULL && context == 0);
    CHECK(words[0] == INT32_MIN && words[1] == INT32_MAX && words[2] == -1);

    CHECK(func_00158630(101, &optional) == -73);
    CHECK(command == 0x60 && payload_size == 28 && words[0] == 101);
    CHECK(memcmp(words + 1, optional.values, sizeof(optional.values)) == 0);
    CHECK(func_00158630(102, NULL) == -73);
    CHECK(words[0] == 102 && words[1] == -1);
    /* The original function leaves words[2..6] unspecified in this case. */

    /* These are guest address values, not native host pointers. */
    CHECK(func_00139A58(UINT32_C(0x71234567)) == UINT32_C(0x81234567));
    CHECK(func_00139A58(UINT32_C(0x6FFFFFFF)) == UINT32_C(0x6FFFFFFF));
    CHECK(func_00139A58(UINT32_C(0x80000000)) == UINT32_C(0x80000000));
    puts("Native scalar widths, payload layouts and callback forwarding passed.");
    return EXIT_SUCCESS;
}
