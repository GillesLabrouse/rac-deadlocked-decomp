#include "dreadzone_wall_helpers.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
static int checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;printf("FAIL line%d: %s\n",__LINE__,#x);}}while(0)
static void raw(void *p,u32 v){memcpy(p,&v,4);}
static u32 word(const void *p){u32 v;memcpy(&v,p,4);return v;}
int main(void){
    u8 backing[0x2680],normal[20],before[0x2680],packet[40];u8 *record=backing+1;
    f32 angle; s32 surface;u32 mode,i;
    {volatile size_t widths[2]={sizeof(u64),sizeof(s64)};CHECK(widths[0]==8&&widths[1]==8);}
    memset(packet,0xA5,sizeof(packet));
    for(i=0;i<16;++i){raw(packet+0x1C,0x70000100U+i);surface=44;
        CHECK(dreadzone_wall_surface_00542FF8(packet,32,&surface)==1);CHECK(surface==(i==15?-1:(s32)i));}
    raw(packet+0x1C,0x80000002U);CHECK(dreadzone_wall_surface_00542FF8(packet,32,&surface)==1&&surface==-1);
    raw(packet+0x1C,12);CHECK(dreadzone_wall_surface_00542FF8(packet,32,(s32 *)(packet+0x1C))==1&&word(packet+0x1C)==12);
    surface=77;CHECK(dreadzone_wall_surface_00542FF8(packet,31,&surface)==-1&&surface==77);
    CHECK(dreadzone_wall_surface_00542FF8(NULL,32,&surface)==-1&&surface==77);
    memset(backing,0xA5,sizeof(backing));memset(normal,0,16);record[0x2659]=0;
    raw(normal+8,0x3F800000U);
    CHECK(dreadzone_wall_inclination_0068AFF8(record,0x265A,normal,16,&angle)==1&&word(&angle)==0x33800000U);
    raw(normal,0x3F800000U);raw(normal+8,0);
    CHECK(dreadzone_wall_inclination_0068AFF8(record,0x265A,normal,16,&angle)==1&&word(&angle)==0x3FC90FDAU);
    /* The zero branch and signed-zero raw lanes are real polynomial inputs. */
    for(i=0;i<16;++i){u32 lane;for(lane=0;lane<4;++lane)raw(normal+lane*4,(i&(1U<<lane))?0x80000000U:0);
        CHECK(dreadzone_wall_inclination_0068AFF8(record,0x265A,normal,16,&angle)==1&&word(&angle)==0);}
    memset(normal,0,16);raw(normal,0x3F800000U);
    for(mode=1;mode<=2;++mode){
        record[0x2659]=(u8)mode;memset(record+0x60,0,64);for(i=0;i<4;++i)raw(record+0x60+i*20,0x3F800000U);
        CHECK(dreadzone_wall_inclination_0068AFF8(record,0x265A,normal,16,&angle)==1&&word(&angle)==0x3FC90FDAU);
        /* Translation row contributes only when the actual normal W is1. */
        raw(normal+12,0x3F800000U);raw(record+0x98,0x3F800000U);
        CHECK(dreadzone_wall_inclination_0068AFF8(record,0x265A,normal,16,&angle)==1&&word(&angle)==0x3F490FDBU);
        raw(normal+12,0);
    }
    for(mode=3;mode<256;++mode){record[0x2659]=(u8)mode;raw(&angle,0xA5A5A5A5U);
        CHECK(dreadzone_wall_inclination_0068AFF8(record,0x265A,NULL,0,&angle)==1&&word(&angle)==0);}
    record[0x2659]=0;memset(normal,0,16);raw(normal,0x3F800000U);memcpy(before,backing,sizeof(backing));
    CHECK(dreadzone_wall_inclination_0068AFF8(record,0x265A,normal,16,(f32 *)(record+0x2658))==1);
    CHECK(word(record+0x2658)==0x3FC90FDAU);CHECK(!memcmp(before,backing,0x2659));
    record[0x2659]=0;raw(&angle,0xA5A5A5A5U);raw(normal+12,0x7F7FFFFFU);
    CHECK(dreadzone_wall_inclination_0068AFF8(record,0x265A,normal,16,&angle)==-2&&word(&angle)==0xA5A5A5A5U);
    raw(normal+12,1);CHECK(dreadzone_wall_inclination_0068AFF8(record,0x265A,normal,16,&angle)==-2&&word(&angle)==0xA5A5A5A5U);
    raw(normal+12,0);raw(normal+8,0x3F80A3D7U);
    CHECK(dreadzone_wall_inclination_0068AFF8(record,0x265A,normal,16,&angle)==-2&&word(&angle)==0xA5A5A5A5U);
    CHECK(dreadzone_wall_inclination_0068AFF8(record,0x2659,normal,16,&angle)==-1&&word(&angle)==0xA5A5A5A5U);
    CHECK(dreadzone_wall_inclination_0068AFF8(record,0x265A,normal,15,&angle)==-1&&word(&angle)==0xA5A5A5A5U);
    CHECK(dreadzone_wall_inclination_0068AFF8((const u8 *)(uintptr_t)(UINTPTR_MAX-0x10),0x265A,normal,16,&angle)==-1);
    printf("Wall helpers: %d checks, %d failures\n",checks,failures);return failures?1:0;
}
