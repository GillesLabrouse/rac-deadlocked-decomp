#include "common.h"
#ifndef DEADLOCKED_NATIVE
void func_L01_0058AA90(u8 *,u8 *,s32);
void func_L01_005919B8(u8 *,s32,s32,f32);
s32 func_L01_00669250(u8 *,s32);
void func_L01_00688C08(u8 *,s32,s32,s32,f32);
/* Complete456-byte cleanup, preserving both history modes and real calls. */
void func_L01_00675430(u8 *player) {
    s32 index;
    u8 *node;
    *(u32 *)(player+0x7F4)=0;
    if(player[0x265A]!=0) {
        player[0x265A]=0;player[0x2658]=0;
        if(*(u32 *)(player+0x7E8)!=0) {
            for(index=0;index<2;++index) {
                node=*(u8 **)(player+0x7E8+index*4);
                if(node!=NULL) {
                    func_L01_0058AA90(*(u8 **)(player+0x2EF0),node,10);
                    *(u32 *)(player+0x7E8+index*4)=0;
                }
            }
            *(u32 *)(player+0x7F4)=0;
        }
        if(*(s32 *)(player+0x25CC)==0 &&
           (*(u8 **)(player+0x2EF0))[0x54]==*(s32 *)(player+0x2DD4)) {
            if(*(s32 *)(player+0x2318)==3)
                func_L01_00688C08(player,0x3E,0,0,-2.f);
            else func_L01_00688C08(player,func_L01_00669250(player,0),0,0,-2.f);
        }
        player[0x2658]=0;player[0x265A]=0;
    } else {
        player[0x265A]=0;player[0x2658]=0;
        for(index=0;index<2;++index) {
            node=*(u8 **)(player+0x7E8+index*4);
            if(node!=NULL) {
                func_L01_0058AA90(*(u8 **)(player+0x2EF0),node,0);
                *(u32 *)(player+0x7E8+index*4)=0;
            }
        }
        if(*(s32 *)(player+0x25CC)==0 &&
           (*(u8 **)(player+0x2EF0))[0x54]==*(s32 *)(player+0x2DD4)) {
            if(*(s32 *)(player+0x2318)==3) {
                func_L01_00688C08(player,0x3E,0,0,-2.f);
                func_L01_005919B8(*(u8 **)(player+0x22F0),4,-2,0.f);
            } else func_L01_00688C08(player,func_L01_00669250(player,0),0,0,-2.f);
        }
    }
}
#endif
