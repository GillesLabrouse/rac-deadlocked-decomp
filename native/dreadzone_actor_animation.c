#include "dreadzone_actor_animation.h"

enum { CLEANUP_MAX_STEPS=33, CLEANUP_MAX_WRITES=33*34 };
typedef struct CleanupWrite { u32 address;u8 value; } CleanupWrite;
typedef struct CleanupPlan {
    const DreadzoneAnimationContext *host;
    CleanupWrite writes[CLEANUP_MAX_WRITES];
    u32 count;
} CleanupPlan;

static u32 cleanup_word(const u8 *p) {
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}
static u8 cleanup_byte(const CleanupPlan *plan,u32 address) {
    u32 i=plan->count;
    while(i!=0) {
        --i;
        if(plan->writes[i].address==address) return plan->writes[i].value;
    }
    /* The complete containing read range is translated before this helper. */
    return *dreadzone_animation_translate(plan->host,address,1);
}
static u32 cleanup_read32(const CleanupPlan *plan,u32 address) {
    return (u32)cleanup_byte(plan,address)|((u32)cleanup_byte(plan,address+1)<<8)|
        ((u32)cleanup_byte(plan,address+2)<<16)|((u32)cleanup_byte(plan,address+3)<<24);
}
static void cleanup_zero(CleanupPlan *plan,u32 address,u32 bytes) {
    u32 i;
    for(i=0;i<bytes;++i) {
        plan->writes[plan->count].address=address+i;
        plan->writes[plan->count].value=0;
        ++plan->count;
    }
}

int dreadzone_animation_0058ACB8(const DreadzoneAnimationContext *host,u32 actor_guest) {
    CleanupPlan plan;
    u8 *actor,*node;
    u32 current,next,slot,occupancy,metadata,steps=0;
    s32 index,status;
    volatile u8 *publication;
    if(!dreadzone_animation_validate(host)) return DREADZONE_ANIMATION_INVALID_STORAGE;
    if(!actor_guest || (actor_guest&3)) return DREADZONE_ANIMATION_NEEDS_ACTOR;
    actor=dreadzone_animation_translate(host,actor_guest,0x54);
    if(!actor) return DREADZONE_ANIMATION_NEEDS_ACTOR;
    plan.host=host;plan.count=0;
    current=cleanup_word(actor+0x50);
    while(current!=0) {
        /*32 real slots can be released and revisited once. A revisit sees the
         * zeroed link and terminates; no invented acyclic-list constraint. */
        if(++steps>CLEANUP_MAX_STEPS || current<0x28EB40 ||
           current-0x28EB40>=0x400 || ((current-0x28EB40)&31))
            return DREADZONE_ANIMATION_NEEDS_NODE_POOL;
        node=dreadzone_animation_translate(host,current,32);
        slot=(current-0x28EB40)/32;
        occupancy=0x28EF40+slot;
        if(!node || !dreadzone_animation_translate(host,occupancy,1))
            return DREADZONE_ANIMATION_NEEDS_NODE_POOL;
        if(dreadzone_animation_overlap(actor_guest,0x54,current,32) ||
           dreadzone_animation_overlap(actor_guest,0x54,occupancy,1))
            return DREADZONE_ANIMATION_INVALID_STORAGE;
        next=cleanup_read32(&plan,current+0x10); /* Before release, as retail. */
        if(cleanup_byte(&plan,current+0x15)&4) {
            u32 half=(u32)cleanup_byte(&plan,current+0xE)|
                ((u32)cleanup_byte(&plan,current+0xF)<<8);
            index=(s32)(half^0x8000U)-0x8000;
            metadata=0x293004U+(u32)(index*8);
            if(dreadzone_animation_overlap(metadata,1,current,32) ||
               metadata==occupancy || dreadzone_animation_overlap(metadata,1,actor_guest,0x54))
                return DREADZONE_ANIMATION_INVALID_STORAGE;
            if(!dreadzone_animation_translate(host,metadata,1))
                return DREADZONE_ANIMATION_NEEDS_METADATA;
            cleanup_zero(&plan,metadata,1);
        }
        cleanup_zero(&plan,occupancy,1);
        cleanup_zero(&plan,current,32);
        current=next;
    }
    /* Every real release footprint is proven under its preceding writes.
     * Context/view descriptors are disjoint from backing by validate(). */
    current=cleanup_word(actor+0x50);
    while(current!=0) {
        node=dreadzone_animation_translate(host,current,32);
        next=cleanup_word(node+0x10);
        status=dreadzone_animation_0058A8D8(host,current);
        if(status!=DREADZONE_ANIMATION_COMPLETED) return status;
        current=next;
    }
    publication=actor+0x50;
    publication[0]=0;publication[1]=0;publication[2]=0;publication[3]=0;
    return DREADZONE_ANIMATION_COMPLETED;
}
