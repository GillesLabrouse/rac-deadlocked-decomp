#include "dreadzone_collision_triangle.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
static unsigned checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"Failed line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static DreadzoneCollisionTriangleRequest request(void) {
    DreadzoneCollisionTriangleRequest r;memset(&r,0,sizeof(r));
    r.vertex[1][0]=1;r.vertex[2][1]=1;
    r.start_relative[0]=r.end_relative[0]=0.25f;
    r.start_relative[1]=r.end_relative[1]=0.25f;
    r.start_relative[2]=-1;r.end_relative[2]=1;r.direction[2]=2;
    r.cell_origin[0]=16;r.cell_origin[1]=32;r.cell_origin[2]=48;
    r.best_fraction=1;r.source_guest=0x00400000U;return r;
}
int main(void) {
    DreadzoneCollisionTriangleRequest r=request();DreadzoneCollisionTriangleContact c,sentinel;
    DreadzoneCollisionTriangleQStatus status,status_before;
    union {DreadzoneCollisionTriangleRequest r;DreadzoneCollisionTriangleContact c;} alias;
    memset(&sentinel,0xA5,sizeof(sentinel));c=sentinel;
    CHECK(dreadzone_collision_triangle(&r,&c)==DREADZONE_TRIANGLE_APPROXIMATE_CONTACT);
    CHECK(c.fraction==0.5f && c.position_scaled[0]==16.25f && c.position_scaled[1]==32.25f && c.position_scaled[2]==48);
    CHECK(c.normal[0]==0 && c.normal[1]==0 && c.normal[2]==-1);
    CHECK(c.vertices_scaled[0][0]==16 && c.vertices_scaled[1][0]==17 && c.vertices_scaled[2][1]==33);
    CHECK(c.source_guest==0x00400000U && c.hit_metadata==0x1000U);
    alias.r=r;CHECK(dreadzone_collision_triangle(&alias.r,&alias.c)==1);CHECK(memcmp(&alias.c,&c,sizeof(c))==0);
    r.best_fraction=0.5f;c=sentinel;CHECK(dreadzone_collision_triangle(&r,&c)==0);CHECK(memcmp(&c,&sentinel,sizeof(c))==0);
    r.best_fraction=0.75f;CHECK(dreadzone_collision_triangle(&r,&c)==1);
    r.best_fraction=nextafterf(0.5f,1.0f);CHECK(dreadzone_collision_triangle(&r,&c)==1);
    r.best_fraction=nextafterf(0.5f,0.0f);c=sentinel;CHECK(dreadzone_collision_triangle(&r,&c)==0);CHECK(memcmp(&c,&sentinel,sizeof(c))==0);
    r=request();r.start_relative[0]=r.end_relative[0]=2;c=sentinel;
    CHECK(dreadzone_collision_triangle(&r,&c)==0);CHECK(memcmp(&c,&sentinel,sizeof(c))==0);
    r=request();r.surface=0x80;r.effective_flags=0x80;CHECK(dreadzone_collision_triangle(&r,&c)==0);
    r=request();r.effective_flags=0x20;CHECK(dreadzone_collision_triangle(&r,&c)==0);
    r.surface=1;CHECK(dreadzone_collision_triangle(&r,&c)==1);
    r=request();r.clip_flags[0]=r.clip_flags[1]=r.clip_flags[2]=0x100;CHECK(dreadzone_collision_triangle(&r,&c)==0);
    r=request();r.direction[2]=0;c=sentinel;CHECK(dreadzone_collision_triangle(&r,&c)==-2);CHECK(memcmp(&c,&sentinel,sizeof(c))==0);
    r=request();r.start_relative[2]=1;r.end_relative[2]=-1;r.direction[2]=-2;CHECK(dreadzone_collision_triangle(&r,&c)==0);
    r.effective_flags=0x1000;CHECK(dreadzone_collision_triangle(&r,&c)==1);
    r=request();r.best_fraction=NAN;c=sentinel;CHECK(dreadzone_collision_triangle(&r,&c)==-2);CHECK(memcmp(&c,&sentinel,sizeof(c))==0);
    r=request();r.vertex[0][0]=1e-40f;CHECK(dreadzone_collision_triangle(&r,&c)==-2);
    r=request();r.direction[2]=-0.0f;CHECK(dreadzone_collision_triangle(&r,&c)==-2);
    r=request();r.clip_flags[1]=0x80000000U;CHECK(dreadzone_collision_triangle(&r,&c)==-2);
    r=request();r.vertex[2][1]=0;CHECK(dreadzone_collision_triangle(&r,&c)==-2);
    r=request();r.start_relative[2]=0;r.direction[2]=1;CHECK(dreadzone_collision_triangle(&r,&c)==0);
    r=request();r.end_relative[2]=0;r.direction[2]=1;CHECK(dreadzone_collision_triangle(&r,&c)==0);
    r=request();r.start_relative[2]=-2;r.end_relative[2]=2;r.direction[2]=4;
    CHECK(dreadzone_collision_triangle(&r,&c)==1);CHECK(c.fraction==0.5f);
    r.best_fraction=c.fraction;r.vertex[0][2]=r.vertex[1][2]=r.vertex[2][2]=-1;r.surface=2;
    CHECK(dreadzone_collision_triangle(&r,&c)==1);CHECK(c.fraction==0.25f && c.hit_metadata==0x1002);
    r.best_fraction=c.fraction;r.vertex[0][2]=r.vertex[1][2]=r.vertex[2][2]=1;sentinel=c;
    CHECK(dreadzone_collision_triangle(&r,&c)==0);CHECK(memcmp(&c,&sentinel,sizeof(c))==0);
    CHECK(dreadzone_collision_triangle(NULL,&c)==-1);CHECK(dreadzone_collision_triangle(&r,NULL)==-1);
    r=request();memset(&status,0xB6,sizeof(status));status_before=status;
    CHECK(dreadzone_collision_triangle_with_q(&r,&c,&status)==1);
    CHECK(status.division_executed==1 && status.q_bits==0x3F000000 && status.exception_bits==0);
    r=request();r.direction[2]=0;r.end_relative[2]=-1;c=sentinel;
    CHECK(dreadzone_collision_triangle_with_q(&r,&c,&status)==0);
    CHECK(status.division_executed==1 && status.q_bits==0xFF7FFFFF && status.exception_bits==0x20);
    CHECK(memcmp(&c,&sentinel,sizeof(c))==0);
    r.start_relative[2]=r.end_relative[2]=0;
    CHECK(dreadzone_collision_triangle_with_q(&r,&c,&status)==0);
    /* Negative-zero plane dot divided by positive-zero denominator. */
    CHECK(status.division_executed==1 && status.q_bits==0xFF7FFFFF && status.exception_bits==0x10);
    r.start_relative[0]=r.end_relative[0]=r.start_relative[1]=r.end_relative[1]=-0.25f;
    CHECK(dreadzone_collision_triangle_with_q(&r,&c,&status)==0);
    CHECK(status.q_bits==0x7F7FFFFF && status.exception_bits==0x10);
    r=request();r.vertex[1][2]=r.vertex[2][2]=-1;
    r.start_relative[0]=r.start_relative[1]=r.start_relative[2]=-1;
    memcpy(r.end_relative,r.start_relative,sizeof(r.end_relative));memset(r.direction,0,sizeof(r.direction));
    CHECK(dreadzone_collision_triangle_with_q(&r,&c,&status)==0);
    CHECK(status.q_bits==0x7F7FFFFF && status.exception_bits==0x20);
    r=request();r.surface=0x80;r.effective_flags=0x80;
    CHECK(dreadzone_collision_triangle_with_q(&r,&c,&status)==0);
    CHECK(status.division_executed==0 && status.q_bits==0 && status.exception_bits==0);
    r=request();r.vertex[0][0]=NAN;status=status_before;c=sentinel;
    CHECK(dreadzone_collision_triangle_with_q(&r,&c,&status)==-2);
    CHECK(memcmp(&status,&status_before,sizeof(status))==0 && memcmp(&c,&sentinel,sizeof(c))==0);
    r=request();CHECK(dreadzone_collision_triangle_with_q(&r,&c,(DreadzoneCollisionTriangleQStatus *)(void *)&c)==-1);
    CHECK(dreadzone_collision_triangle_with_q(&r,&c,NULL)==-1);
    printf("%u collision triangle checks passed\n",checks);return 0;
}
