#include "dreadzone_actor_orientation_ram.h"
#include "dreadzone_actor_direction.h"
#include "dreadzone_angle_pair.h"
#include "dreadzone_input.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
typedef struct Run {DreadzoneActorOrientationRAMRequest r;DreadzoneActorOrientationRAMInfo info;int error;} Run;
static u8 *at(Run *r,u32 a,u32 n,u32 align){
    if(a%align||a<r->r.guest_base||n>r->r.storage_bytes||a-r->r.guest_base>r->r.storage_bytes-n){r->error=-1;return NULL;}
    return r->r.storage+a-r->r.guest_base;
}
static u32 word(Run *r,u32 a,u32 n){u32 v=0;u8 *p=at(r,a,n,n);if(p)memcpy(&v,p,n);return v;}
static s32 integer(Run *r,u32 a){u32 w=word(r,a,4);s32 v;memcpy(&v,&w,4);return v;}
static float check(Run *r,float x){
    u32 w;memcpy(&w,&x,4);w&=0x7FFFFFFFU;
    if(w&&(w<0x00800000U||w>=0x7F800000U))r->error=-3;return x;
}
static float value(Run *r,u32 a,float maximum){
    u32 w=word(r,a,4);float v;memcpy(&v,&w,4);check(r,v);
    if(fabsf(v)>maximum||(v!=0.f&&fabsf(v)<0x1p-40f))r->error=-3;return v;
}
static void put(Run *r,u32 a,float v){u8 *p=at(r,a,4,4);check(r,v);if(p)memcpy(p,&v,4);}
static float add(Run *r,float a,float b){volatile float v=a+b;return check(r,v);}
static float sub(Run *r,float a,float b){volatile float v=a-b;return check(r,v);}
static float mul(Run *r,float a,float b){volatile float v=a*b;return check(r,v);}
static float root(Run *r,float a){volatile float v=sqrtf(a);return check(r,v);}
static int pair(Run *r,float a,float p[2]){
    if(dreadzone_angle_pair(a,p)<0){r->error=-3;return 0;}return 1;
}
static float angle(Run *r,float x,float y){
    static const float coefficients[8]={0.999999344348907470703125f,-0.33329856395721435546875f,
        0.1994653642177581787109375f,-0.13908533751964569091796875f,
        0.096420042216777801513671875f,-0.05590988695621490478515625f,
        0.0218612290918827056884765625f,-0.0040540578775107860565185546875f};
    static const float quadrants[16]={1,0,-1,1.57079637050628662109375f,
        -1,0,1,-1.57079637050628662109375f,-1,3.1415927410125732421875f,1,1.57079637050628662109375f,
        1,-3.1415927410125732421875f,-1,-1.57079637050628662109375f};
    u8 *p;float a=fabsf(x),b=fabsf(y),ratio;
    if(a==0.f&&b==0.f)return func_L01_0055D708(x,y);
    p=at(r,0x3B9540,sizeof(coefficients),16);if(!p||memcmp(p,coefficients,sizeof(coefficients)))r->error=-2;
    p=at(r,0x3B9580,sizeof(quadrants),16);if(!p||memcmp(p,quadrants,sizeof(quadrants)))r->error=-2;
    ratio=(a<b?a-b:b-a)/(a+b);
    if(ratio!=0.f&&fabsf(ratio)<.015625f)r->error=-3;
    if(r->error)return 0;return check(r,func_L01_0055D708(x,y));
}
static void matrix(Run *r,const float a[3],float m[4][4]){
    float p[3][2],sx,cx,sy,cy,sz,cz,t,u;
    u32 i;for(i=0;i<3;i++)if(!pair(r,a[i],p[i]))return;
    sx=p[0][0];cx=p[0][1];sy=p[1][0];cy=p[1][1];sz=p[2][0];cz=p[2][1];
    memset(m,0,64);m[3][3]=1;
    t=mul(r,sx,sy);u=mul(r,cx,sy);
    m[1][0]=sub(r,mul(r,cz,t),mul(r,cx,sz));
    m[1][1]=add(r,mul(r,cx,cz),mul(r,t,sz));m[1][2]=mul(r,cy,sx);
    m[2][0]=add(r,mul(r,u,cz),mul(r,sx,sz));
    m[2][1]=add(r,mul(r,-cz,sx),mul(r,u,sz));m[2][2]=mul(r,cx,cy);
    m[0][0]=mul(r,cy,cz);m[0][1]=mul(r,cy,sz);m[0][2]=-sy;
}
static void extract(Run *r,float m[4][4]){
    float pitch,yaw,p[2],x,y,z,roll,t,u;u32 b=r->r.record_guest;
    /* Genuine VU initial squared quad also consumes its zero W. */
    mul(r,m[0][2],m[0][2]);mul(r,m[0][3],m[0][3]);
    pitch=angle(r,root(r,add(r,mul(r,m[0][0],m[0][0]),mul(r,m[0][1],m[0][1]))),-m[0][2]);
    put(r,b+0xB4,pitch);yaw=angle(r,m[0][0],m[0][1]);put(r,b+0xB8,yaw);
    x=m[1][0];y=m[1][1];z=m[1][2];
    if(fabsf(yaw)>.00001f){
        if(!pair(r,-yaw,p))return;
        t=mul(r,p[0],x);u=mul(r,p[1],y);y=add(r,t,u);
        x=sub(r,mul(r,p[1],m[1][0]),mul(r,p[0],m[1][1]));
    }
    if(fabsf(pitch)>.00001f){
        if(!pair(r,-pitch,p))return;
        t=mul(r,p[1],x);u=mul(r,p[0],z);
        z=add(r,mul(r,-p[0],x),mul(r,p[1],z));x=add(r,t,u);
    }
    x=root(r,add(r,mul(r,x,x),mul(r,y,y)));
    roll=angle(r,x,z);put(r,b+0xB0,roll);
    if(y<0.f)put(r,b+0xB0,sub(r,roll<0.f?-3.1415927410125732421875f:3.1415927410125732421875f,roll));
    r->info.extractions++;
}
static void compose(Run *r,float x,float y){
    float old[3],delta[3]={x,y,0},a[4][4],d[4][4],out[4][4],acc;u32 b=r->r.record_guest,i,j,k;
    if(x==0.f&&y==0.f){put(r,b+0xB8,func_L01_0055D828(value(r,b+0xB8,3.1415927410125732421875f),0));return;}
    for(i=0;i<3;i++)old[i]=value(r,b+0xB0+4*i,3.1415927410125732421875f);
    matrix(r,delta,d);matrix(r,old,a);if(r->error)return;
    for(j=0;j<4;j++)for(i=0;i<4;i++){
        acc=mul(r,a[0][i],d[j][0]);for(k=1;k<4;k++)acc=add(r,acc,mul(r,a[k][i],d[j][k]));out[j][i]=acc;
    }
    r->info.compositions++;extract(r,out);
}
static void update(Run *r){
    u32 b=r->r.record_guest,mode=word(r,b+0x2659,1),i,active,flag,frame;float local[4],boost,time,force,damping,maximum;
    s32 state,previous;u8 *p;
    if(word(r,b+0x3D6,2))mode=1;
    if(mode!=0){if(mode<3)r->error=-2;else r->info.skips++;return;}
    state=integer(r,b+0x25CC);previous=integer(r,b+0x25D4);
    if((u32)((u32)state-36U)<2U||previous==17||state==118||state==48||state==44||state==43){r->info.skips++;return;}
    active=word(r,b+0x266D,1);boost=0;word(r,r->r.gp_guest-0x7B93U,4);
    if(!active){
        flag=word(r,0x220649,1);frame=word(r,r->r.gp_guest-0x7BC7U,4);
        if(flag&&((word(r,b+0x2F10,4)&7)!=(frame&7))){r->info.skips++;return;}
    }
    if(r->error)return;
    p=at(r,b+0xB0,16,16);if(!p)return;memcpy(local,p,16);
    for(i=0;i<2&&!r->error;i++){
        boost=value(r,i||!active?0x21DDEC:r->r.gp_guest-0x7B93U,16);
        check(r,local[i]);if(fabsf(local[i])>3.1415927410125732421875f)r->error=-3;
        time=value(r,b+0x25C4,16);force=mul(r,mul(r,boost,.015f),time);
        damping=mul(r,mul(r,boost,.3f),time);maximum=mul(r,mul(r,value(r,0x21DDF4,16),5.23598766326904296875f),time);
        if(force<0||force>1||damping<0||damping>1||maximum<0||maximum>16){r->error=-3;return;}
        value(r,b+0xC0+4*i,6.283185482025146484375f);p=at(r,b+0xC0+4*i,4,4);
        if(!p||r->error)return;
        check(r,dreadzone_direction_rotation_bytes((u8 *)(local+i),p,0,0,force,damping,maximum));r->info.turns++;
    }
    if(!r->error)compose(r,value(r,b+0xC0,3.1415927410125732421875f),value(r,b+0xC4,3.1415927410125732421875f));
}
static int overlap(uintptr_t a,uintptr_t n,uintptr_t b,uintptr_t m){return a>=b?a-b<m:b-a<n;}
int dreadzone_actor_orientation_ram_00683310(const DreadzoneActorOrientationRAMRequest *request,DreadzoneActorOrientationRAMInfo *info){
    Run r;u8 *copy,*destination;uintptr_t a=(uintptr_t)request,b=(uintptr_t)info,c;
    if(!request||!info||a>UINTPTR_MAX-sizeof(*request)||b>UINTPTR_MAX-sizeof(*info)||overlap(a,sizeof(*request),b,sizeof(*info)))return -1;
    memset(&r,0,sizeof(r));memcpy(&r.r,request,sizeof(r.r));c=(uintptr_t)r.r.storage;
    if(!c||!r.r.storage_bytes||r.r.storage_bytes>16U*1024U*1024U||c>UINTPTR_MAX-r.r.storage_bytes||
       (u64)r.r.guest_base+r.r.storage_bytes>0x80000000ULL||
       (r.r.guest_base<0x70004000U&&(u64)r.r.guest_base+r.r.storage_bytes>0x70000000ULL)||
       overlap(b,sizeof(*info),c,r.r.storage_bytes)||!at(&r,r.r.record_guest,0x31F0,16))return -1;
    destination=r.r.storage;copy=(u8 *)malloc(r.r.storage_bytes);if(!copy)return -4;
    memcpy(copy,destination,r.r.storage_bytes);r.r.storage=copy;
    update(&r);
    if(!r.error){memcpy(destination,copy,r.r.storage_bytes);memcpy(info,&r.info,sizeof(*info));}
    free(copy);return r.error?r.error:1;
}
