/* Complete L01 retail CPU routine; baseline SN build, draft-only coverage. */
/* Nonmatching: compiled276 bytes versus296 retail bytes. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_equipment.h"
#define EQ_RWORD(p,o) dreadzone_setup_word((p)+(o))
#define EQ_WWORD(p,o,value) dreadzone_setup_store32((p)+(o),(u32)(value))
#define EQ_READ(address) dreadzone_setup_read32(&ctx->inventory,(address))
#define EQ_BYTE(address) (*dreadzone_equipment_mutable(ctx,(address),1))
#define EQ_MWORD(address) dreadzone_equipment_mutable_word(ctx,(address))
#define EQ_STORE(address,value) dreadzone_equipment_store_word(ctx,(address),(u32)(value))
#define EQ_INVENTORY(record) dreadzone_equipment_inventory(ctx,(record))
#define EQ_SETUP_PASS &ctx->inventory,
#else
#define EQ_CTX
#define EQ_ONLY void
#define EQ_PASS
#define EQ_ARG
#define EQ_RWORD(p,o) (*(volatile u32 *)((p)+(o)))
#define EQ_WWORD(p,o,value) (*(volatile u32 *)((p)+(o))=(u32)(value))
#define EQ_READ(address) (*(volatile u32 *)(address))
#define EQ_BYTE(address) (*(volatile u8 *)(address))
#define EQ_MWORD(address) (*(volatile u32 *)(address))
#define EQ_STORE(address,value) (*(volatile u32 *)(address)=(u32)(value))
#define EQ_INVENTORY(record) (*(u8 **)((record)+0x26E4))
#define EQ_SETUP_PASS
#endif
#ifndef DEADLOCKED_NATIVE
s32 func_L01_00688578(u8 *,s32);
void func_L01_005E9258(s32);
s32 func_L01_006C0A58(u8 *,s32,s32);
void func_L01_006BE0D0(u8 *,s32,s32);
#endif
void func_L01_00690B60(EQ_CTX u8 *record) {
    s32 i;
    for(i=0;i<4;++i) {
        u32 marker=0x00171B69U+(u32)i;
        u32 selector=0x00220810U+(u32)i*4U;
        if(!EQ_BYTE(marker)) continue;
        if(EQ_MWORD(selector)==EQ_BYTE(marker)) EQ_STORE(selector,2);
        if(EQ_RWORD(record,0x24B0)==EQ_BYTE(marker)) EQ_WWORD(record,0x24B0,2);
        if(EQ_BYTE(marker-2)==EQ_BYTE(marker)) EQ_BYTE(marker-2)=2;
        if(EQ_RWORD(record,0x2318)==EQ_BYTE(marker)) {
            (void)func_L01_00688578(EQ_PASS record,2);
            EQ_WWORD(record,0x268C,0);
        }
        func_L01_005E9258(EQ_PASS i);
        if(EQ_RWORD(record,0x268C)==EQ_BYTE(marker)) EQ_WWORD(record,0x268C,0);
        if(EQ_RWORD(record,0x2674)==EQ_BYTE(marker)) {
            EQ_WWORD(record,0x2674,2);
            EQ_WWORD(record,0x268C,0);
        }
        (void)func_L01_006C0A58(EQ_PASS EQ_INVENTORY(record),EQ_BYTE(marker),1);
        func_L01_006BE0D0(EQ_SETUP_PASS EQ_INVENTORY(record),EQ_BYTE(marker),-1);
    }
}
#ifdef DEADLOCKED_NATIVE
static int equipment_host_span(const void *p,uintptr_t bytes) {
    return p && (uintptr_t)p<=UINTPTR_MAX-bytes;
}
static int equipment_host_overlap(const void *a,u32 an,const void *b,u32 bn) {
    uintptr_t aa=(uintptr_t)a,bb=(uintptr_t)b;
    return aa>=bb?aa-bb<bn:bb-aa<an;
}
static int equipment_guest_overlap(u32 a,u32 an,u32 b,u32 bn) {
    return a>=b?a-b<bn:b-a<an;
}
static void equipment_region(const DreadzoneEquipmentContext *ctx,u32 index,
                             const u8 **storage,u32 *guest,u32 *bytes) {
    const DreadzoneInventorySetupContext *v=&ctx->inventory;
    if(index==0) { *storage=v->record;*guest=v->record_guest;*bytes=v->record_bytes; }
    else if(index==1) { *storage=v->pool.storage;*guest=v->pool.guest_address;*bytes=v->pool.bytes; }
    else if(index-2<v->read_count) {
        const DreadzoneInventorySetupReadMemory *r=v->reads+index-2;
        *storage=r->storage;*guest=r->guest_address;*bytes=r->bytes;
    } else {
        const DreadzoneEquipmentMemory *r=ctx->mutable_views+index-2-v->read_count;
        *storage=r->storage;*guest=r->guest_address;*bytes=r->bytes;
    }
}
static int equipment_valid(const DreadzoneEquipmentContext *ctx) {
    const DreadzoneInventorySetupContext *v;
    const u8 *a,*b;
    u32 i,j,ag,bg,an,bn,read_bytes,mutable_bytes;
    if(!equipment_host_span(ctx,sizeof(*ctx))) return 0;
    v=&ctx->inventory;
    if(v->read_count>64 || ctx->mutable_count>64) return 0;
    read_bytes=v->read_count*(u32)sizeof(*v->reads);
    mutable_bytes=ctx->mutable_count*(u32)sizeof(*ctx->mutable_views);
    if((read_bytes && !equipment_host_span(v->reads,read_bytes)) ||
       (mutable_bytes && !equipment_host_span(ctx->mutable_views,mutable_bytes))) return 0;
    if((read_bytes && equipment_host_overlap(ctx,sizeof(*ctx),v->reads,read_bytes)) ||
       (mutable_bytes && equipment_host_overlap(ctx,sizeof(*ctx),ctx->mutable_views,mutable_bytes)) ||
       (read_bytes && mutable_bytes && equipment_host_overlap(v->reads,read_bytes,ctx->mutable_views,mutable_bytes))) return 0;
    if(v->record_bytes<0x31F0 || (v->record_guest&3) ||
       (u64)v->record_guest+v->record_bytes>0x80000000ULL ||
       v->pool.guest_address!=0x001D3F90 || v->pool.bytes<0x12B0 ||
       (u64)v->pool.guest_address+v->pool.bytes>=0x80000000ULL) return 0;
    for(i=0;i<2+v->read_count+ctx->mutable_count;++i) {
        equipment_region(ctx,i,&a,&ag,&an);
        if(!an || !equipment_host_span(a,an) || ag>0xFFFFFFFFU-(an-1)) return 0;
        if(equipment_host_overlap(ctx,sizeof(*ctx),a,an) ||
           (read_bytes && equipment_host_overlap(v->reads,read_bytes,a,an)) ||
           (mutable_bytes && equipment_host_overlap(ctx->mutable_views,mutable_bytes,a,an))) return 0;
        for(j=0;j<i;++j) {
            equipment_region(ctx,j,&b,&bg,&bn);
            if(equipment_host_overlap(a,an,b,bn) || equipment_guest_overlap(ag,an,bg,bn)) return 0;
        }
    }
    return 1;
}
static const u8 *equipment_read_map(const DreadzoneEquipmentContext *ctx,u32 address,u32 bytes) {
    u32 i;
    if(!bytes || address>0xFFFFFFFFU-(bytes-1)) return 0;
    for(i=0;i<ctx->inventory.read_count;++i) {
        const DreadzoneInventorySetupReadMemory *r=ctx->inventory.reads+i;
        if(address>=r->guest_address && bytes<=r->bytes && address-r->guest_address<=r->bytes-bytes)
            return r->storage+(address-r->guest_address);
    }
    return 0;
}
static u8 *equipment_mutable_map(const DreadzoneEquipmentContext *ctx,u32 address,u32 bytes) {
    u32 i;
    if(!bytes || address>0xFFFFFFFFU-(bytes-1)) return 0;
    for(i=0;i<ctx->mutable_count;++i) {
        const DreadzoneEquipmentMemory *r=ctx->mutable_views+i;
        if(address>=r->guest_address && bytes<=r->bytes && address-r->guest_address<=r->bytes-bytes)
            return r->storage+(address-r->guest_address);
    }
    return 0;
}
u8 *dreadzone_equipment_mutable(const DreadzoneEquipmentContext *ctx,u32 address,u32 bytes) {
    u8 *p=equipment_mutable_map(ctx,address,bytes);
    if(!p) dreadzone_setup_unavailable();
    return p;
}
u32 dreadzone_equipment_mutable_word(const DreadzoneEquipmentContext *ctx,u32 address) {
    return dreadzone_setup_word(dreadzone_equipment_mutable(ctx,address,4));
}
void dreadzone_equipment_store_word(const DreadzoneEquipmentContext *ctx,u32 address,u32 value) {
    dreadzone_setup_store32(dreadzone_equipment_mutable(ctx,address,4),value);
}
static u8 *equipment_pool_map(const DreadzoneEquipmentContext *ctx,const u8 *record) {
    const DreadzoneInventoryMemory *pool=&ctx->inventory.pool;
    u32 address=dreadzone_setup_word(record+0x26E4);
    if((address&15) || address<pool->guest_address || pool->bytes<0x12B0 ||
       address-pool->guest_address>pool->bytes-0x12B0) return 0;
    return pool->storage+(address-pool->guest_address);
}
u8 *dreadzone_equipment_inventory(const DreadzoneEquipmentContext *ctx,const u8 *record) {
    u8 *p=equipment_pool_map(ctx,record);
    if(!p) dreadzone_setup_unavailable();
    return p;
}
static int equipment_selection_preflight(const DreadzoneEquipmentContext *ctx,s32 item,int in_maintenance) {
    const u8 *mode,*level,*definition;
    s32 category;
    mode=equipment_read_map(ctx,0x0021E824,4);
    if(!mode) return DREADZONE_EQUIPMENT_NEEDS_MEMORY;
    if(dreadzone_setup_word(mode)==0) {
        level=equipment_read_map(ctx,0x0021E828,4);
        if(!level) return DREADZONE_EQUIPMENT_NEEDS_MEMORY;
        if(dreadzone_setup_word(level)==0 && func_L01_006BDDC0(item))
            return DREADZONE_EQUIPMENT_SELECTION_REJECTED;
    }
    if(!func_L01_006BE040(item)) return DREADZONE_EQUIPMENT_SELECTION_REJECTED;
    definition=equipment_read_map(ctx,0x003BF460+(u32)item*0xB0,0xB0);
    if(!definition) return DREADZONE_EQUIPMENT_NEEDS_MEMORY;
    category=(s32)(definition[0x2C]^0x80U)-0x80;
    /* Retail reads 266D after the category word store. Category -2 overwrites
     * 266C..266F: all accepted items have a zero second byte, clearing 266D. */
    if(category!=-2 && ctx->inventory.record[0x266D]) return DREADZONE_EQUIPMENT_UNSUPPORTED_UI;
    /* A setter overwrite of 26E4 would change the subsequent real inventory
     * reload to item 2, outside the declared inventory pool. Refuse atomically. */
    if(in_maintenance && equipment_guest_overlap(0x2674+(u32)(category*4),4,0x26E4,4))
        return DREADZONE_EQUIPMENT_UNSAFE_FIELD_ALIAS;
    return DREADZONE_EQUIPMENT_COMPLETED;
}
int dreadzone_equipment_00688578(const DreadzoneEquipmentContext *ctx,s32 item) {
    int status;
    if(!equipment_valid(ctx)) return DREADZONE_EQUIPMENT_INVALID_STORAGE;
    status=equipment_selection_preflight(ctx,item,0);
    if(status) return status;
    (void)func_L01_00688578(ctx,ctx->inventory.record,item);
    return DREADZONE_EQUIPMENT_COMPLETED;
}
int dreadzone_equipment_006C0A58(const DreadzoneEquipmentContext *ctx,s32 item,s32 restore_slots) {
    u8 *inventory;
    if(!equipment_valid(ctx)) return DREADZONE_EQUIPMENT_INVALID_STORAGE;
    if(item<0 || item>=32) return DREADZONE_EQUIPMENT_UNSUPPORTED_ITEM;
    inventory=equipment_pool_map(ctx,ctx->inventory.record);
    if(!inventory) return DREADZONE_EQUIPMENT_NEEDS_INVENTORY;
    (void)func_L01_006C0A58(ctx,inventory,item,restore_slots);
    return DREADZONE_EQUIPMENT_COMPLETED;
}
int dreadzone_equipment_00690C88(const DreadzoneEquipmentContext *ctx) {
    if(!equipment_valid(ctx)) return DREADZONE_EQUIPMENT_INVALID_STORAGE;
    if(!equipment_mutable_map(ctx,0x00171B69,4)) return DREADZONE_EQUIPMENT_NEEDS_MEMORY;
    func_L01_00690C88(ctx);
    return DREADZONE_EQUIPMENT_COMPLETED;
}
int dreadzone_equipment_00690B60(const DreadzoneEquipmentContext *ctx) {
    const u8 *record;
    u8 *marker;
    u32 i;
    int status;
    if(!equipment_valid(ctx)) return DREADZONE_EQUIPMENT_INVALID_STORAGE;
    record=ctx->inventory.record;
    if(!equipment_mutable_map(ctx,0x00171B67,6)) return DREADZONE_EQUIPMENT_NEEDS_MEMORY;
    for(i=0;i<4;++i) {
        marker=equipment_mutable_map(ctx,0x00171B69+i,1);
        if(!*marker) continue;
        if(*marker>=32) return DREADZONE_EQUIPMENT_UNSUPPORTED_ITEM;
        if(!equipment_pool_map(ctx,record)) return DREADZONE_EQUIPMENT_NEEDS_INVENTORY;
        if(!equipment_read_map(ctx,0x00171B66,1) ||
           !equipment_mutable_map(ctx,0x00220810+i*4,4) ||
           !equipment_mutable_map(ctx,0x0020C690+i*0xC,0xC) ||
           !equipment_mutable_map(ctx,0x003301E0+i*0xB8,4)) return DREADZONE_EQUIPMENT_NEEDS_MEMORY;
        if(dreadzone_setup_word(record+0x2318)==*marker) {
            status=equipment_selection_preflight(ctx,2,1);
            if(status) return status;
        }
    }
    func_L01_00690B60(ctx,ctx->inventory.record);
    return DREADZONE_EQUIPMENT_COMPLETED;
}
#endif
