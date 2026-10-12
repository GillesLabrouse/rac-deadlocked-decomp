#include "dreadzone_actor_offset_probe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
enum { BASE=0x21D000,SIZE=0x1A0000,RECORD=0x368F60 };
#define CHECK(x) do {if(!(x)){fprintf(stderr,"Offset probe failure line %d\n",__LINE__);free(data);free(before);return 1;}} while(0)
static void word(u8 *p,u32 a,u32 v){memcpy(p+a-BASE,&v,4);}
static void half(u8 *p,u32 a,u16 v){memcpy(p+a-BASE,&v,2);}
static void real(u8 *p,u32 a,f32 v){memcpy(p+a-BASE,&v,4);}
static u32 read(u8 *p,u32 a){u32 v;memcpy(&v,p+a-BASE,4);return v;}
static void vector(u8 *p,u32 a,f32 x,f32 y,f32 z,f32 w){real(p,a,x);real(p,a+4,y);real(p,a+8,z);real(p,a+12,w);}
static u32 vertex(s32 x,s32 y,s32 z){return ((u32)x&1023U)|(((u32)y&1023U)<<10)|(((u32)z&4095U)<<20);}
int main(void) {
    u8 *data=(u8 *)calloc(SIZE,1),*before=(u8 *)malloc(SIZE);
    DreadzoneOffsetProbeRequest q={0};DreadzoneOffsetProbeInfo info,old;
    u32 i,target,answer;int status;
    if(!data||!before){free(data);free(before);return 1;}
    q.storage=data;q.guest_base=BASE;q.storage_bytes=SIZE;q.record_guest=RECORD;
    q.local_z=.7f;q.limit=4.f;
    for(i=0;i<5;++i)q.incoming_vf25_29_w[i]=0x3F000000U+i;
    /* Cached result must not read unreached mode, radius, local W or geometry. */
    data[RECORD+0x2659-BASE]=2;word(data,RECORD+0x204,0x7FC00000U);
    q.incoming_local_w[0]=0x7FC00000U;q.incoming_local_w[1]=1;
    for(target=0;target<4;++target) {
        real(data,RECORD+0x2F90,.7f);real(data,RECORD+0x2F94,5.f);real(data,RECORD+0x2F98,2.f);
        q.output_guest=target?RECORD+0x2F8C+4*target:0;
        memcpy(before,data,SIZE);
        CHECK(dreadzone_actor_offset_probe_006708B0(&q,&info)==1);
        CHECK(info.original_integer==1&&!info.cache_updates&&!info.wall_calls);
        CHECK(memcmp(info.outgoing_vf25_29_w,q.incoming_vf25_29_w,20)==0);
        if(q.output_guest)word(before,q.output_guest,0x40000000U);
        CHECK(memcmp(data,before,SIZE)==0);
    }
    real(data,RECORD+0x2F90,.7f);real(data,RECORD+0x2F94,5.f);real(data,RECORD+0x2F98,2000.f);
    q.output_guest=0xDEADBEEFU;memcpy(before,data,SIZE);
    CHECK(dreadzone_actor_offset_probe_006708B0(&q,&info)==1&&!info.original_integer);
    CHECK(memcmp(data,before,SIZE)==0);
    real(data,RECORD+0x2F98,2.f);memcpy(before,data,SIZE);old=info;
    CHECK(dreadzone_actor_offset_probe_006708B0(&q,&info)==-1);
    CHECK(memcmp(data,before,SIZE)==0&&memcmp(&info,&old,sizeof(info))==0);
    q.output_guest=0;q.local_z=NAN;
    CHECK(dreadzone_actor_offset_probe_006708B0(&q,&info)==-3);
    CHECK(memcmp(data,before,SIZE)==0&&memcmp(&info,&old,sizeof(info))==0);
    /* Constructed genuine terrain hit, including executed W square and alias. */
    memset(data,0,SIZE);q.local_z=.7f;q.actor_pool_count=65536;q.grid_block_count=3072;
    q.output_guest=RECORD+0x2F98;q.incoming_local_w[0]=0x40000000U;q.incoming_local_w[1]=0;
    vector(data,RECORD+0x20,0,0,1,0);vector(data,RECORD+0x30,0,1,0,0);
    vector(data,RECORD+0x40,0,0,0,0);vector(data,RECORD+0x50,0,0,0,1);
    vector(data,RECORD+0xA0,10,10,8,1);real(data,RECORD+0x204,.75f);
    real(data,RECORD+0x2F90,-100.f);real(data,RECORD+0x2F98,2000.f);
    word(data,0x242340,0x3A0000);word(data,0x222D14,0x2A0000);word(data,0x21DDB8,1);
    half(data,0x3A0000,2);half(data,0x3A0002,1);half(data,0x3A0004,8);
    half(data,0x3A0020,2);half(data,0x3A0022,1);word(data,0x3A0024,64);
    half(data,0x3A0040,2);half(data,0x3A0042,1);word(data,0x3A0044,0x8002);
    data[0x3A0080-BASE]=1;data[0x3A0082-BASE]=3;
    word(data,0x3A0084,vertex(-16,-16,-16));word(data,0x3A0088,vertex(16,-16,-16));word(data,0x3A008C,vertex(0,16,-16));
    data[0x3A0090-BASE]=0;data[0x3A0091-BASE]=1;data[0x3A0092-BASE]=2;
    memcpy(before,data,SIZE);status=dreadzone_actor_offset_probe_006708B0(&q,&info);
    CHECK(status==1&&info.original_integer==1&&info.cache_updates==1&&info.wall_calls==1);
    answer=read(data,RECORD+0x2F98);CHECK(answer==0x3F828F60U&&info.wall.write_words==23);
    CHECK(info.outgoing_vf25_29_w[3]==0&&info.outgoing_vf25_29_w[4]==0);
    for(i=0;i<3;++i)CHECK(info.outgoing_vf25_29_w[i]==q.incoming_vf25_29_w[i]);
    memcpy(data,before,SIZE);old=info;q.incoming_local_w[0]=0x7149F2CAU; /* Finite1e30; W square overflows. */
    CHECK(dreadzone_actor_offset_probe_006708B0(&q,&info)==-3);
    CHECK(memcmp(data,before,SIZE)==0&&memcmp(&info,&old,sizeof(info))==0);
    q.incoming_local_w[0]=0x40000000U;data[RECORD+0x2659-BASE]=1;memcpy(before,data,SIZE);
    CHECK(dreadzone_actor_offset_probe_006708B0(&q,&info)==-2);
    CHECK(memcmp(data,before,SIZE)==0&&memcmp(&info,&old,sizeof(info))==0);
    free(data);free(before);puts("Offset cache, terrain hit, output aliases and atomic refusals passed");return 0;
}
