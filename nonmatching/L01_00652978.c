/* cflags: -ffast-math */
/* Complete original132 bytes plus four-byte padding; returns raw quad in v0.
 * a0 is unused. Calls actual C60 first for f14, then for f13. PROT3W selects
 * the returned vf15.y lane. Native uses the recovered ordered polynomial;
 * no host sin/cos replacement or captured microprogram result is used. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_velocity.h"
#include "../native/dreadzone_angle_pair.h"
#endif
#include "../include/dreadzone_actor_velocity_internal.h"
#ifdef DEADLOCKED_NATIVE
int dreadzone_velocity_00652978(f32 speed,f32 heading,f32 pitch,VelocityVector *out) {
    f32 p[2],h[2];volatile f32 x,y,z;
    if(dreadzone_angle_pair(pitch,p)!=DREADZONE_ANGLE_APPROXIMATE ||
       dreadzone_angle_pair(heading,h)!=DREADZONE_ANGLE_APPROXIMATE)
        return DREADZONE_VELOCITY_UNSUPPORTED_FLOAT;
    x=h[0]*speed;y=h[1]*speed;z=p[0]*speed;
    x=x*p[1];y=y*p[1];
    out->v[0]=y;out->v[1]=x;out->v[2]=z;out->v[3]=0.f;
    return DREADZONE_VELOCITY_APPROXIMATE;
}
#else
VelocityQuad func_L01_00652978(const u8 *unused,f32 speed,f32 heading,f32 pitch) {
    VelocityLanes result;f32 sp,cp,sh,ch,x,y,z;
    (void)unused;
    V_ANGLE_PAIR(pitch,sp,cp);
    V_ANGLE_PAIR(heading,sh,ch);
    x=sh*speed;y=ch*speed;z=sp*speed;
    x=x*cp;y=y*cp;
    result.v[0]=y;result.v[1]=x;result.v[2]=z;result.v[3]=0.f;
    return result.quad;
}
#endif
