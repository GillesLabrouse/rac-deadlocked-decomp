#include "dreadzone_vertex_blend.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#if FLT_RADIX != 2 || FLT_MANT_DIG != 24 || FLT_MAX_EXP != 128
#error Vertex blend requires IEEE binary32 host floats
#endif
typedef char vertex_float_size_check[sizeof(float) == 4 ? 1 : -1];

static u16 read16(const u8 *p) { return (u16)((u32)p[0] | (u32)p[1] << 8); }
static float read_float(const u8 *p) {
    u32 bits = (u32)p[0] | (u32)p[1]<<8 | (u32)p[2]<<16 | (u32)p[3]<<24;
    float value; memcpy(&value, &bits, 4); return value;
}
static void write_float(u8 *p, float value) {
    u32 bits; memcpy(&bits, &value, 4);
    p[0]=(u8)bits; p[1]=(u8)(bits>>8); p[2]=(u8)(bits>>16); p[3]=(u8)(bits>>24);
}
static int valid(float v) { return isfinite(v) && (v==0.0f || fabsf(v)>=FLT_MIN); }
static int range(const void *p, size_t bytes) {
    return p!=NULL && bytes!=0 && (uintptr_t)p<=UINTPTR_MAX-bytes;
}
static int overlap(const void *a,size_t n,const void *b,size_t m) {
    return (uintptr_t)a<(uintptr_t)b+m && (uintptr_t)b<(uintptr_t)a+n;
}
static int load(const DreadzoneVertexState *state,u32 address,float matrix[16]) {
    u32 i;
    if ((address&3)!=0 || address>244 ||
        (state->matrix_defined & ((u64)1<<(address/4)))==0) return 0;
    for(i=0;i<16;i++) {
        matrix[i]=read_float(state->matrix_cache+address*16+i*4);
        if(!valid(matrix[i])) return 0;
    }
    return 1;
}
static int store(DreadzoneVertexState *state,u32 address,const float matrix[16]) {
    u32 i;
    if((address&3)!=0 || address>244) return 0;
    for(i=0;i<16;i++) write_float(state->matrix_cache+address*16+i*4,matrix[i]);
    state->matrix_defined |= (u64)1<<(address/4);
    return 1;
}
static int transfer(DreadzoneVertexState *state,const u8 *joints,u32 count,u32 joint,u32 address) {
    float matrix[16]; u32 i;
    if(joint>=count) return 0;
    for(i=0;i<16;i++) {
        matrix[i]=read_float(joints+joint*64+i*4);
        if(!valid(matrix[i])) return 0;
    }
    return store(state,address,matrix);
}
static int blended(DreadzoneVertexState *state,const u8 *row,u32 ways,float result[16]) {
    float sources[3][16],weights[3]; u32 i,j,addresses[3];
    addresses[0]=row[2]; addresses[1]=row[3];
    addresses[2]=((u32)read16(row)>>9)*2;
    for(j=0;j<ways;j++) {
        if(!load(state,addresses[j],sources[j])) return 0;
        /* Actual micro0204/0244 and0454/048C: ITOF12 then MULi16. */
        weights[j]=(float)row[4+j]*(1.0f/256.0f);
    }
    for(i=0;i<16;i++) {
        volatile float a=sources[0][i]*weights[0];
        volatile float b=sources[1][i]*weights[1];
        volatile float sum=a+b;
        if(!valid(a)||!valid(b)||!valid(sum)) return 0;
        result[i]=sum;
        if(ways==3) {
            volatile float c=sources[2][i]*weights[2];
            volatile float total=sum+c;
            if(!valid(c)||!valid(total)) return 0;
            result[i]=total;
        }
    }
    return store(state,row[7],result);
}
static int position(const float matrix[16],const u8 *raw,float result[4]) {
    float xyz[3]; u32 i;
    for(i=0;i<3;i++) {
        u32 bits=read16(raw+10+i*2);
        xyz[i]=(float)(bits&32768 ? (s32)bits-65536 : (s32)bits);
    }
    /* Actual local intermediate: MULAy, MADDAz, MADDAw, MADD translation.
     * Microvf13 lanes are normal-pair,x,y,z, not GLB coordinates. */
    for(i=0;i<4;i++) {
        volatile float a=matrix[i]*xyz[0];
        volatile float b=matrix[4+i]*xyz[1];
        volatile float sum=a+b;
        volatile float c=matrix[8+i]*xyz[2];
        volatile float sum2=sum+c;
        volatile float total=sum2+matrix[12+i];
        if(!valid(a)||!valid(b)||!valid(sum)||!valid(c)||!valid(sum2)||!valid(total)) return 0;
        result[i]=total;
    }
    return 1;
}

int dreadzone_vertex_blend_prepare(const DreadzoneVertexBlend *request) {
    DreadzoneVertexBlend input;
    DreadzoneVertexPacket packets[256];
    DreadzoneVertexState *state;
    DreadzoneBlendedVertex *output;
    const void *mutable[4]; size_t lengths[4];
    size_t output_bytes,needed; uintptr_t work;
    u32 p,i,j,n=0;
    if(!range(request,sizeof(input))) return DREADZONE_VERTEX_INVALID;
    memcpy(&input,request,sizeof(input));
    if(input.packet_count==0||input.packet_count>256||input.joint_count==0||input.joint_count>127||
        input.output_capacity==0||input.output_capacity>256*127||
        !range(input.packets,input.packet_count*sizeof(*packets))||
        !range(input.joint_matrices,input.joint_count*64)||
        !range(input.state,sizeof(*state))||!range(input.output_count,sizeof(u32)))
        return DREADZONE_VERTEX_INVALID;
    output_bytes=input.output_capacity*sizeof(*output);
    needed=sizeof(*state)+output_bytes+15;
    if(!range(input.output,output_bytes)||!range(input.workspace,input.workspace_bytes)||
        input.workspace_bytes<needed) return DREADZONE_VERTEX_CAPACITY;
    memcpy(packets,input.packets,input.packet_count*sizeof(*packets));
    mutable[0]=input.state;lengths[0]=sizeof(*state);
    mutable[1]=input.output;lengths[1]=output_bytes;
    mutable[2]=input.output_count;lengths[2]=sizeof(u32);
    mutable[3]=input.workspace;lengths[3]=input.workspace_bytes;
    for(i=0;i<4;i++) {
        if(overlap(mutable[i],lengths[i],input.joint_matrices,input.joint_count*64)||
            overlap(mutable[i],lengths[i],request,sizeof(input))||
            overlap(mutable[i],lengths[i],input.packets,input.packet_count*sizeof(*packets)))
            return DREADZONE_VERTEX_INVALID;
        for(j=0;j<i;j++) if(overlap(mutable[i],lengths[i],mutable[j],lengths[j]))
            return DREADZONE_VERTEX_INVALID;
    }
    for(p=0;p<input.packet_count;p++) {
        if(!range(packets[p].storage,packets[p].bytes)||packets[p].bytes<16||
            (packets[p].bytes&15)!=0||packets[p].bytes>255*16) return DREADZONE_VERTEX_INVALID;
        for(i=0;i<4;i++) if(overlap(mutable[i],lengths[i],packets[p].storage,packets[p].bytes))
            return DREADZONE_VERTEX_INVALID;
    }
    work=((uintptr_t)input.workspace+15)&~(uintptr_t)15;
    state=(DreadzoneVertexState *)work;
    output=(DreadzoneBlendedVertex *)(work+sizeof(*state));
    memcpy(state,input.state,sizeof(*state));
    for(p=0;p<input.packet_count;p++) {
        const u8 *packet=packets[p].storage,*vertices,*dups;
        u32 transfers=read16(packet),two=read16(packet+2),three=read16(packet+4);
        u32 main=read16(packet+6),duplicates=read16(packet+8),total=read16(packet+10);
        u32 start=read16(packet+12),count=two+three+main,epilogue,duplicate_start;
        if(total!=count+duplicates||total>127||count<3||(start&15)!=0||
            start>packets[p].bytes||count*16>packets[p].bytes-start)
            return DREADZONE_VERTEX_UNSUPPORTED;
        epilogue=(packets[p].bytes-start)/16-count;
        duplicate_start=(16+transfers*2+7)&~7U;
        if(epilogue<1||epilogue>6||count+epilogue<7||
            duplicate_start>start||duplicates*2>start-duplicate_start)
            return DREADZONE_VERTEX_UNSUPPORTED;
        if(total>input.output_capacity-n) return DREADZONE_VERTEX_CAPACITY;
        for(i=0;i<transfers;i++)
            if(!transfer(state,input.joint_matrices,input.joint_count,
                packet[16+i*2],packet[17+i*2])) return DREADZONE_VERTEX_UNSUPPORTED;
        vertices=packet+start;dups=packet+duplicate_start;
        for(i=0;i<count;i++) {
            const u8 *raw=vertices+i*16; u32 id,delayed=i+7;
            DreadzoneBlendedVertex *result=output+n++;
            memset(result,0,sizeof(*result));
            if(i<two) {
                if(!transfer(state,input.joint_matrices,input.joint_count,read16(raw)>>9,raw[6])||
                    !blended(state,raw,2,result->matrix)) return DREADZONE_VERTEX_UNSUPPORTED;
            } else if(i<two+three) {
                if(!blended(state,raw,3,result->matrix)) return DREADZONE_VERTEX_UNSUPPORTED;
            } else {
                if(!transfer(state,input.joint_matrices,input.joint_count,read16(raw)>>9,raw[3])||
                    !load(state,raw[2],result->matrix)) return DREADZONE_VERTEX_UNSUPPORTED;
            }
            if(!position(result->matrix,raw,result->position)) return DREADZONE_VERTEX_UNSUPPORTED;
            if(delayed<count+epilogue) id=read16(vertices+delayed*16)&511;
            else {
                u32 terminal=delayed-count-epilogue;
                if(terminal>=6) return DREADZONE_VERTEX_UNSUPPORTED;
                id=read16(vertices+(count+epilogue-1)*16+4+terminal*2)&511;
            }
            result->packet=p;result->row=(u16)i;result->index=(u16)id;
            memcpy(result->original,raw,16);
            state->vertex_cache[id]=*result;state->vertex_defined[id]=1;
        }
        for(i=0;i<duplicates;i++) {
            u32 id=read16(dups+i*2)>>7;
            DreadzoneBlendedVertex *result=output+n++;
            if(id>=512||state->vertex_defined[id]!=1) return DREADZONE_VERTEX_UNSUPPORTED;
            *result=state->vertex_cache[id];
            if(result->index!=id) return DREADZONE_VERTEX_UNSUPPORTED;
            for(j=0;j<16;j++) if(!valid(result->matrix[j])) return DREADZONE_VERTEX_UNSUPPORTED;
            for(j=0;j<4;j++) if(!valid(result->position[j])) return DREADZONE_VERTEX_UNSUPPORTED;
            result->packet=p;result->row=(u16)(count+i);result->duplicate=1;
        }
    }
    memcpy(input.state,state,sizeof(*state));
    memcpy(input.output,output,n*sizeof(*output));
    memcpy(input.output_count,&n,sizeof(n));
    return DREADZONE_VERTEX_APPROXIMATE;
}
