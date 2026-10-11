#include "common.h"
#ifndef DEADLOCKED_NATIVE
u8 *func_L01_0058A850(u8 *);
s32 func_L01_00688E60(u8 *,s32);
void func_L01_005A36B0(u8 *,s32,s32);
void func_L01_00688A60(u8 *,s32,s32,f32);
void func_L01_00675430(u8 *);
extern s32 D_00220798[];
/* Complete600-byte transition, with every actual mode/callee retained. */
void func_L01_00688C08(u8 *player,s32 clip,s32 frame,s32 flags,f32 blend) {
    u8 *actor,*node,*definition,*other,*classBase;
    f32 decay;
    if(blend<0.f) blend=(f32)D_00220798[*(s32 *)(player+0x650)];
    actor=*(u8 **)(player+0x2EF0);
    node=func_L01_0058A850(actor);
    if(node!=NULL) {
        *(u8 **)(node+0x10)=*(u8 **)(actor+0x50);
        *(u8 **)(actor+0x50)=node;
        decay=1.f/blend;
        *(u32 *)node=*(u32 *)(actor+0x40);
        *(f32 *)(node+8)=0.f;
        *(u32 *)(node+4)=*(u32 *)(actor+0x44);
        *(u16 *)(node+0xC)=*(u16 *)(actor+0x4C);
        node[0x14]=actor[0x54];node[0x17]=0xFF;node[0x15]=actor[0x55];
        *(f32 *)(node+0x18)=1.f-decay;*(f32 *)(node+0x1C)=decay;node[0x16]=0xFF;
    }
    classBase=*(u8 **)(actor+0x24);
    actor[0x54]=(u8)clip;
    definition=*(u8 **)(classBase+0x48+clip*4);
    *(f32 *)(actor+0x44)=(f32)frame;*(u8 **)(actor+0x40)=definition;
    *(f32 *)(actor+0x48)=1.f;
    *(u16 *)(actor+0x4C)=*(u16 *)(definition+0x20+frame*4);
    *(f32 *)(player+0x640)=1.f;*(s32 *)(player+0x644)=*(s16 *)(actor+0x4C);
    if(func_L01_00688E60(player,clip)!=0) {
        other=*(u8 **)(player+0x2EF0);
        other[0x55]&=0xEF;*(u32 *)(player+0x648)&=~0x10U;
    } else {
        other=*(u8 **)(player+0x2EF0);
        other[0x55]|=0x10;*(u32 *)(player+0x648)|=0x10;
    }
    if((u32)(frame-1)<11U && !(flags&8)) func_L01_005A36B0(actor,clip,frame&0xFFFF);
    definition=*(u8 **)(*(u8 **)(actor+0x24)+0x48+clip*4);
    actor[0x6D]=definition[0x11];
    definition=*(u8 **)(*(u8 **)(actor+0x24)+0x48+clip*4);
    actor[0x6C]=definition[0x12];
    if(flags&0x10) *(u16 *)(actor+0x36)|=4;
    else *(u16 *)(actor+0x36)&=0xFFFB;
    if(*(s16 *)(player+0x2E2C)==0) func_L01_00688A60(player,clip,frame,blend);
    if(player[0x2658]==0 && clip!=*(s32 *)(player+0x2DD4)) func_L01_00675430(player);
}
#endif
