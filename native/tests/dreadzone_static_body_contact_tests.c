#include "dreadzone_static_body_contact.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static u8 bytes[0x6000],saved[sizeof(bytes)];
static unsigned checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"Failed line%d: %s\n",__LINE__,#x);return 1;}}while(0)
static void word(u32 offset,u32 x) {memcpy(bytes+offset,&x,4);}
static void half(u32 offset,u16 x) {memcpy(bytes+offset,&x,2);}
static u32 get(u32 offset) {u32 x;memcpy(&x,bytes+offset,4);return x;}
static void value(u32 offset,float x) {memcpy(bytes+offset,&x,4);}
static float load(u32 offset) {float x;memcpy(&x,bytes+offset,4);return x;}
static DreadzoneStaticBodyRequest fixture(void) {
    DreadzoneStaticBodyRequest r;u32 i;
    memset(bytes,0xA5,sizeof(bytes));memset(&r,0,sizeof(r));
    r.storage=bytes;r.storage_bytes=sizeof(bytes);r.guest_base=0x100000;
    r.center_guest=0x100000;r.packet_guest=0x100100;r.table_guest=0x100200;r.radius=1.f;
    for(i=0;i<5;i++)r.incoming_vf25_29_w[i]=0x80706050+i;
    value(0,101.f);value(4,101.f);value(8,99.5f);
    word(0x200,1);half(0x210,102*64);half(0x212,102*64);half(0x214,100*64);half(0x216,4*64);
    bytes[0x218]=1;half(0x21A,3);word(0x21C,0x100300);
    for(i=0;i<3;i++) {
        half(0x300+i*8,(u16)((i==1?104:100)*64));
        half(0x302+i*8,(u16)((i==2?104:100)*64));half(0x304+i*8,100*64);half(0x306+i*8,0);
    }
    word(0x318,0x00020100);word(0x31C,0);
    return r;
}
int main(void) {
    DreadzoneStaticBodyRequest r=fixture();DreadzoneStaticBodyInfo info,before;
    memcpy(saved,bytes,sizeof(bytes));memset(&info,0x5A,sizeof(info));
    CHECK(dreadzone_static_body_contact(&r,&info)==1);
    CHECK(info.selected_groups==1&&info.tested_faces==1&&info.contacts==1&&info.write_words==26);
    CHECK(get(0x118)==0&&get(0x11C)==0x100C);
    CHECK(load(0x120)==101.f&&load(0x124)==101.f&&load(0x128)==100.f);
    CHECK(load(0x140)==0.f&&load(0x144)==0.f&&load(0x148)==-65536.f);
    CHECK(load(0x138)<99.f&&load(0x138)>98.999f);
    CHECK(get(0x12C)==r.incoming_vf25_29_w[0]&&get(0x14C)==r.incoming_vf25_29_w[1]);
    CHECK(get(0x15C)==r.incoming_vf25_29_w[2]&&get(0x16C)==r.incoming_vf25_29_w[3]&&get(0x17C)==r.incoming_vf25_29_w[4]);
    CHECK(memcmp(bytes,saved,0x118)==0&&memcmp(bytes+0x180,saved+0x180,sizeof(bytes)-0x180)==0);
    r=fixture();value(8,100.5f);memcpy(saved,bytes,sizeof(bytes));
    CHECK(dreadzone_static_body_contact(&r,&info)==0);CHECK(memcmp(bytes,saved,sizeof(bytes))==0&&info.write_words==0);
    r=fixture();r.collect_end_guest=0x105000;
    CHECK(dreadzone_static_body_contact(&r,&info)==1&&info.write_words==18);
    CHECK(get(0x4FCC)==0x100C&&get(0x4FDC)==0&&load(0x4FC0)==100.f&&load(0x4FF8)==-65536.f);
    CHECK(load(0x4FFC)==4294967296.f&&get(0x118)==0xA5A5A5A5U);
    r=fixture();r.collect_end_guest=0x105000;bytes[0x218]=70;
    {u32 i;for(i=0;i<71;i++)word(0x318+i*4,0x00020100);}
    CHECK(dreadzone_static_body_contact(&r,&info)==64&&info.contacts==65&&info.write_words==64*18);
    CHECK(get(0x400C)==0x100C&&get(0x3FFC)==0xA5A5A5A5U);
    /* The first collect store overwrites the next face, which was genuinely
     * prefetched before that store. Both original faces must still execute. */
    r=fixture();r.collect_end_guest=0x100350;bytes[0x218]=2;word(0x31C,0x00020100);word(0x320,0);
    CHECK(dreadzone_static_body_contact(&r,&info)==2&&info.contacts==2&&info.tested_faces==2);
    CHECK(get(0x31C)==0x100C&&get(0x2DC)==0x100C);
    r=fixture();bytes[0x218]=2;word(0x31C,0x00020100);word(0x320,0);
    CHECK(dreadzone_static_body_contact(&r,&info)==1&&info.contacts==1&&info.tested_faces==2);
    /* On-plane contact really executes the zero-denominator final VRSQRT. */
    r=fixture();value(8,100.f);
    CHECK(dreadzone_static_body_contact(&r,&info)==1&&info.q_exception_bits==0x20&&info.q_bits==0x7F7FFFFFU);
    CHECK(load(0x138)==100.f);
    /* A zero-area triangle genuinely executes both invalid plane division
     * and the saturated final reciprocal square root in this numeric model. */
    r=fixture();value(8,100.f);
    {u32 i;for(i=0;i<3;i++){half(0x300+i*8,100*64);half(0x302+i*8,100*64);}}
    CHECK(dreadzone_static_body_contact(&r,&info)==1&&info.q_exception_bits==0x30);
    CHECK(load(0x148)==0.f&&load(0x138)==100.f);
    /* Packet publication may alias its consumed input table. No source field
     * is re-read after these actual final stores. */
    r=fixture();r.packet_guest=r.table_guest;
    CHECK(dreadzone_static_body_contact(&r,&info)==1&&get(0x21C)==0x100C);
    CHECK(load(0x228)==100.f);
    /* A later invalid source read refuses the whole transaction after a real
     * contact has already been planned. Public backing and info stay intact. */
    r=fixture();r.collect_end_guest=0x105000;bytes[0x218]=2;word(0x31C,0x00020103);
    memcpy(saved,bytes,sizeof(bytes));before=info;
    CHECK(dreadzone_static_body_contact(&r,&info)==DREADZONE_STATIC_BODY_UNSUPPORTED);
    CHECK(memcmp(bytes,saved,sizeof(bytes))==0&&memcmp(&info,&before,sizeof(info))==0);
    r=fixture();r.packet_guest=0xFFFFFFF0U;memcpy(saved,bytes,sizeof(bytes));memset(&info,0x5A,sizeof(info));before=info;
    CHECK(dreadzone_static_body_contact(&r,&info)==DREADZONE_STATIC_BODY_INVALID);
    CHECK(memcmp(bytes,saved,sizeof(bytes))==0&&memcmp(&info,&before,sizeof(info))==0);
    r=fixture();bytes[0x318]=3;memcpy(saved,bytes,sizeof(bytes));
    CHECK(dreadzone_static_body_contact(&r,&info)==DREADZONE_STATIC_BODY_UNSUPPORTED);CHECK(memcmp(bytes,saved,sizeof(bytes))==0);
    r=fixture();r.storage_bytes=0x80000001U;
    CHECK(dreadzone_static_body_contact(&r,&info)==DREADZONE_STATIC_BODY_INVALID);
    r=fixture();r.radius=NAN;memcpy(saved,bytes,sizeof(bytes));
    CHECK(dreadzone_static_body_contact(&r,&info)==DREADZONE_STATIC_BODY_UNSUPPORTED);CHECK(memcmp(bytes,saved,sizeof(bytes))==0);
    r=fixture();r.collect_end_guest=0x100020;memcpy(saved,bytes,sizeof(bytes));
    CHECK(dreadzone_static_body_contact(&r,&info)==DREADZONE_STATIC_BODY_INVALID);CHECK(memcmp(bytes,saved,sizeof(bytes))==0);
    /* A fully mapped broadphase can still execute a trapping signed SUB. */
    r=fixture();value(0,0.f);value(4,0.f);value(8,0.f);r.radius=511.f;
    half(0x210,65535);half(0x212,65535);half(0x214,65535);half(0x216,65535);
    memcpy(saved,bytes,sizeof(bytes));before=info;
    CHECK(dreadzone_static_body_contact(&r,&info)==DREADZONE_STATIC_BODY_UNSUPPORTED);
    CHECK(memcmp(bytes,saved,sizeof(bytes))==0&&memcmp(&info,&before,sizeof(info))==0);
    r=fixture();r.radius=1e30f;
    CHECK(dreadzone_static_body_contact(&r,&info)==DREADZONE_STATIC_BODY_UNSUPPORTED);
    r=fixture();r.center_guest++;
    CHECK(dreadzone_static_body_contact(&r,&info)==DREADZONE_STATIC_BODY_INVALID);
    r=fixture();r.collect_end_guest=0x105001;
    CHECK(dreadzone_static_body_contact(&r,&info)==DREADZONE_STATIC_BODY_INVALID);
    r=fixture();word(0x200,0);r.table_guest=r.guest_base+sizeof(bytes)-4;
    word(sizeof(bytes)-4,0);memcpy(saved,bytes,sizeof(bytes));
    CHECK(dreadzone_static_body_contact(&r,&info)==DREADZONE_STATIC_BODY_INVALID);
    CHECK(memcmp(bytes,saved,sizeof(bytes))==0);
    r=fixture();
    CHECK(dreadzone_static_body_contact(&r,(DreadzoneStaticBodyInfo *)(bytes+0x100))==DREADZONE_STATIC_BODY_INVALID);
    r=fixture();r.table_guest=0;
    CHECK(dreadzone_static_body_contact(&r,&info)==0&&info.contacts==0);
    printf("%u static body checks passed\n",checks);return 0;
}
