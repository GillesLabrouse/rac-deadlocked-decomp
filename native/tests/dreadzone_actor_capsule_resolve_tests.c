#include "dreadzone_actor_capsule_resolve.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE 0x21D000U
#define SIZE 0x1F3000U
#define RECORD 0x368F60U
static unsigned checks,failures;
#define CHECK(x) do {++checks;if(!(x)){++failures;printf("line %d: %s\n",__LINE__,#x);}}while(0)
static u8 *bytes;
static void word(u32 a,u32 v){memcpy(bytes+a-BASE,&v,4);}
static void half(u32 a,u16 v){memcpy(bytes+a-BASE,&v,2);}
static void real(u32 a,f32 v){memcpy(bytes+a-BASE,&v,4);}
static u32 get(u32 a){u32 v;memcpy(&v,bytes+a-BASE,4);return v;}
static f32 value(u32 a){f32 v;memcpy(&v,bytes+a-BASE,4);return v;}
static u32 vertex(s32 x,s32 y,s32 z){return ((u32)x&1023U)|(((u32)y&1023U)<<10)|(((u32)z&4095U)<<20);}
static DreadzoneCapsuleResolveRequest setup(void) {
    DreadzoneCapsuleResolveRequest r;u32 i;memset(bytes,0,SIZE);memset(&r,0,sizeof(r));
    r.storage=bytes;r.guest_base=BASE;r.storage_bytes=SIZE;r.record_guest=RECORD;r.actor_pool_count=1;
    for(i=0;i<5;++i)r.incoming_vf25_29_w[i]=0xA0B0C000U+i;
    word(0x242340,0x400000);word(0x222D14,0x230000);
    half(0x400000,2);half(0x400002,1);half(0x400004,8);
    half(0x400020,2);half(0x400022,1);word(0x400024,64);
    half(0x400040,2);half(0x400042,1);word(0x400044,0x8002);
    bytes[0x400080-BASE]=1;bytes[0x400082-BASE]=3;
    word(0x400084,vertex(-16,-16,-16));word(0x400088,vertex(16,-16,-16));word(0x40008C,vertex(0,16,-16));
    bytes[0x400090-BASE]=0;bytes[0x400091-BASE]=2;bytes[0x400092-BASE]=1;bytes[0x400093-BASE]=1;
    word(RECORD+0x2EF0,0x230000);word(RECORD+0x25CC,2);word(RECORD+0x25D4,2);
    real(RECORD+0x48,1.f);real(RECORD+0x278,-1.f);
    real(RECORD+0xA0,10.f);real(RECORD+0xA4,10.f);real(RECORD+0xA8,9.5f);word(RECORD+0xAC,0xDEADBEEF);
    real(RECORD+0x204,.5f);real(RECORD+0x1F4,.5f);real(RECORD+0x1F0,1.5f);
    bytes[RECORD+0x266D-BASE]=1;
    for(i=0;i<32;++i)memcpy(bytes+RECORD+0x26F0+i*16-BASE,bytes+RECORD+0xA0-BASE,16);
    word(RECORD+0x2C7C,32);
    return r;
}
static void static_triangle(void) {
    u32 i;word(0x222BE0,0x260000);word(0x260000,1);
    half(0x260010,102*64);half(0x260012,102*64);half(0x260014,100*64);half(0x260016,4*64);
    bytes[0x260018-BASE]=1;half(0x26001A,3);word(0x26001C,0x261000);
    for(i=0;i<3;++i) {
        half(0x261000+i*8,(u16)((i==1?104:100)*64));
        half(0x261002+i*8,(u16)((i==2?104:100)*64));half(0x261004+i*8,100*64);
    }
    word(0x261018,0x00020100);
    real(RECORD+0xA0,101.f);real(RECORD+0xA4,101.f);real(RECORD+0xA8,99.f);
    real(RECORD+0x204,.75f);
}
static void refused(DreadzoneCapsuleResolveRequest *r,int expected) {
    u8 *saved=(u8 *)malloc(SIZE);DreadzoneCapsuleResolveInfo info,before;
    CHECK(saved!=NULL);if(!saved)return;memcpy(saved,bytes,SIZE);
    memset(&info,0xA5,sizeof(info));before=info;
    CHECK(dreadzone_actor_capsule_resolve(r,&info)==expected);
    CHECK(!memcmp(saved,bytes,SIZE));CHECK(!memcmp(&before,&info,sizeof(info)));free(saved);
}
int main(void) {
    u8 *allocation=(u8 *)malloc(SIZE+32);DreadzoneCapsuleResolveRequest r,*embedded;
    DreadzoneCapsuleResolveInfo info;u32 i;u8 before[16];
    if(!allocation)return 1;bytes=allocation+16;memset(allocation,0xCD,16);memset(bytes+SIZE,0xAB,16);
    r=setup();CHECK(dreadzone_actor_capsule_resolve(&r,&info)==1);
    CHECK(info.has_original_return==1&&info.original_return==1);
    CHECK(info.capsule_calls>=1&&info.static_calls==info.capsule_calls&&info.contact_iterations>=1);
    CHECK(value(RECORD+0xA8)>9.5f&&value(RECORD+0xA8)<10.f);
    /* Contact position is packet+30, whose fourth word is the real flags. */
    CHECK(get(RECORD+0xAC)==0x200&&bytes[RECORD+0x226-BASE]==1);
    CHECK(value(RECORD+0x1E8)==9.75f&&get(RECORD+0x20C)==0);
    for(i=0;i<5;++i)CHECK(get(0x24236C+(i?16+i*16:0))==r.incoming_vf25_29_w[i]);
    r=setup();r.operation=DREADZONE_CAPSULE_RESOLVE_F518;
    CHECK(dreadzone_actor_capsule_resolve(&r,&info)==1);CHECK(info.has_original_return==0&&info.resolve_calls==1);
    CHECK(info.history_restores==0&&get(RECORD+0x2C7C)==32);
    /* A real miss still executes probe generation and leaves markers intact. */
    r=setup();bytes[0x400093-BASE]=0;word(RECORD+0x20C,0x12345678);
    memcpy(before,bytes+RECORD+0xA0-BASE,16);
    CHECK(dreadzone_actor_capsule_resolve(&r,&info)==1);CHECK(info.contact_iterations==0);
    CHECK(get(0x242350)==1&&get(RECORD+0x20C)==0x12345678);
    CHECK(!memcmp(before,bytes+RECORD+0xA0-BASE,16));
    r=setup();static_triangle();CHECK(dreadzone_actor_capsule_resolve(&r,&info)==1);
    CHECK(info.contact_iterations==1&&info.capsule_calls==2&&info.static_calls==2);
    CHECK(value(RECORD+0x1E8)==100.f&&get(RECORD+0x20C)==0);
    /* Suspension aliases the genuine static-table global: its zero is read
     * during the probe, then actual metadata restores the original pointer. */
    r=setup();static_triangle();word(RECORD+0x2314,2);word(RECORD+0x22F0,0x234000);
    half(0x2340BC,0x2094);word(0x2340AC,0x234100);word(0x234100,0x222B48);
    word(0x222B6C,0x234300);word(0x234310,0x260000);
    CHECK(dreadzone_actor_capsule_resolve(&r,&info)==1);
    CHECK(info.contact_iterations==0&&get(0x222BE0)==0x260000&&value(RECORD+0xA8)==99.f);
    /* The fixed packet can overlap actor radii/height. Later iterations must
     * consume the just-published data, as verified by original-instruction replay. */
    r=setup();memcpy(bytes+0x2421B0-BASE,bytes+RECORD-BASE,0x31F0);r.record_guest=0x2421B0;
    word(0x242340,0x400000);
    CHECK(dreadzone_actor_capsule_resolve(&r,&info)==1);CHECK(info.contact_iterations==2);
    /* Complete original early-return routes consume no probe or fake effects. */
    r=setup();half(RECORD+0x3A0,1);word(RECORD+0x2E18,1);
    CHECK(dreadzone_actor_capsule_resolve(&r,&info)==1);CHECK(info.original_return==1&&info.capsule_calls==0);
    r=setup();word(0x21DE10,31);word(0x21DDB4,2);
    CHECK(dreadzone_actor_capsule_resolve(&r,&info)==1);CHECK(info.capsule_calls==0&&get(0x242350)==0);
    /* Actual item suspension and restoration are memory mutations, not hooks. */
    r=setup();word(RECORD+0x2314,2);word(RECORD+0x22F0,0x234000);
    half(0x2340BC,0x2094);word(0x2340AC,0x234100);word(0x234100,0x234200);
    word(0x234298,1);word(0x234224,0x234300);word(0x234310,7);
    CHECK(dreadzone_actor_capsule_resolve(&r,&info)==1);CHECK(get(0x234298)==7);
    /* Borrowed request may reside in RAM: the original pointer is captured. */
    r=setup();embedded=(DreadzoneCapsuleResolveRequest *)(bytes+0x1000);*embedded=r;
    CHECK(dreadzone_actor_capsule_resolve(embedded,&info)==1);CHECK(info.contact_iterations>=1);
    r=setup();real(RECORD+0x278,1.f);refused(&r,-2); /* Real dot selects unrecovered sphere. */
    r=setup();half(RECORD+0x3D6,1);refused(&r,-2);
    r=setup();half(RECORD+0x2E18,1);refused(&r,-2);
    r=setup();word(RECORD+0x25D4,15);refused(&r,-2);
    r=setup();word(RECORD+0x25D4,17);refused(&r,-2); /* Actual initial dispatch reaches the secondary sphere. */
    r=setup();word(RECORD+0x204,0x7FC00001);refused(&r,-3);
    r=setup();r.record_guest++;refused(&r,-1);
    r=setup();r.guest_base=0xFFFFFFF0;refused(&r,-1);
    r=setup();r.storage=(u8 *)(UINTPTR_MAX-2);refused(&r,-1);
    r=setup();r.record_guest=BASE+SIZE+16;refused(&r,-1);
    r=setup();r.actor_pool_count=65537;refused(&r,-1);
    r=setup();CHECK(dreadzone_actor_capsule_resolve(&r,(DreadzoneCapsuleResolveInfo *)(bytes+0x1000))==-1);
    for(i=0;i<16;++i)CHECK(allocation[i]==0xCD&&bytes[SIZE+i]==0xAB);
    free(allocation);printf("%u checks, %u failures\n",checks,failures);return failures?1:0;
}
