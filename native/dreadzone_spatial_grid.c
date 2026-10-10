#include "dreadzone_spatial_grid.h"
#include <stdlib.h>
#include <string.h>

typedef struct SpatialWork {
    u8 actor[0x100],grid[0x4000],blocks[0xC000],bitmap[0xC0];
} SpatialWork;

static u16 read16(const u8 *p) { return (u16)((u16)p[0]|((u16)p[1]<<8)); }
static u32 read32(const u8 *p) {
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}
static void write16(u8 *p,u32 v) { p[0]=(u8)v;p[1]=(u8)(v>>8); }
static void write32(u8 *p,u32 v) {
    p[0]=(u8)v;p[1]=(u8)(v>>8);p[2]=(u8)(v>>16);p[3]=(u8)(v>>24);
}
static int rectangle_valid(u32 r) {
    u32 x=r&255,y=(r>>8)&255,endx=(r>>16)&255,endy=r>>24;
    return (r&0x80000000U)!=0 || (x<=endx && y<=endy && endx<64 && endy<64);
}
static int outside(u32 x,u32 y,u32 r) {
    return (s8)x<(s8)r || (s8)y<(s8)(r>>8) ||
           (s8)x>(s8)(r>>16) || (s8)y>(s8)(r>>24);
}
static int span(const void *p,u32 bytes,uintptr_t *start,uintptr_t *end) {
    uintptr_t base=(uintptr_t)p;
    if(!p || bytes==0 || base>UINTPTR_MAX-bytes) return 0;
    *start=base;*end=base+bytes;return 1;
}
static int descriptors_valid(const DreadzoneSpatialRequest *original,
                             const DreadzoneSpatialRequest *r) {
    uintptr_t starts[5],ends[5];u32 i,j,difference;
    if(r->actor_bytes<0x100 || r->grid_bytes<0x4000 || r->block_bytes<0xC000 || r->bitmap_bytes<0xC0)
        return 0;
    /* Check complete supplied spans, including unused backing and the descriptor. */
    if(!span(original,(u32)sizeof(*original),starts,ends) ||
       !span(r->actor,r->actor_bytes,starts+1,ends+1) ||
       !span(r->grid,r->grid_bytes,starts+2,ends+2) ||
       !span(r->blocks,r->block_bytes,starts+3,ends+3) ||
       !span(r->bitmap,r->bitmap_bytes,starts+4,ends+4)) return 0;
    for(i=0;i<5;++i) for(j=0;j<i;++j)
        if(starts[i]<ends[j] && starts[j]<ends[i]) return 0;
    if(r->actor_pool_count==0 || r->actor_pool_count>65536 ||
       (r->actor_guest&15) || (r->actor_pool_guest&15) ||
       r->actor_guest<r->actor_pool_guest ||
       ((r->actor_guest^r->actor_pool_guest)&0x80000000U) ||
       (u64)r->actor_pool_guest+(u64)r->actor_pool_count*256>0x100000000ULL ||
       (u64)r->actor_guest+256>0x100000000ULL) return 0;
    difference=r->actor_guest-r->actor_pool_guest;
    if((difference&255) || (difference>>8)>=r->actor_pool_count) return 0;
    /* The separately supplied actor may not alias the actual guest globals. */
    if(r->actor_guest<0x0027FF20U && r->actor_guest+256>0x0026FF20U) return 0;
    if(r->actor_guest<0x0028BFE0U && r->actor_guest+256>0x0028BF20U) return 0;
    return rectangle_valid(r->rectangle);
}
static int contents_valid(const SpatialWork *w,u32 pool_count) {
    u8 owners[1536];u32 cell;
    memset(owners,0,sizeof(owners));
    if(!rectangle_valid(read32(w->actor+0xA4))) return 0;
    for(cell=0;cell<4096;++cell) {
        const u8 *c=w->grid+cell*4;
        u32 first=read16(c),count=c[2],capacity=c[3],i;
        if(capacity==0) { if(count!=0) return 0;continue; }
        if(capacity>16 || (capacity&(capacity-1)) || first%capacity ||
           first+capacity>1536 || count>capacity*16) return 0;
        for(i=first;i<first+capacity;++i) {
            if(owners[i] || !(w->bitmap[i>>3]&(1U<<(i&7)))) return 0;
            owners[i]=1;
        }
        for(i=0;i<count;++i) if(read16(w->blocks+first*32+i*2)>=pool_count) return 0;
    }
    return 1;
}
static int release(SpatialWork *w,u32 first,u32 count) {
    u32 i;
    if(count==0 || first+count>1536) return DREADZONE_SPATIAL_INVALID;
    for(i=0;i<count;++i) {
        u32 at=first+i,bit=1U<<(at&7);
        u8 old=w->bitmap[at>>3];
        if(!(old&bit)) return DREADZONE_SPATIAL_TRAP;
        w->bitmap[at>>3]=(u8)(old&~bit);
    }
    return 0;
}
static int allocate(SpatialWork *w,u32 count,u32 *first) {
    u32 word_index,offset,base_mask;
    if(count==0 || count>16 || (count&(count-1))) return DREADZONE_SPATIAL_INVALID;
    base_mask=(1U<<count)-1;
    for(word_index=0;word_index<48;++word_index) {
        u32 used=read32(w->bitmap+word_index*4);
        for(offset=0;offset<32;offset+=count) {
            u32 mask=base_mask<<offset;
            if((used&mask)==0) {
                write32(w->bitmap+word_index*4,used|mask);
                *first=word_index*32+offset;return 0;
            }
        }
    }
    return DREADZONE_SPATIAL_EXHAUSTED;
}
static void copy_blocks(SpatialWork *w,u32 destination,u32 source,u32 count) {
    u32 i;
    for(i=0;i<count;++i) {
        u8 pair[32];
        /* Each original iteration loads both quads before either store. */
        memcpy(pair,w->blocks+(source+i)*32,32);
        memcpy(w->blocks+(destination+i)*32,pair,32);
    }
}
static int apply(SpatialWork *w,u32 actor_index,u32 rectangle) {
    u32 old=read32(w->actor+0xA4),x,y;
    int status;
    if(!(old&0x80000000U)) {
        for(y=(old>>8)&255;y<=(old>>24);++y) for(x=old&255;x<=((old>>16)&255);++x) {
            if(outside(x,y,rectangle)) {
                u8 *c=w->grid+(x+y*64)*4;
                u32 first=read16(c),count=c[2],capacity=c[3],i;
                for(i=0;i<count;++i) if(read16(w->blocks+first*32+i*2)==actor_index) break;
                if(i==count) {
                    /* Retail's trap branch delay slot reads the end halfword
                     * first, even for an empty list. Refuse if that read leaves
                     * the bounded block backing instead of pretending a trap. */
                    if(first*32+count*2+2>0xC000) return DREADZONE_SPATIAL_INVALID;
                    return DREADZONE_SPATIAL_TRAP;
                }
                write16(w->blocks+first*32+i*2,read16(w->blocks+first*32+(count-1)*2));
                write16(w->blocks+first*32+(count-1)*2,0);
                --count;c[2]=(u8)count;
                if(count==0 || (capacity!=1 && (s32)(capacity*8)-4-(s32)count>0)) {
                    u32 destination,new_capacity=capacity>>1;
                    status=release(w,first,capacity);if(status) return status;
                    write16(c,0);c[3]=(u8)new_capacity;
                    if(new_capacity) {
                        status=allocate(w,new_capacity,&destination);if(status) return status;
                        write16(c,destination);copy_blocks(w,destination,first,new_capacity);
                    }
                }
            }
        }
    }
    old=read32(w->actor+0xA4);write32(w->actor+0xA4,rectangle);
    if(rectangle&0x80000000U) return 0;
    for(y=(rectangle>>8)&255;y<=rectangle>>24;++y)
        for(x=rectangle&255;x<=((rectangle>>16)&255);++x) if(outside(x,y,old)) {
            u8 *c=w->grid+(x+y*64)*4;
            u32 count=c[2],capacity=c[3],first=read16(c);
            if(capacity==0 || capacity*16<=count) {
                u32 destination,new_capacity=capacity ? capacity*2 : 1;
                c[3]=(u8)new_capacity;
                status=allocate(w,new_capacity,&destination);if(status) return status;
                write16(c,destination);
                if(capacity) {
                    status=release(w,first,capacity);if(status) return status;
                    copy_blocks(w,destination,first,new_capacity>>1);
                }
                first=read16(c);count=c[2];
            }
            c[2]=(u8)(count+1);write16(w->blocks+first*32+count*2,actor_index);
        }
    return 0;
}
int dreadzone_spatial_update(const DreadzoneSpatialRequest *request) {
    DreadzoneSpatialRequest r;
    SpatialWork *w;
    int status;
    if(!request || (uintptr_t)request>UINTPTR_MAX-sizeof(r)) return DREADZONE_SPATIAL_INVALID;
    /* Read-only descriptor copy precedes any indirect backing access. */
    memcpy(&r,request,sizeof(r));
    if(!descriptors_valid(request,&r)) return DREADZONE_SPATIAL_INVALID;
    w=(SpatialWork *)malloc(sizeof(*w));
    if(!w) return DREADZONE_SPATIAL_HOST_MEMORY;
    memcpy(w->actor,r.actor,0x100);memcpy(w->grid,r.grid,0x4000);
    memcpy(w->blocks,r.blocks,0xC000);memcpy(w->bitmap,r.bitmap,0xC0);
    if(!contents_valid(w,r.actor_pool_count)) { free(w);return DREADZONE_SPATIAL_INVALID; }
    status=apply(w,(r.actor_guest-r.actor_pool_guest)>>8,r.rectangle);
    if(status) { free(w);return status; }
    memcpy(r.actor,w->actor,0x100);memcpy(r.grid,w->grid,0x4000);
    memcpy(r.blocks,w->blocks,0xC000);memcpy(r.bitmap,w->bitmap,0xC0);
    free(w);return DREADZONE_SPATIAL_COMMITTED;
}
