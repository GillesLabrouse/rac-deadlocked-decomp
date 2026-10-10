#include "dreadzone_actor_snapshot.h"
#include <stdio.h>
#include <string.h>

static unsigned checks, failures;
#define CHECK(x) do { ++checks; if (!(x)) { ++failures; if (failures < 10) printf("line %d failed: %s\n", __LINE__, #x); } } while (0)
static void word(u8 *p, u32 v) {
    p[0]=(u8)v; p[1]=(u8)(v>>8); p[2]=(u8)(v>>16); p[3]=(u8)(v>>24);
}
static int excluded(u32 action) {
    static const u32 values[5]={0x71,0x31,0x1C,0x8E,0x8F};
    unsigned i;
    for (i=0;i<5;i++) if (action==values[i]) return 1;
    return 0;
}
static u8 record[0x31F0], client[0x100], entity[0x100], modes[4];
static u8 oldrecord[sizeof(record)], oldclient[sizeof(client)], oldentity[sizeof(entity)];
static DreadzoneSnapshotMemory views[3];
static DreadzoneSnapshotContext context;
static void setup(void) {
    memset(record,0xA7,sizeof(record)); memset(client,0x39,sizeof(client)); memset(entity,0xD3,sizeof(entity));
    modes[0]=0; modes[1]=1; modes[2]=2; modes[3]=255;
    word(record+0x2F10,3); word(record+0x3F4,0); word(record+0x25CC,0);
    word(record+0x2EFC,0x1000000); record[0x266D]=1;
    word(client+0x50,0x1100000); entity[0x86]=0; entity[0x87]=0;
    views[0].storage=client; views[0].guest_base=0x1000000; views[0].bytes=sizeof(client);
    views[1].storage=entity; views[1].guest_base=0x1100000; views[1].bytes=sizeof(entity);
    views[2]=views[1];
    context.record=record; context.record_bytes=sizeof(record); context.record_guest=0x1200000;
    context.mode_bytes_00171DE0=modes; context.mode_count=4; context.state_0021DDB4=0;
    context.views=views; context.view_count=2;
}
static void run(int status, s32 expected) {
    s32 output=-12345;
    u8 oldmodes[4];
    memcpy(oldrecord,record,sizeof(record)); memcpy(oldclient,client,sizeof(client));
    memcpy(oldentity,entity,sizeof(entity)); memcpy(oldmodes,modes,sizeof(modes));
    CHECK(dreadzone_actor_0068DE60(&context,&output)==status);
    CHECK(output==(status==DREADZONE_SNAPSHOT_COMPLETED?expected:-12345));
    CHECK(memcmp(record,oldrecord,sizeof(record))==0);
    CHECK(memcmp(client,oldclient,sizeof(client))==0);
    CHECK(memcmp(entity,oldentity,sizeof(entity))==0);
    CHECK(memcmp(modes,oldmodes,sizeof(modes))==0);
}
/* Separate complete-snapshot fixtures, retaining every untouched client byte. */
static u8 snap_client[0x400], snap_expected[0x400], table[0x60], child[0x300];
static DreadzoneSnapshotMemory snap_views[4];
s32 D_002208A8, D_002208AC; /* Actual callback-library globals; callback is not invoked here. */
static u32 readword(const u8 *p) {
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}
static void snapshot_setup(unsigned seed) {
    unsigned i;
    setup();
    for(i=0;i<sizeof(record);i++) record[i]=(u8)(i*17+seed*11+(i>>8));
    for(i=0;i<sizeof(snap_client);i++) snap_client[i]=(u8)(i*7+seed*23);
    memset(table,0,sizeof(table)); memset(child,0x79,sizeof(child));
    word(record+0x2EFC,0x1000000); word(record+0x14,0x3B10C0);
    word(record+0x2EF4,(seed&1)?0x1300000:0); word(child+0x2B0,0x80000100+seed);
    word(record+0x2F10,3); word(record+0x3F4,0); record[0x266D]=1;
    word(snap_client+0x50,0x1100000); word(table+0x48,(seed&2)?0x690E68:0x6A7D28);
    snap_views[0].storage=snap_client; snap_views[0].guest_base=0x1000000; snap_views[0].bytes=sizeof(snap_client);
    snap_views[1]=views[1]; snap_views[2].storage=table; snap_views[2].guest_base=0x3B10C0; snap_views[2].bytes=sizeof(table);
    snap_views[3].storage=child; snap_views[3].guest_base=0x1300000; snap_views[3].bytes=sizeof(child);
    context.views=snap_views; context.view_count=4;
}
static void snapshot_oracle(void) {
    /* Numeric source/destination mapping derived independently from retail stores. */
    static const u16 quadsource[]={0x20,0x30,0x40,0x50,0xA0,0xB0,0xF0,0x190,0x270,0x1D90,0xD0,0x19A0};
    static const u16 quaddest[]={0x150,0x160,0x170,0x180,0x190,0x1A0,0x1B0,0x1C0,0x1D0,0x1E0,0x1F0,0x230};
    static const u16 source[]={0x2EF0,0x2EC,0x2DDC,0x25D4,0x25D8,0x1B0,0x1B4,0x2CC,0x2E0,0x2F0,0x2F00,0x1DC4};
    static const u16 dest[]={0x240,0x248,0x24C,0x254,0x258,0x25C,0x260,0x264,0x268,0x26C,0x284,0x298};
    static const u16 hs[]={0x2FC,0x2FE,0x2F8,0x5F4,0x1D1E};
    static const u16 bs[]={0x1D0E,0x435,0x439,0x436,0x43A,0x434,0x266A};
    static const u16 bd[]={0x27A,0x27E,0x27F,0x280,0x281,0x282,0x283};
    u32 action=readword(record+0x25CC), subtype=readword(record+0x25D4), i;
    u16 signedbits=(u16)((u16)record[0x3D8]|((u16)record[0x3D9]<<8));
    s32 value=readword(record+0x2EF4) && readword(table+0x48)==0x6A7D28?(s32)readword(child+0x2B0):0;
    memcpy(snap_expected,snap_client,sizeof(snap_expected));
    for(i=0;i<12;i++) memcpy(snap_expected+quaddest[i],record+quadsource[i],16);
    memcpy(snap_expected+0x200,record+0x1970,48);
    for(i=0;i<12;i++) memcpy(snap_expected+dest[i],record+source[i],4);
    word(snap_expected+0x244,(u32)value);
    word(snap_expected+0x250,action==0x99 && readword(record+0x25D8)==0x76?0x76:action);
    for(i=0;i<5;i++) memcpy(snap_expected+0x270+i*2,record+hs[i],2);
    for(i=0;i<7;i++) snap_expected[bd[i]]=record[bs[i]];
    if(action!=0x31) snap_expected[0x27B]=0;
    snap_expected[0x27C]=1;
    snap_expected[0x27D]=(u8)((!readword(record+0x3F4) && context.state_0021DDB4!=2 && context.state_0021DDB4!=6 &&
        (record[0x266D]==0 || readword(snap_client+0x50)==0 || ((u16)entity[0x86]|((u16)entity[0x87]<<8))!=5) && !excluded(action))?modes[3]:0);
    word(snap_expected+0x294,signedbits&0x8000?(u32)signedbits|0xFFFF0000U:signedbits);
    if(subtype==0xE) { memcpy(snap_expected+0x288,record+0x1ED4,4); memcpy(snap_expected+0x28C,record+0x1EEC,4); memcpy(snap_expected+0x290,record+0x1EE0,4); }
    if(subtype==0xD && !readword(record+0x1AE0) && readword(record+0x1EA4)) memcpy(snap_expected+0x288,record+0x1EA4,4);
    word(snap_expected+0x29C,0); word(snap_expected+0x2A0,0);
    if(subtype==0x1A || subtype==0xF) {
        memcpy(snap_expected+0x2A0,record+(subtype==0x1A?0x560:0x4A0),4);
        memcpy(snap_expected+0x29C,record+(subtype==0x1A?0x570:0x4B0),4);
    }
}
static void snapshot_run(int status) {
    u8 saved[sizeof(snap_client)], oldtable[sizeof(table)], oldchild[sizeof(child)];
    unsigned i;
    memcpy(saved,snap_client,sizeof(saved)); memcpy(oldrecord,record,sizeof(record));
    memcpy(oldentity,entity,sizeof(entity)); memcpy(oldtable,table,sizeof(table)); memcpy(oldchild,child,sizeof(child));
    CHECK(dreadzone_actor_0066A2A0(&context,snap_client,0x1000000,sizeof(snap_client))==status);
    for(i=0;i<sizeof(snap_client);i++) CHECK(snap_client[i]==(status==0?snap_expected[i]:saved[i]));
    CHECK(memcmp(oldrecord,record,sizeof(record))==0); CHECK(memcmp(oldentity,entity,sizeof(entity))==0);
    CHECK(memcmp(oldtable,table,sizeof(table))==0); CHECK(memcmp(oldchild,child,sizeof(child))==0);
}
static void complete_snapshot_tests(void) {
    static const u32 actions[]={0,0x31,0x99,0x71,0x8F,0xFFFFFFFF};
    static const u32 types[]={0,0xD,0xE,0xF,0x1A,0xFFFFFFFF};
    unsigned seed;
    for(seed=0;seed<576;seed++) {
        snapshot_setup(seed); word(record+0x25CC,actions[(seed/4)%6]); word(record+0x25D4,types[(seed/24)%6]);
        word(record+0x25D8,(seed&4)?0x76:0x75); word(record+0x1AE0,(seed&8)?1:0); word(record+0x1EA4,(seed&16)?0xABCD1234:0);
        word(record+0x3F4,(seed&32)?1:0); context.state_0021DDB4=(seed&64)?6:0;
        entity[0x86]=(u8)((seed&128)?5:0); modes[3]=(u8)seed;
        snapshot_oracle(); snapshot_run(0);
    }
    snapshot_setup(1); word(table+0x48,0x668070); snapshot_run(DREADZONE_SNAPSHOT_UNKNOWN_GETTER);
    snapshot_setup(1); context.view_count=2; snapshot_run(DREADZONE_SNAPSHOT_NEEDS_VTABLE);
    snapshot_setup(1); word(record+0x14,0xFFFFFFB8); snapshot_run(DREADZONE_SNAPSHOT_NEEDS_VTABLE);
    snapshot_setup(1); snap_views[3].bytes=0x2B3; snapshot_run(DREADZONE_SNAPSHOT_NEEDS_GETTER_OBJECT);
    snapshot_setup(1); word(record+0x2EF4,0xFFFFFE00); snapshot_run(DREADZONE_SNAPSHOT_NEEDS_GETTER_OBJECT);
    snapshot_setup(1); snap_views[1]=snap_views[0]; snapshot_run(DREADZONE_SNAPSHOT_NEEDS_CLIENT);
    snapshot_setup(1); snap_views[1].storage=snap_client+1; snapshot_run(DREADZONE_SNAPSHOT_INVALID_STORAGE);
    snapshot_setup(1); context.mode_bytes_00171DE0=snap_client+0x150; snapshot_run(DREADZONE_SNAPSHOT_INVALID_STORAGE);
    /* Retail reads these after its preceding snapshot writes. Preflight must
     * reject them, rather than complete with a stale handler/getter/predicate. */
    snapshot_setup(1); word(snap_client+0x50,0x10000CA); snap_client[0x150]=5; snap_client[0x151]=0;
    record[0x20]=1; record[0x21]=0; snapshot_run(DREADZONE_SNAPSHOT_INVALID_STORAGE);
    snapshot_setup(1); word(record+0x2EF4,0x1000000-0x160); word(snap_client+0x150,0xDEADBEEF);
    word(record+0x20,0x11223344); snapshot_run(DREADZONE_SNAPSHOT_INVALID_STORAGE);
    snapshot_setup(1); word(record+0x14,0x1000120); word(snap_client+0x168,0x690E68);
    word(record+0x38,0x6A7D28);
    snapshot_run(DREADZONE_SNAPSHOT_INVALID_STORAGE);
    /* A read through the identical client mapping outside the write footprint
     * is a real supported dependency, not an unconditional client-alias ban. */
    snapshot_setup(1); word(record+0x2EF4,0x1000000); word(snap_client+0x2B0,0xAFBECD01);
    snapshot_oracle(); word(snap_expected+0x244,0xAFBECD01); snapshot_run(0);
    snapshot_setup(1); word(record+0x2EFC,0x1000010); snapshot_run(DREADZONE_SNAPSHOT_NEEDS_CLIENT);
    snapshot_setup(1); CHECK(dreadzone_actor_0066A2A0(&context,snap_client,0x1000000,0x2A3)==DREADZONE_SNAPSHOT_NEEDS_CLIENT);
    snapshot_setup(1); CHECK(dreadzone_actor_0066A2A0(&context,record,0x1000000,sizeof(record))==DREADZONE_SNAPSHOT_INVALID_STORAGE);
}
int main(void) {
    unsigned mode, gate, state, action, relation;
    static const s32 states[]={0,1,2,3,6,7,-1};
    static const u16 relations[]={0,5,6,0xFFFF,0x8005};
    for(mode=0;mode<256;mode++) for(gate=0;gate<2;gate++) for(state=0;state<7;state++)
        for(action=0;action<0x100;action++) for(relation=0;relation<5;relation++) {
            s32 expected;
            setup(); modes[3]=(u8)mode; word(record+0x3F4,gate); context.state_0021DDB4=states[state];
            word(record+0x25CC,action); entity[0x86]=(u8)relations[relation]; entity[0x87]=(u8)(relations[relation]>>8);
            expected=(mode && !gate && states[state]!=2 && states[state]!=6 && relations[relation]!=5 && !excluded(action))?(s32)mode:0;
            /* Exhaustive branch/value oracle; large immutable images are checked separately. */
            { s32 result=-1; CHECK(dreadzone_actor_0068DE60(&context,&result)==0); CHECK(result==expected); }
        }
    for(action=0;action<256;action++) { setup(); word(record+0x25CC,action); run(0,excluded(action)?0:255); }
    setup(); record[0x266D]=0; context.view_count=0; run(0,255);
    setup(); word(client+0x50,0); context.view_count=1; run(0,255);
    setup(); context.view_count=0; run(DREADZONE_SNAPSHOT_NEEDS_CLIENT,0);
    modes[3]=0; run(0,0);
    setup(); context.view_count=0; word(record+0x3F4,1); run(0,0);
    setup(); context.view_count=0; context.state_0021DDB4=2; run(0,0);
    setup(); context.view_count=0; context.state_0021DDB4=6; run(0,0);
    setup(); context.view_count=1; run(DREADZONE_SNAPSHOT_NEEDS_RELATED_OBJECT,0);
    setup(); context.view_count=3; run(DREADZONE_SNAPSHOT_NEEDS_RELATED_OBJECT,0);
    setup(); views[1]=views[0]; run(DREADZONE_SNAPSHOT_NEEDS_CLIENT,0);
    setup(); views[0].bytes=0x53; run(DREADZONE_SNAPSHOT_NEEDS_CLIENT,0);
    setup(); views[1].bytes=0x87; run(DREADZONE_SNAPSHOT_NEEDS_RELATED_OBJECT,0);
    setup(); word(record+0x2EFC,0xFFFFFFB0); run(DREADZONE_SNAPSHOT_NEEDS_CLIENT,0);
    setup(); word(client+0x50,0xFFFFFF80); run(DREADZONE_SNAPSHOT_NEEDS_RELATED_OBJECT,0);
    setup(); word(record+0x2EFC,0); run(DREADZONE_SNAPSHOT_NEEDS_CLIENT,0);
    setup(); word(record+0x2EFC,0x1000001); run(DREADZONE_SNAPSHOT_NEEDS_CLIENT,0);
    setup(); word(client+0x50,0x1100001); run(DREADZONE_SNAPSHOT_NEEDS_RELATED_OBJECT,0);
    setup(); context.record_bytes=0x31EF; run(DREADZONE_SNAPSHOT_INVALID_STORAGE,0);
    setup(); context.record_guest=0xFFFFD000; run(DREADZONE_SNAPSHOT_INVALID_STORAGE,0);
    setup(); context.record_guest++; run(DREADZONE_SNAPSHOT_INVALID_STORAGE,0);
    setup(); word(record+0x2F10,0xFFFFFFFF); run(DREADZONE_SNAPSHOT_INVALID_STORAGE,0);
    setup(); word(record+0x2F10,4); run(DREADZONE_SNAPSHOT_INVALID_STORAGE,0);
    setup(); context.mode_count=0; run(DREADZONE_SNAPSHOT_INVALID_STORAGE,0);
    setup(); context.mode_count=0xFFFFFFFF; run(DREADZONE_SNAPSHOT_INVALID_STORAGE,0);
    setup(); context.mode_bytes_00171DE0=0; run(DREADZONE_SNAPSHOT_INVALID_STORAGE,0);
    setup(); context.views=0; run(DREADZONE_SNAPSHOT_NEEDS_CLIENT,0);
    CHECK(dreadzone_actor_0068DE60(0,0)==DREADZONE_SNAPSHOT_INVALID_STORAGE);
    complete_snapshot_tests();
    printf("Actor predicate/snapshot: %u checks, %u failures. Actual supported getters and CPU copy execute; no constructor effects simulated.\n",checks,failures);
    return failures?1:0;
}
