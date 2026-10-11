#include "common.h"

/* Complete L01 00542FF8..00543028 (0x30 including final padding).
 * NONMATCHING until independently linked: the actual packet metadata is
 * signed before the low nibble is inspected; 15 is the absent surface.
 */
extern volatile s32 D_00242340[];
s32 func_L01_00542FF8(void)
{
    s32 metadata=D_00242340[7];
    if(metadata<0)return -1;
    metadata&=15;
    if(metadata==15)return -1;
    return metadata;
}
