#include "dreadzone_actor_post_position_leaf.h"
#include "dreadzone_angle_pair.h"
#include "dreadzone_input.h"
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

/* Reads after the real optional SH at 68654C see that pending store. */
static int after_mark(const DreadzonePostPositionLeafRequest *r,uint32_t address,uint32_t mark,uint32_t *out) {
    uint32_t i,at=r->record_guest+0x3CCu;unsigned char raw[4];
    if(!read_word(r,address,4,out))return 0;
    memcpy(raw,out,4);
    if(mark)for(i=0;i<4;++i) {
        if(address+i==at)raw[i]=5;
        if(address+i==at+1u)raw[i]=0;
    }
    memcpy(out,raw,4);return 1;
}
static int marked_number(const DreadzonePostPositionLeafRequest *r,uint32_t address,uint32_t mark,float *out) {
    uint32_t bits;return after_mark(r,address,mark,&bits) && number(bits,out);
}
static float minimum(float a,float b) {return a<b || (a==b && signbit(a)) ? a:b;}
static float maximum(float a,float b) {return a>b || (a==b && !signbit(a)) ? a:b;}
static int heading(const DreadzonePostPositionLeafRequest *r,float x,float y,uint32_t mark,float *out) {
    static const float coefficients[8]={
        .999999344348907470703125f,-.33329856395721435546875f,
        .1994653642177581787109375f,-.13908533751964569091796875f,
        .096420042216777801513671875f,-.05590988695621490478515625f,
        .0218612290918827056884765625f,-.0040540578775107860565185546875f};
    static const float quadrants[16]={1.f,0.f,-1.f,1.57079637050628662109375f,
        -1.f,0.f,1.f,-1.57079637050628662109375f,
        -1.f,3.1415927410125732421875f,1.f,1.57079637050628662109375f,
        1.f,-3.1415927410125732421875f,-1.f,-1.57079637050628662109375f};
    float a=fabsf(x),b=fabsf(y),total,delta,ratio,value;uint32_t i,raw,expected;
    if(a>0x1p30f || b>0x1p30f)return 0;
    if(a==0.f && b==0.f){*out=func_L01_0055D708(x,y);return 1;}
    for(i=0;i<8;++i) {
        memcpy(&expected,coefficients+i,4);
        if(!after_mark(r,0x3B9540u+i*4u,mark,&raw) || raw!=expected)return 0;
    }
    for(i=0;i<16;++i) {
        memcpy(&expected,quadrants+i,4);
        if(!after_mark(r,0x3B9580u+i*4u,mark,&raw) || raw!=expected)return 0;
    }
    if(!sum(a,b,&total) || !difference(minimum(a,b),maximum(a,b),&delta))return 0;
    {volatile float divided=delta/total;value=divided;memcpy(&raw,&value,4);}
    if(!number(raw,&ratio) || (ratio!=0.f && fabsf(ratio)<.015625f))return 0;
    value=func_L01_0055D708(x,y);memcpy(&raw,&value,4);return number(raw,out);
}
/* The actual 594A88 ordered clamps/stores, followed by 594D28 settle.
 * All writes target a private scalar until the complete leaf succeeds. */
static int scalar(float prior,float value,float force,float damping,float limit,float *rate,uint32_t *stores) {
    float target,bound,current,push,drag,step,updated,left,tolerance;
    if(!difference(0.f,value,&target))return 0;
    bound=fabsf(target);current=maximum(minimum(prior,bound),-bound);
    if(!product(force,target,&push) || !product(damping,current,&drag) ||
       !difference(push,drag,&step) || !sum(current,step,&updated))return 0;
    *stores=2;
    if(limit>0.f){updated=maximum(minimum(updated,limit),-limit);++*stores;}
    updated=maximum(minimum(updated,bound),-bound);
    if(!sum(value,updated,&current) || !difference(0.f,current,&left) ||
       !product(limit,.01f,&tolerance))return 0;
    if(fabsf(left)<tolerance){updated=0.f;++*stores;}
    *rate=updated;return 1;
}
static int active(const DreadzonePostPositionLeafRequest *r,float distance,float threshold,float speed,uint32_t mark,
                  float *rate,uint32_t quad[4],uint32_t *stores) {
    float ax,bx,ay,by,x,y,angle,near,clock,time,force,damping,limit,value,prior,pair[2];
    uint32_t raw;
    if(!marked_number(r,r->record_guest+0xA0u,mark,&ax) ||
       !marked_number(r,r->record_guest+0x1E0u,mark,&bx) ||
       !marked_number(r,r->record_guest+0xA4u,mark,&ay) ||
       !marked_number(r,r->record_guest+0x1E4u,mark,&by) ||
       !difference(ax,bx,&x) || !difference(ay,by,&y) || !heading(r,x,y,mark,&angle) ||
       !product(speed,.05f,&near))return 0;
    /* BC1TL takes or annuls its B8 load after the angle callee really ran. */
    if(distance<near && !marked_number(r,r->record_guest+0xB8u,mark,&angle))return 0;
    if(!marked_number(r,0x21DDECu,mark,&time) || !difference(distance,threshold,&value) ||
       !product(time,.03f,&force) || !product(time,.3f,&damping) ||
       !marked_number(r,0x21DDF4u,mark,&clock) || !product(clock,2.8f,&limit) ||
       !marked_number(r,r->record_guest+0x25C4u,mark,&clock) ||
       !product(force,clock,&force) || !product(damping,clock,&damping) || !product(limit,clock,&limit) ||
       !marked_number(r,r->record_guest+0x214u,mark,&prior) ||
       !scalar(prior,value,force,damping,limit,rate,stores) || dreadzone_angle_pair(angle,pair)<0)return 0;
    if(!product(pair[1],*rate,&x) || !product(pair[0],*rate,&y))return 0;
    memcpy(quad,&x,4);memcpy(quad+1,&y,4);quad[2]=0;
    /* VMOVE.XYZ retains the loaded destination W verbatim, even a NaN. */
    if(!after_mark(r,r->record_guest+0x14Cu,mark,&raw))return 0;
    quad[3]=raw;++*stores;return 1;
}

int dreadzone_actor_post_position_leaf_00686420(const DreadzonePostPositionLeafRequest *request,DreadzonePostPositionLeafInfo *info) {
    DreadzonePostPositionLeafRequest r;DreadzonePostPositionLeafInfo result={0,0};
    uint32_t flag,delay_bits,half,zero=0,mark=0,state;
    uint16_t five=5;float height,time,clock,threshold,previous,distance,speed,rate=0.f;
    uint32_t quad[4]={0,0,0,0},active_return=0;
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
        if(distance<threshold) {
            if(!active(&r,distance,threshold,speed,mark,&rate,quad,&result.stores))return -2;
            active_return=1;result.original_low_word=quad[0];
        } else memcpy(&result.original_low_word,&distance,4);
        if(mark) {memcpy(r.storage+r.record_guest-r.guest_base+0x3CCu,&five,2);++result.stores;}
        if(active_return) {
            memcpy(r.storage+r.record_guest-r.guest_base+0x214u,&rate,4);
            memcpy(r.storage+r.record_guest-r.guest_base+0x140u,quad,16);
        }
    }
    memcpy(info,&result,sizeof(result));return 1;
}
