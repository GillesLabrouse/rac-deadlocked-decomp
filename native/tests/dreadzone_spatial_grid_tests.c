#include "dreadzone_spatial_grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static u8 actor[258],grid[0x4002],blocks[0xC002],bitmap[0xC2];
static u8 original_actor[258],original_grid[0x4002],original_blocks[0xC002],original_bitmap[0xC2];
static unsigned checks;
#define REQUIRE(v) do { ++checks;if(!(v)){fprintf(stderr,"Spatial check line%d: %s\n",__LINE__,#v);exit(1);} } while(0)
static void put32(u8 *p,u32 v) { memcpy(p,&v,4); }
static u16 get16(const u8 *p) { u16 v;memcpy(&v,p,2);return v; }
static u32 get32(const u8 *p) { u32 v;memcpy(&v,p,4);return v; }
static DreadzoneSpatialRequest fixture(void) {
    DreadzoneSpatialRequest r;
    memset(actor,0xA5,sizeof(actor));memset(grid,0,sizeof(grid));
    memset(blocks,0xA5,sizeof(blocks));memset(bitmap,0,sizeof(bitmap));
    grid[0]=grid[sizeof(grid)-1]=bitmap[0]=bitmap[sizeof(bitmap)-1]=0xA5;
    put32(actor+1+0xA4,0xFFFFFFFFU);
    r=(DreadzoneSpatialRequest){actor+1,0x100,grid+1,0x4000,blocks+1,0xC000,
        bitmap+1,0xC0,0x01001100,0x01000000,128,0x03030303};
    return r;
}
static void snapshot(void) {
    memcpy(original_actor,actor,sizeof(actor));memcpy(original_grid,grid,sizeof(grid));
    memcpy(original_blocks,blocks,sizeof(blocks));memcpy(original_bitmap,bitmap,sizeof(bitmap));
}
static void unchanged(void) {
    REQUIRE(!memcmp(original_actor,actor,sizeof(actor)));REQUIRE(!memcmp(original_grid,grid,sizeof(grid)));
    REQUIRE(!memcmp(original_blocks,blocks,sizeof(blocks)));REQUIRE(!memcmp(original_bitmap,bitmap,sizeof(bitmap)));
}
static void guards(void) {
    REQUIRE(actor[0]==0xA5 && actor[257]==0xA5);
    REQUIRE(grid[0]==0xA5 && grid[0x4001]==0xA5);
    REQUIRE(blocks[0]==0xA5 && blocks[0xC001]==0xA5);
    REQUIRE(bitmap[0]==0xA5 && bitmap[0xC1]==0xA5);
}
static void movement(void) {
    DreadzoneSpatialRequest r=fixture();u8 *cell=grid+1+(3+3*64)*4;
    u32 i;
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_COMMITTED);
    REQUIRE(get16(cell)==0 && cell[2]==1 && cell[3]==1 && bitmap[1]==1);
    REQUIRE(get16(blocks+1)==17 && get32(actor+1+0xA4)==r.rectangle);
    for(i=2;i<32;++i) REQUIRE(blocks[1+i]==0xA5);
    snapshot();REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_COMMITTED);unchanged();
    r.rectangle=0x03030404;
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_INVALID);unchanged();
    r.rectangle=0x04040404;
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_COMMITTED);
    REQUIRE(cell[2]==0 && cell[3]==0 && get16(cell)==0);
    cell=grid+1+(4+4*64)*4;REQUIRE(cell[2]==1 && cell[3]==1 && get16(cell)==0);
    REQUIRE(get16(blocks+1)==17 && bitmap[1]==1);guards();
    r.rectangle=0xFFFFFFFFU;
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_COMMITTED);
    REQUIRE(cell[2]==0 && cell[3]==0 && bitmap[1]==0 && get16(blocks+1)==0);guards();
}
static void capacity(void) {
    DreadzoneSpatialRequest r=fixture();u8 *cell=grid+1+(3+3*64)*4;u32 i;
    cell[2]=16;cell[3]=1;bitmap[1]=1;
    for(i=0;i<16;++i) { u16 v=(u16)i;memcpy(blocks+1+i*2,&v,2); }
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_COMMITTED);
    REQUIRE(get16(cell)==2 && cell[2]==17 && cell[3]==2 && bitmap[1]==0xC);
    for(i=0;i<16;++i) REQUIRE(get16(blocks+1+64+i*2)==i);
    REQUIRE(get16(blocks+1+96)==17);guards();
    r=fixture();cell=grid+1+(3+3*64)*4;cell[2]=12;cell[3]=2;bitmap[1]=3;
    for(i=0;i<12;++i) { u16 v=(u16)i;memcpy(blocks+1+i*2,&v,2); }
    { u16 id=17;memcpy(blocks+1+4,&id,2); }
    put32(actor+1+0xA4,0x03030303);r.rectangle=0xFFFFFFFFU;
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_COMMITTED);
    REQUIRE(get16(cell)==0 && cell[2]==11 && cell[3]==1 && bitmap[1]==1);
    REQUIRE(get16(blocks+1+4)==11 && get16(blocks+1+22)==0);guards();
    /* Emptying an unusually oversized two-block cell halves it to one
     * allocated block, as retail does; it does not normalize capacity to zero. */
    r=fixture();cell=grid+1+(3+3*64)*4;cell[2]=1;cell[3]=2;bitmap[1]=3;
    {u16 id=17;memcpy(blocks+1,&id,2);}
    put32(actor+1+0xA4,0x03030303);r.rectangle=0xFFFFFFFFU;
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_COMMITTED);
    REQUIRE(cell[2]==0 && cell[3]==1 && get16(cell)==0 && bitmap[1]==1);
    REQUIRE(get16(blocks+1)==0);guards();
    /* Count is a serialized byte. The256th membership write wraps it to zero
     * without an allocation, rather than silently changing the record layout. */
    r=fixture();cell=grid+1+(3+3*64)*4;cell[2]=255;cell[3]=16;
    bitmap[1]=bitmap[2]=255;
    for(i=0;i<255;++i) {u16 id=(u16)(i%128);memcpy(blocks+1+i*2,&id,2);}
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_COMMITTED);
    REQUIRE(cell[2]==0 && cell[3]==16 && get16(blocks+1+510)==17);
    REQUIRE(bitmap[1]==255 && bitmap[2]==255);guards();
    r=fixture();r.actor_pool_count=65536;r.actor_guest=0x01FFFF00U;
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_COMMITTED);
    REQUIRE(get16(blocks+1)==65535);guards();
}
static void refusals(void) {
    DreadzoneSpatialRequest r=fixture(),bad;
    memset(bitmap+1,255,0xC0);snapshot();
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_EXHAUSTED);unchanged();
    r=fixture();put32(actor+1+0xA4,0x03030303);r.rectangle=0xFFFFFFFFU;snapshot();
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_TRAP);unchanged();
    r=fixture();snapshot();bad=r;bad.bitmap=bad.grid;
    REQUIRE(dreadzone_spatial_update(&bad)==DREADZONE_SPATIAL_INVALID);unchanged();
    bad=r;bad.actor_guest+=1;
    REQUIRE(dreadzone_spatial_update(&bad)==DREADZONE_SPATIAL_INVALID);unchanged();
    bad=r;bad.actor_pool_guest=0xFFFFFF00U;bad.actor_guest=0xFFFFFF00U;
    REQUIRE(dreadzone_spatial_update(&bad)==DREADZONE_SPATIAL_INVALID);unchanged();
    bad=r;bad.block_bytes=0xBFFF;
    REQUIRE(dreadzone_spatial_update(&bad)==DREADZONE_SPATIAL_INVALID);unchanged();
    bad=r;bad.rectangle=0x40400000U;
    REQUIRE(dreadzone_spatial_update(&bad)==DREADZONE_SPATIAL_INVALID);unchanged();
    { DreadzoneSpatialRequest *aliased=(DreadzoneSpatialRequest *)(void *)(blocks+16);
      memcpy(aliased,&r,sizeof(r));snapshot();
      REQUIRE(dreadzone_spatial_update(aliased)==DREADZONE_SPATIAL_INVALID);unchanged(); }
    r=fixture();bad=r;bad.actor=(u8 *)(uintptr_t)(UINTPTR_MAX-31);snapshot();
    REQUIRE(dreadzone_spatial_update(&bad)==DREADZONE_SPATIAL_INVALID);unchanged();
    REQUIRE(dreadzone_spatial_update((const DreadzoneSpatialRequest *)(UINTPTR_MAX-8))==DREADZONE_SPATIAL_INVALID);unchanged();
    bad=r;bad.actor_pool_guest=0x7FFFFF00U;bad.actor_guest=0x80000000U;
    REQUIRE(dreadzone_spatial_update(&bad)==DREADZONE_SPATIAL_INVALID);unchanged();
    /* The first old cell can be removed before a later missing-member trap.
     * Native preflight must keep that earlier mutation private as well. */
    r=fixture();put32(actor+1+0xA4,0x03040303);r.rectangle=0xFFFFFFFFU;
    grid[1+(3+3*64)*4+2]=1;grid[1+(3+3*64)*4+3]=1;
    bitmap[1]=1;{u16 id=17;memcpy(blocks+1,&id,2);}snapshot();
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_TRAP);unchanged();
    /* A removed block supplies the first new cell, then the second cell runs
     * out. Neither the earlier removal nor that allocation may publish. */
    r=fixture();put32(actor+1+0xA4,0x03030303);r.rectangle=0x05060505;
    grid[1+(3+3*64)*4+2]=1;grid[1+(3+3*64)*4+3]=1;
    memset(bitmap+1,255,0xC0);{u16 id=17;memcpy(blocks+1,&id,2);}snapshot();
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_EXHAUSTED);unchanged();
    r=fixture();put32(actor+1+0xA4,0x03030303);r.rectangle=0xFFFFFFFFU;
    {u16 invalid_first=65535;memcpy(grid+1+(3+3*64)*4,&invalid_first,2);}snapshot();
    REQUIRE(dreadzone_spatial_update(&r)==DREADZONE_SPATIAL_INVALID);unchanged();
}
int main(void) {
    movement();capacity();refusals();
    printf("Spatial grid: %u checks passed\n",checks);return 0;
}
