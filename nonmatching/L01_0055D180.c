/* Complete 48-byte L01 signed timer helper.
 * NONMATCHING; baseline SN compiler flags. */
#include "common.h"
s32 func_L01_0055D180(s16 *timer) {
    s32 value=*timer;
    if(value==0) return 1;
    if(value<1) value=1;
    --value;
    *timer=(s16)value;
    return value>0?0:2;
}
