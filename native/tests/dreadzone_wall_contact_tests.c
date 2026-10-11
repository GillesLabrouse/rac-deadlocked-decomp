#include "dreadzone_wall_contact.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks,failures;
#define CHECK(x) do{checks++;if(!(x)){failures++;printf("FAIL line%d: %s\n",__LINE__,#x);}}while(0)
static void put(u8 *p,u32 v){memcpy(p,&v,4);}
static u32 get(const u8 *p){u32 v;memcpy(&v,p,4);return v;}
static void half(u8 *p,u16 v){memcpy(p,&v,2);}
static void real(u8 *p,float v){memcpy(p,&v,4);}
static float value(const u8 *p){float v;memcpy(&v,p,4);return v;}
static u32 vertex(s32 x,s32 y,s32 z){return ((u32)x&1023U)|(((u32)y&1023U)<<10)|(((u32)z&4095U)<<20);}
static void fixture(u8 *p,DreadzoneWallRequest *r){
    u32 i;u8 *tree=p+0x20000,*block=tree+128;
    memset(p,0xA5,0x30000);memset(p+0x1000,0,0x1C000);memset(r,0,sizeof(*r));
    r->storage=p;r->guest_base=0x400000;r->storage_bytes=0x30000;r->start_guest=0x400100;r->end_guest=0x400110;
    r->packet_guest=0x400200;r->grid_guest=0x401000;r->actor_pool_pointer_guest=0x400400;r->actor_pool_count=1;r->grid_block_count=3072;r->caller_flags=2;
    for(i=0;i<3;i++){r->incoming_vf25_27_w[i]=0xA0B0C000U+i;real(p+0x100+4*i,10.f);real(p+0x110+4*i,10.f);}
    real(p+0x108,9.f);real(p+0x118,11.f);put(p+0x10C,123);put(p+0x11C,456);
    put(p+0x200,0x420000);put(p+0x210,0);put(p+0x400,0x41E000);
    half(tree,2);half(tree+2,1);half(tree+4,8);half(tree+32,2);half(tree+34,1);put(tree+36,64);
    half(tree+64,2);half(tree+66,1);put(tree+68,0x8002);
    memset(block,0,32);block[0]=1;block[2]=3;put(block+4,vertex(-16,-16,-16));put(block+8,vertex(16,-16,-16));put(block+12,vertex(0,16,-16));block[16]=0;block[17]=1;block[18]=2;
}
static void actor(u8 *p){u32 i;half(p+0x1000,0);p[0x1002]=1;for(i=0;i<3;i++){real(p+0x1E000+i*4,10240.f);real(p+0x1E010+i*4,0.f);}real(p+0x1E00C,1024.f);put(p+0x1E098,0x41F000);put(p+0x1E09C,0xFFFFFFFFU);put(p+0x1E0A0,0);put(p+0x1F00C,2);put(p+0x1F004,1);}
static void unchanged(DreadzoneWallRequest *r,DreadzoneWallInfo *info,int expected){
    u8 *before=(u8 *)malloc(r->storage_bytes);DreadzoneWallInfo old;memcpy(before,r->storage,r->storage_bytes);memset(info,0x7C,sizeof(*info));old=*info;
    CHECK(dreadzone_wall_contact(r,info)==expected);CHECK(memcmp(before,r->storage,r->storage_bytes)==0);CHECK(memcmp(&old,info,sizeof(old))==0);free(before);
}
int main(void){
    u8 *allocation=(u8 *)malloc(0x30002),*p;DreadzoneWallRequest r;DreadzoneWallInfo info;u32 i;
    if(!allocation)return 1;p=allocation+1;allocation[0]=0xC8;allocation[0x30001]=0x9B;
    fixture(p,&r);CHECK(dreadzone_wall_contact(&r,&info)==1);
    CHECK(get(p+0x210)==1&&get(p+0x218)==0&&get(p+0x21C)==0x1000);
    CHECK(value(p+0x220)==10.f&&value(p+0x224)==10.f&&value(p+0x228)==9.75f);
    CHECK(get(p+0x230)==0xA5A5A5A5U&&get(p+0x10C)==123&&get(p+0x11C)==456);
    CHECK(get(p+0x22C)==r.incoming_vf25_27_w[0]&&get(p+0x24C)==r.incoming_vf25_27_w[1]&&get(p+0x25C)==r.incoming_vf25_27_w[2]);
    CHECK(get(p+0x26C)==0&&get(p+0x27C)==0&&info.q_exception_bits==0);
    fixture(p,&r);real(p+0x100,8.1f);real(p+0x110,8.1f);CHECK(dreadzone_wall_contact(&r,&info)==0&&get(p+0x210)==1&&get(p+0x21C)==0xA5A5A5A5U);
    fixture(p,&r);put(p+0x210,0xFFFFFFFFU);CHECK(dreadzone_wall_contact(&r,&info)==0&&get(p+0x210)==0xFFFFFFFFU);
    fixture(p,&r);real(p+0x100,-1.f);CHECK(dreadzone_wall_contact(&r,&info)==0&&get(p+0x210)==1);
    fixture(p,&r);put(p+0x210,0x7FFFFFFFU);unchanged(&r,&info,DREADZONE_WALL_UNSUPPORTED);
    fixture(p,&r);real(p+0x100,NAN);unchanged(&r,&info,DREADZONE_WALL_UNSUPPORTED);
    fixture(p,&r);r.caller_flags=0;unchanged(&r,&info,DREADZONE_WALL_UNSUPPORTED);
    fixture(p,&r);r.cache_guest=0x400800;unchanged(&r,&info,DREADZONE_WALL_UNSUPPORTED);
    fixture(p,&r);r.start_guest++;unchanged(&r,&info,DREADZONE_WALL_INVALID);
    fixture(p,&r);r.packet_guest=0x42FFF0;unchanged(&r,&info,DREADZONE_WALL_INVALID);
    /* Actual initial tree/leaf reads happen even if integer endpoints match. */
    fixture(p,&r);for(i=0;i<3;i++)real(p+0x110+4*i,value(p+0x100+4*i));
    put(p+0x200,0x430000);unchanged(&r,&info,DREADZONE_WALL_UNSUPPORTED);
    fixture(p,&r);for(i=0;i<3;i++)real(p+0x110+4*i,value(p+0x100+4*i));
    put(p+0x20000+68,0x01000002U);unchanged(&r,&info,DREADZONE_WALL_UNSUPPORTED);
    /* A fetched block is not decoded on this return: its unused vertex
     * count is deliberately malformed, but the source bytes are mapped. */
    fixture(p,&r);for(i=0;i<3;i++)real(p+0x110+4*i,value(p+0x100+4*i));
    p[0x20000+128+2]=255;CHECK(dreadzone_wall_contact(&r,&info)==0&&get(p+0x210)==1&&info.grid_records==1&&info.terrain_cells==0&&info.actor_candidates==0);
    /* Negative zero is the VMIN result in either operand order. */
    fixture(p,&r);for(i=0;i<3;i++)real(p+0x110+4*i,value(p+0x100+4*i));
    real(p+0x100,-0.f);real(p+0x110,0.f);put(p+0x200,0x430000);
    CHECK(dreadzone_wall_contact(&r,&info)==0&&get(p+0x210)==1&&info.grid_records==0);
    fixture(p,&r);for(i=0;i<3;i++)real(p+0x110+4*i,value(p+0x100+4*i));
    real(p+0x100,0.f);real(p+0x110,-0.f);put(p+0x200,0x430000);
    CHECK(dreadzone_wall_contact(&r,&info)==0&&get(p+0x210)==1&&info.grid_records==0);
    fixture(p,&r);actor(p);CHECK(dreadzone_wall_contact(&r,&info)==1&&get(p+0x1E0A0)==1&&info.actors_marked==1&&info.actors_filtered==1);
    fixture(p,&r);actor(p);put(p+0x1E09C,0);unchanged(&r,&info,DREADZONE_WALL_UNSUPPORTED);
    fixture(p,&r);actor(p);r.ignored_actor_guest=0x41E000;CHECK(dreadzone_wall_contact(&r,&info)==1&&info.actors_ignored==1&&get(p+0x1E0A0)==0);
    fixture(p,&r);actor(p);put(p+0x1E098,0x41E094);CHECK(dreadzone_wall_contact(&r,&info)==1&&get(p+0x1E0A0)==1&&info.actors_marked==1);
    /* Generation writes change the next header before that header is read.
     * The original only snapshots XY coordinates, not all cell headers. */
    fixture(p,&r);real(p+0x100,63.f);real(p+0x110,65.f);put(p+0x400,0x400F70);
    for(i=0;i<3;i++)real(p+0xF70+i*4,i?10240.f:65536.f);real(p+0xF7C,1024.f);
    for(i=0;i<3;i++)real(p+0xF80+i*4,0.f);put(p+0x1008,0x41F000);put(p+0x100C,0x10000);put(p+0x1010,0x10000);put(p+0x1F00C,0);half(p+0x5000,0);
    CHECK(dreadzone_wall_contact(&r,&info)==0&&get(p+0x1010)==1&&info.actor_candidates==1);
    CHECK(allocation[0]==0xC8&&allocation[0x30001]==0x9B);
    {volatile size_t widths[2]={sizeof(u64),sizeof(s64)};CHECK(widths[0]==8&&widths[1]==8);}
    fixture(p,&r);r.guest_base=0x7FFF0000U;unchanged(&r,&info,DREADZONE_WALL_INVALID);
    fixture(p,&r);r.guest_base=0x80000000U;unchanged(&r,&info,DREADZONE_WALL_INVALID);
    fixture(p,&r);r.guest_base=0xFFFF0000U;unchanged(&r,&info,DREADZONE_WALL_INVALID);
    CHECK(dreadzone_wall_contact(NULL,&info)==DREADZONE_WALL_INVALID);CHECK(dreadzone_wall_contact(&r,NULL)==DREADZONE_WALL_INVALID);
    free(allocation);printf("%u checks, %u failures\n",checks,failures);return failures?1:0;
}
