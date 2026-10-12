#include "dreadzone_client_history_copy.h"

static int guest_span(const DreadzoneClientHistoryCopyMemory *r, uint32_t a, size_t n, uint8_t **p) {
    uint64_t end=(uint64_t)a+n, rend=(uint64_t)r->guest_base+r->length;
    if (!n || a<r->guest_base || end>0x80000000ULL || end>rend) return 0;
    *p=r->bytes+(size_t)(a-r->guest_base); return 1;
}
static int guest_overlap(uint32_t a,size_t an,uint32_t b,size_t bn) {
    return (uint64_t)a < (uint64_t)b+bn && (uint64_t)b < (uint64_t)a+an;
}
static int host_overlap(uintptr_t a,size_t an,uintptr_t b,size_t bn) {
    return a < b+bn && b < a+an;
}
int dreadzone_client_history_copy_00535200(const DreadzoneClientHistoryCopyMemory *r, uint32_t index) {
    uintptr_t rb, req; uint32_t row,hd,bs,bd,hs; uint8_t *rp,*hp,*dp,*sp,*bp,*pub;
    uint64_t end; size_t i; uint32_t table;
    req=(uintptr_t)r;
    if (!r || req>UINTPTR_MAX-sizeof(*r)) return -1;
    if (!r->bytes || index>=4 || !r->length || r->length>0x1000000U || r->guest_base>=0x80000000U) return -1;
    end=(uint64_t)r->guest_base+r->length; if(end>0x80000000ULL) return -1;
    rb=(uintptr_t)r->bytes;
    if (rb>UINTPTR_MAX-r->length || req>UINTPTR_MAX-sizeof(*r) || host_overlap(rb,r->length,req,sizeof(*r))) return -1;
    row=0x230280U+index*0x470U; hd=0x231810U+index*0xB0U; bs=0x23C010U+index*0x780U; bd=0x23C510U+index*0x780U;
    table=0x231AD0U+index*0x2100U;
    if(!guest_span(r,row+0x50,4,&rp)||!guest_span(r,hd,0xB0,&dp)||!guest_span(r,hd+0x70,4,&pub)||!guest_span(r,bs,0x280,&sp)||!guest_span(r,bd,0x280,&bp)) return -1;
    hs=(uint32_t)rp[0]|((uint32_t)rp[1]<<8)|((uint32_t)rp[2]<<16)|((uint32_t)rp[3]<<24);
    if(hs<table || hs>=table+48U*0xB0U || ((hs-table)%0xB0U)!=0) return -1;
    if ((hs|hd|bs|bd)&15U) return -1;
    if (!guest_span(r,hs,0xC0,&hp)) return -1; /* Actual B0 loop reads C0. */
    /* Preserve original sequential-copy semantics only on independent copy domains. */
    if(guest_overlap(hs,0xC0,hd,0xB0)||guest_overlap(bs,0x280,bd,0x280) ||
       guest_overlap(hs,0xC0,bs,0x280)||guest_overlap(hs,0xC0,bd,0x280)||
       guest_overlap(hd,0xB0,bs,0x280)||guest_overlap(hd,0xB0,bd,0x280)) return -1;
    /* The second copy reads and stores both quads of its final pair. */
    for(i=0;i<0xB0;i++) dp[i]=hp[i];
    for(i=0;i<0x280;i++) bp[i]=sp[i];
    pub[0]=(uint8_t)bd; pub[1]=(uint8_t)(bd>>8); pub[2]=(uint8_t)(bd>>16); pub[3]=(uint8_t)(bd>>24);
    return 0;
}
