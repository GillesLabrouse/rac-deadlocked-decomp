#include "game_random.h"
#include <string.h>
#include <stdio.h>
#define CHECK(x) do { if (!(x)) {fprintf(stderr,"startup RNG check failed line%d\n",__LINE__); return 1;} } while(0)
static void put(u8 *p,u32 value)
{
    unsigned i;for(i=0;i<4;i++)p[i]=(u8)(value>>(i*8));
}
static u32 get(const u8 *p)
{
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}
int main(void)
{
    static const u32 seeds[]={0,1,0x80000000U,0xFFFFFFFFU};
    static const u32 next[]={2531011U,5890752U,2150014659U,4294138566U};
    static const s32 returns[]={2531011,5890752,2531011,2146654918};
    union {u32 alignment;u8 bytes[32];} backing;
    u8 *ram=backing.bytes,before[32];DeadlockedRandomState view={ram+7,4};
    s32 output;unsigned i,j;
    for(i=0;i<4;i++) {
        memset(ram,0xA5,sizeof(backing.bytes));put(ram+7,seeds[i]);memcpy(before,ram,sizeof(before));
        CHECK(deadlocked_random_001633D0(&view,&output)==0);
        CHECK(output==returns[i]&&get(ram+7)==next[i]);
        for(j=0;j<sizeof(before);j++)if(j<7||j>=11)CHECK(ram[j]==before[j]);
    }
    memcpy(before,ram,sizeof(before));output=0x12345678;view.bytes=3;
    CHECK(deadlocked_random_001633D0(&view,&output)==-1);
    view.bytes=4;
    CHECK(deadlocked_random_001633D0(&view,(s32 *)(ram+8))==-1);
    CHECK(deadlocked_random_001633D0(NULL,&output)==-1);
    CHECK(output==0x12345678&&!memcmp(before,ram,sizeof(before)));
    puts("PASS original startup RNG vectors, 31-bit output, unaligned seed and atomic refusals");
    return 0;
}
