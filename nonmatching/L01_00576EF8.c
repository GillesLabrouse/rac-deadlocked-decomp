/* cflags: -mno-split-addresses */
#include "common.h"

/* Actual L01 00576EF8..00577020, 296 bytes. Definition is a guest pointer;
 * texture records and byte remap are consumed by the real relocator. */
#ifndef DEADLOCKED_NATIVE
void func_L01_005B5598(s32,s32);
void func_L01_00576BC0(u8 *,u8 *,u8 *,s32);
extern u32 D_00222D18,D_00222D1C,D_0026AA40[],D_0026E830[];
extern u8 D_0026B590[];
extern s16 D_0026B2B0[],D_0026B490[];
void func_L01_00576EF8(u8 *definition,u8 *textures,u8 *remap,s32 objectClass) {
    u32 slot;
    if (definition==NULL) {
        func_L01_005B5598(objectClass,1);
        slot=D_00222D1C;
        D_0026B590[objectClass]=(u8)slot;
        D_0026B490[slot]=(s16)objectClass;
        D_00222D1C=slot+1;
    } else {
        slot=D_00222D18;
        D_0026B590[objectClass]=(u8)slot;
        D_0026B2B0[slot]=(s16)objectClass;
        D_0026AA40[slot]=(u32)definition;
        D_0026E830[slot]=(u32)definition[0x2D]<<10;
        if (definition[0x2D]==0xFF) D_0026E830[slot]=0x00100000;
        func_L01_00576BC0(definition,textures,remap,objectClass);
        func_L01_005B5598(objectClass,0);
        D_00222D18++;
    }
}
#endif
