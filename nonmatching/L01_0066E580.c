/* asflags: --float-immediates */
#include "common.h"

/* Complete L01 0066E580..0066E720, including both collision-volume updates.
 * Requires a real actor at record.2EF0 and the installed VU programs/model
 * and spatial structures required by0058DF58. Actor pointers are reloaded
 * across each genuine callee. No executable native success path is exposed.
 * If actor.98 is nonnull and record.3A0 is zero, the two volume centers are
 * produced by0068A178(record,out,in,f12) and committed by00587188(actor,
 * index,center,f12 radius,f13 height). Their VU bodies remain dependencies.
 * Nonmatching: the compiled function is 436 bytes versus 416 retail bytes. */
#ifndef DEADLOCKED_NATIVE
typedef unsigned int RecordTransformQuad __attribute__((mode(TI)));
typedef struct RecordTransformVector { f32 x,y,z,w; }
    __attribute__((aligned(16))) RecordTransformVector;

void func_L01_0066A678(u8 *);
void func_L01_0066A6F8(u8 *);
void *func_L01_0055D3B8(void *,const void *,s32);
void func_L01_0058DF58(u8 *);
void func_L01_0068A178(u8 *,RecordTransformVector *,RecordTransformVector *,f32);
void func_L01_00587188(u8 *,s32,RecordTransformVector *,f32,f32);
s32 func_L01_00673A00(u8 *);

#define RWORD(p,o) (*(volatile u32 *)((p)+(o)))
#define RBYTE(p,o) (*(volatile u8 *)((p)+(o)))
#define RFLOAT(p,o) (*(volatile f32 *)((p)+(o)))
#define RACTOR() ((u8 *)RWORD(record,0x2EF0))
#define RQUAD(p) (*(RecordTransformQuad *)(p))

/* COP1 MAX.S is retained rather than substituting host NaN/zero policy. */
#define RECORD_VOLUME_MAX(result,height) do { \
    f32 minimum=0.05f; \
    __asm__("max.s %0,%1,%2" : "=f"(result) : "f"(height),"f"(minimum)); \
} while(0)

void func_L01_0066E580(u8 *record)
{
    u8 *actor,*definition;
    RecordTransformVector center;
    f32 lower,radius,height;
    RBYTE(RACTOR(),0x2C)=0xFF;
    actor=RACTOR();RQUAD(actor+0x10)=RQUAD(record+0xA0);
    func_L01_0066A678(record);
    actor=RACTOR();RQUAD(actor+0xF0)=RQUAD(record+0xB0);
    func_L01_0066A6F8(record);
    actor=RACTOR();func_L01_0055D3B8(actor+0xC0,record+0x20,0x30);
    actor=RACTOR();RBYTE(actor,0x55)=RBYTE(actor,0x55)&0x7F;
    actor=RACTOR();RBYTE(actor,0x55)=RBYTE(actor,0x55)&0xDF;
    func_L01_0058DF58(RACTOR());
    actor=RACTOR();definition=(u8 *)RWORD(actor,0x24);
    if(definition!=NULL && RBYTE(definition,6)==0) actor=RACTOR();
    RBYTE(actor,0x66)=0x10;
    if(*(volatile s16 *)(record+0x3A0)!=0) return;
    if(RWORD(RACTOR(),0x98)==0) return;
    RQUAD(&center)=RQUAD(record+0xA0);
    func_L01_0068A178(record,&center,&center,-RFLOAT(record,0x1F4));
    lower=RFLOAT(record,0x1F4);
    radius=RFLOAT(record,0x1F0);
    actor=RACTOR();
    height=radius-lower;
    radius=RFLOAT(record,0x204);
    RECORD_VOLUME_MAX(height,height);
    func_L01_00587188(actor,0,&center,radius,height);
    lower=RFLOAT(record,0x204)+0.05f;
    if(RWORD(record,0x25D4)==4) lower=lower+0.15f;
    if(func_L01_00673A00(record)!=0) lower=RFLOAT(record,0x1F4);
    RQUAD(&center)=RQUAD(record+0xA0);
    func_L01_0068A178(record,&center,&center,-lower);
    radius=RFLOAT(record,0x1F0);
    height=radius-lower;
    radius=RFLOAT(record,0x204);
    actor=RACTOR();
    RECORD_VOLUME_MAX(height,height);
    func_L01_00587188(actor,1,&center,radius,height);
}
#endif
