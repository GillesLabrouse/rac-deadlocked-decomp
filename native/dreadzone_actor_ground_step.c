#include "dreadzone_actor_ground_step.h"
#include "dreadzone_angle_pair.h"
#include "dreadzone_input.h"
#include "dreadzone_wall_contact.h"
#include "dreadzone_wall_helpers.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct GroundRun {
    DreadzoneGroundRequest request;
    DreadzoneGroundInfo info;
    int status;
    DreadzoneProbeRegisters registers;
} GroundRun;
static int span(const void *p,uintptr_t n){return p && (uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *p,uintptr_t n,const void *q,uintptr_t m){
    uintptr_t a=(uintptr_t)p,b=(uintptr_t)q;return a>=b?a-b<m:b-a<n;
}
static void fail(GroundRun *r,int status){if(r->status>0)r->status=status;}
static u8 *memory(GroundRun *r,u32 address,u32 count,u32 alignment){
    u32 base=r->request.guest_base,n=r->request.storage_bytes;
    if((address&(alignment-1)) || address<base || count>n || address-base>n-count){
        fail(r,DREADZONE_GROUND_INVALID);return NULL;
    }
    return r->request.storage+(address-base);
}
static u32 word(GroundRun *r,u32 address){u32 v=0;u8 *p=memory(r,address,4,4);if(p)memcpy(&v,p,4);return v;}
static s32 integer(GroundRun *r,u32 address){u32 v=word(r,address);s32 s;memcpy(&s,&v,4);return s;}
static s16 half(GroundRun *r,u32 address){s16 v=0;u8 *p=memory(r,address,2,2);if(p)memcpy(&v,p,2);return v;}
static u8 byte(GroundRun *r,u32 address){u8 *p=memory(r,address,1,1);return p?*p:0;}
static void store(GroundRun *r,u32 address,const void *value,u32 n,u32 alignment){
    u8 *p=memory(r,address,n,alignment);
    /* The recovered atan implementation uses immutable numeric coefficients.
     * A guest write into their original backing changes future retail reads. */
    if(address<0x3B95C0U && (u64)address+n>0x3B9540U){fail(r,DREADZONE_GROUND_UNSUPPORTED_PATH);return;}
    if(p)memcpy(p,value,n);
}
static void write_word(GroundRun *r,u32 address,u32 v){store(r,address,&v,4,4);}
static void write_half(GroundRun *r,u32 address,s16 v){store(r,address,&v,2,2);}
static void write_byte(GroundRun *r,u32 address,u8 v){store(r,address,&v,1,1);}
static void quad(GroundRun *r,u32 address,u32 v[4]){u8 *p=memory(r,address,16,16);if(p)memcpy(v,p,16);else memset(v,0,16);}
static void write_quad(GroundRun *r,u32 address,const u32 v[4]){store(r,address,v,16,16);}
static f32 floating(GroundRun *r,u32 raw){
    f32 v;u32 m=raw&0x7FFFFFFFU;memcpy(&v,&raw,4);
    if(m && (m<0x00800000U || m>=0x7F800000U))fail(r,DREADZONE_GROUND_UNSUPPORTED_FLOAT);
    return v;
}
static u32 bits(GroundRun *r,f32 v){u32 w;memcpy(&w,&v,4);floating(r,w);return w;}
static f32 checked(GroundRun *r,f32 v){bits(r,v);return v;}
static f32 bounded(GroundRun *r,u32 raw,f32 limit,f32 floor){
    f32 v=floating(r,raw);
    if(fabsf(v)>limit || (v!=0.f && fabsf(v)<floor))fail(r,DREADZONE_GROUND_UNSUPPORTED_FLOAT);
    return v;
}
static f32 number(GroundRun *r,u32 address,f32 limit){return bounded(r,word(r,address),limit,0x1p-40f);}
static void write_float(GroundRun *r,u32 address,f32 v){write_word(r,address,bits(r,v));}
static f32 add(GroundRun *r,f32 a,f32 b){volatile f32 v=a+b;return checked(r,v);}
static f32 sub(GroundRun *r,f32 a,f32 b){volatile f32 v=a-b;return checked(r,v);}
static f32 mul(GroundRun *r,f32 a,f32 b){volatile f32 v=a*b;return checked(r,v);}
static f32 divide(GroundRun *r,f32 a,f32 b){
    volatile f32 v;
    if(b==0.f){fail(r,DREADZONE_GROUND_UNSUPPORTED_FLOAT);return 0.f;}
    v=a/b;return checked(r,v);
}
static f32 root(GroundRun *r,f32 a){volatile f32 v;
    if(a<0.f){fail(r,DREADZONE_GROUND_UNSUPPORTED_FLOAT);return 0.f;}
    v=sqrtf(a);return checked(r,v);
}
static f32 dot(GroundRun *r,const f32 a[3],const f32 b[3]){
    f32 x=mul(r,a[0],b[0]),y=mul(r,a[1],b[1]),z=mul(r,a[2],b[2]);
    return add(r,add(r,x,y),mul(r,1.f,z));
}
static f32 angle(GroundRun *r,f32 x,f32 y){
    static const f32 coefficients[8]={
        0.999999344348907470703125f,-0.33329856395721435546875f,
        0.1994653642177581787109375f,-0.13908533751964569091796875f,
        0.096420042216777801513671875f,-0.05590988695621490478515625f,
        0.0218612290918827056884765625f,-0.0040540578775107860565185546875f
    };
    static const f32 quadrants[16]={
        1.f,0.f,-1.f,1.57079637050628662109375f,
        -1.f,0.f,1.f,-1.57079637050628662109375f,
        -1.f,3.1415927410125732421875f,1.f,1.57079637050628662109375f,
        1.f,-3.1415927410125732421875f,-1.f,-1.57079637050628662109375f
    };
    u8 *table;
    f32 a=fabsf(x),b=fabsf(y),ratio,sum,result;
    if(fabsf(x)>1073741824.f || fabsf(y)>1073741824.f){fail(r,DREADZONE_GROUND_UNSUPPORTED_FLOAT);return 0.f;}
    if(a==0.f && b==0.f)return func_L01_0055D708(x,y);
    table=memory(r,0x3B9540,sizeof(coefficients),16);
    if(table && memcmp(table,coefficients,sizeof(coefficients)))fail(r,DREADZONE_GROUND_UNSUPPORTED_PATH);
    table=memory(r,0x3B9580,sizeof(quadrants),16);
    if(table && memcmp(table,quadrants,sizeof(quadrants)))fail(r,DREADZONE_GROUND_UNSUPPORTED_PATH);
    sum=add(r,a,b);ratio=divide(r,sub(r,a<b?a:b,a<b?b:a),sum);
    /* Explicit sufficient domain: ratio powers and every nonzero coefficient
     * product remain normal. Near-equal nonzero pairs outside it refuse. */
    if(ratio!=0.f && fabsf(ratio)<0.015625f){fail(r,DREADZONE_GROUND_UNSUPPORTED_FLOAT);return 0.f;}
    if(r->status<0)return 0.f;
    result=func_L01_0055D708(x,y);return checked(r,result);
}
static int trig(GroundRun *r,f32 a,f32 pair[2]){
    if(dreadzone_angle_pair(a,pair)<0){fail(r,DREADZONE_GROUND_UNSUPPORTED_FLOAT);return 0;}return 1;
}
static u32 address(GroundRun *r,u32 base,u32 offset){
    if(base>0x7FFFFFFFU-offset){fail(r,DREADZONE_GROUND_INVALID);return 0;}return base+offset;
}
static void clipped(GroundRun *r,u32 first,u32 second){
    u32 current[4],base[4],i,j;f32 delta[3],scale;
    for(i=0;i<3 && r->status>0;++i){
        quad(r,second,current);
        if(floating(r,current[i])<0.f){
            quad(r,first,base);
            for(j=0;j<3;++j)delta[j]=sub(r,bounded(r,current[j],8192.f,0x1p-40f),bounded(r,base[j],8192.f,0x1p-40f));
            scale=fabsf(divide(r,sub(r,floating(r,base[i]),0.100000001490116119384765625f),delta[i]));
            for(j=0;j<3;++j)base[j]=bits(r,add(r,floating(r,base[j]),mul(r,delta[j],scale)));
            write_quad(r,second,base);
        }
    }
}
static void endpoints(GroundRun *r,u32 first,u32 second,s32 directional,f32 d12,f32 d13){
    u32 b=r->request.record_guest,position[4],raw[4],source,i,k;
    f32 delta[3],magnitude,amount,factor;
    if(directional){
        for(k=0;k<2 && r->status>0;++k){
            s32 selected=integer(r,0x21E878);amount=k?d12:d13;
            source=selected?b+0x270:address(r,word(r,b+0x2EF0),0xE0);
            if(selected)amount=-amount;
            quad(r,source,raw);
            for(i=0;i<3;++i)delta[i]=bounded(r,raw[i],16.f,0x1p-40f);
            /* Retail VMUL also executes W even though the dot discards it. */
            mul(r,floating(r,raw[3]),floating(r,raw[3]));
            magnitude=dot(r,delta,delta);factor=divide(r,amount,root(r,magnitude));
            quad(r,b+0xA0,position);
            for(i=0;i<3;++i)position[i]=bits(r,add(r,bounded(r,position[i],8192.f,0x1p-40f),mul(r,delta[i],factor)));
            write_quad(r,k?first:second,position);
        }
        clipped(r,first,second);
    }else{
        quad(r,b+0xA0,position);write_quad(r,second,position);memcpy(raw,position,16);
        raw[2]=bits(r,add(r,bounded(r,raw[2],8192.f,0x1p-40f),d13));write_quad(r,second,raw);
        if(floating(r,raw[2])<0.f || (floating(r,raw[2])==0.f && (raw[2]&0x80000000U)))raw[2]=0;
        write_quad(r,second,raw);write_quad(r,first,position);
        position[2]=bits(r,add(r,bounded(r,position[2],8192.f,0x1p-40f),d12));write_quad(r,first,position);
    }
}
static s32 packet_flags(GroundRun *r){s32 flags=integer(r,0x24235C);return flags<0?-1:(flags&0x60)>>5;}
static s32 history(GroundRun *r,s32 command,s32 maximum){
    s32 count=integer(r,r->request.record_guest+0x264C),i;u32 clock=word(r,r->request.record_guest+0x370),b=r->request.record_guest;
    if(count>8){fail(r,DREADZONE_GROUND_UNSUPPORTED_PATH);return 0;}
    for(i=0;i<count;++i){
        s32 signed_clock;memcpy(&signed_clock,&clock,4);
        if(maximum<signed_clock)return 0;
        if(integer(r,b+0x25EC+(u32)i*4)==command)return 1;
        clock+=word(r,b+0x262C+(u32)i*4);
    }
    return 0;
}
static f32 height(GroundRun *r,const u32 input[4],u32 output){
    u32 special[4],up[4]={0,0,0x3F800000U,0};f32 z,dx,dy,delta,radius;
    if(integer(r,0x2237D8)!=0){fail(r,DREADZONE_GROUND_UNSUPPORTED_PATH);return 0.f;}
    if(integer(r,0x2237DC)!=0){
        quad(r,0x2237C0,special);
        z=bounded(r,input[2],8192.f,0x1p-40f);delta=sub(r,z,bounded(r,special[2],8192.f,0x1p-40f));
        if(fabsf(delta)<.5f){
            dx=sub(r,bounded(r,input[0],8192.f,0x1p-40f),bounded(r,special[0],8192.f,0x1p-40f));
            dy=sub(r,bounded(r,input[1],8192.f,0x1p-40f),bounded(r,special[1],8192.f,0x1p-40f));
            r->registers.high64[0]=bits(r,mul(r,delta,delta));r->registers.high64[1]=bits(r,mul(r,floating(r,input[3]),floating(r,input[3])));r->registers.last_writer_pc=0x593548;
            radius=number(r,0x2237CC,8192.f);
            if(root(r,add(r,add(r,mul(r,dx,dx),mul(r,dy,dy)),0.f))<radius){
                if(output)write_quad(r,output,up);
                return number(r,0x2237C8,8192.f);
            }
        }
    }
    if(output)write_quad(r,output,up);return bounded(r,input[2],8192.f,0x1p-40f);
}
static void side_offset(GroundRun *r,u32 output,s32 add_position,f32 distance,f32 a,f32 z){
    u32 b=r->request.record_guest,raw[4],matrix[4][4],position[4],i,j;
    f32 pair[2],in[4],out[4],acc;
    if(!trig(r,a,pair))return;
    quad(r,output,raw);in[0]=mul(r,pair[1],distance);in[1]=mul(r,pair[0],distance);
    in[2]=add(r,0.f,z);in[3]=floating(r,raw[3]);
    for(i=0;i<4;++i)raw[i]=bits(r,in[i]);write_quad(r,output,raw);
    for(j=0;j<4;++j)quad(r,b+0x20+j*16,matrix[j]);
    for(i=0;i<4;++i){
        acc=mul(r,bounded(r,matrix[0][i],16.f,0x1p-40f),in[0]);
        for(j=1;j<4;++j)acc=add(r,acc,mul(r,bounded(r,matrix[j][i],16.f,0x1p-40f),in[j]));
        out[i]=acc;raw[i]=bits(r,acc);
    }
    write_quad(r,output,raw);
    if(add_position){
        quad(r,b+0xA0,position);
        for(i=0;i<3;++i)raw[i]=bits(r,add(r,out[i],bounded(r,position[i],8192.f,0x1p-40f)));
        write_quad(r,output,raw);
    }
}
static void surface_euler(GroundRun *r,const u32 input[4],u32 output,f32 local[2]){
    u32 body=word(r,r->request.record_guest+0x2EF0),i,j;
    f32 p[3][2],col[3][3],sx,cx,sy,cy,sz,cz,a,b,c,d,e,f,g,h,v[4],out[3],acc,magnitude,pitch;
    for(i=0;i<4;++i)v[i]=bounded(r,input[i],16.f,0x1p-40f);
    for(i=0;i<3;++i)if(!trig(r,number(r,address(r,body,0xF0+i*4),3.1415927410125732421875f),p[i]))return;
    sx=p[0][0];cx=p[0][1];sy=p[1][0];cy=p[1][1];sz=p[2][0];cz=p[2][1];
    a=mul(r,sx,sy);b=mul(r,cx,sy);c=mul(r,cy,sx);d=mul(r,cx,cy);
    e=mul(r,a,sz);f=mul(r,cx,sz);g=mul(r,cz,a);h=mul(r,cx,cz);
    col[1][0]=sub(r,g,f);col[1][1]=add(r,h,e);col[1][2]=c;
    e=mul(r,b,sz);f=mul(r,-cz,sx);g=mul(r,sx,sz);h=mul(r,b,cz);
    col[2][0]=add(r,h,g);col[2][1]=add(r,f,e);col[2][2]=d;
    col[0][0]=mul(r,cy,cz);col[0][1]=mul(r,cy,sz);col[0][2]=-sy;
    for(i=0;i<3;++i){
        acc=mul(r,col[i][0],v[0]);for(j=1;j<3;++j)acc=add(r,acc,mul(r,col[i][j],v[j]));
        out[i]=add(r,acc,mul(r,0.f,v[3]));
    }
    /* Actual PEXTLW 682630 then PCPYUD 682648: a1.word1 is column2.Y. */
    r->registers.high64[0]=bits(r,col[2][1]);r->registers.high64[1]=0;r->registers.last_writer_pc=0x682648;
    magnitude=root(r,add(r,mul(r,out[0],out[0]),mul(r,out[2],out[2])));
    pitch=angle(r,magnitude,out[1]);
    if(local)local[0]=pitch;else write_float(r,output,pitch);
    pitch=-angle(r,out[2],out[0]);
    if(local)local[1]=pitch;else write_float(r,address(r,output,4),pitch);
}

static s32 converted(GroundRun *r,f32 value){
    if(!isfinite(value) || (double)value< -2147483648.0 || (double)value>=2147483648.0){
        fail(r,DREADZONE_GROUND_UNSUPPORTED_FLOAT);return 0;
    }
    return (s32)value; /* Actual finite in-range CVT.W.S truncation. */
}
static void normalize(GroundRun *r,u32 raw[4]){
    f32 v[3],magnitude,q;u32 i;
    for(i=0;i<3;++i)v[i]=bounded(r,raw[i],1073741824.f,0x1p-40f);
    mul(r,floating(r,raw[3]),floating(r,raw[3]));
    magnitude=dot(r,v,v);
    /* The unchanged dot Y lane is1, so retail's low64 predicate is nonzero
     * even for zero XYZ. Its actual1/sqrt(0) Q saturates positive Fmax. */
    if(magnitude==0.f){u32 maximum=0x7F7FFFFFU;memcpy(&q,&maximum,4);}
    else q=divide(r,1.f,root(r,magnitude));
    for(i=0;i<3;++i)raw[i]=bits(r,mul(r,v[i],q));
}
static void local_clip(GroundRun *r,const u32 first[4],u32 second[4]){
    u32 i,j;f32 delta[3],scale;
    r->registers.high64[0]=second[2];r->registers.high64[1]=second[3];r->registers.last_writer_pc=0x5963AC;
    for(i=0;i<3 && r->status>0;++i){
        if(floating(r,second[i])<0.f){
            for(j=0;j<3;++j)delta[j]=sub(r,floating(r,second[j]),floating(r,first[j]));
            r->registers.high64[0]=bits(r,delta[2]);r->registers.high64[1]=second[3];r->registers.last_writer_pc=0x59640C;
            scale=fabsf(divide(r,sub(r,floating(r,first[i]),0.100000001490116119384765625f),delta[i]));
            for(j=0;j<3;++j)second[j]=bits(r,add(r,floating(r,first[j]),mul(r,delta[j],scale)));
            second[3]=first[3];
        }
        if(i<2){r->registers.high64[0]=second[2];r->registers.high64[1]=second[3];r->registers.last_writer_pc=0x596464;}
    }
}
static void local_endpoints(GroundRun *r,u32 first[4],u32 second[4],f32 d12,f32 d13){
    u32 b=r->request.record_guest,raw[4],*output,source,i,k;
    f32 v[3],amount,magnitude,q;
    for(k=0;k<2 && r->status>0;++k){
        s32 selected=integer(r,0x21E878);amount=k?d12:d13;
        source=selected?b+0x270:address(r,word(r,b+0x2EF0),0xE0);
        if(selected)amount=-amount;
        quad(r,source,raw);
        for(i=0;i<3;++i)v[i]=bounded(r,raw[i],16.f,0x1p-40f);
        mul(r,floating(r,raw[3]),floating(r,raw[3]));
        magnitude=dot(r,v,v);q=divide(r,amount,root(r,magnitude));
        output=k?first:second;quad(r,b+0xA0,output);
        for(i=0;i<3;++i)output[i]=bits(r,add(r,bounded(r,output[i],8192.f,0x1p-40f),mul(r,v[i],q)));
    }
    local_clip(r,first,second);
}
static void local_side(GroundRun *r,u32 output[4],f32 angle_value,f32 z){
    u32 b=r->request.record_guest,matrix[4][4],position[4],i,j;f32 pair[2],in[4],out[4],acc;
    if(!trig(r,angle_value,pair))return;
    in[0]=mul(r,pair[1],.2f);in[1]=mul(r,pair[0],.2f);in[2]=add(r,0.f,z);in[3]=floating(r,output[3]);
    for(j=0;j<4;++j)quad(r,b+0x20+j*16,matrix[j]);
    for(i=0;i<4;++i){
        acc=mul(r,bounded(r,matrix[0][i],16.f,0x1p-40f),in[0]);
        for(j=1;j<4;++j)acc=add(r,acc,mul(r,bounded(r,matrix[j][i],16.f,0x1p-40f),in[j]));
        out[i]=acc;output[i]=bits(r,acc);
    }
    quad(r,b+0xA0,position);
    for(i=0;i<3;++i)output[i]=bits(r,add(r,out[i],bounded(r,position[i],8192.f,0x1p-40f)));
}
static int probe(GroundRun *r,const u32 first[4],const u32 second[4],u32 flags,int side){
    DreadzoneWallRequest q;DreadzoneWallInfo info;int status;
    memset(&q,0,sizeof(q));q.storage=r->request.storage;
    q.guest_base=r->request.guest_base;q.storage_bytes=r->request.storage_bytes;
    q.packet_guest=0x242340;q.grid_guest=0x26FF20;q.actor_pool_pointer_guest=0x222D14;
    q.actor_pool_count=r->request.actor_pool_count;q.grid_block_count=r->request.grid_block_count;
    q.ignored_actor_guest=word(r,r->request.record_guest+0x2EF0);q.caller_flags=flags;q.cache_guest=0;
    memcpy(q.incoming_vf25_27_w,r->info.outgoing_vf25_29_w,12);
    if(r->status<0)return 0;
    {u32 prior_writer=r->registers.last_writer_pc;status=dreadzone_wall_contact_registers(&q,first,second,r->registers.high64,&r->registers,&info);if(status>=0 && !r->registers.last_writer_pc)r->registers.last_writer_pc=prior_writer;}
    if(status<0){fail(r,status==DREADZONE_WALL_INVALID?DREADZONE_GROUND_INVALID:DREADZONE_GROUND_UNSUPPORTED_PATH);return 0;}
    r->info.probe_calls++;r->info.side_probe_calls+=side!=0;
    r->info.outgoing_vf25_29_w[3]=r->info.outgoing_vf25_29_w[4]=0;
    return status;
}
static s32 surface(GroundRun *r){
    s32 out=0;u8 *p=memory(r,0x242340,128,16);int status;
    if(!p)return 0;status=dreadzone_wall_surface_00542FF8(p,128,&out);
    if(status<0)fail(r,DREADZONE_GROUND_UNSUPPORTED_PATH);return out;
}
static f32 inclination(GroundRun *r){
    u8 *record=memory(r,r->request.record_guest,0x31F0,16),*normal=memory(r,0x242380,16,16);
    f32 out=0.f;int status;if(!record || !normal)return 0.f;
    status=dreadzone_wall_inclination_0068AFF8(record,0x31F0,normal,16,&out);
    if(status<0)fail(r,DREADZONE_GROUND_UNSUPPORTED_FLOAT);return checked(r,out);
}
static void ground_update(GroundRun *r){
    u32 b=r->request.record_guest,first[4]={0},second[4]={0},raw[4],position[4],normal[4],side[2][4]={{0}},i,index;
    s32 state,previous,count,rounded,force=r->request.integer0;f32 time,h,depth,value,delta,distance,v[3],n[3],euler[2],tilt;
    if(integer(r,0x21E824)!=0){fail(r,DREADZONE_GROUND_UNSUPPORTED_PATH);return;}
    if(force==0 && number(r,b+0x2D4,16.f)>-.1f && integer(r,b+0x2F10)%2==integer(r,0x21DDB8)%2){
        state=integer(r,b+0x25CC);
        if((state!=2 || number(r,b+0x2E08,16.f)>.5f) && state!=3 && byte(r,b+0x2659)!=1 && word(r,b+0x2EC)==0)return;
    }
    if(byte(r,b+0x2659)!=0){fail(r,DREADZONE_GROUND_UNSUPPORTED_PATH);return;}
    time=bounded(r,word(r,b+0x25C4),16.f,0x1p-20f);
    if(time<0.f)fail(r,DREADZONE_GROUND_UNSUPPORTED_FLOAT);
    normal[0]=normal[1]=normal[3]=0;normal[2]=0x3F800000U;write_quad(r,b+0x250,normal);
    rounded=converted(r,add(r,(f32)half(r,b+0x2FE),time));write_half(r,b+0x2FE,(s16)rounded);
    rounded=converted(r,add(r,(f32)half(r,b+0x2FC),time));write_half(r,b+0x2FC,(s16)rounded);
    write_float(r,b+0x2CC,32.f);write_word(r,b+0x2D0,0);write_word(r,b+0x2C8,0);
    memset(raw,0,16);write_quad(r,b+0x280,raw);write_half(r,b+0x2FA,0);write_word(r,b+0x2EC,0);
    count=integer(r,b+0x2E8);if(count!=0)write_word(r,b+0x2E8,(u32)converted(r,add(r,(f32)count,time)));
    h=mul(r,number(r,b+0x204,16.f),2.2f);previous=integer(r,b+0x25D4);
    if(previous==2)h=mul(r,number(r,b+0x204,16.f),2.9f);
    state=integer(r,b+0x25CC);
    if((u32)((u32)previous-17U)<2U || state==106 || state==129 || state==117 || state==116)h=0.f;
    /* Original673A00 is a pure predicate, so the subsequent selector reload
     * cannot change here. A guest alias is nevertheless read at its real site. */
    depth=integer(r,0x21E824)==1?-20.f:-48.f;
    local_endpoints(r,first,second,h,depth);
    if(!probe(r,first,second,2,0))goto ground_done;
    write_byte(r,b+0x43B,(u8)packet_flags(r));if(word(r,0x242358)!=0)write_byte(r,b+0x43B,0);
    write_half(r,b+0x430,(s16)surface(r));
    if(half(r,b+0x430)==0){
        quad(r,0x242360,raw);r->registers.high64[0]=raw[2];r->registers.high64[1]=raw[3];r->registers.last_writer_pc=0x67E144;value=height(r,raw,b+0x260);write_float(r,b+0x2E0,value);
        if(number(r,b+0xA8,8192.f)<value && integer(r,b+0x2E8)<=0)write_word(r,b+0x2E8,1);
        if(!probe(r,first,second,0x24,0))goto ground_done;
        write_byte(r,b+0x43B,(u8)packet_flags(r));
    }
    if(half(r,b+0x430)==13){
        value=number(r,0x242368,8192.f);write_float(r,b+0x2E0,value);first[2]=bits(r,add(r,0.f,sub(r,value,.01f)));
        if(!probe(r,first,second,4,0))goto ground_done;
    }
    if(half(r,b+0x430)==3){
        value=number(r,0x242368,8192.f);write_float(r,b+0x2E4,value);first[2]=bits(r,add(r,0.f,sub(r,value,.01f)));
        if(!probe(r,first,second,4,0))goto ground_done;
    }
    if(half(r,b+0x430)==11 || (half(r,b+0x430)==1 && integer(r,b+0x25CC)==123))goto ground_done;
    if(half(r,b+0x430)==4){write_word(r,b+0x2E4,word(r,0x242368));goto ground_done;}
    if(word(r,0x242358)!=0){fail(r,DREADZONE_GROUND_UNSUPPORTED_PATH);return;}
    if(integer(r,0x24235C)<=0)goto ground_done;
    quad(r,0x242360,raw);write_quad(r,b+0x280,raw);write_word(r,b+0x2C8,word(r,0x242368));
    if(integer(r,0x21E878)!=0){fail(r,DREADZONE_GROUND_UNSUPPORTED_PATH);return;}
    quad(r,b+0xA0,position);quad(r,b+0x280,raw);
    for(i=0;i<3;++i)v[i]=sub(r,bounded(r,position[i],8192.f,0x1p-40f),bounded(r,raw[i],8192.f,0x1p-40f));
    /* VU computes a square of the unchanged A0 W too. */
    mul(r,floating(r,position[3]),floating(r,position[3]));distance=root(r,dot(r,v,v));write_float(r,b+0x2CC,distance);
    write_word(r,b+0x2EC,word(r,0x242358));quad(r,0x242380,normal);normalize(r,normal);write_quad(r,b+0x250,normal);
    tilt=inclination(r);write_float(r,b+0x2D0,tilt);
    value=angle(r,number(r,0x242380,1073741824.f),number(r,0x242384,1073741824.f));write_float(r,b+0x2DC,value);
    quad(r,b+0xA0,position);quad(r,b+0x280,raw);quad(r,b+0x250,normal);
    for(i=0;i<3;++i){v[i]=sub(r,floating(r,raw[i]),floating(r,position[i]));n[i]=floating(r,normal[i]);}
    mul(r,floating(r,raw[3]),floating(r,normal[3]));value=dot(r,v,n);
    if(value>0.f)write_float(r,b+0x2CC,-number(r,b+0x2CC,8192.f));
    if(number(r,b+0x2CC,8192.f)<.02f){
        write_half(r,b+0x2FC,0);
        if(number(r,b+0x2D0,3.1415927410125732421875f)<=.872664630413055419921875f || byte(r,b+0x2659)!=0){
            write_half(r,b+0x2FE,0);write_word(r,b+0x630,word(r,0x242358));
        }
    }
    ++r->info.ground_contacts;
ground_done:
    if(r->status<0)return;
    if(number(r,b+0x2E0,8192.f)<number(r,b+0xA8,8192.f))write_word(r,b+0x2E8,0);
    if(half(r,b+0x430)==4 && half(r,b+0x2FE)==0)
        write_half(r,b+0x2FE,(s16)converted(r,add(r,(f32)half(r,b+0x2FE),number(r,b+0x25C4,16.f))));
    if(half(r,b+0x2FE)!=0)write_word(r,b+0x2F0,0);
    else{
        write_word(r,b+0x2F0,(u32)converted(r,add(r,(f32)integer(r,b+0x2F0),number(r,b+0x25C4,16.f))));
        quad(r,b+0x280,raw);write_quad(r,b+0x290,raw);
        if(!history(r,0x38,0x78) || half(r,b+0x420)!=0)write_word(r,b+0x1F70,0);
    }
    memset(raw,0,16);write_quad(r,b+0x270,raw);
    state=byte(r,b+0x2659);
    if(state!=0){fail(r,DREADZONE_GROUND_UNSUPPORTED_PATH);return;}
    /* Actual mode0 branch67E5D4 ->67E5F4. Mode1 owns the normal/2AC
     * calculation at67E608; those fields are not read on this selected path. */
    memset(raw,0,16);raw[2]=0xBF800000U;write_quad(r,b+0x270,raw);
    state=byte(r,b+0x266D);
    write_word(r,b+0x2D4,0);write_word(r,b+0x2D8,0);write_word(r,b+0x2C0,0);write_word(r,b+0x2C4,0);
    write_word(r,b+0x2B8,0);write_word(r,b+0x2BC,0);write_word(r,b+0x2B0,0);write_word(r,b+0x2B4,0);
    if(state==0 && (word(r,b+0x2F10)&7)!=(word(r,0x21DDB8)&7))return;
    if(!(number(r,b+0x2CC,8192.f)<.25f))return;
    state=integer(r,b+0x25D4);
    if(state!=0 && state!=12 && state!=9 && state!=7 && !(state==4 && byte(r,b+0x1D0C)!=0) && state!=1)return;
    quad(r,b+0x250,raw);surface_euler(r,raw,0,euler);
    if(euler[1]>-.785398185253143310546875f && euler[1]<.785398185253143310546875f)write_float(r,b+0x2D4,euler[1]);
    if(euler[0]>-.785398185253143310546875f && euler[0]<.785398185253143310546875f)write_float(r,b+0x2D8,euler[0]);
    side[0][3]=r->request.incoming_side_output_w[0];side[1][3]=r->request.incoming_side_output_w[1];
    for(index=0;index<2 && r->status>0;++index){
        h=index?-1.57079637050628662109375f:1.57079637050628662109375f;
        local_side(r,side[0],h,.449999988079071044921875f);local_side(r,side[1],h,-.449999988079071044921875f);
        if(probe(r,side[0],side[1],0x22,1)){
            if(integer(r,0x21E878)!=0){fail(r,DREADZONE_GROUND_UNSUPPORTED_PATH);return;}
            delta=sub(r,number(r,0x242368,8192.f),number(r,b+0x2C8,8192.f));
            write_float(r,b+0x2B0+index*4,delta);if(fabsf(delta)>.4f)write_word(r,b+0x2B0+index*4,0);
            quad(r,0x242380,normal);normalize(r,normal);surface_euler(r,normal,0,euler);
            if(euler[1]>-.785398185253143310546875f && euler[1]<.785398185253143310546875f)write_float(r,b+0x2B8+index*4,euler[1]);
            if(euler[0]>-.785398185253143310546875f && euler[0]<.785398185253143310546875f)write_float(r,b+0x2C0+index*4,euler[0]);
        }else{
            write_word(r,b+0x2B0+index*4,0);write_word(r,b+0x2B8+index*4,word(r,b+0x2D4));write_word(r,b+0x2C0+index*4,word(r,b+0x2D8));
        }
    }
}

static int ground_execute(const DreadzoneGroundRequest *request,const u32 incoming[2],DreadzoneProbeRegisters *output,DreadzoneGroundInfo *info,int tracked){
    GroundRun r;u8 *shadow,*destination;u32 b,end,input[4];int status;
    if(!span(request,sizeof(*request)) || !span(info,sizeof(*info)) ||
       overlap(request,sizeof(*request),info,sizeof(*info)))return DREADZONE_GROUND_INVALID;
    if(tracked && (!span(incoming,8) || !span(output,sizeof(*output)) ||
       request->operation!=DREADZONE_GROUND_UPDATE ||
       overlap(output,sizeof(*output),request,sizeof(*request)) ||
       overlap(output,sizeof(*output),info,sizeof(*info))))return DREADZONE_GROUND_INVALID;
    memset(&r,0,sizeof(r));memcpy(&r.request,request,sizeof(r.request));b=r.request.record_guest;
    if(tracked)memcpy(r.registers.high64,incoming,8);
    if(!r.request.storage_bytes || r.request.storage_bytes>64U*1024U*1024U ||
       !span(r.request.storage,r.request.storage_bytes) || r.request.guest_base>=0x80000000U ||
       r.request.storage_bytes>0x80000000U-r.request.guest_base || (b&15) ||
       r.request.operation>7 || r.request.actor_pool_count>65536 ||
       overlap(info,sizeof(*info),r.request.storage,r.request.storage_bytes))return DREADZONE_GROUND_INVALID;
    if(tracked && overlap(output,sizeof(*output),r.request.storage,r.request.storage_bytes))return DREADZONE_GROUND_INVALID;
    end=r.request.guest_base+r.request.storage_bytes;
    if(b<r.request.guest_base || b>end || end-b<0x31F0 ||
       (r.request.guest_base<0x70004000U && end>0x70000000U))return DREADZONE_GROUND_INVALID;
    destination=r.request.storage;shadow=(u8 *)malloc(r.request.storage_bytes);
    if(!shadow)return DREADZONE_GROUND_TRANSACTION_MEMORY;
    memcpy(shadow,destination,r.request.storage_bytes);r.request.storage=shadow;r.status=1;
    memcpy(r.info.outgoing_vf25_29_w,r.request.incoming_vf25_29_w,20);
    switch(r.request.operation){
        case DREADZONE_GROUND_UPDATE:ground_update(&r);break;
        case DREADZONE_GROUND_ENDPOINTS:
            endpoints(&r,r.request.value_guest,r.request.target_guest,r.request.integer0,
                bounded(&r,bits(&r,r.request.scalar[0]),8192.f,0x1p-40f),
                bounded(&r,bits(&r,r.request.scalar[1]),8192.f,0x1p-40f));break;
        case DREADZONE_GROUND_CLIP:clipped(&r,r.request.value_guest,r.request.target_guest);break;
        case DREADZONE_GROUND_HEIGHT:
            quad(&r,r.request.value_guest,input);r.info.original_float=height(&r,input,r.request.target_guest);r.info.has_float_return=1;break;
        case DREADZONE_GROUND_PACKET_FLAGS:r.info.original_integer=packet_flags(&r);r.info.has_integer_return=1;break;
        case DREADZONE_GROUND_HISTORY:r.info.original_integer=history(&r,r.request.integer0,r.request.integer1);r.info.has_integer_return=1;break;
        case DREADZONE_GROUND_SURFACE_EULER:quad(&r,r.request.value_guest,input);surface_euler(&r,input,r.request.target_guest,NULL);break;
        case DREADZONE_GROUND_SIDE_OFFSET:
            side_offset(&r,r.request.target_guest,r.request.integer0,
                bounded(&r,bits(&r,r.request.scalar[0]),16.f,0x1p-40f),
                bounded(&r,bits(&r,r.request.scalar[1]),3.1415927410125732421875f,0x1p-40f),
                bounded(&r,bits(&r,r.request.scalar[2]),8192.f,0x1p-40f));break;
    }
    status=r.status;if(status>0){memcpy(destination,shadow,r.request.storage_bytes);memcpy(info,&r.info,sizeof(*info));if(tracked)memcpy(output,&r.registers,sizeof(*output));}
    free(shadow);return status;
}

int dreadzone_actor_ground_step(const DreadzoneGroundRequest *request,DreadzoneGroundInfo *info){
    return ground_execute(request,NULL,NULL,info,0);
}
int dreadzone_actor_ground_registers(const DreadzoneGroundRequest *request,const u32 incoming[2],DreadzoneProbeRegisters *output,DreadzoneGroundInfo *info){
    return ground_execute(request,incoming,output,info,1);
}
