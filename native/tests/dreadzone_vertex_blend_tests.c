#include "dreadzone_vertex_blend.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static u32 checks,failures;
#define CHECK(c) do { checks++; if(!(c)) { failures++; printf("FAIL line%d: %s\n",__LINE__,#c); } } while(0)
static void put16(u8 *p,u32 v) { p[0]=(u8)v;p[1]=(u8)(v>>8); }
static void put_float(u8 *p,float v) { u32 bits;memcpy(&bits,&v,4);p[0]=(u8)bits;p[1]=(u8)(bits>>8);p[2]=(u8)(bits>>16);p[3]=(u8)(bits>>24); }
static void packet(u8 bytes[144],int second) {
    u32 i;memset(bytes,0,144);
    put16(bytes,second ? 0 : 3);put16(bytes+2,second ? 0 : 1);
    put16(bytes+4,second ? 0 : 1);put16(bytes+6,second ? 3 : 1);
    put16(bytes+8,1);put16(bytes+10,4);put16(bytes+12,32);
    if(!second) { bytes[16]=0;bytes[17]=0;bytes[18]=1;bytes[19]=4;bytes[20]=2;bytes[21]=8; }
    put16(bytes+(second ? 16 : 24),(second ? 100 : 101)<<7);
    for(i=0;i<3;i++) {
        u8 *row=bytes+32+i*16;
        put16(row,0);row[2]=12;row[3]=244;
        put16(row+10,2);put16(row+12,3);put16(row+14,4);
    }
    if(!second) {
        bytes[34]=0;bytes[35]=4;bytes[36]=64;bytes[37]=192;bytes[38]=244;bytes[39]=12;
        put16(bytes+48,4<<9);bytes[50]=0;bytes[51]=4;bytes[52]=64;bytes[53]=64;bytes[54]=128;bytes[55]=16;
        put16(bytes+64,2<<9);bytes[66]=16;bytes[67]=20;
    }
    for(i=0;i<6;i++) put16(bytes+32+6*16+4+i*2,(second ? 110 : 100)+i);
}
int main(void) {
    DreadzoneVertexState *state=(DreadzoneVertexState *)malloc(sizeof(*state));
    DreadzoneVertexState *before=(DreadzoneVertexState *)malloc(sizeof(*state));
    DreadzoneBlendedVertex output[10],saved[10];
    u8 first[144],second[144],joints[192],delayed[176];
    DreadzoneVertexPacket packets[2];DreadzoneVertexBlend request;
    u32 count=0,i,j,workspace_bytes=(u32)(sizeof(*state)+sizeof(output)+31);
    u8 *guard=(u8 *)malloc(workspace_bytes+32);
    CHECK(state!=NULL && before!=NULL && guard!=NULL);
    if(state==NULL||before==NULL||guard==NULL) return 1;
    memset(joints,0,sizeof(joints));
    for(i=0;i<3;i++) for(j=0;j<16;j++)
        put_float(joints+i*64+j*4,j==15 ? 1.0f : j%5==0 ? (float)(i+1) : j==12 ? (float)(i*8) : 0);
    packet(first,0);packet(second,1);
    packets[0].storage=first;packets[0].bytes=144;packets[1].storage=second;packets[1].bytes=144;
    memset(&request,0,sizeof(request));request.packets=packets;request.packet_count=2;
    request.joint_matrices=joints;request.joint_count=3;request.state=state;
    request.output=output;request.output_capacity=10;request.output_count=&count;
    request.workspace=guard+16;request.workspace_bytes=workspace_bytes;
    memset(guard,0xD2,workspace_bytes+32);memset(state,0,sizeof(*state));memset(output,0xA7,sizeof(output));
    CHECK(dreadzone_vertex_blend_prepare(&request)==DREADZONE_VERTEX_APPROXIMATE);
    CHECK(count==8);
    for(i=0;i<3;i++) { CHECK(output[i].index==100+i);CHECK(output[i].packet==0);CHECK(output[i].row==i);CHECK(output[i].duplicate==0); }
    CHECK(output[0].matrix[0]==1.75f && output[0].matrix[12]==6.0f);
    CHECK(output[0].position[0]==9.5f && output[0].position[1]==5.25f && output[0].position[2]==7.0f && output[0].position[3]==1);
    CHECK(output[1].matrix[0]==2.25f && output[1].matrix[12]==10.0f);
    CHECK(output[1].position[0]==14.5f && output[1].position[1]==6.75f && output[1].position[2]==9);
    CHECK(memcmp(output[1].matrix,output[2].matrix,64)==0);
    CHECK(output[3].index==101 && output[3].duplicate==1 && output[3].row==3);
    CHECK(memcmp(output[3].position,output[1].position,16)==0);
    CHECK(output[7].index==100 && output[7].duplicate==1 && output[7].packet==1);
    CHECK(memcmp(output[7].matrix,output[0].matrix,64)==0);
    for(i=4;i<7;i++) CHECK(output[i].index==106+i && output[i].matrix[0]==1.75f);
    for(i=0;i<16;i++) { CHECK(guard[i]==0xD2);CHECK(guard[workspace_bytes+16+i]==0xD2); }
    for(i=8*sizeof(*output);i<sizeof(output);i++) CHECK(((u8 *)output)[i]==0xA7);
    memcpy(before,state,sizeof(*state));memcpy(saved,output,sizeof(output));
#define REFUSAL(expected) do { count=0xABCD; CHECK(dreadzone_vertex_blend_prepare(&request)==expected); CHECK(count==0xABCD); CHECK(memcmp(before,state,sizeof(*state))==0); CHECK(memcmp(saved,output,sizeof(output))==0); } while(0)
    put16(second+12,33);REFUSAL(DREADZONE_VERTEX_UNSUPPORTED);put16(second+12,32);
    put16(second+16,511<<7);REFUSAL(DREADZONE_VERTEX_UNSUPPORTED);put16(second+16,100<<7);
    request.output_capacity=7;REFUSAL(DREADZONE_VERTEX_CAPACITY);request.output_capacity=10;
    request.workspace_bytes=10;REFUSAL(DREADZONE_VERTEX_CAPACITY);request.workspace_bytes=workspace_bytes;
    first[17]=248;REFUSAL(DREADZONE_VERTEX_UNSUPPORTED);first[17]=0;
    put_float(joints+128,INFINITY);REFUSAL(DREADZONE_VERTEX_UNSUPPORTED);put_float(joints+128,3);
    request.packet_count=257;REFUSAL(DREADZONE_VERTEX_INVALID);request.packet_count=2;
    request.packets=(const DreadzoneVertexPacket *)(uintptr_t)(UINTPTR_MAX-4);REFUSAL(DREADZONE_VERTEX_INVALID);request.packets=packets;
    request.workspace=state;REFUSAL(DREADZONE_VERTEX_INVALID);request.workspace=guard+16;
    packets[1].storage=(const u8 *)(uintptr_t)(UINTPTR_MAX-4);REFUSAL(DREADZONE_VERTEX_INVALID);packets[1].storage=second;
    packets[1].storage=(const u8 *)output;REFUSAL(DREADZONE_VERTEX_INVALID);packets[1].storage=second;
    /* Undefined source cache must be refused even if its bytes look valid. */
    request.packet_count=1;packets[0].storage=second;
    state->matrix_defined=0;memcpy(before,state,sizeof(*state));REFUSAL(DREADZONE_VERTEX_UNSUPPORTED);
    packets[0].storage=first;request.packet_count=2;
    /* Future record7, the epilogue header and six packed terminal IDs all
     * identify genuine transformed rows; signed coordinates retain sign. */
    memset(delayed,0,sizeof(delayed));put16(delayed,1);put16(delayed+6,8);
    put16(delayed+10,8);put16(delayed+12,32);delayed[16]=0;delayed[17]=0;
    for(i=0;i<8;i++) {
        u8 *row=delayed+32+i*16;row[2]=0;row[3]=244;
        put16(row+10,0xFFFF);put16(row+12,0x8000);put16(row+14,0x7FFF);
    }
    put16(delayed+32+7*16,200);put16(delayed+32+8*16,201);
    for(i=0;i<6;i++) put16(delayed+32+8*16+4+i*2,202+i);
    packets[0].storage=delayed;packets[0].bytes=176;request.packet_count=1;
    memset(state,0,sizeof(*state));
    CHECK(dreadzone_vertex_blend_prepare(&request)==DREADZONE_VERTEX_APPROXIMATE);CHECK(count==8);
    for(i=0;i<8;i++) {
        CHECK(output[i].index==200+i);CHECK(output[i].row==i);
        CHECK(output[i].position[0]==-1 && output[i].position[1]==-32768 && output[i].position[2]==32767);
        CHECK(state->vertex_defined[200+i]==1);
    }
    memcpy(before,state,sizeof(*state));memcpy(saved,output,sizeof(output));
    /* A late packet failure must preserve a completed earlier packet and
     * all preexisting cache bytes. */
    packets[0].storage=first;packets[0].bytes=144;
    packets[1].storage=second;packets[1].bytes=144;request.packet_count=2;
    put16(second+16,511<<7);REFUSAL(DREADZONE_VERTEX_UNSUPPORTED);put16(second+16,100<<7);
    request.packet_count=1;
    request.output_count=(u32 *)(uintptr_t)(UINTPTR_MAX-1);count=0xABCD;
    CHECK(dreadzone_vertex_blend_prepare(&request)==DREADZONE_VERTEX_INVALID);
    CHECK(memcmp(before,state,sizeof(*state))==0);CHECK(memcmp(saved,output,sizeof(output))==0);
    request.output_count=&count;
    free(guard);free(before);free(state);
    printf("%u checks, %u failures\n",checks,failures);
    return failures!=0;
}
