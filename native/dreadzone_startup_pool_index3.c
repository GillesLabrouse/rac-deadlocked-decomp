#include "dreadzone_startup_pool.h"
#include <limits.h>
#include <string.h>

static int guest_span(const DreadzoneStartupPoolMemory *m, uint32_t address, size_t size, uint8_t **out) {
    uint64_t end=(uint64_t)address+(uint64_t)size;
    uint64_t mapped=(uint64_t)m->guest_base+(uint64_t)m->length;
    if (!size || address<m->guest_base || end>UINT64_C(0x80000000) || end>mapped) return 0;
    *out=m->bytes+(size_t)(address-m->guest_base); return 1;
}
static uint32_t read32(const uint8_t *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static void write32(uint8_t *p,uint32_t v) {
    p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);
}
int dreadzone_startup_pool_006671D0_index3(const DreadzoneStartupPoolMemory *request) {
    DreadzoneStartupPoolMemory m; uintptr_t rp,bp; uint8_t *countp,*stridep,*root,*basep,*table,*records=0;
    uint32_t count,stride,i; uint64_t extent;
    if (!request) return -1;
    rp=(uintptr_t)(const void *)request;
    if (rp>UINTPTR_MAX-sizeof(m)) return -1;
    memcpy(&m,request,sizeof(m));
    if (!m.bytes || !m.length || m.length>0x01000000u || m.guest_base>=0x80000000u ||
        (uint64_t)m.guest_base+m.length>0x80000000u) return -1;
    bp=(uintptr_t)(void *)m.bytes;
    if (bp>UINTPTR_MAX-m.length || (rp<bp+m.length && bp<rp+sizeof(m))) return -1;
    if (!guest_span(&m,0x003B0054u,4,&countp) || !guest_span(&m,0x003B009Cu,4,&stridep) ||
        !guest_span(&m,0x0036612Cu,4,&root) || !guest_span(&m,0x00366148u,4,&basep) ||
        !guest_span(&m,0x00366164u,4,&table)) return -1;
    count=read32(countp); stride=read32(stridep);
    if ((int32_t)count>0) {
        if (count>4096u || (stride&3u)) return -1;
        extent=(uint64_t)(count-1u)*(uint64_t)stride+0x18u;
        /* Retail reloads count/stride during iteration. Keep the loop writes
         * disjoint from both globals so those reloads retain their entry values. */
        if (extent>SIZE_MAX || (uint64_t)0x0037F450u+extent>0x003B0054u ||
            !guest_span(&m,0x0037F450u,(size_t)extent,&records)) return -1;
    }
    /* Preserve the original ordered publication before the optional record walk. */
    write32(root,0); write32(basep,0x0037F450u); write32(table,0x0037F450u);
    if ((int32_t)count<=0) return 0;
    for (i=0;i<count;i++) {
        uint8_t *r=records+(size_t)((uint64_t)i*stride);
        write32(r+0x14,0x003B1510u);
        write32(r+0x10,i==0?0u:(uint32_t)(0x0037F450u+(uint64_t)(i-1u)*stride));
        write32(r+0x0C,i+1u==count?0u:(uint32_t)(0x0037F450u+(uint64_t)(i+1u)*stride));
    }
    return 0;
}
