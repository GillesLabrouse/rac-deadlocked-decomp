#include "dreadzone_animation_replacement.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct ReplacementWrite { u8 *target; u32 value, bytes; } ReplacementWrite;
typedef struct ReplacementPlan {
    DreadzoneAnimationReplacementView views[32];
    u32 view_count, count, capacity;
    ReplacementWrite *writes;
} ReplacementPlan;

static u8 *replacement_translate(const ReplacementPlan *plan,u32 address,u32 bytes) {
    u32 i;
    if(!bytes || address>UINT32_MAX-(bytes-1)) return NULL;
    for(i=0;i<plan->view_count;i++) {
        const DreadzoneAnimationReplacementView *v=&plan->views[i];
        if(address>=v->guest_base && address-v->guest_base<=v->bytes &&
           bytes<=v->bytes-(address-v->guest_base)) return v->storage+(address-v->guest_base);
    }
    return NULL;
}
static int replacement_read(const ReplacementPlan *plan,u32 address,u32 bytes,u32 *value) {
    u8 *target=replacement_translate(plan,address,bytes);
    u32 i,j,result=0;
    if(!target) return 0;
    for(j=0;j<bytes;j++) {
        u8 byte=target[j];
        for(i=plan->count;i>0;i--) {
            const ReplacementWrite *w=&plan->writes[i-1];
            uintptr_t wp=(uintptr_t)w->target,tp=(uintptr_t)(target+j);
            if(tp>=wp && tp-wp<w->bytes) { byte=(u8)(w->value>>(8*(tp-wp)));break; }
        }
        result|=(u32)byte<<(j*8);
    }
    *value=result;return 1;
}
static int replacement_write(ReplacementPlan *plan,u32 address,u32 bytes,u32 value) {
    u8 *target=replacement_translate(plan,address,bytes);
    ReplacementWrite *w;
    /* Pool bounds are explicit immutable inputs in this host contract. Retail
     * reloads those global words, so a write aliasing them is outside this domain. */
    if(!target || plan->count>=plan->capacity ||
       (address<0x222D18 && address+bytes>0x222D14) ||
       (address<0x222D34 && address+bytes>0x222D30)) return 0;
    w=&plan->writes[plan->count++];w->target=target;w->bytes=bytes;w->value=value;
    return 1;
}
static int replacement_copy(ReplacementPlan *plan,u32 destination,u32 source,u32 bytes) {
    u32 value;
    return replacement_read(plan,source,bytes,&value) && replacement_write(plan,destination,bytes,value);
}

int dreadzone_animation_replace(const DreadzoneAnimationReplacement *request) {
    DreadzoneAnimationReplacement r;
    ReplacementPlan p;
    u32 i,j,value,flag,destination_flags,actor,actor_definition,actor_kind,flags;
    int status=DREADZONE_ANIMATION_REPLACEMENT_INVALID;
    if(!request) return status;
    r=*request;
    if(!r.source || !r.destination) return DREADZONE_ANIMATION_REPLACED;
    if(!r.views || !r.view_count || r.view_count>32 || (r.source&3) || (r.destination&3) ||
       r.source>UINT32_MAX-0x444 || r.destination>UINT32_MAX-0x444 ||
       r.actors_end<r.actors_begin || (r.actors_begin&255) ||
       ((r.actors_end-r.actors_begin)&255) || (r.actors_end-r.actors_begin)/256>32768)
        return status;
    memset(&p,0,sizeof(p));
    p.view_count=r.view_count;
    memcpy(p.views,r.views,r.view_count*sizeof(p.views[0]));
    for(i=0;i<p.view_count;i++) {
        const DreadzoneAnimationReplacementView *a=&p.views[i];
        uintptr_t ap=(uintptr_t)a->storage;
        if(!a->storage || !a->bytes || a->guest_base>UINT32_MAX-a->bytes || ap>UINTPTR_MAX-a->bytes)
            return status;
        for(j=0;j<i;j++) {
            const DreadzoneAnimationReplacementView *b=&p.views[j];
            uintptr_t bp=(uintptr_t)b->storage;
            if((a->guest_base<b->guest_base+b->bytes && b->guest_base<a->guest_base+a->bytes) ||
               (ap<bp+b->bytes && bp<ap+a->bytes)) return status;
        }
    }
    p.capacity=268+(r.actors_end-r.actors_begin)/256;
    p.writes=(ReplacementWrite *)calloc(p.capacity,sizeof(*p.writes));
    if(!p.writes) return DREADZONE_ANIMATION_REPLACEMENT_CAPACITY;
    if(!replacement_read(&p,r.destination+0x47,1,&flag)) goto done;
    flag&=8;
    if(!replacement_copy(&p,r.destination+0xC,r.source+0xC,1) ||
       !replacement_copy(&p,r.destination+0xD,r.source+0xD,1) ||
       !replacement_copy(&p,r.destination+0x10,r.source+0x10,4) ||
       !replacement_copy(&p,r.destination+0x1C,r.source+0x1C,4) ||
       !replacement_copy(&p,r.destination+0x28,r.source+0x28,4) ||
       !replacement_copy(&p,r.destination+0x44,r.source+0x44,2) ||
       !replacement_read(&p,r.source+0x47,1,&value) ||
       !replacement_write(&p,r.destination+0x47,1,(value&0xF7)|flag) ||
       !replacement_copy(&p,r.destination+0x46,r.source+0x46,1)) goto done;
    for(i=0;;i++) {
        if(!replacement_read(&p,r.source+0xC,1,&value)) goto done;
        if(i>=value) break;
        if(!replacement_copy(&p,r.destination+0x48+i*4,r.source+0x48+i*4,4)) goto done;
    }
    for(actor=r.actors_begin;actor<r.actors_end;actor+=256) {
        if(!replacement_read(&p,actor+0x24,4,&actor_definition)) goto done;
        if(!actor_definition) continue;
        if(!replacement_read(&p,actor+0x20,1,&actor_kind)) goto done;
        if(actor_kind>=0xFD || actor_definition!=r.destination) continue;
        if(!replacement_read(&p,actor+0x36,2,&flags) ||
           !replacement_read(&p,r.destination+0x47,1,&destination_flags)) goto done;
        if(flags&8) {
            if(!(destination_flags&8) && !replacement_write(&p,actor+0x36,2,flags&0xFFF7)) goto done;
        } else if((destination_flags&8) && !replacement_write(&p,actor+0x36,2,flags|8)) goto done;
    }
    for(i=0;i<p.count;i++) for(j=0;j<p.writes[i].bytes;j++)
        p.writes[i].target[j]=(u8)(p.writes[i].value>>(j*8));
    status=DREADZONE_ANIMATION_REPLACED;
done:
    free(p.writes);return status;
}
