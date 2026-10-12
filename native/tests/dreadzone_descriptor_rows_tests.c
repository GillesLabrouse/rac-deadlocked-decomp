#include "dreadzone_descriptor_rows.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE 0x222990U
#define SOURCE 0x300000U
#define CURSOR 0x300800U
#define OUTPUT 0x31000CU
#define BYTES (OUTPUT+32U-BASE)
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"descriptor rows check failed line %d\n",__LINE__);return 1; } } while(0)
static void put(uint8_t *p,uint32_t a,uint32_t v) { memcpy(p+a-BASE,&v,4); }
static uint32_t get(const uint8_t *p,uint32_t a) { uint32_t v;memcpy(&v,p+a-BASE,4);return v; }
int main(void) {
    uint8_t *allocation=(uint8_t *)malloc(BYTES+26),*before=(uint8_t *)malloc(BYTES),*ram;
    DreadzoneDescriptorRowsMemory view,bad;
    const uint32_t words[8]={0x12345678,0x7FC12345,0x80000000,0x7F800001,0xFF800000,0x00000001,0xFFFFFFFF,0x87654321};
    unsigned i;
    CHECK(allocation&&before);memset(allocation,0xA7,BYTES+26);ram=allocation+13;
    view.bytes=ram;view.guest_base=BASE;view.length=BYTES;
    put(ram,SOURCE,1);put(ram,CURSOR,OUTPUT);
    for(i=0;i<8;i++)put(ram,SOURCE+16+4*i,words[i]);
    CHECK(dreadzone_descriptor_rows_0057B498(&view,SOURCE,CURSOR)==0);
    CHECK(get(ram,BASE)==OUTPUT&&get(ram,BASE+4)==1&&get(ram,CURSOR)==0x310040U);
    CHECK(get(ram,OUTPUT)==words[1]&&get(ram,OUTPUT+4)==words[2]&&get(ram,OUTPUT+8)==words[3]);
    CHECK(get(ram,OUTPUT+12)==words[0]);
    for(i=4;i<8;i++)CHECK(get(ram,OUTPUT+4*i)==words[i]);
    for(i=0;i<8;i++)CHECK(get(ram,SOURCE+16+4*i)==words[i]);
    CHECK(allocation[12]==0xA7&&allocation[BYTES+13]==0xA7);
    put(ram,CURSOR,OUTPUT);memcpy(before,ram,BYTES);bad=view;bad.length--;
    CHECK(dreadzone_descriptor_rows_0057B498(&bad,SOURCE,CURSOR)==-1&&!memcmp(before,ram,BYTES));
    put(ram,CURSOR,SOURCE+16);memcpy(before,ram,BYTES);
    CHECK(dreadzone_descriptor_rows_0057B498(&view,SOURCE,CURSOR)==-1&&!memcmp(before,ram,BYTES));
    put(ram,CURSOR,OUTPUT);put(ram,SOURCE,0xFFFFFFFF);memcpy(before,ram,BYTES);
    CHECK(dreadzone_descriptor_rows_0057B498(&view,SOURCE,CURSOR)==-1&&!memcmp(before,ram,BYTES));
    put(ram,SOURCE,0);memcpy(before,ram+OUTPUT-BASE,32);
    CHECK(dreadzone_descriptor_rows_0057B498(&view,SOURCE,CURSOR)==0);
    CHECK(get(ram,BASE)==OUTPUT&&get(ram,BASE+4)==0&&get(ram,CURSOR)==0x310040U);
    CHECK(!memcmp(before,ram+OUTPUT-BASE,32));
    CHECK(dreadzone_descriptor_rows_0057B498(NULL,SOURCE,CURSOR)==-1);
    free(before);free(allocation);
    puts("PASS descriptor raw-bit transport, cursor alignment, empty input and atomic guards");return 0;
}
