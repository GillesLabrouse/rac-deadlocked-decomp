#include "dreadzone_angle_pair.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#if FLT_RADIX != 2 || FLT_MANT_DIG != 24 || FLT_MAX_EXP != 128
#error Angle pair requires IEEE binary32 host floats
#endif
typedef char angle_float_size[sizeof(float)==4 ? 1 : -1];

static float constant(u32 bits) {
    float value;
    memcpy(&value,&bits,4);
    return value;
}
static int valid(float value) {
    return isfinite(value) && (value==0.0f || fabsf(value)>=FLT_MIN);
}
/* Keep every separately rounded operation, then flush a subnormal result to
 * its signed zero within the explicitly approximate bounded model. */
static float flush_result(float value) {
    u32 bits;memcpy(&bits,&value,4);
    if((bits&0x7FFFFFFFU)!=0 && (bits&0x7FFFFFFFU)<0x00800000U) {
        bits&=0x80000000U;memcpy(&value,&bits,4);
    }
    return value;
}
static int add(float a,float b,float *out) {
    volatile float result=a+b;
    *out=flush_result(result);
    return valid(*out);
}
static int multiply(float a,float b,float *out) {
    volatile float result=a*b;
    *out=flush_result(result);
    return valid(*out);
}

#define ADD(a,b,out) do { if(!add((a),(b),&(out))) return 0; } while(0)
#define MUL(a,b,out) do { if(!multiply((a),(b),&(out))) return 0; } while(0)
static int polynomial(float angle,float *output) {
    float pi=constant(0x40490FDBU),upper,lower,a,a2,a4,c7a,c9a,c3a;
    float acc,product,tail;
    ADD(pi,-angle,upper);
    ADD(-pi,-angle,lower);
    a=angle<upper ? angle : upper;
    a=a>lower ? a : lower;
    MUL(a,a,a2);
    MUL(a,constant(0xB94FB21FU),c7a);
    MUL(a,constant(0x362E9C14U),c9a);
    MUL(a,constant(0xBE2AAAA4U),c3a);
    MUL(a2,a2,a4);
    MUL(a,constant(0x3C08873EU),acc);
    MUL(c7a,a2,product);
    ADD(acc,product,acc);
    MUL(c9a,a4,product);
    ADD(acc,product,tail);
    /* C60's taken IBNE executes MULAw ACC=a in its real delay pair. */
    acc=a;
    MUL(c3a,a2,product);
    ADD(acc,product,acc);
    MUL(tail,a4,product);
    ADD(acc,product,*output);
    return 1;
}

int dreadzone_angle_pair(float angle,float output_xy[2]) {
    float result[2],shifted,pi=constant(0x40490FDBU);
    if(output_xy==NULL || (uintptr_t)output_xy>UINTPTR_MAX-sizeof(result))
        return DREADZONE_ANGLE_INVALID;
    if(!valid(angle) || (angle!=0.0f && fabsf(angle)<0.0000000000009094947017729282379150390625f) || angle < -pi || angle > pi)
        return DREADZONE_ANGLE_UNSUPPORTED;
    if(!add(angle,constant(0x3FC90FDBU),&shifted) ||
        !polynomial(angle,&result[0]) || !polynomial(shifted,&result[1]))
        return DREADZONE_ANGLE_UNSUPPORTED;
    if(!valid(result[0]) || !valid(result[1])) return DREADZONE_ANGLE_UNSUPPORTED;
    memcpy(output_xy,result,sizeof(result));
    return DREADZONE_ANGLE_APPROXIMATE;
}
