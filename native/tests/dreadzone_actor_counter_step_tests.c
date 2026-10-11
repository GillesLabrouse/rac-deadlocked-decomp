#include "dreadzone_actor_counter_step.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
static u32 checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"actor counter check failed line%d\n",__LINE__);return 1;}}while(0)
int main(void){
    s32 counter,returned;u8 storage[12],before[12];
    const s32 inputs[]={INT_MIN,-1,0,1,2,255,INT_MAX};
    const s32 outputs[]={0,0,0,0,1,254,INT_MAX-1};
    const s32 returns[]={2,2,1,2,0,0,0};u32 i;
    for(i=0;i<sizeof(inputs)/sizeof(inputs[0]);++i){
        memset(storage,0xA5,sizeof(storage));memcpy(storage+4,inputs+i,4);memcpy(before,storage,sizeof(storage));
        returned=dreadzone_actor_counter_step_0055D148((s32 *)(storage+4));memcpy(&counter,storage+4,4);
        CHECK(returned==returns[i]&&counter==outputs[i]);
        CHECK(!memcmp(storage,before,4)&&!memcmp(storage+8,before+8,4));
    }
    counter=4;
    CHECK(dreadzone_actor_counter_step_0055D148(&counter)==0&&counter==3);
    CHECK(dreadzone_actor_counter_step_0055D148(&counter)==0&&counter==2);
    CHECK(dreadzone_actor_counter_step_0055D148(&counter)==0&&counter==1);
    CHECK(dreadzone_actor_counter_step_0055D148(&counter)==2&&counter==0);
    CHECK(dreadzone_actor_counter_step_0055D148(&counter)==1&&counter==0);
    CHECK(dreadzone_actor_counter_step_0055D148(NULL)==-1);
    printf("actor counter checks: %u\n",checks);return 0;
}
