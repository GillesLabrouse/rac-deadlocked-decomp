#include "dreadzone_actor_ground_step.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { BASE=0x21D000,SIZE=0x1A0000,RECORD=0x368F60,FIRST=0x240000,SECOND=0x240010 };
static unsigned checks;
#define CHECK(test) do{++checks;if(!(test)){fprintf(stderr,"Ground helper check failed at line%d\n",__LINE__);return 1;}}while(0)
static void word(u8 *b,u32 a,u32 value){memcpy(b+a-BASE,&value,4);}
static void real(u8 *b,u32 a,f32 value){memcpy(b+a-BASE,&value,4);}
static u32 read_word(const u8 *b,u32 a){u32 v;memcpy(&v,b+a-BASE,4);return v;}
static f32 read_float(const u8 *b,u32 a){f32 v;memcpy(&v,b+a-BASE,4);return v;}
static void vector(u8 *b,u32 a,f32 x,f32 y,f32 z,u32 w){real(b,a,x);real(b,a+4,y);real(b,a+8,z);word(b,a+12,w);}
static void short_word(u8 *b,u32 a,u16 value){memcpy(b+a-BASE,&value,2);}
static u32 packed_vertex(s32 x,s32 y,s32 z){return ((u32)x&1023U)|(((u32)y&1023U)<<10)|(((u32)z&4095U)<<20);}
static void fixture(u8 *b){
    static const f32 coefficients[8]={
        .999999344348907470703125f,-.33329856395721435546875f,
        .1994653642177581787109375f,-.13908533751964569091796875f,
        .096420042216777801513671875f,-.05590988695621490478515625f,
        .0218612290918827056884765625f,-.0040540578775107860565185546875f
    };
    static const f32 quadrants[16]={
        1.f,0.f,-1.f,1.57079637050628662109375f,-1.f,0.f,1.f,-1.57079637050628662109375f,
        -1.f,3.1415927410125732421875f,1.f,1.57079637050628662109375f,
        1.f,-3.1415927410125732421875f,-1.f,-1.57079637050628662109375f
    };
    memset(b,0,SIZE);word(b,RECORD+0x2EF0,0x230000);
    vector(b,RECORD+0xA0,10.f,20.f,30.f,0xDEADBEEFU);
    vector(b,RECORD+0x270,0.f,0.f,-1.f,0);vector(b,0x2300E0,0.f,0.f,1.f,0);
    vector(b,RECORD+0x20,1.f,0.f,0.f,0);vector(b,RECORD+0x30,0.f,1.f,0.f,0);
    vector(b,RECORD+0x40,0.f,0.f,1.f,0);vector(b,RECORD+0x50,0.f,0.f,0.f,0x3F800000U);
    memcpy(b+0x3B9540-BASE,coefficients,sizeof(coefficients));memcpy(b+0x3B9580-BASE,quadrants,sizeof(quadrants));
}
static DreadzoneGroundRequest request(u8 *b,u32 operation){
    DreadzoneGroundRequest q;memset(&q,0,sizeof(q));q.storage=b;q.guest_base=BASE;q.storage_bytes=SIZE;
    q.record_guest=RECORD;q.operation=operation;q.value_guest=FIRST;q.target_guest=SECOND;return q;
}
/* Explicit small flat terrain fixture, independent of extracted level data.
 * Its tagF bypasses the genuine surface-zero second-mask gate. */
static void ground_fixture(u8 *b){
    u32 tree=0x3A0000,block=tree+128;
    fixture(b);word(b,0x242340,tree);word(b,0x222D14,0x2A0000);word(b,0x21DDB8,1);
    short_word(b,tree,2);short_word(b,tree+2,1);short_word(b,tree+4,8);
    short_word(b,tree+32,2);short_word(b,tree+34,1);word(b,tree+36,64);
    short_word(b,tree+64,2);short_word(b,tree+66,1);word(b,tree+68,0x8002);
    b[block-BASE]=1;b[block+2-BASE]=3;
    word(b,block+4,packed_vertex(-16,-16,-16));word(b,block+8,packed_vertex(16,-16,-16));
    word(b,block+12,packed_vertex(0,16,-16));b[block+16-BASE]=0;b[block+17-BASE]=2;
    b[block+18-BASE]=1;b[block+19-BASE]=0x0F;
    vector(b,RECORD+0xA0,10.f,10.f,9.76f,0x3F800000U);
    real(b,RECORD+0x204,.75f);real(b,RECORD+0x25C4,1.f);
    short_word(b,RECORD+0x2FE,7);short_word(b,RECORD+0x2FC,3);word(b,RECORD+0x2F0,10);
    /* Actual mode0 gravity never consumes mode1's progress operand. */
    word(b,RECORD+0x2AC,0x7FC00000U);
}
int main(void){
    u8 *b=(u8 *)malloc(SIZE),*snapshot=(u8 *)malloc(SIZE);DreadzoneGroundRequest q;DreadzoneGroundInfo info,old_info;
    DreadzoneProbeRegisters registers,old_registers;u32 incoming[2]={0x3F000000U,0xBF800000U};
    u32 i;int status;
    CHECK(b && snapshot);fixture(b);q=request(b,DREADZONE_GROUND_PACKET_FLAGS);
    for(i=0;i<4;++i){word(b,0x24235C,i<<5);CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(info.has_integer_return && info.original_integer==(s32)i);CHECK(!info.has_float_return);}
    word(b,0x24235C,0x80000060U);CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(info.original_integer==-1);

    fixture(b);q=request(b,DREADZONE_GROUND_HISTORY);q.integer0=0x38;q.integer1=0x78;
    word(b,RECORD+0x264C,2);word(b,RECORD+0x370,100);word(b,RECORD+0x25EC,1);word(b,RECORD+0x25F0,0x38);
    word(b,RECORD+0x262C,30);CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(info.original_integer==0);
    word(b,RECORD+0x262C,0x7FFFFFFFU);CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(info.original_integer==1);
    word(b,RECORD+0x370,121);word(b,RECORD+0x25EC,0x38);CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(info.original_integer==0);
    word(b,RECORD+0x370,120);CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(info.original_integer==1);
    word(b,RECORD+0x264C,0xFFFFFFFFU);CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(info.original_integer==0);

    fixture(b);q=request(b,DREADZONE_GROUND_ENDPOINTS);q.scalar[0]=2.f;q.scalar[1]=-48.f;
    CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(read_float(b,FIRST+8)==32.f);CHECK(read_float(b,SECOND+8)==0.f);
    CHECK(read_word(b,FIRST+12)==0xDEADBEEFU && read_word(b,SECOND+12)==0xDEADBEEFU);
    q.value_guest=q.target_guest;CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(read_float(b,SECOND+8)==32.f);

    fixture(b);q=request(b,DREADZONE_GROUND_CLIP);vector(b,FIRST,1.f,0.f,0.f,0xDEADBEEFU);vector(b,SECOND,-1.f,0.f,0.f,0);
    CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(fabsf(read_float(b,SECOND)-.1f)<1e-6f);
    CHECK(read_word(b,SECOND+12)==0xDEADBEEFU);CHECK(read_float(b,SECOND+4)==0.f && read_float(b,SECOND+8)==0.f);

    fixture(b);q=request(b,DREADZONE_GROUND_HEIGHT);vector(b,FIRST,101.f,101.f,100.f,0x3F800000U);
    vector(b,0x2237C0,101.f,101.f,100.125f,0x3F800000U);word(b,0x2237DC,1);q.target_guest=0x2237C0;
    CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(info.has_float_return && info.original_float==1.f);
    CHECK(read_word(b,0x2237C0)==0 && read_word(b,0x2237C4)==0 && read_float(b,0x2237C8)==1.f);

    fixture(b);q=request(b,DREADZONE_GROUND_SURFACE_EULER);vector(b,FIRST,0.f,0.f,0.f,0);
    word(b,SECOND+8,0xCAFEBABEU);word(b,SECOND+12,0xDEADBEEFU);
    CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(read_float(b,SECOND)==0.f);
    CHECK(read_word(b,SECOND+4)==0x80000000U);CHECK(read_word(b,SECOND+8)==0xCAFEBABEU && read_word(b,SECOND+12)==0xDEADBEEFU);

    fixture(b);q=request(b,DREADZONE_GROUND_SIDE_OFFSET);q.integer0=1;q.scalar[0]=.2f;q.scalar[1]=0.f;q.scalar[2]=-.45f;
    vector(b,SECOND,4.f,5.f,6.f,0x40000000U);vector(b,RECORD+0x50,2.f,3.f,4.f,0x3F800000U);
    CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(fabsf(read_float(b,SECOND)-14.2f)<1e-5f);
    CHECK(read_float(b,SECOND+4)==26.f);CHECK(fabsf(read_float(b,SECOND+8)-37.55f)<1e-5f);CHECK(read_float(b,SECOND+12)==2.f);

    /* Descriptor resides in the destination it names. It must be captured,
     * otherwise the first two stores destroy the pointer used for publication. */
    fixture(b);q=request(b,DREADZONE_GROUND_HEIGHT);vector(b,FIRST,1.f,2.f,3.f,0);q.target_guest=SECOND;
    memcpy(b+SECOND-BASE,&q,sizeof(q));CHECK(dreadzone_actor_ground_step((DreadzoneGroundRequest *)(b+SECOND-BASE),&info)==1);
    CHECK(info.original_float==3.f && read_float(b,SECOND+8)==1.f);

    fixture(b);q=request(b,DREADZONE_GROUND_HEIGHT);vector(b,FIRST,1.f,2.f,3.f,0);word(b,0x2237D8,1);
    memcpy(snapshot,b,SIZE);memset(&info,0xA5,sizeof(info));old_info=info;
    CHECK(dreadzone_actor_ground_step(&q,&info)==DREADZONE_GROUND_UNSUPPORTED_PATH);CHECK(!memcmp(b,snapshot,SIZE));CHECK(!memcmp(&info,&old_info,sizeof(info)));
    q=request(b,DREADZONE_GROUND_ENDPOINTS);q.integer0=1;q.scalar[0]=2.f;q.scalar[1]=-48.f;q.target_guest=RECORD+0x2EF0;
    status=dreadzone_actor_ground_step(&q,&info);CHECK(status<0);CHECK(!memcmp(b,snapshot,SIZE));CHECK(!memcmp(&info,&old_info,sizeof(info)));
    q=request(b,DREADZONE_GROUND_SURFACE_EULER);q.target_guest=0x3B9540;CHECK(dreadzone_actor_ground_step(&q,&info)<0);CHECK(!memcmp(b,snapshot,SIZE));
    q=request(b,DREADZONE_GROUND_HISTORY);word(b,RECORD+0x264C,9);memcpy(snapshot,b,SIZE);
    CHECK(dreadzone_actor_ground_step(&q,&info)==DREADZONE_GROUND_UNSUPPORTED_PATH);CHECK(!memcmp(b,snapshot,SIZE));
    q=request(b,DREADZONE_GROUND_PACKET_FLAGS);q.storage=(u8 *)(UINTPTR_MAX-8);CHECK(dreadzone_actor_ground_step(&q,&info)==DREADZONE_GROUND_INVALID);
    q=request(b,DREADZONE_GROUND_PACKET_FLAGS);q.guest_base=0xFFFF0000U;CHECK(dreadzone_actor_ground_step(&q,&info)==DREADZONE_GROUND_INVALID);
    q=request(b,DREADZONE_GROUND_PACKET_FLAGS);CHECK(dreadzone_actor_ground_step(&q,(DreadzoneGroundInfo *)b)==DREADZONE_GROUND_INVALID);
    ground_fixture(b);q=request(b,DREADZONE_GROUND_UPDATE);q.integer0=1;q.actor_pool_count=1;q.grid_block_count=3072;
    for(i=0;i<5;++i)q.incoming_vf25_29_w[i]=0x3F800000U+i;
    CHECK(dreadzone_actor_ground_step(&q,&info)==1);
    CHECK(info.probe_calls==1 && info.side_probe_calls==0 && info.ground_contacts==1);
    CHECK(!info.has_integer_return && !info.has_float_return);
    CHECK(read_word(b,RECORD+0x2FC)==0 && read_word(b,RECORD+0x2F0)==11);
    CHECK(read_word(b,RECORD+0x630)==0 && !memcmp(b+RECORD+0x280-BASE,b+RECORD+0x290-BASE,16));
    CHECK(read_word(b,RECORD+0x270)==0 && read_word(b,RECORD+0x274)==0);
    CHECK(read_word(b,RECORD+0x278)==0xBF800000U && read_word(b,RECORD+0x27C)==0);
    CHECK(read_word(b,RECORD+0x2AC)==0x7FC00000U);
    CHECK(!memcmp(info.outgoing_vf25_29_w,q.incoming_vf25_29_w,12));
    CHECK(info.outgoing_vf25_29_w[3]==0 && info.outgoing_vf25_29_w[4]==0);

    for(i=0;i<4;++i){
        static const u8 tags[4]={0,13,3,4};
        ground_fixture(b);b[0x3A0093-BASE]=tags[i];q=request(b,DREADZONE_GROUND_UPDATE);
        q.integer0=1;q.actor_pool_count=1;q.grid_block_count=3072;
        CHECK(dreadzone_actor_ground_step(&q,&info)==1);
        CHECK(info.probe_calls==(i==3?1U:2U) && info.ground_contacts==0);
        CHECK(info.side_probe_calls==0);
    }
    ground_fixture(b);word(b,RECORD+0x2F10,1);q=request(b,DREADZONE_GROUND_UPDATE);
    q.integer0=1;q.actor_pool_count=1;q.grid_block_count=3072;
    q.incoming_side_output_w[0]=0x3F800000U;q.incoming_side_output_w[1]=0xBF000000U;
    CHECK(dreadzone_actor_ground_step(&q,&info)==1);
    CHECK(info.probe_calls==3 && info.side_probe_calls==2 && info.ground_contacts==1);

    for(i=0;i<4;++i){
        static const u32 selected_bad_values[4]={0x7FC00000U,0x7F800000U,1U,0x46000400U};
        ground_fixture(b);word(b,RECORD+0xA0,selected_bad_values[i]);q=request(b,DREADZONE_GROUND_UPDATE);
        q.integer0=1;q.actor_pool_count=1;q.grid_block_count=3072;
        memcpy(snapshot,b,SIZE);memset(&info,0xA5,sizeof(info));old_info=info;
        CHECK(dreadzone_actor_ground_step(&q,&info)==DREADZONE_GROUND_UNSUPPORTED_FLOAT);
        CHECK(!memcmp(b,snapshot,SIZE) && !memcmp(&info,&old_info,sizeof(info)));
    }

    ground_fixture(b);q=request(b,DREADZONE_GROUND_UPDATE);q.integer0=0;
    word(b,RECORD+0x2F10,1);memcpy(snapshot,b,SIZE);
    CHECK(dreadzone_actor_ground_step(&q,&info)==1);CHECK(!memcmp(b,snapshot,SIZE));
    CHECK(info.probe_calls==0 && info.ground_contacts==0);
    q.integer0=1;b[RECORD+0x2659-BASE]=1;memcpy(snapshot,b,SIZE);memset(&info,0xA5,sizeof(info));old_info=info;
    CHECK(dreadzone_actor_ground_step(&q,&info)==DREADZONE_GROUND_UNSUPPORTED_PATH);
    CHECK(!memcmp(b,snapshot,SIZE) && !memcmp(&info,&old_info,sizeof(info)));
    b[RECORD+0x2659-BASE]=0;word(b,0x21E824,1);memcpy(snapshot,b,SIZE);
    CHECK(dreadzone_actor_ground_step(&q,&info)==DREADZONE_GROUND_UNSUPPORTED_PATH);
    CHECK(!memcmp(b,snapshot,SIZE) && !memcmp(&info,&old_info,sizeof(info)));
    /* Genuine scheduled exit preserves arbitrary raw state without consuming it. */
    ground_fixture(b);q=request(b,DREADZONE_GROUND_UPDATE);word(b,RECORD+0x2F10,1);
    incoming[0]=0x7FC0BEEFU;incoming[1]=0x80000000U;memcpy(snapshot,b,SIZE);
    CHECK(dreadzone_actor_ground_registers(&q,incoming,&registers,&info)==1);
    CHECK(!memcmp(b,snapshot,SIZE) && !info.probe_calls);
    CHECK(!memcmp(registers.high64,incoming,8) && registers.last_writer_pc==0);

    ground_fixture(b);q=request(b,DREADZONE_GROUND_UPDATE);q.integer0=1;
    q.actor_pool_count=1;q.grid_block_count=3072;
    CHECK(dreadzone_actor_ground_registers(&q,incoming,&registers,&info)==1);
    CHECK(info.ground_contacts==1 && info.probe_calls==1);
    CHECK(registers.high64[0]==0 && registers.high64[1]==0 && registers.last_writer_pc==0x53E464);

    /* Empty, out-of-grid clipped ray retains the final Z delta and raw W;
     * a failed contact alone never implies a cleared high register. */
    ground_fixture(b);q=request(b,DREADZONE_GROUND_UPDATE);q.integer0=1;
    q.actor_pool_count=1;q.grid_block_count=3072;
    vector(b,RECORD+0xA0,100.f,100.f,1.f,0x3F800000U);
    CHECK(dreadzone_actor_ground_registers(&q,incoming,&registers,&info)==1);
    CHECK(info.probe_calls==1 && !info.ground_contacts);
    CHECK(registers.high64[0]==0xC246999AU && registers.high64[1]==0x3F800000U);
    CHECK(registers.last_writer_pc==0x59640C);

    /* Input/output snapshot alias is allowed on the preserving path. */
    ground_fixture(b);q=request(b,DREADZONE_GROUND_UPDATE);word(b,RECORD+0x2F10,1);
    registers.high64[0]=0xDEADBEEFU;registers.high64[1]=0x7FC01234U;registers.last_writer_pc=0xAAAAAAAAU;
    CHECK(dreadzone_actor_ground_registers(&q,registers.high64,&registers,&info)==1);
    CHECK(registers.high64[0]==0xDEADBEEFU && registers.high64[1]==0x7FC01234U && !registers.last_writer_pc);

    /* Publication destinations cannot overwrite inputs or persistent backing. */
    memcpy(snapshot,b,SIZE);old_info=info;old_registers=registers;
    CHECK(dreadzone_actor_ground_registers(&q,incoming,(DreadzoneProbeRegisters *)&q,&info)==DREADZONE_GROUND_INVALID);
    CHECK(!memcmp(b,snapshot,SIZE) && !memcmp(&info,&old_info,sizeof(info)));
    CHECK(dreadzone_actor_ground_registers(&q,incoming,(DreadzoneProbeRegisters *)&info,&info)==DREADZONE_GROUND_INVALID);
    CHECK(!memcmp(b,snapshot,SIZE) && !memcmp(&info,&old_info,sizeof(info)));
    CHECK(dreadzone_actor_ground_registers(&q,incoming,(DreadzoneProbeRegisters *)b,&info)==DREADZONE_GROUND_INVALID);
    CHECK(!memcmp(b,snapshot,SIZE) && !memcmp(&info,&old_info,sizeof(info)));
    CHECK(dreadzone_actor_ground_registers(&q,NULL,&registers,&info)==DREADZONE_GROUND_INVALID);
    CHECK(!memcmp(b,snapshot,SIZE) && !memcmp(&registers,&old_registers,sizeof(registers)));
    q.operation=DREADZONE_GROUND_HEIGHT;
    CHECK(dreadzone_actor_ground_registers(&q,incoming,&registers,&info)==DREADZONE_GROUND_INVALID);
    CHECK(!memcmp(b,snapshot,SIZE) && !memcmp(&info,&old_info,sizeof(info)) && !memcmp(&registers,&old_registers,sizeof(registers)));

    ground_fixture(b);q=request(b,DREADZONE_GROUND_UPDATE);q.integer0=1;
    q.actor_pool_count=1;q.grid_block_count=3072;word(b,RECORD+0xA0,0x7FC0BEEFU);
    memcpy(snapshot,b,SIZE);old_info=info;old_registers=registers;
    CHECK(dreadzone_actor_ground_registers(&q,incoming,&registers,&info)==DREADZONE_GROUND_UNSUPPORTED_FLOAT);
    CHECK(!memcmp(b,snapshot,SIZE) && !memcmp(&info,&old_info,sizeof(info)) && !memcmp(&registers,&old_registers,sizeof(registers)));

    /* Synthetic surface-zero input exercises helper composition, not original
     * level coverage. Standalone original-helper proof verifies its producer. */
    ground_fixture(b);b[0x3A0093-BASE]=0;word(b,0x2237DC,1);
    vector(b,0x2237C0,10.f,10.f,9.75f,0x40000000U);
    q=request(b,DREADZONE_GROUND_UPDATE);q.integer0=1;q.actor_pool_count=1;q.grid_block_count=3072;
    CHECK(dreadzone_actor_ground_step(&q,&info)==1);
    memcpy(snapshot,b,SIZE);old_info=info;
    ground_fixture(b);b[0x3A0093-BASE]=0;word(b,0x2237DC,1);
    vector(b,0x2237C0,10.f,10.f,9.75f,0x40000000U);
    CHECK(dreadzone_actor_ground_registers(&q,incoming,&registers,&info)==1);
    CHECK(!memcmp(b,snapshot,SIZE) && !memcmp(&info,&old_info,sizeof(info)));
    CHECK(info.probe_calls==2 && !info.ground_contacts);
    CHECK(registers.high64[0]==0 && registers.high64[1]==0 && registers.last_writer_pc==0x53E464);

    ground_fixture(b);b[0x3A0093-BASE]=0;word(b,0x2237D8,1);
    memcpy(snapshot,b,SIZE);old_info=info;old_registers=registers;
    CHECK(dreadzone_actor_ground_registers(&q,incoming,&registers,&info)==DREADZONE_GROUND_UNSUPPORTED_PATH);
    CHECK(!memcmp(b,snapshot,SIZE) && !memcmp(&info,&old_info,sizeof(info)) && !memcmp(&registers,&old_registers,sizeof(registers)));
    free(snapshot);free(b);printf("Ground CPU helpers and whole update: %u checks passed.\n",checks);return 0;
}
