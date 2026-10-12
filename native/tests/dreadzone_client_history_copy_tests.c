#include "dreadzone_client_history_copy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { BASE=0x230000,BYTES=0x10000 };
static uint8_t *ram,*before,*expected;
static unsigned checks,failures;
#define CHECK(x) do {checks++;if(!(x)){failures++;fprintf(stderr,"client copy failed line%d\n",__LINE__);}}while(0)
static void word(uint8_t *p,uint32_t address,uint32_t value){memcpy(p+(address-BASE),&value,4);}
static void refuse(DreadzoneClientHistoryCopyMemory *view,uint32_t index){
    DreadzoneClientHistoryCopyMemory old=*view;
    memcpy(before,ram,BYTES);
    CHECK(dreadzone_client_history_copy_00535200(view,index)==-1);
    CHECK(!memcmp(before,ram,BYTES));CHECK(!memcmp(&old,view,sizeof(old)));
}
int main(void){
    uint8_t *allocation=malloc(BYTES+2);
    DreadzoneClientHistoryCopyMemory view;
    static const uint32_t slots[]={0,1,46,47};
    unsigned index,s,j;
    before=malloc(BYTES);expected=malloc(BYTES);
    if(!allocation||!before||!expected){free(allocation);free(before);free(expected);return 2;}
    ram=allocation+1;view=(DreadzoneClientHistoryCopyMemory){BASE,ram,BYTES};
    allocation[0]=0xCD;allocation[BYTES+1]=0xDE;
    for(index=0;index<4;index++)for(s=0;s<4;s++){
        uint32_t row=0x230280+index*0x470,hs=0x231AD0+index*0x2100+slots[s]*0xB0;
        uint32_t hd=0x231810+index*0xB0,bs=0x23C010+index*0x780,bd=0x23C510+index*0x780;
        for(j=0;j<BYTES;j++)ram[j]=(uint8_t)(j*37+index*19+s);
        word(ram,row+0x50,hs);memcpy(expected,ram,BYTES);
        memcpy(expected+(hd-BASE),ram+(hs-BASE),0xB0);
        memcpy(expected+(bd-BASE),ram+(bs-BASE),0x280);
        word(expected,hd+0x70,bd);
        CHECK(dreadzone_client_history_copy_00535200(&view,index)==0);
        CHECK(!memcmp(expected,ram,BYTES));
    }
    word(ram,0x2302D0,0x231AD0);refuse(&view,4);
    word(ram,0x2302D0,0xDEAD0000);refuse(&view,0);
    word(ram,0x2302D0,0x231AD1);refuse(&view,0);
    word(ram,0x2302D0,0x233BD0);refuse(&view,0);
    word(ram,0x2302D0,0x231AD0);view.length=0;refuse(&view,0);
    view.length=0x1000001;refuse(&view,0);
    view.length=BYTES;view.guest_base=0x7FFFF000;refuse(&view,0);
    view.guest_base=BASE;view.bytes=(uint8_t *)(UINTPTR_MAX-3);refuse(&view,0);
    view.bytes=ram;view.length=0xC78F;refuse(&view,0);
    view.length=BYTES;
    CHECK(dreadzone_client_history_copy_00535200(NULL,0)==-1);
    CHECK(dreadzone_client_history_copy_00535200((const DreadzoneClientHistoryCopyMemory *)(UINTPTR_MAX-3),0)==-1);
    CHECK(allocation[0]==0xCD&&allocation[BYTES+1]==0xDE);
    free(allocation);free(before);free(expected);
    printf("client history copy: %u checks, %u failures\n",checks,failures);
    return failures?1:0;
}
