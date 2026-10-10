#include "../dreadzone_actor_class.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
 fprintf(stderr,"Check failed at line %d: %s\n",__LINE__,#condition); exit(EXIT_FAILURE); } } while (0)
enum { CLASS_ID=0x23F7, TYPE_INDEX=3, TABLE_BASE=0x00260000,
 DEFINITION=0x003C0000, AUXILIARY=0x003C1000, ENVIRONMENT=0x003C2000 };
typedef struct Fixture {
 u8 tables[0x10000];
 u8 definition[0x100],auxiliary[0x20],environment[0x80];
 u8 actor_storage[0x122];
 DreadzoneClassView views[4];
 DreadzoneClassContext context;
} Fixture;
static void put16(u8 *p,u16 v) { memcpy(p,&v,2); }
static void put32(u8 *p,u32 v) { memcpy(p,&v,4); }
static u8 *actor(Fixture *f) { return f->actor_storage+0x11; }
static void setup(Fixture *f,int mapped) {
 memset(f,0,sizeof(*f));
 memset(f->actor_storage,0xB7,sizeof(f->actor_storage));
 f->views[0].guest_base=TABLE_BASE;f->views[0].bytes=sizeof(f->tables);f->views[0].storage=f->tables;
 f->views[1].guest_base=DEFINITION;f->views[1].bytes=sizeof(f->definition);f->views[1].storage=f->definition;
 f->views[2].guest_base=AUXILIARY;f->views[2].bytes=sizeof(f->auxiliary);f->views[2].storage=f->auxiliary;
 f->views[3].guest_base=ENVIRONMENT;f->views[3].bytes=sizeof(f->environment);f->views[3].storage=f->environment;
 f->context.views=f->views;f->context.view_count=4;
 f->tables[0xB590+CLASS_ID]=TYPE_INDEX;
 put16(f->tables+0xB2B0+TYPE_INDEX*2,mapped?CLASS_ID:0xFFFF);
 put32(f->tables+0xDD90+TYPE_INDEX*4,0x00612340);
 put32(f->tables+0xAE00+TYPE_INDEX*4,0x003F0000);
 put32(f->tables+0xAA40+TYPE_INDEX*4,DEFINITION);
 put32(actor(f)+0x58,0);
}
static void defaults(u8 expected[0x100],u8 preserved) {
 memset(expected,0,0x100);
 expected[0x2D]=preserved;
 expected[0x6D]=expected[0x2C]=expected[0x21]=expected[0x56]=0xFF;
 expected[0x64]=expected[0x65]=0xFF;
 expected[0x22]=TYPE_INDEX;
 put16(expected+0x6E,0xFFFF);
 expected[0xA7]=expected[0x23]=expected[0xA6]=0x80;
 put16(expected+0x2E,0x7F80);
 expected[0xA5]=expected[0xA4]=0x7F;
 put16(expected+0xBC,CLASS_ID);
 put32(expected+0x3C,0x00404040);
}
static void check_guards(const Fixture *f) {
 u32 i;
 for (i=0;i<0x11;i++) CHECK(f->actor_storage[i]==0xB7);
 for (i=0x111;i<sizeof(f->actor_storage);i++) CHECK(f->actor_storage[i]==0xB7);
}
static void failure(Fixture *f,int result,s32 id,s32 skip,u32 bytes) {
 u8 saved[0x122];
 memcpy(saved,f->actor_storage,sizeof(saved));
 CHECK(dreadzone_actor_initialize_class_slice(&f->context,actor(f),bytes,id,skip)==result);
 CHECK(memcmp(saved,f->actor_storage,sizeof(saved))==0);
}
static void check_mismatch(void) {
 Fixture f;
 u8 expected[0x100],saved[0x10000];
 setup(&f,0);defaults(expected,0xB7);
 put16(expected+0x34,0x45);put32(expected+0xA8,0x00612340);
 memcpy(saved,f.tables,sizeof(saved));
 CHECK(dreadzone_actor_initialize_class_slice(&f.context,actor(&f),0x100,CLASS_ID,0)==DREADZONE_CLASS_INITIALIZED);
 CHECK(memcmp(expected,actor(&f),sizeof(expected))==0);
 CHECK(memcmp(saved,f.tables,sizeof(saved))==0);check_guards(&f);
 setup(&f,0);actor(&f)[0x20]=0xFF;
 put32(f.tables+0xDD90+TYPE_INDEX*4,0);defaults(expected,0);
 put16(expected+0x34,0x47);
 CHECK(dreadzone_actor_initialize_class_slice(&f.context,actor(&f),0x100,CLASS_ID,1)==DREADZONE_CLASS_INITIALIZED);
 CHECK(memcmp(expected,actor(&f),sizeof(expected))==0);check_guards(&f);
}
static void check_mapped_minimal(void) {
 Fixture f;
 u8 expected[0x100];
 setup(&f,1);actor(&f)[0x20]=0xFF;defaults(expected,0);
 put32(f.definition+0x24,0x7FC12345);put32(f.definition+0x10,0xFFFF1234);
 put16(f.definition+0x44,0x100);f.definition[0xE]=0x80;f.definition[0xA]=1;f.definition[0x47]=0xA3;
 put32(expected+0x24,DEFINITION);put32(expected+0xA8,0x003F0000);
 put16(expected+0x34,0x140);expected[0x64]=0x80;
 put32(expected+0x98,0xFFFF1234);put32(expected+0x70,0x7FC12345);
 put16(expected+0x4C,1);put16(expected+0x36,0xA3);
 CHECK(dreadzone_actor_initialize_class_slice(&f.context,actor(&f),0x100,CLASS_ID,1)==DREADZONE_CLASS_INITIALIZED);
 CHECK(memcmp(expected,actor(&f),sizeof(expected))==0);check_guards(&f);
}
static void check_auxiliary_branches(void) {
 Fixture f;
 u8 expected[0x100],definition_saved[0x100],aux_saved[0x20];
 setup(&f,1);defaults(expected,0xB7);
 put32(f.tables+0xAE00+TYPE_INDEX*4,0);
 put16(f.definition+0x44,0x200);f.definition[0xE]=0x35;f.definition[0xA]=1;
 f.definition[0xF]=1;f.definition[6]=1;f.definition[0xC]=1;f.definition[0x47]=0x12;
 put32(f.definition+0x40,0xABCDEF00);put32(f.definition+0x48,AUXILIARY);
 f.auxiliary[0x10]=1;f.auxiliary[0x11]=0x80;f.auxiliary[0x12]=0x21;
 put32(expected+0x24,DEFINITION);put16(expected+0x34,0x652);
 expected[0x64]=0x35;expected[0x65]=0x6A;
 put16(expected+0x4C,1);expected[0x76]=expected[0x66]=0x18;
 put32(expected+0x60,0xABCDEF00);put32(expected+0x40,AUXILIARY);
 expected[0x6C]=0x21;expected[0x6D]=0x80;put16(expected+0x36,0x12);
 memcpy(definition_saved,f.definition,sizeof(definition_saved));memcpy(aux_saved,f.auxiliary,sizeof(aux_saved));
 CHECK(dreadzone_actor_initialize_class_slice(&f.context,actor(&f),0x100,CLASS_ID,1)==DREADZONE_CLASS_INITIALIZED);
 CHECK(memcmp(expected,actor(&f),sizeof(expected))==0);
 CHECK(memcmp(definition_saved,f.definition,sizeof(definition_saved))==0);
 CHECK(memcmp(aux_saved,f.auxiliary,sizeof(aux_saved))==0);check_guards(&f);
 setup(&f,1);defaults(expected,0xB7);
 put32(f.tables+0xAE00+TYPE_INDEX*4,0);put32(f.definition+0x48,AUXILIARY);
 f.definition[0xC]=1;f.auxiliary[0x10]=2;f.auxiliary[0x11]=0xFE;f.auxiliary[0x12]=7;
 put32(expected+0x24,DEFINITION);expected[0x64]=0;put16(expected+0x4C,1);
 put16(expected+0x34,0x40);put32(expected+0x40,AUXILIARY);
 expected[0x6C]=7;expected[0x6D]=0xFE;
 CHECK(dreadzone_actor_initialize_class_slice(&f.context,actor(&f),0x100,CLASS_ID,1)==DREADZONE_CLASS_INITIALIZED);
 CHECK(memcmp(expected,actor(&f),sizeof(expected))==0);check_guards(&f);
}
static void check_environment(void) {
 Fixture f;
 u8 expected[0x100];
 setup(&f,0);defaults(expected,0xB7);
 put16(expected+0x34,0x45);put32(expected+0xA8,0x00612340);
 f.context.environment=ENVIRONMENT;put16(f.environment,0x10);
 put32(f.environment+0x50,0xAABBCCDD);f.environment[0x5A]=0x93;
 put32(expected+0x38,0x00009393);put32(expected+0x3C,0xAABBCCDD);
 CHECK(dreadzone_actor_initialize_class_slice(&f.context,actor(&f),0x100,CLASS_ID,1)==DREADZONE_CLASS_INITIALIZED);
 CHECK(memcmp(expected,actor(&f),sizeof(expected))==0);check_guards(&f);
 memset(expected,0xB7,sizeof(expected));
 dreadzone_actor_pack_environment(expected,0x01234567,0x89,0xAB,0xCD);
 CHECK(expected[0x38]==0x89 && expected[0x39]==0xAB && expected[0x3A]==0xCD && expected[0x3B]==0);
 CHECK(expected[0x3C]==0x67 && expected[0x3D]==0x45 && expected[0x3E]==0x23 && expected[0x3F]==1);
 CHECK(expected[0x37]==0xB7 && expected[0x40]==0xB7);
}
static void check_unresolved_and_bounds(void) {
 Fixture f;
 DreadzoneClassView duplicated[2];
 setup(&f,1);put32(actor(&f)+0x58,0x003D0000);
 failure(&f,DREADZONE_CLASS_NEEDS_CLEANUP,CLASS_ID,0,0x100);
 f.definition[8]=4;
 failure(&f,DREADZONE_CLASS_NEEDS_MODEL_SETUP,CLASS_ID,1,0x100);
 f.definition[8]=0;put32(f.definition+0x48,0xFFFF0000);
 failure(&f,DREADZONE_CLASS_INVALID,CLASS_ID,1,0x100);
 put32(f.definition+0x48,0);f.context.environment=0xFFFFF000;
 failure(&f,DREADZONE_CLASS_INVALID,CLASS_ID,1,0x100);
 f.context.environment=0;
 failure(&f,DREADZONE_CLASS_INVALID,CLASS_ID,1,0xFF);
 failure(&f,DREADZONE_CLASS_INVALID,-1,1,0x100);
 duplicated[0]=duplicated[1]=f.views[0];f.context.views=duplicated;f.context.view_count=2;
 failure(&f,DREADZONE_CLASS_INVALID,CLASS_ID,1,0x100);
 f.context.views=f.views;f.context.view_count=4;f.views[3].guest_base=0xFFFFFFF0;
 failure(&f,DREADZONE_CLASS_INVALID,CLASS_ID,1,0x100);
 CHECK(dreadzone_actor_initialize_class_slice(NULL,actor(&f),0x100,CLASS_ID,1)==DREADZONE_CLASS_INVALID);
 CHECK(dreadzone_actor_initialize_class_slice(&f.context,NULL,0x100,CLASS_ID,1)==DREADZONE_CLASS_INVALID);
 check_guards(&f);
}
int main(void) {
 check_mismatch();check_mapped_minimal();check_auxiliary_branches();check_environment();check_unresolved_and_bounds();
 puts("Dreadzone actual actor class slice and packed-field checks passed.");return EXIT_SUCCESS;
}
