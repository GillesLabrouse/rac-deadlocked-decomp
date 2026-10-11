#include "common.h"

/* Actual L01 00576BC0..00576EF8, 824 bytes. Relocates serialized class data
 * in place. This operation is not idempotent. Native geometry conversion uses
 * the recovered packing routines through the same ordered transaction. */
#ifndef DEADLOCKED_NATIVE
typedef unsigned int ClassRemapQuad __attribute__((mode(TI)));
/* a2/a3 are retained at the call site but are active callee inputs. */
void func_L01_00576AA8(u8 *,u8 *,u8 *,s32);
extern u32 D_001DFD1C __attribute__((section(".data")));
extern u8 D_002699A0[],D_0026B590[],D_0026EFB0[];
static inline u32 relocate_word(u8 *p,u32 base) {
    u32 value=*(u32 *)p;
    if (value!=0) *(u32 *)p=value+base;
    return value==0 ? 0 : value+base;
}
void func_L01_00576BC0(u8 *definition,u8 *textures,u8 *remap,s32 objectClass) {
    u32 base=(u32)definition,value;
    u8 *records,*record,*auxiliary,*p;
    s32 count,i,high,geometryCount;
    geometryCount=definition[4]+definition[5]+definition[6]+definition[0xA];
    if (definition[0x2C]!=0) geometryCount+=definition[definition[0x2C]*16+1];
    records=(u8 *)relocate_word(definition,base);
    if (records!=NULL) for (i=0;i<geometryCount;i++,records+=16) {
        *(u32 *)records+=base;
        *(u32 *)(records+8)+=base;
    }
    relocate_word(definition+0x10,base);
    relocate_word(definition+0x14,base);
    relocate_word(definition+0x18,base);
    records=(u8 *)relocate_word(definition+0x1C,base);
    if (records!=NULL) {
        count=*(s32 *)records;
        for (i=0;i<count;i++) {
            records=(u8 *)*(u32 *)(definition+0x1C);
            *(u32 *)(records+4+i*4)+=base;
        }
    }
    record=(u8 *)relocate_word(definition+0x20,base);
    if (record!=NULL) {
        do {
            *(u32 *)(record+0xC)+=base;
            p=record;
            while (*p!=0xFF) { *p=remap[*p]; p++; }
            value=*(u32 *)(record+0xC);
            record+=16;
        } while ((s32)value>=0);
    }
    high=definition[0xB]>>4;
    if (definition[0xB]!=0) {
        record=(u8 *)(*(u32 *)(definition+0x20)-((definition[0xB]&15)*high<<10)-16);
        for (i=0;i<high;i++) {
            if (textures!=NULL) *(s16 *)(record+i*2)=(s16)(*(s16 *)(textures+remap[i]*16+0xA)+((s32)D_001DFD1C>>8));
            else *(u16 *)(record+i*2)=(u16)((*(u64 *)(D_002699A0+remap[i]*24)>>37)&0x3FFF);
        }
    }
    relocate_word(definition+0x28,base);
    if (definition[0xC]!=0) for (i=0;i<definition[0xC];i++) {
        auxiliary=(u8 *)relocate_word(definition+0x48+i*4,base);
        if (auxiliary!=NULL) {
            if (auxiliary[0x13]!=0) return;
            if (definition[8]==0 && auxiliary[0x10]>=2) auxiliary[0x10]=1;
            records=(u8 *)relocate_word(auxiliary+0x18,(u32)auxiliary);
            if (records!=NULL) {
                relocate_word(records,(u32)auxiliary);
                records=(u8 *)*(u32 *)(auxiliary+0x18);
                relocate_word(records+4,(u32)auxiliary);
            }
            relocate_word(auxiliary+0x14,(u32)auxiliary);
            relocate_word(auxiliary+0x1C,(u32)auxiliary);
        }
    }
    if (objectClass>=0) *(ClassRemapQuad *)(D_0026EFB0+D_0026B590[objectClass]*16)=*(ClassRemapQuad *)remap;
    records=(u8 *)*(u32 *)definition;
    if (records!=NULL) func_L01_00576AA8(records,textures,remap,geometryCount);
    if (definition[0x47]==0xFF) definition[0x47]=0;
}
#else
#include "../native/dreadzone_class_tables.h"
#include "../native/dreadzone_geometry.h"
#include <stdint.h>
#include <string.h>

/* Journal real writes during preflight, then apply them in retail order.
 * Reads see previous planned writes, including aliases within one definition.
 * The bounded host domain refuses malformed/unbounded tables atomically. */
#define CLASS_WRITE_LIMIT 8192
typedef struct ClassWrite { u8 *target; u64 value; u32 bytes; } ClassWrite;
typedef struct ClassPlan {
    const DreadzoneTableContext *context;
    u32 count;
    ClassWrite writes[CLASS_WRITE_LIMIT];
} ClassPlan;

static int class_context_valid(const DreadzoneTableContext *context) {
    u32 i,j;
    if (context==NULL || context->views==NULL || context->view_count==0 || context->view_count>32) return 0;
    for (i=0;i<context->view_count;i++) {
        const DreadzoneTableView *a=&context->views[i];
        uintptr_t ap=(uintptr_t)a->storage;
        if (a->storage==NULL || a->bytes==0 || a->guest_base>0xFFFFFFFFU-a->bytes || ap>UINTPTR_MAX-a->bytes) return 0;
        for (j=0;j<i;j++) {
            const DreadzoneTableView *b=&context->views[j];
            uintptr_t bp=(uintptr_t)b->storage;
            if (a->guest_base<b->guest_base+b->bytes && b->guest_base<a->guest_base+a->bytes) return 0;
            if (ap<bp+b->bytes && bp<ap+a->bytes) return 0;
        }
    }
    return 1;
}
static u8 *class_address(ClassPlan *plan,u32 address,u32 bytes) {
    u32 i;
    for (i=0;i<plan->context->view_count;i++) {
        const DreadzoneTableView *view=&plan->context->views[i];
        if (address>=view->guest_base && address-view->guest_base<=view->bytes
            && bytes<=view->bytes-(address-view->guest_base)) return view->storage+address-view->guest_base;
    }
    return NULL;
}
static int class_read64(ClassPlan *plan,u32 address,u32 bytes,u64 *value) {
    u8 *p=class_address(plan,address,bytes);
    u32 i,j;
    if (p==NULL || bytes==0 || bytes>8) return 0;
    *value=0;
    for (i=0;i<bytes;i++) {
        u32 byte=p[i];
        uintptr_t at=(uintptr_t)(p+i);
        for (j=plan->count;j>0;j--) {
            const ClassWrite *write=&plan->writes[j-1];
            uintptr_t start=(uintptr_t)write->target;
            if (at>=start && at-start<write->bytes) { byte=(u32)((write->value>>((at-start)*8))&255); break; }
        }
        *value|=(u64)byte<<(i*8);
    }
    return 1;
}
static int class_read(ClassPlan *plan,u32 address,u32 bytes,u32 *value) {
    u64 full;
    if (bytes>4 || !class_read64(plan,address,bytes,&full)) return 0;
    *value=(u32)full;
    return 1;
}
static int class_write(ClassPlan *plan,u32 address,u32 bytes,u64 value) {
    u8 *p=class_address(plan,address,bytes);
    ClassWrite *write;
    if (p==NULL || plan->count==CLASS_WRITE_LIMIT || (bytes!=1 && bytes!=2 && bytes!=4 && bytes!=8)) return 0;
    write=&plan->writes[plan->count++];
    write->target=p; write->value=value; write->bytes=bytes;
    return 1;
}
static void class_apply(ClassPlan *plan) {
    u32 i,j;
    for (i=0;i<plan->count;i++) {
        ClassWrite *write=&plan->writes[i];
        u8 bytes[8];
        for (j=0;j<write->bytes;j++) bytes[j]=(u8)(write->value>>(j*8));
        memcpy(write->target,bytes,write->bytes);
    }
}
static int class_geometry_read(void *context,u32 address,u32 bytes,u64 *value) {
    return class_read64((ClassPlan *)context,address,bytes,value);
}
static int class_geometry_plan(ClassPlan *plan,u32 headers,u32 textures,u32 remap,u32 count) {
    DreadzoneGeometryView views[32];
    DreadzoneGeometryContext context;
    DreadzoneGeometryPlan *geometry=NULL;
    u64 i,total,value;
    u32 address,bytes,j;
    int result;
    for (j=0;j<plan->context->view_count;j++) {
        views[j].guest_base=plan->context->views[j].guest_base;
        views[j].bytes=plan->context->views[j].bytes;
        views[j].storage=plan->context->views[j].storage;
    }
    context.views=views; context.view_count=plan->context->view_count;
    result=dreadzone_geometry_prepare_with_reader(&context,class_geometry_read,plan,
        headers,textures,remap,(s32)count,&geometry);
    if (result!=DREADZONE_GEOMETRY_OK) {
        return result==DREADZONE_GEOMETRY_UNMAPPED ? DREADZONE_TABLE_NEEDS_GEOMETRY : DREADZONE_TABLE_INVALID;
    }
    total=dreadzone_geometry_write_count(geometry);
    if (total>CLASS_WRITE_LIMIT-plan->count) result=DREADZONE_TABLE_INVALID;
    else {
        result=DREADZONE_TABLE_COMPLETED;
        for (i=0;i<total;i++) {
            if (!dreadzone_geometry_write_at(geometry,i,&address,&bytes,&value) ||
                !class_write(plan,address,bytes,value)) { result=DREADZONE_TABLE_INVALID; break; }
        }
    }
    dreadzone_geometry_destroy(geometry);
    return result;
}
static int class_relocate_word(ClassPlan *plan,u32 address,u32 base,u32 *value) {
    if (!class_read(plan,address,4,value)) return 0;
    if (*value!=0) { *value+=base; if (!class_write(plan,address,4,*value)) return 0; }
    return 1;
}
static int class_dispatch_plan(ClassPlan *plan,s32 objectClass,s32 alternate) {
    u32 record,index,key,callback,metadata,slot;
    u32 counter=alternate!=0 ? 0x00222D1C : 0x00222D18;
    u32 callbacks=alternate!=0 ? 0x0026DD90 : 0x0026AE00;
    u32 indices=alternate!=0 ? 0x0026DF90 : 0x0026B1C0;
    if (!class_read(plan,counter,4,&slot) || slot>=(alternate!=0 ? 128U : 240U)) return 0;
    for (index=0;index<313;index++) {
        record=0x003C5E00+index*12;
        if (!class_read(plan,record,4,&key)) return 0;
        if (key==0xFFFFFFFFU || (s32)key==objectClass) break;
    }
    if (index==313 || !class_read(plan,record+4,4,&callback) || !class_read(plan,record+8,4,&metadata)) return 0;
    /* Arithmetic shift before byte truncation, as in retail. */
    index=(u32)(((s32)(metadata-0x003C6CAC))>>5);
    return class_write(plan,callbacks+slot*4,4,callback) && class_write(plan,indices+slot,1,index);
}
static int class_definition_plan(ClassPlan *plan,u32 base,u32 textures,u32 remap,s32 objectClass) {
    u32 count=0,geometryCount,value,records,record,auxiliary,p,i,j,byte,high,low,geometry,quad[4];
    int geometryResult;
    for (i=4;i<=6;i++) { if (!class_read(plan,base+i,1,&value)) return 0; count+=value; }
    if (!class_read(plan,base+0xA,1,&value)) return 0;
    count+=value;
    if (!class_read(plan,base+0x2C,1,&value)) return 0;
    if (value!=0) { if (!class_read(plan,base+value*16+1,1,&value)) return 0; count+=value; }
    geometryCount=count;
    if (!class_relocate_word(plan,base,base,&records)) return 0;
    if (records!=0) for (i=0;i<count;i++) {
        record=records+i*16;
        if (!class_read(plan,record,4,&value) || !class_write(plan,record,4,value+base)
            || !class_read(plan,record+8,4,&value) || !class_write(plan,record+8,4,value+base)) return 0;
    }
    for (i=0x10;i<=0x18;i+=4) if (!class_relocate_word(plan,base+i,base,&value)) return 0;
    if (!class_relocate_word(plan,base+0x1C,base,&records)) return 0;
    if (records!=0) {
        if (!class_read(plan,records,4,&count)) return 0;
        if ((s32)count>0) for (i=0;i<count;i++) {
            if (!class_read(plan,base+0x1C,4,&records)) return 0;
            if (!class_read(plan,records+4+i*4,4,&value) || !class_write(plan,records+4+i*4,4,value+base)) return 0;
        }
    }
    if (!class_relocate_word(plan,base+0x20,base,&record)) return 0;
    if (record!=0) {
        do {
            if (!class_read(plan,record+12,4,&value) || !class_write(plan,record+12,4,value+base)) return 0;
            p=record;
            for (;;) {
                if (!class_read(plan,p,1,&byte)) return 0;
                if (byte==255) break;
                if (!class_read(plan,remap+byte,1,&byte) || !class_write(plan,p,1,byte)) return 0;
                p++;
            }
            if (!class_read(plan,record+12,4,&value)) return 0;
            record+=16;
        } while ((s32)value>=0);
    }
    if (!class_read(plan,base+0xB,1,&byte)) return 0;
    high=byte>>4; low=byte&15;
    if (byte!=0) {
        if (!class_read(plan,base+0x20,4,&record)) return 0;
        record-=((low*high)<<10)+16;
        for (i=0;i<high;i++) {
            if (!class_read(plan,remap+i,1,&byte)) return 0;
            if (textures!=0) {
                if (!class_read(plan,textures+byte*16+10,2,&value) || !class_read(plan,0x001DFD1C,4,&j)) return 0;
                value=(u32)((s16)value+((s32)j>>8));
            } else {
                if (!class_read(plan,0x002699A0+byte*24+4,4,&value)) return 0;
                value=(value>>5)&0x3FFF;
            }
            if (!class_write(plan,record+i*2,2,value)) return 0;
        }
    }
    if (!class_relocate_word(plan,base+0x28,base,&value) || !class_read(plan,base+0xC,1,&count)) return 0;
    for (i=0;;i++) {
        /* Retail reloads this byte at 00576E84, including null entries.
         * Auxiliary relocation can alias and change the definition itself. */
        if (!class_read(plan,base+0xC,1,&count)) return 0;
        if (i>=count) break;
        if (!class_relocate_word(plan,base+0x48+i*4,base,&auxiliary)) return 0;
        if (auxiliary==0) continue;
        if (!class_read(plan,auxiliary+0x13,1,&value)) return 0;
        if (value!=0) return DREADZONE_TABLE_COMPLETED; /* Genuine early return. */
        if (!class_read(plan,base+8,1,&value)) return 0;
        if (value==0) {
            if (!class_read(plan,auxiliary+0x10,1,&value)) return 0;
            if (value>=2 && !class_write(plan,auxiliary+0x10,1,1)) return 0;
        }
        if (!class_relocate_word(plan,auxiliary+0x18,auxiliary,&records)) return 0;
        if (records!=0) {
            if (!class_relocate_word(plan,records,auxiliary,&value)
                || !class_read(plan,auxiliary+0x18,4,&records)
                || !class_relocate_word(plan,records+4,auxiliary,&value)) return 0;
        }
        if (!class_relocate_word(plan,auxiliary+0x14,auxiliary,&value)
            || !class_relocate_word(plan,auxiliary+0x1C,auxiliary,&value)) return 0;
    }
    if (objectClass>=0) {
        if (objectClass>=10240 || !class_read(plan,0x0026B590+(u32)objectClass,1,&byte) || byte>=240) return 0;
        /* The original LQ captures the aligned quad before SQ writes it.
         * Byte remapping above and in geometry still uses the exact pointer. */
        for (i=0;i<4;i++) if (!class_read(plan,(remap&~15U)+i*4,4,&quad[i])) return 0;
        for (i=0;i<4;i++) if (!class_write(plan,0x0026EFB0+byte*16+i*4,4,quad[i])) return 0;
    }
    if (!class_read(plan,base,4,&geometry)) return 0;
    if (geometry!=0) {
        geometryResult=class_geometry_plan(plan,geometry,textures,remap,geometryCount);
        if (geometryResult!=DREADZONE_TABLE_COMPLETED) return geometryResult;
    }
    if (!class_read(plan,base+0x47,1,&value)) return 0;
    if (value==255 && !class_write(plan,base+0x47,1,0)) return 0;
    return DREADZONE_TABLE_COMPLETED;
}

int dreadzone_class_reset_table_slice(const DreadzoneTableContext *context) {
    static const u32 ranges[][3]={
        {0x0026B590,0x2800,0xFFFFFFFFU},{0x0026B2B0,0x1E0,0xFFFFFFFFU},
        {0x0026EFB0,0xF00,0xFFFFFFFFU},{0x0026E830,0xF0,0},
        {0x00353D40,0x800,0xFFFFFFFFU},{0x00353940,0x80,0},
        {0x0034CB20,0x400,0xFFFFFFFFU},{0x0034C520,0x40,0}
    };
    ClassPlan plan;
    u32 i,j;
    if (!class_context_valid(context)) return 0;
    plan.context=context; plan.count=0;
    for (i=0;i<8;i++) for (j=0;j<ranges[i][1];j+=4)
        if (!class_write(&plan,ranges[i][0]+j,4,ranges[i][2])) return 0;
    for (i=128;i>0;i--) if (!class_write(&plan,0x0028DA20+(i-1)*4,4,0)) return 0;
    class_apply(&plan);
    return DREADZONE_TABLE_COMPLETED;
}
int dreadzone_class_publish_dispatch(const DreadzoneTableContext *context,s32 objectClass,s32 alternate) {
    ClassPlan plan;
    if (!class_context_valid(context) || objectClass<0 || objectClass>=10240) return 0;
    plan.context=context; plan.count=0;
    if (!class_dispatch_plan(&plan,objectClass,alternate)) return 0;
    class_apply(&plan);
    return DREADZONE_TABLE_COMPLETED;
}
int dreadzone_class_relocate_definition(const DreadzoneTableContext *context,u32 definition,u32 textures,u32 remap,s32 objectClass) {
    ClassPlan plan;
    int result;
    if (!class_context_valid(context) || definition==0) return 0;
    plan.context=context; plan.count=0;
    result=class_definition_plan(&plan,definition,textures,remap,objectClass);
    if (result==DREADZONE_TABLE_COMPLETED) class_apply(&plan);
    return result;
}
int dreadzone_class_publish_definition(const DreadzoneTableContext *context,u32 definition,u32 textures,u32 remap,s32 objectClass) {
    ClassPlan plan;
    u32 slot,value,counter;
    int result;
    if (!class_context_valid(context) || objectClass<0 || objectClass>=10240) return 0;
    plan.context=context; plan.count=0;
    counter=definition==0 ? 0x00222D1C : 0x00222D18;
    if (!class_read(&plan,counter,4,&slot) || slot>=(definition==0 ? 128U : 240U)) return 0;
    if (definition==0 && !class_dispatch_plan(&plan,objectClass,1)) return 0;
    if (!class_write(&plan,0x0026B590+(u32)objectClass,1,slot)
        || !class_write(&plan,(definition==0 ? 0x0026B490 : 0x0026B2B0)+slot*2,2,(u32)objectClass)) return 0;
    if (definition!=0) {
        if (!class_write(&plan,0x0026AA40+slot*4,4,definition) || !class_read(&plan,definition+0x2D,1,&value)) return 0;
        value=value==255 ? 0x00100000 : value<<10;
        if (!class_write(&plan,0x0026E830+slot*4,4,value)) return 0;
        result=class_definition_plan(&plan,definition,textures,remap,objectClass);
        if (result!=DREADZONE_TABLE_COMPLETED) return result;
        if (!class_dispatch_plan(&plan,objectClass,0)) return 0;
        /* Relocation may have aliased the real counter: reload like retail. */
        if (!class_read(&plan,counter,4,&slot)) return 0;
    }
    if (!class_write(&plan,counter,4,slot+1)) return 0;
    class_apply(&plan);
    return DREADZONE_TABLE_COMPLETED;
}
#endif
