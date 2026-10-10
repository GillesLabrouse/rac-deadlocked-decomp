#include "dreadzone_topology.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static unsigned checks,failures;
#define CHECK(x) do { checks++;if(!(x)) { failures++;printf("Line %u failed: %s\n",(unsigned)__LINE__,#x); } } while(0)
static void word(u8 *p,u32 value) { p[0]=(u8)value;p[1]=(u8)(value>>8);p[2]=(u8)(value>>16);p[3]=(u8)(value>>24); }
static void half(u8 *p,s16 value) { p[0]=(u8)value;p[1]=(u8)((u16)value>>8); }
static void make_packet(u8 data[128],int textured) {
    static const u8 indices[]={129,130,3,132,5,6,7,8,9,0};u32 i;
    memset(data,0,128);word(data,0x750980C2);
    for(i=0;i<9;i++) { half(data+4+i*4,(s16)(-4096+(s32)i*37));half(data+6+i*4,(s16)(8192-(s32)i*17)); }
    word(data+40,0x6E04812D);data[44]=0xFE;
    data[45]=(u8)(textured?4:0);data[46]=(u8)(textured?1:0);
    memcpy(data+48,indices,sizeof(indices));
    if(textured) {
        data[48]=0;word(data+60,0x6C048131);word(data+64,4);word(data+80,7);
    }
}
static u32 make_multi(u8 data[272],u32 groups) {
    static const u8 indices[]={0,130,3,0,5,6,7,8,9,1,2,3,0};u32 i;
    memset(data,0,272);word(data,0x750980C2);
    for(i=0;i<9;i++) { half(data+4+i*4,(s16)(-4096+(s32)i*37));half(data+6+i*4,(s16)(8192-(s32)i*17)); }
    word(data+40,0x6E05812D);data[44]=0xFE;data[45]=5;data[46]=1;
    memcpy(data+48,indices,sizeof(indices));if(groups==3)data[53]=0;
    word(data+64,0x6C008132|(groups*4)<<16);
    for(i=0;i<groups;i++) {
        u8 *group=data+68+i*64;
        word(group,0xFF27);word(group+4,4);word(group+8,0x41A00004);
        word(group+12,0xE7);
        word(group+16,11*(i+1));word(group+24,8);
        word(group+40,6);word(group+56,52);
    }
    for(i=0;i<groups;i++) word(data+68+i*16+12,i+1<groups?(i==0?4:6):0);
    return groups==3?272:208;
}
int main(void) {
    u8 first[128],second[128];DreadzoneBlendedVertex vertices[18];
    DreadzoneTopologyPacket packets[2];DreadzoneTopologyTexture texture;
    struct {u32 before;DreadzoneTopologyTriangle rows[6];u32 after;} output;
    DreadzoneTopologyRequest request;u32 count,i,j;
    make_packet(first,1);make_packet(second,0);memset(vertices,0,sizeof(vertices));
    for(i=0;i<18;i++) { vertices[i].packet=i/9;vertices[i].index=(u16)(100+i); }
    packets[0].vif=first;packets[0].vif_bytes=128;packets[0].first_vertex=0;packets[0].vertex_count=9;
    packets[1].vif=second;packets[1].vif_bytes=64;packets[1].first_vertex=9;packets[1].vertex_count=9;
    memset(&request,0,sizeof(request));request.packets=packets;request.packet_count=2;
    request.vertices=vertices;request.vertex_count=18;request.texture=&texture;
    request.triangles=output.rows;request.triangle_capacity=6;request.triangle_count=&count;
    texture.index=-77;texture.defined=0;count=0xABCDEF12;memset(&output,0xA5,sizeof(output));
    CHECK(dreadzone_topology_prepare(&request)==DREADZONE_TOPOLOGY_DIAGNOSTIC);
    CHECK(count==6);CHECK(texture.index==7 && texture.defined==1);
    CHECK(output.before==0xA5A5A5A5 && output.after==0xA5A5A5A5);
    for(i=0;i<6;i++) {
        u32 base=(i/3)*9,start=i%3==0?0:i%3+1;
        CHECK(output.rows[i].packet==i/3);CHECK(output.rows[i].texture==7);
        for(j=0;j<3;j++) {
            u32 ordinal=start+j;
            CHECK(output.rows[i].output_index[j]==base+ordinal);
            CHECK(output.rows[i].guest_id[j]==100+base+ordinal);
            CHECK(output.rows[i].st[j][0]==-4096+(s32)ordinal*37);
            CHECK(output.rows[i].st[j][1]==8192-(s32)ordinal*17);
        }
    }
    {
        DreadzoneTopologyTriangle saved[6];DreadzoneTopologyTexture saved_texture=texture;u32 saved_count=count;
        memcpy(saved,output.rows,sizeof(saved));
        second[44]=0;
        CHECK(dreadzone_topology_prepare(&request)==DREADZONE_TOPOLOGY_UNSUPPORTED);
        CHECK(memcmp(saved,output.rows,sizeof(saved))==0 && count==saved_count && memcmp(&texture,&saved_texture,sizeof(texture))==0);
        second[44]=0xFE;request.triangle_capacity=5;
        CHECK(dreadzone_topology_prepare(&request)==DREADZONE_TOPOLOGY_CAPACITY);
        CHECK(memcmp(saved,output.rows,sizeof(saved))==0 && count==saved_count && memcmp(&texture,&saved_texture,sizeof(texture))==0);
        request.triangle_capacity=6;second[56]=127;
        CHECK(dreadzone_topology_prepare(&request)==DREADZONE_TOPOLOGY_INVALID);
        CHECK(memcmp(saved,output.rows,sizeof(saved))==0 && count==saved_count && memcmp(&texture,&saved_texture,sizeof(texture))==0);
        second[56]=9;
        packets[1].first_vertex=0xFFFFFFFF;
        CHECK(dreadzone_topology_prepare(&request)==DREADZONE_TOPOLOGY_INVALID);
        packets[1].first_vertex=9;
        request.triangles=(DreadzoneTopologyTriangle *)first;
        CHECK(dreadzone_topology_prepare(&request)==DREADZONE_TOPOLOGY_INVALID);
        request.triangles=output.rows;request.triangle_count=(u32 *)&packets[0];
        CHECK(dreadzone_topology_prepare(&request)==DREADZONE_TOPOLOGY_INVALID);
        request.triangle_count=&count;request.vertices=(DreadzoneBlendedVertex *)(UINTPTR_MAX-8);
        CHECK(dreadzone_topology_prepare(&request)==DREADZONE_TOPOLOGY_INVALID);
        request.vertices=vertices;
        packets[0].vif=(u8 *)(UINTPTR_MAX-8);
        CHECK(dreadzone_topology_prepare(&request)==DREADZONE_TOPOLOGY_INVALID);
        packets[0].vif=first;
    }
    request.packets=&packets[1];request.packet_count=1;packets[1].first_vertex=0;
    for(i=0;i<9;i++)vertices[i].packet=0;
    texture.defined=0;
    CHECK(dreadzone_topology_prepare(&request)==DREADZONE_TOPOLOGY_UNDEFINED_TEXTURE);
    CHECK(count==6);
    {
        u8 vif[272];DreadzoneTopologyTriangle rows[8];u32 groups;
        request.packets=packets;request.packet_count=1;request.triangles=rows;request.triangle_capacity=8;
        packets[0].vif=vif;packets[0].first_vertex=0;packets[0].vertex_count=9;
        for(groups=2;groups<=3;groups++) {
            static const u32 expected_two[5][3]={{0,1,2},{3,4,5},{4,5,6},{5,6,7},{6,7,8}};
            static const u32 expected_three[3][3]={{0,1,2},{5,6,7},{6,7,8}};
            packets[0].vif_bytes=make_multi(vif,groups);texture.defined=0;count=0xFEDCBA98;
            CHECK(dreadzone_topology_prepare(&request)==DREADZONE_TOPOLOGY_DIAGNOSTIC);
            CHECK(count==(groups==3?3u:5u));CHECK(texture.index==(s32)(11*groups) && texture.defined==1);
            for(i=0;i<count;i++) {
                CHECK(rows[i].texture==(s32)(i==0?11:11*groups));
                for(j=0;j<3;j++) {
                    u32 ordinal=groups==3?expected_three[i][j]:expected_two[i][j];
                    CHECK(rows[i].output_index[j]==ordinal);CHECK(rows[i].guest_id[j]==100+ordinal);
                    CHECK(rows[i].st[j][0]==-4096+(s32)ordinal*37);CHECK(rows[i].st[j][1]==8192-(s32)ordinal*17);
                }
            }
        }
    }
    printf("%u checks, %u failures\n",checks,failures);
    return failures?1:0;
}
