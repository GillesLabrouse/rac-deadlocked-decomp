#include "dreadzone_wall_contact.h"
#include "dreadzone_collision_grid.h"
#include "dreadzone_terrain_contact.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct WallWork {
    DreadzoneWallRequest r;
    DreadzoneWallInfo info;
    u8 *memory;
    int failed;
    u32 endpoint_words[2][4];
    int endpoints_local;
} WallWork;
static u32 wall_bits(float value){u32 word;memcpy(&word,&value,4);return word;}
static float wall_float(u32 word){float value;memcpy(&value,&word,4);return value;}
static int wall_valid(float v){return isfinite(v)&&(v==0.f||fabsf(v)>=FLT_MIN);}
static float wall_check(WallWork *p,float v){if(!wall_valid(v))p->failed=DREADZONE_WALL_UNSUPPORTED;return v;}
static float wall_add(WallWork *p,float a,float b){volatile float v=a+b;return wall_check(p,v);}
static float wall_sub(WallWork *p,float a,float b){volatile float v=a-b;return wall_check(p,v);}
static float wall_mul(WallWork *p,float a,float b){volatile float v=a*b;return wall_check(p,v);}
static float wall_div(WallWork *p,float a,float b){
    float q;
    if(b==0.f){q=wall_float(0x7F7FFFFFU|((wall_bits(a)^wall_bits(b))&0x80000000U));p->info.q_exception_bits|=a==0.f?16U:32U;}
    else{volatile float v=a/b;q=wall_check(p,v);}
    p->info.q_bits=wall_bits(q);return q;
}
static float wall_dot(WallWork *p,const float a[3],const float b[3]){
    float x=wall_mul(p,a[0],b[0]),y=wall_mul(p,a[1],b[1]),z=wall_mul(p,a[2],b[2]);
    return wall_add(p,wall_add(p,x,y),wall_mul(p,1.f,z));
}
static int wall_range(WallWork *p,u32 address,u32 bytes,u32 alignment){
    if(address%alignment||address<p->r.guest_base||(u64)address+bytes>(u64)p->r.guest_base+p->r.storage_bytes){p->failed=DREADZONE_WALL_INVALID;return 0;}return 1;
}
static u32 wall_read(WallWork *p,u32 address,u32 bytes){u32 value=0;if(wall_range(p,address,bytes,bytes))memcpy(&value,p->memory+address-p->r.guest_base,bytes);return value;}
static void wall_store(WallWork *p,u32 address,u32 value){if(wall_range(p,address,4,4)){memcpy(p->memory+address-p->r.guest_base,&value,4);p->info.write_words++;}}
static float wall_load(WallWork *p,u32 address){return wall_check(p,wall_float(wall_read(p,address,4)));}
static void wall_quad(WallWork *p,u32 address,const float xyz[3],u32 w){u32 i;if(!wall_range(p,address,16,16))return;for(i=0;i<3;i++)wall_store(p,address+4*i,wall_bits(xyz[i]));wall_store(p,address+12,w);}
static int wall_add_address(WallWork *p,u32 base,u32 offset,u32 *result){u64 value=(u64)base+offset;if(value>0x7FFFFFFFULL){p->failed=DREADZONE_WALL_UNSUPPORTED;return 0;}*result=(u32)value;return 1;}

/* Complete reached noneligible primitive-header path0053E760..E79C.
 * Definition+4 is only a nonzero gate, not a record count. The previous
 * signed flag terminates on the next iteration; a selected current negative
 * flag still reaches real geometry and must refuse. No shape miss is invented.
 */
static int wall_primitive_headers(WallWork *p,u32 definition,u32 actor,u32 flags){
    u32 cursor,disabled,previous=0,headers=0,category=(flags&4U)?1U:2U;
    if(!wall_add_address(p,definition,16,&cursor))return 0;
    disabled=wall_read(p,actor+0x9C,4);if(p->failed)return 0;
    while(!(previous&0x8000U)){
        u32 field,next,kind,current,skip;
        if(headers++>=DREADZONE_WALL_MAX_PRIMITIVE_HEADERS){p->failed=DREADZONE_WALL_UNSUPPORTED;return 0;}
        skip=disabled&1U;disabled>>=1;
        if(!wall_add_address(p,cursor,2,&field))return 0;
        current=wall_read(p,field,2);kind=wall_read(p,cursor,1);(void)kind;
        if(p->failed||!wall_add_address(p,cursor,32,&next))return 0;
        cursor=next;previous=current;
        if(skip||!(current&category))continue;
        p->failed=DREADZONE_WALL_UNSUPPORTED;return 0;
    }
    return 1;
}

static int wall_dynamic(WallWork *p,const DreadzoneCollisionGridRecord *records,u32 count,
                        const float start[3],const float direction[3],float best,u32 generation){
    u32 coarse[DREADZONE_COLLISION_GRID_MAX_RECORDS],n=0,i,j,flags=p->r.caller_flags^16U,pool;
    float normalized[3],q=wall_div(p,1.f,wall_dot(p,direction,direction));
    for(i=0;i<3;i++)normalized[i]=wall_mul(p,direction[i],q);
    pool=wall_read(p,p->r.actor_pool_pointer_guest,4);
    for(i=0;i<count;i++){
        u32 cell,x,y;
        if(records[i].fraction_bits==0x3F800000U)break;
        cell=records[i].packed_cell;x=(cell&255U)>>2;y=((cell>>8)&255U)>>2;
        cell=x|(y<<16);if(!n||coarse[n-1]!=cell)coarse[n++]=cell;
    }
    p->info.dynamic_cells=n;
    for(i=0;i<n;i++){
        u32 cell,first,entries,list,end,index,actor;
        if(!wall_add_address(p,p->r.grid_guest,(coarse[i]>>8)+(coarse[i]&65535U)*4,&cell))return 0;
        first=wall_read(p,cell,2);entries=wall_read(p,cell+2,1);
        if(first>=p->r.grid_block_count||entries*2U>(p->r.grid_block_count-first)*32U){p->failed=DREADZONE_WALL_UNSUPPORTED;return 0;}
        if(!wall_add_address(p,p->r.grid_guest,0x4000U+first*32U,&list)||!wall_add_address(p,list,entries*2U,&end))return 0;
        /* Original overwrites scratch cell with end, then preloads the next
         * actor index even for an empty list or after the last candidate. */
        for(;;list+=2){
            float position[3],toward[3],projection[3],product[3],t,broad,radius;
            u32 definition,visited,type,mark,check,model;
            index=wall_read(p,list,2);
            if(!wall_add_address(p,pool,index*256U,&actor))return 0;
            if(list==end)break;
            if(index>=p->r.actor_pool_count){p->failed=DREADZONE_WALL_UNSUPPORTED;return 0;}
            if(!wall_range(p,actor,16,16))return 0;
            for(j=0;j<3;j++)position[j]=wall_load(p,actor+j*4);
            radius=wall_load(p,actor+12);p->info.actor_candidates++;
            if(actor==p->r.ignored_actor_guest){p->info.actors_ignored++;continue;}
            for(j=0;j<3;j++)toward[j]=wall_sub(p,position[j],start[j]);
            radius=wall_mul(p,radius,radius);mark=wall_read(p,actor+0x34,2);
            definition=wall_read(p,actor+0x98,4);
            if((flags&9U)&&!(mark&0x4000U)){p->info.actors_filtered++;continue;}
            t=wall_dot(p,toward,normalized);if(t<0.f)t=0.f;if(t>best)t=best;
            for(j=0;j<3;j++){
                projection[j]=wall_add(p,wall_add(p,0.f,start[j]),wall_mul(p,direction[j],t));
                toward[j]=wall_sub(p,projection[j],position[j]);product[j]=wall_mul(p,toward[j],toward[j]);
            }
            broad=wall_add(p,wall_add(p,wall_sub(p,product[0],radius),wall_mul(p,1.f,product[1])),wall_mul(p,1.f,product[2]));
            if(!wall_add_address(p,definition,(flags&4U)?0U:2U,&check))return 0;
            (void)check;
            if(!definition){p->info.actors_filtered++;continue;}
            visited=wall_read(p,actor+0xA0,4);
            if(!(wall_bits(broad)&0x80000000U)&&wall_bits(broad)!=0){p->info.actors_filtered++;continue;}
            wall_store(p,actor+0xA0,generation);p->info.actors_marked++;
            /* The real store precedes both the comparison and delay-slot
             * definition+C reload, including mapped aliases and visited actors. */
            if(!wall_add_address(p,definition,12,&check))return 0;
            type=wall_read(p,check,4);
            if(visited==generation){p->info.actors_filtered++;continue;}
            model=wall_read(p,actor+0x58,4);(void)model;
            if(type==((flags&2U)?0U:1U)){p->info.actors_filtered++;continue;}
            if(!wall_range(p,actor+0x10,16,16)||!wall_range(p,actor+0xC0,48,16)||!wall_range(p,actor+0x70,16,16))return 0;
            type=wall_read(p,check,4);visited=wall_read(p,actor+0x9C,4);
            for(j=0;j<3;j++)(void)wall_mul(p,wall_load(p,actor+0x10+j*4),1024.f);
            if(type&&(s32)visited>=0){if(flags&0x2000U){p->info.actors_filtered++;continue;}}
            else{
                if(!wall_add_address(p,definition,4,&check))return 0;
                type=wall_read(p,check,4);
                if((flags&2U)||!type){p->info.actors_filtered++;continue;}
                if(!wall_primitive_headers(p,definition,actor,flags))return 0;
                p->info.actors_filtered++;continue;
            }
            p->failed=DREADZONE_WALL_UNSUPPORTED;return 0;
        }
    }
    return !p->failed;
}
static int wall_calculate(WallWork *p){
    float start[3],end[3],scaled_start[3],direction[3];
    DreadzoneCollisionGridRequest grid;DreadzoneCollisionGridInfo grid_info;
    DreadzoneCollisionGridRecord records[DREADZONE_COLLISION_GRID_MAX_RECORDS];
    DreadzoneTerrainContactRequest terrain;DreadzoneTerrainContactInfo terrain_info;
    DreadzoneCollisionTriangleContact contact;u32 i,j,generation,root;int result;
    if((p->r.caller_flags!=2U&&p->r.caller_flags!=0x24U&&p->r.caller_flags!=4U&&p->r.caller_flags!=0x22U)||p->r.cache_guest)return DREADZONE_WALL_UNSUPPORTED;
    if(!p->endpoints_local&&(!wall_range(p,p->r.start_guest,16,16)||!wall_range(p,p->r.end_guest,16,16)))return p->failed;
    if(!wall_range(p,p->r.packet_guest,128,16))return p->failed;
    for(i=0;i<3;i++){
        start[i]=p->endpoints_local?wall_check(p,wall_float(p->endpoint_words[0][i])):wall_load(p,p->r.start_guest+4*i);
        end[i]=p->endpoints_local?wall_check(p,wall_float(p->endpoint_words[1][i])):wall_load(p,p->r.end_guest+4*i);
    }
    generation=wall_read(p,p->r.packet_guest+16,4);if((s32)generation<0)return 0;
    if(generation==0x7FFFFFFFU)return DREADZONE_WALL_UNSUPPORTED;
    generation++;wall_store(p,p->r.packet_guest+16,generation);
    for(i=0;i<3;i++){
        float minimum=start[i],maximum=start[i];
        /* Within the finite normal/zero domain, VMIN/VMAX distinguish
         * signed zero: the minimum is -0 if either operand is -0, while
         * the maximum is +0 if either operand is +0. */
        if(end[i]<minimum)minimum=end[i];
        if(end[i]>maximum)maximum=end[i];
        if(start[i]==0.f&&end[i]==0.f){
            minimum=wall_float((wall_bits(start[i])|wall_bits(end[i]))&0x80000000U);
            maximum=wall_float((wall_bits(start[i])&wall_bits(end[i]))&0x80000000U);
        }
        if(signbit(wall_sub(p,minimum,0.f))||!signbit(wall_sub(p,maximum,1024.f)))return 0;
        scaled_start[i]=grid.start_scaled[i]=wall_mul(p,start[i],1024.f);
        grid.end_scaled[i]=wall_mul(p,end[i],1024.f);direction[i]=wall_sub(p,grid.end_scaled[i],scaled_start[i]);
    }
    root=wall_read(p,p->r.packet_guest,4);
    if(dreadzone_collision_grid(&grid,records,DREADZONE_COLLISION_GRID_MAX_RECORDS,&grid_info)!=1)return DREADZONE_WALL_UNSUPPORTED;
    p->info.grid_records=grid_info.record_count;
    memset(&terrain,0,sizeof(terrain));terrain.storage=p->memory;terrain.guest_base=p->r.guest_base;terrain.bytes=p->r.storage_bytes;terrain.root=root;terrain.caller_flags=p->r.caller_flags;
    memcpy(terrain.start_world,start,sizeof(start));memcpy(terrain.end_world,end,sizeof(end));
    result=dreadzone_terrain_contact(&terrain,&contact,&terrain_info);if(result<0)return DREADZONE_WALL_UNSUPPORTED;
    /* The original first tree lookup and leaf fetch precede this exit.
     * Even identical integer endpoints must validate that actual read. */
    if(!grid_info.has_integer_displacement)return 0;
    p->info.terrain_cells=terrain_info.cells_processed;p->info.terrain_faces=terrain_info.faces_processed;p->info.terrain_quad_faces=terrain_info.quad_faces_processed;
    p->info.q_exception_bits=terrain_info.triangle_q_exceptions;
    if(grid_info.zero_q_division_mask)p->info.q_exception_bits|=32U;
    if(!wall_dynamic(p,records,grid_info.record_count,scaled_start,direction,result?contact.fraction:1.f,generation))return p->failed;
    if(result){
        float xyz[3];for(i=0;i<3;i++)xyz[i]=wall_mul(p,contact.position_scaled[i],1.f/1024.f);
        wall_quad(p,p->r.packet_guest+32,xyz,p->r.incoming_vf25_27_w[0]);
        wall_quad(p,p->r.packet_guest+64,contact.normal,p->r.incoming_vf25_27_w[1]);
        for(i=0;i<3;i++){for(j=0;j<3;j++)xyz[j]=wall_mul(p,contact.vertices_scaled[i][j],1.f/1024.f);wall_quad(p,p->r.packet_guest+80+i*16,xyz,i==0?p->r.incoming_vf25_27_w[2]:0U);}
        wall_store(p,p->r.packet_guest+24,0);wall_store(p,p->r.packet_guest+28,contact.hit_metadata);
    }
    return p->failed?p->failed:result;
}
static int wall_execute(const DreadzoneWallRequest *request,const u32 *start_words,const u32 *end_words,DreadzoneWallInfo *info){
    WallWork p;uintptr_t a,b,c;int result;
    if(!request||!info)return DREADZONE_WALL_INVALID;
    a=(uintptr_t)request;b=(uintptr_t)info;
    if(a>UINTPTR_MAX-sizeof(*request)||b>UINTPTR_MAX-sizeof(*info)||(a<b+sizeof(*info)&&b<a+sizeof(*request)))return DREADZONE_WALL_INVALID;
    memset(&p,0,sizeof(p));memcpy(&p.r,request,sizeof(p.r));c=(uintptr_t)p.r.storage;
    if(start_words){
        uintptr_t first=(uintptr_t)start_words,second=(uintptr_t)end_words;
        if(!end_words||first>UINTPTR_MAX-16U||second>UINTPTR_MAX-16U)return DREADZONE_WALL_INVALID;
        memcpy(p.endpoint_words[0],start_words,16);memcpy(p.endpoint_words[1],end_words,16);p.endpoints_local=1;
    }
    if(!p.r.storage||!p.r.storage_bytes||c>UINTPTR_MAX-p.r.storage_bytes||(u64)p.r.guest_base+p.r.storage_bytes>0x80000000ULL||
       (p.r.guest_base<=0x70003FFFU&&(u64)p.r.guest_base+p.r.storage_bytes>0x70000000ULL)||
       (b<c+p.r.storage_bytes&&c<b+sizeof(*info))||!p.r.actor_pool_count||p.r.actor_pool_count>65536U||!p.r.grid_block_count||p.r.grid_block_count>3072U)return DREADZONE_WALL_INVALID;
    p.memory=(u8 *)malloc(p.r.storage_bytes);if(!p.memory)return DREADZONE_WALL_UNSUPPORTED;
    memcpy(p.memory,p.r.storage,p.r.storage_bytes);result=wall_calculate(&p);if(p.failed)result=p.failed;
    if(result>=0){memcpy(p.r.storage,p.memory,p.r.storage_bytes);memcpy(info,&p.info,sizeof(*info));}
    free(p.memory);return result;
}
int dreadzone_wall_contact(const DreadzoneWallRequest *request,DreadzoneWallInfo *info){
    return wall_execute(request,NULL,NULL,info);
}
int dreadzone_wall_contact_vectors(const DreadzoneWallRequest *request,const u32 start_words[4],const u32 end_words[4],DreadzoneWallInfo *info){
    if(!start_words||!end_words)return DREADZONE_WALL_INVALID;
    return wall_execute(request,start_words,end_words,info);
}
