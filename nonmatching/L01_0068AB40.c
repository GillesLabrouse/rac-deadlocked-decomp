/* Complete104-byte original. Mode0 returns inputZ as raw transport. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_velocity.h"
#endif
#include "../include/dreadzone_actor_velocity_internal.h"
#ifdef DEADLOCKED_NATIVE
f32 func_L01_0068AB40(const u8 *record,const VelocityVector *input) {
    if(record[0x2659]) return NAN;
    return V_FLOAT(input,8);
}
#else
f32 func_L01_0068AB40(const u8 *record,const VelocityVector *input) {
    if(!record[0x2659]) return input->v[2];
    if(record[0x2659]<3) {
        VelocityLanes result;
        __asm__ volatile("lqc2 $vf3,0(%2)\n\tlqc2 $vf1,0x60(%0)\n\t"
            "vmulax.xyzw ACC,$vf1,$vf3x\n\tlqc2 $vf2,0x70(%0)\n\t"
            "lqc2 $vf1,0x80(%0)\n\tvmadday.xyzw ACC,$vf2,$vf3y\n\t"
            "vmaddaz.xyzw ACC,$vf1,$vf3z\n\tvmaddw.xyzw $vf1,$vf0,$vf3w\n\t"
            "sqc2 $vf1,0(%1)" : : "r"(record),"r"(&result),"r"(input) : "memory");
        return result.v[2];
    }
    return 0.f;
}
#endif
