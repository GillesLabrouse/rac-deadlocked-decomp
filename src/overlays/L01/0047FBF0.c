#include "common.h"
/* Null follows the branch to the shared one-return store. Nonnull actors
 * with either signed class1099 or171C do likewise; other classes return0. */
s32 func_L01_0047FBF0(void *actor) {
    s32 result=0;
    s16 class_id;
    if(actor==NULL)goto return_one;
    class_id=*(s16 *)((u8 *)actor+0xBC);
    if(class_id==0x1099)goto return_one;
    if(class_id!=0x171C)goto return_result;
return_one:
    result=1;
return_result:
    return result;
}
