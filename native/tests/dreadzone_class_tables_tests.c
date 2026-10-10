#include "dreadzone_class_tables.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define REQUIRE(test) do { if (!(test)) { fprintf(stderr,"Class table check failed at line %d: %s\n",__LINE__,#test); exit(1); } } while (0)
#define DATA 0x00400000U
#define REMAP 0x00601000U
static u8 tables[0x20002],globals[0x102],dispatch[0x1002],data[0x10002];
static u8 textures[0x1002],remap[0x102],info[0x102],other[0x20002],late[0x10002];
static DreadzoneTableView views[9];
static DreadzoneTableContext context;
static void put32(u8 *p,u32 value) { memcpy(p,&value,4); }
static u32 get32(const u8 *p) { u32 value; memcpy(&value,p,4); return value; }
static u16 get16(const u8 *p) { u16 value; memcpy(&value,p,2); return value; }
static void fixture(void) {
    u32 i;
    memset(tables,0xA5,sizeof(tables)); memset(globals,0,sizeof(globals));
    memset(dispatch,0,sizeof(dispatch)); memset(data,0,sizeof(data));
    memset(textures,0,sizeof(textures)); memset(remap,0,sizeof(remap)); memset(info,0,sizeof(info));
    memset(other,0xA5,sizeof(other)); memset(late,0xA5,sizeof(late));
    views[0]=(DreadzoneTableView){0x00260000,0x20000,tables+1};
    views[1]=(DreadzoneTableView){0x00222D00,0x100,globals+1};
    views[2]=(DreadzoneTableView){0x003C5E00,0x1000,dispatch+1};
    views[3]=(DreadzoneTableView){DATA,0x10000,data+1};
    views[4]=(DreadzoneTableView){0x00600000,0x1000,textures+1};
    views[5]=(DreadzoneTableView){REMAP,0x100,remap+1};
    views[6]=(DreadzoneTableView){0x001DFD00,0x100,info+1};
    views[7]=(DreadzoneTableView){0x00340000,0x20000,other+1};
    views[8]=(DreadzoneTableView){0x00280000,0x10000,late+1};
    context.views=views; context.view_count=9;
    put32(dispatch+1,9); put32(dispatch+5,0x00610200); put32(dispatch+9,0x003C6CAC+64);
    put32(dispatch+13,0xFFFFFFFFU); put32(dispatch+17,0x00610300); put32(dispatch+21,0x003C6CAC+96);
    for (i=0;i<256;i++) remap[1+i]=(u8)(i+20);
    put32(tables+1+0xE830,0xDEADBEEF);
}
static void definition(void) {
    u8 *d=data+1;
    d[0x2D]=0xFF; d[0x47]=0xFF;
    put32(d+0x10,0x80); put32(d+0x18,0x90);
    put32(d+0x1C,0xA0); put32(d+0xA0,2); put32(d+0xA4,0xB0); put32(d+0xA8,0xC0);
    put32(d+0x20,0x100); d[0x100]=2; d[0x101]=0; d[0x102]=0xFF; put32(d+0x10C,0x80000000U);
    put32(d+0x28,0x1C0); d[0xC]=2;
    put32(d+0x48,0x200); put32(d+0x4C,0x220);
    d[0x210]=3; put32(d+0x218,0x40); put32(d+0x240,0x10);
    put32(d+0x214,0x50); put32(d+0x21C,0x60);
}
static void publication_checks(void) {
    u8 before[0x10002];
    fixture(); definition();
    put32(globals+1+0x18,239);
    REQUIRE(dreadzone_class_publish_definition(&context,DATA,0,REMAP,9)==DREADZONE_TABLE_COMPLETED);
    REQUIRE(tables[1+0xB590+9]==239 && get16(tables+1+0xB2B0+239*2)==9);
    REQUIRE(get32(tables+1+0xAA40+239*4)==DATA && get32(tables+1+0xAE00+239*4)==0x00610200);
    REQUIRE(tables[1+0xB1C0+239]==2 && get32(globals+1+0x18)==240);
    REQUIRE(get32(tables+1+0xE830+239*4)==0x00100000);
    REQUIRE(memcmp(tables+1+0xEFB0+239*16,remap+1,16)==0);
    REQUIRE(get32(data+1+0x10)==DATA+0x80 && get32(data+1+0x14)==0);
    REQUIRE(get32(data+1+0x1C)==DATA+0xA0 && get32(data+1+0xA4)==DATA+0xB0);
    REQUIRE(data[1+0x100]==22 && data[1+0x101]==20 && data[1+0x102]==255);
    REQUIRE(get32(data+1+0x10C)==0x80400000U && data[1+0x47]==0);
    REQUIRE(get32(data+1+0x48)==DATA+0x200 && data[1+0x210]==1);
    REQUIRE(get32(data+1+0x218)==DATA+0x240 && get32(data+1+0x240)==DATA+0x210);
    REQUIRE(get32(data+1+0x244)==0 && get32(data+1+0x214)==DATA+0x250);
    memcpy(before,data,sizeof(data));
    REQUIRE(dreadzone_class_publish_definition(&context,DATA,0,REMAP,9)==DREADZONE_TABLE_INVALID);
    REQUIRE(memcmp(before,data,sizeof(data))==0);
    fixture(); put32(globals+1+0x1C,127);
    REQUIRE(dreadzone_class_publish_definition(&context,0,0,0,10239)==DREADZONE_TABLE_COMPLETED);
    REQUIRE(tables[1+0xB590+10239]==127 && get16(tables+1+0xB490+254)==10239);
    REQUIRE(get32(tables+1+0xDD90+127*4)==0x00610300 && tables[1+0xDF90+127]==3);
    REQUIRE(get32(globals+1+0x1C)==128);
    REQUIRE(dreadzone_class_publish_definition(&context,0,0,0,9)==DREADZONE_TABLE_INVALID);
    REQUIRE(dreadzone_class_publish_definition(&context,0,0,0,10240)==DREADZONE_TABLE_INVALID);
    REQUIRE(dreadzone_class_publish_dispatch(&context,-1,0)==DREADZONE_TABLE_INVALID);
}
static void relocation_checks(void) {
    u8 before[0x10002];
    fixture(); definition(); data[1+0x213]=1;
    REQUIRE(dreadzone_class_publish_definition(&context,DATA,0,REMAP,9)==DREADZONE_TABLE_COMPLETED);
    REQUIRE(get32(data+1+0x48)==DATA+0x200 && get32(data+1+0x4C)==0x220);
    REQUIRE(data[1+0x47]==255 && tables[1+0xEFB0]==0xA5 && get32(globals+1+0x18)==1);
    fixture(); definition(); put32(data+1,0x1000);
    memcpy(before,data,sizeof(data));
    REQUIRE(dreadzone_class_publish_definition(&context,DATA,0,REMAP,9)==DREADZONE_TABLE_NEEDS_GEOMETRY);
    REQUIRE(memcmp(before,data,sizeof(data))==0 && get32(globals+1+0x18)==0 && tables[1+0xB590+9]==0xA5);
    fixture(); definition(); views[5].bytes=2;
    memcpy(before,data,sizeof(data));
    REQUIRE(dreadzone_class_publish_definition(&context,DATA,0,REMAP,9)==DREADZONE_TABLE_INVALID);
    REQUIRE(memcmp(before,data,sizeof(data))==0 && get32(globals+1+0x18)==0);
    fixture(); views[8]=views[7];
    REQUIRE(dreadzone_class_publish_definition(&context,0,0,0,9)==DREADZONE_TABLE_INVALID);
    fixture(); views[8].storage=views[7].storage;
    REQUIRE(dreadzone_class_publish_definition(&context,0,0,0,9)==DREADZONE_TABLE_INVALID);
    fixture(); views[8].guest_base=0xFFFFFFFFU;
    REQUIRE(dreadzone_class_publish_definition(&context,0,0,0,9)==DREADZONE_TABLE_INVALID);
    fixture(); data[1+0x47]=255;
    REQUIRE(dreadzone_class_relocate_definition(&context,DATA,0,0,-1)==DREADZONE_TABLE_COMPLETED);
    REQUIRE(data[1+0x47]==0);
}
static void texture_checks(void) {
    u8 *d=data+1;
    fixture(); put32(d+0x20,0x100); d[0x100]=255; put32(d+0x10C,0x80000000U); d[0xB]=0x10;
    remap[1]=0; put32(textures+1+8,0xFFFE0000U); put32(info+1+0x1C,0x01000000U);
    REQUIRE(dreadzone_class_relocate_definition(&context,DATA,0x00600000,REMAP,-1)==DREADZONE_TABLE_COMPLETED);
    REQUIRE(get16(d+0xF0)==0xFFFE);
    fixture(); put32(d+0x20,0x100); d[0x100]=255; put32(d+0x10C,0x80000000U); d[0xB]=0x10;
    remap[1]=0; put32(tables+1+0x99A4,0x00124680U);
    REQUIRE(dreadzone_class_relocate_definition(&context,DATA,0,REMAP,-1)==DREADZONE_TABLE_COMPLETED);
    REQUIRE(get16(d+0xF0)==((0x00124680U>>5)&0x3FFF));
}
static void reset_checks(void) {
    fixture();
    REQUIRE(dreadzone_class_reset_table_slice(&context)==DREADZONE_TABLE_COMPLETED);
    REQUIRE(get32(tables+1+0xB590)==0xFFFFFFFFU && tables[1+0xDD8F]==255 && tables[1+0xDD90]==0xA5);
    REQUIRE(get32(tables+1+0xB2B0)==0xFFFFFFFFU && tables[1+0xB48F]==255 && tables[1+0xB490]==0xA5);
    REQUIRE(get32(tables+1+0xEFB0)==0xFFFFFFFFU && tables[1+0xFEAF]==255);
    REQUIRE(get32(tables+1+0xE830)==0 && tables[1+0xE91F]==0 && tables[1+0xE920]==0xA5);
    REQUIRE(get32(other+1+0x13940)==0 && get32(other+1+0x13D40)==0xFFFFFFFFU);
    REQUIRE(get32(late+1+0xDA20)==0 && get32(late+1+0xDC1C)==0);
    REQUIRE(tables[0]==0xA5 && tables[sizeof(tables)-1]==0xA5 && data[0]==0 && data[sizeof(data)-1]==0);
    fixture(); views[8].bytes=0xDA20;
    REQUIRE(dreadzone_class_reset_table_slice(&context)==DREADZONE_TABLE_INVALID);
    REQUIRE(get32(tables+1+0xB590)==0xA5A5A5A5U);
}
int main(void) {
    publication_checks(); relocation_checks(); texture_checks(); reset_checks();
    puts("Dreadzone actual class table publication and relocation checks passed.");
    return 0;
}
