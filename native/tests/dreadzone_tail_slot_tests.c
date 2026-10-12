#include "dreadzone_tail_slot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { BASE=0x100000, BYTES=0x300000 };
static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;printf("FAIL %d: %s\n",__LINE__,#x);}}while(0)
static void put(u8 *p,u32 address,u32 value){memcpy(p+address-BASE,&value,4);}
static u32 get(const u8 *p,u32 address){u32 value;memcpy(&value,p+address-BASE,4);return value;}
static void refusal(u8 *p,DreadzoneTailSlotRequest *r){
    u8 *before=(u8 *)malloc(BYTES);DreadzoneTailSlotInfo info,old;
    CHECK(before!=NULL);if(!before)return;memcpy(before,p,BYTES);memset(&info,0x6c,sizeof(info));old=info;
    CHECK(dreadzone_tail_slot_00535A30(r,&info)<0);
    CHECK(!memcmp(before,p,BYTES));CHECK(!memcmp(&info,&old,sizeof(info)));free(before);
}
int main(void){
    u8 *allocation=(u8 *)malloc(BYTES+16),*p,*before=(u8 *)malloc(BYTES);
    const u32 indices[]={0,1,255};u32 i,entry,pointer;DreadzoneTailSlotRequest r,captured,*alias;
    DreadzoneTailSlotInfo info;
    if(!allocation||!before){free(allocation);free(before);return 1;}p=allocation+1;
    for(i=0;i<3;++i){
        memset(p,0,BYTES);memcpy(before,p,BYTES);r=(DreadzoneTailSlotRequest){p,BASE,BYTES,indices[i]};
        CHECK(dreadzone_tail_slot_00535A30(&r,&info)==1);CHECK(!memcmp(p,before,BYTES));
        CHECK(info.return_low==0&&info.did_clear==0&&info.read_count==1);
        CHECK(info.entry_guest==0x2303A4U+indices[i]*0x470U);
    }
    entry=0x2303A4U+0x470U;pointer=0x300000;
    memset(p,0,BYTES);put(p,entry,pointer);put(p,pointer+0x98,0xDEADBEEF);
    memcpy(before,p,BYTES);put(before,pointer+0x98,0);r=(DreadzoneTailSlotRequest){p,BASE,BYTES,1};
    CHECK(dreadzone_tail_slot_00535A30(&r,&info)==1);CHECK(!memcmp(p,before,BYTES));
    CHECK(info.return_low==pointer&&info.did_clear==1&&info.clear_guest==pointer+0x98);
    /* The only store may clear the table entry after the original pointer load. */
    memset(p,0,BYTES);pointer=entry-0x98;put(p,entry,pointer);
    CHECK(dreadzone_tail_slot_00535A30(&r,&info)==1);CHECK(get(p,entry)==0&&info.return_low==pointer);
    /* Captured descriptor may itself be modified by the original guest store. */
    memset(p,0,BYTES);alias=(DreadzoneTailSlotRequest *)(void *)(p+0x1000);memcpy(alias,&r,sizeof(r));
    pointer=BASE+0x1000+(u32)offsetof(DreadzoneTailSlotRequest,index)-0x98U;put(p,entry,pointer);
    CHECK(dreadzone_tail_slot_00535A30(alias,&info)==1);memcpy(&captured,alias,sizeof(captured));
    CHECK(captured.storage==p&&captured.index==0&&info.slot_guest==0x2306F0U&&info.return_low==pointer);
    /* P need not itself map: the original reaches only the wrapped P+98 store. */
    memset(p,0,BYTES);pointer=0xFFFFFF68U;memcpy(p+entry,&pointer,4);i=0xDEADBEEF;memcpy(p,&i,4);
    r=(DreadzoneTailSlotRequest){p,0,BYTES,1};
    CHECK(dreadzone_tail_slot_00535A30(&r,&info)==1);memcpy(&i,p,4);
    CHECK(i==0&&info.return_low==pointer&&info.clear_guest==0&&info.did_clear==1);
    r=(DreadzoneTailSlotRequest){p,BASE,BYTES,1};
    memset(p,0,BYTES);put(p,entry,0x3FFF90);refusal(p,&r); /* mapped pointer, unmapped clear */
    memset(p,0,BYTES);put(p,entry,0x300001);refusal(p,&r); /* guest alignment */
    memset(p,0,BYTES);r.index=256;refusal(p,&r);
    r.index=1;r.storage_bytes=0x1000001;refusal(p,&r); /* declared cap is checked before access */
    r.storage_bytes=BYTES;memcpy(before,p,BYTES);
    CHECK(dreadzone_tail_slot_00535A30(&r,(DreadzoneTailSlotInfo *)(void *)(p+0x2000))<0);
    CHECK(!memcmp(p,before,BYTES));
    printf("tail-slot: %u checks, %u failures\n",checks,failures);free(before);free(allocation);return failures?1:0;
}
