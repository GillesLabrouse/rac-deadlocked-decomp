#include "dreadzone_equipment.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef struct EquipmentFixture {
    u8 leading_guard[16],record[0x31F0],record_guard[16];
    u8 pool[0x12B0*4],pool_guard[16],globals[8],globals_guard[16];
    u8 flag[1],flag_guard[16],definitions[0xB0*32],definitions_guard[16];
    u8 markers[6],markers_guard[16],selectors[16],selectors_guard[16];
    u8 recent[48],recent_guard[16],hud[0xB8*4],trailing_guard[16];
    DreadzoneInventorySetupReadMemory reads[3];
    DreadzoneEquipmentMemory writes[4];
    DreadzoneEquipmentContext context;
} EquipmentFixture;
static EquipmentFixture fixture,expected;
static unsigned long long checks;
static unsigned failures;
static u32 random_state=0x690B60;
static u32 random_word(void) {
    random_state^=random_state<<13;random_state^=random_state>>17;random_state^=random_state<<5;
    return random_state;
}
static void check(int okay,const char *message) {
    ++checks;
    if(!okay && failures++<12) printf("FAIL: %s\n",message);
}
static void bytes(const u8 *a,const u8 *b,size_t n,const char *message) {
    /* One full-image assertion covers every byte without inflating the count
     * of assertions or repeating millions of scalar test calls. */
    check(memcmp(a,b,n)==0,message);
}
static void fill(u8 *p,size_t n) { size_t i;for(i=0;i<n;++i) p[i]=(u8)random_word(); }
static void bind(EquipmentFixture *f) {
    DreadzoneInventorySetupContext *v=&f->context.inventory;
    f->reads[0]=(DreadzoneInventorySetupReadMemory){f->globals,0x21E824,8};
    f->reads[1]=(DreadzoneInventorySetupReadMemory){f->flag,0x171B66,1};
    f->reads[2]=(DreadzoneInventorySetupReadMemory){f->definitions,0x3BF460,sizeof(f->definitions)};
    f->writes[0]=(DreadzoneEquipmentMemory){f->markers,0x171B67,sizeof(f->markers)};
    f->writes[1]=(DreadzoneEquipmentMemory){f->selectors,0x220810,sizeof(f->selectors)};
    f->writes[2]=(DreadzoneEquipmentMemory){f->recent,0x20C690,sizeof(f->recent)};
    f->writes[3]=(DreadzoneEquipmentMemory){f->hud,0x330148,sizeof(f->hud)};
    v->record=f->record;v->record_guest=0x368F60;v->record_bytes=sizeof(f->record);
    v->pool=(DreadzoneInventoryMemory){f->pool,0x1D3F90,sizeof(f->pool)};
    v->reads=f->reads;v->read_count=3;
    f->context.mutable_views=f->writes;f->context.mutable_count=4;
}
static void initialize(u32 index) {
    fill((u8 *)&fixture,offsetof(EquipmentFixture,reads));bind(&fixture);
    dreadzone_setup_store32(fixture.record+0x26E4,0x1D3F90+index*0x12B0);
    dreadzone_setup_store32(fixture.record+0x2318,0);
    fixture.record[0x266D]=1;
    dreadzone_setup_store32(fixture.globals,0);dreadzone_setup_store32(fixture.globals+4,1);
}
static void copy_expected(void) { expected=fixture;bind(&expected); }
static void compare(void) {
    bytes((u8 *)&fixture,(u8 *)&expected,offsetof(EquipmentFixture,reads),"complete mapped images");
}
static u8 *selected(EquipmentFixture *f) {
    return f->pool+dreadzone_setup_word(f->record+0x26E4)-0x1D3F90;
}
static void deletion_oracle(u8 *inventory,u32 item,int restore) {
    u32 k;
    for(k=0;k<10;++k) {
        u8 *slot=inventory+0xA44+item*0x44+k*4;
        u32 value=*slot;
        if(value) { *slot=0;if(restore && value<=8) ++inventory[0x20+value]; }
    }
}
static void maintenance_oracle(EquipmentFixture *f) {
    u32 i,j;
    for(i=0;i<4;++i) {
        u32 item=f->markers[i+2];u8 *inventory;
        if(!item) continue;
        if(dreadzone_setup_word(f->selectors+i*4)==item) dreadzone_setup_store32(f->selectors+i*4,2);
        if(dreadzone_setup_word(f->record+0x24B0)==item) dreadzone_setup_store32(f->record+0x24B0,2);
        if(f->markers[i]==item) f->markers[i]=2;
        if(dreadzone_setup_word(f->record+0x2318)==item) {
            s32 category=(s32)(f->definitions[2*0xB0+0x2C]^0x80)-0x80;
            dreadzone_setup_store32(f->record+0x2674+4*category,2);
            dreadzone_setup_store32(f->record+0x268C,0);
        }
        for(j=0;j<3;++j) dreadzone_setup_store32(f->recent+i*12+j*4,0);
        dreadzone_setup_store32(f->hud+0x98+i*0xB8,dreadzone_setup_word(f->hud+0x98+i*0xB8)|13);
        if(dreadzone_setup_word(f->record+0x268C)==item) dreadzone_setup_store32(f->record+0x268C,0);
        if(dreadzone_setup_word(f->record+0x2674)==item) {
            dreadzone_setup_store32(f->record+0x2674,2);dreadzone_setup_store32(f->record+0x268C,0);
        }
        inventory=selected(f);deletion_oracle(inventory,item,1);
        dreadzone_setup_store32(inventory+0xA34+item*0x44,0);
        dreadzone_setup_store16(inventory+0xA30+item*0x44,0xFFFF);
    }
}
static void normal_cases(void) {
    u32 seed,item,k,i,index;
    for(seed=0;seed<384;++seed) {
        index=seed%4;initialize(index);
        for(i=0;i<4;++i) {
            item=(seed%7==0)?0:1+random_word()%31;fixture.markers[i+2]=(u8)item;
            if(seed&1) dreadzone_setup_store32(fixture.selectors+i*4,item);
            for(k=0;k<10;++k) selected(&fixture)[0xA44+item*0x44+k*4]=(u8)(random_word()%12);
        }
        if(seed&2) fixture.markers[0]=fixture.markers[2];
        if(seed&4) fixture.markers[1]=fixture.markers[3];
        if(seed&8) dreadzone_setup_store32(fixture.record+0x2674,fixture.markers[2]);
        if(seed&16) dreadzone_setup_store32(fixture.record+0x24B0,fixture.markers[3]);
        if(seed&32) dreadzone_setup_store32(fixture.record+0x268C,fixture.markers[4]);
        if(seed%3==0 && fixture.markers[2]) {
            fixture.record[0x266D]=0;
            fixture.definitions[2*0xB0+0x2C]=(u8)(seed%27);
            dreadzone_setup_store32(fixture.record+0x2318,fixture.markers[2]);
        }
        /* Real fresh-actor flag1 cases retain state2318=0 and execute removals. */
        copy_expected();maintenance_oracle(&expected);
        check(dreadzone_equipment_00690B60(&fixture.context)==0,"maintenance result");compare();
    }
    for(item=0;item<32;++item) for(i=0;i<3;++i) {
        initialize(item%4);
        for(k=0;k<10;++k) selected(&fixture)[0xA44+item*0x44+k*4]=(u8)(k==9?255:k);
        memset(selected(&fixture)+0x21,255,8);copy_expected();
        deletion_oracle(selected(&expected),item,i!=0);
        check(dreadzone_equipment_006C0A58(&fixture.context,(s32)item,(s32)i)==0,"deletion result");compare();
    }
    for(item=1;item<32;++item) for(i=0;i<256;++i) {
        s32 category=(s32)(i^0x80)-0x80;
        initialize(0);fixture.record[0x266D]=0;fixture.definitions[item*0xB0+0x2C]=(u8)i;copy_expected();
        dreadzone_setup_store32(expected.record+0x2674+4*category,item);
        check(dreadzone_equipment_00688578(&fixture.context,(s32)item)==0,"signed category setter");compare();
    }
    initialize(0);copy_expected();memset(expected.markers+2,0,4);
    check(dreadzone_equipment_00690C88(&fixture.context)==0,"marker clear result");compare();
}
static void refusal(int entry,s32 item,int result,const char *message) {
    int got;
    copy_expected();
    if(entry==0) got=dreadzone_equipment_00690B60(&fixture.context);
    else if(entry==1) got=dreadzone_equipment_00688578(&fixture.context,item);
    else if(entry==2) got=dreadzone_equipment_006C0A58(&fixture.context,item,1);
    else got=dreadzone_equipment_00690C88(&fixture.context);
    check(got==result,message);compare();
}
static void boundaries(void) {
    u32 i;
    initialize(0);fixture.definitions[2*0xB0+0x2C]=254;copy_expected();
    dreadzone_setup_store32(expected.record+0x266C,2);
    check(dreadzone_equipment_00688578(&fixture.context,2)==0,"real category store clears flag before read");compare();
    initialize(0);memset(fixture.markers,0,sizeof(fixture.markers));
    fixture.markers[2]=7;fixture.definitions[2*0xB0+0x2C]=254;
    dreadzone_setup_store32(fixture.record+0x2318,7);copy_expected();maintenance_oracle(&expected);
    check(dreadzone_equipment_00690B60(&fixture.context)==0,"maintenance real flag alias clears before UI guard");compare();
    initialize(0);fixture.markers[2]=7;fixture.definitions[2*0xB0+0x2C]=0;
    dreadzone_setup_store32(fixture.record+0x2318,7);
    refusal(0,0,-4,"actual flag1 selection requires UI");
    initialize(0);fixture.record[0x266D]=0;fixture.markers[2]=7;fixture.definitions[2*0xB0+0x2C]=28;
    dreadzone_setup_store32(fixture.record+0x2318,7);refusal(0,0,-6,"inventory field alias refused");
    initialize(0);fixture.definitions[2*0xB0+0x2C]=0;
    refusal(1,2,-4,"standalone flag1 selection requires UI");
    initialize(0);fixture.record[0x266D]=0;dreadzone_setup_store32(fixture.globals+4,0);
    for(i=11;i<=16;++i) if(i<14 || i==16) refusal(1,(s32)i,1,"actual mode0 level0 predicate");
    refusal(1,0,1,"invalid selection0");refusal(1,32,1,"invalid selection32");
    initialize(0);refusal(2,-1,-5,"deletion negative item");refusal(2,32,-5,"deletion item32");
    initialize(0);fixture.markers[2]=32;refusal(0,0,-5,"unsupported marker32");
    initialize(0);dreadzone_setup_store32(fixture.record+0x26E4,0x1D3F91);refusal(2,2,-3,"unaligned inventory pointer");
    for(i=0;i<4;++i) {
        initialize(0);fixture.context.mutable_count=i;fixture.markers[2]=7;
        refusal(0,0,-2,"missing actual mutable view");
    }
    initialize(0);fixture.context.inventory.read_count=0;fixture.record[0x266D]=0;
    refusal(1,2,-2,"missing actual mode");
    initialize(0);fixture.reads[0].bytes=4;fixture.record[0x266D]=0;
    refusal(1,2,-2,"missing level after actual mode0 read");
    initialize(0);fixture.context.inventory.read_count=2;fixture.record[0x266D]=0;
    refusal(1,2,-2,"missing real definition row");
    initialize(0);fixture.writes[0].guest_address=0xFFFFFFFF;refusal(3,0,-1,"wrapping guest view");
    initialize(0);fixture.writes[0].storage=(u8 *)(UINTPTR_MAX-2);refusal(3,0,-1,"wrapping host backing");
    initialize(0);fixture.context.inventory.record=(u8 *)(UINTPTR_MAX-2);refusal(3,0,-1,"wrapping host actor");
    initialize(0);fixture.context.inventory.pool.storage=(u8 *)(UINTPTR_MAX-2);refusal(3,0,-1,"wrapping host inventory");
    initialize(0);fixture.context.mutable_views=(const DreadzoneEquipmentMemory *)(UINTPTR_MAX-2);
    refusal(3,0,-1,"wrapping descriptor range");
    initialize(0);fixture.context.inventory.record_bytes=0x31EF;refusal(3,0,-1,"short record");
    initialize(0);fixture.writes[0].storage=fixture.pool;refusal(3,0,-1,"host backing overlap");
    initialize(0);fixture.writes[0].guest_address=fixture.context.inventory.record_guest;refusal(3,0,-1,"guest backing overlap");
    initialize(0);fixture.context.mutable_views=(const DreadzoneEquipmentMemory *)fixture.record;
    /* The descriptor object itself is readable but inside backing; construct
     * legitimate descriptors there so rejection observes the host alias. */
    memcpy(fixture.record,fixture.writes,sizeof(fixture.writes));refusal(3,0,-1,"descriptor object inside writable backing");
    initialize(0);fixture.context.inventory.reads=(const DreadzoneInventorySetupReadMemory *)&fixture.context;
    refusal(3,0,-1,"descriptor object overlaps context");
    initialize(0);fixture.context.mutable_views=(const DreadzoneEquipmentMemory *)fixture.reads;
    refusal(3,0,-1,"read and write descriptor arrays overlap");
    initialize(0);
    {
        DreadzoneEquipmentContext *alias=(DreadzoneEquipmentContext *)fixture.record;
        memcpy(alias,&fixture.context,sizeof(*alias));copy_expected();
        check(dreadzone_equipment_00690B60(alias)==-1,"context object inside writable backing");compare();
    }
    check(dreadzone_equipment_00690B60((const DreadzoneEquipmentContext *)(UINTPTR_MAX-2))==-1,"wrapping context before dereference");
    check(dreadzone_equipment_00688578((const DreadzoneEquipmentContext *)(UINTPTR_MAX-2),2)==-1,"wrapping setter context");
    check(dreadzone_equipment_006C0A58((const DreadzoneEquipmentContext *)(UINTPTR_MAX-2),2,1)==-1,"wrapping deletion context");
    check(dreadzone_equipment_00690C88((const DreadzoneEquipmentContext *)(UINTPTR_MAX-2))==-1,"wrapping marker context");
}
int main(void) {
    normal_cases();boundaries();printf("%llu equipment checks, %u failures\n",checks,failures);
    return failures?1:0;
}
