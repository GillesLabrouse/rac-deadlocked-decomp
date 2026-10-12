#include "dreadzone_slot_word.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { BASE=0x00100000U, RAM_BYTES=0x00300000U };
static unsigned checks,failures;
#define CHECK(x) do { ++checks; if(!(x)){++failures;fprintf(stderr,"slot line %d: %s\n",__LINE__,#x);} } while(0)
static void put32(u8 *p,u32 a,u32 v){memcpy(p+a-BASE,&v,4);}
static u32 get32(const u8 *p,u32 a){u32 v;memcpy(&v,p+a-BASE,4);return v;}
static int same(const u8 *a,const u8 *b){return memcmp(a,b,RAM_BYTES)==0;}
static void null_case(u8 *p,u32 index){
    DreadzoneSlotWordRequest r={p,BASE,RAM_BYTES,index};DreadzoneSlotWordInfo info;
    u32 slot=0x230280U+index*0x470U;
    memset(p,0,RAM_BYTES);memset(&info,0xA5,sizeof(info));
    CHECK(dreadzone_slot_word_00535A58(&r,&info)==1);CHECK(info.slot_guest==slot);CHECK(info.entry_guest==slot+0x124U);
    CHECK(info.entry_pointer==0&&info.read_count==1&&info.read_guest[0]==slot+0x124U);
    CHECK(info.return_low==0x230280U&&info.did_store==0);
}
int main(void){
    u8 *allocation,*p,*before;DreadzoneSlotWordRequest r,*alias;
    DreadzoneSlotWordRequest captured;DreadzoneSlotWordInfo info,old;u32 entry=0x2303A4U,P=0x00300000U,Q=0x00310000U,payload=0x80000001U;int status;
    allocation=(u8 *)malloc(RAM_BYTES+16);before=(u8 *)malloc(RAM_BYTES);if(!allocation||!before)return 2;
    p=allocation+1; /* Guest words remain aligned; host RAM is deliberately unaligned. */
    null_case(p,0);null_case(p,1);null_case(p,255);

    memset(p,0,RAM_BYTES);put32(p,entry,P);put32(p,P+0x24,Q);put32(p,Q+0x10,payload);
    r=(DreadzoneSlotWordRequest){p,BASE,RAM_BYTES,0};memset(&info,0,sizeof(info));
    CHECK(dreadzone_slot_word_00535A58(&r,&info)==1);CHECK(info.return_low==Q&&info.did_store==1);
    CHECK(info.read_count==3&&info.read_guest[0]==entry&&info.read_guest[1]==P+0x24&&info.read_guest[2]==Q+0x10);
    CHECK(info.store_guest==P+0x98&&get32(p,P+0x98)==payload);

    /* P+0x98 aliases the slot pointer. The original loads the pointer first, then commits the payload. */
    memset(p,0,RAM_BYTES);P=entry-0x98U;Q=0x00310000U;
    put32(p,entry,P);put32(p,P+0x24,Q);put32(p,Q+0x10,0xFEDCBA98U);
    r=(DreadzoneSlotWordRequest){p,BASE,RAM_BYTES,0};CHECK(dreadzone_slot_word_00535A58(&r,&info)==1);
    CHECK(info.entry_pointer==P&&info.return_low==Q&&info.store_guest==entry);
    CHECK(get32(p,entry)==0xFEDCBA98U);

    /* The request itself may live in the backing RAM; it is captured before any guest store. */
    memset(p,0,RAM_BYTES);put32(p,entry,P);put32(p,P+0x24,Q);put32(p,Q+0x10,payload);
    alias=(DreadzoneSlotWordRequest *)(void *)(p+0x1000);r=(DreadzoneSlotWordRequest){p,BASE,RAM_BYTES,0};memcpy(alias,&r,sizeof(r));
    CHECK(dreadzone_slot_word_00535A58(alias,&info)==1);memcpy(&captured,alias,sizeof(captured));
    CHECK(captured.storage==p&&captured.index==0&&get32(p,P+0x98)==payload);

    /* A late unmapped Q+0x10 refuses atomically, including the output receipt. */
    memset(p,0,RAM_BYTES);P=0x00300000U;Q=0x7FFFFFF0U;put32(p,entry,P);put32(p,P+0x24,Q);
    memcpy(before,p,RAM_BYTES);memset(&info,0x6C,sizeof(info));old=info;r=(DreadzoneSlotWordRequest){p,BASE,RAM_BYTES,0};
    status=dreadzone_slot_word_00535A58(&r,&info);CHECK(status<0);CHECK(same(p,before));CHECK(memcmp(&info,&old,sizeof(info))==0);

    /* Index and output-overlap guards preserve all caller-visible bytes. */
    memset(p,0,RAM_BYTES);memcpy(before,p,RAM_BYTES);memset(&info,0x4B,sizeof(info));old=info;r=(DreadzoneSlotWordRequest){p,BASE,RAM_BYTES,256};
    CHECK(dreadzone_slot_word_00535A58(&r,&info)<0);CHECK(same(p,before));CHECK(memcmp(&info,&old,sizeof(info))==0);
    r=(DreadzoneSlotWordRequest){p,BASE,RAM_BYTES,0};CHECK(dreadzone_slot_word_00535A58(&r,(DreadzoneSlotWordInfo *)(void *)(p+0x2000))<0);CHECK(same(p,before));
    printf("slot-word: %u checks, %u failures\n",checks,failures);
    free(before);free(allocation);return failures?1:0;
}
