#include "dreadzone_equipment_ui.h"
#include <stdio.h>
#include <string.h>
/* The existing callback translation unit supplies the forward-copy helper.
 * Its callback is never invoked by UI. These are genuine unused storage. */
s32 D_002208A8,D_002208AC;
typedef struct UiFixture {
    u8 guard0[16],record[0x31F0],guard1[16],actor1[0x31F0],guard2[16];
    u8 pool[0x12B0],guard3[16],globals[8],guard4[16],definitions[32*0xB0],guard5[16];
    u8 query[2*0x54],guard6[16],recent[48],guard7[16],hud[2*0xB8],guard8[16],summary[0x80],guard9[16];
    DreadzoneInventorySetupReadMemory reads[4];
    DreadzoneEquipmentMemory writes[2],extra[1];
    DreadzoneEquipmentContext equipment;
    DreadzoneEquipmentUiContext context;
} UiFixture;
static UiFixture fixture,expected;
static unsigned checks,failures;
static u32 random_state=0x5E93D8;
static u32 next(void) {
    random_state^=random_state<<13;random_state^=random_state>>17;random_state^=random_state<<5;
    return random_state;
}
static void check(int okay,const char *message) {
    ++checks;if(!okay && failures++<10) printf("FAIL: %s\n",message);
}
static void bind(UiFixture *f) {
    DreadzoneInventorySetupContext *v=&f->equipment.inventory;
    f->reads[0]=(DreadzoneInventorySetupReadMemory){f->globals,0x21E824,sizeof(f->globals)};
    f->reads[1]=(DreadzoneInventorySetupReadMemory){f->definitions,0x3BF460,sizeof(f->definitions)};
    f->reads[2]=(DreadzoneInventorySetupReadMemory){f->query,0x31B2F8,sizeof(f->query)};
    f->reads[3]=(DreadzoneInventorySetupReadMemory){f->actor1,0x36C150,sizeof(f->actor1)};
    f->writes[0]=(DreadzoneEquipmentMemory){f->recent,0x20C690,sizeof(f->recent)};
    f->writes[1]=(DreadzoneEquipmentMemory){f->hud,0x330148,sizeof(f->hud)};
    f->extra[0]=(DreadzoneEquipmentMemory){f->summary,0x3312E0,sizeof(f->summary)};
    v->record=f->record;v->record_guest=0x368F60;v->record_bytes=sizeof(f->record);
    v->pool=(DreadzoneInventoryMemory){f->pool,0x1D3F90,sizeof(f->pool)};
    v->reads=f->reads;v->read_count=4;
    f->equipment.mutable_views=f->writes;f->equipment.mutable_count=2;
    f->context.equipment=&f->equipment;f->context.extra_writes=f->extra;f->context.extra_write_count=1;
}
static void initialize(void) {
    size_t i;u32 j;
    for(i=0;i<offsetof(UiFixture,reads);++i) ((u8 *)&fixture)[i]=(u8)next();
    bind(&fixture);
    dreadzone_setup_store32(fixture.record+0x26E4,0x1D3F90);
    dreadzone_setup_store32(fixture.globals,0);dreadzone_setup_store32(fixture.globals+4,1);
    for(j=0;j<2;++j) {
        dreadzone_setup_store32(fixture.hud+j*0xB8+0x24,0);
        dreadzone_setup_store32(fixture.hud+j*0xB8+0x28,0);
        fixture.query[j*0x54+0x1C]=0;
    }
    for(j=0;j<12;++j) dreadzone_setup_store32(fixture.recent+j*4,next()%32);
    for(j=0;j<32;++j) {fixture.definitions[j*0xB0+0x2C]=0;fixture.definitions[j*0xB0+0x28]=(u8)(next()%2);}
    fixture.definitions[2*0xB0+0x28]=1;
}
static void snapshot(void) {expected=fixture;bind(&expected);}
static void compare(void) {
    check(memcmp(&fixture,&expected,offsetof(UiFixture,reads))==0,"complete image and every backing guard");
    check(D_002208A8==12345 && D_002208AC==-23456,"unused callback globals retained");
}
static void oracle(UiFixture *f,u32 index,u32 item) {
    u8 *queue=f->recent+index*12;
    u32 current=dreadzone_setup_word(queue),i,other=1-index;
    if(current!=item && !f->query[index*0x54+0x1C] && f->definitions[current*0xB0+0x28]) {
        u32 position=2;
        for(i=0;i<3;++i) {
            u32 value=dreadzone_setup_word(queue+i*4);
            if(!value || value==item) {position=i;break;}
        }
        for(i=position;i>0;--i) dreadzone_setup_store32(queue+i*4,dreadzone_setup_word(queue+(i-1)*4));
    }
    dreadzone_setup_store32(queue,item); /* Query gates shuffle, never this store. */
    dreadzone_setup_store32(f->hud+index*0xB8+0x98,dreadzone_setup_word(f->hud+index*0xB8+0x98)|9);
    if(dreadzone_setup_word(f->globals)!=0 || dreadzone_setup_word(f->globals+4)!=1) return;
    if(dreadzone_setup_word(f->recent+other*12)==item) {
        u32 copy[3],count=0;
        for(i=0;i<3;++i) {copy[i]=dreadzone_setup_word(f->recent+other*12+i*4);if(copy[i]) ++count;else break;}
        for(i=0;i<count;++i) dreadzone_setup_store32(f->recent+other*12+i*4,copy[(i+1)%count]);
    }
    for(i=0;i<2;++i) dreadzone_setup_store32(f->hud+i*0xB8+0x98,dreadzone_setup_word(f->hud+i*0xB8+0x98)|12);
    dreadzone_setup_store32(f->summary+0x10,dreadzone_setup_word(f->summary+0x10)|1);
    dreadzone_setup_store32(f->summary+0x74,dreadzone_setup_word(f->summary+0x74)|1);
}
static void normal_cases(void) {
    u32 seed,index,item,j;
    for(seed=0;seed<1024;++seed) {
        initialize();index=seed%2;item=seed%8<4?2:2+next()%30;
        fixture.definitions[item*0xB0+0x28]=1;
        fixture.query[index*0x54+0x1C]=(u8)(seed%3==0);
        if(seed%4==0) memset(fixture.recent,0,sizeof(fixture.recent));
        if(seed%4==1) {
            for(j=0;j<2;++j) dreadzone_setup_store32(fixture.recent+j*12,item);
            if(seed&16) dreadzone_setup_store32(fixture.recent+(1-index)*12+8,0);
        }
        if(seed%8==6) dreadzone_setup_store32(fixture.globals,1);
        if(seed%8==7) dreadzone_setup_store32(fixture.globals+4,0);
        if(seed&32) dreadzone_setup_store32(fixture.actor1+0x2674,0);
        snapshot();oracle(&expected,index,item);
        if(seed&1) check(dreadzone_equipment_ui_005D7558(&fixture.context,(s32)index,(s32)item,1)==0,"wrapper complete result");
        else check(dreadzone_equipment_ui_005E93D8(&fixture.context,(s32)index,(s32)item,1)==0,"UI complete result");
        compare();
    }
}
static void refusal(s32 index,s32 item,s32 force,int status,const char *message) {
    snapshot();check(dreadzone_equipment_ui_005E93D8(&fixture.context,index,item,force)==status,message);compare();
}
static void boundaries(void) {
    u32 i;
    initialize();refusal(-1,2,1,-5,"negative index");refusal(2,2,1,-5,"index2");
    refusal(0,0,1,-5,"item0");refusal(0,32,1,-5,"item32");refusal(0,2,0,-4,"reentrant force0");
    refusal(0,2,2,-4,"unsupported force2");refusal(0,1,1,1,"actual item1 rejection");
    for(i=0;i<256;++i) if(i) {
        initialize();fixture.definitions[2*0xB0+0x2C]=(u8)i;refusal(0,2,1,1,"every nonzero signed category rejects");
    }
    initialize();fixture.definitions[2*0xB0+0x28]=0;refusal(0,2,1,-3,"unsupported secondary type");
    for(i=0;i<2;++i) {
        initialize();dreadzone_setup_store32(fixture.hud+i*0xB8+0x24,7);refusal(0,2,1,-3,"secondary24 refuses atomically");
        initialize();dreadzone_setup_store32(fixture.hud+i*0xB8+0x28,7);refusal(0,2,1,-3,"secondary28 refuses atomically");
    }
    initialize();fixture.equipment.inventory.read_count=1;refusal(0,2,1,-2,"missing definition");
    initialize();fixture.reads[0].bytes=4;refusal(0,2,1,-2,"missing level before dirty effects");
    initialize();fixture.reads[2].guest_address=0x31B800;dreadzone_setup_store32(fixture.recent,0);
    refusal(0,2,1,-2,"missing query before queue write");
    initialize();fixture.context.extra_write_count=0;refusal(0,2,1,-2,"missing summary before queue write");
    initialize();fixture.equipment.mutable_count=1;refusal(0,2,1,-2,"missing state row");
    initialize();fixture.equipment.inventory.read_count=3;
    dreadzone_setup_store32(fixture.recent,2);dreadzone_setup_store32(fixture.recent+12,2);
    refusal(0,2,1,-2,"missing reached counterpart actor before rotation");
    initialize();fixture.writes[0].bytes=24;
    dreadzone_setup_store32(fixture.recent,2);dreadzone_setup_store32(fixture.recent+12,2);
    dreadzone_setup_store32(fixture.recent+16,3);dreadzone_setup_store32(fixture.recent+20,7);
    refusal(0,2,1,-2,"odd-count real paired-word lookahead unavailable");
    initialize();dreadzone_setup_store32(fixture.recent,0xFFFFFFFF);refusal(0,2,1,-5,"unbound old definition index");
    initialize();fixture.extra[0].storage=(u8 *)(UINTPTR_MAX-1);refusal(0,2,1,-1,"wrapping extra host view");
    initialize();fixture.context.equipment=(const DreadzoneEquipmentContext *)(UINTPTR_MAX-1);
    refusal(0,2,1,-1,"wrapping borrowed equipment context");
    initialize();fixture.equipment.inventory.reads=(const DreadzoneInventorySetupReadMemory *)(UINTPTR_MAX-1);
    refusal(0,2,1,-1,"wrapping borrowed read descriptors");
    initialize();fixture.context.extra_writes=(const DreadzoneEquipmentMemory *)(UINTPTR_MAX-1);
    refusal(0,2,1,-1,"wrapping extra descriptor array");
    initialize();fixture.extra[0].guest_address=0xFFFFFFF0;refusal(0,2,1,-1,"wrapping extra guest view");
    initialize();fixture.extra[0].storage=fixture.record;refusal(0,2,1,-1,"host alias with actor");
    initialize();fixture.extra[0].guest_address=0x330148;refusal(0,2,1,-1,"guest alias with state");
    initialize();fixture.context.extra_writes=(const DreadzoneEquipmentMemory *)&fixture.context;
    refusal(0,2,1,-1,"descriptor aliases context");
    initialize();fixture.context.extra_writes=fixture.equipment.mutable_views;
    refusal(0,2,1,-1,"descriptor arrays overlap");
    initialize();
    {
        DreadzoneEquipmentContext *alias=(DreadzoneEquipmentContext *)fixture.summary;
        memcpy(alias,&fixture.equipment,sizeof(*alias));fixture.context.equipment=alias;snapshot();
        check(dreadzone_equipment_ui_005E93D8(&fixture.context,0,2,1)==-1,"borrowed descriptor inside writable summary");compare();
    }
    initialize();
    {
        DreadzoneEquipmentUiContext *alias=(DreadzoneEquipmentUiContext *)fixture.summary;
        memcpy(alias,&fixture.context,sizeof(*alias));snapshot();
        check(dreadzone_equipment_ui_005E93D8(alias,0,2,1)==-1,"UI descriptor inside writable summary");compare();
    }
    check(dreadzone_equipment_ui_005E93D8((const DreadzoneEquipmentUiContext *)(UINTPTR_MAX-1),0,2,1)==-1,"wrapping UI context");
    check(dreadzone_equipment_ui_005D7558((const DreadzoneEquipmentUiContext *)(UINTPTR_MAX-1),0,2,1)==-1,"wrapping wrapper context");
}
int main(void) {
    D_002208A8=12345;D_002208AC=-23456;
    normal_cases();boundaries();printf("%u equipment UI checks, %u failures\n",checks,failures);
    return failures?1:0;
}
