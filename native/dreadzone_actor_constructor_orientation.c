/* Constructor C60 polynomial/ordered products and finite directions/endpoints.
 * Finite Q uses separately rounded sqrt and division. Physical VU precision,
 * flags and timing remain excluded; exceptional arithmetic refuses.
 * Output order: C/D/E basis, scaled, unit, start, end, dot scratch.
 */
#include "dreadzone_actor_constructor_internal.h"
#include "dreadzone_angle_pair.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

static int normal(float x) {
    return isfinite(x) && (x==0.f || fabsf(x)>=FLT_MIN);
}
static int mul(float a,float b,float *out) {
    volatile float x=a*b;*out=x;return normal(x);
}
static int add(float a,float b,float *out) {
    volatile float x=a+b;*out=x;return normal(x);
}
static float flip(float x) {
    uint32_t bits;memcpy(&bits,&x,4);bits^=0x80000000U;
    memcpy(&x,&bits,4);return x;
}
#define MUL(a,b,c) do {if(!mul((a),(b),&(c)))return -2;}while(0)
#define ADD(a,b,c) do {if(!add((a),(b),&(c)))return -2;}while(0)

int dreadzone_constructor_orientation(const float angles[3],const float position[4],
                                 float output[32]) {
    float a[3],p[4],pair[3][2],o[32]={0};
    float sx,cx,sy,cy,sz,cz,sxsy,cxsy,cysx,cxcy,product,other;
    float squared[4],sum,dot,scaled_q,unit_q;
    uint64_t observed;float denominator;
    unsigned i;
    if(!angles || !position || !output)return -1;
    memcpy(a,angles,sizeof(a));memcpy(p,position,sizeof(p));
    for(i=0;i<4;i++)if(!normal(p[i]) || fabsf(p[i])>8192.f)return -2;
    for(i=0;i<3;i++)
        if(dreadzone_angle_pair(a[i],pair[i])!=DREADZONE_ANGLE_APPROXIMATE)return -2;
    sx=pair[0][0];cx=pair[0][1];sy=pair[1][0];cy=pair[1][1];
    sz=pair[2][0];cz=pair[2][1];
    MUL(sx,sy,sxsy);MUL(cx,sy,cxsy);MUL(cy,sx,cysx);MUL(cx,cy,cxcy);
    MUL(cz,sxsy,product);MUL(cx,sz,other);ADD(product,flip(other),o[4]);
    MUL(cx,cz,product);MUL(sxsy,sz,other);ADD(product,other,o[5]);o[6]=cysx;
    MUL(cz,cxsy,product);MUL(sx,sz,other);ADD(product,other,o[8]);
    MUL(flip(cz),sx,product);MUL(cxsy,sz,other);ADD(product,other,o[9]);o[10]=cxcy;
    MUL(cy,cz,o[0]);MUL(cy,sz,o[1]);o[2]=flip(sy);
    for(i=0;i<4;i++)MUL(o[8+i],o[8+i],squared[i]);
    ADD(squared[0],squared[1],sum);ADD(sum,squared[2],dot);
    squared[0]=dot;memcpy(&observed,squared,sizeof(observed));
    /* Genuine branch examines both X and Y raw words after QMFC2. Q uses
     * positive finite squared length here; unsupported exceptional division
     * refuses rather than inventing a hardware exceptional result. */
    if(dot<=0.f || !normal(dot))return -2;
    denominator=sqrtf(dot);
    scaled_q=-3.f/denominator;unit_q=1.f/denominator;
    if(!normal(scaled_q)||!normal(unit_q))return -2;
    for(i=0;i<3;i++) {
        if(observed) {MUL(o[8+i],scaled_q,o[12+i]);MUL(o[8+i],unit_q,o[16+i]);}
        ADD(o[16+i],p[i],o[20+i]);ADD(o[12+i],p[i],o[24+i]);
    }
    /* W is basis E.W=+0 and is retained by each XYZ-only operation. */
    memcpy(o+28,squared,sizeof(squared));memcpy(output,o,sizeof(o));return 1;
}
