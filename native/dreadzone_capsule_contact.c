#include "dreadzone_capsule_contact.h"
#include "dreadzone_collision_tree.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct CapsuleWork {
    DreadzoneCapsuleRequest r;
    DreadzoneCapsuleInfo info;
    u8 *memory;
    int failed;
} CapsuleWork;
typedef struct CapsuleCell {u32 leaf;float origin[3];} CapsuleCell;
typedef struct CapsuleBlock {u8 data[4096];u32 bytes;} CapsuleBlock;
typedef struct CapsuleWinner {
    float position[3],normal[3],vertex[3][3],best;
    u32 metadata,vf2_w;
    int have;
} CapsuleWinner;
static u32 caps_bits(float v) {u32 x;memcpy(&x,&v,4);return x;}
static float caps_float(u32 x) {float v;memcpy(&v,&x,4);return v;}
static int caps_valid(float v) {return isfinite(v)&&(v==0.f||fabsf(v)>=FLT_MIN);}
static float caps_check(CapsuleWork *p,float v) {
    if(!caps_valid(v))p->failed=DREADZONE_CAPSULE_UNSUPPORTED;
    return v;
}
static float caps_add(CapsuleWork *p,float a,float b) {volatile float v=a+b;return caps_check(p,v);}
static float caps_sub(CapsuleWork *p,float a,float b) {volatile float v=a-b;return caps_check(p,v);}
static float caps_mul(CapsuleWork *p,float a,float b) {volatile float v=a*b;return caps_check(p,v);}
static float caps_min(float a,float b) {return b<a?b:a;}
static float caps_max(float a,float b) {return b>a?b:a;}
static float caps_div(CapsuleWork *p,float a,float b) {
    float q;
    if(b==0.f) {
        q=caps_float(0x7F7FFFFFU|((caps_bits(a)^caps_bits(b))&0x80000000U));
        p->info.q_exception_bits|=a==0.f?0x10U:0x20U;
    } else {volatile float v=a/b;q=caps_check(p,v);}
    p->info.q_bits=caps_bits(q);return q;
}
static float caps_dot(CapsuleWork *p,const float a[3],const float b[3]) {
    float x=caps_mul(p,a[0],b[0]),y=caps_mul(p,a[1],b[1]),z=caps_mul(p,a[2],b[2]);
    return caps_add(p,caps_add(p,x,y),caps_mul(p,1.f,z));
}
static void caps_difference(CapsuleWork *p,const float a[3],const float b[3],float o[3]) {
    u32 i;for(i=0;i<3;i++)o[i]=caps_sub(p,a[i],b[i]);
}
static void caps_cross(CapsuleWork *p,const float a[3],const float b[3],float o[3]) {
    u32 i;for(i=0;i<3;i++)o[i]=caps_sub(p,caps_mul(p,a[(i+1)%3],b[(i+2)%3]),caps_mul(p,b[(i+1)%3],a[(i+2)%3]));
}
static void caps_projection(CapsuleWork *p,const float base[3],const float edge[3],float q,float out[3]) {
    u32 i;for(i=0;i<3;i++)out[i]=caps_add(p,caps_add(p,0.f,base[i]),caps_mul(p,edge[i],q));
}
static void caps_sides(CapsuleWork *p,const float point[3],const float e2[3],const float e3[3],const float n[3],float out[3]) {
    float a[3],b[3],c[3];
    caps_cross(p,point,e2,c);out[0]=caps_dot(p,c,n);
    caps_cross(p,e3,point,c);out[1]=caps_dot(p,c,n);
    caps_difference(p,e3,e2,a);caps_difference(p,point,e2,b);caps_cross(p,b,a,c);out[2]=caps_dot(p,c,n);
}
static void caps_edge(CapsuleWork *p,const float point[3],const float e2[3],const float e3[3],const float side[3],float out[3]) {
    float begin[3],end[3],edge[3],toward[3],q;u32 i;
    for(i=0;i<3;i++) {
        if(caps_bits(side[0])&0x80000000U){begin[i]=0.f;end[i]=caps_add(p,e2[i],0.f);}
        else if(caps_bits(side[1])&0x80000000U){begin[i]=caps_add(p,e3[i],0.f);end[i]=0.f;}
        else{begin[i]=caps_add(p,e2[i],0.f);end[i]=caps_add(p,e3[i],0.f);}
    }
    caps_difference(p,point,begin,toward);caps_difference(p,end,begin,edge);
    q=caps_div(p,caps_dot(p,toward,edge),caps_dot(p,edge,edge));q=caps_min(caps_max(q,0.f),1.f);
    caps_projection(p,begin,edge,q,out);
}
/* Complete original plane/vertical-segment/edge kernel53FF48..5402EC. */
static int caps_triangle(CapsuleWork *p,const float vertices[3][3],const float center[3],const float origin[3],float height,u32 flags,u32 surface,CapsuleWinner *winner) {
    float e2[3],e3[3],n[3],bottom[3],top[3],reference[3],projection[3],normalpart[3],side[3],d0,d1,q,distance,difference,norm2;
    u32 i,raw;
    if((flags&0x90U)&surface)return 0;
    if((flags&0x20U)&&((flags>>8)^surface)%16U==0)return 0;
    caps_difference(p,vertices[1],vertices[0],e2);caps_difference(p,vertices[2],vertices[0],e3);
    caps_difference(p,center,vertices[0],bottom);memcpy(top,bottom,sizeof(top));top[2]=caps_add(p,top[2],height);
    caps_cross(p,e3,e2,n);d0=caps_dot(p,bottom,n);d1=caps_dot(p,top,n);
    if(!(flags&0x1000U)&&((caps_bits(d0)&caps_bits(d1))&0x80000000U))return 0;
    if(fabsf(n[2])<1.f) {
        reference[0]=bottom[0];reference[1]=bottom[1];
        reference[2]=caps_add(p,caps_max(caps_min(caps_sub(p,0.f,bottom[2]),height),0.f),bottom[2]);
    } else {
        q=caps_div(p,d0,caps_mul(p,height,n[2]));
        raw=caps_bits(bottom[2])|caps_bits(caps_sub(p,bottom[2],e2[2]))|caps_bits(caps_sub(p,bottom[2],e3[2]));
        if(!(raw&0x80000000U))memcpy(reference,bottom,sizeof(reference));
        else {
            raw=caps_bits(top[2])&caps_bits(caps_sub(p,top[2],e2[2]))&caps_bits(caps_sub(p,top[2],e3[2]));
            if(raw&0x80000000U)memcpy(reference,top,sizeof(reference));
            else {
                memcpy(projection,bottom,sizeof(projection));projection[2]=caps_sub(p,bottom[2],caps_mul(p,height,q));
                caps_sides(p,projection,e2,e3,n,side);
                if((caps_bits(side[0])|caps_bits(side[1])|caps_bits(side[2]))&0x80000000U)caps_edge(p,projection,e2,e3,side,projection);
                reference[0]=bottom[0];reference[1]=bottom[1];
                reference[2]=caps_add(p,caps_max(caps_min(caps_sub(p,projection[2],bottom[2]),height),0.f),bottom[2]);
            }
        }
    }
    norm2=caps_dot(p,n,n);q=caps_div(p,caps_dot(p,reference,n),norm2);
    for(i=0;i<3;i++)normalpart[i]=caps_mul(p,n[i],q);
    caps_difference(p,reference,normalpart,projection);distance=caps_dot(p,normalpart,normalpart);
    difference=caps_sub(p,winner->best,distance);if(caps_bits(difference)&0x80000000U)return 0;
    caps_sides(p,projection,e2,e3,n,side);
    if((caps_bits(side[0])|caps_bits(side[1])|caps_bits(side[2]))&0x80000000U) {
        caps_edge(p,projection,e2,e3,side,projection);caps_difference(p,projection,reference,normalpart);
        distance=caps_dot(p,normalpart,normalpart);difference=caps_sub(p,winner->best,distance);
        if(caps_bits(difference)&0x80000000U)return 0;
    }
    if(p->failed)return 0;
    winner->best=caps_mul(p,distance,caps_float(0x3F7FDF3BU));winner->metadata=surface+0x1000U;winner->have=1;
    for(i=0;i<3;i++) {
        float first=caps_add(p,vertices[0][i],origin[i]);
        winner->position[i]=caps_add(p,projection[i],first);winner->normal[i]=caps_add(p,n[i],0.f);
        winner->vertex[0][i]=caps_add(p,0.f,first);
        winner->vertex[1][i]=caps_add(p,e2[i],first);winner->vertex[2][i]=caps_add(p,e3[i],first);
    }
    return 1;
}
static int caps_range(CapsuleWork *p,u32 address,u32 bytes,u32 alignment) {
    if(!bytes||address%alignment||address<p->r.guest_base||(u64)address+bytes>0x80000000ULL||
       (u64)address+bytes>(u64)p->r.guest_base+p->r.storage_bytes){p->failed=DREADZONE_CAPSULE_INVALID;return 0;}
    return 1;
}
static u32 caps_read(CapsuleWork *p,u32 address,u32 bytes) {
    u32 v=0;if(caps_range(p,address,bytes,bytes))memcpy(&v,p->memory+address-p->r.guest_base,bytes);return v;
}
static void caps_store(CapsuleWork *p,u32 address,u32 value) {
    if(!p->failed&&caps_range(p,address,4,4)){memcpy(p->memory+address-p->r.guest_base,&value,4);p->info.write_words++;}
}
static float caps_load_float(CapsuleWork *p,u32 address) {return caps_check(p,caps_float(caps_read(p,address,4)));}
static void caps_quad(CapsuleWork *p,u32 address,const float xyz[3],u32 w) {
    u32 i;for(i=0;i<3;i++)caps_store(p,address+4*i,caps_bits(xyz[i]));caps_store(p,address+12,w);
}
static int caps_fetch(CapsuleWork *p,u32 root,u32 leaf,CapsuleBlock *b) {
    u64 address=(u64)root+(leaf>>8);u32 bytes=(leaf&255U)*16;
    if(!bytes||address>=0x80000000ULL){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
    if(!caps_range(p,(u32)address,bytes,16))return 0;
    memset(b->data,0,8);memcpy(b->data,p->memory+(u32)address-p->r.guest_base,bytes);b->bytes=bytes;
    p->info.dma_memory_copies++;return 1;
}
static s32 caps_signed(u32 v,u32 width) {u32 mask=(1U<<width)-1,sign=1U<<(width-1);return (s32)(v&mask)-(s32)((v&sign)*2);}
static u32 caps_clip(s32 value,s32 low,s32 high) {return ((u16)((u32)value-(u32)low)>>15)|(((u16)((u32)high-(u32)value)>>15)<<1);}

static int caps_cells(CapsuleWork *p,const float center[3],const float low[3],const float high[3],
                      float radius,float height,u32 root,CapsuleCell cells[127],u32 *count,CapsuleBlock *first) {
    DreadzoneCollisionTreeRequest tree;u32 begin[3],end[3],x,y,z,i;
    float base[3],current[3],radius2=caps_mul(p,radius,radius),height_end=caps_add(p,center[2],height);
    for(i=0;i<3;i++){begin[i]=(u32)(s32)low[i]>>2;end[i]=(u32)(s32)high[i]>>2;base[i]=(float)(begin[i]*4096U);}
    tree.storage=p->memory;tree.guest_base=p->r.guest_base;tree.bytes=p->r.storage_bytes;tree.root=root;
    *count=0;current[2]=base[2];
    for(z=begin[2];z<=end[2];z++,current[2]=caps_add(p,current[2],4096.f)) {
        current[1]=base[1];
        for(y=begin[1];y<=end[1];y++,current[1]=caps_add(p,current[1],4096.f)) {
            current[0]=base[0];
            for(x=begin[0];x<=end[0];x++,current[0]=caps_add(p,current[0],4096.f)) {
                float square[3],distance;u32 leaf;
                for(i=0;i<3;i++) {
                    float closest=caps_min(caps_max(current[i],center[i]),caps_add(p,current[i],4096.f));
                    float on_segment=i==2?caps_max(center[i],caps_min(height_end,closest)):caps_add(p,center[i],0.f);
                    float delta=caps_sub(p,closest,on_segment);square[i]=caps_mul(p,delta,delta);
                }
                distance=caps_add(p,caps_add(p,caps_add(p,caps_sub(p,0.f,radius2),square[0]),caps_mul(p,1.f,square[1])),caps_mul(p,1.f,square[2]));
                if(p->failed)return 0;
                if(!(caps_bits(distance)&0x80000000U))continue;
                tree.x=(s32)y;tree.y=(s32)z;tree.z=(s32)x;p->info.tree_calls++;
                if(dreadzone_collision_tree_lookup(&tree,&leaf)!=DREADZONE_TREE_READ){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
                if(!leaf)continue;
                if(*count==127){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
                cells[*count].leaf=leaf;for(i=0;i<3;i++)cells[*count].origin[i]=caps_add(p,current[i],2048.f);
                if(*count==0&&!caps_fetch(p,root,leaf,first))return 0;
                (*count)++;
            }
        }
    }
    p->info.selected_cells=*count;return !p->failed;
}
static int caps_decode(CapsuleWork *p,const CapsuleCell *cell,const CapsuleBlock *block,
                       const float center[3],const float low[3],const float high[3],float height,CapsuleWinner *winner) {
    float relative[3],vertex[256][3];s32 lo[3],hi[3];u32 clip[256],nv=block->data[2],nf=block->data[0],nq=block->data[3];
    u32 i,j,face_start=4+nv*4,pass;
    p->info.cells_processed++;if(!nv){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
    if(nq>nf||face_start+nf*4+nq>block->bytes){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
    for(i=0;i<3;i++) {
        float a=caps_sub(p,low[i],cell->origin[i]),b=caps_sub(p,high[i],cell->origin[i]);
        if(!caps_valid(a)||!caps_valid(b)||fabsf(a)>=2147483648.f||fabsf(b)>=2147483648.f){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
        lo[i]=(s32)a;hi[i]=(s32)b;relative[i]=caps_sub(p,center[i],cell->origin[i]);
    }
    for(i=0;i<((nv+1)&~1U);i++) {
        u32 word;memcpy(&word,block->data+4+i*4,4);clip[i]=0;
        vertex[i][0]=(float)(caps_signed(word,10)*64);vertex[i][1]=(float)(caps_signed(word>>10,10)*64);
        vertex[i][2]=(float)(caps_signed(word>>20,12)*16);
        for(j=0;j<3;j++)clip[i]|=caps_clip((s32)vertex[i][j],lo[j],hi[j])<<(j*8);
    }
    /* Original next-pair preloads stay inside the full scratch bank, including
     * untouched tail bytes. XYZ-only VITOF leaves the final VF2 W at zero. */
    winner->vf2_w=0;
    for(pass=0;pass<2;pass++) {
        u32 face_count=pass?nq:nf;
        for(i=0;i<face_count;i++) {
            const u8 *source=block->data+face_start+i*4;u8 face[4];float triangle[3][3];u32 common;
            memcpy(face,source,4);
            /* This is the actual original scratch quad mutation's effect. */
            if(pass){face[1]=source[2];face[2]=block->data[face_start+nf*4+i];}
            for(j=0;j<3;j++)if(face[j]>=nv){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
            common=clip[face[0]]&clip[face[1]]&clip[face[2]];p->info.faces_processed++;if(pass)p->info.quad_faces_processed++;
            if((s32)common>0)continue;
            /* VF2 is loaded before both face/surface filters, even on rejection. */
            winner->vf2_w=clip[face[1]];
            for(j=0;j<3;j++)memcpy(triangle[j],vertex[face[j]],sizeof(triangle[j]));
            caps_triangle(p,triangle,relative,cell->origin,height,p->r.caller_flags^0x10U,face[3],winner);
            if(p->failed)return 0;
        }
    }
    return 1;
}
static int caps_dynamic(CapsuleWork *p,const float center[3],const float low[3],const float high[3],float radius,float height,u32 generation) {
    u32 x,y,begin[2],end[2],base,flags=p->r.caller_flags^0x10U;
    struct {u32 begin,count;} lists[255];u32 list_count=0,k;
    if(!p->r.actor_pool_count||p->r.actor_pool_count>65536){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
    base=caps_read(p,p->r.actor_pool_pointer_guest,4);
    for(x=0;x<2;x++){begin[x]=(u32)(s32)low[x]>>14;end[x]=(u32)(s32)high[x]>>14;if(end[x]>63){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}}
    if((end[0]-begin[0]+1)*(end[1]-begin[1]+1)>255){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
    for(y=begin[1];y<=end[1];y++)for(x=begin[0];x<=end[0];x++) {
        u64 c=(u64)p->r.grid_guest+y*256U+x*4U;u32 first,count;
        if(c>=0x80000000ULL){p->failed=DREADZONE_CAPSULE_INVALID;return 0;}
        first=caps_read(p,(u32)c,2);count=caps_read(p,(u32)c+2,1);
        if(first>=1536||first*32+count*2>0xC000U){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
        lists[list_count].begin=first;lists[list_count++].count=count;
    }
    /* Actual scratch records capture all cell headers before visitation writes. */
    for(k=0;k<list_count;k++) {
        u32 first=lists[k].begin,count=lists[k].count,i;
        for(i=0;i<=count;i++) {
            u64 link=(u64)p->r.grid_guest+0x4000U+first*32U+i*2U,actor;u32 index,definition,visited,type,model;
            float bounds[4],delta[2],total,zlow,zhigh,broad;u32 j;
            if(link>=0x80000000ULL){p->failed=DREADZONE_CAPSULE_INVALID;return 0;}
            index=caps_read(p,(u32)link,2);if(p->failed)return 0;if(i==count)break;
            if(index>=p->r.actor_pool_count){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
            actor=(u64)base+index*256U;if(actor>=0x80000000ULL||!caps_range(p,(u32)actor,256,16))return 0;
            /* The original quad read precedes the ignored-actor branch. */
            for(j=0;j<4;j++)bounds[j]=caps_float(caps_read(p,(u32)actor+j*4,4));p->info.actor_candidates++;
            if((u32)actor==p->r.ignored_actor_guest){p->info.actors_ignored++;continue;}
            for(j=0;j<4;j++)if(!caps_valid(bounds[j])){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
            total=caps_add(p,radius,bounds[3]);
            for(j=0;j<2;j++){delta[j]=caps_sub(p,bounds[j],center[j]);delta[j]=caps_mul(p,delta[j],delta[j]);}
            zlow=caps_sub(p,caps_sub(p,caps_sub(p,bounds[2],total),center[2]),height);
            zhigh=caps_sub(p,caps_add(p,bounds[2],total),center[2]);
            broad=caps_add(p,caps_sub(p,delta[0],caps_mul(p,total,total)),caps_mul(p,1.f,delta[1]));
            definition=caps_read(p,(u32)actor+0x98,4);
            if((u64)definition+((flags&4U)?0U:2U)>0x7FFFFFFFULL){p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;}
            visited=caps_read(p,(u32)actor+0xA0,4);
            if(visited==generation||!definition||(!signbit(broad)&&broad!=0.f)||!(zhigh>0.f)||!signbit(zlow)){p->info.actors_filtered++;continue;}
            if((u64)definition+16>0x80000000ULL){p->failed=DREADZONE_CAPSULE_INVALID;return 0;}
            type=caps_read(p,definition+12,4);
            caps_store(p,(u32)actor+0xA0,generation);p->info.actors_marked++;
            model=caps_read(p,(u32)actor+0x58,4);(void)model;
            if(type==((flags&2U)?0U:1U)){p->info.actors_filtered++;continue;}
            /* These quads are genuinely read before the geometry/primitive
             * branches. Preserve the memory contract without inventing poses. */
            if(!caps_range(p,(u32)actor+0x10,16,16)||!caps_range(p,(u32)actor+0xC0,48,16)||!caps_range(p,(u32)actor+0x70,16,16))return 0;
            type=caps_read(p,definition+12,4);
            visited=caps_read(p,(u32)actor+0x9C,4);
            for(j=0;j<3;j++)(void)caps_mul(p,caps_load_float(p,(u32)actor+0x10+j*4),1024.f);
            if(type&&(s32)visited>=0) {
                if(flags&0x2000U){p->info.actors_filtered++;continue;}
            } else {
                u32 primitives=caps_read(p,definition+4,4);
                if((flags&2U)||!primitives){p->info.actors_filtered++;continue;}
            }
            p->failed=DREADZONE_CAPSULE_UNSUPPORTED;return 0;
        }
    }
    return !p->failed;
}
static int caps_calculate(CapsuleWork *p) {
    float world[3],center[3],low_world[3],high_world[3],low[3],high[3],radius,height;
    CapsuleCell cells[127];CapsuleBlock buffers[2];CapsuleWinner winner;u32 generation,root,count=0,i,j;
    memset(&winner,0,sizeof(winner));memset(buffers,0xA5,sizeof(buffers));
    if(!caps_range(p,p->r.center_guest,16,16)||!caps_range(p,p->r.packet_guest,128,16))return p->failed;
    if(!caps_valid(p->r.radius)||!caps_valid(p->r.height)||p->r.radius<=0.f||p->r.height<=0.f)return DREADZONE_CAPSULE_UNSUPPORTED;
    for(i=0;i<3;i++) {
        world[i]=caps_load_float(p,p->r.center_guest+i*4);center[i]=caps_mul(p,world[i],1024.f);
        low_world[i]=caps_sub(p,world[i],p->r.radius);high_world[i]=caps_add(p,world[i],p->r.radius);
        if(i==2)high_world[i]=caps_add(p,high_world[i],p->r.height);
        if(!(low_world[i]>0.f)||!(high_world[i]<1024.f))return DREADZONE_CAPSULE_UNSUPPORTED;
        low[i]=caps_mul(p,low_world[i],1024.f);high[i]=caps_mul(p,high_world[i],1024.f);
    }
    radius=caps_mul(p,p->r.radius,1024.f);height=caps_mul(p,p->r.height,1024.f);
    generation=caps_read(p,p->r.packet_guest+16,4);
    if((s32)generation<0)return 0;
    if(generation==0x7FFFFFFFU)return DREADZONE_CAPSULE_UNSUPPORTED;
    generation++;caps_store(p,p->r.packet_guest+16,generation);
    winner.best=caps_mul(p,caps_mul(p,radius,radius),caps_float(0x3F7FDF3BU));
    root=caps_read(p,p->r.packet_guest,4);
    if(!(p->r.caller_flags&1U)) {
        if(!caps_cells(p,center,low_world,high_world,radius,height,root,cells,&count,buffers))return p->failed;
        for(i=0;i<count;i++) {
            if(i+1<count&&!caps_fetch(p,root,cells[i+1].leaf,buffers+((i+1)&1U)))return p->failed;
            if(!caps_decode(p,cells+i,buffers+(i&1U),center,low,high,height,&winner))return p->failed;
        }
    }
    if(!caps_dynamic(p,center,low,high,radius,height,generation))return p->failed;
    if(winner.have) {
        float position[3],adjusted[3],q=caps_div(p,radius,caps_check(p,sqrtf(fabsf(winner.best))));
        for(i=0;i<3;i++)position[i]=caps_mul(p,winner.position[i],1.f/1024.f);
        caps_quad(p,p->r.packet_guest+32,position,p->r.incoming_vf25_29_w[0]);
        caps_quad(p,p->r.packet_guest+64,winner.normal,p->r.incoming_vf25_29_w[1]);
        for(i=0;i<3;i++){float v[3];for(j=0;j<3;j++)v[j]=caps_mul(p,winner.vertex[i][j],1.f/1024.f);caps_quad(p,p->r.packet_guest+80+i*16,v,p->r.incoming_vf25_29_w[i+2]);}
        caps_store(p,p->r.packet_guest+24,0);caps_store(p,p->r.packet_guest+28,winner.metadata);
        if(winner.best<0.125f) {
            memcpy(adjusted,position,sizeof(adjusted));
            if(caps_bits(winner.normal[2])&0x80000000U)adjusted[2]=caps_sub(p,caps_sub(p,adjusted[2],p->r.radius),p->r.height);
            else adjusted[2]=caps_add(p,adjusted[2],p->r.radius);
        } else {
            float dz=caps_max(caps_min(caps_sub(p,winner.position[2],center[2]),height),0.f);
            for(i=0;i<3;i++) {
                float delta=caps_sub(p,center[i],winner.position[i]);if(i==2)delta=caps_add(p,delta,dz);
                delta=caps_mul(p,delta,q);if(i==2)delta=caps_sub(p,delta,dz);
                adjusted[i]=caps_add(p,caps_mul(p,delta,1.f/1024.f),position[i]);
            }
        }
        caps_quad(p,p->r.packet_guest+48,adjusted,winner.vf2_w);
    }
    return p->failed?p->failed:winner.have;
}
int dreadzone_capsule_contact(const DreadzoneCapsuleRequest *request,DreadzoneCapsuleInfo *info) {
    CapsuleWork p;uintptr_t a,b,c;int result;
    if(!request||!info)return DREADZONE_CAPSULE_INVALID;
    a=(uintptr_t)request;b=(uintptr_t)info;
    if(a>UINTPTR_MAX-sizeof(*request)||b>UINTPTR_MAX-sizeof(*info)||(a<b+sizeof(*info)&&b<a+sizeof(*request)))return DREADZONE_CAPSULE_INVALID;
    memset(&p,0,sizeof(p));memcpy(&p.r,request,sizeof(p.r));c=(uintptr_t)p.r.storage;
    if(!p.r.storage||!p.r.storage_bytes||p.r.storage_bytes>0x80000000U||c>UINTPTR_MAX-p.r.storage_bytes||
       (u64)p.r.guest_base+p.r.storage_bytes>0x80000000ULL||
       (p.r.guest_base<=0x70003FFFU&&(u64)p.r.guest_base+p.r.storage_bytes>0x70000000ULL)||
       (b<c+p.r.storage_bytes&&c<b+sizeof(*info)))return DREADZONE_CAPSULE_INVALID;
    p.memory=(u8 *)malloc(p.r.storage_bytes);if(!p.memory)return DREADZONE_CAPSULE_UNSUPPORTED;
    memcpy(p.memory,p.r.storage,p.r.storage_bytes);result=caps_calculate(&p);if(p.failed)result=p.failed;
    if(result>=0){memcpy(p.r.storage,p.memory,p.r.storage_bytes);memcpy(info,&p.info,sizeof(*info));}
    free(p.memory);return result;
}
