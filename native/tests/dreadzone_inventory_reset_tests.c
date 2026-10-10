#include "dreadzone_inventory_reset.h"
#include <stdio.h>
#include <string.h>
static unsigned checks,failures;
#define CHECK(x) do { ++checks; if(!(x)) { ++failures; if(failures<8) fprintf(stderr,"line%d: %s\n",__LINE__,#x); } } while(0)
static u8 data[0x12B0+64],expected[sizeof(data)];
static u8 inventory_byte(u32 offset,u8 prior) {
    if(offset==0) return 1;
    if((offset>=2 && offset<0x20) || offset==0x20 ||
        (offset>=0x2A && offset<0x30)) return 0;
    if(offset>=0x30 && offset<0xA30) return (offset-0x30)%0x50==2?0xFF:0;
    if(offset>=0xA30 && offset<0x12B0) return (offset-0xA30)%0x44<2?0xFF:0;
    return prior;
}
static void resets(void) {
    u32 seed,i,start;
    DreadzoneInventoryMemory view;
    for(seed=0;seed<128;++seed) {
        start=16+seed%16;
        for(i=0;i<sizeof(data);++i) data[i]=(u8)(seed*31+i*17+(i>>8));
        memcpy(expected,data,sizeof(data));
        for(i=0;i<0x12B0;++i) expected[start+i]=inventory_byte(i,data[start+i]);
        view.storage=data+start; view.guest_address=0x1D3F90+(seed%4)*0x12B0; view.bytes=0x12B0;
        CHECK(dreadzone_inventory_006C0D88(&view)==0);
        for(i=0;i<sizeof(data);++i) CHECK(data[i]==expected[i]);
    }
    memcpy(expected,data,sizeof(data));
    CHECK(dreadzone_inventory_006C0D88(NULL)==-1);
    view.storage=NULL; CHECK(dreadzone_inventory_006C0D88(&view)==-1);
    view.storage=data+16; view.bytes=0x12AF; CHECK(dreadzone_inventory_006C0D88(&view)==-1);
    view.bytes=0x12B0; view.guest_address=0x1D3F91; CHECK(dreadzone_inventory_006C0D88(&view)==-1);
    view.guest_address=0x7FFFF000; CHECK(dreadzone_inventory_006C0D88(&view)==-1);
    view.guest_address=0xFFFFF000; CHECK(dreadzone_inventory_006C0D88(&view)==-1);
    view.guest_address=0x1D3F90; view.storage=(u8 *)(uintptr_t)(UINTPTR_MAX-16);
    CHECK(dreadzone_inventory_006C0D88(&view)==-1);
    for(i=0;i<sizeof(data);++i) CHECK(data[i]==expected[i]);
}
static void aligned_clear(void) {
    u32 size,i,start;
    DreadzoneInventoryMemory view;
    for(size=0;size<=0x880;size+=16) for(start=16;start<32;++start) {
        memset(data,0x6D,sizeof(data));
        view.storage=data+start; view.guest_address=0x400000; view.bytes=size;
        CHECK(dreadzone_clear_0055D2F8(&view,(s32)size)==0);
        for(i=0;i<sizeof(data);++i) CHECK(data[i]==(i>=start && i<start+size?0:0x6D));
    }
    memset(data,0x6D,sizeof(data)); memcpy(expected,data,sizeof(data));
    view.storage=data+16; view.guest_address=0x400000; view.bytes=0x880;
    CHECK(dreadzone_clear_0055D2F8(NULL,16)==-1);
    CHECK(dreadzone_clear_0055D2F8(&view,-16)==-1);
    CHECK(dreadzone_clear_0055D2F8(&view,17)==-1);
    CHECK(dreadzone_clear_0055D2F8(&view,0x890)==-1);
    view.guest_address++; CHECK(dreadzone_clear_0055D2F8(&view,16)==-1);
    view.guest_address=0x7FFFF800; CHECK(dreadzone_clear_0055D2F8(&view,16)==-1);
    view.guest_address=0x400000; view.storage=(u8 *)(uintptr_t)(UINTPTR_MAX-16);
    CHECK(dreadzone_clear_0055D2F8(&view,16)==-1);
    for(i=0;i<sizeof(data);++i) CHECK(data[i]==expected[i]);
}
int main(void) {
    resets(); aligned_clear();
    printf("Inventory reset and aligned clear: %u checks, %u failures\n",checks,failures);
    return failures!=0;
}
