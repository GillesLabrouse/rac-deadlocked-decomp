/* Complete supported ordinary L01 00668070 RAM composition. Hardware bank,
 * register/stack ABI and selected startup/frame effects are separate contracts.
 * The outer clone retains atomicity across every genuine bounded component.
 */
#include "dreadzone_actor_constructor_internal.h"
#include "dreadzone_actor_ground_step.h"
#include "dreadzone_player_record_transform.h"
#include "dreadzone_actor_animation.h"
#include "dreadzone_camera_init.h"
#include "dreadzone_equipment.h"
#include "game_random.h"
#include <stdlib.h>
#include <string.h>

typedef struct ConstructorMemory {u8 *p;u32 base,n;int bad;} ConstructorMemory;
static u8 *map(ConstructorMemory *m,u32 a,u32 n) {
    if(a<m->base||n>m->n||a-m->base>m->n-n){m->bad=1;return NULL;}
    return m->p+a-m->base;
}
static u32 word(ConstructorMemory *m,u32 a){u32 w=0;u8 *p=map(m,a,4);if(p)memcpy(&w,p,4);return w;}
static u16 half(ConstructorMemory *m,u32 a){u16 w=0;u8 *p=map(m,a,2);if(p)memcpy(&w,p,2);return w;}
static u8 byte(ConstructorMemory *m,u32 a){u8 *p=map(m,a,1);return p?*p:0;}
static void put(ConstructorMemory *m,u32 a,u32 w){u8 *p=map(m,a,4);if(p)memcpy(p,&w,4);}
static void put_half(ConstructorMemory *m,u32 a,u16 w){u8 *p=map(m,a,2);if(p)memcpy(p,&w,2);}
static void put_byte(ConstructorMemory *m,u32 a,u8 w){u8 *p=map(m,a,1);if(p)*p=w;}
static void copy_quad(ConstructorMemory *m,u32 to,u32 from){u8 q[16],*p=map(m,from,16);if(p)memcpy(q,p,16);p=map(m,to,16);if(p&&!m->bad)memcpy(p,q,16);}
static void put_float(ConstructorMemory *m,u32 a,float f){u32 w;memcpy(&w,&f,4);put(m,a,w);}
/* Later ordinary branches require these reloaded selectors to stay in the
 * supported domain, including when model outputs alias guest globals. */
static int ordinary_domain(ConstructorMemory *m,u32 record) {
    return word(m,0x21E824)==0 && word(m,0x21E878)==0 &&
           word(m,record+0x2F10)==0 && !m->bad;
}
static int host_span(const void *p,uintptr_t n){return p&&(uintptr_t)p<=UINTPTR_MAX-n;}

typedef struct Exclusion {u32 a,n;} Exclusion;
static int inventory_views(ConstructorMemory *m,u32 record,DreadzoneEquipmentContext *ctx,
    DreadzoneInventorySetupReadMemory reads[16],DreadzoneEquipmentMemory mutable_views[4]) {
    Exclusion excluded[6]={{0x171B67,6},{0x1D3F90,0x12B0},{0x20C690,0x48},{0x220810,0x10},{0x3301E0,0x450},{0,0x31F0}};
    u32 i,j,cursor=m->base,count=0;Exclusion temporary;
    excluded[5].a=record;
    for(i=1;i<6;i++)for(j=i;j>0&&excluded[j].a<excluded[j-1].a;j--){temporary=excluded[j];excluded[j]=excluded[j-1];excluded[j-1]=temporary;}
    for(i=0;i<6;i++) {
        if(excluded[i].a<cursor||excluded[i].n>m->n||excluded[i].a-m->base>m->n-excluded[i].n)return -2;
        if(excluded[i].a>cursor){reads[count].storage=map(m,cursor,excluded[i].a-cursor);reads[count].guest_address=cursor;reads[count++].bytes=excluded[i].a-cursor;}
        cursor=excluded[i].a+excluded[i].n;
    }
    if(cursor-m->base<m->n){reads[count].storage=map(m,cursor,m->n-(cursor-m->base));reads[count].guest_address=cursor;reads[count++].bytes=m->n-(cursor-m->base);}
    memset(ctx,0,sizeof(*ctx));ctx->inventory.record=map(m,record,0x31F0);ctx->inventory.record_guest=record;ctx->inventory.record_bytes=0x31F0;
    ctx->inventory.pool.storage=map(m,0x1D3F90,0x12B0);ctx->inventory.pool.guest_address=0x1D3F90;ctx->inventory.pool.bytes=0x12B0;
    ctx->inventory.reads=reads;ctx->inventory.read_count=count;
    mutable_views[0].guest_address=0x171B67;mutable_views[0].bytes=6;
    mutable_views[1].guest_address=0x20C690;mutable_views[1].bytes=0x48;
    mutable_views[2].guest_address=0x220810;mutable_views[2].bytes=0x10;
    mutable_views[3].guest_address=0x3301E0;mutable_views[3].bytes=0x450;
    for(i=0;i<4;i++)mutable_views[i].storage=map(m,mutable_views[i].guest_address,mutable_views[i].bytes);
    ctx->mutable_views=mutable_views;ctx->mutable_count=4;return m->bad?-2:1;
}

static int constructor_complete(u8 *storage,u32 base,u32 bytes,u32 record,u32 actors,u32 blocks,
    const u32 incoming_w[3],const u32 side_w[2],const u8 initial_scratch[0x4000],const u8 initial_vu[0x1000]) {
    ConstructorMemory m;u8 scratch[0x4000],vu[0x1000];u32 w[3],side[2],i;int status=-2,selected;
    DreadzoneGroundRequest ground={0};DreadzoneGroundInfo ground_info;
    DreadzonePlayerRecordTransform transform;DreadzoneAnimationContext animation;
    DreadzoneAnimationMemory animation_views[2];DeadlockedRandomState random;s32 random_value;
    DreadzoneCameraView camera_view;DreadzoneCameraInit camera;
    DreadzoneEquipmentContext equipment;DreadzoneInventorySetupReadMemory reads[16];DreadzoneEquipmentMemory mutable_views[4];
    if(!bytes||bytes>16U*1024U*1024U||!host_span(storage,bytes)||!host_span(incoming_w,12)||!host_span(side_w,8)||
       !host_span(initial_scratch,sizeof(scratch))||!host_span(initial_vu,sizeof(vu))||(u64)base+bytes>0x70000000ULL||
       (record&15U)||record>0x7FFFFFFFU-0x31F0U)return -1;
    memcpy(w,incoming_w,12);memcpy(side,side_w,8);memcpy(scratch,initial_scratch,sizeof(scratch));memcpy(vu,initial_vu,sizeof(vu));
    m.base=base;m.n=bytes;m.bad=0;m.p=(u8 *)malloc(bytes);if(!m.p)return -3;memcpy(m.p,storage,bytes);
    if(word(&m,0x21E824)!=0||m.bad)goto done;
    if(dreadzone_constructor_prefix(m.p,base,bytes,record,actors,blocks,w)!=1)goto done;
    ground.storage=m.p;ground.guest_base=base;ground.storage_bytes=bytes;ground.record_guest=record;
    ground.operation=DREADZONE_GROUND_UPDATE;ground.integer0=1;ground.actor_pool_count=actors;ground.grid_block_count=blocks;
    memcpy(ground.incoming_vf25_29_w,w,12);memcpy(ground.incoming_side_output_w,side,8);
    if(dreadzone_actor_ground_step(&ground,&ground_info)!=1)goto done;
    transform.storage=m.p;transform.guest_base=base;transform.bytes=bytes;transform.record=record;transform.actor_pool_count=actors;
    transform.scratch=scratch;transform.scratch_bytes=sizeof(scratch);transform.vu_data=vu;transform.vu_data_bytes=sizeof(vu);
    if(dreadzone_player_record_transform(&transform)!=1 || !ordinary_domain(&m,record))goto done;
    animation.record=map(&m,record,0x31F0);animation.record_guest=record;animation.record_bytes=0x31F0;
    animation_views[0].storage=m.p;animation_views[0].guest_base=base;animation_views[0].bytes=record-base;
    animation_views[1].storage=map(&m,record+0x31F0,bytes-(record-base)-0x31F0);animation_views[1].guest_base=record+0x31F0;
    animation_views[1].bytes=bytes-(record-base)-0x31F0;animation.views=animation_views;animation.view_count=2;
    if(m.bad||!animation_views[0].bytes||!animation_views[1].bytes||dreadzone_animation_00688FE0(&animation,0,0)!=0)goto done;
    if(!ordinary_domain(&m,record))goto done;
    put_byte(&m,record+0x2ED5,255);put_half(&m,record+0x26E8,0xFFFF);put_half(&m,record+0x26EA,0xFFFF);
    if(inventory_views(&m,record,&equipment,reads,mutable_views)!=1)goto done;
    /* The mode0/index0 branch cannot equal level1; preserve the next actual
       selected marker and execute its real bounded equipment path if reached. */
    selected=byte(&m,0x171B67);
    if(selected&&dreadzone_equipment_00688578(&equipment,selected)<0)goto done;
    for(i=0;i<9;i++)put(&m,record+0x2D30+i*4,0xFFFFFFFFU);
    put_float(&m,record+0x2E24,2.125f);put_float(&m,record+0x2E28,1.25f);
    random.storage=map(&m,0x21DC80,4);random.bytes=4;
    if(m.bad||deadlocked_random_0058FF38(&random,0xB4,0x12C,&random_value)!=0)goto done;
    put(&m,record+0x1870,(u32)random_value);put(&m,record+0x2E58,0);
    put_float(&m,record+0x2E48,10.f);put_float(&m,record+0x1874,0.007000000216066837310791015625f);
    put_float(&m,record+0x1878,0.300000011920928955078125f);put_float(&m,record+0x1480,1.15999996662139892578125f);
    put(&m,record+0x1484,word(&m,record+0xA8));put_float(&m,record+0x1488,0.119999997317790985107421875f);
    put_float(&m,record+0x2E4C,13.f);put(&m,record+0x2E60,11);
    camera_view.guest_base=base;camera_view.bytes=bytes;camera_view.storage=m.p;
    camera.views=&camera_view;camera.view_count=1;camera.output=record+0x1970;camera.position=record+0xA0;camera.actor=word(&m,record+0x634);
    {u32 angle=word(&m,record+0xB8);memcpy(&camera.angle12,&angle,4);}camera.angle13=0.f;
    if(m.bad||dreadzone_camera_init(&camera)!=1)goto done;
    put_float(&m,record+0x1AFC,1.14999997615814208984375f);
    if(byte(&m,record+0x266D)){put(&m,record+0x1A58,record+0x18D0);put(&m,record+0x1A5C,record+0x1900);}
    else{put(&m,record+0x1A58,record+0x20);put(&m,record+0x1A5C,record+0x60);}
    if(byte(&m,0x171DE0)==0)put_half(&m,record+0x1A50,(u16)(half(&m,record+0x1A50)|1));
    put(&m,record+0x1A54,0x68FE70);put(&m,record+0x1A9C,word(&m,record+0x2F10));put(&m,record+0x1AA8,word(&m,record+0x2F10));put(&m,record+0x1AA0,record);
    if(byte(&m,0x171DE0)==1){put(&m,record+0x1A30,0xC0C00000U);put_float(&m,record+0x1A94,6.f);put(&m,record+0x1A38,0);put_half(&m,record+0x1A50,(u16)(half(&m,record+0x1A50)&0xFFFE));}
    put_byte(&m,record+0x2E84,255);
    if(!ordinary_domain(&m,record))goto done;
    if(m.bad||dreadzone_inventory_006909C8(&equipment.inventory)!=0)goto done;
    put_float(&m,record+0x2FC0,1.f);
    if(m.bad||dreadzone_equipment_00690B60(&equipment)!=0||dreadzone_equipment_00690C88(&equipment)!=0)goto done;
    copy_quad(&m,record+0x2F80,record+0xB0);put_half(&m,record+0x2E2C,1);put(&m,record+0x2D78,1);put(&m,record+0x2D88,2);
    put_float(&m,record+0x2D8C,10.f);put_float(&m,record+0x2D90,5.5f);put(&m,record+0x2D98,4);put(&m,record+0x2DA8,0x67);
    put_float(&m,record+0x2DAC,8.f);put_float(&m,record+0x2DB0,5.f);copy_quad(&m,record+0x2F70,record+0xA0);
    put_float(&m,record+0x2D7C,10.f);put_float(&m,record+0x2D80,5.5f);put_float(&m,record+0x2D9C,8.f);put_float(&m,record+0x2DA0,5.f);
    if(!m.bad){memcpy(storage,m.p,bytes);status=1;}
done:free(m.p);return status;
}

int dreadzone_actor_construct(const DreadzoneActorConstructor *request) {
    DreadzoneActorConstructor q;
    if(!host_span(request,sizeof(q)))return DREADZONE_CONSTRUCTOR_INVALID;
    memcpy(&q,request,sizeof(q));
    if((q.guest_base&15U)||!q.actor_pool_count||q.actor_pool_count>4096||
       !q.grid_block_count||q.grid_block_count>3072||q.scratch_bytes<0x4000||
       q.vu_data_bytes<0x1000)return DREADZONE_CONSTRUCTOR_INVALID;
    return constructor_complete(q.storage,q.guest_base,q.bytes,q.record,
        q.actor_pool_count,q.grid_block_count,q.incoming_vf25_27_w,
        q.incoming_side_output_w,q.initial_scratch,q.initial_vu_data);
}
