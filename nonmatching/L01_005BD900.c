/* Complete 76-byte angle-history leaf (72 executable bytes and 4 padding).
 * NONMATCHING recovery draft; bounded original/SN replay validates behavior.
 */
#include "common.h"

extern u8 D_001EE600[];

f32 func_L01_005BD900(s32 requested,u8 *input) {
    s32 count,index;
    if (!input) input=D_001EE600;
    count=*(s32 *)(input+0x190);
    if (count<requested) requested=count;
    /* SUBU/ADDIU wrap before the signed DIV remainder. */
    index=(s32)((u32)(s32)*(s16 *)(input+0x18E)-(u32)requested+30U);
    index%=30;
    return *(f32 *)(input+0x258+index*4);
}
