/* cflags: -ffast-math */
/* asflags: --float-immediates */
/* Complete 296-byte turn gateway, NONMATCHING. The nonzero2659 PS2 path
 * genuinely calls0067CEF8; native refuses it before any publication. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_direction.h"
#endif
#include "../include/dreadzone_actor_direction_internal.h"
#ifdef DEADLOCKED_NATIVE
int func_L01_0067CFD0(const DreadzoneActorInputContext *ctx,u8 *record,s32 mode,
                     f32 force,f32 damping,f32 maximum) {
    const u8 *p;s32 pressed,status;
#else
extern volatile f32 D_0021DDEC,D_0021DDF4;
void func_L01_0067CFD0(u8 *record,s32 mode,f32 force,f32 damping,f32 maximum) {
    s32 pressed;
#endif
    if(D_WORD(record,0x25CC)!=0x3F && D_WORD(record,0x1AE0)) {
        if(!record[0x265A]) {
#ifdef DEADLOCKED_NATIVE
            status=dreadzone_direction_history(ctx,0x28,2,0,D_WORD(record,0x2F00),&pressed);
            if(status) return status;
#else
            pressed=func_L01_005BD868(0x28,2,0,(u8 *)D_WORD(record,0x2F00));
#endif
            if(!pressed) goto rotate;
        }
        if(D_WORD(record,0x2318)!=1) {
            f32 first,second;
#ifdef DEADLOCKED_NATIVE
            p=dreadzone_actor_input_read(ctx,0x0021DDEC,4);
            if(!p) return DREADZONE_DIRECTION_NEEDS_MEMORY;
            first=dreadzone_direction_float(p);
            p=dreadzone_actor_input_read(ctx,0x0021DDF4,4);
            if(!p) return DREADZONE_DIRECTION_NEEDS_MEMORY;
            second=dreadzone_direction_float(p);
            if(!dreadzone_direction_valid_float(first,16.f) ||
               !dreadzone_direction_valid_float(second,16.f) ||
               !dreadzone_direction_valid_float(D_FLOAT(record,0x25C4),16.f))
                return DREADZONE_DIRECTION_UNSUPPORTED_FLOAT;
#else
            first=D_0021DDEC;second=D_0021DDF4;
#endif
            first=first*0.20000000298023223876953125f;
            second=second*52.3600006103515625f;
            first=first*D_FLOAT(record,0x25C4);second=second*D_FLOAT(record,0x25C4);
            force=D_MAX(force,first);maximum=D_MAX(maximum,second);
        }
    }
rotate:
    if(record[0x2659]) {
#ifdef DEADLOCKED_NATIVE
        return DREADZONE_DIRECTION_UNSUPPORTED_MODE;
#else
        func_L01_0067CEF8(record,mode,force,damping,maximum);
#endif
    } else {
#ifdef DEADLOCKED_NATIVE
        f32 error=dreadzone_direction_rotation_bytes(record+0xB8,record+0x17A4,mode,
#else
        f32 error=func_L01_005952B8((f32 *)(record+0xB8),(f32 *)(record+0x17A4),mode,
#endif
                                   D_FLOAT(record,0x17A0),force,damping,maximum);
        D_STORE_FLOAT(record,0x17A8,error);
    }
#ifdef DEADLOCKED_NATIVE
    return DREADZONE_DIRECTION_APPROXIMATE;
#endif
}
