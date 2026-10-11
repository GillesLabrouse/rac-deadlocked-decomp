#include "dreadzone_player_volume_internal.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
static uint32_t r32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static float rf(const uint8_t *p){float f;memcpy(&f,p,4);return f;}
static void wf(uint8_t *p,float f){memcpy(p,&f,4);}
static int valid(float f){return isfinite(f)&&(f==0.0f||fabsf(f)>=FLT_MIN);}
static int product(float a,float b,float *v){volatile float x=a*b;*v=x;return valid(a)&&valid(b)&&valid(x);}
static int sum(float a,float b,float *v){volatile float x=a+b;*v=x;return valid(a)&&valid(b)&&valid(x);}
static uint8_t *map(const DreadzonePlayerVolume *q,uint32_t a,uint32_t n){
    if(a<q->guest_base||(uint64_t)a+n>(uint64_t)q->guest_base+q->bytes)return NULL;
    return q->storage+a-q->guest_base;
}
int dreadzone_player_volume_apply(const DreadzonePlayerVolume *q){
    uint8_t *actor=map(q,q->actor,0x100),*header,*row;
    uint32_t address,i,j;int32_t extent,count;float matrix[12],delta[4],local[4],term,acc,factor,scale,radius,height;
    if(!actor)return -2;
    if((q->actor&15)||q->index>127)return -3;
    address=r32(actor+0x98);header=map(q,address,8);if(!header)return -2;
    if(address&3)return -3;
    extent=(int32_t)r32(header+4);count=extent>=0?extent/32:-1-(int32_t)((~(uint32_t)extent)/32);
    if((int32_t)q->index>=count)return 0;
    if((address&15)||(uint64_t)address+16+q->index*32+32>UINT32_MAX)return -3;
    row=map(q,address+16+q->index*32,32);if(!row)return -2;
    memcpy(matrix,actor+0xC0,sizeof(matrix));
    scale=rf(actor+0x70);if(!valid(scale)||scale==0.0f)return -3;
    {volatile float x=1024.0f/scale;factor=x;}if(!valid(factor))return -3;
    for(j=0;j<3;j++){
        volatile float d=q->center[j]-rf(actor+0x10+j*4);delta[j]=d;
        if(!valid(q->center[j])||!valid(rf(actor+0x10+j*4))||!valid(d))return -3;
    }
    delta[3]=q->center[3];if(!valid(delta[3]))return -3;
    /* PEXT/PCPY transpose the three complete actor rows and canonical VF0.
     * Existing basis W lanes genuinely contribute to each XYZ dot product. */
    for(i=0;i<4;i++){
        float coefficient[4];
        if(i<3)memcpy(coefficient,matrix+i*4,sizeof(coefficient));
        else {coefficient[0]=coefficient[1]=coefficient[2]=0.0f;coefficient[3]=1.0f;}
        if(!product(coefficient[0],delta[0],&acc))return -3;
        for(j=1;j<4;j++)if(!product(coefficient[j],delta[j],&term)||!sum(acc,term,&acc))return -3;
        local[i]=acc;if(i<3&&!product(acc,factor,local+i))return -3;
    }
    if(!product(q->radius,factor,&radius)||!product(q->height,factor,&height))return -3;
    memcpy(row+0x10,local,16);wf(row+0x1C,radius);wf(row+4,height);return 1;
}
int dreadzone_player_volume_update(const DreadzonePlayerVolume *request){
    DreadzonePlayerVolume q;uint8_t *ram,*original;int status;
    if(!request||sizeof(q)>UINTPTR_MAX-(uintptr_t)request)return -1;q=*request;
    if(!q.storage||!q.bytes||q.bytes>0x1000000u||(q.guest_base&15)||q.guest_base>=0x80000000u||
       (uint64_t)q.guest_base+q.bytes>0x80000000u||q.bytes>UINTPTR_MAX-(uintptr_t)q.storage||
       ((uint64_t)q.guest_base<0x70004000u&&(uint64_t)q.guest_base+q.bytes>0x70000000u))return -1;
    original=q.storage;ram=malloc(q.bytes);if(!ram)return -4;memcpy(ram,original,q.bytes);q.storage=ram;
    status=dreadzone_player_volume_apply(&q);if(status==1)memcpy(original,ram,q.bytes);
    free(ram);return status;
}
