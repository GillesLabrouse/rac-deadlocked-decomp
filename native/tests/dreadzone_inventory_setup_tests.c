#include "dreadzone_inventory_setup.h"
#include <stdio.h>
#include <string.h>
static unsigned checks,failures,fixture_seed;
#define CHECK(value) do { ++checks;if(!(value)) { ++failures;if(failures<9) printf("line%d: %s\n",__LINE__,#value); } } while(0)
enum { RECORD,POOL,GLOBALS,FLAG,TABLES,REGIONS };
static const u32 sizes[REGIONS]={0x31F0,0x12B0*4,0xB00,1,0x2400};
static const u32 guests[REGIONS]={0x368F60,0x1D3F90,0x21DE00,0x171B66,0x3BDDC0};
static const u32 item_ids[]={2,3,12,7,14,1};
static u8 backing[REGIONS][0x4B00],expected[REGIONS][0x4B00];
static u8 *regions[REGIONS];
static u32 padding;
static DreadzoneInventorySetupReadMemory reads[3];
static DreadzoneInventorySetupContext context;
static void word(u8 *p,u32 n) { p[0]=(u8)n;p[1]=(u8)(n>>8);p[2]=(u8)(n>>16);p[3]=(u8)(n>>24); }
static void half(u8 *p,u16 n) { p[0]=(u8)n;p[1]=(u8)(n>>8); }
static u32 readword(const u8 *p) { return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24); }
static s32 readhalf(const u8 *p) { return (s32)(((u32)p[0]|((u32)p[1]<<8))^0x8000U)-0x8000; }
static void setup(unsigned seed,u32 mode,u32 level,s32 difficulty,u32 index,int existing) {
    unsigned r,i,item;
    fixture_seed=seed;
    padding=16+seed%16;
    for(r=0;r<REGIONS;++r) {
        for(i=0;i<sizes[r]+64;++i) backing[r][i]=(u8)(seed*31+r*17+i*13+(i>>8));
        regions[r]=backing[r]+padding;
    }
    for(i=0;i<3;++i) {
        reads[i].storage=regions[i+2];reads[i].guest_address=guests[i+2];reads[i].bytes=sizes[i+2];
    }
    word(regions[RECORD]+0x2F10,index);
    word(regions[GLOBALS]+0xA24,mode);word(regions[GLOBALS]+0xA28,level);
    word(regions[GLOBALS]+0x164,(u32)difficulty);
    regions[FLAG][0]=(u8)(seed%3);
    regions[POOL][0]=(u8)(existing?1:0);
    for(item=0;item<20;++item) {
        u8 *definition=regions[TABLES]+0x16A0+item*0xB0;
        definition[0x28]=(u8)((seed+item)%2);
        half(definition+0x42,(u16)(seed*719+item*3001));
        word(regions[TABLES]+0x28+item*4,seed*0x170001U+item*0x801003U);
        for(i=0;i<10;++i) {
            word(regions[TABLES]+0x398+item*0x28+i*4,i<1+seed%9?100U+i:0);
            word(regions[TABLES]+0x78+item*0x28+i*4,i==0?(item+seed)%9:0);
        }
    }
    context.record=regions[RECORD];context.record_guest=guests[RECORD];context.record_bytes=sizes[RECORD];
    context.pool.storage=regions[POOL];context.pool.guest_address=guests[POOL];context.pool.bytes=sizes[POOL];
    context.reads=reads;context.read_count=3;
}
static void save(void) {
    unsigned r;
    for(r=0;r<REGIONS;++r) memcpy(expected[r],backing[r],sizes[r]+64);
}
static void verify(void) {
    unsigned r,i;
    for(r=0;r<REGIONS;++r) for(i=0;i<sizes[r]+64;++i) {
        if(backing[r][i]!=expected[r][i] && failures<8)
            printf("seed%u region%u offset%X: %02X expected %02X\n",fixture_seed,r,i,backing[r][i],expected[r][i]);
        CHECK(backing[r][i]==expected[r][i]);
    }
}
static void expected_reset(u8 *inventory) {
    unsigned i;
    inventory[0x20]=0;half(inventory+0x2A,0);word(inventory+0x2C,0);
    memset(inventory+0x30,0,0x1280);
    for(i=0;i<32;++i) { inventory[0x32+i*0x50]=0xFF;half(inventory+0xA30+i*0x44,0xFFFF); }
    for(i=0;i<10;++i) { inventory[2+i]=0;half(inventory+0xC+i*2,0); }
    inventory[0]=1;
}
static void expected_result(u32 mode,u32 level,s32 difficulty,u32 index,int existing) {
    u8 *inventory=expected[POOL]+padding;
    unsigned i;
    u32 item,slot,amount;
    if(mode==1) {
        inventory+=index*0x12B0;
        expected_reset(inventory);
    } else if(!existing) {
        expected_reset(inventory);
        for(i=0;i<6;++i) {
            const u8 *definition;
            item=item_ids[i];
            if(item==7 && level!=1) continue;
            definition=regions[TABLES]+0x16A0+item*0xB0;
            half(inventory+0xA30+item*0x44,0);
            slot=readword(regions[TABLES]+0x78+item*0x28);
            /* Reset preserves shared counts at21..28. Existing slots are
             * consumed. Newly acquired slots bind only for item IDs>=2. */
            if(slot>=1 && slot<=8) {
                if(inventory[0x20+slot]) {
                    inventory[0xA44+item*0x44]=(u8)slot;
                    inventory[0x20+slot]--;
                } else if(item>=2) inventory[0xA44+item*0x44]=(u8)slot;
                else inventory[0x20+slot]=1;
            }
            if(definition[0x28]) {
                amount=(u32)readhalf(definition+0x42);
                if(inventory[0xA44+item*0x44]==2) amount+=readword(regions[TABLES]+0x28+item*4);
                amount&=0xFFFFU;
                if(amount&0x8000U) amount=0;
                half(inventory+0xA32+item*0x44,(u16)amount);
            }
        }
        inventory[0x20+(difficulty>=3?4:2)]++;
    }
    word(expected[RECORD]+padding+0x26E4,guests[POOL]+(mode==1?index*0x12B0:0));
}
static void rejected(const DreadzoneInventorySetupContext *host,int status) {
    save();CHECK(dreadzone_inventory_006909C8(host)==status);verify();
}
int main(void) {
    unsigned seed;
    for(seed=0;seed<512;++seed) {
        u32 mode=seed%4==3?1:0,index=(seed/4)%4,level=seed&1?1:0;
        s32 difficulty=(s32)(seed%7)-2;
        int existing=seed%4==2;
        setup(seed,mode,level,difficulty,index,existing);save();
        expected_result(mode,level,difficulty,index,existing);
        CHECK(dreadzone_inventory_006909C8(&context)==0);verify();
    }
    /* A caller-owned zero-initialized pool models the real fresh BSS state.
     * Zeroing here is fixture preparation, not an attributed reset effect. */
    for(seed=0;seed<64;++seed) {
        u32 level=seed&1;
        s32 difficulty=(s32)(seed%7)-2;
        setup(seed,0,level,difficulty,0,0);
        memset(regions[POOL],0,sizes[POOL]);save();
        expected_result(0,level,difficulty,0,0);
        CHECK(dreadzone_inventory_006909C8(&context)==0);verify();
    }
    setup(1,0,1,0,0,0);context.record_bytes--;rejected(&context,-1);
    setup(1,0,1,0,0,0);context.record_guest=0xFFFFF000;rejected(&context,-1);
    setup(1,0,1,0,0,0);context.pool.guest_address++;rejected(&context,-1);
    setup(1,0,1,0,0,0);context.pool.bytes=0x12AF;rejected(&context,-1);
    setup(1,0,1,0,0,0);context.pool.storage=(u8 *)(UINTPTR_MAX-15);rejected(&context,-1);
    setup(1,0,1,0,0,0);context.record=(u8 *)(UINTPTR_MAX-15);rejected(&context,-1);
    setup(1,0,1,0,0,0);context.reads=(const DreadzoneInventorySetupReadMemory *)(UINTPTR_MAX-7);rejected(&context,-1);
    setup(1,0,1,0,0,0);reads[2].storage=(const u8 *)(UINTPTR_MAX-7);rejected(&context,-1);
    setup(1,0,1,0,0,0);rejected((const DreadzoneInventorySetupContext *)(UINTPTR_MAX-7),-1);
    setup(1,0,1,0,0,0);context.read_count=65;rejected(&context,-1);
    setup(1,2,1,0,0,0);rejected(&context,-2);
    setup(1,1,1,0,4,0);rejected(&context,-4);
    setup(1,1,1,0,0xFFFFFFFF,0);rejected(&context,-4);
    setup(1,0,1,0,0,0);reads[0].bytes=0xA27;rejected(&context,-3);
    setup(1,0,1,0,0,0);reads[2].bytes=0x16A0;rejected(&context,-3);
    setup(1,0,1,0,0,0);regions[FLAG][0]=0;word(regions[TABLES]+0x398+2*0x28,0);rejected(&context,-5);
    setup(1,0,1,0,0,0);reads[2].storage=regions[POOL];rejected(&context,-1);
    setup(1,0,1,0,0,0);reads[2].guest_address=guests[POOL];rejected(&context,-1);
    setup(1,0,1,0,0,0);reads[2]=reads[0];rejected(&context,-1);
    setup(1,0,1,0,0,0);context.record=regions[POOL];rejected(&context,-1);
    /* Smaller mode1/existing domains need only the real mode word; no fake
     * table or configuration dependency is supplied to those completed paths. */
    setup(1,1,1,0,2,0);context.read_count=1;save();expected_result(1,1,0,2,0);
    CHECK(dreadzone_inventory_006909C8(&context)==0);verify();
    setup(1,0,1,0,0,1);context.read_count=1;save();expected_result(0,1,0,0,1);
    CHECK(dreadzone_inventory_006909C8(&context)==0);verify();
    {
        union { DreadzoneInventorySetupContext context;u8 bytes[0x12B0*4]; } alias_context;
        union { DreadzoneInventorySetupReadMemory views[3];u8 bytes[0x12B0*4]; } alias_reads;
        u8 before[0x12B0*4];
        memset(&alias_context,0xA6,sizeof(alias_context));
        memset(&alias_reads,0xB8,sizeof(alias_reads));
        setup(1,0,1,0,0,0);alias_context.context=context;
        alias_context.context.pool.storage=alias_context.bytes;
        memcpy(before,alias_context.bytes,sizeof(before));rejected(&alias_context.context,-1);
        CHECK(memcmp(before,alias_context.bytes,sizeof(before))==0);
        setup(1,0,1,0,0,0);memcpy(alias_reads.views,reads,sizeof(reads));
        context.reads=alias_reads.views;context.pool.storage=alias_reads.bytes;
        memcpy(before,alias_reads.bytes,sizeof(before));rejected(&context,-1);
        CHECK(memcmp(before,alias_reads.bytes,sizeof(before))==0);
    }
    rejected(0,-1);
    printf("Inventory setup: %u checks, %u failures. Real default grants, ammunition and slot consumption verified.\n",checks,failures);
    return failures?1:0;
}
