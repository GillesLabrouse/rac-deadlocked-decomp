/* cflags: -mno-split-addresses */
/* Complete CPU snapshot0066A2A0..0066A500,608 bytes. NONMATCHING.
 * Guest vtable+48 is the actual dispatch, not a supplied host callback. */
#include "common.h"
void func_L01_0055D3B8(void *, const void *, u32);
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_snapshot.h"
static u32 snapshot_read(const u8 *p) {
    return (u32)p[0] | ((u32)p[1]<<8) | ((u32)p[2]<<16) | ((u32)p[3]<<24);
}
static void snapshot_write(u8 *p,u32 v) {
    p[0]=(u8)v; p[1]=(u8)(v>>8); p[2]=(u8)(v>>16); p[3]=(u8)(v>>24);
}
static int snapshot_overlap(const u8 *a,u32 an,const u8 *b,u32 bn) {
    uintptr_t aa=(uintptr_t)a,bb=(uintptr_t)b;
    return aa<=bb ? bb-aa<an : aa-bb<bn;
}
/* Preflight reads cannot precede an aliased retail write. The supported
 * bounded domain excludes any such read intersecting the snapshot footprint. */
static int snapshot_read_alias(u32 client_guest,u32 address,u32 bytes) {
    u32 start=client_guest+0x150;
    return address>=start ? address-start<0x154 : start-address<bytes;
}
#define WORD(o) snapshot_read(record+(o))
#define HALF(o) ((u16)record[o] | ((u16)record[(o)+1]<<8))
#define SIGNED_HALF(o) ((s32)((HALF(o)^0x8000U)-0x8000U))
#define STORE(o,v) snapshot_write(snapshot+(o),(u32)(v))
#define STORE_HALF(o,v) do { u16 sh_value=(u16)(v); snapshot[o]=(u8)sh_value; snapshot[(o)+1]=(u8)(sh_value>>8); } while(0)
#define QUAD(dst,src) do { u32 q; for(q=0;q<16;q++) client[dst+q]=record[src+q]; } while(0)
int dreadzone_actor_0066A2A0(const DreadzoneSnapshotContext *host,
                          u8 *client,u32 client_guest,u32 client_bytes) {
    const u8 *record,*entry=0;
    u8 *snapshot;
    u32 i,table,handler;
    s32 getter,predicate,status;
    if (!host || !host->record || host->record_bytes<0x31F0 || (host->record_guest&15) ||
        host->record_guest>0xFFFFFFFFU-(host->record_bytes-1)) return DREADZONE_SNAPSHOT_INVALID_STORAGE;
    record=host->record;
    if (!client || !client_guest || (client_guest&15) || client_bytes<0x2A4 ||
        client_guest>0xFFFFFFFFU-(client_bytes-1) || client_guest!=WORD(0x2EFC))
        return DREADZONE_SNAPSHOT_NEEDS_CLIENT;
    if (snapshot_overlap(record,host->record_bytes,client,client_bytes) ||
        (client_guest>=host->record_guest ? client_guest-host->record_guest<host->record_bytes : host->record_guest-client_guest<client_bytes))
        return DREADZONE_SNAPSHOT_INVALID_STORAGE;
    if (host->mode_bytes_00171DE0 && host->mode_count &&
        (snapshot_overlap(host->mode_bytes_00171DE0,host->mode_count,client,client_bytes) ||
         (client_guest>=0x00171DE0 ? client_guest-0x00171DE0<host->mode_count : 0x00171DE0-client_guest<client_bytes)))
        return DREADZONE_SNAPSHOT_INVALID_STORAGE;
    if (host->view_count && !host->views) return DREADZONE_SNAPSHOT_NEEDS_VTABLE;
    for(i=0;i<host->view_count;i++) {
        const DreadzoneSnapshotMemory *v=&host->views[i];
        if (!v->storage || !v->bytes || v->guest_base>0xFFFFFFFFU-(v->bytes-1))
            return DREADZONE_SNAPSHOT_INVALID_STORAGE;
        if (v->storage==client && v->guest_base==client_guest && v->bytes==client_bytes) continue;
        if (snapshot_overlap(v->storage,v->bytes,client,client_bytes) ||
            (v->guest_base>=client_guest ? v->guest_base-client_guest<client_bytes : client_guest-v->guest_base<v->bytes))
            return DREADZONE_SNAPSHOT_INVALID_STORAGE;
    }
    table=WORD(0x14);
    if (!table || (table&3) || table>0xFFFFFFFFU-0x4B) return DREADZONE_SNAPSHOT_NEEDS_VTABLE;
    if(snapshot_read_alias(client_guest,table+0x48,4)) return DREADZONE_SNAPSHOT_INVALID_STORAGE;
    for(i=0;i<host->view_count;i++) {
        const DreadzoneSnapshotMemory *v=&host->views[i];
        u32 address=table+0x48;
        if (address>=v->guest_base && v->bytes>=4 && address-v->guest_base<=v->bytes-4) {
            if (entry) return DREADZONE_SNAPSHOT_NEEDS_VTABLE;
            entry=v->storage+(address-v->guest_base);
        }
    }
    if (!entry) return DREADZONE_SNAPSHOT_NEEDS_VTABLE;
    handler=snapshot_read(entry);
    if (handler==0x006A7D28) {
        u32 object=WORD(0x2EF4);
        if(object && object<=0xFFFFFFFFU-0x2B3 && snapshot_read_alias(client_guest,object+0x2B0,4))
            return DREADZONE_SNAPSHOT_INVALID_STORAGE;
        status=dreadzone_actor_006A7D28(host,&getter);
        if(status) return status;
    } else if (handler==0x00690E68) getter=func_L01_00690E68(record);
    else return DREADZONE_SNAPSHOT_UNKNOWN_GETTER;
    status=dreadzone_actor_0068DE60(host,&predicate);
    if(status) return status;
    if(host->mode_bytes_00171DE0[WORD(0x2F10)] && !WORD(0x3F4) &&
       host->state_0021DDB4!=2 && host->state_0021DDB4!=6 && record[0x266D]) {
        u32 related=snapshot_read(client+0x50);
        if(related && related<=0xFFFFFFFFU-0x87 && snapshot_read_alias(client_guest,related+0x86,2))
            return DREADZONE_SNAPSHOT_INVALID_STORAGE;
    }
    snapshot=client+0x150;
#else
#define WORD(o) (*(const u32 *)(record+(o)))
#define HALF(o) (*(const u16 *)(record+(o)))
#define SIGNED_HALF(o) (*(const s16 *)(record+(o)))
#define STORE(o,v) (*(u32 *)(snapshot+(o))=(u32)(v))
#define STORE_HALF(o,v) (*(u16 *)(snapshot+(o))=(u16)(v))
typedef struct SnapshotQuad { u32 words[4]; } SnapshotQuad;
#define QUAD(dst,src) (*(SnapshotQuad *)(client+(dst))=*(const SnapshotQuad *)(record+(src)))
s32 func_L01_0068DE60(const u8 *);
void func_L01_0066A2A0(const u8 *record) {
    u8 *client=*(u8 **)(record+0x2EFC);
    u8 *snapshot=client+0x150;
    s32 getter,predicate;
#endif
    QUAD(0x150,0x20); QUAD(0x160,0x30); QUAD(0x170,0x40); QUAD(0x180,0x50);
    QUAD(0x190,0xA0); QUAD(0x1A0,0xB0); QUAD(0x1B0,0xF0); QUAD(0x1C0,0x190);
    QUAD(0x1D0,0x270); QUAD(0x1E0,0x1D90); QUAD(0x1F0,0xD0); QUAD(0x230,0x19A0);
    func_L01_0055D3B8(client+0x200,record+0x1970,0x30);
    STORE(0xF0,WORD(0x2EF0));
#ifndef DEADLOCKED_NATIVE
    getter=((s32 (*)(const u8 *))(*(u32 *)(WORD(0x14)+0x48)))(record);
#endif
    STORE(0xF4,getter);
    STORE(0xF8,WORD(0x2EC)); STORE(0xFC,WORD(0x2DDC));
    STORE(0x100,WORD(0x25CC)==0x99 && WORD(0x25D8)==0x76 ? WORD(0x25D8) : WORD(0x25CC));
    STORE(0x104,WORD(0x25D4)); STORE(0x108,WORD(0x25D8));
    STORE(0x10C,WORD(0x1B0)); STORE(0x110,WORD(0x1B4));
    STORE(0x114,WORD(0x2CC)); STORE(0x118,WORD(0x2E0)); STORE(0x11C,WORD(0x2F0));
    STORE_HALF(0x120,HALF(0x2FC)); STORE_HALF(0x122,HALF(0x2FE)); STORE_HALF(0x124,HALF(0x2F8));
    STORE_HALF(0x126,HALF(0x5F4)); STORE_HALF(0x128,HALF(0x1D1E));
    snapshot[0x12A]=record[0x1D0E];
    if(WORD(0x25CC)!=0x31) snapshot[0x12B]=0;
    snapshot[0x12C]=1; snapshot[0x133]=record[0x266A];
#ifndef DEADLOCKED_NATIVE
    predicate=func_L01_0068DE60(record);
#endif
    snapshot[0x12D]=(u8)predicate;
    snapshot[0x12E]=record[0x435]; snapshot[0x12F]=record[0x439];
    snapshot[0x130]=record[0x436]; snapshot[0x131]=record[0x43A]; snapshot[0x132]=record[0x434];
    STORE(0x134,WORD(0x2F00)); STORE(0x144,SIGNED_HALF(0x3D8)); STORE(0x148,WORD(0x1DC4));
    if(WORD(0x25D4)==0xE) {
        STORE(0x138,WORD(0x1ED4)); STORE(0x13C,WORD(0x1EEC)); STORE(0x140,WORD(0x1EE0));
    }
    if(WORD(0x25D4)==0xD && !WORD(0x1AE0) && WORD(0x1EA4)) STORE(0x138,WORD(0x1EA4));
    if(WORD(0x25D4)==0x1A) {
        STORE(0x150,WORD(0x560)); STORE(0x14C,WORD(0x570));
    } else if(WORD(0x25D4)==0xF) {
        STORE(0x150,WORD(0x4A0)); STORE(0x14C,WORD(0x4B0));
    } else { STORE(0x14C,0); STORE(0x150,0); }
#ifdef DEADLOCKED_NATIVE
    return DREADZONE_SNAPSHOT_COMPLETED;
#endif
}
