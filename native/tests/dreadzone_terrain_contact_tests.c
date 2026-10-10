#include "dreadzone_terrain_contact.h"
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"Failed line%d: %s\n",__LINE__,#x);return 1;}}while(0)
static void put16(u8 *p,u32 x) {u16 v=(u16)x;memcpy(p,&v,2);}
static void put32(u8 *p,u32 x) {memcpy(p,&x,4);}
static u32 vertex(s32 x,s32 y,s32 z) {
    return ((u32)(x/64)&1023U)|(((u32)(y/64)&1023U)<<10)|(((u32)(z/16)&4095U)<<20);
}
static void table(u8 data[192],u32 columns) {
    u32 index;memset(data,0,192);put16(data,0);put16(data+2,1);put16(data+4,2);
    /* Root indexes physical Z, row physical Y, final node physical X. */
    put16(data+8,0);put16(data+10,1);put32(data+12,24);
    put16(data+24,0);put16(data+26,columns);
    for(index=0;index<columns;index++) {
        put32(data+28+index*4,((64+index*32)<<8)|2);
    }
}
static void triangle(u8 *block) {
    block[0]=1;block[2]=3;block[3]=0;
    put32(block+4,vertex(-1024,-1024,0));put32(block+8,vertex(1024,-1024,0));
    put32(block+12,vertex(-1024,1024,0));block[16]=0;block[17]=1;block[18]=2;block[19]=0;
}
int main(void) {
    u8 data[192],before[192];DreadzoneTerrainContactRequest r;
    DreadzoneCollisionTriangleContact c,sentinel;DreadzoneTerrainContactInfo info,old;
    table(data,1);triangle(data+64);memset(&r,0,sizeof(r));r.storage=data;r.guest_base=r.root=0x00400000;r.bytes=192;
    r.start_world[0]=r.end_world[0]=r.start_world[1]=r.end_world[1]=1.75f;
    r.start_world[2]=1;r.end_world[2]=3;memset(&sentinel,0xA5,sizeof(sentinel));c=sentinel;
    memcpy(before,data,192);CHECK(dreadzone_terrain_contact(&r,&c,&info)==1);
    CHECK(c.fraction==0.5f && c.position_scaled[0]==1792 && c.position_scaled[1]==1792 && c.position_scaled[2]==2048);
    CHECK(c.source_guest==r.root && c.hit_metadata==0x1000 && c.normal[2]==-4194304);
    CHECK(info.grid_records==2 && info.cells_processed==1 && info.faces_processed==1 && info.quad_faces_processed==0);
    CHECK(memcmp(data,before,192)==0);
    r.start_world[0]=r.end_world[0]=0.25f;c=sentinel;CHECK(dreadzone_terrain_contact(&r,&c,&info)==0);
    CHECK(memcmp(&c,&sentinel,sizeof(c))==0);
    r.start_world[0]=r.end_world[0]=1.75f;
    /* Nonintersecting diagonal wall, with a real zero denominator after clips. */
    put32(data+68,vertex(-1024,1024,-1024));put32(data+72,vertex(1024,-1024,-1024));put32(data+76,vertex(-1024,1024,1024));
    CHECK(dreadzone_terrain_contact(&r,&c,&info)==0);CHECK(info.triangle_q_exceptions==0x20);
    /* Quad continuation covers the half absent from the primary face. */
    table(data,1);data[64]=1;data[66]=4;data[67]=1;
    put32(data+68,vertex(-1024,-1024,0));put32(data+72,vertex(1024,-1024,0));
    put32(data+76,vertex(1024,1024,0));put32(data+80,vertex(-1024,1024,0));
    data[84]=0;data[85]=1;data[86]=2;data[87]=0;data[88]=3;
    r.start_world[0]=r.end_world[0]=1.25f;r.start_world[1]=r.end_world[1]=2.75f;
    memcpy(before,data,192);CHECK(dreadzone_terrain_contact(&r,&c,&info)==1);
    CHECK(info.faces_processed==2 && info.quad_faces_processed==1 && c.vertices_scaled[2][0]==1024 && c.vertices_scaled[2][1]==3072);
    CHECK(memcmp(data,before,192)==0);
    /* The next block is fetched but never decoded after an earlier hit. */
    table(data,3);triangle(data+64);triangle(data+96);data[130]=255;
    r.start_world[0]=1.75f;r.end_world[0]=9.75f;r.start_world[1]=r.end_world[1]=1.75f;
    CHECK(dreadzone_terrain_contact(&r,&c,&info)==1);CHECK(c.fraction==0.5f && c.position_scaled[0]==5888);
    CHECK(info.cells_processed==2 && info.faces_processed==2);
    /* A missing next-block mapping must be refused before current contact. */
    r.bytes=128;c=sentinel;memset(&info,0xB6,sizeof(info));old=info;
    CHECK(dreadzone_terrain_contact(&r,&c,&info)==-2);
    CHECK(memcmp(&c,&sentinel,sizeof(c))==0 && memcmp(&info,&old,sizeof(info))==0);
    table(data,1);triangle(data+64);r.bytes=192;r.start_world[0]=r.end_world[0]=1.75f;
    data[82]=255;CHECK(dreadzone_terrain_contact(&r,&c,&info)==-2);
    CHECK(memcmp(&c,&sentinel,sizeof(c))==0 && memcmp(&info,&old,sizeof(info))==0);
    r.caller_flags=1;CHECK(dreadzone_terrain_contact(&r,&c,&info)==-2);
    r.caller_flags=0;CHECK(dreadzone_terrain_contact(&r,(DreadzoneCollisionTriangleContact *)(void *)data,&info)==-1);
    CHECK(dreadzone_terrain_contact(NULL,&c,&info)==-1);
    /* Distinct physical axes prove the caller's real tree-register mapping. */
    table(data,1);triangle(data+64);put16(data,3);put16(data+8,5);put16(data+24,7);
    r.start_world[0]=r.end_world[0]=29.75f;r.start_world[1]=r.end_world[1]=21.75f;
    r.start_world[2]=13;r.end_world[2]=15;c=sentinel;
    CHECK(dreadzone_terrain_contact(&r,&c,&info)==1);
    CHECK(c.fraction==0.5f && c.position_scaled[0]==30464 && c.position_scaled[1]==22272 && c.position_scaled[2]==14336);
    CHECK(info.cells_processed==1 && info.faces_processed==1);
    {
        union {u64 aligned;u8 bytes[sizeof(r)+sizeof(c)];} backing;
        DreadzoneTerrainContactRequest *input=(DreadzoneTerrainContactRequest *)(void *)backing.bytes;
        DreadzoneCollisionTriangleContact *output=(DreadzoneCollisionTriangleContact *)(void *)backing.bytes;
        memcpy(input,&r,sizeof(r));
        CHECK(dreadzone_terrain_contact(input,output,&info)==1);
        CHECK(memcmp(output,&c,sizeof(c))==0);
    }
    printf("%u terrain contact checks passed\n",checks);return 0;
}
