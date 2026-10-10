#include "dreadzone_equipment_ui.h"

static int ui_host_span(const void *p,uintptr_t bytes) {
    return p && (uintptr_t)p<=UINTPTR_MAX-bytes;
}
static int ui_host_overlap(const void *a,u32 an,const void *b,u32 bn) {
    uintptr_t aa=(uintptr_t)a,bb=(uintptr_t)b;
    return aa>=bb?aa-bb<bn:bb-aa<an;
}
static int ui_guest_overlap(u32 a,u32 an,u32 b,u32 bn) {
    return a>=b?a-b<bn:b-a<an;
}
static void ui_region(const DreadzoneEquipmentUiContext *ctx,u32 i,
                      const u8 **storage,u32 *address,u32 *bytes,int *writable) {
    const DreadzoneEquipmentContext *e=ctx->equipment;
    const DreadzoneInventorySetupContext *v=&e->inventory;
    *writable=1;
    if(i==0) {*storage=v->record;*address=v->record_guest;*bytes=v->record_bytes;}
    else if(i==1) {*storage=v->pool.storage;*address=v->pool.guest_address;*bytes=v->pool.bytes;}
    else if(i-2<v->read_count) {
        const DreadzoneInventorySetupReadMemory *r=v->reads+i-2;
        *storage=r->storage;*address=r->guest_address;*bytes=r->bytes;*writable=0;
    } else if(i-2-v->read_count<e->mutable_count) {
        const DreadzoneEquipmentMemory *r=e->mutable_views+i-2-v->read_count;
        *storage=r->storage;*address=r->guest_address;*bytes=r->bytes;
    } else {
        const DreadzoneEquipmentMemory *r=ctx->extra_writes+i-2-v->read_count-e->mutable_count;
        *storage=r->storage;*address=r->guest_address;*bytes=r->bytes;
    }
}
static u32 ui_region_count(const DreadzoneEquipmentUiContext *ctx) {
    return 2+ctx->equipment->inventory.read_count+ctx->equipment->mutable_count+ctx->extra_write_count;
}
static int ui_valid(const DreadzoneEquipmentUiContext *ctx) {
    const DreadzoneEquipmentContext *e;
    const DreadzoneInventorySetupContext *v;
    const void *metadata[5];u32 lengths[5],i,j,count;
    const u8 *a,*b;u32 ag,an,bg,bn;int writable;
    if(!ui_host_span(ctx,sizeof(*ctx))) return 0;
    e=ctx->equipment;if(!ui_host_span(e,sizeof(*e))) return 0;
    v=&e->inventory;
    if(v->read_count>64 || e->mutable_count>64 || ctx->extra_write_count>64) return 0;
    metadata[0]=ctx;lengths[0]=sizeof(*ctx);
    metadata[1]=e;lengths[1]=sizeof(*e);
    metadata[2]=v->reads;lengths[2]=v->read_count*(u32)sizeof(*v->reads);
    metadata[3]=e->mutable_views;lengths[3]=e->mutable_count*(u32)sizeof(*e->mutable_views);
    metadata[4]=ctx->extra_writes;lengths[4]=ctx->extra_write_count*(u32)sizeof(*ctx->extra_writes);
    for(i=0;i<5;++i) {
        if(!lengths[i]) continue;
        if(!ui_host_span(metadata[i],lengths[i])) return 0;
        for(j=0;j<i;++j) if(lengths[j] && ui_host_overlap(metadata[i],lengths[i],metadata[j],lengths[j])) return 0;
    }
    if(v->record_bytes<0x31F0 || (v->record_guest&3) ||
       (u64)v->record_guest+v->record_bytes>0x80000000ULL ||
       v->pool.guest_address!=0x001D3F90 || v->pool.bytes<0x12B0 ||
       (u64)v->pool.guest_address+v->pool.bytes>=0x80000000ULL) return 0;
    count=ui_region_count(ctx);
    for(i=0;i<count;++i) {
        ui_region(ctx,i,&a,&ag,&an,&writable);
        if(!an || !ui_host_span(a,an) || ag>0xFFFFFFFFU-(an-1)) return 0;
        for(j=0;j<5;++j) if(lengths[j] && ui_host_overlap(a,an,metadata[j],lengths[j])) return 0;
        for(j=0;j<i;++j) {
            ui_region(ctx,j,&b,&bg,&bn,&writable);
            if(ui_host_overlap(a,an,b,bn) || ui_guest_overlap(ag,an,bg,bn)) return 0;
        }
    }
    return 1;
}
static const u8 *ui_map(const DreadzoneEquipmentUiContext *ctx,u32 address,u32 bytes,int write) {
    u32 i,guest,size;int writable;const u8 *storage;
    if(!bytes || address>0xFFFFFFFFU-(bytes-1)) return 0;
    for(i=0;i<ui_region_count(ctx);++i) {
        ui_region(ctx,i,&storage,&guest,&size,&writable);
        if((!write || writable) && address>=guest && bytes<=size && address-guest<=size-bytes)
            return storage+(address-guest);
    }
    return 0;
}
static const u8 *ui_definition(const DreadzoneEquipmentUiContext *ctx,u32 item) {
    const DreadzoneInventorySetupContext *v=&ctx->equipment->inventory;
    u32 i,address=0x003BF460U+item*0xB0U;
    for(i=0;i<v->read_count;++i) {
        const DreadzoneInventorySetupReadMemory *r=v->reads+i;
        if(address>=r->guest_address && r->bytes>=0xB0 && address-r->guest_address<=r->bytes-0xB0)
            return r->storage+(address-r->guest_address);
    }
    return 0;
}
const u8 *dreadzone_ui_data(const DreadzoneEquipmentUiContext *ctx,u32 address,u32 bytes) {
    const u8 *p=ui_map(ctx,address,bytes?bytes:1,0);
    if(!p) dreadzone_setup_unavailable();
    return p;
}
u32 dreadzone_ui_read32(const DreadzoneEquipmentUiContext *ctx,u32 address) {
    return dreadzone_setup_word(dreadzone_ui_data(ctx,address,4));
}
u8 dreadzone_ui_read8(const DreadzoneEquipmentUiContext *ctx,u32 address) {
    return *dreadzone_ui_data(ctx,address,1);
}
void dreadzone_ui_write32(const DreadzoneEquipmentUiContext *ctx,u32 address,u32 value) {
    const u8 *p=ui_map(ctx,address,4,1);
    if(!p) dreadzone_setup_unavailable();
    dreadzone_setup_store32((u8 *)p,value);
}
u32 dreadzone_ui_actor(const DreadzoneEquipmentUiContext *ctx,s32 index) {
    /* The already recovered 006658C0 has exactly this guest-address algorithm.
     * Translate its guest32 result only when the caller actually reads fields. */
    (void)ctx;
    return 0x00368F60U+(u32)index*0x31F0U;
}
static int ui_preflight(const DreadzoneEquipmentUiContext *ctx,s32 index,s32 item,s32 force) {
    const u8 *p,*definition,*mode,*level,*queue;
    u32 i,current,after,other,row,actor; s32 count;
    if(!ui_valid(ctx)) return DREADZONE_EQUIPMENT_UI_INVALID_STORAGE;
    if(index<0 || index>1 || item<1 || item>31) return DREADZONE_EQUIPMENT_UI_INVALID_ITEM;
    if(force!=1) return DREADZONE_EQUIPMENT_UI_UNSUPPORTED_FORCE;
    for(i=0;i<2;++i) {
        p=ui_map(ctx,0x00330148U+i*0xB8U+0x24,8,0);
        if(!p) return DREADZONE_EQUIPMENT_UI_NEEDS_MEMORY;
        if(dreadzone_setup_word(p) || dreadzone_setup_word(p+4)) return DREADZONE_EQUIPMENT_UI_UNSUPPORTED_SECONDARY;
    }
    if(item==1) return DREADZONE_EQUIPMENT_UI_ITEM_REJECTED;
    definition=ui_definition(ctx,(u32)item);
    if(!definition) return DREADZONE_EQUIPMENT_UI_NEEDS_MEMORY;
    if(!func_L01_006BDDF0(ctx,item)) return DREADZONE_EQUIPMENT_UI_ITEM_REJECTED;
    if(!definition[0x28]) return DREADZONE_EQUIPMENT_UI_UNSUPPORTED_SECONDARY;
    row=0x00330148U+(u32)index*0xB8U;
    queue=ui_map(ctx,0x0020C690U+(u32)index*0xCU,0xC,1);
    if(!queue || !ui_map(ctx,row+0x98,4,1)) return DREADZONE_EQUIPMENT_UI_NEEDS_MEMORY;
    /* The real common store occurs even when the query suppresses shuffling. */
    current=dreadzone_setup_word(queue);after=(u32)item;
    if(current!=(u32)item) {
        p=ui_map(ctx,0x0031B314U+(u32)index*0x54U,1,0);
        if(!p) return DREADZONE_EQUIPMENT_UI_NEEDS_MEMORY;
        if(!*p) {
            if(current>=32) return DREADZONE_EQUIPMENT_UI_INVALID_ITEM;
            if(!ui_definition(ctx,current)) return DREADZONE_EQUIPMENT_UI_NEEDS_MEMORY;
        }
    }
    mode=ui_map(ctx,0x0021E824,4,0);
    if(!mode) return DREADZONE_EQUIPMENT_UI_NEEDS_MEMORY;
    if(dreadzone_setup_word(mode)!=0) return DREADZONE_EQUIPMENT_UI_COMPLETED;
    level=ui_map(ctx,0x0021E828,4,0);
    if(!level) return DREADZONE_EQUIPMENT_UI_NEEDS_MEMORY;
    if(dreadzone_setup_word(level)!=1) return DREADZONE_EQUIPMENT_UI_COMPLETED;
    other=(u32)(index==0);
    queue=ui_map(ctx,0x0020C690U+other*0xCU,0xC,1);
    if(!queue || !ui_map(ctx,0x003301E0U+other*0xB8U,4,1) ||
       !ui_map(ctx,0x003312F0,4,1) || !ui_map(ctx,0x00331354,4,1)) return DREADZONE_EQUIPMENT_UI_NEEDS_MEMORY;
    if(dreadzone_setup_word(queue)==after) {
        count=func_L01_005EA230(ctx,(s32)other);
        /* The original paired-word copy reads one unused lookahead word for
         * odd word counts. Validate that actual RAM read even though the
         * borrowed forward byte copy preserves only the logical byte effects. */
        if((count&1) && !ui_map(ctx,0x0020C690U+other*0xCU+(u32)count*4U,4,0))
            return DREADZONE_EQUIPMENT_UI_NEEDS_MEMORY;
        actor=dreadzone_ui_actor(ctx,(s32)other);
        p=ui_map(ctx,actor+0x2674,4,0);
        if(!p || (!dreadzone_setup_word(p) && !ui_map(ctx,actor+0x268C,4,0))) return DREADZONE_EQUIPMENT_UI_NEEDS_MEMORY;
    }
    return DREADZONE_EQUIPMENT_UI_COMPLETED;
}
static int ui_run(const DreadzoneEquipmentUiContext *ctx,s32 index,s32 item,s32 force,int wrapper) {
    int status=ui_preflight(ctx,index,item,force);
    if(status) return status;
    if(wrapper) (void)func_L01_005D7558(ctx,index,item,force);
    else (void)func_L01_005E93D8(ctx,index,item,force);
    return DREADZONE_EQUIPMENT_UI_COMPLETED;
}
int dreadzone_equipment_ui_005D7558(const DreadzoneEquipmentUiContext *ctx,s32 index,s32 item,s32 force) {
    return ui_run(ctx,index,item,force,1);
}
int dreadzone_equipment_ui_005E93D8(const DreadzoneEquipmentUiContext *ctx,s32 index,s32 item,s32 force) {
    return ui_run(ctx,index,item,force,0);
}
