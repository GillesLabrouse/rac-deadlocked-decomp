#include "dreadzone_movement_basis.h"
#include <fenv.h>
#include <stdio.h>
#include <string.h>
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
#include <xmmintrin.h>
#endif
/* Synthetic expected words independently derived by decoding actual EE/MMI
 * and the defined XY data cone of C60. No game memory or assets are embedded.
 * These fixtures validate the declared approximation, not physical VU banks. */
static const struct { uint32_t angles[3], matrices[32]; } fixtures[] = {
    {{0x00000000U,0x00000000U,0x00000000U},
     {0x3F800000U,0x00000000U,0x80000000U,0x00000000U,0x00000000U,0x3F800000U,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U,0x3F800000U,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U,0x00000000U,0x00000000U,0x80000000U,0x00000000U,0x3F800000U,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U}},
    {{0x80000000U,0x00000000U,0x80000000U},
     {0x3F800000U,0x00000000U,0x80000000U,0x00000000U,0x00000000U,0x3F800000U,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U,0x3F800000U,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U,0x00000000U,0x00000000U,0x80000000U,0x00000000U,0x3F800000U,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U}},
    {{0x3E800000U,0xBF000000U,0x3F400000U},
     {0x3F2461CEU,0x3F192343U,0x3EF57744U,0x00000000U,0xBF3F4ACEU,0x3F20CAA4U,0x3E5E5405U,0x00000000U,0xBE2F5AD9U,0xBEFECD18U,0x3F59AD4DU,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U,0x3F2461CEU,0xBF3F4ACEU,0xBE2F5AD9U,0x00000000U,0x3F192343U,0x3F20CAA4U,0xBEFECD18U,0x00000000U,0x3EF57744U,0x3E5E5405U,0x3F59AD4DU,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U}},
    {{0xBF800000U,0x3F000000U,0xC0000000U},
     {0xBEBAFBE9U,0xBF4C48A6U,0xBEF57744U,0x00000000U,0x3F28BFEFU,0x3E1164CDU,0xBF3D0BBAU,0x00000000U,0x3F284826U,0xBF15F16EU,0x3EF2C517U,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U,0xBEBAFBE9U,0x3F28BFEFU,0x3F284826U,0x00000000U,0xBF4C48A6U,0x3E1164CDU,0xBF15F16EU,0x00000000U,0xBEF57744U,0xBF3D0BBAU,0x3EF2C517U,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U}},
    {{0x3FC00000U,0xBFC00000U,0x40200000U},
     {0xBD681F87U,0x3D2D66B4U,0x3F7F5BD5U,0x00000000U,0x3F413A9FU,0xBF26F32FU,0x3D9081C9U,0x00000000U,0x3F274BE2U,0x3F41C4C3U,0x3BA3F67DU,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U,0xBD681F87U,0x3F413A9FU,0x3F274BE2U,0x00000000U,0x3D2D66B4U,0xBF26F32FU,0x3F41C4C3U,0x00000000U,0x3F7F5BD5U,0x3D9081C9U,0x3BA3F67DU,0x00000000U,0x00000000U,0x00000000U,0x00000000U,0x3F800000U}}
};
static int require(int ok, const char *reason) {
    if (!ok) { fprintf(stderr, "Movement basis failure: %s\n", reason); return 0; }
    return 1;
}
int main(void) {
    uint8_t owner[0xC4], before[sizeof(owner)];
    const uint32_t unsafe[] = {1,0x00800000,0x7FC00001,0x7F800000,0xFF800000,0x40800000,0xC0800000};
    size_t case_index, shift, angle, bad;
    int original_round = fegetround();
    if (!require(fesetround(FE_TONEAREST)==0,"rounding setup")) return 1;
    for (case_index=0;case_index<sizeof(fixtures)/sizeof(fixtures[0]);++case_index)
        for (shift=0;shift<8;++shift) {
            uint8_t *record=owner+shift;
            memset(owner,0xA5,sizeof(owner));
            memcpy(record+0xB0,fixtures[case_index].angles,12);
            memcpy(before,owner,sizeof(owner));
            if (!require(dreadzone_movement_basis_before_locomotion(record,0xBC)==1,"supported fixture")) return 1;
            if (!require(memcmp(record+0x20,fixtures[case_index].matrices,128)==0,"raw-reference words including final transpose column")) return 1;
            memcpy(before+shift+0x20,record+0x20,128);
            if (!require(memcmp(before,owner,sizeof(owner))==0,"exclusive output footprint")) return 1;
        }
    for (angle=0;angle<3;++angle) for (bad=0;bad<sizeof(unsafe)/sizeof(unsafe[0]);++bad) {
        memset(owner,0xA5,sizeof(owner));
        memcpy(owner+1+0xB0,fixtures[2].angles,12);
        memcpy(owner+1+0xB0+angle*4,&unsafe[bad],4);
        memcpy(before,owner,sizeof(owner));
        if (!require(dreadzone_movement_basis_before_locomotion(owner+1,0xBC)==-2 && memcmp(before,owner,sizeof(owner))==0,"atomic unsafe angle")) return 1;
    }
    if (!require(dreadzone_movement_basis_before_locomotion(NULL,0xBC)==-1,"null backing")) return 1;
    if (!require(dreadzone_movement_basis_before_locomotion(owner+1,0xBB)==-1 && memcmp(before,owner,sizeof(owner))==0,"short backing")) return 1;
    if (!require(dreadzone_movement_basis_before_locomotion((uint8_t *)(UINTPTR_MAX-15),0xBC)==-1,"numeric pointer wrap")) return 1;
    memcpy(owner+1+0xB0,fixtures[2].angles,12); memcpy(before,owner,sizeof(owner));
    if (!require(fesetround(FE_DOWNWARD)==0,"rounding change")) return 1;
    if (!require(dreadzone_movement_basis_before_locomotion(owner+1,0xBC)==-2 && memcmp(before,owner,sizeof(owner))==0,"atomic rounding refusal")) return 1;
    if (!require(fesetround(FE_TONEAREST)==0,"rounding restore")) return 1;
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    {
        const unsigned previous=_mm_getcsr();
        const unsigned modes[]={0x8000U,0x40U,0x8040U};
        size_t mode;
        for (mode=0;mode<sizeof(modes)/sizeof(modes[0]);++mode) {
            int status;
            _mm_setcsr((previous&~0x8040U)|modes[mode]);
            status=dreadzone_movement_basis_before_locomotion(owner+1,0xBC);
            _mm_setcsr(previous);
            if (!require(status==-2 && memcmp(before,owner,sizeof(owner))==0,"atomic FTZ/DAZ environment refusal")) return 1;
        }
    }
#endif
    if (!require(fesetround(original_round)==0,"original rounding restore")) return 1;
    return 0;
}
