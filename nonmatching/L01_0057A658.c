#include "common.h"

/* Actual L01 entry 0057A658; final return delay slot at0057AB98, followed
 * by a zero marker. Independently reviewed extent: 0x548 bytes.
 *
 * This is a PS2 reconstruction, not a native texture backend. The PIF path
 * retains the actual GS permutation, GIF upload, syscall and completion
 * dependencies. Their effects must exist before class publication can be
 * treated as executable on another platform.
 *
 * Retail clears only the sixteen remap bytes. In particular descriptor
 * +0, +0xC and +0xE are NOT initialized for a one-mip PIF. The explicit
 * local frame preserves that missing initialization and the adjacency of
 * the descriptor region and transfer packet; no zero-filled stand-in is
 * supplied to the downstream geometry patcher. C compilation still needs
 * stack-layout comparison before claiming the original incoming stack
 * bytes are reproduced. There is deliberately no DEADLOCKED_NATIVE entry.
 */

typedef struct ClassTextureFrame {
    u8 remap[0x10];
    u8 descriptors[0xF0];
    u64 packet[12];
} ClassTextureFrame;

extern u8 D_0026B590[];
extern volatile s32 D_0028E7A0 __attribute__((section(".data")));
extern u8 D_0028E7A4[];
extern s32 D_00222D18,D_00222D20,D_0021DA34,D_0021DA30;
extern s32 D_001DFD1C __attribute__((section(".data")));
extern u32 D_0026E030[],D_0026E830[];
extern u8 D_002699A0[],D_0026EFA0[],D_0021EB00[];
extern u64 D_001DFCA8[],D_001DFCC0[],D_001DFCD8[];

void func_L01_00633558(s32);
void func_L01_0055D2A8(void *,u32,s32);
void func_L01_00575180(u8 *,s32,s32,s32);
s32 func_00138E08(u8 *,s32,s32,s32,s32,s32,s32,s32);
s32 func_00126E20(s32);
void func_00139130(u8 *,u8 *);
void func_00131A08(s32,s32);
void func_L01_00576EF8(u8 *,u8 *,u8 *,s32);

/* PLZCW counts redundant sign bits after the most significant bit. Widths
 * in the real armor PIFs are positive powers of two, but retain the zero
 * and negative word behavior as well instead of substituting a log call. */
static __inline__ s32 class_texture_width_exponent(u32 width) {
    u32 bits,mask;
    s32 redundant=0;
    bits=width ^ ((width & 0x80000000U) ? 0xFFFFFFFFU : 0);
    for (mask=0x40000000U;mask!=0 && (bits & mask)==0;mask>>=1)
        redundant++;
    return 30-redundant;
}

void func_L01_0057A658(s32 objectClass,u8 *definition,u8 *pifArchive,
                      s32 specialType,s32 track) {
    ClassTextureFrame frame;
    s32 firstTexture,i,count;
    u8 *descriptor,*pif;
    if (D_0026B590[objectClass]!=0xFF) return;
    if (track!=0) {
        s32 classes,textures,palette;
        if (D_0028E7A0>=0x28) return;
        func_L01_00633558(1);
        classes=D_00222D18;
        textures=D_00222D20;
        palette=D_0021DA34;
        *(volatile s32 *)(D_0028E7A4+(u32)D_0028E7A0*4)=objectClass;
        *(volatile s32 *)(D_0028E7A4+0xA0+(u32)D_0028E7A0*4)=classes;
        *(volatile s32 *)(D_0028E7A4+0x140+(u32)D_0028E7A0*4)=textures;
        *(volatile s32 *)(D_0028E7A4+0x1E0+(u32)D_0028E7A0*4)=palette;
        *(volatile s32 *)(D_0028E7A4+0x280+(u32)D_0028E7A0*4)=0;
        D_0028E7A0++;
    }
    firstTexture=D_00222D20;
    func_L01_0055D2A8(frame.remap,0xFFFFFFFFU,0x10);
    if (pifArchive!=NULL) {
        count=*(s32 *)pifArchive;
        descriptor=frame.descriptors;
        for (i=0;i<count;i++) {
            s32 texture,mipCount;
            s32 palette,relative,bytes;
            u32 pixels,product;
            pif=pifArchive+*(u32 *)(pifArchive+4+(u32)i*4);
            texture=D_00222D20;
            frame.remap[i]=(u8)D_00222D20;
            pixels=(u32)(pif+0x420);
            D_0026E030[texture]=pixels;
            D_0026E030[texture]=pixels+
                ((u32)class_texture_width_exponent(*(u32 *)(pif+8))<<28);
            func_L01_00575180(pif+0x420,*(s32 *)(pif+0x10),
                             *(s32 *)(pif+8),*(s32 *)(pif+0xC));
            if (*(s32 *)(pif+0x1C)>=2) {
                product=*(u32 *)(pif+8) * *(u32 *)(pif+0xC);
                func_L01_00575180(pif+0x420+product,*(s32 *)(pif+0x10),
                                 *(s32 *)(pif+8)>>1,*(s32 *)(pif+0xC)>>1);
            }
            texture=D_00222D20;
            if (texture<0x100) D_00222D20=(s32)((u32)texture+1);
            mipCount=(s16)*(u16 *)(pif+0x1C);
            *(u16 *)(descriptor+4)=*(u16 *)(pif+8);
            *(u16 *)(descriptor+6)=*(u16 *)(pif+0xC);
            *(u16 *)(descriptor+8)=*(u16 *)(pif+0x1C);
            if (mipCount>=4) mipCount=3;
            *(u16 *)(descriptor+8)=(u16)mipCount;
            func_00138E08((u8 *)frame.packet,(s16)(D_0021DA34>>8),
                         1,0,0,0,16,16);
            func_00126E20(0);
            func_00139130((u8 *)frame.packet,pif+0x20);
            func_00131A08(0,0);
            palette=D_0021DA34;
            relative=(s32)((u32)palette-(u32)D_001DFD1C);
            D_0021DA34=(s32)((u32)palette+0x400);
            *(s16 *)(descriptor+0xA)=(s16)(relative>>8);
            if (*(s32 *)(pif+0x1C)>=3) {
                func_00138E08((u8 *)frame.packet,(s16)(D_0021DA34>>8),
                             1,0x13,0,0,(s16)(*(s32 *)(pif+8)>>2),
                             (s16)(*(s32 *)(pif+0xC)>>2));
                func_00126E20(0);
                product=*(u32 *)(pif+8) * *(u32 *)(pif+0xC);
                bytes=(s32)(product*5U)>>2;
                func_00139130((u8 *)frame.packet,
                             pif+(u32)bytes+0x420);
                func_00131A08(0,0);
                palette=D_0021DA34;
                relative=(s32)((u32)palette-(u32)D_001DFD1C);
                product=*(u32 *)(pif+8) * *(u32 *)(pif+0xC);
                *(u16 *)(descriptor+0xE)=0;
                *(s16 *)(descriptor+0xC)=(s16)(relative>>8);
                bytes=(s32)product>>4;
                if (bytes<=0xFF) bytes=0x100;
                D_0021DA34=(s32)((u32)palette+(u32)bytes);
            }
            descriptor+=0x10;
        }
        D_0021DA30=D_0021DA34;
    } else if (specialType!=0) {
        u64 *source;
        s32 copied,j;
        source=NULL;
        copied=0;
        if (specialType==1) { source=D_001DFCA8;copied=2; }
        else if (specialType==2) { source=D_001DFCC0;copied=2; }
        else if (specialType==3) { source=D_001DFCD8;copied=3; }
        for (j=0;j<copied;j++) *(u64 *)(D_002699A0+j*0x18)=source[j];
        for (j=0;j<4;j++) {
            *(u64 *)(D_002699A0+j*0x18+8)=((u64)0xFFA0<<32)|0xE0;
            *(u64 *)(D_002699A0+j*0x18+0x10)=
                ((u64)0x8000<<39)|((u64)0x8000<<19)|0x4000;
        }
        func_L01_00576EF8(definition,NULL,D_0021EB00,objectClass);
        D_0026E830[D_00222D18-1]=0xFFFC0000U;
        func_L01_0055D2A8(D_0026EFA0+(u32)D_00222D18*16,
                         0xFFFFFFFFU,0x10);
        return;
    }
    func_L01_00576EF8(definition,
        (u8 *)((u32)frame.descriptors-((u32)firstTexture<<4)),
        frame.remap,objectClass);
}
