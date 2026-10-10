#include "common.h"

/* Nonmatching: complete original736 bytes, compiled1068 bytes. */
/* Complete0058DC78..0058DF58, including the out-of-line shrink/grow/trap
 * blocks beyond the splitter's1C8 extent. Cell records are four bytes:
 * u16 first32-byte block, u8 actor count, u8 capacity in blocks. The grid
 * has64x64 cells; its1536 blocks contain16 halfword actor indices each.
 * Actor.A4 is the packed inclusive rectangle[xmin,ymin,xmax,ymax]. A
 * negative rectangle is the unregistered sentinel. Actor index derives
 * from the actual pool base and256-byte actor stride, independent of class.
 *
 * The original hand-written helper ABI saves live register halves around
 * calls. C locals replace that register storage without replacing helpers
 * or their bitmap effects. There is no collision or matrix/VU computation
 * here. The QMT C2-to-vf0 operations have no effect on immutable vf0.
 * Signed saturated byte difference predicates are expressed as equivalent
 * signed-byte comparisons: saturation preserves each difference's sign.
 * Retail failure is a real TEQ trap; allocation exhaustion is unbounded.
 */
#ifndef DEADLOCKED_NATIVE
typedef struct SpatialCellPS2 { u16 first;u8 count;u8 blocks; } SpatialCellPS2;
typedef unsigned int SpatialQuadPS2 __attribute__((mode(TI)));
extern u8 *D_00222D14;
extern volatile SpatialCellPS2 D_0026FF20[];
extern u8 D_00273F20[];
void func_L01_0058DBA0(u32,s32);
s32 func_L01_0058DC00(s32);

static __inline__ s32 spatial_outside(u32 x,u32 y,u32 rectangle)
{
    return (s8)x<(s8)rectangle || (s8)y<(s8)(rectangle>>8) ||
           (s8)x>(s8)(rectangle>>16) || (s8)y>(s8)(rectangle>>24);
}

static __inline__ void spatial_copy_blocks(u32 destination,u32 source,u32 blocks)
{
    u8 *out=D_00273F20+(destination<<5);
    u8 *in=D_00273F20+(source<<5);
    do {
        SpatialQuadPS2 lo=*(SpatialQuadPS2 *)in;
        SpatialQuadPS2 hi=*(SpatialQuadPS2 *)(in+16);
        *(SpatialQuadPS2 *)out=lo;*(SpatialQuadPS2 *)(out+16)=hi;
        in+=32;out+=32;
    } while(--blocks!=0);
}

void func_L01_0058DC78(u8 *actor,u32 rectangle)
{
    /* Retail's ADDI t8,a1,0 sign-extends the low word even when a packed
     * caller supplied a zero-extended64-bit register. Keep that boundary. */
    volatile u32 inputRectangle=rectangle;
    u32 actorIndex=((u32)actor-(u32)D_00222D14)>>8;
    u32 old=*(volatile u32 *)(actor+0xA4);
    u32 x=old&255,y=*(volatile u8 *)(actor+0xA5);
    rectangle=inputRectangle;
    if((s32)old>=0) {
        for(;;) {
            if(spatial_outside(x,y,rectangle)) {
                volatile SpatialCellPS2 *cell=D_0026FF20+x+(y<<6);
                volatile u16 *scan=(volatile u16 *)(D_00273F20+((u32)cell->first<<5));
                volatile u16 *end=scan+cell->count;
                for(;;) {
                    u32 value=*scan;
                    if(scan==end) {
                        __asm__ volatile("teq $0,$0,0" : : : "memory");
                        return;
                    }
                    ++scan;
                    if(value==actorIndex) break;
                }
                scan[-1]=end[-1];end[-1]=0;
                {
                    s32 count=(s32)cell->count-1;
                    u32 blocks=cell->blocks;
                    cell->count=(u8)count;
                    if(count==0 || (blocks!=1 && (s32)(blocks*8)-4-count>0)) {
                        u32 source,destination,newBlocks;
                        func_L01_0058DBA0(cell->first,cell->blocks);
                        newBlocks=(u32)cell->blocks>>1;
                        source=(u32)(s32)(s16)cell->first;
                        cell->first=0;cell->blocks=(u8)newBlocks;
                        if(newBlocks!=0) {
                            destination=(u32)func_L01_0058DC00((s32)newBlocks);
                            cell->first=(u16)destination;
                            spatial_copy_blocks(destination,source,(u32)(s32)(s8)cell->blocks);
                        }
                    }
                }
            }
            {
                u32 endX=*(volatile u8 *)(actor+0xA6);
                u32 endY=*(volatile u8 *)(actor+0xA7);
                if(x++!=endX) continue;
                x=*(volatile u8 *)(actor+0xA4);
                if(y++!=endY) continue;
                break;
            }
        }
    }
    old=*(volatile u32 *)(actor+0xA4);
    x=rectangle&255;
    *(volatile u32 *)(actor+0xA4)=rectangle;
    y=(rectangle>>8)&255;
    if((s32)rectangle<0) return;
    for(;;) {
        if(spatial_outside(x,y,old)) {
            volatile SpatialCellPS2 *cell=D_0026FF20+x+(y<<6);
            u32 blocks=cell->blocks,count=cell->count;
            if(blocks==0 || (s32)(blocks*16)-count<=0) {
                u32 oldBlocks=cell->blocks;
                u32 newBlocks=oldBlocks*2;
                u32 source,destination;
                if(newBlocks<1) newBlocks=1;
                cell->blocks=(u8)newBlocks;
                destination=(u32)func_L01_0058DC00((s32)newBlocks);
                source=cell->first;
                cell->first=(u16)destination;
                if(oldBlocks!=0) {
                    func_L01_0058DBA0(source,(s32)oldBlocks);
                    destination=cell->first;
                    spatial_copy_blocks(destination,source,(u32)cell->blocks>>1);
                }
                count=cell->count;
            }
            {
                u32 first=cell->first;
                cell->count=(u8)(count+1);
                *(volatile u16 *)(D_00273F20+(first<<5)+(count<<1))=(u16)actorIndex;
            }
        }
        {
            u32 endX=*(volatile u8 *)(actor+0xA6);
            u32 endY=*(volatile u8 *)(actor+0xA7);
            if(x++!=endX) continue;
            x=*(volatile u8 *)(actor+0xA4);
            if(y++!=endY) continue;
            break;
        }
    }
}
#endif
