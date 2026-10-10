/* Bounded integration: real turn -> C60 velocity -> accumulation.
 * Numeric expected C60 lanes are independently checked by the actual resident
 * instruction-pair replay. This is not a complete actor frame or movement. */
#include "dreadzone_actor_direction.h"
#include "dreadzone_actor_velocity.h"
#include "dreadzone_angle_pair.h"
#include <stdio.h>
#include <string.h>
static u8 arena[0x31F2],saved[0x31F2],flag[4];
static unsigned checks,failures;
#define CHECK(x) do {++checks;if(!(x)){++failures;printf("FAIL line%d: %s\n",__LINE__,#x);}}while(0)
static f32 value(u32 b) {f32 f;memcpy(&f,&b,4);return f;}
static void put(u8 *p,u32 o,u32 b) {memcpy(p+o,&b,4);}
static u32 get(const u8 *p,u32 o) {u32 b;memcpy(&b,p+o,4);return b;}
int main(void) {
    const u32 targets[]={0,0x80000000U,0x2B800000U,0xAB800000U,0x33800000U,0xB3800000U};
    DreadzoneActorInputMemory view={flag,0x0021E878,4,1};
    DreadzoneActorInputContext ctx={arena+1,0x00368F60,0x31F0,&view,1};
    f32 pair[2];u32 i;
    for(i=0;i<sizeof(targets)/sizeof(targets[0]);++i) {
        memset(arena,0,sizeof(arena));arena[0]=0xD3;arena[sizeof(arena)-1]=0x7A;
        put(ctx.record,0x17A0,targets[i]);put(ctx.record,0x1784,0x3F800000);
        CHECK(dreadzone_actor_turn_0067CFD0(&ctx,0,.125f,.25f,.375f)==1);
        CHECK(get(ctx.record,0xB8)==targets[i]);
        CHECK(get(ctx.record,0x17A4)==0 && get(ctx.record,0x17A8)==0);
        CHECK(dreadzone_angle_pair(value(targets[i]),pair)==1);
        CHECK(dreadzone_actor_velocity_0067D3A0(&ctx,100000.f)==1);
        CHECK(get(ctx.record,0x130)==0x3F800000);
        CHECK(get(ctx.record,0x134)==get((u8 *)pair,0));
        CHECK(get(ctx.record,0x138)==0 && get(ctx.record,0x13C)==0);
        /* A small original contribution uses the real below-threshold clear
         * branch, while retaining the genuine velocity130 bit pattern. */
        put(ctx.record,0x1A0,0x33800000);put(ctx.record,0x1A4,0xB3800000);
        CHECK(dreadzone_actor_accumulated_motion_006834C8(&ctx,.125f)==1);
        CHECK(get(ctx.record,0x134)==get((u8 *)pair,0));
        CHECK(get(ctx.record,0x1A0)==0 && get(ctx.record,0x1A4)==0);
        CHECK(arena[0]==0xD3 && arena[sizeof(arena)-1]==0x7A);
    }
    put(ctx.record,0xB8,0x2B7FFFFF);memcpy(saved,arena,sizeof(arena));
    CHECK(dreadzone_actor_velocity_0067D3A0(&ctx,100000.f)==-4);
    CHECK(memcmp(saved,arena,sizeof(arena))==0);
    put(ctx.record,0xB8,0);put(ctx.record,0x130,0x2B7FFFFF);memcpy(saved,arena,sizeof(arena));
    CHECK(dreadzone_actor_accumulated_motion_006834C8(&ctx,.125f)==-4);
    CHECK(memcmp(saved,arena,sizeof(arena))==0);
    pair[0]=13.f;pair[1]=17.f;
    CHECK(dreadzone_angle_pair(value(0x2B7FFFFF),pair)==-2);
    CHECK(pair[0]==13.f && pair[1]==17.f);
    /* Component/angle expansion does not broaden original scalar contracts. */
    memcpy(saved,arena,sizeof(arena));
    CHECK(dreadzone_actor_accumulated_motion_006834C8(&ctx,value(0x33800000))==-4);
    CHECK(memcmp(saved,arena,sizeof(arena))==0);
    CHECK(dreadzone_actor_height_offset_0068A178(&ctx,ctx.record_guest+0x130,
        ctx.record_guest+0x1A0,value(0x33800000))==-4);
    CHECK(memcmp(saved,arena,sizeof(arena))==0);
    printf("Small-angle movement: %u checks, %u failures\n",checks,failures);
    return failures?1:0;
}
