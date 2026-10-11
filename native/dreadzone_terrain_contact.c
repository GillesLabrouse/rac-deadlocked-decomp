#include "dreadzone_terrain_contact.h"
#include "dreadzone_collision_grid.h"
#include "dreadzone_collision_tree.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
typedef struct TerrainBlock {u8 data[4080];u32 bytes;} TerrainBlock;
static int terrain_valid(float value) {return isfinite(value)&&(value==0.0f||fabsf(value)>=FLT_MIN);}
static int terrain_sub(float a,float b,float *out) {volatile float v=a-b;*out=v;return terrain_valid(v);}
static int terrain_mul(float a,float b,float *out) {volatile float v=a*b;*out=v;return terrain_valid(v);}
static int terrain_add(float a,float b,float *out) {volatile float v=a+b;*out=v;return terrain_valid(v);}
static float terrain_float(u32 bits) {float value;memcpy(&value,&bits,4);return value;}
static u32 terrain_word(const u8 *p) {u32 word;memcpy(&word,p,4);return word;}
static s32 terrain_signed(u32 word,u32 width) {
    u32 mask=(1U<<width)-1,sign=1U<<(width-1);
    return (s32)(word&mask)-(s32)((word&sign)*2U);
}
static int terrain_overlap(const void *a,size_t a_bytes,const void *b,size_t b_bytes) {
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
    if(x>UINTPTR_MAX-a_bytes||y>UINTPTR_MAX-b_bytes)return 1;
    return x<y+b_bytes&&y<x+a_bytes;
}
static int terrain_fetch(const DreadzoneTerrainContactRequest *r,u32 cell,TerrainBlock *block) {
    DreadzoneCollisionTreeRequest tree;
    u32 packed;unsigned long long address,end;
    tree.storage=r->storage;tree.guest_base=r->guest_base;tree.bytes=r->bytes;tree.root=r->root;
    /* Actual53DDA0/53DDA4/53DDAC and53DFEC..53DFFC pass at=X,
     * v0=Y,v1=Z. The isolated tree API names x=v0,y=v1,z=at. */
    tree.x=(s32)(cell>>8&255U);tree.y=(s32)(cell>>16&255U);tree.z=(s32)(cell&255U);
    if(dreadzone_collision_tree_lookup(&tree,&packed)!=DREADZONE_TREE_READ)return 0;
    if(packed==0){block->bytes=0;return 1;}
    block->bytes=(packed&255U)*16U;
    if(block->bytes==0)return 0;
    address=(unsigned long long)r->root+(packed>>8);end=address+block->bytes;
    if(address>=0x80000000ULL||end>0x80000000ULL||address<r->guest_base||
       end>(unsigned long long)r->guest_base+r->bytes)return 0;
    memcpy(block->data,r->storage+(size_t)(address-r->guest_base),block->bytes);
    return 1;
}
static int terrain_decode(const TerrainBlock *block,const DreadzoneCollisionGridRecord *current,
                          const DreadzoneCollisionGridRecord *next,const float start[3],
                          const float end[3],const float direction[3],
                          DreadzoneCollisionTriangleRequest *triangle,float vertices[255][3],
                          u32 clips[255]) {
    u32 axis,index,count=block->data[2];
    float first[3],last[3];s32 low[3],high[3];
    if(4U+count*4U>block->bytes)return 0;
    for(axis=0;axis<3;axis++) {
        float product;u32 coordinate=(current->packed_cell>>(8*axis))&255U;
        s32 a,b,origin=(s32)(coordinate*4096U+2048U);
        triangle->cell_origin[axis]=(float)origin;
        if(!terrain_sub(start[axis],(float)origin,triangle->start_relative+axis)||
           !terrain_sub(end[axis],(float)origin,triangle->end_relative+axis))return 0;
        triangle->direction[axis]=direction[axis];
        if(!terrain_mul(direction[axis],terrain_float(current->fraction_bits),&product)||
           !terrain_add(product,start[axis],first+axis)||
           !terrain_mul(direction[axis],terrain_float(next->fraction_bits),&product)||
           !terrain_add(product,start[axis],last+axis))return 0;
        /* The actual clipped endpoint FTOI0 values remain within signed32. */
        if(first[axis]<0||first[axis]>1048576.0f||last[axis]<0||last[axis]>1048576.0f)return 0;
        a=(s32)first[axis]-origin;b=(s32)last[axis]-origin;
        low[axis]=terrain_signed((u32)(a<b?a:b)-1U,16);
        high[axis]=terrain_signed((u32)(a>b?a:b)+1U,16);
    }
    for(index=0;index<count;index++) {
        u32 word=terrain_word(block->data+4+index*4);
        s32 xyz[3];xyz[0]=terrain_signed(word,10)*64;
        xyz[1]=terrain_signed(word>>10,10)*64;xyz[2]=terrain_signed(word>>20,12)*16;
        clips[index]=0;
        for(axis=0;axis<3;axis++) {
            u32 below=terrain_signed((u32)xyz[axis]-(u32)low[axis],16)<0?1U:0U;
            u32 above=terrain_signed((u32)high[axis]-(u32)xyz[axis],16)<0?2U:0U;
            clips[index]|=(below|above)<<(axis*8);vertices[index][axis]=(float)xyz[axis];
        }
    }
    return 1;
}
int dreadzone_terrain_contact_tracked(const DreadzoneTerrainContactRequest *request,
                             DreadzoneCollisionTriangleContact *contact,
                             DreadzoneTerrainContactInfo *info,DreadzoneProbeRegisters *state) {
    DreadzoneProbeRegisters tracked={{0,0},0};
    DreadzoneTerrainContactRequest r;DreadzoneTerrainContactInfo detail;
    DreadzoneCollisionGridRequest grid_request;DreadzoneCollisionGridInfo grid_info;
    DreadzoneCollisionGridRecord records[DREADZONE_COLLISION_GRID_MAX_RECORDS];
    DreadzoneCollisionTriangleRequest triangle;DreadzoneCollisionTriangleContact candidate;
    TerrainBlock buffers[2];float direction[3],vertices[255][3];u32 clips[255];
    u32 axis,record,current_buffer=0,start_cell=0;int have_contact=0;
    if(request==NULL||contact==NULL||info==NULL||
       (uintptr_t)request>UINTPTR_MAX-sizeof(r))return DREADZONE_TRIANGLE_INVALID;
    if(state){
        if(terrain_overlap(state,sizeof(*state),request,sizeof(r))||
           terrain_overlap(state,sizeof(*state),contact,sizeof(*contact))||
           terrain_overlap(state,sizeof(*state),info,sizeof(*info)))return DREADZONE_TRIANGLE_INVALID;
        memcpy(&tracked,state,sizeof(tracked));
    }
    memcpy(&r,request,sizeof(r));
    if(r.storage==NULL||r.bytes==0||r.guest_base>=0x80000000U||
       (unsigned long long)r.guest_base+r.bytes>0x80000000ULL||
       (uintptr_t)r.storage>UINTPTR_MAX-r.bytes||
       terrain_overlap(contact,sizeof(*contact),info,sizeof(*info))||
       terrain_overlap(contact,sizeof(*contact),r.storage,r.bytes)||
       terrain_overlap(info,sizeof(*info),r.storage,r.bytes))return DREADZONE_TRIANGLE_INVALID;
    if(state&&terrain_overlap(state,sizeof(*state),r.storage,r.bytes))return DREADZONE_TRIANGLE_INVALID;
    if(r.caller_flags&1U)return DREADZONE_TRIANGLE_UNSUPPORTED;
    memset(&detail,0,sizeof(detail));
    /* Original0053DF80 initializes only the live closest fraction to1. */
    candidate.fraction=1.0f;
    for(axis=0;axis<3;axis++) {
        if(!terrain_valid(r.start_world[axis])||!terrain_valid(r.end_world[axis])||
           signbit(r.start_world[axis])||signbit(r.end_world[axis])||
           r.start_world[axis]>=1024.0f||r.end_world[axis]>=1024.0f||
           !terrain_mul(r.start_world[axis],1024.0f,grid_request.start_scaled+axis)||
           !terrain_mul(r.end_world[axis],1024.0f,grid_request.end_scaled+axis)||
           !terrain_sub(grid_request.end_scaled[axis],grid_request.start_scaled[axis],direction+axis))
            return DREADZONE_TRIANGLE_UNSUPPORTED;
        start_cell|=((u32)grid_request.start_scaled[axis]>>12)<<(axis*8);
    }
    /* Original initial lookup/fetch precedes construction of the grid lists. */
    if(!terrain_fetch(&r,start_cell,buffers))return DREADZONE_TRIANGLE_UNSUPPORTED;
    if(dreadzone_collision_grid(&grid_request,records,DREADZONE_COLLISION_GRID_MAX_RECORDS,&grid_info)!=1)
        return DREADZONE_TRIANGLE_UNSUPPORTED;
    detail.grid_records=grid_info.record_count;detail.grid_zero_q_mask=grid_info.zero_q_division_mask;
    for(record=0;record+1<grid_info.record_count;record++,current_buffer^=1U) {
        TerrainBlock *block=buffers+current_buffer;u32 next_buffer=current_buffer^1U;
        u32 nv,nf,nq,face_start,pass,index;
        if(records[record].fraction_bits==0x3F800000U||
           (have_contact && candidate.fraction<=terrain_float(records[record].fraction_bits)))break;
        /* This read/copy must occur before any current-block field decoding.
         * The actual RAM-to-scratch DMA fills the alternate buffer here. */
        if(records[record+1].fraction_bits!=0x3F800000U) {
            if(!terrain_fetch(&r,records[record+1].packed_cell,buffers+next_buffer))return DREADZONE_TRIANGLE_UNSUPPORTED;
        } else buffers[next_buffer].bytes=0;
        detail.cells_processed++;
        if(block->bytes==0||block->data[2]==0)continue;
        nv=block->data[2];nf=block->data[0];nq=block->data[3];face_start=4+nv*4;
        if(nq>nf||face_start+nf*4+nq>block->bytes)return DREADZONE_TRIANGLE_UNSUPPORTED;
        if(!terrain_decode(block,records+record,records+record+1,grid_request.start_scaled,
                           grid_request.end_scaled,direction,&triangle,vertices,clips))return DREADZONE_TRIANGLE_UNSUPPORTED;
        /* Actual final packed vertex producer53E184, before eligibility. */
        if(state){tracked.high64[0]=tracked.high64[1]=0;tracked.last_writer_pc=0x53E184U;}
        triangle.effective_flags=r.caller_flags^0x10U;triangle.source_guest=r.root;
        for(pass=0;pass<2;pass++) {
            u32 count=pass==0?nf:nq;
            if(pass==1) {
                /* Exact scratch face mutation precedes the entire quad pass. */
                for(index=0;index<nq;index++) {
                    u8 *face=block->data+face_start+index*4;
                    face[1]=face[2];face[2]=block->data[face_start+nf*4+index];
                }
            }
            for(index=0;index<count;index++) {
                u8 *face=block->data+face_start+index*4;DreadzoneCollisionTriangleQStatus q_status;
                int result;u32 corner;
                for(corner=0;corner<3;corner++) {
                    if(face[corner]>=nv)return DREADZONE_TRIANGLE_UNSUPPORTED;
                    memcpy(triangle.vertex[corner],vertices[face[corner]],3*sizeof(float));
                    triangle.clip_flags[corner]=clips[face[corner]];
                }
                triangle.surface=face[3];triangle.best_fraction=have_contact?candidate.fraction:1.0f;
                result=dreadzone_collision_triangle_with_q(&triangle,&candidate,&q_status);
                if(result<0)return DREADZONE_TRIANGLE_UNSUPPORTED;
                detail.faces_processed++;if(pass)detail.quad_faces_processed++;
                detail.triangle_q_exceptions|=q_status.exception_bits;
                if(result==1)have_contact=1;
            }
        }
    }
    if(have_contact)memcpy(contact,&candidate,sizeof(candidate));
    if(state)memcpy(state,&tracked,sizeof(tracked));
    memcpy(info,&detail,sizeof(detail));return have_contact?1:0;
}

int dreadzone_terrain_contact(const DreadzoneTerrainContactRequest *request,
                             DreadzoneCollisionTriangleContact *contact,
                             DreadzoneTerrainContactInfo *info){
    return dreadzone_terrain_contact_tracked(request,contact,info,NULL);
}
