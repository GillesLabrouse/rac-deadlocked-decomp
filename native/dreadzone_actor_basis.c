#include "dreadzone_actor_basis.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

static int valid(float value) {
    return isfinite(value) && (value==0.0f || fabsf(value)>=FLT_MIN);
}
static int product(float a,float b,float *out) {
    volatile float value=a*b;*out=value;return valid(value);
}
static int sum(float a,float b,float *out) {
    volatile float value=a+b;*out=value;return valid(value);
}
static float subtract_from_zero(float value) {
    /* Actual SUBy reads VF0's +0, rather than negating a sign bit. */
    volatile float result=0.0f-value;
    return result;
}

/* Actual B10: one MULAx, one MADDAy, one MADDz for each output column.
 * Every product and ACC addition remains separate; no host matrix library.
 */
static int compose(float basis[4][4],float axes[3][4]) {
    float output[3][4],acc,mul;
    u32 column,lane;
    for(column=0;column<3;column++)
        for(lane=0;lane<4;lane++) {
            if(!product(axes[0][lane],basis[column][0],&acc) ||
                !product(axes[1][lane],basis[column][1],&mul) ||
                !sum(acc,mul,&acc) ||
                !product(axes[2][lane],basis[column][2],&mul) ||
                !sum(acc,mul,&output[column][lane])) return 0;
        }
    memcpy(basis,output,sizeof(output));
    return 1;
}

int dreadzone_actor_basis(const float angles_xyz[3],float output_quads[16]) {
    float input[3],basis[4][4]={{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
    float pair[2],axes[3][4];
    float pi;
    u32 i,pi_bits=0x40490FDBU;
    if(angles_xyz==NULL || output_quads==NULL ||
        (uintptr_t)angles_xyz>UINTPTR_MAX-sizeof(input) ||
        (uintptr_t)output_quads>UINTPTR_MAX-sizeof(basis))
        return DREADZONE_BASIS_INVALID;
    memcpy(input,angles_xyz,sizeof(input));memcpy(&pi,&pi_bits,4);
    for(i=0;i<3;i++)
        if(!valid(input[i]) || input[i]<-pi || input[i]>pi)
            return DREADZONE_BASIS_UNSUPPORTED;
    if(input[0]!=0.0f) {
        if(dreadzone_angle_pair(input[0],pair)!=DREADZONE_ANGLE_APPROXIMATE)
            return DREADZONE_BASIS_UNSUPPORTED;
        /* C40 returns [cos,sin]; C60's complete adapter returns [sin,cos]. */
        basis[1][1]=pair[1];basis[1][2]=pair[0];
        basis[2][2]=pair[1];basis[2][1]=subtract_from_zero(pair[0]);
    }
    if(input[1]!=0.0f) {
        if(dreadzone_angle_pair(input[1],pair)!=DREADZONE_ANGLE_APPROXIMATE)
            return DREADZONE_BASIS_UNSUPPORTED;
        memset(axes,0,sizeof(axes));
        axes[0][0]=pair[1];axes[0][2]=subtract_from_zero(pair[0]);
        axes[1][1]=1.0f;axes[2][0]=pair[0];axes[2][2]=pair[1];
        if(!compose(basis,axes)) return DREADZONE_BASIS_UNSUPPORTED;
    }
    if(input[2]!=0.0f) {
        if(dreadzone_angle_pair(input[2],pair)!=DREADZONE_ANGLE_APPROXIMATE)
            return DREADZONE_BASIS_UNSUPPORTED;
        memset(axes,0,sizeof(axes));
        axes[0][0]=pair[1];axes[0][1]=pair[0];
        axes[1][0]=subtract_from_zero(pair[0]);axes[1][1]=pair[1];axes[2][2]=1.0f;
        if(!compose(basis,axes)) return DREADZONE_BASIS_UNSUPPORTED;
    }
    memcpy(output_quads,basis,sizeof(basis));
    return DREADZONE_BASIS_APPROXIMATE;
}
