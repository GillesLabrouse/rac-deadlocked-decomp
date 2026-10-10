/* cflags: -ffast-math */
/* asflags: --float-immediates */
/* Complete224-byte velocity producer. A caller angle<=pi overrides recordB8;
 * the original large sentinel selects recordB8. Native domain is2659==0.
 * Nonzero PS2 mode executes its real C60 and record20 matrix path. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_velocity.h"
#endif
#include "../include/dreadzone_actor_velocity_internal.h"
#ifndef DEADLOCKED_NATIVE
extern u8 D_002239D0[];
#endif
#ifdef DEADLOCKED_NATIVE
int dreadzone_velocity_0067D3A0(u8 *record,f32 caller_angle) {
    VelocityVector result;f32 heading=V_FLOAT(record,0xB8),speed;
    int status;
    if(caller_angle<=3.1415927410125732421875f) heading=caller_angle;
    speed=V_FLOAT(record,0x1784);
    if(record[0x2659]) return DREADZONE_VELOCITY_UNSUPPORTED_MODE;
    status=dreadzone_velocity_00652978(speed,heading,V_FLOAT(record,0x2D4),&result);
    if(status<0) return status;
    memcpy(record+0x130,&result,16);
    return status;
}
#else
void func_L01_0067D3A0(u8 *record,f32 caller_angle) {
    VelocityLanes result;f32 heading=V_FLOAT(record,0xB8),speed;
    if(caller_angle<=3.1415927410125732421875f) heading=caller_angle;
    speed=V_FLOAT(record,0x1784);
    if(record[0x2659]) {
        VelocityLanes local;f32 sine,cosine;
        V_ANGLE_PAIR(0.f,sine,cosine);
        local.v[0]=cosine*speed;local.v[1]=sine*speed;
        local.v[2]=0.f;local.v[3]=0.f;
        /* Source vector must remain in vf2 while matrix columns overwrite3. */
        __asm__ volatile("lqc2 $vf2,0(%2)\n\tlqc2 $vf1,0x20(%0)\n\t"
            "lqc2 $vf3,0x30(%0)\n\tlqc2 $vf4,0x40(%0)\n\t"
            "vmulax.xyzw ACC,$vf1,$vf2x\n\tvmadday.xyzw ACC,$vf3,$vf2y\n\t"
            "lqc2 $vf1,0x50(%0)\n\tvmaddaz.xyzw ACC,$vf4,$vf2z\n\t"
            "vmaddw.xyzw $vf1,$vf1,$vf2w\n\tsqc2 $vf1,0(%1)"
            : : "r"(record),"r"(&result),"r"(&local) : "memory");
    } else result.quad=func_L01_00652978(D_002239D0,speed,heading,V_FLOAT(record,0x2D4));
    *(VelocityQuad *)(record+0x130)=result.quad;
}
#endif
