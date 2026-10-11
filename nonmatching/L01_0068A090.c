#include "common.h"

/* Complete actual L01 0068A090..0068A178. Matching is not established.
 * Reads original signed mode2F8 and real globals; metadata query index18.
 * Return is the actual mode, not a boolean and not record+2659 publication. */
extern s32 D_0021E878_[];
extern s32 D_0021E87C;
extern s32 func_L01_006BF278(void *metadata,s32 index);
#define HALF(o) (*(s16 *)(record+(o)))
#define WORD(o) (*(s32 *)(record+(o)))

s16 func_L01_0068A090(u8 *record) {
    s16 mode=HALF(0x2F8);
    s32 state;
    if(D_0021E878_[0]!=0 && D_0021E87C==0)mode=2;
    if(D_0021E87C!=0 || (state=WORD(0x25CC))==59 ||
        (state==112 && record[0x438]==0) || state==111 || HALF(0x3D6)!=0 ||
        (record[0x437]!=0 && func_L01_006BF278(*(void **)(record+0x26E4),18)!=0 &&
         WORD(0x25CC)==0 && HALF(0x2FE)<4))mode=1;
    if(mode==1 && record[0x438]!=0 && record[0x437]==0)mode=0;
    return mode;
}
