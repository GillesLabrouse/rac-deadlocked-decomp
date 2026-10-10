/* Complete 144-byte L01 actor input bridge, including four-byte padding.
 * NONMATCHING; baseline SN compiler flags. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_input_bridge.h"
#include <math.h>
int func_L01_006A7D48(const DreadzoneActorInputContext *ctx,u8 *record) {
    const u8 *p;u8 *write;
    u32 input,bits;
    f32 magnitude,x,y,angle;
    int status=func_L01_0067BD58(ctx,record);
    if(status) return status;
    p=dreadzone_actor_input_read(ctx,0x0021E8F4,4);
    if(!p) return DREADZONE_ACTOR_INPUT_NEEDS_MEMORY;
    if(dreadzone_actor_input_word(p)) {
        write=dreadzone_actor_input_write(ctx,0x0021E8F4,4);
        if(!write) return DREADZONE_ACTOR_INPUT_NEEDS_MEMORY;
        dreadzone_actor_input_store32(write,0);
        return DREADZONE_ACTOR_INPUT_COMPLETED;
    }
    input=dreadzone_actor_input_word(record+0x2F00);
    if(input&3 || input>0xFFFFFFFFU-0x10C) return DREADZONE_ACTOR_INPUT_NEEDS_MEMORY;
    p=dreadzone_actor_input_read(ctx,input+0x108,4);
    if(!p) return DREADZONE_ACTOR_INPUT_NEEDS_MEMORY;
    dreadzone_actor_input_store32(record+0x120,dreadzone_actor_input_word(p));
    p=dreadzone_actor_input_read(ctx,input+0x10C,4);
    if(!p) return DREADZONE_ACTOR_INPUT_NEEDS_MEMORY;
    dreadzone_actor_input_store32(record+0x124,dreadzone_actor_input_word(p)^0x80000000U);
    magnitude=func_L01_0067C570(record,(const f32 *)(record+0x120));
    if(!isfinite(magnitude)) return DREADZONE_ACTOR_INPUT_UNSUPPORTED_FLOAT;
    {union {f32 f;u32 u;} value;value.f=magnitude;dreadzone_actor_input_store32(record+0x2E38,value.u);}
    magnitude=func_L01_0067C570(record,(const f32 *)(record+0x120));
    if(!isfinite(magnitude)) return DREADZONE_ACTOR_INPUT_UNSUPPORTED_FLOAT;
    {union {f32 f;u32 u;} value;value.f=magnitude;dreadzone_actor_input_store32(record+0x2E08,value.u);}
    bits=dreadzone_actor_input_word(record+0x120);{union {f32 f;u32 u;} value;value.u=bits;x=value.f;}
    bits=dreadzone_actor_input_word(record+0x124);{union {f32 f;u32 u;} value;value.u=bits;y=value.f;}
    angle=func_L01_0055D708(x,y);
    if(!isfinite(angle)) return DREADZONE_ACTOR_INPUT_UNSUPPORTED_FLOAT;
    {union {f32 f;u32 u;} value;value.f=angle;dreadzone_actor_input_store32(record+0x2E0C,value.u);}
    return DREADZONE_ACTOR_INPUT_APPROXIMATE_VU;
}
#else
void func_L01_0067BD58(u8 *);
f32 func_L01_0067C570(const u8 *,const f32 *);
f32 func_L01_0055D708(f32,f32);
extern volatile s32 D_0021E8F4;
void func_L01_006A7D48(u8 *record) {
    u8 *input;
    func_L01_0067BD58(record);
    if(D_0021E8F4) {D_0021E8F4=0;return;}
    input=*(u8 **)(record+0x2F00);
    *(f32 *)(record+0x120)=*(f32 *)(input+0x108);
    *(f32 *)(record+0x124)=-*(f32 *)(input+0x10C);
    *(f32 *)(record+0x2E38)=func_L01_0067C570(record,(f32 *)(record+0x120));
    *(f32 *)(record+0x2E08)=func_L01_0067C570(record,(f32 *)(record+0x120));
    *(f32 *)(record+0x2E0C)=func_L01_0055D708(*(f32 *)(record+0x120),*(f32 *)(record+0x124));
}
#endif
