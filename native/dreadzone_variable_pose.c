#include "dreadzone_variable_pose.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Variable pose requires IEEE binary32 floats
#endif
typedef char variable_pose_float_width[sizeof(float)==4 ? 1 : -1];
typedef struct PosePair { u32 quad[2][4], pointer[2]; } PosePair;
typedef struct PoseDecoder {
    u8 scratch[0x4000];
    float cache[256][4];
    u8 defined[256];
    PosePair previous[2];
    u32 fixed_end,variable_end,next_end,static_cursor,descriptor,opcode;
    u32 variable_quad,variable_packed,bank;
    s32 counter[3];
    float fraction;
} PoseDecoder;
static u16 pose_half(const u8 *p) { return (u16)((u16)p[0]|((u16)p[1]<<8)); }
static u32 pose_bits(float f) { u32 bits;memcpy(&bits,&f,4);return bits; }
static s32 pose_signed(u32 value) {
    return value&0x80000000U ? -1-(s32)(~value) : (s32)value;
}
static s32 pose_signed_half(u16 value) { return value&0x8000 ? (s32)value-65536 : (s32)value; }
static int pose_span(u32 address,u32 bytes,u32 begin,u32 end) {
    return address>=begin && address<=end && bytes<=end-address;
}
static int pose_group(PoseDecoder *d,PosePair *result) {
    s32 current[8],next[8],bias[8],index,kind,counter;
    u8 descriptor[8],opcode;
    u32 j,pointer_address;
    float values[8];
    PosePair *p=&d->previous[d->bank];
    if(!pose_span(d->descriptor,8,0x2010,d->fixed_end) ||
       !pose_span(d->opcode,1,0x2010,d->fixed_end)) return 0;
    memcpy(descriptor,d->scratch+d->descriptor,8);d->descriptor+=8;
    opcode=d->scratch[d->opcode++];
    if(opcode!=0 && opcode!=24 && opcode!=48) return 0;
    kind=opcode==0 ? 0 : opcode==24 ? 1 : 2;
    counter=d->counter[kind]+=32;
    if(opcode==0) {
        if(!pose_span(d->static_cursor,16,0x2010,d->fixed_end)) return 0;
        for(j=0;j<8;j++) current[j]=next[j]=pose_signed_half(pose_half(d->scratch+d->static_cursor+j*2));
        d->static_cursor+=16;
    } else if(opcode==24) {
        if(!pose_span(d->variable_quad,16,0x3000,d->variable_end) ||
           !pose_span(d->variable_quad+0x800,16,0x3800,d->next_end)) return 0;
        for(j=0;j<8;j++) {
            current[j]=pose_signed_half(pose_half(d->scratch+d->variable_quad+j*2));
            next[j]=pose_signed_half(pose_half(d->scratch+d->variable_quad+0x800+j*2));
        }
        d->variable_quad+=16;
    } else {
        if(!pose_span(d->static_cursor,16,0x2010,d->fixed_end) ||
           !pose_span(d->variable_packed,8,0x3000,d->variable_end) ||
           !pose_span(d->variable_packed+0x800,8,0x3800,d->next_end)) return 0;
        for(j=0;j<8;j++) {
            s32 a=d->scratch[d->variable_packed+j],b=d->scratch[d->variable_packed+0x800+j];
            if(a>=128) a-=256;
            if(b>=128) b-=256;
            bias[j]=pose_signed_half(pose_half(d->scratch+d->static_cursor+j*2));
            current[j]=pose_signed_half((u16)(a*4+bias[j]));
            next[j]=pose_signed_half((u16)(b*4+bias[j]));
        }
        d->static_cursor+=16;d->variable_packed+=8;
    }
    for(j=0;j<8;j++) {
        u16 low=j==0 ? (u16)counter : 0;
        u32 cw=((u32)(u16)current[j]<<16)|descriptor[j];
        u32 nw=((u32)(u16)next[j]<<16)|low;
        volatile float current_float=(float)pose_signed(cw),next_float=(float)pose_signed(nw);
        volatile float remainder=1.0f-d->fraction;
        volatile float a=current_float*remainder,b=next_float*d->fraction;
        volatile float value=a+b;
        values[j]=value;
    }
    for(j=0;j<8;j++) {
        u32 lane;
        index=(counter&127)+128+(s32)j*4;
        for(lane=0;lane<4;lane++) d->cache[index][lane]=values[j];
        d->defined[index]=1;
    }
    for(j=0;j<8;j++) {
        if(!d->defined[descriptor[j]]) return 0;
        p->quad[j/4][j%4]=pose_bits(d->cache[descriptor[j]][j%4]);
    }
    for(j=0;j<2;j++) {
        volatile float index_float=(float)descriptor[j*4+3];
        volatile float delta=index_float*1208925819614629174706176.0f;
        volatile float address=158456325028528675187087900672.0f+delta;
        pointer_address=pose_bits(address);p->pointer[j]=pointer_address;
    }
    *result=d->previous[1-d->bank];d->bank=1-d->bank;
    return 1;
}
static int pose_store(PoseDecoder *d,u32 address,const u32 *quad) {
    u32 j;
    if(!pose_span(address,16,0,0x2000) || (address&15)) return 0;
    for(j=0;j<4;j++) {
        u32 v=quad[j];u8 *p=d->scratch+address+j*4;
        p[0]=(u8)v;p[1]=(u8)(v>>8);p[2]=(u8)(v>>16);p[3]=(u8)(v>>24);
    }
    return 1;
}
int dreadzone_variable_pose_decode(const DreadzoneVariablePose *request) {
    DreadzoneVariablePose r;
    PoseDecoder d;
    PosePair result;
    u8 *header;
    u32 i,j,base,count,offset;
    if(!request) return DREADZONE_VARIABLE_POSE_INVALID;
    r=*request;
    if(!r.scratch || r.bytes<0x4000 || (uintptr_t)r.scratch>UINTPTR_MAX-r.bytes ||
       !r.vu_data || r.vu_data_bytes<0x1000 || (uintptr_t)r.vu_data>UINTPTR_MAX-r.vu_data_bytes)
        return DREADZONE_VARIABLE_POSE_INVALID;
    if(!isfinite(r.fraction) || r.fraction<0 || r.fraction>1 ||
       (r.fraction!=0 && r.fraction<FLT_MIN)) return DREADZONE_VARIABLE_POSE_UNSUPPORTED;
    memset(&d,0,sizeof(d));memcpy(d.scratch,r.scratch,0x4000);header=d.scratch+0x2000;
    memcpy(d.cache,r.vu_data,0x1000);memset(d.defined,1,128);
    if(!header[1] || !header[2] || !header[8] || header[6]>127 || header[7]>127 ||
       header[1]>128 || header[2]>128) return DREADZONE_VARIABLE_POSE_UNSUPPORTED;
    d.fixed_end=0x2000+(u32)header[1]*16;
    d.variable_end=0x3000+(u32)header[2]*16;d.next_end=0x3800+(u32)header[2]*16;
    d.static_cursor=0x2010;d.descriptor=0x2010+(u32)header[3]*16;
    d.opcode=d.descriptor+(u32)header[4]*8;
    d.variable_quad=0x3000;d.variable_packed=0x3000+(u32)header[5]*16;
    d.counter[0]=-32;d.counter[1]=-30;d.counter[2]=-31;d.fraction=r.fraction;
    if(!pose_group(&d,&result)) return DREADZONE_VARIABLE_POSE_UNSUPPORTED;
    d.descriptor-=8;
    if(!pose_group(&d,&result)) return DREADZONE_VARIABLE_POSE_UNSUPPORTED;
    d.descriptor-=8;
    if(!pose_group(&d,&result)) return DREADZONE_VARIABLE_POSE_UNSUPPORTED;
    base=(u32)header[6]*64;
    for(i=0;i<header[8];i++) {
        if(!pose_group(&d,&result) || !pose_store(&d,base+i*128,result.quad[0]) ||
           !pose_store(&d,base+i*128+64,result.quad[1])) return DREADZONE_VARIABLE_POSE_UNSUPPORTED;
    }
    for(j=0;j<2;j++) {
        count=header[9+j];offset=16+j*16;
        for(i=0;i<count;i++) {
            if(!pose_group(&d,&result) || result.pointer[0]<0x70000000 || result.pointer[1]<0x70000000 ||
               !pose_store(&d,result.pointer[0]-0x70000000+offset,result.quad[0]) ||
               !pose_store(&d,result.pointer[1]-0x70000000+offset,result.quad[1]))
                return DREADZONE_VARIABLE_POSE_UNSUPPORTED;
        }
    }
    memcpy(r.scratch,d.scratch,0x4000);
    return DREADZONE_VARIABLE_POSE_APPROXIMATE;
}
