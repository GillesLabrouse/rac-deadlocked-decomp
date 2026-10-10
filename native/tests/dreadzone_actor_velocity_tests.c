#include "dreadzone_actor_velocity.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static u8 record[0x31F0+2],baseline[0x31F0+2],extra[66],saved[66];
static u32 checks,failures;
static void check(int good) {++checks;if(!good) {++failures;printf("Failure %u\n",checks);}}
static void put(u8 *p,u32 o,f32 value) {memcpy(p+o,&value,4);}
static f32 get(const u8 *p,u32 o) {f32 value;memcpy(&value,p+o,4);return value;}
static u32 raw(const void *p) {u32 bits;memcpy(&bits,p,4);return bits;}
static DreadzoneActorInputContext fixture(void) {
    DreadzoneActorInputContext c;
    memset(record,0xA9,sizeof(record));record[0]=0xD3;record[sizeof(record)-1]=0x7A;
    memset(extra,0xB4,sizeof(extra));extra[0]=0xD3;extra[65]=0x7A;
    record[1+0x2659]=0;put(record+1,0x1780,1.f);put(record+1,0x1784,0.f);
    put(record+1,0xB8,0.f);put(record+1,0x2D4,0.f);
    c=(DreadzoneActorInputContext){record+1,0x368F60,0x31F0,0,0};
    return c;
}
static int near(f32 a,f32 b) {return fabsf(a-b)<=0.00001f;}
static void motion_checks(void) {
    DreadzoneActorInputContext c;DreadzoneActorInputMemory view;u32 i;
    c=fixture();view=(DreadzoneActorInputMemory){extra+1,0x210000,64,1};
    c.views=&view;c.view_count=1;
    dreadzone_actor_input_store32(extra+1,0x7FC12345U);
    dreadzone_actor_input_store32(extra+5,0x80000000U);
    dreadzone_actor_input_store32(extra+9,0xFFC23456U);
    dreadzone_actor_input_store32(extra+13,0xDEADBEEFU);
    memcpy(saved,extra,sizeof(extra));memcpy(baseline,record,sizeof(record));
    check(dreadzone_actor_vertical_projection_0068ABA8(&c,0x210010,0x210000,-0.f)==1);
    check(raw(extra+17)==0x7FC12345U && raw(extra+21)==0x80000000U);
    check(raw(extra+25)==0 && raw(extra+29)==0xDEADBEEFU);
    memcpy(saved+17,extra+17,16);
    check(memcmp(saved,extra,sizeof(extra))==0 && memcmp(baseline,record,sizeof(record))==0);
    check(dreadzone_actor_vertical_projection_0068ABA8(&c,0x210000,0x210000,-3.f)==1);
    check(raw(extra+1)==0x7FC12345U && get(extra+1,8)==-3.f && raw(extra+13)==0xDEADBEEFU);
    for(i=0;i<4;++i) {
        f32 reduction=i==0?2.f:i==1?-1.f:i==2?16.f:0.f;
        c=fixture();put(c.record,0x1A0,i==3?0.00048828125f:3.f);
        put(c.record,0x1A4,i==3?0.f:4.f);put(c.record,0x1A8,NAN);
        dreadzone_actor_input_store32(c.record+0x1AC,0xDEADBEEFU);
        put(c.record,0x130,1.f);put(c.record,0x134,-1.f);put(c.record,0x138,.5f);
        dreadzone_actor_input_store32(c.record+0x13C,0x7FC12345U);
        memcpy(baseline,record,sizeof(record));
        check(dreadzone_actor_accumulated_motion_006834C8(&c,reduction)==1);
        if(i<3) {
            check(near(get(c.record,0x1A0),i==0?1.8f:i==1?3.6f:0.f));
            check(near(get(c.record,0x1A4),i==0?2.4f:i==1?4.8f:0.f));
            check(raw(c.record+0x1A8)==0 && raw(c.record+0x1AC)==0xDEADBEEFU);
            check(near(get(c.record,0x130),1.f+get(c.record,0x1A0)));
            check(near(get(c.record,0x134),-1.f+get(c.record,0x1A4)));
            check(get(c.record,0x138)==.5f && raw(c.record+0x13C)==0x7FC12345U);
        } else {
            static const u8 zero[16]={0};
            check(memcmp(c.record+0x1A0,zero,16)==0);
            check(memcmp(c.record+0x130,baseline+1+0x130,16)==0);
        }
        memcpy(baseline+1+0x1A0,c.record+0x1A0,16);
        memcpy(baseline+1+0x130,c.record+0x130,16);
        check(memcmp(baseline,record,sizeof(record))==0);
    }
    for(i=0;i<4;++i) {
        c=fixture();put(c.record,0x1A0,3.f);put(c.record,0x1A4,4.f);
        put(c.record,0x130,1.f);put(c.record,0x134,0.f);put(c.record,0x138,0.f);
        if(i==0)c.record[0x2659]=1;
        if(i==1)put(c.record,0x1A4,NAN);
        if(i==2)put(c.record,0x130,17.f);
        memcpy(baseline,record,sizeof(record));
        check(dreadzone_actor_accumulated_motion_006834C8(&c,i==3?NAN:1.f)<0);
        check(memcmp(baseline,record,sizeof(record))==0);
    }
}
static void height_offset_checks(void) {
    DreadzoneActorInputContext c;DreadzoneActorInputMemory view;u32 i;
    for(i=0;i<6;++i) {
        u32 output=(i&1)?0x210000:0x210010;
        f32 z=i<2?7.f:i<4?0.f:-0.f;
        f32 offset=i<2?2.f:(i==3 || i==5)?-0.f:0.f;
        c=fixture();view=(DreadzoneActorInputMemory){extra+1,0x210000,64,1};
        c.views=&view;c.view_count=1;
        dreadzone_actor_input_store32(extra+1,0x7FC12345U);
        dreadzone_actor_input_store32(extra+5,0xDEADBEEFU);
        put(extra+1,8,z);dreadzone_actor_input_store32(extra+13,0xFFC12345U);
        dreadzone_actor_input_store32(extra+17,0x80000000U);
        dreadzone_actor_input_store32(extra+21,0x7FC98765U);
        put(extra+1,24,-12.f);dreadzone_actor_input_store32(extra+29,0x12345678U);
        memcpy(saved,extra,sizeof(extra));memcpy(baseline,record,sizeof(record));
        check(dreadzone_actor_height_offset_0068A178(&c,output,0x210000,offset)==1);
        {
            f32 expected=z-offset;u32 delta=1+output-0x210000;
            check(raw(extra+delta+8)==raw(&expected));
            memcpy(saved+delta+8,&expected,4);
        }
        check(memcmp(saved,extra,sizeof(extra))==0);
        check(memcmp(baseline,record,sizeof(record))==0);
    }
    for(i=0;i<7;++i) {
        u32 input=0x210000,output=0x210010;f32 offset=1.f;
        c=fixture();view=(DreadzoneActorInputMemory){extra+1,0x210000,64,1};
        c.views=&view;c.view_count=1;put(extra+1,8,7.f);
        switch(i) {
        case 0:view.writable=0;break;
        case 1:input++;break;
        case 2:output=0x210040;break;
        case 3:offset=NAN;break;
        case 4:put(extra+1,8,NAN);break;
        case 5:put(extra+1,8,17.f);break;
        case 6:c.record[0x2659]=1;break;
        }
        memcpy(saved,extra,sizeof(extra));memcpy(baseline,record,sizeof(record));
        check(dreadzone_actor_height_offset_0068A178(&c,output,input,offset)<0);
        check(memcmp(saved,extra,sizeof(extra))==0);
        check(memcmp(baseline,record,sizeof(record))==0);
    }
}
int main(void) {
    DreadzoneActorInputContext c;DreadzoneActorInputMemory view;
    VelocityVector v;u32 i,j;f32 result;clock_t start=clock();
    c=fixture();memcpy(baseline,record,sizeof(record));
    check(dreadzone_actor_speed_0067D350(&c,.125f,.25f)==1);
    check(get(c.record,0x1784)==.125f);put(baseline+1,0x1784,.125f);
    check(memcmp(record,baseline,sizeof(record))==0);
    check(dreadzone_actor_velocity_0067D3A0(&c,100000.f)==1);
    check(near(get(c.record,0x130),.125f));check(get(c.record,0x134)==0.f);
    check(get(c.record,0x138)==0.f && raw(c.record+0x13C)==0);
    check(record[0]==0xD3 && record[sizeof(record)-1]==0x7A);
    for(i=0;i<17;++i) for(j=0;j<17;++j) {
        f32 heading=((f32)i-8.f)*.375f,pitch=((f32)j-8.f)*.375f;
        c=fixture();put(c.record,0xB8,heading);put(c.record,0x2D4,pitch);
        put(c.record,0x1784,.5f);memcpy(baseline,record,sizeof(record));
        check(dreadzone_actor_velocity_0067D3A0(&c,100000.f)==1);
        check(near(get(c.record,0x130),.5f*cosf(heading)*cosf(pitch)));
        check(near(get(c.record,0x134),.5f*sinf(heading)*cosf(pitch)));
        check(near(get(c.record,0x138),.5f*sinf(pitch)));
        memcpy(baseline+1+0x130,c.record+0x130,16);
        check(memcmp(record,baseline,sizeof(record))==0);
    }
    c=fixture();view=(DreadzoneActorInputMemory){extra+1,0x210000,64,1};c.views=&view;c.view_count=1;
    put(extra+1,0,3.f);put(extra+1,4,4.f);put(extra+1,8,-7.f);
    dreadzone_actor_input_store32(extra+1+12,0x7FC12345U);
    memcpy(saved,extra,sizeof(extra));memcpy(baseline,record,sizeof(record));
    check(dreadzone_actor_planar_projection_0068AC58(&c,0x210010,0x210000,10.f)==1);
    check(get(extra+1,16)==6.f && get(extra+1,20)==8.f);
    check(get(extra+1,24)==-7.f && raw(extra+1+28)==0x7FC12345U);
    memcpy(saved+17,extra+17,16);check(memcmp(saved,extra,sizeof(extra))==0);
    check(memcmp(baseline,record,sizeof(record))==0);
    check(dreadzone_actor_planar_projection_0068AC58(&c,0x210000,0x210000,5.f)==1);
    check(get(extra+1,0)==3.f && get(extra+1,4)==4.f);
    for(i=0;i<4;++i) {
        c=fixture();view=(DreadzoneActorInputMemory){extra+1,0x210000,64,1};c.views=&view;c.view_count=1;
        dreadzone_actor_input_store32(extra+1,(i&1)?0x80000000U:0);
        dreadzone_actor_input_store32(extra+5,(i&2)?0x80000000U:0);
        put(extra+1,8,0.f);dreadzone_actor_input_store32(extra+13,0xFFC12345U);
        put(c.record,0x1784,1.f);
        check(dreadzone_actor_normalized_velocity_0067D480(&c,0x210000)==1);
        check(raw(c.record+0x130)==raw(extra+1) && raw(c.record+0x134)==raw(extra+5));
        check(raw(c.record+0x13C)==0xFFC12345U);
        check(dreadzone_actor_planar_projection_0068AC58(&c,0x210010,0x210000,1.f)==1);
        check(raw(extra+17)==0 && raw(extra+21)==0);
    }
    c=fixture();put(c.record,0x1784,2.f);put(c.record,0x130,0.f);
    put(c.record,0x134,3.f);put(c.record,0x138,4.f);put(c.record,0x13C,-0.f);
    check(dreadzone_actor_normalized_velocity_0067D480(&c,c.record_guest+0x130)==1);
    check(near(get(c.record,0x134),1.2f) && near(get(c.record,0x138),1.6f));
    check(raw(c.record+0x13C)==0x80000000U);
    c=fixture();put(c.record,0x1784,2.f);
    put(c.record,0x1780,0.f);check(dreadzone_actor_speed_0067D350(&c,.125f,.25f)==1);
    check(get(c.record,0x1784)==1.75f);
    for(i=0;i<18;++i) {
        c=fixture();view=(DreadzoneActorInputMemory){extra+1,0x210000,64,1};c.views=&view;c.view_count=1;
        memset(extra+1,0,64);put(extra+1,0,1.f);
        switch(i) {
        case 0:c.record_guest=0xFFFFFFF0U;break;
        case 1:c.record=(u8 *)(UINTPTR_MAX-100U);break;
        case 2:c.record_guest++;break;
        case 3:c.view_count=65;break;
        case 4:view.storage=c.record+0x1800;break;
        case 5:view.guest_base=c.record_guest+0x1800;break;
        case 6:view.storage=(u8 *)(UINTPTR_MAX-3U);break;
        case 7:view.guest_base=0xFFFFFFF0U;break;
        case 8:view.writable=2;break;
        case 9:c.record[0x2659]=1;break;
        case 10:put(c.record,0x1784,NAN);break;
        case 11:put(c.record,0xB8,4.f);break;
        case 12:put(c.record,0x2D4,4.f);break;
        case 13:put(c.record,0x2D4,0.0000000000001f);break;
        case 14:put(c.record,0x1784,17.f);break;
        case 15:c.views=(DreadzoneActorInputMemory *)(UINTPTR_MAX-1U);break;
        case 16:c.views=(DreadzoneActorInputMemory *)c.record;break;
        case 17:view.storage=(u8 *)&c;break;
        }
        memcpy(baseline,record,sizeof(record));memcpy(saved,extra,sizeof(extra));
        check(dreadzone_actor_velocity_0067D3A0(&c,100000.f)<0);
        check(memcmp(baseline,record,sizeof(record))==0 && memcmp(saved,extra,sizeof(extra))==0);
    }
    c=fixture();view=(DreadzoneActorInputMemory){extra+1,0x210000,64,0};c.views=&view;c.view_count=1;
    memcpy(baseline,record,sizeof(record));memcpy(saved,extra,sizeof(extra));
    check(dreadzone_actor_planar_projection_0068AC58(&c,0x210000,0x210000,1.f)==-2);
    check(dreadzone_actor_normalized_velocity_0067D480(&c,0x210001)==-2);
    check(memcmp(baseline,record,sizeof(record))==0 && memcmp(saved,extra,sizeof(extra))==0);
    /* Refusal must be atomic even when a shadow has already been allocated. */
    for(i=0;i<8;++i) {
        u32 input=0x210000,output=0x210010;f32 requested=1.f;
        c=fixture();view=(DreadzoneActorInputMemory){extra+1,0x210000,64,1};
        c.views=&view;c.view_count=1;
        put(extra+1,0,3.f);put(extra+1,4,4.f);
        switch(i) {
        case 0:output++;break;
        case 1:output=0x210040;break;
        case 2:input=0xFFFFFFF0U;break;
        case 3:output=0xFFFFFFF0U;break;
        case 4:requested=NAN;break;
        case 5:requested=-1.f;break;
        case 6:put(extra+1,0,NAN);break;
        case 7:put(extra+1,4,0.0000000000001f);break;
        }
        memcpy(baseline,record,sizeof(record));memcpy(saved,extra,sizeof(extra));
        check(dreadzone_actor_planar_projection_0068AC58(&c,output,input,requested)<0);
        check(memcmp(baseline,record,sizeof(record))==0 && memcmp(saved,extra,sizeof(extra))==0);
    }
    for(i=0;i<2;++i) {
        c=fixture();view=(DreadzoneActorInputMemory){extra+1,0x210000,64,1};
        c.views=&view;c.view_count=1;memset(extra+1,0,16);
        put(c.record,0x1784,i?-1.f:-0.f);
        memcpy(baseline,record,sizeof(record));memcpy(saved,extra,sizeof(extra));
        check(dreadzone_actor_normalized_velocity_0067D480(&c,0x210000)==-4);
        check(memcmp(baseline,record,sizeof(record))==0 && memcmp(saved,extra,sizeof(extra))==0);
    }
    c=fixture();put(c.record,0x1784,.25f);memcpy(baseline,record,sizeof(record));
    check(dreadzone_actor_speed_0067D350(&c,-0.f,0.f)==1);
    check(memcmp(baseline,record,sizeof(record))==0);
    /* A tiny normal output near a quarter turn remains within the domain. */
    put(c.record,0xB8,1.57079637050628662109375f);
    check(dreadzone_actor_velocity_0067D3A0(&c,100000.f)==1);
    check(fabsf(get(c.record,0x130))<0.00000095367431640625f);
    check(near(get(c.record,0x134),.25f));
    put((u8 *)&v,0,3.f);put((u8 *)&v,4,4.f);put((u8 *)&v,8,-7.f);
    dreadzone_actor_input_store32((u8 *)&v+12,0x7FC12345U);
    result=func_L01_0068A920(c.record,&v);check(result==5.f);
    result=func_L01_0068AB40(c.record,&v);check(result==-7.f);
    dreadzone_actor_input_store32((u8 *)&v+8,0xFFC12345U);
    result=func_L01_0068AB40(c.record,&v);check(raw(&result)==0xFFC12345U);
    motion_checks();
    height_offset_checks();
    printf("Velocity: %u checks, %u failures, %.3f seconds\n",checks,failures,
        (double)(clock()-start)/CLOCKS_PER_SEC);
    return failures?1:0;
}
