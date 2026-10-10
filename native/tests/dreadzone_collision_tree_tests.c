#include "dreadzone_collision_tree.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static unsigned checks,failures;
#define CHECK(x) do {checks++;if(!(x)){failures++;printf("FAIL line%d: %s\n",__LINE__,#x);}}while(0)
static void half(u8 *p,u16 value) {memcpy(p,&value,2);}
static void word(u8 *p,u32 value) {memcpy(p,&value,4);}
int main(void) {
    u8 storage[258],before[258],*table=storage+1;
    DreadzoneCollisionTreeRequest r={table,0x400000,256,0x400000,11,21,31};
    u32 leaf=0xDEADBEEFU;
    memset(storage,0xD3,sizeof(storage));
    half(table,20);half(table+2,2);half(table+4,8);half(table+6,16);
    half(table+32,10);half(table+34,2);word(table+36,96);word(table+40,128);
    half(table+64,10);half(table+66,2);word(table+68,160);word(table+72,192);
    half(table+192,30);half(table+194,2);word(table+196,0x80000001U);word(table+200,0x765432ABU);
    memcpy(before,storage,sizeof(storage));
    CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==DREADZONE_TREE_READ);
    CHECK(leaf==0x765432ABU && memcmp(before,storage,sizeof(storage))==0);
    r.z=30;CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==1 && leaf==0x80000001U);
    r.z=32;CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==1 && leaf==0);
    r.y=19;CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==1 && leaf==0);
    r.y=21;r.x=9;CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==1 && leaf==0);
    r.x=11;r.z=31;word(table+72,0);CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==1 && leaf==0);
    word(table+72,192);r.bytes=201;leaf=0xDEADBEEFU;
    CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==DREADZONE_TREE_UNSUPPORTED);
    CHECK(leaf==0xDEADBEEFU);r.bytes=256;
    r.y=(-2147483647-1);CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==DREADZONE_TREE_UNSUPPORTED);
    CHECK(leaf==0xDEADBEEFU);r.y=21;
    half(table,0);r.y=(-2147483647-1);
    CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==DREADZONE_TREE_UNSUPPORTED);
    CHECK(leaf==0xDEADBEEFU);half(table,20);r.y=21;
    half(table+2,0);CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==1 && leaf==0);
    half(table+2,2);half(table+6,0);CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==1 && leaf==0);
    half(table+6,16);leaf=0xDEADBEEFU;
    word(table+72,193);CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==DREADZONE_TREE_UNSUPPORTED);
    CHECK(leaf==0xDEADBEEFU);word(table+72,192);
    r.guest_base=0xFFFFFFF0U;r.bytes=32;
    CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==DREADZONE_TREE_UNSUPPORTED);
    CHECK(leaf==0xDEADBEEFU);r.guest_base=0x400000;r.bytes=256;
    CHECK(dreadzone_collision_tree_lookup(NULL,&leaf)==DREADZONE_TREE_INVALID);
    CHECK(dreadzone_collision_tree_lookup(&r,NULL)==DREADZONE_TREE_INVALID);
    CHECK(dreadzone_collision_tree_lookup(&r,(u32 *)(table+200))==DREADZONE_TREE_INVALID);
    CHECK(dreadzone_collision_tree_lookup(&r,&r.root)==DREADZONE_TREE_INVALID);
    CHECK(dreadzone_collision_tree_lookup(&r,(u32 *)(uintptr_t)(UINTPTR_MAX-1))==DREADZONE_TREE_INVALID);
    r.storage=(u8 *)(uintptr_t)(UINTPTR_MAX-3);
    CHECK(dreadzone_collision_tree_lookup(&r,&leaf)==DREADZONE_TREE_INVALID);
    printf("%u checks, %u failures\n",checks,failures);return failures ? 1 : 0;
}
