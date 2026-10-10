#include "dreadzone_general_matrices.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error General matrices require IEEE binary32 floats
#endif
typedef char general_matrices_float_width[sizeof(float)==4 ? 1 : -1];
static float gm_load(const u8 *p) {
    u32 bits=(u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
    float f;memcpy(&f,&bits,4);return f;
}
static void gm_store(u8 *p,float f) {
    u32 bits;memcpy(&bits,&f,4);
    p[0]=(u8)bits;p[1]=(u8)(bits>>8);p[2]=(u8)(bits>>16);p[3]=(u8)(bits>>24);
}
static float gm_mul(float a,float b) { volatile float f=a*b;return f; }
static float gm_add(float a,float b) { volatile float f=a+b;return f; }
static float gm_sub(float a,float b) { volatile float f=a-b;return f; }
static int gm_finite(float f) { return isfinite(f) && (f==0 || fabsf(f)>=FLT_MIN); }
static void gm_identity(u8 *p) {
    u32 col,row;for(col=0;col<4;col++)for(row=0;row<4;row++)gm_store(p+col*16+row*4,col==row ? 1.f : 0.f);
}
int dreadzone_local_pose_initialize(const DreadzoneLocalPoseInit *request) {
    DreadzoneLocalPoseInit r;
    u8 source[127*16],output[127*64];
    u32 joint,lane;
    if(!request) return DREADZONE_GENERAL_MATRICES_INVALID;
    r=*request;
    if(!r.skeleton || !r.scratch || !r.joints || r.joints>127 ||
       r.skeleton_bytes<r.joints*16 || r.scratch_bytes<0x4000 ||
       (uintptr_t)r.skeleton>UINTPTR_MAX-r.skeleton_bytes ||
       (uintptr_t)r.scratch>UINTPTR_MAX-r.scratch_bytes)
        return DREADZONE_GENERAL_MATRICES_INVALID;
    memcpy(source,r.skeleton,r.joints*16);memset(output,0,r.joints*64);
    for(joint=0;joint<r.joints;joint++) {
        u8 *record=output+joint*64;
        gm_store(record+12,1.f);
        for(lane=0;lane<3;lane++) {
            float value=gm_mul(gm_load(source+joint*16+lane*4),65536.f);
            if(!gm_finite(value)) return DREADZONE_GENERAL_MATRICES_UNSUPPORTED;
            gm_store(record+16+lane*4,268435456.f);
            gm_store(record+32+lane*4,value);
        }
        memcpy(record+44,source+joint*16+12,4);
        memcpy(record+48,record+32,16);
    }
    memcpy(r.scratch,output,r.joints*64);
    return DREADZONE_GENERAL_MATRICES_APPROXIMATE;
}
int dreadzone_general_matrices_prepare(const DreadzoneGeneralMatrices *request) {
    DreadzoneGeneralMatrices r;
    u8 work[0x4000];
    u32 joint,j,row,col,parent,inherit;
    if(!request) return DREADZONE_GENERAL_MATRICES_INVALID;
    r=*request;
    if(!r.scratch || r.bytes<sizeof(work) || (uintptr_t)r.scratch>UINTPTR_MAX-r.bytes)
        return DREADZONE_GENERAL_MATRICES_INVALID;
    if(!r.joints) return DREADZONE_GENERAL_MATRICES_APPROXIMATE;
    if(r.joints>127) return DREADZONE_GENERAL_MATRICES_UNSUPPORTED;
    memcpy(work,r.scratch,sizeof(work));gm_identity(work+0x3FC0);
    for(joint=0;joint<r.joints;joint++) {
        u8 *record=work+joint*64,*bank=work+0x2000+joint*64;
        const u8 *parent_record;
        float q[4],square[4],minus[3],plus[3],diagonal[4],scale[3],translation[3];
        float pb[4][4],basis[3][4],render[3][4],out_translation[4],reciprocal;
        parent=record[0x3C]&127;inherit=record[0x3C]&128;
        if(parent!=127 && parent>=joint) return DREADZONE_GENERAL_MATRICES_UNSUPPORTED;
        parent_record=work+0x2000+parent*64;
        for(j=0;j<4;j++) {
            q[j]=gm_load(record+j*4);
            if(!gm_finite(q[j])) return DREADZONE_GENERAL_MATRICES_UNSUPPORTED;
            square[j]=gm_mul(q[j],q[j]);
            if(!gm_finite(square[j])) return DREADZONE_GENERAL_MATRICES_UNSUPPORTED;
        }
        for(j=0;j<3;j++) {
            float cross=gm_mul(q[(j+1)%3],q[(j+2)%3]),product=gm_mul(q[j],q[3]);
            minus[j]=gm_sub(cross,product);plus[j]=gm_add(cross,product);
            scale[j]=gm_mul(gm_load(record+16+j*4),1.f/268435456.f);
            translation[j]=gm_mul(gm_load(record+32+j*4),1.f/65536.f);
            if(!gm_finite(scale[j]) || !gm_finite(translation[j])) return DREADZONE_GENERAL_MATRICES_UNSUPPORTED;
        }
        for(j=0;j<4;j++) {
            float a=gm_mul(square[0],j==0 || j==3 ? .5f : -.5f);
            a=gm_add(a,gm_mul(square[1],j==1 || j==3 ? .5f : -.5f));
            a=gm_add(a,gm_mul(square[2],j==2 || j==3 ? .5f : -.5f));
            diagonal[j]=gm_add(a,gm_mul(square[3],.5f));
        }
        if(!gm_finite(diagonal[3]) || diagonal[3]<=0) return DREADZONE_GENERAL_MATRICES_UNSUPPORTED;
        { volatile float value=1.f/diagonal[3];reciprocal=value; }
        if(!gm_finite(reciprocal)) return DREADZONE_GENERAL_MATRICES_UNSUPPORTED;
        for(col=0;col<4;col++)for(row=0;row<4;row++)pb[col][row]=gm_load(parent_record+col*16+row*4);
        for(row=0;row<4;row++) {
            float a=gm_mul(pb[1][row],minus[2]);
            a=gm_add(a,gm_mul(pb[2][row],plus[1]));a=gm_add(a,gm_mul(pb[0][row],diagonal[0]));
            basis[0][row]=gm_mul(a,reciprocal);
            a=gm_mul(pb[0][row],plus[2]);a=gm_add(a,gm_mul(pb[1][row],diagonal[1]));
            a=gm_add(a,gm_mul(pb[2][row],minus[0]));basis[1][row]=gm_mul(a,reciprocal);
            a=gm_mul(pb[0][row],minus[1]);a=gm_add(a,gm_mul(pb[1][row],plus[0]));
            a=gm_add(a,gm_mul(pb[2][row],diagonal[2]));basis[2][row]=gm_mul(a,reciprocal);
            a=gm_mul(pb[0][row],translation[0]);a=gm_add(a,gm_mul(pb[1][row],translation[1]));
            a=gm_add(a,gm_mul(pb[2][row],translation[2]));out_translation[row]=gm_add(a,gm_mul(pb[3][row],1.f));
            if(!gm_finite(out_translation[row])) return DREADZONE_GENERAL_MATRICES_UNSUPPORTED;
            for(col=0;col<3;col++) {
                render[col][row]=gm_mul(basis[col][row],scale[col]);
                if(!gm_finite(render[col][row]) || !gm_finite(basis[col][row])) return DREADZONE_GENERAL_MATRICES_UNSUPPORTED;
            }
        }
        for(col=0;col<3;col++)for(row=0;row<4;row++) {
            gm_store(bank+col*16+row*4,inherit ? render[col][row] : basis[col][row]);
            gm_store(record+col*16+row*4,render[col][row]);
        }
        for(row=0;row<4;row++) {
            gm_store(bank+48+row*4,out_translation[row]);gm_store(record+48+row*4,out_translation[row]);
        }
    }
    memcpy(r.scratch,work,sizeof(work));
    return DREADZONE_GENERAL_MATRICES_APPROXIMATE;
}
