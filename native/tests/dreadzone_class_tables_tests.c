#include "dreadzone_class_tables.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define REQUIRE(test) do { if (!(test)) { fprintf(stderr,"Class table check failed at line %d: %s\n",__LINE__,#test); exit(1); } } while (0)
#define DATA 0x00400000U
#define REMAP 0x00601000U
static u8 tables[0x20002],globals[0x102],dispatch[0x1002],data[0x10002];
static u8 textures[0x1002],remap[0x102],info[0x102],other[0x20002],late[0x10002];
static DreadzoneTableView views[11];
static DreadzoneTableContext context;
static void put32(u8 *p,u32 value) { memcpy(p,&value,4); }
static u32 get32(const u8 *p) { u32 value; memcpy(&value,p,4); return value; }
static u16 get16(const u8 *p) { u16 value; memcpy(&value,p,2); return value; }
static void put64(u8 *p,u64 value) { memcpy(p,&value,8); }
static u64 get64(const u8 *p) { u64 value; memcpy(&value,p,8); return value; }
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
    fixture(); definition(); put32(data+1,0x1000); data[1+4]=1;
    put32(data+1+0x1000,0x02000000); put32(data+1+0x1004,0x00040004);
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
static void remap_quad_alignment_checks(void) {
    u32 offset;
    u8 before[0x10002];
    for (offset=0;offset<16;offset++) {
        fixture(); definition();
        REQUIRE(dreadzone_class_publish_definition(&context,DATA,0,REMAP+offset,9)==DREADZONE_TABLE_COMPLETED);
        REQUIRE(memcmp(tables+1+0xEFB0,remap+1,16)==0);
        /* The byte list uses the exact pointer, independently of LQ. */
        REQUIRE(data[1+0x100]==(u8)(22+offset));
        REQUIRE(data[1+0x101]==(u8)(20+offset));
    }
    fixture(); definition();
    views[5].guest_base=REMAP+1; views[5].bytes=255; views[5].storage=remap+2;
    memcpy(before,data,sizeof(data));
    REQUIRE(dreadzone_class_publish_definition(&context,DATA,0,REMAP+1,9)==DREADZONE_TABLE_INVALID);
    REQUIRE(memcmp(before,data,sizeof(data))==0);
    REQUIRE(get32(globals+1+0x18)==0 && tables[1+0xB590+9]==0xA5);
}
static void geometry_publication_checks(void) {
    u8 *d=data+1,*packet;
    u64 first=0x0123456789ABCDEFULL,second=0xC,third=0xFEDCBA9876543210ULL;
    u32 i;
    fixture(); definition(); d[4]=1; put32(d,0x1000);
    put32(d+0x1000,0x2000); put32(d+0x1004,0x00040004);
    /* Auxiliary count2 must not replace the initial geometry count1. */
    put32(d+0x1014,0xABCD5555);
    packet=d+0x2000; memset(packet,0xCD,64);
    put32(packet,2); put32(packet+4,3); put32(packet+0x10,4); put32(packet+0x14,5); put32(packet+0x20,0);
    put64(tables+1+0x99A0+20*24,first);
    put64(tables+1+0x99A0+20*24+8,second);
    put64(tables+1+0x99A0+20*24+16,third);
    REQUIRE(dreadzone_class_publish_definition(&context,DATA,0,REMAP,9)==DREADZONE_TABLE_COMPLETED);
    REQUIRE(get32(d)==DATA+0x1000 && get32(d+0x1000)==DATA+0x2000);
    REQUIRE(get32(d+0x1004)==4 && get32(d+0x1014)==0xABCD5555);
    REQUIRE(get64(packet)==(((u64)2<<32)|((u64)3<<6)|0x20|(second&0x1C)));
    REQUIRE(get64(packet+0x10)==(4|((u64)5<<2)|((u64)20<<24)));
    REQUIRE(get64(packet+0x20)==first && get64(packet+0x30)==third);
    for(i=0;i<4;i++) REQUIRE(get64(packet+i*16+8)==0xCDCDCDCDCDCDCDCDULL);
    REQUIRE(d[0x47]==0 && get32(globals+1+0x18)==1 && get32(tables+1+0xAA40)==DATA);
    REQUIRE(data[0]==0 && data[sizeof(data)-1]==0);
}
static void auxiliary_reload_checks(void) {
    u8 *d=data+1;
    u8 relocated[10]={0xD3,0,0,0,0,0x30,0,0,0,0x7C};
    fixture(); definition();
    d[8]=1; d[0xC]=8; put32(d+0x48,0xFFFFFFF8U); put32(d+0x10,0);
    /* auxiliary=definition-8: its +0x14 relocation changes the count word
     * from8 to definition. The reloaded low byte0 ends the retail loop. */
    REQUIRE(dreadzone_class_publish_definition(&context,DATA,0,REMAP,9)==DREADZONE_TABLE_COMPLETED);
    REQUIRE(get32(d+0xC)==DATA && get32(d+0x4C)==0x220);
    REQUIRE(get32(d+0x218)==0x40 && get32(d+0x240)==0x10);
    fixture(); definition(); d[8]=1; d[0xC]=1;
    put32(d+0x218,0x18);
    /* The first record aliases auxiliary+0x18 and relocates its pointer
     * again. Retail reloads that pointer before visiting the second word. */
    views[9]=(DreadzoneTableView){0x00800418,8,relocated+1};
    context.view_count=10;
    REQUIRE(dreadzone_class_publish_definition(&context,DATA,0,REMAP,9)==DREADZONE_TABLE_COMPLETED);
    REQUIRE(get32(d+0x218)==0x00800418 && get32(d+0x21C)==DATA+0x260);
    REQUIRE(get32(relocated+5)==DATA+0x230);
    REQUIRE(relocated[0]==0xD3 && relocated[9]==0x7C);
}
static void list_pointer_reload_checks(void) {
    u8 *d=data+1;
    u8 relocated[14]={0xD3,0,0,0,0,0,0,0,0,0x30,0,0,0,0x7C};
    u8 unused[18]={0xD3,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x80,0x7C};
    fixture(); definition();
    put32(d+0x18,2-DATA); put32(d+0x1C,0x18); put32(d+0x20,0);
    views[9]=(DreadzoneTableView){0x00800018,12,relocated+1};
    views[10]=(DreadzoneTableView){0x00800000,16,unused+1};
    context.view_count=11;
    /* The first item is the list pointer itself. The second iteration must
     * reload its new value, and must leave definition+0x20 untouched. */
    REQUIRE(dreadzone_class_publish_definition(&context,DATA,0,REMAP,9)==DREADZONE_TABLE_COMPLETED);
    REQUIRE(get32(d+0x18)==2 && get32(d+0x1C)==0x00800018 && get32(d+0x20)==0);
    REQUIRE(get32(relocated+9)==DATA+0x30);
    REQUIRE(get32(unused+13)==0x80000000U);
    REQUIRE(relocated[0]==0xD3 && relocated[13]==0x7C && unused[0]==0xD3 && unused[17]==0x7C);
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
    publication_checks(); relocation_checks(); remap_quad_alignment_checks(); texture_checks(); reset_checks(); geometry_publication_checks(); auxiliary_reload_checks(); list_pointer_reload_checks();
    puts("Dreadzone actual class table publication and relocation checks passed.");
    return 0;
}
