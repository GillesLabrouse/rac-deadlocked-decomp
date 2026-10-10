#include "dreadzone_actor_basis.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static unsigned checks,failures;
#define CHECK(x) do {checks++;if(!(x)){failures++;printf("FAIL line%d: %s\n",__LINE__,#x);}} while(0)
static u32 bits(float value) {u32 b;memcpy(&b,&value,4);return b;}
int main(void) {
    float angles[3]={0,0,-0.0f},out[16],before[16],overlap[20];
    float quarter[2];u32 i,mask;
    CHECK(dreadzone_actor_basis(angles,out)==DREADZONE_BASIS_APPROXIMATE);
    for(i=0;i<16;i++) CHECK(bits(out[i])==(i%5==0 ? 0x3F800000U : 0));
    CHECK(dreadzone_angle_pair(0.25f,quarter)==DREADZONE_ANGLE_APPROXIMATE);
    angles[0]=0.25f;
    CHECK(dreadzone_actor_basis(angles,out)==DREADZONE_BASIS_APPROXIMATE);
    CHECK(bits(out[5])==0x3F780AA5 && bits(out[6])==0x3E7D5776);
    CHECK(bits(out[9])==0xBE7D5776 && bits(out[10])==0x3F780AA5);
    CHECK(out[0]==1 && out[15]==1);
    {
        u32 pi_bits=0x40490FDBU;
        memcpy(&angles[0],&pi_bits,4);
        CHECK(dreadzone_actor_basis(angles,out)==DREADZONE_BASIS_APPROXIMATE);
        CHECK(bits(out[6])==0 && bits(out[9])==0);
    }
    for(mask=0;mask<8;mask++) {
        angles[0]=mask&1 ? 0.25f : 0;
        angles[1]=mask&2 ? -0.5f : -0.0f;
        angles[2]=mask&4 ? 1.0f : 0;
        CHECK(dreadzone_actor_basis(angles,out)==DREADZONE_BASIS_APPROXIMATE);
        for(i=0;i<16;i++) CHECK(isfinite(out[i]));
        CHECK(out[12]==0 && out[13]==0 && out[14]==0 && out[15]==1);
        for(i=0;i<20;i++) overlap[i]=(float)i;
        memcpy(overlap,angles,sizeof(angles));
        CHECK(dreadzone_actor_basis(overlap,overlap+1)==DREADZONE_BASIS_APPROXIMATE);
        CHECK(memcmp(out,overlap+1,sizeof(out))==0);
        CHECK(overlap[0]==angles[0] && overlap[17]==17 && overlap[19]==19);
    }
    memcpy(before,out,sizeof(out));angles[0]=NAN;
    CHECK(dreadzone_actor_basis(angles,out)==DREADZONE_BASIS_UNSUPPORTED);
    CHECK(memcmp(before,out,sizeof(out))==0);
    angles[0]=0;angles[2]=4;
    CHECK(dreadzone_actor_basis(angles,out)==DREADZONE_BASIS_UNSUPPORTED);
    CHECK(memcmp(before,out,sizeof(out))==0);
    angles[0]=0;angles[1]=0;angles[2]=1.0e-8f;
    CHECK(dreadzone_actor_basis(angles,out)==DREADZONE_BASIS_APPROXIMATE);
    CHECK(bits(out[1])==bits(angles[2]) && bits(out[4])==bits(-angles[2]));
    memcpy(before,out,sizeof(out));angles[2]=1.0e-13f;
    CHECK(dreadzone_actor_basis(angles,out)==DREADZONE_BASIS_UNSUPPORTED);
    CHECK(memcmp(before,out,sizeof(out))==0);
    CHECK(dreadzone_actor_basis(NULL,out)==DREADZONE_BASIS_INVALID);
    CHECK(dreadzone_actor_basis(angles,NULL)==DREADZONE_BASIS_INVALID);
    CHECK(dreadzone_actor_basis((float *)(uintptr_t)(UINTPTR_MAX-3),out)==DREADZONE_BASIS_INVALID);
    CHECK(dreadzone_actor_basis(angles,(float *)(uintptr_t)(UINTPTR_MAX-3))==DREADZONE_BASIS_INVALID);
    printf("%u checks, %u failures\n",checks,failures);
    return failures ? 1 : 0;
}
