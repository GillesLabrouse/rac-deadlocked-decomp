#include "dreadzone_collision_grid.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
static unsigned checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"Failed line%d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void) {
    DreadzoneCollisionGridRequest r={{2048,2048,2048},{6144,6144,6144}};
    DreadzoneCollisionGridRecord out[239],before[239];DreadzoneCollisionGridInfo info,old;
    memset(out,0xA5,sizeof(out));memcpy(before,out,sizeof(out));memset(&info,0xB6,sizeof(info));old=info;
    CHECK(dreadzone_collision_grid(&r,out,239,&info)==1);
    CHECK(info.record_count==5 && info.zero_q_division_mask==0 && info.has_integer_displacement==1);
    CHECK(out[0].fraction_bits==0 && out[0].packed_cell==0);
    CHECK(out[1].fraction_bits==0x3F000000 && out[1].packed_cell==1);
    CHECK(out[2].fraction_bits==0x3F000000 && out[2].packed_cell==0x101);
    CHECK(out[3].fraction_bits==0x3F000000 && out[3].packed_cell==0x10101);
    CHECK(out[4].fraction_bits==0x3F800000 && out[4].packed_cell==0);
    CHECK(memcmp(out+5,before+5,sizeof(out)-5*sizeof(*out))==0);
    r.start_scaled[0]=r.start_scaled[1]=r.start_scaled[2]=6144;
    r.end_scaled[0]=r.end_scaled[1]=r.end_scaled[2]=2048;
    CHECK(dreadzone_collision_grid(&r,out,239,&info)==1);
    CHECK(out[1].packed_cell==0x10100 && out[2].packed_cell==0x10000 && out[3].packed_cell==0);
    r.start_scaled[0]=r.end_scaled[0]=r.start_scaled[2]=r.end_scaled[2]=2048;
    CHECK(dreadzone_collision_grid(&r,out,239,&info)==1);CHECK(info.record_count==3 && info.zero_q_division_mask==5);
    r.end_scaled[1]=r.start_scaled[1];CHECK(dreadzone_collision_grid(&r,out,239,&info)==1);
    CHECK(info.record_count==1 && info.zero_q_division_mask==0 && info.has_integer_displacement==0);
    r.end_scaled[1]+=0.25f;CHECK(dreadzone_collision_grid(&r,out,239,&info)==1);CHECK(info.has_integer_displacement==0);
    memcpy(out,before,sizeof(out));info=old;
    r.end_scaled[1]=r.start_scaled[1]+80*4096;CHECK(dreadzone_collision_grid(&r,out,239,&info)==-2);
    CHECK(memcmp(out,before,sizeof(out))==0 && memcmp(&info,&old,sizeof(info))==0);
    r.end_scaled[1]=r.start_scaled[1]+79*4096;CHECK(dreadzone_collision_grid(&r,out,80,&info)==-3);
    CHECK(memcmp(out,before,sizeof(out))==0 && memcmp(&info,&old,sizeof(info))==0);
    CHECK(dreadzone_collision_grid(&r,out,239,&info)==1);CHECK(info.record_count==81);
    r.start_scaled[0]=r.start_scaled[1]=r.start_scaled[2]=128*4096+2048;
    r.end_scaled[0]=r.end_scaled[1]=r.end_scaled[2]=207*4096+2048;
    CHECK(dreadzone_collision_grid(&r,out,239,&info)==1);CHECK(info.record_count==239);
    CHECK(out[237].packed_cell==0xCFCFCF && out[238].fraction_bits==0x3F800000);
    memcpy(out,before,sizeof(out));
    CHECK(dreadzone_collision_grid(&r,out,239,(DreadzoneCollisionGridInfo *)(void *)out)==-1);
    CHECK(memcmp(out,before,sizeof(out))==0);
    r.start_scaled[0]=-0.0f;CHECK(dreadzone_collision_grid(&r,out,239,&info)==-2);
    r.start_scaled[0]=NAN;CHECK(dreadzone_collision_grid(&r,out,239,&info)==-2);
    CHECK(dreadzone_collision_grid(NULL,out,239,&info)==-1);
    printf("%u collision grid checks passed\n",checks);return 0;
}
