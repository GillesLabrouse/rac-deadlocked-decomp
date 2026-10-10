/* cflags: -mno-split-addresses */
/* asflags: --float-immediates */
#include "common.h"

/* L01 class/table initializer, 00586B10..00586E80 (880 bytes).
 * ABI: (actor, full-width class ID, skipCleanup). Cleanup helpers receive
 * actor only; m2c's additional leftover arguments are not consumed inputs.
 * Complete PS2 reconstruction retains real model and cleanup callees.
 * Host execution supports only paths that need neither subsystem.
 * Outstanding dependencies:
 * 0058B268: release bookkeeping for actor->58 in live matrix/model pool.
 * 0058ACB8: cleanup of model-related actor state.
 * 0058B148: allocate model records using definition byte 8 and 00222D78..94.
 * 0058A648: scratchpad/vector model update via 0058A440 and 00589950.
 * These functions are never replaced with success callbacks in the host path.
 */
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_class.h"
#include <string.h>
static inline u16 class_u16(const u8 *p) { u16 v; memcpy(&v,p,2); return v; }
static inline u32 class_u32(const u8 *p) { u32 v; memcpy(&v,p,4); return v; }
static inline void class_set16(u8 *p,u16 v) { memcpy(p,&v,2); }
static inline void class_set32(u8 *p,u32 v) { memcpy(p,&v,4); }
#else
static inline u16 class_u16(const u8 *p) { return *(const u16 *)p; }
static inline u32 class_u32(const u8 *p) { return *(const u32 *)p; }
static inline void class_set16(u8 *p,u16 v) { *(u16 *)p=v; }
static inline void class_set32(u8 *p,u32 v) { *(u32 *)p=v; }
void func_L01_0058B268(u8 *);
void func_L01_0058ACB8(u8 *);
void func_L01_0058B148(u8 *);
void func_L01_0058A648(u8 *);
void func_L01_0055D2A8(void *,u32,s32);
void func_L01_00589210(u8 *);
extern u8 D_0026B590[];
extern s16 D_0026B2B0[];
extern u32 D_0026DD90[];
extern u32 D_0026AE00[];
extern u32 D_0026AA40[];
#endif

typedef char class_word_size_check[sizeof(u32) == 4 ? 1 : -1];
typedef char class_half_size_check[sizeof(u16) == 2 ? 1 : -1];

static inline void class_defaults(u8 *actor,s32 objectClass,u8 index,u8 preserved) {
    actor[0x2D]=preserved;
    actor[0x6D]=0xFF;
    class_set16(actor+0xBC,(u16)objectClass);
    actor[0x2C]=0xFF;
    actor[0x21]=0xFF;
    actor[0x56]=0xFF;
    actor[0x64]=0xFF;
    actor[0x65]=0xFF;
    class_set16(actor+0x94,0);
    actor[0x6C]=0;
    actor[0x22]=index;
    class_set16(actor+0x6E,0xFFFF);
    actor[0xA7]=0x80;
    actor[0x23]=0x80;
    actor[0xA6]=0x80;
    class_set16(actor+0x2E,0x7F80);
    actor[0xA5]=0x7F;
    actor[0xA4]=0x7F;
}

/* After a reset, the mismatch path uses a handler guest address instead of
 * a class definition. Retail stores it but does not invoke it here. */
static inline void class_mismatch(u8 *actor,u32 handler) {
    u16 flags=class_u16(actor+0x34)|0x45;
    class_set32(actor+0x24,0);
    class_set16(actor+0x34,flags);
    class_set32(actor+0x98,0);
    class_set32(actor+0xA8,handler);
    if (handler==0) class_set16(actor+0x34,flags|2);
}

#ifndef DEADLOCKED_NATIVE
void func_L01_00586B10(u8 *actor,s32 objectClass,s32 skipCleanup) {
    u8 preserved,index;
    u8 *definition;
    u8 *auxiliary;
    u16 flags;
    u32 model;
    if (skipCleanup==0 && class_u32(actor+0x58)!=0) {
        func_L01_0058B268(actor);
        func_L01_0058ACB8(actor);
    }
    if (actor[0x20]==0xFF) actor[0x2D]=0;
    preserved=actor[0x2D];
    func_L01_0055D2A8(actor,0,0x100);
    index=D_0026B590[objectClass];
    class_defaults(actor,objectClass,index,preserved);
    func_L01_00589210(actor);
    index=actor[0x22];
    if (D_0026B2B0[index]!=objectClass) {
        class_mismatch(actor,D_0026DD90[index]);
        return;
    }
    model=D_0026AE00[index];
    class_set32(actor+0xA8,model);
    if (model==0) class_set16(actor+0x34,class_u16(actor+0x34)|2);
    definition=(u8 *)D_0026AA40[actor[0x22]];
    class_set32(actor+0x24,(u32)definition);
    actor[0x64]=definition[0xE];
    flags=class_u16(actor+0x34)|class_u16(definition+0x44);
    class_set16(actor+0x34,flags);
    class_set32(actor+0x98,class_u32(definition+0x10));
    class_set32(actor+0x70,class_u32(definition+0x24));
    if (definition[0xA]!=0 && (s8)definition[0xE]>=0) actor[0x65]=(u8)(definition[0xE]<<1);
    class_set16(actor+0x4C,1);
    class_set32(actor+0x48,0x3F800000U);
    definition=(u8 *)class_u32(actor+0x24);
    actor[0x57]=definition[8];
    if (definition[8]!=0) {
        func_L01_0058B148(actor);
        definition=(u8 *)class_u32(actor+0x24);
        if (class_u32(definition+0x1C)!=0 && class_u32((u8 *)class_u32(definition+0x1C))!=0) {
            class_set32(actor+0x48,0);
            class_set16(actor+0x34,class_u16(actor+0x34)|0x40);
        } else {
            auxiliary=(u8 *)class_u32(actor+0x98);
            if (auxiliary!=NULL && (s16)class_u16(auxiliary+2)!=0) {
                class_set32(actor+0x48,0);
                class_set16(actor+0x34,class_u16(actor+0x34)|0x40);
            } else {
                actor[0x55]|=0x40;
            }
        }
    } else {
        /* The zero-count branch targets 00586D38, not 00586D48. */
        class_set32(actor+0x48,0);
        class_set16(actor+0x34,class_u16(actor+0x34)|0x40);
    }
    definition=(u8 *)class_u32(actor+0x24);
    if (class_u32(definition+0x40)!=0) {
        class_set16(actor+0x34,class_u16(actor+0x34)|0x10);
        class_set32(actor+0x60,class_u32(definition+0x40));
    }
    definition=(u8 *)class_u32(actor+0x24);
    if (definition[0xF]!=0) {
        actor[0x76]=0x18;
        class_set16(actor+0x34,class_u16(actor+0x34)|0x400);
        class_set32(actor+0x78,0);
        class_set32(actor+0x7C,0);
        actor[0x77]=0;
    }
    definition=(u8 *)class_u32(actor+0x24);
    if (definition[6]!=0) actor[0x66]=0x18;
    definition=(u8 *)class_u32(actor+0x24);
    auxiliary=(u8 *)class_u32(definition+0x48);
    if (auxiliary!=NULL) {
        class_set32(actor+0x40,(u32)auxiliary);
        actor[0x54]=0;
        actor[0x6C]=auxiliary[0x12];
        actor[0x6D]=auxiliary[0x11];
        if (auxiliary[0x10]>=2) class_set16(actor+0x34,class_u16(actor+0x34)&0xFFFD);
        definition=(u8 *)class_u32(actor+0x24);
        if (definition[0xC]==1) {
            auxiliary=(u8 *)class_u32(definition+0x48);
            if (auxiliary[0x10]<2) {
                class_set32(actor+0x48,0);
                if ((s8)auxiliary[0x11]<0) class_set16(actor+0x34,class_u16(actor+0x34)|0x40);
            }
        }
    }
    if (class_u32(actor+0x58)!=0) func_L01_0058A648(actor);
    definition=(u8 *)class_u32(actor+0x24);
    class_set16(actor+0x36,class_u16(actor+0x36)|definition[0x47]);
}
#else
const u8 *dreadzone_class_translate(const DreadzoneClassContext *context,u32 address,u32 bytes) {
    u32 i,offset;
    const u8 *result=NULL;
    if (context==NULL || context->views==NULL || context->view_count>32) return NULL;
    for (i=0;i<context->view_count;i++) {
        const DreadzoneClassView *view=&context->views[i];
        if (view->storage==NULL || view->guest_base>0xFFFFFFFFU-view->bytes) return NULL;
        if (address<view->guest_base) continue;
        offset=address-view->guest_base;
        if (offset<=view->bytes && bytes<=view->bytes-offset) {
            if (result!=NULL) return NULL; /* Ambiguous guest mapping. */
            result=view->storage+offset;
        }
    }
    return result;
}

int dreadzone_actor_initialize_class_slice(const DreadzoneClassContext *context,
    u8 *actor,u32 actorBytes,s32 objectClass,s32 skipCleanup) {
    const u8 *indexPointer,*mapped,*handle,*modelPointer,*definitionPointer,*definition=NULL,*auxiliary=NULL;
    const u8 *environment,*entry;
    u32 definitionAddress=0,auxiliaryAddress=0,model=0,handler=0,upper=0;
    u32 entryAddress,clearOffset;
    u16 flags;
    u8 index,preserved,component=0;
    if (actor==NULL || actorBytes<0x100 || objectClass<0
        || (u32)objectClass>0xFFFFFFFFU-0x0026B590U) return DREADZONE_CLASS_INVALID;
    if (skipCleanup==0 && class_u32(actor+0x58)!=0) return DREADZONE_CLASS_NEEDS_CLEANUP;
    indexPointer=dreadzone_class_translate(context,0x0026B590U+(u32)objectClass,1);
    if (indexPointer==NULL) return DREADZONE_CLASS_INVALID;
    index=*indexPointer;
    mapped=dreadzone_class_translate(context,0x0026B2B0U+index*2U,2);
    if (mapped==NULL) return DREADZONE_CLASS_INVALID;
    if ((s16)class_u16(mapped)!=objectClass) {
        handle=dreadzone_class_translate(context,0x0026DD90U+index*4U,4);
        if (handle==NULL) return DREADZONE_CLASS_INVALID;
        handler=class_u32(handle);
    } else {
        modelPointer=dreadzone_class_translate(context,0x0026AE00U+index*4U,4);
        definitionPointer=dreadzone_class_translate(context,0x0026AA40U+index*4U,4);
        if (modelPointer==NULL || definitionPointer==NULL) return DREADZONE_CLASS_INVALID;
        model=class_u32(modelPointer);
        definitionAddress=class_u32(definitionPointer);
        if (definitionAddress==0) return DREADZONE_CLASS_INVALID;
        definition=dreadzone_class_translate(context,definitionAddress,0x4C);
        if (definition==NULL) return DREADZONE_CLASS_INVALID;
        if (definition[8]!=0) return DREADZONE_CLASS_NEEDS_MODEL_SETUP;
        auxiliaryAddress=class_u32(definition+0x48);
        if (auxiliaryAddress!=0) {
            auxiliary=dreadzone_class_translate(context,auxiliaryAddress,0x13);
            if (auxiliary==NULL) return DREADZONE_CLASS_INVALID;
        }
    }
    if (context->environment!=0) {
        environment=dreadzone_class_translate(context,context->environment,2);
        if (environment==NULL || context->environment>0xFFFFFFFFU-class_u16(environment)) return DREADZONE_CLASS_INVALID;
        entryAddress=context->environment+class_u16(environment);
        entry=dreadzone_class_translate(context,entryAddress,0x4B);
        if (entry==NULL) return DREADZONE_CLASS_INVALID;
        upper=class_u32(entry+0x40);
        component=entry[0x4A];
    }
    preserved=actor[0x20]==0xFF ? 0 : actor[0x2D];
    /* Actual 0055D2A8 writes words and rounds positive byte counts up to four.
     * This fixed positive 256-byte request performs exactly 64 word stores.
     * memcpy inside class_set32 also permits unaligned host actor storage. */
    for (clearOffset=0;clearOffset<0x100;clearOffset+=4)
        class_set32(actor+clearOffset,0);
    class_defaults(actor,objectClass,index,preserved);
    dreadzone_actor_initialize_environment(actor,upper,component,context->environment!=0);
    if (definition==NULL) {
        class_mismatch(actor,handler);
        return DREADZONE_CLASS_INITIALIZED;
    }
    class_set32(actor+0xA8,model);
    flags=model==0 ? 2 : 0;
    class_set32(actor+0x24,definitionAddress);
    actor[0x64]=definition[0xE];
    flags|=class_u16(definition+0x44);
    class_set16(actor+0x34,flags);
    class_set32(actor+0x98,class_u32(definition+0x10));
    class_set32(actor+0x70,class_u32(definition+0x24));
    if (definition[0xA]!=0 && (s8)definition[0xE]>=0) actor[0x65]=(u8)(definition[0xE]<<1);
    class_set16(actor+0x4C,1);
    class_set32(actor+0x48,0x3F800000U);
    actor[0x57]=0;
    /* Retail's zero-model-count branch resets the earlier 1.0 value and
     * adds flag 0x40 before joining the remaining table setup. */
    class_set32(actor+0x48,0);
    class_set16(actor+0x34,class_u16(actor+0x34)|0x40);
    if (class_u32(definition+0x40)!=0) {
        class_set16(actor+0x34,class_u16(actor+0x34)|0x10);
        class_set32(actor+0x60,class_u32(definition+0x40));
    }
    if (definition[0xF]!=0) {
        actor[0x76]=0x18;
        class_set16(actor+0x34,class_u16(actor+0x34)|0x400);
        class_set32(actor+0x78,0);
        class_set32(actor+0x7C,0);
        actor[0x77]=0;
    }
    if (definition[6]!=0) actor[0x66]=0x18;
    if (auxiliary!=NULL) {
        class_set32(actor+0x40,auxiliaryAddress);
        actor[0x54]=0;
        actor[0x6C]=auxiliary[0x12];
        actor[0x6D]=auxiliary[0x11];
        if (auxiliary[0x10]>=2) class_set16(actor+0x34,class_u16(actor+0x34)&0xFFFD);
        if (definition[0xC]==1 && auxiliary[0x10]<2) {
            class_set32(actor+0x48,0);
            if ((s8)auxiliary[0x11]<0) class_set16(actor+0x34,class_u16(actor+0x34)|0x40);
        }
    }
    class_set16(actor+0x36,class_u16(actor+0x36)|definition[0x47]);
    return DREADZONE_CLASS_INITIALIZED;
}
#endif
