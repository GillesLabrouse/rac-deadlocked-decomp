#include "common.h"

/* Complete actual L01 00575A18..00575B48 (304 bytes).
 * Copy selected animation/auxiliary definition fields into retained geometry,
 * preserving destination byte47 bit8, then reconcile actor.half36 bit8.
 * The source animation-pointer count is reloaded after every actual store;
 * no disjointness or alias assumption is added to this PS2 reconstruction. */
#ifndef DEADLOCKED_NATIVE
extern u8 *D_00222D14 __attribute__((section(".data")));
extern u8 *D_00222D30;
#define ABYTE(p,o) (*(volatile u8 *)((p)+(o)))
#define AHALF(p,o) (*(volatile u16 *)((p)+(o)))
#define AWORD(p,o) (*(volatile u32 *)((p)+(o)))
void func_L01_00575A18(u8 *destination,u8 *source)
{
    u8 flag;
    u8 *actor;
    s32 i;
    if(source==NULL || destination==NULL) return;
    flag=ABYTE(destination,0x47)&8;
    ABYTE(destination,0xC)=ABYTE(source,0xC);
    ABYTE(destination,0xD)=ABYTE(source,0xD);
    AWORD(destination,0x10)=AWORD(source,0x10);
    AWORD(destination,0x1C)=AWORD(source,0x1C);
    AWORD(destination,0x28)=AWORD(source,0x28);
    AHALF(destination,0x44)=AHALF(source,0x44);
    ABYTE(destination,0x47)=(ABYTE(source,0x47)&0xF7)|flag;
    ABYTE(destination,0x46)=ABYTE(source,0x46);
    for(i=0;i<ABYTE(source,0xC);i++)
        AWORD(destination,0x48+i*4)=AWORD(source,0x48+i*4);
    for(actor=D_00222D14;(u32)actor<(u32)D_00222D30;actor+=0x100) {
        if(AWORD(actor,0x24)!=0 && ABYTE(actor,0x20)<0xFD
            && AWORD(actor,0x24)==(u32)destination) {
            u16 flags=AHALF(actor,0x36);
            if(flags&8) {
                if(!(ABYTE(destination,0x47)&8)) AHALF(actor,0x36)=flags&0xFFF7;
            } else if(ABYTE(destination,0x47)&8) AHALF(actor,0x36)=flags|8;
        }
    }
}
#endif
