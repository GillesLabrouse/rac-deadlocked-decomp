#include "dreadzone_display_projection_ram.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fenv.h>

#define BASE 0x1D3F40U
#define BYTES (0x2400ACU-BASE)
#define CHECK(x) do { if(!(x)) {fprintf(stderr,"projection check failed line%d\n",__LINE__);return 1;} } while(0)
static void dimensions(uint8_t *p,int16_t width,int16_t height) {
    memcpy(p+(0x206B90U-BASE),&width,2);memcpy(p+(0x206B92U-BASE),&height,2);
}
static uint32_t word(const uint8_t *p,uint32_t address) {
    uint32_t result;memcpy(&result,p+(address-BASE),4);return result;
}
int main(void) {
    uint8_t *allocation=(uint8_t *)malloc(BYTES+26),*before=(uint8_t *)malloc(BYTES);
    uint8_t *ram;uint32_t zw[2]={0x7FC12345U,0x80000000U};int mode;
    CHECK(allocation&&before);ram=allocation+13;memset(allocation,0xA7,BYTES+26);
    dimensions(ram,512,416);memcpy(ram+(0x23FFE8U-BASE),zw,8);
    CHECK(dreadzone_display_projection_ram_005469A0(ram,BASE,BYTES)==1);
    CHECK(word(ram,0x240050)==0x43800000U&&word(ram,0x240054)==0x43500000U);
    CHECK(word(ram,0x240058)==0x44800000U&&word(ram,0x24005C)==0x44500000U);
    CHECK(!memcmp(ram+(0x23FFE8U-BASE),zw,8));
    CHECK(allocation[12]==0xA7&&allocation[BYTES+13]==0xA7);
    dimensions(ram,511,415);
    CHECK(dreadzone_display_projection_ram_005469A0(ram,BASE,BYTES)==1);
    CHECK(word(ram,0x1D3F48)==255&&word(ram,0x1D3F4C)==207);
    CHECK(word(ram,0x240084)==510&&word(ram,0x24008C)==414);
    memcpy(before,ram,BYTES);
    CHECK(dreadzone_display_projection_ram_005469A0(ram,BASE,BYTES-1)==-1);
    CHECK(!memcmp(before,ram,BYTES));
    dimensions(ram,0,415);memcpy(before,ram,BYTES);
    CHECK(dreadzone_display_projection_ram_005469A0(ram,BASE,BYTES)==-2);
    CHECK(!memcmp(before,ram,BYTES));
    dimensions(ram,512,416);memcpy(before,ram,BYTES);mode=fegetround();
    CHECK(fesetround(FE_DOWNWARD)==0);
    CHECK(dreadzone_display_projection_ram_005469A0(ram,BASE,BYTES)==-2);
    CHECK(fesetround(mode)==0);CHECK(!memcmp(before,ram,BYTES));
    CHECK(dreadzone_display_projection_ram_005469A0(NULL,BASE,BYTES)==-1);
    free(before);free(allocation);puts("PASS projection RAM contract, minimal unaligned mapping and atomic refusals");return 0;
}
