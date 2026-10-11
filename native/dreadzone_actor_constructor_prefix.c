/* Actual reset, occupied-pool selection, finite orientation, two wall probes
 * and backlink/scale region of supported L01 00668070. The complete outer
 * constructor owns publication and remaining ground/transform/tail work.
 */
#include "dreadzone_actor_constructor_internal.h"
#include "dreadzone_input_context_init.h"
#include "dreadzone_actor_parameters.h"
#include "dreadzone_actor_visibility.h"
#include "dreadzone_wall_contact.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct PrefixMemory {u8 *p;u32 base,n;int bad;} PrefixMemory;
static u8 *at(PrefixMemory *m,u32 a,u32 n) {
    if(a<m->base||n>m->n||a-m->base>m->n-n){m->bad=1;return NULL;}
    return m->p+a-m->base;
}
static u32 word(PrefixMemory *m,u32 a) {u32 w=0;u8 *p;if(a&3U){m->bad=1;return 0;}p=at(m,a,4);if(p)memcpy(&w,p,4);return w;}
static u16 half(PrefixMemory *m,u32 a) {u16 w=0;u8 *p;if(a&1U){m->bad=1;return 0;}p=at(m,a,2);if(p)memcpy(&w,p,2);return w;}
static void put(PrefixMemory *m,u32 a,u32 w) {u8 *p;if(a&3U){m->bad=1;return;}p=at(m,a,4);if(p)memcpy(p,&w,4);}
static void put_half(PrefixMemory *m,u32 a,u16 w) {u8 *p;if(a&1U){m->bad=1;return;}p=at(m,a,2);if(p)memcpy(p,&w,2);}
static int separate(u32 a,u32 n,u32 b,u32 k){return a>=b?a-b>=k:b-a>=n;}
static int orientation(PrefixMemory *m,u32 actor,u32 position,float out[32]) {
    float angles[3],p[4];u8 *a=at(m,actor+0xF0,12),*v=at(m,position,16),*basis;
    if(m->bad)return -2;memcpy(angles,a,12);memcpy(p,v,16);
    if(dreadzone_constructor_orientation(angles,p,out)!=1)return -2;
    basis=at(m,actor+0xC0,48);if(m->bad)return -2;memcpy(basis,out,48);return 1;
}
static int probe(PrefixMemory *m,u32 actor,u32 actors,u32 blocks,const u32 w[3],float out[32]) {
    DreadzoneWallRequest q={0};DreadzoneWallInfo info;u32 start[4],end[4];
    q.storage=m->p;q.guest_base=m->base;q.storage_bytes=m->n;
    q.packet_guest=0x242340;q.grid_guest=0x26FF20;q.actor_pool_pointer_guest=0x222D14;
    q.actor_pool_count=actors;q.grid_block_count=blocks;q.ignored_actor_guest=actor;q.caller_flags=2;
    memcpy(q.incoming_vf25_27_w,w,12);memcpy(start,out+20,16);memcpy(end,out+24,16);
    return dreadzone_wall_contact_vectors(&q,start,end,&info);
}
int dreadzone_constructor_prefix(u8 *storage,u32 base,u32 bytes,u32 record,u32 actors,u32 blocks,const u32 incoming_w[3]) {
    PrefixMemory m;u8 *r;u32 pool,end,scan,actor=0,definition,vtable;u16 kind;
    float out[32],scale;int result=-2,hit;u32 w[3];DreadzoneActorParameterView parameters;
    DreadzoneActorVisibilityRequest visibility;DreadzoneActorVisibilityInfo visibility_info;
    if(!storage||!incoming_w||(uintptr_t)incoming_w>UINTPTR_MAX-12U||!bytes||(uintptr_t)storage>UINTPTR_MAX-bytes||bytes>16U*1024U*1024U||(u64)base+bytes>0x70000000ULL||
       (record&15U)||record>0x7FFFFFFFU-0x31F0U||!actors||actors>4096||!blocks||blocks>3072)return -1;
    memcpy(w,incoming_w,12);m.base=base;m.n=bytes;m.bad=0;m.p=(u8 *)malloc(bytes);if(!m.p)return -3;
    memcpy(m.p,storage,bytes);r=at(&m,record,0x31F0);pool=word(&m,0x222D14);end=word(&m,0x222D2C);
    if(m.bad||(pool&15U)||end<pool||end-pool>actors*0x100U||((end-pool)&255U)||
       !separate(record,0x31F0,pool,end-pool)||!separate(record,0x31F0,0x100000,0x1F0000))goto done;
    vtable=word(&m,record+0x14);if((vtable&3U)||vtable>0x7FFFFFFFU-0x2CU)goto done;
    if(word(&m,vtable+0x28)!=0x6A6598||word(&m,0x21E878)!=0)goto done;
    if(!at(&m,pool,end-pool)||m.bad)goto done;
    func_L01_006A6598((DreadzoneActorInit31F0 *)r);put(&m,record+0x2F10,0);
    parameters.record=r;parameters.record_guest=record;parameters.record_bytes=0x31F0;
    parameters.mode_0021E824=(s32)word(&m,0x21E824);
    if(m.bad||dreadzone_actor_00669A20(&parameters)!=0)goto done;
    for(scan=pool;scan<end;scan+=0x100) {
        kind=half(&m,scan+0xBC);if(kind==0x171C)goto done;
        if(kind==0x23F7){actor=scan;break;}
    }
    if(!actor||m.bad)goto done;
    put(&m,record+0x2EF0,actor);put_half(&m,actor+0x74,(u16)(half(&m,actor+0x74)|half(&m,0x2206B0)));
    put(&m,record+0x2F14,1);
    visibility.storage=m.p;visibility.guest_base=base;visibility.storage_bytes=bytes;
    visibility.actor_guest=actor;visibility.mode_word=1;
    if(m.bad||dreadzone_actor_visibility_005A8630(&visibility,&visibility_info)!=1)goto done;
    if(orientation(&m,actor,actor+0x10,out)!=1)goto done;
    memcpy(at(&m,record+0xB0,16),at(&m,actor+0xF0,16),16);
    hit=probe(&m,actor,actors,blocks,w,out);if(hit<0)goto done;
    if(hit)memcpy(at(&m,actor+0x10,16),at(&m,0x242360,16),16);
    memcpy(at(&m,record+0xA0,16),at(&m,actor+0x10,16),16);
    put_half(&m,actor+0x34,(u16)(half(&m,actor+0x34)|8));put(&m,actor+0x90,record);
    if(orientation(&m,actor,record+0xA0,out)!=1)goto done;
    hit=probe(&m,actor,actors,blocks,w,out);if(hit<0)goto done;
    if(hit)memcpy(at(&m,record+0xA0,16),at(&m,0x242360,16),16);
    put_half(&m,actor+0x34,(u16)(half(&m,actor+0x34)|2));definition=word(&m,actor+0x24);
    if((definition&3U)||definition>0x7FFFFFFFU-0x2CU)goto done;
    {u32 raw=word(&m,definition+0x24);volatile float value;memcpy(&scale,&raw,4);
     if(!isfinite(scale)||scale<=0.f||scale>16.f)goto done;value=scale*1.2000000476837158203125f;scale=value;}
    {u32 raw;memcpy(&raw,&scale,4);put(&m,actor+0x70,raw);}put(&m,record+0x634,actor);
    if(!m.bad){memcpy(storage,m.p,bytes);result=1;}
done:free(m.p);return result;
}
