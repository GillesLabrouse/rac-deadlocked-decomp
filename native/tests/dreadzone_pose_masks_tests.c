#include "dreadzone_pose_masks.h"
#include <stdio.h>
#include <string.h>
static unsigned checks,failures;
#define CHECK(x) do {++checks;if(!(x)){++failures;printf("line %d: %s\n",__LINE__,#x);}}while(0)
typedef struct Fixture {
    u8 scratch[0x4020],vu[0x1020],lists[32];DreadzoneModelView view;DreadzonePoseMasks request;
} Fixture;
static void setup(Fixture *f) {
    memset(f,0,sizeof(*f));memset(f->scratch,0xA5,sizeof(f->scratch));memset(f->vu,0x5A,sizeof(f->vu));
    f->view=(DreadzoneModelView){0x500000,sizeof(f->lists),f->lists};
    f->request=(DreadzonePoseMasks){f->scratch,0x4000,f->vu,0x1000,&f->view,1,2,5,0,0};
    f->lists[4]=2;f->lists[5]=4;f->lists[6]=6;f->lists[7]=255;
    f->lists[16+4]=1;f->lists[16+5]=3;f->lists[16+6]=8;f->lists[16+7]=255;
}
static void expect_mask(const Fixture *f,u32 joint,u8 value) {
    CHECK(f->scratch[joint*64+0x3D]==value);CHECK(f->vu[joint*16+12]==value);
    CHECK(f->vu[joint*16+13]==0 && f->vu[joint*16+14]==0 && f->vu[joint*16+15]==0);
}
static void refusal(Fixture *f,int status) {
    Fixture before=*f;CHECK(dreadzone_pose_masks_prepare(&f->request)==status);
    CHECK(!memcmp(before.scratch,f->scratch,sizeof(f->scratch)));
    CHECK(!memcmp(before.vu,f->vu,sizeof(f->vu)));CHECK(!memcmp(before.lists,f->lists,sizeof(f->lists)));
}
int main(void) {
    Fixture f;u32 i;u8 expected_scratch[0x4020],expected_vu[0x1020];
    setup(&f);f.request.second_list_guest=0xDEADBEEF; /* Null first list genuinely returns early. */
    memcpy(expected_scratch,f.scratch,sizeof(expected_scratch));memcpy(expected_vu,f.vu,sizeof(expected_vu));
    for(i=2;i<=7;++i){expected_scratch[i*64+0x3D]=(u8)(i==7);memset(expected_vu+i*16+12,0,4);expected_vu[i*16+12]=(u8)(i==7);}
    CHECK(dreadzone_pose_masks_prepare(&f.request)==0);
    CHECK(!memcmp(expected_scratch,f.scratch,sizeof(expected_scratch)));CHECK(!memcmp(expected_vu,f.vu,sizeof(expected_vu)));
    setup(&f);f.request.first_list_guest=0x500000;CHECK(dreadzone_pose_masks_prepare(&f.request)==0);
    for(i=2;i<=7;++i)expect_mask(&f,i,(u8)(i==3 || i==5 || i==7));
    setup(&f);f.request.first_list_guest=0x500000;f.request.second_list_guest=0x500010;
    CHECK(dreadzone_pose_masks_prepare(&f.request)==0);expect_mask(&f,1,0);expect_mask(&f,3,0);expect_mask(&f,8,0);expect_mask(&f,5,1);
    CHECK(f.scratch[0x3D]==0xA5);CHECK(f.scratch[9*64+0x3D]==0xA5);
    /* Real scratch list alias: the next ID is loaded before writing its current flag. */
    setup(&f);f.request.first_joint=0;f.request.joint_count=2;f.request.first_list_guest=0x70000038;
    f.scratch[0x38]=0;f.scratch[0x39]=0;f.scratch[0x3C]=0;f.scratch[0x3D]=1;f.scratch[0x3E]=255;
    CHECK(dreadzone_pose_masks_prepare(&f.request)==0);expect_mask(&f,0,0);expect_mask(&f,1,0);expect_mask(&f,2,1);
    setup(&f);f.request.first_list_guest=0x500020;refusal(&f,-2);
    setup(&f);f.request.first_list_guest=0x500000;f.view.bytes=6;refusal(&f,-2);
    setup(&f);f.request.first_list_guest=0x500000;f.request.second_list_guest=0x500010;f.lists[20]=255;refusal(&f,-3);
    setup(&f);f.request.first_list_guest=0x500000;f.request.second_list_guest=0x500010;f.lists[21]=1;refusal(&f,-3);
    setup(&f);f.request.joint_count=0;refusal(&f,-3);
    setup(&f);f.request.first_joint=127;f.request.joint_count=2;refusal(&f,-3);
    setup(&f);f.view.guest_base=0xFFFFFFF0;refusal(&f,-1);
    setup(&f);f.view.storage=f.scratch;refusal(&f,-1);
    setup(&f);f.request.vu_data=f.scratch;refusal(&f,-1);
    CHECK(dreadzone_pose_masks_prepare(NULL)==-1);
    printf("%u checks, %u failures\n",checks,failures);return failures?1:0;
}
