#include "dreadzone_world_projection.h"
#include <fenv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE 0x223650U
#define BYTES (0x3B9970U-BASE)
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"world projection check failed line %d\n",__LINE__); return 1; } } while(0)
static void scalar(uint8_t *ram,uint32_t address,float value) {
    memcpy(ram+(address-BASE),&value,4);
}
static uint32_t word(const uint8_t *ram,uint32_t address) {
    uint32_t value;
    memcpy(&value,ram+(address-BASE),4);
    return value;
}
int main(void) {
    uint8_t *allocation=(uint8_t *)malloc(BYTES+26),*before=(uint8_t *)malloc(BYTES);
    uint8_t *ram,descriptor[sizeof(DreadzoneWorldProjectionMemory)+1];
    DreadzoneWorldProjectionMemory view,bad;
    int round_mode;
    CHECK(allocation&&before);
    memset(allocation,0xA7,BYTES+26);ram=allocation+13;
    view.bytes=ram;view.guest_base=BASE;view.length=BYTES;
    scalar(ram,0x223650,6.0f);scalar(ram,0x223654,4.0f);
    scalar(ram,0x223658,2.0f);scalar(ram,0x240070,1.0f);
    memcpy(descriptor+1,&view,sizeof(view));
    CHECK(dreadzone_world_projection_00628210((const DreadzoneWorldProjectionMemory *)(const void *)(descriptor+1))==0);
    CHECK(word(ram,0x223660)==6144&&word(ram,0x223664)==4096&&word(ram,0x223668)==2048);
    CHECK(word(ram,0x3B9930)==0x3E800000U&&word(ram,0x3B9934)==0xBF000000U);
    CHECK(word(ram,0x3B9938)==0&&word(ram,0x3B993C)==0x40C00000U);
    CHECK(word(ram,0x3B9950)==0xBF800000U&&word(ram,0x3B9954)==0x40400000U);
    CHECK(word(ram,0x3B9960)==0xBF000000U&&word(ram,0x3B9964)==0x40000000U);
    CHECK(word(ram,0x3B9968)==0&&word(ram,0x3B996C)==0);
    CHECK(allocation[12]==0xA7&&allocation[BYTES+13]==0xA7);
    memcpy(before,ram,BYTES);bad=view;bad.length--;
    CHECK(dreadzone_world_projection_00628210(&bad)==-1&&!memcmp(before,ram,BYTES));
    bad=view;bad.guest_base++;
    CHECK(dreadzone_world_projection_00628210(&bad)==-1&&!memcmp(before,ram,BYTES));
    scalar(ram,0x240070,0.0f);memcpy(before,ram,BYTES);
    CHECK(dreadzone_world_projection_00628210(&view)==-1&&!memcmp(before,ram,BYTES));
    scalar(ram,0x240070,1.0f);scalar(ram,0x223650,0.1f);memcpy(before,ram,BYTES);
    CHECK(dreadzone_world_projection_00628210(&view)==-1&&!memcmp(before,ram,BYTES));
    scalar(ram,0x223650,6.0f);memcpy(before,ram,BYTES);round_mode=fegetround();
    CHECK(fesetround(FE_DOWNWARD)==0);
    CHECK(dreadzone_world_projection_00628210(&view)==-1&&!memcmp(before,ram,BYTES));
    CHECK(fesetround(round_mode)==0);
    memcpy(ram,&view,sizeof(view));memcpy(before,ram,BYTES);
    CHECK(dreadzone_world_projection_00628210((const DreadzoneWorldProjectionMemory *)(const void *)ram)==-1);
    CHECK(!memcmp(before,ram,BYTES));
    CHECK(dreadzone_world_projection_00628210(NULL)==-1);
    free(before);free(allocation);
    puts("PASS world projection minimal unaligned RAM/descriptor and atomic domain guards");
    return 0;
}
