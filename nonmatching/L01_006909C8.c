/* Complete CPU routine recovered from the actual L01 retail overlay.
 * Draft-only source, outside matching coverage. Uses baseline SN flags. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_inventory_setup.h"
#include <stdlib.h>
#define SETUP_READ8(address) dreadzone_setup_read8(ctx,(address))
#define SETUP_READ32(address) dreadzone_setup_read32(ctx,(address))
#define SETUP_TABLE(address) dreadzone_setup_table(ctx,(address))
#define SETUP_HALF(p) dreadzone_setup_half(p)
#define SETUP_STORE16(p,value) dreadzone_setup_store16((p),(u16)(value))
#define SETUP_STORE32(p,value) dreadzone_setup_store32((p),(u32)(value))
#else
#define SETUP_CTX
#define SETUP_ONLY void
#define SETUP_PASS
#define SETUP_ARG
#define SETUP_READ8(address) (*(const u8 *)(address))
#define SETUP_READ32(address) (*(const u32 *)(address))
#define SETUP_TABLE(address) ((const u8 *)(address))
#define SETUP_HALF(p) (*(const s16 *)(p))
#define SETUP_STORE16(p,value) (*(u16 *)(p)=(u16)(value))
#define SETUP_STORE32(p,value) (*(u32 *)(p)=(u32)(value))
#endif
void func_L01_006BF2D0(SETUP_CTX u8 *inventory,s32 item,s32 level,s32 preserve_amount,s32 force_slot);
void func_L01_006C01F8(SETUP_CTX u8 *inventory,s32 slot,s32 cost,s32 item);
#ifndef DEADLOCKED_NATIVE
extern u8 D_001D3F90[];
void func_L01_006C0D88(u8 *inventory);
#define SETUP_POOL(index) (D_001D3F90+(u32)(index)*0x12B0U)
#define SETUP_RESET(inventory) func_L01_006C0D88(inventory)
#define SETUP_RECORD_INDEX(record) (*(const s32 *)((record)+0x2F10))
#define SETUP_BIND(record,inventory) (*(u8 **)((record)+0x26E4)=(inventory))
#else
#define SETUP_POOL(index) (ctx->pool.storage+(u32)(index)*0x12B0U)
#define SETUP_RESET(inventory) dreadzone_setup_reset(ctx,(inventory))
#define SETUP_RECORD_INDEX(record) ((s32)dreadzone_setup_word((record)+0x2F10))
#define SETUP_BIND(record,inventory) dreadzone_setup_store32((record)+0x26E4,ctx->pool.guest_address+(u32)((inventory)-ctx->pool.storage))
#endif
void func_L01_006909C8(SETUP_CTX u8 *record) {
    u8 *inventory;
    if(SETUP_READ32(0x0021E824U)==1) {
        inventory=SETUP_POOL(SETUP_RECORD_INDEX(record));
        SETUP_RESET(inventory);
        SETUP_BIND(record,inventory);
        return;
    }
    inventory=SETUP_POOL(0);
    if(!inventory[0]) {
        SETUP_RESET(inventory);
        func_L01_006BF2D0(SETUP_PASS inventory,2,0,0,0);
        func_L01_006BF2D0(SETUP_PASS inventory,3,0,0,0);
        func_L01_006BF2D0(SETUP_PASS inventory,12,0,0,0);
        if(SETUP_READ32(0x0021E824U)==0 && SETUP_READ32(0x0021E828U)==1)
            func_L01_006BF2D0(SETUP_PASS inventory,7,0,0,0);
        func_L01_006BF2D0(SETUP_PASS inventory,14,0,0,0);
        func_L01_006BF2D0(SETUP_PASS inventory,1,0,0,0);
        func_L01_006C01F8(SETUP_PASS inventory,(s32)SETUP_READ32(0x0021DF64U)>=3?4:2,-1,0);
    }
    SETUP_BIND(record,inventory);
}
#ifdef DEADLOCKED_NATIVE
static int setup_host_span(const void *storage,uintptr_t bytes) {
    return storage && (uintptr_t)storage<=UINTPTR_MAX-bytes;
}
static int setup_host_overlap(const void *a,u32 an,const void *b,u32 bn) {
    uintptr_t aa=(uintptr_t)a,bb=(uintptr_t)b;
    return aa>=bb?aa-bb<bn:bb-aa<an;
}
static int setup_guest_overlap(u32 a,u32 an,u32 b,u32 bn) {
    return a>=b?a-b<bn:b-a<an;
}
static const u8 *setup_map(const DreadzoneInventorySetupContext *ctx,u32 address,u32 bytes) {
    u32 i;
    if(!bytes || address>0xFFFFFFFFU-(bytes-1)) return 0;
    for(i=0;i<ctx->read_count;++i) {
        const DreadzoneInventorySetupReadMemory *v=ctx->reads+i;
        if(address>=v->guest_address && bytes<=v->bytes && address-v->guest_address<=v->bytes-bytes)
            return v->storage+(address-v->guest_address);
    }
    return 0;
}
u32 dreadzone_setup_word(const u8 *p) {
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}
s32 dreadzone_setup_half(const u8 *p) {
    return (s32)(((u32)p[0]|((u32)p[1]<<8))^0x8000U)-0x8000;
}
void dreadzone_setup_store16(u8 *p,u16 value) {
    p[0]=(u8)value;p[1]=(u8)(value>>8);
}
void dreadzone_setup_store32(u8 *p,u32 value) {
    p[0]=(u8)value;p[1]=(u8)(value>>8);p[2]=(u8)(value>>16);p[3]=(u8)(value>>24);
}
void dreadzone_setup_unavailable(void) {
    /* Fail closed on misuse of an internal helper; never return a fake effect. */
    abort();
}
const u8 *dreadzone_setup_table(const DreadzoneInventorySetupContext *ctx,u32 address) {
    const u8 *p=setup_map(ctx,address,1);
    if(!p) dreadzone_setup_unavailable();
    return p;
}
u8 dreadzone_setup_read8(const DreadzoneInventorySetupContext *ctx,u32 address) {
    return *dreadzone_setup_table(ctx,address);
}
u32 dreadzone_setup_read32(const DreadzoneInventorySetupContext *ctx,u32 address) {
    const u8 *p=setup_map(ctx,address,4);
    if(!p) dreadzone_setup_unavailable();
    return dreadzone_setup_word(p);
}
void dreadzone_setup_reset(const DreadzoneInventorySetupContext *ctx,u8 *inventory) {
    DreadzoneInventoryMemory selected={inventory,
        ctx->pool.guest_address+(u32)(inventory-ctx->pool.storage),0x12B0};
    if(dreadzone_inventory_006C0D88(&selected)!=DREADZONE_INVENTORY_RESET_COMPLETED)
        dreadzone_setup_unavailable();
}
static int setup_storage_valid(const DreadzoneInventorySetupContext *ctx) {
    u32 i,j,descriptor_bytes;
    if(!setup_host_span(ctx,sizeof(*ctx))) return 0;
    if(ctx->read_count>64) return 0;
    descriptor_bytes=ctx->read_count*(u32)sizeof(*ctx->reads);
    if(descriptor_bytes && !setup_host_span(ctx->reads,descriptor_bytes)) return 0;
    if(ctx->record_bytes<0x31F0 || (ctx->record_guest&3) ||
       !setup_host_span(ctx->record,ctx->record_bytes) ||
       (u64)ctx->record_guest+ctx->record_bytes>0x80000000ULL ||
       ctx->pool.guest_address!=0x001D3F90 || ctx->pool.bytes<0x12B0 ||
       !setup_host_span(ctx->pool.storage,ctx->pool.bytes) ||
       (u64)ctx->pool.guest_address+ctx->pool.bytes>=0x80000000ULL) return 0;
    if(setup_host_overlap(ctx->record,ctx->record_bytes,ctx->pool.storage,ctx->pool.bytes) ||
       setup_guest_overlap(ctx->record_guest,ctx->record_bytes,ctx->pool.guest_address,ctx->pool.bytes)) return 0;
    if(setup_host_overlap(ctx,sizeof(*ctx),ctx->record,ctx->record_bytes) ||
       setup_host_overlap(ctx,sizeof(*ctx),ctx->pool.storage,ctx->pool.bytes) ||
       (descriptor_bytes && (setup_host_overlap(ctx->reads,descriptor_bytes,ctx->record,ctx->record_bytes) ||
                            setup_host_overlap(ctx->reads,descriptor_bytes,ctx->pool.storage,ctx->pool.bytes)))) return 0;
    for(i=0;i<ctx->read_count;++i) {
        const DreadzoneInventorySetupReadMemory *a=ctx->reads+i;
        if(!a->bytes || !setup_host_span(a->storage,a->bytes) ||
           a->guest_address>0xFFFFFFFFU-(a->bytes-1)) return 0;
        if(setup_host_overlap(ctx,sizeof(*ctx),a->storage,a->bytes) ||
           setup_host_overlap(ctx->reads,descriptor_bytes,a->storage,a->bytes) ||
           setup_host_overlap(ctx->record,ctx->record_bytes,a->storage,a->bytes) ||
           setup_host_overlap(ctx->pool.storage,ctx->pool.bytes,a->storage,a->bytes) ||
           setup_guest_overlap(ctx->record_guest,ctx->record_bytes,a->guest_address,a->bytes) ||
           setup_guest_overlap(ctx->pool.guest_address,ctx->pool.bytes,a->guest_address,a->bytes)) return 0;
        for(j=0;j<i;++j) {
            const DreadzoneInventorySetupReadMemory *b=ctx->reads+j;
            if(setup_host_overlap(a->storage,a->bytes,b->storage,b->bytes) ||
               setup_guest_overlap(a->guest_address,a->bytes,b->guest_address,b->bytes)) return 0;
        }
    }
    return 1;
}
int dreadzone_inventory_006909C8(const DreadzoneInventorySetupContext *ctx) {
    static const u32 items[]={2,3,12,7,14,1};
    const u8 *p;
    u32 mode,index,i,item;
    if(!setup_storage_valid(ctx)) return DREADZONE_INVENTORY_SETUP_INVALID_STORAGE;
    p=setup_map(ctx,0x0021E824,4);
    if(!p) return DREADZONE_INVENTORY_SETUP_NEEDS_READ_MEMORY;
    mode=dreadzone_setup_word(p);
    if(mode>1) return DREADZONE_INVENTORY_SETUP_UNSUPPORTED_MODE;
    if(mode==1) {
        index=dreadzone_setup_word(ctx->record+0x2F10);
        if(index&0x80000000U || index>ctx->pool.bytes/0x12B0U-1U)
            return DREADZONE_INVENTORY_SETUP_NEEDS_POOL;
    } else if(!ctx->pool.storage[0]) {
        if(!setup_map(ctx,0x0021E828,4) || !setup_map(ctx,0x0021DF64,4) ||
           !setup_map(ctx,0x00171B66,1)) return DREADZONE_INVENTORY_SETUP_NEEDS_READ_MEMORY;
        for(i=0;i<6;++i) {
            item=items[i];
            if(item==7 && dreadzone_setup_read32(ctx,0x0021E828)!=1) continue;
            if(!setup_map(ctx,0x003BF460+item*0xB0,0xB0) ||
               !setup_map(ctx,0x003BE158+item*0x28,0x28) ||
               !setup_map(ctx,0x003BDE38+item*0x28,0x28) ||
               !setup_map(ctx,0x003BDDE8+item*4,4)) return DREADZONE_INVENTORY_SETUP_NEEDS_READ_MEMORY;
            /* Every fresh grant remains at level0; a zero first progression
             * word would clamp to -1 and leave the bounded fresh-item domain. */
            if(!dreadzone_setup_read8(ctx,0x00171B66) &&
               !dreadzone_setup_read32(ctx,0x003BE158+item*0x28))
                return DREADZONE_INVENTORY_SETUP_UNSUPPORTED_TABLE;
        }
    }
    func_L01_006909C8(ctx,ctx->record);
    return DREADZONE_INVENTORY_SETUP_COMPLETED;
}
#endif
