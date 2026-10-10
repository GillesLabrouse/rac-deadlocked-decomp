#include "dreadzone_collision_grid.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
static u32 grid_bits(float value) {
    u32 result;memcpy(&result,&value,4);return result;
}
static int grid_valid(float value) {
    return isfinite(value)&&(value==0.0f || fabsf(value)>=FLT_MIN);
}
static int grid_sub(float a,float b,float *out) {
    volatile float value=a-b;*out=value;return grid_valid(value);
}
static int grid_mul(float a,float b,float *out) {
    volatile float value=a*b;*out=value;return grid_valid(value);
}
static int grid_overlap(const void *a,size_t a_bytes,const void *b,size_t b_bytes) {
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
    if(x>UINTPTR_MAX-a_bytes || y>UINTPTR_MAX-b_bytes)return 1;
    return x<y+b_bytes && y<x+a_bytes;
}
int dreadzone_collision_grid(const DreadzoneCollisionGridRequest *request,
                            DreadzoneCollisionGridRecord *records,u32 capacity,
                            DreadzoneCollisionGridInfo *info) {
    DreadzoneCollisionGridRequest r;
    DreadzoneCollisionGridRecord result[DREADZONE_COLLISION_GRID_MAX_RECORDS];
    DreadzoneCollisionGridInfo metadata;
    u32 list[3][80],index[3]={0,0,0},count[3],start_cell[3],end_cell[3];
    s32 step[3];float direction[3];u32 axis,total=0,packed,n;
    if(request==NULL || records==NULL || info==NULL ||
       (uintptr_t)request>UINTPTR_MAX-sizeof(r))return DREADZONE_GRID_INVALID;
    memcpy(&r,request,sizeof(r));memset(&metadata,0,sizeof(metadata));
    for(axis=0;axis<3;axis++) {
        float a=r.start_scaled[axis],b=r.end_scaled[axis];
        if(!grid_valid(a)||!grid_valid(b)||(grid_bits(a)|grid_bits(b))&0x80000000U ||
           a>=1048576.0f || b>=1048576.0f)return DREADZONE_GRID_UNSUPPORTED;
        if(!grid_sub(b,a,direction+axis))return DREADZONE_GRID_UNSUPPORTED;
        start_cell[axis]=(u32)a>>12;end_cell[axis]=(u32)b>>12;
        if((u32)a!=(u32)b)metadata.has_integer_displacement=1;
        step[axis]=end_cell[axis]>=start_cell[axis]?1:-1;
        count[axis]=end_cell[axis]>=start_cell[axis]?end_cell[axis]-start_cell[axis]:start_cell[axis]-end_cell[axis];
        if(count[axis]>79)return DREADZONE_GRID_UNSUPPORTED;
        total+=count[axis];
    }
    packed=start_cell[0]|(start_cell[1]<<8)|(start_cell[2]<<16);
    result[0].fraction_bits=0;result[0].packed_cell=packed;
    if(total!=0) {
        for(axis=0;axis<3;axis++) {
            float reciprocal;
            s32 boundary=(s32)start_cell[axis]+(step[axis]>0?1:0);
            if(direction[axis]==0.0f) {
                /* DIV uses VF0.w=1: genuine divide-by-zero Q saturation.
                 * No fraction consumes Q when this axis has no crossings. */
                u32 word=0x7F7FFFFFU|(grid_bits(direction[axis])&0x80000000U);
                memcpy(&reciprocal,&word,4);metadata.zero_q_division_mask|=1U<<axis;
            } else {
                volatile float value=1.0f/direction[axis];reciprocal=value;
                if(!grid_valid(reciprocal))return DREADZONE_GRID_UNSUPPORTED;
            }
            for(n=0;n<count[axis];n++,boundary+=step[axis]) {
                float offset,fraction;u32 word;
                if(!grid_sub((float)(boundary*4096),r.start_scaled[axis],&offset) ||
                   !grid_mul(offset,reciprocal,&fraction))return DREADZONE_GRID_UNSUPPORTED;
                word=grid_bits(fraction);
                if((s32)word>(s32)0x3F800000U)word=0x3F800000U;
                if((s32)word<0)word=0;
                list[axis][n]=word;
            }
            list[axis][count[axis]]=0x50000000U;
        }
        for(n=0;n<total;n++) {
            u32 selected=0;
            /* Original strict less-than comparisons retain X then Y ties. */
            if((s32)list[1][index[1]]<(s32)list[0][index[0]])selected=1;
            if((s32)list[2][index[2]]<(s32)list[selected][index[selected]])selected=2;
            result[n+1].fraction_bits=list[selected][index[selected]++];
            packed+=(u32)step[selected]<<(8*selected);
            result[n+1].packed_cell=packed;
        }
    }
    if(metadata.has_integer_displacement) {
        result[total+1].fraction_bits=0x3F800000U;result[total+1].packed_cell=0;
        metadata.record_count=total+2;
    } else metadata.record_count=1;
    if(capacity<metadata.record_count)return DREADZONE_GRID_CAPACITY;
    if(grid_overlap(records,metadata.record_count*sizeof(*records),info,sizeof(*info)))return DREADZONE_GRID_INVALID;
    memcpy(records,result,metadata.record_count*sizeof(*records));
    memcpy(info,&metadata,sizeof(metadata));return DREADZONE_GRID_APPROXIMATE;
}
