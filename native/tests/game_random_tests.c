#include "game_random.h"
#include <stdio.h>
#include <string.h>
static unsigned checks,failures;
#define CHECK(x) do { ++checks; if(!(x)) { ++failures; if(failures<8) printf("line%d: %s\n",__LINE__,#x); } } while(0)
static void put(u8 *p,u32 value) { unsigned i;for(i=0;i<4;i++) p[i]=(u8)(value>>(i*8)); }
static u32 get(const u8 *p) { return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24); }
int main(void) {
    static const s32 sequence[]={89,2963,18174,14244,3697,7036};
    u8 bytes[32],before[32];
    DeadlockedRandomState view={bytes+7,4};
    s32 output;
    unsigned i,offset;
    memset(bytes,0xCD,sizeof(bytes));put(view.storage,1);
    for(i=0;i<6;i++) { CHECK(deadlocked_random_00163408(&view,&output)==0);CHECK(output==sequence[i]); }
    for(offset=0;offset<16;offset++) {
        memset(bytes,0xCD,sizeof(bytes));view.storage=bytes+offset;put(view.storage,1);
        memcpy(before,bytes,sizeof(bytes));
        CHECK(deadlocked_random_0058FF38(&view,180,300,&output)==0);CHECK(output==269);
        CHECK(get(view.storage)==5890752U);
        for(i=0;i<sizeof(bytes);i++) if(i<offset || i>=offset+4) CHECK(bytes[i]==before[i]);
        put(view.storage,1);CHECK(deadlocked_random_0058FF38(&view,5,4,&output)==0);CHECK(output==94);
        put(view.storage,1);CHECK(deadlocked_random_0058FF38(&view,5,3,&output)==0);CHECK(output==5);
        put(view.storage,1);CHECK(deadlocked_random_0058FF38(&view,(s32)0x80000000U,0x7FFFFFFF,&output)==0);
        CHECK((u32)output==0x80000059U);
    }
    view.storage=bytes;view.bytes=4;put(bytes,1);memcpy(before,bytes,sizeof(bytes));output=123;
    CHECK(deadlocked_random_00163408(&view,(s32 *)bytes)==-1);
    CHECK(deadlocked_random_0058FF38(&view,180,300,(s32 *)(bytes+1))==-1);
    view.bytes=3;CHECK(deadlocked_random_00163408(&view,&output)==-1);view.bytes=4;
    view.storage=(u8 *)(UINTPTR_MAX-1);CHECK(deadlocked_random_00163408(&view,&output)==-1);
    view.storage=bytes;CHECK(deadlocked_random_00163408(&view,(s32 *)(UINTPTR_MAX-1))==-1);
    CHECK(deadlocked_random_00163408((const DeadlockedRandomState *)(UINTPTR_MAX-1),&output)==-1);
    CHECK(deadlocked_random_00163408(0,&output)==-1);CHECK(deadlocked_random_00163408(&view,0)==-1);
    { union { u32 alignment;u8 data[8]; } unaligned;
      CHECK(deadlocked_random_00163408(&view,(s32 *)(unaligned.data+1))==-1); }
    CHECK(output==123);CHECK(!memcmp(bytes,before,sizeof(bytes)));
    /* Seed bytes overwrite their own readable descriptor pointer. */
    view.storage=(u8 *)&view;view.bytes=(u32)sizeof(view);
    { u32 seed=get((u8 *)&view);u64 widened=(u64)seed*3359741U+2531011U;
      CHECK(deadlocked_random_0058FF38(&view,180,300,&output)==0);
      CHECK(output==180+(s32)(((widened>>16)&32767U)%121U));
      CHECK(get((u8 *)&view)==(u32)widened); }
    printf("Game RNG: %u checks, %u failures. Genuine seed transition and signed/wrapped range semantics verified.\n",checks,failures);
    return failures?1:0;
}
