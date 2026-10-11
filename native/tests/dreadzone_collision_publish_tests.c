#include "dreadzone_collision_publish.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE 0x210000U
#define SIZE 0x100000U
#define CONTAINER 0x280000U
static u8 bytes[SIZE],before[SIZE];
static unsigned checks;
#define CHECK(value) do {++checks;if(!(value)) {fprintf(stderr,"Failure at line %d: %s\n",__LINE__,#value);exit(1);}}while(0)
static void word(u32 address,u32 value){memcpy(bytes+address-BASE,&value,4);}
static u32 read_word(u32 address){u32 value;memcpy(&value,bytes+address-BASE,4);return value;}
static DreadzoneCollisionPublishRequest setup(void) {
    DreadzoneCollisionPublishRequest q;memset(bytes,0xA5,sizeof(bytes));
    q.storage=bytes;q.guest_base=BASE;q.storage_bytes=SIZE;q.container_guest=CONTAINER;
    word(CONTAINER,0);word(CONTAINER+4,0);word(CONTAINER+8,0);return q;
}
static void require_atomic(DreadzoneCollisionPublishRequest *q,int expected) {
    DreadzoneCollisionPublishInfo info,saved;memset(&info,0xA5,sizeof(info));saved=info;
    memcpy(before,bytes,SIZE);
    CHECK(dreadzone_collision_publish_00577C70(q,&info)==expected);
    CHECK(!memcmp(before,bytes,SIZE));CHECK(!memcmp(&saved,&info,sizeof(info)));
}
int main(void) {
    DreadzoneCollisionPublishRequest q;DreadzoneCollisionPublishInfo info;u32 i;
    q=setup();
    CHECK(dreadzone_collision_publish_00577C70(&q,&info)==DREADZONE_COLLISION_PUBLISH_OK);
    CHECK(read_word(0x242340)==0xA5A5A5A5);CHECK(read_word(0x222BE0)==0);
    CHECK(read_word(0x222BE4)==0);CHECK(info.published_words==2);
    CHECK(info.main_relocations==0 && info.alternative_relocations==0);
    q=setup();word(CONTAINER,0x40);word(CONTAINER+4,0x100);word(CONTAINER+8,0x200);
    word(CONTAINER+0x100,2);word(CONTAINER+0x200,1);
    word(CONTAINER+0x11C,0x80);word(CONTAINER+0x12C,0x90);word(CONTAINER+0x21C,0x60);
    CHECK(dreadzone_collision_publish_00577C70(&q,&info)==1);
    CHECK(read_word(0x242340)==CONTAINER+0x40);
    CHECK(read_word(0x222BE0)==CONTAINER+0x100 && read_word(0x222BE4)==CONTAINER+0x200);
    CHECK(read_word(CONTAINER+0x11C)==CONTAINER+0x180);
    CHECK(read_word(CONTAINER+0x12C)==CONTAINER+0x190);
    CHECK(read_word(CONTAINER+0x21C)==CONTAINER+0x260);
    CHECK(info.main_relocations==2 && info.alternative_relocations==1 && info.published_words==3);
    /* The original publisher is not idempotent; a second call relocates again. */
    CHECK(dreadzone_collision_publish_00577C70(&q,&info)==1);
    CHECK(read_word(CONTAINER+0x11C)==2U*CONTAINER+0x280);
    q=setup();word(CONTAINER+4,0x100);word(CONTAINER+8,0x200);
    word(CONTAINER+0x100,0xffffffffU);word(CONTAINER+0x200,0);
    CHECK(dreadzone_collision_publish_00577C70(&q,&info)==1);
    CHECK(info.main_relocations==0 && info.alternative_relocations==0);
    CHECK(read_word(0x222BE0)==CONTAINER+0x100 && read_word(0x222BE4)==CONTAINER+0x200);
    /* First successful publication/relocation is rolled back on a later fault. */
    q=setup();word(CONTAINER,0x40);word(CONTAINER+4,0x100);word(CONTAINER+0x100,1);
    word(CONTAINER+0x11C,0x80);word(CONTAINER+8,0x70000000U-CONTAINER);
    require_atomic(&q,DREADZONE_COLLISION_PUBLISH_INVALID);
    q=setup();q.container_guest++;require_atomic(&q,DREADZONE_COLLISION_PUBLISH_INVALID);
    q=setup();q.storage_bytes=3;require_atomic(&q,DREADZONE_COLLISION_PUBLISH_INVALID);
    q=setup();CHECK(dreadzone_collision_publish_00577C70(NULL,&info)==-1);
    CHECK(dreadzone_collision_publish_00577C70(&q,NULL)==-1);
    CHECK(dreadzone_collision_publish_00577C70(&q,(DreadzoneCollisionPublishInfo *)&q)==-1);
    CHECK(dreadzone_collision_publish_00577C70(&q,(DreadzoneCollisionPublishInfo *)bytes)==-1);
    /* Captured request may live in RAM; original stores execute on its shadow. */
    q=setup();memcpy(bytes+0x300,&q,sizeof(q));
    CHECK(dreadzone_collision_publish_00577C70((DreadzoneCollisionPublishRequest *)(bytes+0x300),&info)==1);
    CHECK(read_word(0x222BE0)==0 && read_word(0x222BE4)==0);
    /* Enforce the documented iteration boundary without committing partial writes. */
    q=setup();q.storage_bytes=SIZE;word(CONTAINER+4,0x100);word(CONTAINER+0x100,65537);
    /* This backing reaches its end before65536 iterations, an actual mapping fault. */
    for(i=0;i<100;++i)word(CONTAINER+0x11C+i*16,0);
    require_atomic(&q,DREADZONE_COLLISION_PUBLISH_INVALID);
    printf("%u collision publication checks passed\n",checks);return 0;
}
