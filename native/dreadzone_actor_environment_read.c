#include "dreadzone_actor_environment_read.h"
#include <stddef.h>
#include <stdint.h>
static uint32_t read_word(const uint8_t *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static void write_word(uint8_t *p,uint32_t v) {
    p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);
}
static int mapped_word(const DreadzoneActorEnvironmentReadRequest *r,uint32_t a) {
    return !(a&3u) && a>=r->guest_base &&
        (uint64_t)a+4<=(uint64_t)r->guest_base+r->storage_bytes;
}
int dreadzone_actor_environment_read_0058D4A0(const DreadzoneActorEnvironmentReadRequest *request) {
    DreadzoneActorEnvironmentReadRequest r;
    uint64_t end,input;uint32_t packed;unsigned i;
    if(!request || sizeof(*request)>UINTPTR_MAX-(uintptr_t)request)return -1;
    r=*request;
    end=(uint64_t)r.guest_base+r.storage_bytes;
    if(!r.storage || !r.storage_bytes || r.storage_bytes>0x1000000u ||
        r.guest_base>=0x80000000u || end>0x80000000u ||
        r.storage_bytes>UINTPTR_MAX-(uintptr_t)r.storage ||
        (r.guest_base<0x70004000u && end>0x70000000u))return -1;
    input=(uint64_t)r.actor_guest+0x3Cu;
    if(input>=0x80000000u || !mapped_word(&r,(uint32_t)input))return -2;
    for(i=0;i<3;++i)if(!mapped_word(&r,r.output_guest[i]))return -2;
    packed=read_word(r.storage+(uint32_t)input-r.guest_base);
    for(i=0;i<3;++i)write_word(r.storage+r.output_guest[i]-r.guest_base,(packed>>(8*i))&255u);
    return 1;
}
