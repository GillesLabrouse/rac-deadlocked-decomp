#include "dreadzone_pose_masks.h"
#include <stdint.h>
#include <string.h>

static int span(const void *p,size_t n){return p && (uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *p,size_t n,const void *q,size_t m) {
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;return a>=b?a-b<m:b-a<n;
}
static int guest_overlap(u32 a,u32 n,u32 b,u32 m){return a>=b?a-b<m:b-a<n;}
static s32 signed_byte(u8 b){return (s32)(b^128U)-128;}
typedef struct MaskState {
    u8 scratch[0x4000],vu[0x1000];DreadzoneModelView views[64];u32 count;
} MaskState;
static const u8 *read(const MaskState *s,u32 guest,u32 bytes) {
    u32 i,offset;
    if(guest>=0x70000000 && guest-0x70000000<=0x4000 && bytes<=0x4000-(guest-0x70000000))
        return s->scratch+guest-0x70000000;
    for(i=0;i<s->count;++i)if(guest>=s->views[i].guest_base) {
        offset=guest-s->views[i].guest_base;
        if(offset<=s->views[i].bytes && bytes<=s->views[i].bytes-offset)return s->views[i].storage+offset;
    }
    return NULL;
}
static int list_start(const MaskState *s,u32 guest,u32 *cursor,s32 *joint) {
    const u8 *p;
    if((guest&1) || guest>0xFFFFFFFEU)return 0;
    p=read(s,guest,2);if(!p)return 0;
    if(guest>0xFFFFFFFFU-((u32)p[0]|((u32)p[1]<<8))-4U)return 0;
    *cursor=guest+((u32)p[0]|((u32)p[1]<<8))+4U;
    p=read(s,*cursor,1);if(!p)return 0;
    *joint=signed_byte(*p);return 1;
}
static int advance_list(const MaskState *s,u32 *cursor,s32 *next) {
    const u8 *p;
    if(*cursor==0xFFFFFFFFU)return 0;
    ++*cursor;p=read(s,*cursor,1);if(!p)return 0;*next=signed_byte(*p);return 1;
}
static void mask(MaskState *s,u32 joint,u8 value) {
    /* VISWR stores an unsigned16 integer in the low half and zero high half. */
    s->vu[joint*16+12]=value;s->vu[joint*16+13]=0;
    s->vu[joint*16+14]=0;s->vu[joint*16+15]=0;
    s->scratch[joint*64+0x3D]=value;
}
int dreadzone_pose_masks_prepare(const DreadzonePoseMasks *request) {
    DreadzonePoseMasks r;MaskState s;u32 i,j,end,cursor=0;size_t descriptors;s32 next=0;
    if(!span(request,sizeof(*request)))return DREADZONE_POSE_MASKS_INVALID;
    r=*request;
    if(!span(r.scratch,r.scratch_bytes) || r.scratch_bytes<0x4000 ||
       !span(r.vu_data,r.vu_data_bytes) || r.vu_data_bytes<0x1000 ||
       overlap(r.scratch,r.scratch_bytes,r.vu_data,r.vu_data_bytes) || r.view_count>64)
        return DREADZONE_POSE_MASKS_INVALID;
    descriptors=r.view_count*sizeof(*r.views);
    if(descriptors && (!span(r.views,descriptors) || overlap(r.views,descriptors,r.scratch,r.scratch_bytes) ||
       overlap(r.views,descriptors,r.vu_data,r.vu_data_bytes)))return DREADZONE_POSE_MASKS_INVALID;
    if(r.first_joint<0 || r.first_joint>127 || r.joint_count<=0 || r.joint_count>128-r.first_joint)
        return DREADZONE_POSE_MASKS_UNSUPPORTED;
    for(i=0;i<r.view_count;++i) {
        s.views[i]=r.views[i];
        if(!s.views[i].bytes || !span(s.views[i].storage,s.views[i].bytes) ||
           s.views[i].guest_base>0xFFFFFFFFU-(s.views[i].bytes-1) ||
           guest_overlap(s.views[i].guest_base,s.views[i].bytes,0x70000000,0x4000) ||
           overlap(s.views[i].storage,s.views[i].bytes,r.scratch,r.scratch_bytes) ||
           overlap(s.views[i].storage,s.views[i].bytes,r.vu_data,r.vu_data_bytes) ||
           (descriptors && overlap(s.views[i].storage,s.views[i].bytes,r.views,descriptors)))
            return DREADZONE_POSE_MASKS_INVALID;
        for(j=0;j<i;++j)if(guest_overlap(s.views[i].guest_base,s.views[i].bytes,s.views[j].guest_base,s.views[j].bytes) ||
           overlap(s.views[i].storage,s.views[i].bytes,s.views[j].storage,s.views[j].bytes))
            return DREADZONE_POSE_MASKS_INVALID;
    }
    s.count=r.view_count;memcpy(s.scratch,r.scratch,0x4000);memcpy(s.vu,r.vu_data,0x1000);
    end=(u32)(r.first_joint+r.joint_count);
    if(r.first_list_guest && !list_start(&s,r.first_list_guest,&cursor,&next))return DREADZONE_POSE_MASKS_NEEDS_MEMORY;
    for(i=(u32)r.first_joint;i<end;++i) {
        u8 value=0;
        if(r.first_list_guest) {
            value=1;
            if((s32)i==next){if(!advance_list(&s,&cursor,&next))return DREADZONE_POSE_MASKS_NEEDS_MEMORY;value=0;}
        }
        mask(&s,i,value);
    }
    mask(&s,end,1);
    if(r.first_list_guest && r.second_list_guest) {
        if(!list_start(&s,r.second_list_guest,&cursor,&next))return DREADZONE_POSE_MASKS_NEEDS_MEMORY;
        if(next<0)return DREADZONE_POSE_MASKS_UNSUPPORTED;
        i=(u32)next;
        do {
            if(i>127 || next<(s32)i)return DREADZONE_POSE_MASKS_UNSUPPORTED;
            if((s32)i==next){if(!advance_list(&s,&cursor,&next))return DREADZONE_POSE_MASKS_NEEDS_MEMORY;mask(&s,i,0);}
            ++i;
        }while(next>=0);
    }
    memcpy(r.scratch,s.scratch,0x4000);memcpy(r.vu_data,s.vu,0x1000);
    return DREADZONE_POSE_MASKS_COMPLETED;
}
