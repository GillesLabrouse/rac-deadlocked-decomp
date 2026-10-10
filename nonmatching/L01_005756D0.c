#include "common.h"

/* Complete actual L01 005756D0..00575A18 (840 bytes).
 * Index and slot selector are full-width s32 inputs. Selected global armor
 * descriptors contain sector offset/count pairs, not host byte pointers.
 * This retains all real reads, GPU packet uploads, waits and class relocation.
 * Loading an armor's static definition alone does not establish animation:
 * the separate 00575A18 copies the already loaded class animation pointers.
 * No executable native path or successful disk/GPU substitute is provided. */
#ifndef DEADLOCKED_NATIVE
extern u8 D_001DFC60[] __attribute__((section(".data")));
extern u8 D_001DFCF0[] __attribute__((section(".data")));
extern u8 D_001C2040[] __attribute__((section(".data")));
extern u8 D_002699A0[] __attribute__((section(".data")));
extern u8 D_0021EB00[];
void func_L01_00633558(s32);
void func_L01_005B68F8(s32);
s32 func_001634A8(s32);
s32 func_00163808(s32,void *,u32,u32,s32,void *,void *);
s32 func_00138E08(u8 *,s32,s32,s32,s32,s32,s32,s32);
void func_00126E20(s32);
void func_00139130(u8 *,u8 *);
void func_00131A08(s32,s32);
void func_L01_00576BC0(u8 *,u8 *,u8 *,s32);
#define LWORD(p,o) (*(u32 *)((p)+(o)))
#define LPTR(p,o) (*(u8 **)((p)+(o)))
static inline s32 armor_exponent(s32 value)
{
    s32 leading;
    __asm__("plzcw %0,%1" : "=r"(leading) : "r"(value));
    return 30-leading;
}
void func_L01_005756D0(s32 index,s32 secondary)
{
    u8 packet[0x60];
    u8 *slot,*definition,*descriptors,*texture,*output;
    u32 paletteAddress,pixelAddress;
    s32 i,count,palette256,pixels256,row,exponent;
    u64 value;
    slot=D_001DFC60+(secondary!=0 ? 0x20 : 0);
    LWORD(slot,0)=(u32)index;
    definition=LPTR(slot,4);
    output=slot+0x10;
    paletteAddress=LWORD(D_001DFCF0,secondary!=0 ? 0x1C : 0x18);
    pixelAddress=LWORD(D_001DFCF0,secondary!=0 ? 0x34 : 0x30);
    func_L01_00633558(1);
    func_L01_005B68F8(1);
    func_001634A8(1);
    descriptors=D_001C2040+(u32)index*16U;
    func_00163808(0,definition,LWORD(D_001C2040,0x9AC)+LWORD(descriptors,0x9B8),
                  LWORD(descriptors,0x9BC),0,NULL,NULL);
    count=(s32)LWORD(definition,0);
    LWORD(slot,8)=(u32)count;
    for(i=0;i<(s32)*(volatile u32 *)(slot+8);i++) {
        texture=definition+LWORD(definition,4+i*4);
        palette256=(s32)paletteAddress>>8;
        pixels256=(s32)pixelAddress>>8;
        func_00138E08(packet,palette256,1,0,0,0,16,16);
        func_00126E20(0);
        func_00139130(packet,texture+0x20);
        paletteAddress+=0x400;
        func_00131A08(0,0);
        row=(s32)LWORD(texture,8)>>6;
        if(row<=0) row=1;
        exponent=armor_exponent((s32)LWORD(texture,8));
        func_00138E08(packet,pixels256,row,0x1B,0,0,
                      *(s16 *)(texture+8),*(s16 *)(texture+0xC));
        func_00126E20(0);
        func_00139130(packet,texture+0x420);
        func_00131A08(0,0);
        value=(u64)(s64)pixels256 | ((u64)(s64)row<<14)
            | ((u64)(s64)exponent<<26) | 0x01B00000ULL
            | ((u64)(s64)exponent<<30) | ((u64)(s64)palette256<<37)
            | 0x8000000000000000ULL | ((u64)0x8000<<19);
        *(u64 *)(output+i*8)=value;
        *(u64 *)(D_002699A0+i*24)=value;
        *(u64 *)(D_002699A0+i*24+8)=0x0000FFA0000000E0ULL;
        *(u64 *)(D_002699A0+i*24+16)=((u64)0x8000<<20)|((u64)0x8000<<19)|0x4000;
        pixelAddress+=(u32)(LWORD(texture,8)*LWORD(texture,0xC))*4U;
    }
    func_001634A8(1);
    func_00163808(0,definition,LWORD(D_001C2040,0x9AC)+LWORD(descriptors,0x9B0),
                  LWORD(descriptors,0x9B4),0,NULL,NULL);
    func_L01_00576BC0(definition,NULL,D_0021EB00,-1);
}
#endif
