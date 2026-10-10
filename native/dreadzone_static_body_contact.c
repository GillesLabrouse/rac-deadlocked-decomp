#include "dreadzone_static_body_contact.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct BodyWrite {u32 address,value;} BodyWrite;
typedef struct BodyPlan {
    DreadzoneStaticBodyRequest r;
    DreadzoneStaticBodyInfo info;
    BodyWrite *writes;
    u32 used,capacity;
    int failed;
} BodyPlan;
static u32 body_bits(float x) {u32 b;memcpy(&b,&x,4);return b;}
static float body_float(u32 b) {float x;memcpy(&x,&b,4);return x;}
static int body_valid(float x) {return isfinite(x)&&(x==0.0f||fabsf(x)>=FLT_MIN);}
static float body_checked(BodyPlan *p,float x) {
    if(!body_valid(x))p->failed=DREADZONE_STATIC_BODY_UNSUPPORTED;
    return x;
}
static float body_add(BodyPlan *p,float a,float b) {volatile float x=a+b;return body_checked(p,x);}
static float body_sub(BodyPlan *p,float a,float b) {volatile float x=a-b;return body_checked(p,x);}
static float body_mul(BodyPlan *p,float a,float b) {volatile float x=a*b;return body_checked(p,x);}
static float body_div(BodyPlan *p,float a,float b) {
    float q;
    if(b==0.0f) {
        u32 bits=0x7F7FFFFFU|((body_bits(a)^body_bits(b))&0x80000000U);
        p->info.q_exception_bits|=a==0.0f?0x10U:0x20U;q=body_float(bits);
    } else {volatile float v=a/b;q=body_checked(p,v);}
    p->info.q_bits=body_bits(q);return q;
}
static float body_sum(BodyPlan *p,const float x[3]) {
    float a=body_add(p,x[0],x[1]);return body_add(p,a,body_mul(p,1.f,x[2]));
}
static void body_difference(BodyPlan *p,const float a[3],const float b[3],float o[3]) {
    u32 i;for(i=0;i<3;i++)o[i]=body_sub(p,a[i],b[i]);
}
static void body_product(BodyPlan *p,const float a[3],const float b[3],float o[3]) {
    u32 i;for(i=0;i<3;i++)o[i]=body_mul(p,a[i],b[i]);
}
static void body_cross(BodyPlan *p,const float a[3],const float b[3],float o[3]) {
    u32 i;for(i=0;i<3;i++) {
        float x=body_mul(p,a[(i+1)%3],b[(i+2)%3]);
        float y=body_mul(p,b[(i+1)%3],a[(i+2)%3]);o[i]=body_sub(p,x,y);
    }
}
static int body_range(BodyPlan *p,u32 address,u32 bytes,u32 alignment) {
    if(!bytes||address%alignment||address<p->r.guest_base||
       address>0x7FFFFFFFU-(bytes-1)||address-p->r.guest_base>p->r.storage_bytes||
       bytes>p->r.storage_bytes-(address-p->r.guest_base)) {
        p->failed=DREADZONE_STATIC_BODY_INVALID;return 0;
    }
    return 1;
}
static u32 body_read(BodyPlan *p,u32 address,u32 bytes) {
    u32 out=0,i,j;
    if(!body_range(p,address,bytes,bytes))return 0;
    for(i=0;i<bytes;i++) {
        u8 byte=p->r.storage[address-p->r.guest_base+i];
        for(j=p->used;j>0;j--) {
            const BodyWrite *w=p->writes+j-1;
            if(address+i>=w->address&&address+i-w->address<4) {
                byte=(u8)(w->value>>(8*(address+i-w->address)));break;
            }
        }
        out|=(u32)byte<<(8*i);
    }
    return out;
}
static int body_address(BodyPlan *p,u32 a,u32 b,u32 *out) {
    if(a>0x7FFFFFFFU-b) {p->failed=DREADZONE_STATIC_BODY_INVALID;return 0;}
    *out=a+b;return 1;
}
static void body_write(BodyPlan *p,u32 address,u32 value) {
    BodyWrite *grow;
    if(p->failed||!body_range(p,address,4,4))return;
    if(p->used==p->capacity) {
        u32 capacity=p->capacity?p->capacity*2:128;
        if(capacity>32768) {p->failed=DREADZONE_STATIC_BODY_UNSUPPORTED;return;}
        grow=(BodyWrite *)realloc(p->writes,(size_t)capacity*sizeof(*grow));
        if(!grow) {p->failed=DREADZONE_STATIC_BODY_UNSUPPORTED;return;}
        p->writes=grow;p->capacity=capacity;
    }
    p->writes[p->used].address=address;p->writes[p->used++].value=value;
}
static void body_quad(BodyPlan *p,u32 address,const float xyz[3],u32 w) {
    u32 i;for(i=0;i<3;i++)body_write(p,address+i*4,body_bits(xyz[i]));body_write(p,address+12,w);
}
static u32 body_clip(u16 vertex,u16 lower,u16 upper) {
    return ((u16)(vertex-lower)>>15)|(((u16)(upper-vertex)>>15)<<1);
}
/* Full sphere/triangle computation542540..5426F0. Source record filters and
 * packed clip words are performed by the caller in the original order. */
static int body_triangle(BodyPlan *p,const float vertex[3][3],const float center[3],
                         float *best,float position[3],float normal[3]) {
    float e2[3],e3[3],delta[3],mul[3],n[3],proj[3],qvec[3],side[3][3];
    float dot,norm2,q,d2,test[3],difference;u32 i;
    body_difference(p,vertex[1],vertex[0],e2);body_difference(p,vertex[2],vertex[0],e3);
    body_difference(p,center,vertex[0],delta);body_cross(p,e3,e2,n);
    body_product(p,delta,n,mul);dot=body_sum(p,mul);body_product(p,n,n,mul);norm2=body_sum(p,mul);
    q=body_div(p,dot,norm2);
    if(body_bits(dot)&0x80000000U)return 0;
    for(i=0;i<3;i++) {
        qvec[i]=body_mul(p,n[i],q);proj[i]=body_add(p,delta[i],body_mul(p,body_sub(p,0.f,n[i]),q));
    }
    body_product(p,qvec,qvec,mul);d2=body_sum(p,mul);
    body_cross(p,proj,e2,side[0]);body_cross(p,e3,proj,side[1]);
    body_difference(p,e3,e2,qvec);body_difference(p,proj,e2,mul);body_cross(p,mul,qvec,side[2]);
    difference=body_sub(p,*best,d2);
    body_product(p,side[0],n,mul);test[0]=body_sum(p,mul);
    if(body_bits(difference)&0x80000000U)return 0;
    for(i=1;i<3;i++) {body_product(p,side[i],n,mul);test[i]=body_sum(p,mul);}
    if((body_bits(test[0])|body_bits(test[1])|body_bits(test[2]))&0x80000000U) {
        float begin[3],end[3],edge[3],toward[3],numerator,denominator;
        for(i=0;i<3;i++) {
            if(body_bits(test[0])&0x80000000U) {begin[i]=0.f;end[i]=e2[i];}
            else if(body_bits(test[1])&0x80000000U) {begin[i]=e3[i];end[i]=0.f;}
            else {begin[i]=e2[i];end[i]=e3[i];}
        }
        body_difference(p,proj,begin,toward);body_difference(p,end,begin,edge);
        body_product(p,toward,edge,mul);numerator=body_sum(p,mul);
        body_product(p,edge,edge,mul);denominator=body_sum(p,mul);
        q=body_div(p,numerator,denominator);q=q<0.f?0.f:q;q=q>1.f?1.f:q;
        for(i=0;i<3;i++)proj[i]=body_add(p,begin[i],body_mul(p,edge[i],q));
        body_difference(p,center,vertex[0],toward);body_difference(p,proj,toward,toward);
        body_product(p,toward,toward,mul);d2=body_sum(p,mul);
        difference=body_sub(p,*best,d2);if(body_bits(difference)&0x80000000U)return 0;
    }
    *best=body_mul(p,d2,body_float(0x3F7FDF3BU));
    for(i=0;i<3;i++) {normal[i]=body_add(p,n[i],0.f);position[i]=body_add(p,proj[i],vertex[0][i]);}
    return !p->failed;
}

static int body_calculate(BodyPlan *p) {
    float center[3],lowerf[4],upperf[4],radius,best,position[3],normal[3],winner[3][3];
    float vertices[256][3];u32 clip[256],selected[1023],selected_count=0;
    u16 lower[4],upper[4];s32 packed_center[4];
    u32 group,count,group_address,record,next,i,j,collect=p->r.collect_end_guest,vf2_w=0;
    int have=0;
    if(!body_range(p,p->r.center_guest,16,16))return p->failed;
    for(i=0;i<3;i++) {
        float x=body_float(body_read(p,p->r.center_guest+i*4,4));
        if(!body_valid(x)||x<0||x>1023.0f||!body_valid(p->r.radius)||p->r.radius<=0) return DREADZONE_STATIC_BODY_UNSUPPORTED;
        center[i]=body_mul(p,x,64.f);packed_center[i]=(s32)center[i];
    }
    radius=body_mul(p,p->r.radius,64.f);
    if(!body_valid(radius)||radius>32767.f)return DREADZONE_STATIC_BODY_UNSUPPORTED;
    packed_center[3]=(s32)body_sub(p,0.f,radius);
    for(i=0;i<4;i++) {
        lowerf[i]=i==3?0.f:body_sub(p,center[i],radius);
        upperf[i]=i==3?0.f:body_add(p,center[i],radius);
        if(lowerf[i]<-32768.f||upperf[i]>65535.f)return DREADZONE_STATIC_BODY_UNSUPPORTED;
        lower[i]=(u16)(s32)lowerf[i];upper[i]=(u16)(s32)upperf[i];
    }
    if(!p->r.table_guest)return 0;
    count=body_read(p,p->r.table_guest,4);
    if((s32)count>1023)return DREADZONE_STATIC_BODY_UNSUPPORTED;
    if(!body_address(p,p->r.table_guest,16,&record))return p->failed;
    /* The initial8-byte preload also executes when count is zero/negative. */
    if(!body_range(p,record,8,8))return p->failed;
    for(group=0;(s32)group<(s32)count;group++) {
        s32 delta[4],absolute[4],square[4],a;u16 packed[4];int reject=0;
        for(i=0;i<4;i++)packed[i]=(u16)body_read(p,record+i*2,2);
        if(!body_address(p,record,16,&next)||!body_range(p,next,8,8))return p->failed;
        for(i=0;i<4;i++) {delta[i]=(s32)packed[i]-packed_center[i];absolute[i]=delta[i]<0?-delta[i]:delta[i];}
        for(i=0;i<3;i++)if(absolute[3]-absolute[i]<0)reject=1;
        if(!reject) {
            for(i=0;i<4;i++) {s32 half=delta[i]>=0?delta[i]/2:-((-delta[i]+1)/2);s32 h=(s16)(u16)half;square[i]=h*h;}
            /* The original uses three trapping signed SUB instructions, not a
             * wrapping aggregate. Reject atomically before a host overflow. */
            a=square[3];
            for(i=0;i<3;i++) {
                int64_t difference=(int64_t)a-square[i];
                if(difference<INT32_MIN||difference>INT32_MAX)return DREADZONE_STATIC_BODY_UNSUPPORTED;
                a=(s32)difference;
            }
            if(a>=0)selected[selected_count++]=record;
        }
        record=next;
    }
    p->info.selected_groups=selected_count;best=body_mul(p,radius,radius);
    for(group=0;group<selected_count;group++) {
        u32 payload,nv,nf,face,face_end,face_word;
        group_address=selected[group];payload=body_read(p,group_address+12,4);nv=body_read(p,group_address+10,2);
        if(nv==0||nv>256)return DREADZONE_STATIC_BODY_UNSUPPORTED;
        if(!body_range(p,payload,8,8))return p->failed;
        for(i=0;i<((nv+1)&~1U);i++) {
            u32 address,flags=0;u16 v[4];
            if(!body_address(p,payload,i*8,&address))return p->failed;
            for(j=0;j<4;j++)v[j]=(u16)body_read(p,address+j*2,2);
            for(j=0;j<4;j++)flags|=body_clip(v[j],lower[j],upper[j])<<(j*8);
            clip[i]=flags;for(j=0;j<3;j++)vertices[i][j]=(float)v[j];
            if(i&1)vf2_w=v[3];
        }
        /* The loop preloads the next16-byte pair before its signed nv-2 test. */
        if(!body_range(p,payload+(((nv+1)&~1U)*8),16,8))return p->failed;
        if(!body_address(p,payload,nv*8,&face))return p->failed;
        nf=body_read(p,group_address+8,1);
        if(!body_address(p,face,nf*4,&face_end))return p->failed;
        face_word=body_read(p,face,4);
        while(face!=face_end) {
            u32 word=face_word,indices[3],common;float triangle[3][3];
            face+=4;face_word=body_read(p,face,4);if(p->failed)return p->failed;
            for(i=0;i<3;i++) {indices[i]=(word>>(i*8))&255;if(indices[i]>=nv)return DREADZONE_STATIC_BODY_UNSUPPORTED;}
            common=clip[indices[0]]&clip[indices[1]]&clip[indices[2]];
            if((s32)common>0)continue;
            for(i=0;i<3;i++)for(j=0;j<3;j++)triangle[i][j]=vertices[indices[i]][j];
            vf2_w=clip[indices[1]];p->info.tested_faces++;
            if(body_triangle(p,triangle,center,&best,position,normal)) {
                have=1;p->info.contacts++;memcpy(winner,triangle,sizeof(winner));
                if(collect) {
                    float square[3],length2;u32 address;
                    best=body_mul(p,radius,radius);
                    if(collect==p->r.collect_end_guest-4096)goto finish;
                    body_product(p,normal,normal,square);length2=body_sum(p,square);collect-=64;
                    for(i=0;i<3;i++) {
                        float v[3];for(j=0;j<3;j++)v[j]=body_mul(p,winner[i][j],1.f/64.f);
                        address=collect+i*16;
                        body_quad(p,address,v,i==2?clip[indices[2]]:0);
                    }
                    body_quad(p,collect+48,normal,body_bits(length2));
                    body_write(p,collect+12,0x100C);body_write(p,collect+28,0);
                }
            }
            if(p->failed)return p->failed;
        }
    }
finish:
    if(collect)return (int)((p->r.collect_end_guest-collect)/64);
    if(have) {
        float position_world[3],adjusted[3],q,distance;
        if(!body_range(p,p->r.packet_guest,128,16))return p->failed;
        distance=body_checked(p,sqrtf(best));q=body_div(p,radius,distance);
        for(i=0;i<3;i++)position_world[i]=body_mul(p,position[i],1.f/64.f);
        body_quad(p,p->r.packet_guest+32,position_world,p->r.incoming_vf25_29_w[0]);
        body_quad(p,p->r.packet_guest+64,normal,p->r.incoming_vf25_29_w[1]);
        for(i=0;i<3;i++) {
            float v[3];for(j=0;j<3;j++)v[j]=body_mul(p,winner[i][j],1.f/64.f);
            body_quad(p,p->r.packet_guest+80+i*16,v,p->r.incoming_vf25_29_w[i+2]);
        }
        body_write(p,p->r.packet_guest+24,0);body_write(p,p->r.packet_guest+28,0x100C);
        for(i=0;i<3;i++) {
            float delta=body_sub(p,center[i],position[i]);
            delta=body_mul(p,delta,q);delta=body_mul(p,delta,1.f/64.f);adjusted[i]=body_add(p,delta,position_world[i]);
        }
        body_quad(p,p->r.packet_guest+48,adjusted,vf2_w);
    }
    return p->failed?p->failed:have;
}

int dreadzone_static_body_contact(const DreadzoneStaticBodyRequest *request,DreadzoneStaticBodyInfo *info) {
    BodyPlan p;uintptr_t a,b,c;u32 i;int result;
    if(!request||!info)return DREADZONE_STATIC_BODY_INVALID;
    a=(uintptr_t)request;b=(uintptr_t)info;
    if(a>UINTPTR_MAX-sizeof(*request)||b>UINTPTR_MAX-sizeof(*info)||
       (a<b+sizeof(*info)&&b<a+sizeof(*request)))return DREADZONE_STATIC_BODY_INVALID;
    memset(&p,0,sizeof(p));memcpy(&p.r,request,sizeof(p.r));c=(uintptr_t)p.r.storage;
    if(!p.r.storage||!p.r.storage_bytes||p.r.storage_bytes>0x80000000U||c>UINTPTR_MAX-p.r.storage_bytes||
       p.r.guest_base>0x7FFFFFFFU-(p.r.storage_bytes-1)||
       (p.r.guest_base<=0x70003FFFU&&p.r.guest_base+p.r.storage_bytes>0x70000000U)||
       (b<c+p.r.storage_bytes&&c<b+sizeof(*info)))return DREADZONE_STATIC_BODY_INVALID;
    if(p.r.collect_end_guest&&(p.r.collect_end_guest<4096||p.r.collect_end_guest>0x7FFFFFFFU||p.r.collect_end_guest%16))return DREADZONE_STATIC_BODY_INVALID;
    result=body_calculate(&p);
    if(p.failed)result=p.failed;
    if(result>=0) {
        for(i=0;i<p.used;i++)memcpy(p.r.storage+p.writes[i].address-p.r.guest_base,&p.writes[i].value,4);
        p.info.write_words=p.used;memcpy(info,&p.info,sizeof(*info));
    }
    free(p.writes);return result;
}
