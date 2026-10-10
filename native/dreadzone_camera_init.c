#include "dreadzone_camera_init.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

static int overlap(uint64_t a,uint64_t n,uint64_t b,uint64_t m) {
    return a<b+m && b<a+n;
}
static u8 *resolve(const DreadzoneCameraView *views,u32 count,u32 address,u32 n) {
    u32 i;
    for(i=0;i<count;i++)
        if(address>=views[i].guest_base &&
            (uint64_t)address+n<=(uint64_t)views[i].guest_base+views[i].bytes)
            return views[i].storage+(address-views[i].guest_base);
    return NULL;
}
static void put32(u8 *p,u32 bits) {
    p[0]=(u8)bits;p[1]=(u8)(bits>>8);p[2]=(u8)(bits>>16);p[3]=(u8)(bits>>24);
}
static void putfloat(u8 *p,float value) {
    u32 bits;memcpy(&bits,&value,4);put32(p,bits);
}
static int valid(float value) {
    return isfinite(value) && (value==0.0f || fabsf(value)>=FLT_MIN);
}
static int multiply(float a,float b,float *out) {
    volatile float result=a*b;*out=result;return valid(result);
}
static int add(float a,float b,float *out) {
    volatile float result=a+b;*out=result;return valid(result);
}
#define MUL(a,b,c) do { if(!multiply(a,b,&(c))) return DREADZONE_CAMERA_UNSUPPORTED; } while(0)
#define ADD(a,b,c) do { if(!add(a,b,&(c))) return DREADZONE_CAMERA_UNSUPPORTED; } while(0)
static void read_after_writes(u8 *dest,u32 address,u32 n,const u8 *source,
    u32 output,const u8 *work) {
    u32 i;
    for(i=0;i<n;i++) {
        uint64_t a=(uint64_t)address+i;
        dest[i]=(a>=output && a<(uint64_t)output+0x150) ? work[a-output] : source[i];
    }
}

int dreadzone_camera_init(const DreadzoneCameraInit *request) {
    DreadzoneCameraInit input;
    DreadzoneCameraView views[8];
    u8 work[0x150],first[16],second[16],third[16],quad[16],global[4];
    u8 *output,*position,*clock;
    u32 i,j,descriptor_bytes;
    float pair0[2],pair13[2],pair12[2],f0,f1,f2,f3,f4,f5,f14,f15;
    if(request==NULL || (uintptr_t)request>UINTPTR_MAX-sizeof(input))
        return DREADZONE_CAMERA_INVALID;
    memcpy(&input,request,sizeof(input));
    if(input.views==NULL || input.view_count==0 || input.view_count>8 ||
        (input.output&15)!=0 || (input.position&15)!=0)
        return DREADZONE_CAMERA_INVALID;
    descriptor_bytes=input.view_count*(u32)sizeof(views[0]);
    if((uintptr_t)input.views>UINTPTR_MAX-descriptor_bytes)
        return DREADZONE_CAMERA_INVALID;
    memcpy(views,input.views,descriptor_bytes);
    for(i=0;i<input.view_count;i++) {
        if(views[i].storage==NULL || views[i].bytes==0 ||
            (uint64_t)views[i].guest_base+views[i].bytes>UINT64_C(0x100000000) ||
            (uintptr_t)views[i].storage>UINTPTR_MAX-views[i].bytes)
            return DREADZONE_CAMERA_INVALID;
        for(j=0;j<i;j++)
            if(overlap(views[i].guest_base,views[i].bytes,views[j].guest_base,views[j].bytes) ||
                overlap((uintptr_t)views[i].storage,views[i].bytes,
                    (uintptr_t)views[j].storage,views[j].bytes))
                return DREADZONE_CAMERA_INVALID;
    }
    output=resolve(views,input.view_count,input.output,sizeof(work));
    position=resolve(views,input.view_count,input.position,16);
    clock=resolve(views,input.view_count,0x0021E754U,4);
    if(output==NULL || position==NULL || clock==NULL)
        return DREADZONE_CAMERA_UNMAPPED;
    if(overlap((uintptr_t)output,sizeof(work),(uintptr_t)request,sizeof(input)) ||
        overlap((uintptr_t)output,sizeof(work),(uintptr_t)input.views,descriptor_bytes))
        return DREADZONE_CAMERA_INVALID;
    if(dreadzone_angle_pair(0.0f,pair0)!=DREADZONE_ANGLE_APPROXIMATE ||
        dreadzone_angle_pair(input.angle13,pair13)!=DREADZONE_ANGLE_APPROXIMATE ||
        dreadzone_angle_pair(input.angle12,pair12)!=DREADZONE_ANGLE_APPROXIMATE)
        return DREADZONE_CAMERA_UNSUPPORTED;
    MUL(pair0[0],pair13[0],f0);
    MUL(pair0[1],pair13[0],f2);
    MUL(f0,pair12[0],f14);
    MUL(pair0[1],pair12[0],f5);
    f4=-pair12[1];
    MUL(pair12[1],f0,f0);
    MUL(pair0[1],pair12[1],f1);
    MUL(f2,pair12[0],f15);
    MUL(pair0[0],pair12[0],f3);
    MUL(f2,pair12[1],f2);
    ADD(f0,-f5,f0);
    ADD(f1,f14,f1);
    MUL(f4,pair0[0],f4);
    ADD(f2,f3,f2);
    ADD(f4,f15,f4);
    putfloat(second,f0);putfloat(second+4,f1);
    putfloat(third,f2);putfloat(third+4,f4);
    MUL(pair13[1],pair12[0],f0);
    MUL(pair13[1],pair12[1],f1);
    f3=-pair13[0];
    MUL(pair13[1],pair0[0],f2);
    MUL(pair0[1],pair13[1],f5);
    putfloat(first,f1);putfloat(first+4,f0);putfloat(first+8,f3);put32(first+12,0);
    putfloat(second+8,f2);put32(second+12,0);
    putfloat(third+8,f5);put32(third+12,0);
    memcpy(work,output,sizeof(work));
    put32(work+0x44,0);
    put32(work+0x48,0x3DD67751U);put32(work+0x4C,0x3D567751U);
    put32(work+0x50,0x3B64C389U);put32(work+0x54,0x3AFE2E98U);
    put32(work+0x70,0x3F19999AU);put32(work+0x74,0x3F2A7EFAU);
    put32(work+0x78,0x3F59999AU);put32(work+0x7C,0x3EB2B8C2U);
    put32(work+0x120,0x42340000U);put32(work+0x5C,0x3F000000U);
    put32(work+0x68,0x3D0EFA36U);put32(work+0x6C,0x3ABEA2F2U);
    put32(work+0xBC,input.actor);
    put32(work+0xB4,0x3FBDE44EU);put32(work+0xB8,0xBFBDE44EU);
    putfloat(work+0x40,input.angle12);putfloat(work+0x60,input.angle13);
    put32(work+0x64,0);work[0x86]=0;work[0x87]=0;
    put32(work+0x88,0);work[0x84]=0;work[0x85]=0;
    put32(work+0x80,0);put32(work+0x12C,0);put32(work+0x138,0);
    put32(work+0x8C,0);put32(work+0xB0,0);
    memset(work+0xA0,0,16);
    memcpy(work+0x10,second,16);memcpy(work+0x20,third,16);
    memcpy(work+0xF0,first,16);memcpy(work,first,16);
    read_after_writes(quad,input.position,16,position,input.output,work);
    memcpy(work+0x30,quad,16);
    read_after_writes(quad,input.position,16,position,input.output,work);
    memcpy(work+0x140,quad,16);
    read_after_writes(global,0x0021E754U,4,clock,input.output,work);
    memcpy(work+0x134,global,4);
    memcpy(output,work,sizeof(work));
    return DREADZONE_CAMERA_APPROXIMATE;
}
