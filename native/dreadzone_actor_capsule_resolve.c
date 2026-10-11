#include "dreadzone_actor_capsule_resolve.h"
#include "dreadzone_capsule_contact.h"
#include "dreadzone_static_body_contact.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct ResolveRun {
    DreadzoneCapsuleResolveRequest request;
    DreadzoneCapsuleResolveInfo info;
    int status;
    int track_registers,state_known;
    DreadzoneCapsuleResolveRegisters registers;
} ResolveRun;
static int span(const void *p,uintptr_t n){return p && (uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *p,uintptr_t n,const void *q,uintptr_t m) {
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;return a>=b?a-b<m:b-a<n;
}
static void fail(ResolveRun *r,int status){if(r->status>0)r->status=status;}
static u8 *memory(ResolveRun *r,u32 address,u32 count,u32 alignment) {
    u32 base=r->request.guest_base,bytes=r->request.storage_bytes;
    if((address&(alignment-1)) || address<base || count>bytes || address-base>bytes-count) {
        fail(r,DREADZONE_CAPSULE_RESOLVE_INVALID);return NULL;
    }
    return r->request.storage+(address-base);
}
static u32 word(ResolveRun *r,u32 address){u32 v=0;u8 *p=memory(r,address,4,4);if(p)memcpy(&v,p,4);return v;}
static s32 integer(ResolveRun *r,u32 address){u32 v=word(r,address);s32 s;memcpy(&s,&v,4);return s;}
static s16 half(ResolveRun *r,u32 address){s16 v=0;u8 *p=memory(r,address,2,2);if(p)memcpy(&v,p,2);return v;}
static u8 byte(ResolveRun *r,u32 address){u8 *p=memory(r,address,1,1);return p?*p:0;}
static void write_word(ResolveRun *r,u32 address,u32 v){u8 *p=memory(r,address,4,4);if(p)memcpy(p,&v,4);}
static void write_byte(ResolveRun *r,u32 address,u8 v){u8 *p=memory(r,address,1,1);if(p)*p=v;}
static void quad(ResolveRun *r,u32 address,u32 v[4]) {
    u8 *p=memory(r,address,16,16);if(p)memcpy(v,p,16);else memset(v,0,16);
}
static void write_quad(ResolveRun *r,u32 address,const u32 v[4]){u8 *p=memory(r,address,16,16);if(p)memcpy(p,v,16);}
static f32 floating(ResolveRun *r,u32 raw) {
    f32 value;u32 absbits=raw&0x7FFFFFFFU;memcpy(&value,&raw,4);
    if(absbits && (absbits<0x00800000U || absbits>=0x7F800000U))fail(r,DREADZONE_CAPSULE_RESOLVE_UNSUPPORTED_FLOAT);
    return value;
}
static f32 number(ResolveRun *r,u32 address){return floating(r,word(r,address));}
static f32 rounded(ResolveRun *r,f32 v){u32 raw;memcpy(&raw,&v,4);floating(r,raw);return v;}
static f32 add(ResolveRun *r,f32 a,f32 b){volatile f32 v=a+b;return rounded(r,v);}
static f32 sub(ResolveRun *r,f32 a,f32 b){volatile f32 v=a-b;return rounded(r,v);}
static f32 mul(ResolveRun *r,f32 a,f32 b){volatile f32 v=a*b;return rounded(r,v);}
static u32 bits(ResolveRun *r,f32 v){u32 raw;rounded(r,v);memcpy(&raw,&v,4);return raw;}
static f32 length(ResolveRun *r,const f32 v[3]) {
    f32 square=add(r,add(r,mul(r,v[0],v[0]),mul(r,v[1],v[1])),mul(r,v[2],v[2]));
    volatile f32 root=sqrtf(square);return rounded(r,root);
}
static int capsule(ResolveRun *r,u32 flags,f32 height) {
    DreadzoneCapsuleRequest q;DreadzoneCapsuleInfo info;int result;
    u32 b=r->request.record_guest;memset(&q,0,sizeof(q));
    q.storage=r->request.storage;q.guest_base=r->request.guest_base;q.storage_bytes=r->request.storage_bytes;
    q.center_guest=b+0xA0;q.packet_guest=0x242340;q.grid_guest=0x26FF20;q.actor_pool_pointer_guest=0x222D14;
    q.actor_pool_count=r->request.actor_pool_count;q.ignored_actor_guest=word(r,b+0x2EF0);q.caller_flags=flags;
    q.radius=number(r,b+0x204);q.height=height;
    memcpy(q.incoming_vf25_29_w,r->request.incoming_vf25_29_w,sizeof(q.incoming_vf25_29_w));
    if(r->status<0)return 0;
    /* A later capsule invalidates every earlier supplementary witness. */
    if(r->track_registers) {
        r->state_known=0;
        r->registers.last_positive_table=0;r->registers.last_positive_count=0;
    }
    result=dreadzone_capsule_contact(&q,&info);r->info.capsule_calls++;
    if(result<0)fail(r,result==-1?DREADZONE_CAPSULE_RESOLVE_INVALID:DREADZONE_CAPSULE_RESOLVE_UNSUPPORTED_PATH);
    return result;
}
static int supplementary(ResolveRun *r) {
    DreadzoneStaticBodyRequest q;DreadzoneStaticBodyInfo info;int result;
    s32 actual_count=0;
    u32 b=r->request.record_guest;memset(&q,0,sizeof(q));
    q.storage=r->request.storage;q.guest_base=r->request.guest_base;q.storage_bytes=r->request.storage_bytes;
    q.table_guest=word(r,0x222BE0);q.center_guest=b+0xA0;q.packet_guest=0x242340;
    q.radius=number(r,b+0x204);
    memcpy(q.incoming_vf25_29_w,r->request.incoming_vf25_29_w,sizeof(q.incoming_vf25_29_w));
    if(r->status<0)return 0;
    /* This reload follows actual capsule visitation and item suspension. */
    if(r->track_registers && q.table_guest)actual_count=integer(r,q.table_guest);
    if(r->status<0)return 0;
    result=dreadzone_static_body_contact(&q,&info);r->info.static_calls++;
    if(r->track_registers && result>=0 && actual_count>0) {
        /* Full supported positive-count body paths execute actual high-half
         * clearing operations. Scalar resolver/history code preserves them. */
        r->state_known=1;r->registers.high64[0]=0;r->registers.high64[1]=0;
        r->registers.last_positive_table=q.table_guest;
        r->registers.last_positive_count=(u32)actual_count;
    }
    if(result<0)fail(r,result==-1?DREADZONE_CAPSULE_RESOLVE_INVALID:DREADZONE_CAPSULE_RESOLVE_UNSUPPORTED_PATH);
    return result;
}
static s32 resolve(ResolveRun *r,s32 attempt) {
    u32 b=r->request.record_guest,saved[4],raw[4],direction[4],basis[4],offset[4]={0},flags;
    s32 previous,state,limit,contacts=0,hit,priorRelated;u32 related=0,item,pointer;
    f32 dot,delta[3],distance,radius,height,amount=0.f;int special,i;
    r->info.resolve_calls++;
    if(integer(r,0x21DE10)==31 && integer(r,0x21EBD8)==0 && integer(r,0x21DDB4)==2)return 1;
    if(half(r,b+0x3A0)!=0)return 1;
    quad(r,b+0xA0,saved);
    if(half(r,b+0x2E18)!=0){fail(r,DREADZONE_CAPSULE_RESOLVE_UNSUPPORTED_PATH);return 0;}
    quad(r,b+0x270,direction);quad(r,b+0x40,basis);
    dot=add(r,add(r,mul(r,floating(r,basis[0]),floating(r,direction[0])),
        mul(r,floating(r,basis[1]),floating(r,direction[1]))),mul(r,floating(r,basis[2]),floating(r,direction[2])));
    special=dot>-0.920000016689300537109375f;previous=integer(r,b+0x25D4);
    /* Only actual selection controls the supported route; no caller fields are changed. */
    if(byte(r,b+0x2659)!=0 || previous==15 || previous==17 || half(r,b+0x3D6)!=0 || special) {
        fail(r,DREADZONE_CAPSULE_RESOLVE_UNSUPPORTED_PATH);return 0;
    }
    if(previous!=17)amount=number(r,b+0x1F4);
    offset[2]=bits(r,amount);quad(r,b+0xA0,raw);
    for(i=0;i<3;++i)raw[i]=bits(r,add(r,floating(r,raw[i]),floating(r,offset[i])));
    write_quad(r,b+0xA0,raw);
    state=integer(r,b+0x25CC);flags=0x24;
    if(state==126 || (state==153 && integer(r,b+0x25D8)==126))flags=0xD24;
    if(state==104 || state==122)flags=0x324;
    limit=byte(r,b+0x266D)!=0?8:1;
    while(r->status>0) {
        previous=integer(r,b+0x25D4);
        if(previous==17 || previous==15 || byte(r,b+0x2659)!=0 || special) {
            fail(r,DREADZONE_CAPSULE_RESOLVE_UNSUPPORTED_PATH);return 0;
        }
        height=sub(r,number(r,b+0x1F0),number(r,b+0x1F4));
        if(height<0.0500000007450580596923828125f)height=0.0500000007450580596923828125f;
        related=0;priorRelated=0;
        if(integer(r,b+0x2314)==2) {
            item=word(r,b+0x22F0);
            if(item) {
                if(item>0xFFFFFFFFU-0xBD){fail(r,DREADZONE_CAPSULE_RESOLVE_INVALID);return 0;}
                if(half(r,item+0xBC)==0x2094) {pointer=word(r,item+0xAC);related=word(r,pointer);}
            }
        }
        if(related) {
            if(related>0xFFFFFFFFU-0x9B){fail(r,DREADZONE_CAPSULE_RESOLVE_INVALID);return 0;}
            priorRelated=integer(r,related+0x98)!=0;write_word(r,related+0x98,0);
        }
        hit=capsule(r,flags,height);
        if(half(r,b+0x39E)==0)hit|=supplementary(r);
        if(related && priorRelated) {
            pointer=word(r,related+0x24);
            if(pointer>0xFFFFFFFFU-0x13){fail(r,DREADZONE_CAPSULE_RESOLVE_INVALID);return 0;}
            write_word(r,related+0x98,word(r,pointer+0x10));
        }
        if(r->status<0)return 0;
        if(!hit)break;
        quad(r,0x242370,raw);write_quad(r,b+0xA0,raw);
        quad(r,0x242380,raw);write_quad(r,b+0x1D0,raw);write_byte(r,b+0x226,1);
        quad(r,0x242360,raw);write_quad(r,b+0x1E0,raw);write_word(r,b+0x20C,word(r,0x242358));
        ++contacts;++r->info.contact_iterations;
        if(contacts>=limit)break;
    }
    quad(r,b+0xA0,raw);
    for(i=0;i<3;++i) {
        raw[i]=bits(r,sub(r,floating(r,raw[i]),floating(r,offset[i])));
        delta[i]=sub(r,floating(r,raw[i]),floating(r,saved[i]));
    }
    write_quad(r,b+0xA0,raw);distance=length(r,delta);radius=number(r,b+0x204);
    if(!(add(r,radius,radius)<distance))return 1;
    if(attempt!=15)return -1;
    for(i=0;i<3;++i){if(delta[i]<-512.f)delta[i]=-512.f;if(delta[i]>512.f)delta[i]=512.f;}
    distance=length(r,delta);
    if(distance!=0.f) {
        volatile f32 ratio=radius/distance;rounded(r,ratio);
        for(i=0;i<3;++i)delta[i]=mul(r,delta[i],ratio);
    }else for(i=0;i<3;++i)delta[i]=0.f;
    for(i=0;i<3;++i)saved[i]=bits(r,add(r,floating(r,saved[i]),delta[i]));
    write_quad(r,b+0xA0,saved);return -1;
}
static int capsule_execute(const DreadzoneCapsuleResolveRequest *request,DreadzoneCapsuleResolveInfo *info,
                           int track,const u32 incoming_high64[2],DreadzoneCapsuleResolveRegisters *output) {
    ResolveRun r;u8 *shadow,*destination;u32 end,b;int status;u32 attempt,limit;s32 result,count,index;
    if(!span(request,sizeof(*request)) || !span(info,sizeof(*info)) ||
       overlap(request,sizeof(*request),info,sizeof(*info)))return DREADZONE_CAPSULE_RESOLVE_INVALID;
    if(track && (!span(incoming_high64,8) || !span(output,sizeof(*output)) ||
       overlap(output,sizeof(*output),request,sizeof(*request)) ||
       overlap(output,sizeof(*output),info,sizeof(*info)) ||
       overlap(output,sizeof(*output),request->storage,request->storage_bytes)))return DREADZONE_CAPSULE_RESOLVE_INVALID;
    memset(&r,0,sizeof(r));r.request=*request;b=r.request.record_guest;r.track_registers=track;
    if(track){memcpy(r.registers.high64,incoming_high64,8);r.state_known=1;}
    if(!r.request.storage_bytes || r.request.storage_bytes>64U*1024U*1024U ||
       !span(r.request.storage,r.request.storage_bytes) || r.request.guest_base>=0x80000000U ||
       r.request.storage_bytes>0x80000000U-r.request.guest_base || (b&15) ||
       r.request.operation>1 || r.request.actor_pool_count>65536 ||
       overlap(info,sizeof(*info),r.request.storage,r.request.storage_bytes))return DREADZONE_CAPSULE_RESOLVE_INVALID;
    end=r.request.guest_base+r.request.storage_bytes;
    if(b<r.request.guest_base || end-b<0x31F0 || b>end)return DREADZONE_CAPSULE_RESOLVE_INVALID;
    destination=r.request.storage;
    shadow=(u8 *)malloc(r.request.storage_bytes);if(!shadow)return DREADZONE_CAPSULE_RESOLVE_TRANSACTION_MEMORY;
    memcpy(shadow,r.request.storage,r.request.storage_bytes);r.request.storage=shadow;r.status=1;
    if(r.request.operation==DREADZONE_CAPSULE_RESOLVE_EF00) {
        r.info.original_return=resolve(&r,r.request.attempt);r.info.has_original_return=1;
    }else {
        limit=byte(&r,b+0x266D)!=0?16U:1U;
        for(attempt=0;attempt<limit && r.status>0;++attempt) {
            result=resolve(&r,(s32)attempt);if(r.status<0 || result==1)break;
            count=integer(&r,b+0x2C7C);index=integer(&r,b+0x2C78);
            if(count>=2) {
                write_word(&r,b+0x2C7C,(u32)(count-1));index=(s32)((u32)index+31U)%32;
                write_word(&r,b+0x2C78,(u32)index);
            }
            index=integer(&r,b+0x2C78);
            if(index<0 || index>=32){fail(&r,DREADZONE_CAPSULE_RESOLVE_UNSUPPORTED_PATH);break;}
            {u32 raw[4];quad(&r,b+0x26F0+(u32)index*16,raw);write_quad(&r,b+0xA0,raw);}
            ++r.info.history_restores;
        }
    }
    if(track && r.status>0 && !r.state_known)fail(&r,DREADZONE_CAPSULE_RESOLVE_UNSUPPORTED_PATH);
    status=r.status;
    if(status>0){memcpy(destination,shadow,r.request.storage_bytes);*info=r.info;if(track)*output=r.registers;}
    free(shadow);return status;
}

int dreadzone_actor_capsule_resolve(const DreadzoneCapsuleResolveRequest *request,DreadzoneCapsuleResolveInfo *info) {
    return capsule_execute(request,info,0,NULL,NULL);
}
int dreadzone_actor_capsule_registers(const DreadzoneCapsuleResolveRequest *request,const u32 incoming_high64[2],
                                    DreadzoneCapsuleResolveRegisters *output,DreadzoneCapsuleResolveInfo *info) {
    return capsule_execute(request,info,1,incoming_high64,output);
}
