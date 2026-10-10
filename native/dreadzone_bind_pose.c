#include "dreadzone_bind_pose.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#if FLT_RADIX != 2 || FLT_MANT_DIG != 24 || FLT_MAX_EXP != 128
#error Bind pose requires IEEE binary32 host floats
#endif
typedef char bind_float_size_check[sizeof(float) == 4 ? 1 : -1];
static u32 read32(const u8 *p) {
    return (u32)p[0] | (u32)p[1]<<8 | (u32)p[2]<<16 | (u32)p[3]<<24;
}
static float read_float(const u8 *p) {
    u32 bits=read32(p);float value;memcpy(&value,&bits,4);return value;
}
static void write_float(u8 *p,float value) {
    u32 bits;memcpy(&bits,&value,4);
    p[0]=(u8)bits;p[1]=(u8)(bits>>8);p[2]=(u8)(bits>>16);p[3]=(u8)(bits>>24);
}
static int valid_float(float value) {
    return isfinite(value) && (value==0.0f || fabsf(value)>=FLT_MIN);
}
static int overlap(uint64_t a,uint64_t n,uint64_t b,uint64_t m) {
    return a<b+m && b<a+n;
}
static const u8 *resolve(const DreadzoneModelView *views,u32 count,u32 address,u32 bytes) {
    u32 i;
    for(i=0;i<count;i++)
        if(address>=views[i].guest_base &&
            (uint64_t)address+bytes<=(uint64_t)views[i].guest_base+views[i].bytes)
            return views[i].storage+(address-views[i].guest_base);
    return NULL;
}

int dreadzone_bind_pose_prepare(const DreadzoneBindPose *request) {
    DreadzoneBindPose input;
    DreadzoneModelView views[16];
    u8 work[0x4000];
    const u8 *poses,*binds;
    u32 i,j,row,column,count;
    if(request==NULL || (uintptr_t)request>UINTPTR_MAX-sizeof(input)) return DREADZONE_BIND_INVALID;
    input=*request;
    if(input.scratch==NULL || input.scratch_bytes<sizeof(work) ||
        (uintptr_t)input.scratch>UINTPTR_MAX-sizeof(work)) return DREADZONE_BIND_INVALID;
    if(input.count<0 || input.count>127) return DREADZONE_BIND_UNSUPPORTED;
    count=(u32)input.count;
    memcpy(work,input.scratch,sizeof(work));
    if(count==0) {
        for(i=0;i<16;i++) write_float(work+i*4,i%5==0 ? 1.0f : 0.0f);
        memcpy(input.scratch,work,64);
        return DREADZONE_BIND_APPROXIMATE;
    }
    if(input.views==NULL || input.view_count==0 || input.view_count>16 ||
        (uintptr_t)input.views>UINTPTR_MAX-input.view_count*sizeof(*views)) return DREADZONE_BIND_INVALID;
    memcpy(views,input.views,input.view_count*sizeof(*views));
    for(i=0;i<input.view_count;i++) {
        if(views[i].storage==NULL || views[i].bytes==0 ||
            (uintptr_t)views[i].storage>UINTPTR_MAX-views[i].bytes ||
            (uint64_t)views[i].guest_base+views[i].bytes>UINT64_C(0x100000000) ||
            overlap(views[i].guest_base,views[i].bytes,UINT64_C(0x70000000),sizeof(work)) ||
            overlap((uintptr_t)views[i].storage,views[i].bytes,(uintptr_t)input.scratch,sizeof(work)))
            return DREADZONE_BIND_INVALID;
        for(j=0;j<i;j++)
            if(overlap(views[i].guest_base,views[i].bytes,views[j].guest_base,views[j].bytes) ||
                overlap((uintptr_t)views[i].storage,views[i].bytes,(uintptr_t)views[j].storage,views[j].bytes))
                return DREADZONE_BIND_INVALID;
    }
    if((input.pose_guest&15)!=0 || (input.bind_guest&15)!=0) return DREADZONE_BIND_INVALID;
    poses=input.pose_guest==0x70000000 ? work : resolve(views,input.view_count,input.pose_guest,count*64);
    binds=resolve(views,input.view_count,input.bind_guest,count*48);
    if(poses==NULL || binds==NULL) return DREADZONE_BIND_UNMAPPED;
    if(input.pose_guest!=0x70000000) memcpy(work,poses,count*64);
    memcpy(work+count*64,binds,count*48);
    for(i=0;i<count;i++) {
        float pose[16],binding[12],result[16];
        for(j=0;j<16;j++) {
            pose[j]=read_float(work+i*64+j*4);
            if(!valid_float(pose[j])) return DREADZONE_BIND_UNSUPPORTED;
        }
        for(j=0;j<12;j++) {
            binding[j]=read_float(work+count*64+i*48+j*4);
            if(!valid_float(binding[j])) return DREADZONE_BIND_UNSUPPORTED;
        }
        for(column=0;column<4;column++) for(row=0;row<4;row++) {
            /* Actual A40/AA8: MULA, MADDA, MADD for basis; translation
             * uses three accumulator operations then MADD pose.w * vf0.w.
             * Volatile intermediates exclude a host fused multiply-add. */
            volatile float p0=pose[row]*binding[column];
            volatile float p1=pose[4+row]*binding[4+column];
            volatile float a1=p0+p1;
            volatile float p2=pose[8+row]*binding[8+column];
            volatile float a2=a1+p2;
            float value=a2;
            if(column==3) {
                volatile float p3=pose[12+row]*1.0f;
                volatile float a3=a2+p3;
                value=a3;
                if(!valid_float(p3)) return DREADZONE_BIND_UNSUPPORTED;
            }
            if(!valid_float(p0) || !valid_float(p1) || !valid_float(a1) ||
                !valid_float(p2) || !valid_float(a2) || !valid_float(value))
                return DREADZONE_BIND_UNSUPPORTED;
            result[column*4+row]=value;
        }
        for(j=0;j<16;j++) write_float(work+i*64+j*4,result[j]);
    }
    memcpy(input.scratch,work,sizeof(work));
    return DREADZONE_BIND_APPROXIMATE;
}
