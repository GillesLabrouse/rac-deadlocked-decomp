/* cflags: -ffast-math */
/* asflags: --float-immediates */
/* Complete592-byte original scalar gateway. The current actor/input state
 * chooses the genuine force/damping/maximum arguments passed to CFD0(mode0).
 * Native executes that real gateway and retains its bounded float domain. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_speed_target.h"
#endif
#include "../include/dreadzone_actor_direction_internal.h"
#ifdef DEADLOCKED_NATIVE
#define T_GLOBAL(name,x) do { status=dreadzone_speed_target_read(ctx,0x##name,&(x)); if(status<0) return status; } while(0)
#define T_SCALAR(p,o,x) do { status=dreadzone_speed_target_scalar(D_FLOAT(p,o),&(x)); if(status<0) return status; } while(0)
#define T_MUL(x,a,b) do { status=dreadzone_speed_target_product((a),(b),&(x)); if(status<0) return status; } while(0)
int func_L01_0067D100(const DreadzoneActorInputContext *ctx) {
    u8 *record=ctx->record;int status;
#else
extern volatile f32 D_0021DDEC,D_0021DDF4;
void func_L01_0067CFD0(u8 *,s32,f32,f32,f32);
#define T_GLOBAL(name,x) ((x)=D_##name)
#define T_SCALAR(p,o,x) ((x)=D_FLOAT(p,o))
#define T_MUL(x,a,b) ((x)=(a)*(b))
void func_L01_0067D100(u8 *record) {
#endif
    f32 gain,scale,first,second,force,damping,maximum,a,b;
    s32 selected;
    T_SCALAR(record,0x2E08,gain);
    T_GLOBAL(0021DDEC,first);
    T_SCALAR(record,0x25C4,scale);
    if(gain==1.f) {
        T_MUL(force,first,0.02199999988079071044921875f);
        T_MUL(force,force,scale);
    } else {
        T_MUL(a,first,0.01999999955296516418457031f);
        T_MUL(b,first,0.02199999988079071044921875f);
        T_MUL(force,first,0.009999999776482582092285156f);
        T_MUL(a,a,scale);T_MUL(b,b,scale);
        T_MUL(force,force,scale);
        T_MUL(a,a,gain);a=D_MIN(a,b);force=D_MAX(a,force);
    }
    T_SCALAR(record,0x2E08,gain);
    T_GLOBAL(0021DDF4,second);
    if(gain==1.f) {
        T_MUL(maximum,second,5.58505344390869140625f);
        T_MUL(maximum,maximum,scale);
    } else {
        T_MUL(a,second,3.8397243022918701171875f);
        T_MUL(b,second,5.58505344390869140625f);
        T_MUL(maximum,second,1.91986215114593505859375f);
        T_MUL(a,a,scale);T_MUL(b,b,scale);
        T_MUL(maximum,maximum,scale);
        T_MUL(a,a,gain);a=D_MIN(a,b);maximum=D_MAX(a,maximum);
    }
    T_MUL(damping,first,0.2700000107288360595703125f);
    T_MUL(damping,damping,scale);
    if(record[0x1C8F]) {
        T_MUL(force,first,0.03500000014901161193847656f);
        T_MUL(maximum,second,11.69370555877685546875f);
        T_MUL(force,force,scale);T_MUL(maximum,maximum,scale);
    }
    selected=D_WORD(record,0x2314)==2?D_WORD(record,0x2318):-1;
    if(selected==2 && record[0x265A] && !record[0x1C8F]) {
        if(gain>0.949999988079071044921875f) {
            T_MUL(force,first,0.01700000092387199401855469f);
            T_MUL(damping,first,0.180000007152557373046875f);
            T_MUL(maximum,second,8.72664642333984375f);
            T_MUL(force,force,scale);T_MUL(damping,damping,scale);
            T_MUL(maximum,maximum,scale);
        } else {
            T_MUL(force,first,0.008999999612569808959960938f);
            T_MUL(maximum,second,4.7123889923095703125f);
            T_MUL(damping,first,0.1500000059604644775390625f);
            T_MUL(force,force,scale);T_MUL(maximum,maximum,scale);
            T_MUL(damping,damping,scale);
            T_MUL(force,force,gain);T_MUL(maximum,maximum,gain);
        }
    }
#ifdef DEADLOCKED_NATIVE
    return dreadzone_actor_turn_0067CFD0(ctx,0,force,damping,maximum);
#else
    func_L01_0067CFD0(record,0,force,damping,maximum);
#endif
}
