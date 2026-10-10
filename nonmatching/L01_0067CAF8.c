/* cflags: -ffast-math */
/* asflags: --float-immediates */
/* Complete196-byte original plus four-byte padding. Direction runs first;
 * the selected globals and actor fields are then reloaded in original order.
 * The mapped speed coefficient is read, not supplied as a replacement target. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_speed_target.h"
#endif
#include "../include/dreadzone_actor_direction_internal.h"
#ifdef DEADLOCKED_NATIVE
#define T_GLOBAL(name,x) do { status=dreadzone_speed_target_read(ctx,0x##name,&(x)); if(status<0) return status; } while(0)
#define T_COEFFICIENT(x) T_GLOBAL(003B01A0,x)
#define T_SCALAR(p,o,x) do { status=dreadzone_speed_target_scalar(D_FLOAT(p,o),&(x)); if(status<0) return status; } while(0)
#define T_MUL(x,a,b) do { status=dreadzone_speed_target_product((a),(b),&(x)); if(status<0) return status; } while(0)
int func_L01_0067CAF8(const DreadzoneActorInputContext *ctx) {
    u8 *record=ctx->record;int status;
    f32 value,first,coefficient,scale,gain,modifier;
    status=dreadzone_actor_direction_0067C5F8(ctx,0,1.f);
    if(status<0) return status;
#else
extern volatile f32 D_003B01A0[],D_0021DDF4;
void func_L01_0067C5F8(u8 *,s32,f32);
#define T_GLOBAL(name,x) ((x)=D_##name)
#define T_COEFFICIENT(x) ((x)=D_003B01A0[0])
#define T_SCALAR(p,o,x) ((x)=D_FLOAT(p,o))
#define T_MUL(x,a,b) ((x)=(a)*(b))
void func_L01_0067CAF8(u8 *record) {
    f32 value,first,coefficient,scale,gain,modifier;
    func_L01_0067C5F8(record,0,1.f);
#endif
    if(D_FLOAT(record,0x1780)>0.f) {
        T_GLOBAL(0021DDF4,first);
        T_COEFFICIENT(coefficient);
        T_SCALAR(record,0x25C4,scale);
        T_MUL(value,coefficient,first);
        T_SCALAR(record,0x2E08,gain);
        T_SCALAR(record,0x2FC0,modifier);
        T_MUL(value,value,scale);
        T_MUL(value,value,gain);
        T_MUL(value,value,modifier);
        D_STORE_FLOAT(record,0x1780,value);
    }
    if(D_WORD(record,0x25CC)==0x72) {
        value=D_FLOAT(record,0x1780);
        T_MUL(value,value,0.800000011920928955078125f);
        T_SCALAR(record,0x25C4,scale);
        D_STORE_FLOAT(record,0x1780,value);
        T_GLOBAL(0021DDF4,first);
        T_MUL(first,first,2.5f);
        T_MUL(first,first,scale);
        value=D_MAX(value,first);
        D_STORE_FLOAT(record,0x1780,value);
    }
#ifdef DEADLOCKED_NATIVE
    return DREADZONE_DIRECTION_APPROXIMATE;
#endif
}
