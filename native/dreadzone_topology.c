#include "dreadzone_topology.h"
#include <stdint.h>
#include <string.h>

static u16 read16(const u8 *p) { return (u16)((u32)p[0] | (u32)p[1]<<8); }
static u32 read32(const u8 *p) { return (u32)p[0] | (u32)p[1]<<8 | (u32)p[2]<<16 | (u32)p[3]<<24; }
static int range(const void *p, size_t bytes) {
    return p!=NULL && bytes!=0 && (uintptr_t)p<=UINTPTR_MAX-bytes;
}
static int overlap(const void *a, size_t n, const void *b, size_t m) {
    return n!=0 && m!=0 && (uintptr_t)a<(uintptr_t)b+m && (uintptr_t)b<(uintptr_t)a+n;
}
typedef struct Layout { const u8 *st, *indices, *textures; u32 st_count, index_bytes, texture_groups; } Layout;
static int layout(const DreadzoneTopologyPacket *packet, Layout *result) {
    u32 cursor=0, found=0;
    memset(result,0,sizeof(*result));
    while(cursor<packet->vif_bytes) {
        u32 word, command, count, bytes, immediate;
        if(packet->vif_bytes-cursor<4) return DREADZONE_TOPOLOGY_INVALID;
        word=read32(packet->vif+cursor); cursor+=4;
        command=(word>>24)&127; count=(word>>16)&255; immediate=word&65535;
        if(command==0) { if(word!=0) return DREADZONE_TOPOLOGY_UNSUPPORTED; continue; }
        if(count==0 || (word>>31)!=0) return DREADZONE_TOPOLOGY_UNSUPPORTED;
        if(found==0 && command==0x75 && immediate==0x80C2) {
            bytes=count*4;result->st=packet->vif+cursor;result->st_count=count;
        } else if(found==1 && command==0x6E && immediate==0x812D) {
            bytes=count*4;result->indices=packet->vif+cursor;result->index_bytes=bytes;
        } else if(found==2 && command==0x6C && immediate==0x812D+result->index_bytes/4 && (count&3)==0) {
            bytes=count*16;result->textures=packet->vif+cursor;result->texture_groups=count/4;
        } else return DREADZONE_TOPOLOGY_UNSUPPORTED;
        if(bytes>packet->vif_bytes-cursor) return DREADZONE_TOPOLOGY_INVALID;
        cursor+=bytes;found++;
    }
    if(found<2 || result->st_count!=packet->vertex_count || result->st_count>127)
        return DREADZONE_TOPOLOGY_INVALID;
    if(result->indices[0]!=0xFE || result->indices[3]!=0 ||
       result->indices[1]!=(result->texture_groups!=0 ? result->index_bytes/4 : 0))
        return DREADZONE_TOPOLOGY_UNSUPPORTED;
    return DREADZONE_TOPOLOGY_DIAGNOSTIC;
}
static int packet_pass(const DreadzoneTopologyRequest *request, u32 number,
                       DreadzoneTopologyTexture *texture, u32 *count, int publish) {
    DreadzoneTopologyPacket packet=request->packets[number]; Layout data;
    u8 ordinal[1020], adc[1020], restart[1020]; s32 material[1020];
    u32 i, tokens=0, group=0, terminated=0, fifo[3], filled=0;
    int status=layout(&packet,&data);
    if(status!=DREADZONE_TOPOLOGY_DIAGNOSTIC) return status;
    for(i=4;i<data.index_bytes;i++) {
        u32 value=data.indices[i];
        restart[tokens]=0;
        if(value==0) {
            /* VU1 keeps the secret cursor in vf10.w, advancing one quad;
             * vi11 separately advances four quads per texture payload. */
            value=group==0 ? data.indices[2] :
                  group<=data.texture_groups ? data.textures[(group-1)*16+12] : 0;
            if(value==0) { terminated=1;break; }
            if(group>=data.texture_groups) return DREADZONE_TOPOLOGY_INVALID;
            texture->index=(s32)read32(data.textures+group*64+16);
            texture->defined=1;group++;value|=128;restart[tokens]=1;
        }
        if((value&127)==0 || (value&127)>packet.vertex_count) return DREADZONE_TOPOLOGY_INVALID;
        if(!texture->defined) return DREADZONE_TOPOLOGY_UNDEFINED_TEXTURE;
        ordinal[tokens]=(u8)((value&127)-1);adc[tokens]=(u8)(value>>7);
        material[tokens]=texture->index;tokens++;
    }
    if(!terminated || tokens<3 || group!=data.texture_groups) return DREADZONE_TOPOLOGY_INVALID;
    for(i=0;i<tokens-3;i++) {
        u32 j;
        /* The inserted texture payload is followed by a fresh PRE/PRIM tag. */
        if(restart[i]) filled=0;
        if(filled<3) fifo[filled++]=ordinal[i];
        else { fifo[0]=fifo[1];fifo[1]=fifo[2];fifo[2]=ordinal[i]; }
        if(filled<3 || adc[i]) continue;
        if(*count==UINT32_MAX) return DREADZONE_TOPOLOGY_CAPACITY;
        if(publish) {
            DreadzoneTopologyTriangle triangle;
            memset(&triangle,0,sizeof(triangle));triangle.packet=number;triangle.texture=material[i];
            for(j=0;j<3;j++) {
                u32 index=packet.first_vertex+fifo[j];
                triangle.output_index[j]=index;triangle.guest_id[j]=request->vertices[index].index;
                triangle.st[j][0]=(s16)read16(data.st+fifo[j]*4);
                triangle.st[j][1]=(s16)read16(data.st+fifo[j]*4+2);
            }
            request->triangles[*count]=triangle;
        }
        (*count)++;
    }
    return DREADZONE_TOPOLOGY_DIAGNOSTIC;
}
int dreadzone_topology_prepare(const DreadzoneTopologyRequest *input) {
    DreadzoneTopologyRequest request; DreadzoneTopologyTexture initial,texture;
    size_t packet_bytes,vertex_bytes,triangle_bytes;u32 i,count=0;
    if(!range(input,sizeof(*input))) return DREADZONE_TOPOLOGY_INVALID;
    request=*input;
    if(request.packet_count==0 || request.packet_count>256 || request.vertex_count==0 ||
       request.vertex_count>256*127 || request.triangle_capacity>256*1016)
        return DREADZONE_TOPOLOGY_INVALID;
    packet_bytes=(size_t)request.packet_count*sizeof(*request.packets);
    vertex_bytes=(size_t)request.vertex_count*sizeof(*request.vertices);
    triangle_bytes=(size_t)request.triangle_capacity*sizeof(*request.triangles);
    if(!range(request.packets,packet_bytes)||!range(request.vertices,vertex_bytes)||
       !range(request.texture,sizeof(*request.texture))||!range(request.triangle_count,4)||
       (triangle_bytes!=0 && !range(request.triangles,triangle_bytes))) return DREADZONE_TOPOLOGY_INVALID;
    initial=*request.texture;if(initial.defined>1) return DREADZONE_TOPOLOGY_INVALID;
#define MUTABLE_OVERLAP(p,n) (overlap((p),(n),request.triangles,triangle_bytes)||overlap((p),(n),request.texture,sizeof(*request.texture))||overlap((p),(n),request.triangle_count,4))
    if(MUTABLE_OVERLAP(input,sizeof(*input))||MUTABLE_OVERLAP(request.packets,packet_bytes)||
       MUTABLE_OVERLAP(request.vertices,vertex_bytes)||
       overlap(request.triangles,triangle_bytes,request.texture,sizeof(*request.texture))||
       overlap(request.triangles,triangle_bytes,request.triangle_count,4)||
       overlap(request.texture,sizeof(*request.texture),request.triangle_count,4)) return DREADZONE_TOPOLOGY_INVALID;
    for(i=0;i<request.packet_count;i++) {
        const DreadzoneTopologyPacket *packet=&request.packets[i];u32 j;
        if(packet->vif_bytes<4 || packet->vif_bytes>65536 || (packet->vif_bytes&15)!=0 ||
           packet->vertex_count==0 || packet->vertex_count>127 || packet->first_vertex>request.vertex_count ||
           packet->vertex_count>request.vertex_count-packet->first_vertex ||
           !range(packet->vif,packet->vif_bytes)||MUTABLE_OVERLAP(packet->vif,packet->vif_bytes))
            return DREADZONE_TOPOLOGY_INVALID;
        for(j=0;j<packet->vertex_count;j++)
            if(request.vertices[packet->first_vertex+j].packet!=i || request.vertices[packet->first_vertex+j].index>511)
                return DREADZONE_TOPOLOGY_INVALID;
    }
#undef MUTABLE_OVERLAP
    texture=initial;
    for(i=0;i<request.packet_count;i++) {
        int status=packet_pass(&request,i,&texture,&count,0);
        if(status!=DREADZONE_TOPOLOGY_DIAGNOSTIC) return status;
    }
    if(count>request.triangle_capacity) return DREADZONE_TOPOLOGY_CAPACITY;
    count=0;texture=initial;
    for(i=0;i<request.packet_count;i++) {
        /* All refusal conditions were checked before any publication. */
        (void)packet_pass(&request,i,&texture,&count,1);
    }
    *request.texture=texture;*request.triangle_count=count;
    return DREADZONE_TOPOLOGY_DIAGNOSTIC;
}
