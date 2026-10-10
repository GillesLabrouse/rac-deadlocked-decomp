#include "dreadzone_actor_direction.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static u8 record[0x31F0],before[0x31F0],input[0x5C0],body[0xF0],flag[4],globals_[12];
static DreadzoneActorInputMemory views[4];static DreadzoneActorInputContext context;
static unsigned checks,failures;
static void check(int good,const char *m) {++checks;if(!good && failures++<12) printf("FAIL: %s\n",m);}
static void put(u8 *p,u32 n,u32 v) {dreadzone_actor_input_store32(p+n,v);}
static void fput(u8 *p,u32 n,float f) {dreadzone_direction_store_float(p+n,f);}
static float get(u32 n) {return dreadzone_direction_float(record+n);}
static void reset(void) {
    memset(record,0,sizeof(record));memset(input,0,sizeof(input));memset(body,0,sizeof(body));memset(flag,0,4);memset(globals_,0,12);
    views[0]=(DreadzoneActorInputMemory){input,0x1EE600,sizeof(input),0};
    views[1]=(DreadzoneActorInputMemory){body,0x230000,sizeof(body),0};
    views[2]=(DreadzoneActorInputMemory){flag,0x21E878,sizeof(flag),0};
    views[3]=(DreadzoneActorInputMemory){globals_,0x21DDEC,sizeof(globals_),0};
    context=(DreadzoneActorInputContext){record,0x368F60,sizeof(record),views,4};
    put(record,0x2EF0,0x230000);put(record,0x2F00,0x1EE600);
    fput(body,0xC0,1);fput(body,0xD4,1);fput(body,0xE8,1);
    fput(record,0x2CF0,1);fput(record,0x2D04,1);fput(record,0x2D18,1);
    fput(record,0x25C4,1);fput(globals_,0,0.5f);fput(globals_,8,0.005f);
}
static void refused(int status,const char *m) {
    memcpy(before,record,sizeof(record));check(dreadzone_actor_direction_0067C5F8(&context,0,1)==status,m);
    check(memcmp(record,before,sizeof(record))==0,"refusal preserves entire actor RAM");
}
static void direction_cases(void) {
    int x,y,mode;double length,angle,dx,dy;unsigned i;
    for(mode=0;mode<2;++mode) for(x=-16;x<=16;++x) for(y=-16;y<=16;++y) {
        reset();fput(record,0x120,(float)x/16);fput(record,0x124,(float)y/16);
        check(dreadzone_actor_direction_0067C5F8(&context,mode,1)==1,"complete actual direction path is approximate");
        dx=(double)x/16;dy=-(double)y/16;
        if(mode) {if(fabs(dy)>0.7 && fabs(dx)<0.12) dx=0;if(fabs(dx)>0.7 && fabs(dy)<0.12) dy=0;}
        length=sqrt(dx*dx+dy*dy);if(length>1) {dx/=length;dy/=length;length=1;}
        if(dx==0 && dy==0) {
            check(get(0x1780)==0 && get(0x17A0)==0,"neutral path copies actual facing and clears magnitude");
            check(get(0x1790)==1 && get(0x1794)==0,"neutral path uses actual body basis column");
        } else {
            angle=atan2(-dx,-dy);
            check(fabs(get(0x17A0)-angle)<0.000004,"direction angle follows independent atan2");
            check(fabs(get(0x1780)-length)<0.000001,"actual capped input length is retained");
            length=sqrt(dx*dx+dy*dy);
            check(fabs(get(0x1790)+dy/length)<0.000001 && fabs(get(0x1794)+dx/length)<0.000001,
                  "actual matrix projection and normalization produce the direction vector");
        }
        check(get(0x1798)==0 && get(0x179C)==0,"projection clears z and retains real w");
    }
    reset();put(record,0x2EF0,context.record_guest+0x16E0);fput(record,0xB8,0.5f);
    for(i=0;i<12;++i) fput(record,0x17A0+i*4,0);fput(record,0x17A0,9);
    check(dreadzone_actor_direction_0067C5F8(&context,0,1)==1,"body matrix aliases actor destination fields");
    check(get(0x1790)==0.5f && get(0x17A0)==0.5f,"neutral matrix reads observe prior actual facing store");
    reset();put(record,0x1AE0,1);put(input,0x190,2);put(input,0x1E0+29*4,0x28);
    put(record,0x2314,2);put(record,0x2318,2);fput(record,0x120,0.5f);fput(record,0x2E08,0.5f);
    check(dreadzone_actor_direction_0067C5F8(&context,0,1)==1 && record[0x5F7]==1,
          "real history/getter calls reach genuine classification stores");
}
static void rotation_cases(void) {
    int mode,i;float a,b,error,previous;
    for(mode=-1;mode<=2;++mode) for(i=-16;i<=16;++i) {
        a=(float)i*0.125f;b=0.125f;previous=a;
        error=func_L01_005952B8(&a,&b,mode,0.75f,0.2f,0.1f,0.3f);
        check(isfinite(a) && isfinite(b) && isfinite(error),"actual scalar rotation callees remain finite");
        check(fabsf(b)<=0.3f && fabsf(a-previous)<=6.6f,"actual speed and one-wrap bounds");
        reset();fput(record,0xB8,previous);fput(record,0x17A4,0.125f);fput(record,0x17A0,0.75f);
        check(dreadzone_actor_turn_0067CFD0(&context,mode,0.2f,0.1f,0.3f)==1,"complete actual turn gateway succeeds");
        check(get(0xB8)==a && get(0x17A4)==b && get(0x17A8)==error,"gateway uses real rotation outputs including remaining error");
    }
    reset();fput(record,0x17A0,0.5f);put(record,0x1AE0,1);record[0x265A]=1;
    check(dreadzone_actor_turn_0067CFD0(&context,0,0.01f,0.1f,0.01f)==1 && get(0x17A4)>0.01f,
          "real global rate boost reaches the actual scalar controller");
    a=0.5f;
    error=func_L01_005952B8(&a,&a,0,1.f,0.2f,0.1f,0.3f);
    check(isfinite(error) && fabsf(a-0.6f)<0.000001,"angle/speed alias retains actual ordered reloads");
}
static void boundaries(void) {
    reset();put(flag,0,1);refused(-3,"nonzero real constructor flag refuses");
    reset();record[0x2659]=2;refused(-3,"unrecovered orientation dependency refuses");
    reset();views[1].guest_base=0x240000;refused(-2,"neutral path requires actual body matrix");
    reset();fput(record,0x120,0.5f);put(record,0x2CF0,0x7FC00000);refused(-4,"late exceptional matrix refuses atomically");
    reset();fput(record,0x120,0.5f);put(record,0x1AE0,1);put(input,0x190,2);views[0].bytes=0x194;
    refused(-2,"late history read discards preceding angle store");
    reset();views[0].storage=(u8 *)(UINTPTR_MAX-1);refused(-1,"wrapping host backing refuses before access");
    reset();views[0].guest_base=0xFFFFFF00;refused(-1,"wrapping guest backing refuses");
    reset();context.record_guest++;refused(-1,"unaligned original VU record refuses");
    reset();context.views=(DreadzoneActorInputMemory *)(record+0x100);
    refused(-1,"descriptor metadata inside writable backing refuses");
    reset();context.views=(DreadzoneActorInputMemory *)(UINTPTR_MAX-8);
    refused(-1,"wrapping descriptor array refuses before access");
    reset();context.record=(u8 *)(UINTPTR_MAX-8);
    refused(-1,"wrapping actor host range refuses before access");
    reset();context.record_guest=0xFFFFF000;
    refused(-1,"wrapping complete actor guest span refuses");
    reset();fput(record,0x120,0.000000476837158203125f);
    refused(-4,"finite input below justified lower bound refuses");
    reset();put(record,0x120,1);
    refused(-4,"actual subnormal axis refuses");
    reset();fput(record,0x120,0.5f);fput(record,0x2CF0,17);
    refused(-4,"finite matrix outside justified range refuses");
    reset();fput(record,0x120,0);fput(record,0x124,-0.5f);put(record,0x5F4,1);put(record,0x2E08,0x7FC00000);
    refused(-4,"actual classification3 exceptional magnitude read refuses atomically");
}
int main(void) {
    direction_cases();rotation_cases();boundaries();
    printf("%u actor direction/rotation checks, %u failures\n",checks,failures);return failures?1:0;
}
