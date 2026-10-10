#include "dreadzone_equipment_selection.h"
#include <stdlib.h>
#include <string.h>

#define SELECTION_MAX_REGIONS 194
#define SELECTION_MAX_BYTES (16U*1024U*1024U)
typedef struct SelectionRegion {
    const u8 *original;
    u8 *copy;
    u32 address,bytes;
    int writable;
} SelectionRegion;

static void selection_region(const DreadzoneEquipmentUiContext *ctx,u32 i,
                             SelectionRegion *r) {
    const DreadzoneEquipmentContext *e=ctx->equipment;
    const DreadzoneInventorySetupContext *v=&e->inventory;
    r->writable=1;r->copy=0;
    if(i==0) {r->original=v->record;r->address=v->record_guest;r->bytes=v->record_bytes;}
    else if(i==1) {r->original=v->pool.storage;r->address=v->pool.guest_address;r->bytes=v->pool.bytes;}
    else if(i-2<v->read_count) {
        const DreadzoneInventorySetupReadMemory *p=v->reads+i-2;
        r->original=p->storage;r->address=p->guest_address;r->bytes=p->bytes;r->writable=0;
    } else if(i-2-v->read_count<e->mutable_count) {
        const DreadzoneEquipmentMemory *p=e->mutable_views+i-2-v->read_count;
        r->original=p->storage;r->address=p->guest_address;r->bytes=p->bytes;
    } else {
        const DreadzoneEquipmentMemory *p=ctx->extra_writes+i-2-v->read_count-e->mutable_count;
        r->original=p->storage;r->address=p->guest_address;r->bytes=p->bytes;
    }
}
static const u8 *selection_map(const SelectionRegion *regions,u32 count,u32 address,u32 bytes) {
    u32 i;
    if(!bytes || address>0xFFFFFFFFU-(bytes-1)) return 0;
    for(i=0;i<count;++i) {
        const SelectionRegion *r=regions+i;
        if(address>=r->address && bytes<=r->bytes && address-r->address<=r->bytes-bytes)
            return (r->copy?r->copy:r->original)+(address-r->address);
    }
    return 0;
}
int dreadzone_equipment_selection_00688578(const DreadzoneEquipmentUiContext *ctx,s32 item) {
    SelectionRegion regions[SELECTION_MAX_REGIONS];
    DreadzoneInventorySetupReadMemory reads[64];
    DreadzoneEquipmentMemory writes[64],extra[64];
    DreadzoneEquipmentContext shadow;
    DreadzoneEquipmentUiContext ui;
    const u8 *p,*definition;
    u32 i,count,total=0,mode,level=0,table,handler;
    s32 category,index;
    int status;
    /* Item1 stops the public UI preflight before all stores. This genuinely
     * validates every descriptor/backing and the required fresh UI state. */
    status=dreadzone_equipment_ui_005D7558(ctx,0,1,1);
    if(status!=DREADZONE_EQUIPMENT_UI_ITEM_REJECTED) return status;
    if((ctx->equipment->inventory.record_guest!=0x00368F60 &&
        ctx->equipment->inventory.record_guest!=0x0036C150) ||
       !ctx->equipment->inventory.record[0x266D])
        return DREADZONE_EQUIPMENT_SELECTION_UNSUPPORTED_ACTOR;
    count=2+ctx->equipment->inventory.read_count+ctx->equipment->mutable_count+ctx->extra_write_count;
    for(i=0;i<count;++i) {
        selection_region(ctx,i,regions+i);
        if(regions[i].bytes>SELECTION_MAX_BYTES-total)
            return DREADZONE_EQUIPMENT_SELECTION_NO_TRANSACTION_MEMORY;
        total+=regions[i].bytes;
    }
    p=selection_map(regions,count,0x0021E824,4);
    if(!p) return DREADZONE_EQUIPMENT_SELECTION_NEEDS_MEMORY;
    mode=dreadzone_setup_word(p);
    if(!mode) {
        p=selection_map(regions,count,0x0021E828,4);
        if(!p) return DREADZONE_EQUIPMENT_SELECTION_NEEDS_MEMORY;
        level=dreadzone_setup_word(p);
    }
    if((!mode && !level && func_L01_006BDDC0(item)) || !func_L01_006BE040(item))
        return DREADZONE_EQUIPMENT_SELECTION_TRANSACTION_REJECTED;
    /* 006BF200 genuinely uses inventory read views, not writable regions. */
    definition=0;
    for(i=0;i<ctx->equipment->inventory.read_count;++i) {
        const DreadzoneInventorySetupReadMemory *r=ctx->equipment->inventory.reads+i;
        u32 address=0x003BF460U+(u32)item*0xB0U;
        if(address>=r->guest_address && r->bytes>=0xB0 && address-r->guest_address<=r->bytes-0xB0)
            definition=r->storage+(address-r->guest_address);
    }
    if(!definition) return DREADZONE_EQUIPMENT_SELECTION_NEEDS_MEMORY;
    category=(s8)definition[0x2C];
    if(category!=0) return DREADZONE_EQUIPMENT_SELECTION_UNSUPPORTED_CATEGORY;
    table=dreadzone_setup_word(ctx->equipment->inventory.record+0x14);
    if(table&3) return DREADZONE_EQUIPMENT_SELECTION_INVALID_STORAGE;
    if(table>0xFFFFFFFFU-0x4C) return DREADZONE_EQUIPMENT_SELECTION_NEEDS_MEMORY;
    p=selection_map(regions,count,table+0x4C,4);
    if(!p) return DREADZONE_EQUIPMENT_SELECTION_NEEDS_MEMORY;
    handler=dreadzone_setup_word(p);
    if(handler!=0x006A9630 && handler!=0x00690EA0)
        return DREADZONE_EQUIPMENT_SELECTION_UNSUPPORTED_HANDLER;
    /* Exact guest aliases are represented by the original validated mapping.
     * New host buffers and descriptors never overlap caller writable memory. */
    for(i=0;i<count;++i) {
        regions[i].copy=(u8 *)malloc(regions[i].bytes);
        if(!regions[i].copy) {
            status=DREADZONE_EQUIPMENT_SELECTION_NO_TRANSACTION_MEMORY;
            goto discard;
        }
        memcpy(regions[i].copy,regions[i].original,regions[i].bytes);
    }
    shadow=*ctx->equipment;ui=*ctx;
    shadow.inventory.record=regions[0].copy;
    shadow.inventory.pool.storage=regions[1].copy;
    for(i=0;i<shadow.inventory.read_count;++i) {
        reads[i]=shadow.inventory.reads[i];reads[i].storage=regions[2+i].copy;
    }
    for(i=0;i<shadow.mutable_count;++i) {
        writes[i]=shadow.mutable_views[i];writes[i].storage=regions[2+shadow.inventory.read_count+i].copy;
    }
    for(i=0;i<ui.extra_write_count;++i) {
        extra[i]=ui.extra_writes[i];extra[i].storage=regions[2+shadow.inventory.read_count+shadow.mutable_count+i].copy;
    }
    shadow.inventory.reads=reads;shadow.mutable_views=writes;
    ui.equipment=&shadow;ui.extra_writes=extra;
    /* Genuine selected setter order: lookup, store, flag read, virtual read,
     * handler call, UI call with force1. No successful helper is captured. */
    definition=func_L01_006BF200(&shadow.inventory,item,0);
    category=(s8)definition[0x2C];
    dreadzone_setup_store32(shadow.inventory.record+0x2674+category*4,(u32)item);
    if(shadow.inventory.record[0x266D]) {
        table=dreadzone_setup_word(shadow.inventory.record+0x14);
        if(table&3) {status=DREADZONE_EQUIPMENT_SELECTION_INVALID_STORAGE;goto discard;}
        p=selection_map(regions,count,table+0x4C,4);
        if(!p) {status=DREADZONE_EQUIPMENT_SELECTION_NEEDS_MEMORY;goto discard;}
        handler=dreadzone_setup_word(p);
        if(handler!=0x006A9630 && handler!=0x00690EA0) {
            status=DREADZONE_EQUIPMENT_SELECTION_UNSUPPORTED_HANDLER;goto discard;
        }
        index=handler==0x006A9630?func_L01_006A9630(shadow.inventory.record):
                                  func_L01_00690EA0(shadow.inventory.record);
        status=dreadzone_equipment_ui_005D7558(&ui,index,item,1);
        if(status<0) goto discard;
    }
    /* Retail ignores the UI predicate's rejected-item return. */
    for(i=0;i<count;++i) if(regions[i].writable)
        memcpy((u8 *)regions[i].original,regions[i].copy,regions[i].bytes);
    status=DREADZONE_EQUIPMENT_SELECTION_COMPLETED;
discard:
    for(i=0;i<count;++i) free(regions[i].copy);
    return status;
}
