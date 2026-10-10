/* asflags: --float-immediates */
#include "common.h"

/* Complete retail 00589CE0..00589F98 (696 bytes), including secondary-layer
 * cleanup, time advancement, wrap/clamp and unlink/release. EE accumulator
 * instructions and actual helpers remain real dependencies. No DMA, events,
 * display cadence or clip selection is performed by this function. */
#ifndef DEADLOCKED_NATIVE
void func_L01_0058ACB8(u8 *);
void func_L01_0058A8D8(u8 *);
typedef union ClockBits { f32 value; s32 word; } ClockBits;
#define CW(p,o) (*(volatile u32 *)((p)+(o)))
#define CH(p,o) (*(volatile u16 *)((p)+(o)))
#define CB(p,o) (*(volatile u8 *)((p)+(o)))
#define CF(p,o) (*(volatile f32 *)((p)+(o)))
#define CLOCK_FRACTION(out,x) do { \
    f32 big=-12582912.0f,one=1.0f; \
    __asm__ volatile("suba.s %1,%2\n\tmsuba.s %1,%3\n\tmadd.s %0,%2,%3" \
        : "=f"(out) : "f"(big),"f"(x),"f"(one)); \
} while(0)
#define CLOCK_INTEGER(out,x) do { \
    ClockBits converted; \
    __asm__ volatile("cvt.w.s %0,%1" : "=f"(converted.value) : "f"(x)); \
    (out)=converted.word; \
} while(0)
void func_L01_00589CE0(u8 *actor) {
    u8 *layer,*definition,*cursor,*first,*last,*node,*next;
    u32 *link;
    ClockBits rate,weight,decay,difference;
    f32 partial,elapsedFloat,remainder,result,one=1.0f;
    s32 index,elapsed,duration;
    u8 flags;
    if(CW(actor,0x50)!=0 && ((s8)CB(actor,0x20)<0 || (s8)CB(actor,0x31)==0))
        func_L01_0058ACB8(actor);
    layer=actor+0x40;
    do {
        result=CF(layer,4);
        duration=(s16)CH(layer,0xC);
        definition=(u8 *)CW(layer,0);
        CLOCK_FRACTION(partial,result);
        rate.word=(s32)CW(layer,8);
        flags=CB(layer,0x15);
        CLOCK_INTEGER(index,result);
        if(rate.word!=0) {
            __asm__ volatile("mula.s %1,%2\n\tmadd.s %0,%3,%4"
                : "=f"(elapsedFloat) : "f"(rate.value),"f"(one),
                  "f"(partial),"f"((f32)duration));
            first=definition+0x20;
            last=first+((s32)CB(definition,0x10)-1)*4;
            cursor=first+index*4;
            flags=(flags&0xFD)|0x20;
            duration=CH(cursor,0);
            CLOCK_INTEGER(elapsed,elapsedFloat);
            CLOCK_FRACTION(remainder,elapsedFloat);
            if(rate.word>=0) {
                if(flags&0x10) {
                    while(cursor!=last && elapsed>=duration) {
                        cursor+=4;elapsed-=duration;duration=CH(cursor,0);
                    }
                    if(cursor==last) { flags|=2;elapsed=0;remainder=0; }
                } else {
                    while(elapsed>=duration) {
                        elapsed-=duration;
                        if(cursor==last) { flags|=2;cursor=first; }
                        else cursor+=4;
                        duration=CH(cursor,0);
                    }
                }
            } else {
                difference.value=remainder;
                if(difference.word<0) { elapsed--;remainder+=1.0f; }
                if(flags&0x10) {
                    while(elapsed<0 && cursor!=first) {
                        cursor-=4;elapsed+=duration;duration=CH(cursor,0);
                    }
                    if(elapsed<0) { flags|=2;elapsed=0;remainder=0; }
                } else {
                    while(elapsed<0) {
                        if(cursor==first) { flags|=2;cursor=last; }
                        else cursor-=4;
                        duration=CH(cursor,0);elapsed+=duration;
                    }
                }
            }
            result=(f32)((u32)(cursor-first)>>2)+((f32)elapsed+remainder)/(f32)duration;
            CB(layer,0x15)=flags;
            CH(layer,0xC)=(u16)duration;
            next=(u8 *)CW(layer,0x10);
            CF(layer,4)=result;
        } else next=(u8 *)CW(layer,0x10);
        layer=next;
    } while(layer!=NULL);
    link=(u32 *)(actor+0x50);
    node=(u8 *)*link;
    while(node!=NULL) {
        weight.value=CF(node,0x18);decay.value=CF(node,0x1C);
        difference.value=weight.value-decay.value;
        next=(u8 *)CW(node,0x10);
        if(decay.word>0) {
            CF(node,0x18)=difference.value;
            if(difference.word<=0) {
                *link=(u32)next;
                func_L01_0058A8D8(node);
                node=(u8 *)*link;
                continue;
            }
        } else if(decay.word!=0) {
            CF(node,0x18)=difference.value;
            difference.value-=1.0f;
            if(difference.word>=0) { CF(node,0x18)=1.0f;CF(node,0x1C)=0; }
        }
        link=(u32 *)(node+0x10);node=next;
    }
}
#endif
