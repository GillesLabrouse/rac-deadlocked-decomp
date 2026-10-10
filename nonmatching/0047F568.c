/* cflags: -mno-split-addresses */
/* Request preflight and state snapshot draft at 0x0047F568.
 * NONMATCHING: remaining register allocation and instruction scheduling differ.
 * Experimental native tests execute this algorithm, but cannot establish full
 * binary equivalence or game launch. The accepted matching core excludes it.
 *
 * The fifth argument is an optional completion byte. Native GP/absolute aliases
 * share one scalar storage location; host pointers retain their full width.
 * Inputs are internal game state: valid request-table indices and snapshot
 * counts must be supplied by the caller. Do not infer their gameplay meanings.
 */
#include "common.h"

typedef struct RequestObject {
    u8 pad0[0x2E20];
    f32 f2E20;
} RequestObject;
typedef struct RequestSnapshot { s32 state; s32 parameter2; s32 parameter3; } RequestSnapshot;

#ifdef DEADLOCKED_NATIVE
extern s32 D_0021DDB4_[];
#define D_0021DDB4_ (D_0021DDB4_[0])
#else
extern s32 D_0021DDB4_ __attribute__((section(".data")));
#endif
#ifdef DEADLOCKED_NATIVE
extern s32 D_0021EDC4_[];
#define D_0021EDC4_ (D_0021EDC4_[0])
#else
extern s32 D_0021EDC4_ __attribute__((section(".data")));
#endif
#ifdef DEADLOCKED_NATIVE
extern s32 D_0021EDD4_[];
#define D_0021EDD4_ (D_0021EDD4_[0])
#else
extern s32 D_0021EDD4_ __attribute__((section(".data")));
#endif
#ifdef DEADLOCKED_NATIVE
extern s32 D_0021EDD8_[];
#define D_0021EDD8_ (D_0021EDD8_[0])
#else
extern s32 D_0021EDD8_ __attribute__((section(".data")));
#endif
extern u8 D_003D7D70[];
extern RequestObject *D_003367B8[];
extern RequestSnapshot D_00270040[];
extern s32 D_0021F2F4;
extern s32 D_0021F2E8;
extern s32 D_0021F2EC;
extern u8 *D_0021F2F0;
extern s32 D_0021EEB4;
extern s32 D_0021DAE0;

#ifdef DEADLOCKED_NATIVE
#define D_0021DDB4 (D_0021DDB4_)
#define D_0021EDC4 (D_0021EDC4_)
#else
extern s32 D_0021DDB4;
extern s32 D_0021EDC4;
#endif

extern s32 func_00465C08(void);

s32 func_0047F568(s32 request, s32 mode, s32 parameter2, s32 parameter3, u8 *completion) {
    s32 result, count, index, byte_offset;
    u8 blocked;
    RequestObject *object;
    RequestSnapshot *snapshot;

    if (completion != 0) *completion = 0;
    result = (D_0021EDC4_ ^ -2) == 0 ? 0 : -1;
    if (mode == 1) result = D_0021F2F4 >= 8 ? -2 : result;
    blocked = request >= 0 ? D_003D7D70[request] : 0;
    result = blocked != 0 ? -3 : result;
    if (D_0021EDD4_ != 1) {
        count = 0;
        object = D_003367B8[0];
        index = 0;
        while (index < 10) {
            if (object != 0 && object->f2E20 == 0.0f &&
                (++count, D_0021DDB4 != 0 || func_00465C08() != 3) &&
                (D_0021EDD4_ != 0 || D_0021EDD8_ != 1 || count >= 2 || D_0021EEB4 == 0)) {
                result = -4;
                break;
            }
            index++;
            object = D_003367B8[index];
        }
    }
    if (result == 0) {
        if (mode != 1) D_0021EDC4 = request;
        else {
            byte_offset = D_0021F2F4 * 12;
            D_0021F2F4++;
            snapshot = (RequestSnapshot *)((u8 *)D_00270040 + byte_offset);
            *(s32 *)((u8 *)D_00270040 + byte_offset) = D_0021DDB4_;
            snapshot->parameter2 = D_0021F2E8;
            snapshot->parameter3 = D_0021F2EC;
            D_0021EDC4_ = request;
        }
        D_0021F2E8 = parameter2;
        D_0021F2EC = parameter3;
        D_0021F2F0 = completion;
    }
    return result;
}

