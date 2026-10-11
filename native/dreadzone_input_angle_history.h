#ifndef DEADLOCKED_DREADZONE_INPUT_ANGLE_HISTORY_H
#define DEADLOCKED_DREADZONE_INPUT_ANGLE_HISTORY_H
#include "common.h"

typedef struct DreadzoneInputAngleHistoryRequest {
    const u8 *storage;
    u32 guest_base,bytes,input_guest;
    s32 requested;
} DreadzoneInputAngleHistoryRequest;

enum DreadzoneInputAngleHistoryResult {
    DREADZONE_INPUT_ANGLE_HISTORY_COMPLETED=0,
    DREADZONE_INPUT_ANGLE_HISTORY_INVALID=-1,
    DREADZONE_INPUT_ANGLE_HISTORY_NEEDS_MEMORY=-2,
    DREADZONE_INPUT_ANGLE_HISTORY_UNALIGNED=-3
};
/* Complete005BD900 query. A null input_guest selects actual001EE600, which
 * must be mapped by the caller. Signed head18E, signed count190, wrapped32
 * subtraction/addition and signed DIV remainder select the raw word at258.
 * Negative remainders can genuinely read before the thirty angle slots.
 * The result retains every LWC1 bit, including exceptional float patterns;
 * this function performs no float arithmetic or controller transport.
 *
 * Supply one stable readable nonwrapping host/guest allocation. Missing and
 * misaligned reached reads refuse before changing the output. Request/output
 * may overlap source backing: request is retained, then all reads precede
 * result publication. Output backing must be writable; allocations and
 * exclusive access are the caller's responsibility. */
int dreadzone_input_angle_history_005BD900(const DreadzoneInputAngleHistoryRequest *,u32 *result_bits);
#endif
