#include "common.h"

/* The completion callback receives unused request data in A0. */
s32 func_001639F0(void *request_data, u32 *completion_word) {
    return *completion_word;
}
