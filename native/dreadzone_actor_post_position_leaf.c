#include "dreadzone_actor_post_position_leaf.h"
#include <math.h>
#include <float.h>
#if FLT_RADIX!=2 || FLT_MANT_DIG!=24 || FLT_MAX_EXP!=128
#error Post-position leaf requires binary32 floats
#endif
typedef char post_position_float_size[sizeof(float)==4 ? 1 : -1];
#include <fenv.h>
#include <stddef.h>
#include <string.h>

static int span(const void *p, size_t n) {
    return p && n && (uintptr_t)p <= UINTPTR_MAX-n;
}
static int overlap(const void *a,size_t an,const void *b,size_t bn) {
    return (uintptr_t)a<(uintptr_t)b+bn && (uintptr_t)b<(uintptr_t)a+an;
}
static int read_word(const DreadzonePostPositionLeafRequest *r,uint32_t address,uint32_t n,uint32_t *v) {
    uint32_t offset;
    if((address&(n-1u)) || address<r->guest_base)return 0;
    offset=address-r->guest_base;
    if(offset>r->storage_bytes || n>r->storage_bytes-offset)return 0;
    *v=0;memcpy(v,r->storage+offset,n);return 1;
}
static int number(uint32_t bits,float *v) {
    uint32_t m=bits&0x7FFFFFFFu;
    if(m && (m<0x00800000u || m>=0x7F800000u))return 0;
    memcpy(v,&bits,4);return 1;
}
static int product(float a,float b,float *out) {
    volatile float rounded=a*b;
    uint32_t bits;float value=rounded;
    memcpy(&bits,&value,4);return number(bits,out);
}
static int numeric_host(void) {
    volatile float minimum=0x1p-126f,half_minimum=minimum*.5f;
    float probe=half_minimum;uint32_t bits;memcpy(&bits,&probe,4);
    return fegetround()==FE_TONEAREST && bits==0x00400000u;
}
static int read_number(const DreadzonePostPositionLeafRequest *r,uint32_t address,float *out) {
    uint32_t bits;return read_word(r,address,4,&bits) && number(bits,out);
}
static int sum(float a,float b,float *out) {
    volatile float value=a+b;float rounded=value;uint32_t bits;
    memcpy(&bits,&rounded,4);return number(bits,out);
}
static int difference(float a,float b,float *out) {
    volatile float value=a-b;float rounded=value;uint32_t bits;
    memcpy(&bits,&rounded,4);return number(bits,out);
}
static int length(const DreadzonePostPositionLeafRequest *r,uint32_t first,uint32_t second,int xy,float *out) {
    float squares[4],a,b,v,total;uint32_t i,bits;
    for(i=0;i<4;++i) {
        if(!read_number(r,first+i*4,&a))return 0;
        if(second && i<3) {
            if(!read_number(r,second+i*4,&b) || !difference(a,b,&a))return 0;
        }
        if(!product(a,a,&squares[i]))return 0;
    }
    if(!sum(squares[0],squares[1],&total))return 0;
    if(!xy && (!product(1.f,squares[2],&v) || !sum(total,v,&total)))return 0;
    v=sqrtf(total);memcpy(&bits,&v,4);
    if(!number(bits,&v))return 0;
    return sum(0.f,v,out); /* Real VADDQ.X after the genuine WAITQ. */
}
int dreadzone_actor_post_position_leaf_00686420(const DreadzonePostPositionLeafRequest *request,DreadzonePostPositionLeafInfo *info) {
    DreadzonePostPositionLeafRequest r;DreadzonePostPositionLeafInfo result={0,0};
    uint32_t flag,delay_bits,half,zero=0,mark=0,state;
    uint16_t five=5;float height,time,clock,threshold,previous,distance,speed;
    if(!span(request,sizeof(r)) || !span(info,sizeof(*info)) ||
       overlap(request,sizeof(r),info,sizeof(*info)))return -1;
    memcpy(&r,request,sizeof(r));
    if(!r.storage_bytes || r.storage_bytes>16u*1024u*1024u || !span(r.storage,r.storage_bytes) ||
       r.guest_base>=0x80000000u || r.storage_bytes>0x80000000u-r.guest_base ||
       (r.guest_base<0x70004000u && r.guest_base+r.storage_bytes>0x70000000u) ||
       overlap(info,sizeof(*info),r.storage,r.storage_bytes) || (r.record_guest&15u) ||
       r.record_guest<r.guest_base || r.record_guest-r.guest_base>r.storage_bytes ||
       0x31F0u>r.storage_bytes-(r.record_guest-r.guest_base))return -1;
    if(!read_word(&r,r.record_guest+0x226u,1,&flag))return -1;
    if(flag) {
        if(!read_number(&r,r.record_guest+0x2CCu,&height))return -2;
        if(r.gp_guest<0x7B8Bu || !read_word(&r,r.gp_guest-0x7B8Bu,4,&delay_bits))return -1;
        if(height>0.f) {
            if(!numeric_host() || !number(delay_bits,&time) ||
               !read_number(&r,r.record_guest+0x25C4u,&clock) ||
               !product(time,3.f,&threshold) || !product(threshold,clock,&threshold) ||
               !read_number(&r,r.record_guest+0x1B0u,&previous))return -2;
            if(previous<threshold) {
                if(!length(&r,r.record_guest+0x130u,0,0,&distance) ||
                   !read_number(&r,0x21DDF4u,&time) || !product(time,3.f,&threshold) ||
                   !read_number(&r,r.record_guest+0x25C4u,&clock) ||
                   !product(threshold,clock,&threshold))return -2;
                if(distance<threshold) {
                    if(!read_number(&r,r.record_guest+0x2E08u,&height))return -2;
                    if(height<.2f) {
                        if(!read_word(&r,r.record_guest+0x25D4u,4,&state))return -1;
                        mark=state==0u || state==2u || state==4u || state==10u || state==1u;
                    }
                }
            }
        }
    }
    if(!read_word(&r,r.record_guest+0x3CCu,2,&half))return -1;
    if(mark)half=5;
    if(!half) {
        result.stores=1;
        memcpy(r.storage+r.record_guest-r.guest_base+0x214u,&zero,4);
    } else {
        if(!numeric_host() || !length(&r,r.record_guest+0xA0u,r.record_guest+0x1E0u,1,&distance) ||
           !read_number(&r,r.record_guest+0x204u,&speed) || !product(speed,.9f,&threshold))return -2;
        if(distance<threshold)return -2; /* Real angle/integrator/C60 branch remains open. */
        memcpy(&result.original_low_word,&distance,4);
        if(mark) {memcpy(r.storage+r.record_guest-r.guest_base+0x3CCu,&five,2);result.stores=1;}
    }
    memcpy(info,&result,sizeof(result));return 1;
}
