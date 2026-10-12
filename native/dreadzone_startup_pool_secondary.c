#include "dreadzone_startup_pool.h"
#include <limits.h>
#include <string.h>
#include <stdint.h>

static int span(const DreadzoneStartupPoolMemory *v, uint32_t a, size_t n, uint8_t **out) {
    uint64_t end=(uint64_t)a+(uint64_t)n, mapped=(uint64_t)v->guest_base+v->length;
    if (!n || a<v->guest_base || end>UINT64_C(0x80000000) || end>mapped) return 0;
    *out=v->bytes+(size_t)(a-v->guest_base); return 1;
}
static uint32_t load32(const uint8_t *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static void store32(uint8_t *p,uint32_t x) {
    p[0]=(uint8_t)x;p[1]=(uint8_t)(x>>8);p[2]=(uint8_t)(x>>16);p[3]=(uint8_t)(x>>24);
}
int dreadzone_startup_pool_006670B8_index2(const DreadzoneStartupPoolMemory *input) {
    DreadzoneStartupPoolMemory v; uintptr_t request, backing; uint8_t *countp,*stridep,*table0,*table1,*table2,*records=NULL;
    uint32_t count,stride; uint64_t extent,i;
    if (!input) return -1;
    request=(uintptr_t)(void *)input;
    if (request>UINTPTR_MAX-sizeof(v)) return -1;
    memcpy(&v,input,sizeof(v));
    if (!v.bytes || !v.length || v.length>0x01000000u || v.guest_base>=0x80000000u ||
        (uint64_t)v.guest_base+v.length>0x80000000u) return -1;
    backing=(uintptr_t)(void *)v.bytes;
    if (backing>UINTPTR_MAX-v.length ||
        (request<backing+v.length && backing<request+sizeof(v))) return -1;
    if (!span(&v,0x003B0050u,4,&countp)||!span(&v,0x003B0098u,4,&stridep)||
        !span(&v,0x00366128u,4,&table0)||!span(&v,0x00366144u,4,&table1)||
        !span(&v,0x00366160u,4,&table2)) return -1;
    count=load32(countp); stride=load32(stridep);
    if ((int32_t)count>0) {
        extent=(uint64_t)(count-1u)*(uint64_t)stride+0x18u;
        /* Keep count/stride reloads unchanged, and all original SWs aligned.
         * A bounded iteration count also excludes nonreturning/huge workloads. */
        if (count>4096u || (stride&3u) || extent>SIZE_MAX ||
            (uint64_t)0x0037F420u+extent>0x003B0050u ||
            !span(&v,0x0037F420u,(size_t)extent,&records)) return -1;
    }
    /* The three table writes precede the count-controlled record loop in retail. */
    store32(table0,0); store32(table1,0x0037F420u); store32(table2,0x0037F420u);
    if ((int32_t)count<=0) return 0;
    for (i=0;i<count;i++) {
        uint64_t off=i*(uint64_t)stride;
        uint8_t *r=records+(size_t)off;
        store32(r+0x14,0x003B0068u);
        store32(r+0x10,i==0?0u:(uint32_t)(0x0037F420u+(i-1)*(uint64_t)stride));
        store32(r+0x0C,i+1==count?0u:(uint32_t)(0x0037F420u+(i+1)*(uint64_t)stride));
    }
    return 0;
}
