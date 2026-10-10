#include "dreadzone_equipment_selection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* The existing callback translation unit supplies the forward-copy helper.
 * Its callback is never invoked by UI. These are genuine unused storage. */
s32 D_002208A8,D_002208AC;
typedef struct UiFixture {
    u8 guard0[16],record[0x31F0],guard1[16],actor1[0x31F0],guard2[16];
    u8 pool[0x12B0],guard3[16],globals[8],guard4[16],definitions[32*0xB0],guard5[16];
    u8 query[2*0x54],guard6[16],recent[48],guard7[16],hud[2*0xB8],guard8[16],summary[0x80],guard9[16];
    u8 table[0x100],guard10[16];
    DreadzoneInventorySetupReadMemory reads[6];
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
    f->reads[4]=(DreadzoneInventorySetupReadMemory){f->table,0x3B10C0,sizeof(f->table)};
    v->record=f->record;v->record_guest=0x368F60;v->record_bytes=sizeof(f->record);
    v->pool=(DreadzoneInventoryMemory){f->pool,0x1D3F90,sizeof(f->pool)};
    v->reads=f->reads;v->read_count=5;
    f->equipment.mutable_views=f->writes;f->equipment.mutable_count=2;
    f->context.equipment=&f->equipment;f->context.extra_writes=f->extra;f->context.extra_write_count=1;
}
static void initialize(void) {
    size_t i;u32 j;
    for(i=0;i<offsetof(UiFixture,reads);++i) ((u8 *)&fixture)[i]=(u8)next();
    bind(&fixture);
    dreadzone_setup_store32(fixture.table+0x4C,0x6A9630);
    dreadzone_setup_store32(fixture.record+0x14,0x3B10C0);
    fixture.record[0x266D]=1;fixture.record[0x2FEA]=0;
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
    u32 seed,index,item,j;int reject;
    for(seed=0;seed<2048;++seed) {
        initialize();index=seed%2;item=seed%8<4?2:1+next()%31;
        fixture.record[0x2FEA]=(u8)index;
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
        if(seed&64) {fixture.equipment.inventory.record_guest=0x36C150;fixture.reads[3].guest_address=0x368F60;}
        if(seed%32==30) {
            dreadzone_setup_store32(fixture.record+0x14,0x330148+index*0xB8+0x98-0x4C);
            dreadzone_setup_store32(fixture.hud+index*0xB8+0x98,0x6A9630);
        }
        if(seed%32==31) {
            dreadzone_setup_store32(fixture.record+0x14,0x20C690+index*12-0x4C);
            dreadzone_setup_store32(fixture.recent+index*12,0x6A9630);
            fixture.query[index*0x54+0x1C]=1;
        }
        snapshot();
        reject=!dreadzone_setup_word(fixture.globals) && !dreadzone_setup_word(fixture.globals+4) &&
               (item==11 || item==12 || item==13 || item==16);
        if(!reject) {
            dreadzone_setup_store32(expected.record+0x2674,item);
            if(item!=1) oracle(&expected,index,item);
        }
        check(dreadzone_equipment_selection_00688578(&fixture.context,(s32)item)==reject,"real setter status");
        compare();
    }
}
static void refusal(s32 item,int status,const char *message) {
    snapshot();check(dreadzone_equipment_selection_00688578(&fixture.context,item)==status,message);compare();
}
static void boundaries(void) {
    u32 i;
    initialize();refusal(0,1,"actual item0 rejection");refusal(32,1,"actual item32 rejection");
    refusal(-1,1,"actual negative item rejection");refusal(INT32_MAX,1,"actual large item rejection");
    for(i=1;i<256;++i) {
        initialize();fixture.definitions[2*0xB0+0x2C]=(u8)i;
        refusal(2,-12,"unsupported category is atomic");
    }
    initialize();fixture.record[0x2FEA]=2;refusal(2,-5,"actual getter index2 rejects UI atomically");
    initialize();fixture.record[0x2FEA]=255;refusal(2,-5,"unsigned byte getter index255 rejects UI atomically");
    initialize();dreadzone_setup_store32(fixture.table+0x4C,0x690EA0);
    refusal(2,-5,"actual base getter minus1 refuses UI atomically");
    initialize();dreadzone_setup_store32(fixture.table+0x4C,0x6A9638);
    refusal(2,-13,"unknown real handler identity refuses");
    initialize();dreadzone_setup_store32(fixture.record+0x14,0xFFFFFFE0);
    refusal(2,-11,"wrapping vtable entry refuses");
    initialize();dreadzone_setup_store32(fixture.record+0x14,0x3B10C1);
    refusal(2,-10,"unaligned original LW virtual table refuses atomically");
    initialize();fixture.equipment.inventory.read_count=4;refusal(2,-11,"unmapped vtable entry refuses");
    initialize();dreadzone_setup_store32(fixture.record+0x14,0x368F60+0x2674-0x4C);
    dreadzone_setup_store32(fixture.record+0x2674,0x6A9630);
    refusal(2,-13,"ordered store destroys aliased virtual entry before dispatch");
    initialize();fixture.record[0x266D]=0;refusal(2,-14,"flag0 remains separate existing domain");
    initialize();fixture.equipment.inventory.record_guest=0x400000;refusal(2,-14,"nonactor guest record refuses");
    initialize();fixture.definitions[2*0xB0+0x28]=0;refusal(2,-3,"unsupported secondary definition after speculative record store");
    initialize();fixture.context.extra_write_count=0;refusal(2,-2,"missing final summary refuses original record store");
    initialize();fixture.reads[2].guest_address=0x31B800;dreadzone_setup_store32(fixture.recent,0);
    refusal(2,-2,"missing reached query refuses speculative writes");
    initialize();fixture.equipment.inventory.read_count=3;refusal(2,-11,"missing vtable despite valid UI context");
    initialize();dreadzone_setup_store32(fixture.hud+0x24,7);refusal(2,-3,"secondary state unsupported");
    initialize();fixture.extra[0].storage=(u8 *)(UINTPTR_MAX-1);refusal(2,-1,"wrapping host backing");
    initialize();fixture.extra[0].guest_address=0xFFFFFFF0;refusal(2,-1,"wrapping guest backing");
    initialize();fixture.extra[0].storage=fixture.record;refusal(2,-1,"overlapping host backing");
    initialize();fixture.extra[0].guest_address=0x330148;refusal(2,-1,"overlapping guest backing");
    initialize();fixture.context.extra_writes=fixture.equipment.mutable_views;refusal(2,-1,"descriptor array overlap");
    initialize(); {
        DreadzoneEquipmentContext *alias=(DreadzoneEquipmentContext *)fixture.summary;
        memcpy(alias,&fixture.equipment,sizeof(*alias));fixture.context.equipment=alias;
        refusal(2,-1,"equipment descriptor in writable backing");
    }
    initialize(); {
        DreadzoneEquipmentUiContext *alias=(DreadzoneEquipmentUiContext *)fixture.summary;
        memcpy(alias,&fixture.context,sizeof(*alias));snapshot();
        check(dreadzone_equipment_selection_00688578(alias,2)==-1,"UI descriptor in writable backing");compare();
    }
    initialize(); {
        u8 *large=(u8 *)malloc(16U*1024U*1024U);
        check(large!=0,"transaction bound fixture allocation");
        if(large) {
            fixture.reads[5]=(DreadzoneInventorySetupReadMemory){large,0x40000000,16U*1024U*1024U};
            fixture.equipment.inventory.read_count=6;
            refusal(2,-15,"bounded transaction refuses before large backing copy");
            free(large);
        }
    }
    check(dreadzone_equipment_selection_00688578((const DreadzoneEquipmentUiContext *)(UINTPTR_MAX-1),2)==-1,"wrapping context");
    initialize();for(i=0;i<256;++i) {
        fixture.record[0x2FEA]=(u8)i;
        check(func_L01_006A9630(fixture.record)==(s32)i,"all byte getter values");
        check(func_L01_00690EA0(fixture.record)==-1,"actual base getter return");
    }
}
int main(void) {
    D_002208A8=12345;D_002208AC=-23456;
    normal_cases();boundaries();printf("%u equipment selection checks, %u failures\n",checks,failures);
    return failures?1:0;
}
