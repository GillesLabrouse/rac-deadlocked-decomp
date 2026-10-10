#include "dreadzone_input_repeat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks;
static void check(int valid,const char *message) {
    checks++;
    if (!valid) { fprintf(stderr,"Input repeat check failed: %s\n",message); exit(1); }
}
static void bind_actor(DreadzoneActorInit31F0 *actor,u32 address) {
    memcpy((u8 *)actor+0x2F00,&address,4);
}
static void test_sequence(void) {
    DreadzoneActorInit31F0 actors[4];
    InputRecord5C0 inputs[4],before;
    DreadzoneRepeatInputView views[4];
    DreadzoneRepeatContext host;
    unsigned i,frame;
    memset(&host,0,sizeof(host)); memset(actors,0xCD,sizeof(actors));
    memset(inputs,0,sizeof(inputs));
    host.player_count=4; host.actor_count=4; host.actors=actors;
    host.inputs=views; host.input_count=4; host.mask_0021F580=0x400;
    for(i=0;i<4;i++) {
        views[i].guest_address=0x1EE600+i*0x5C0; views[i].storage=&inputs[i];
        bind_actor(&actors[i],views[i].guest_address);
        inputs[i].f1C0=(s32)(0x400U<<i);
        inputs[i].f1C4=0x777;
    }
    for(frame=0;frame<41;frame++) {
        for(i=0;i<4;i++) inputs[i].f1C4=0;
        check(dreadzone_input_repeat(&host)==0,"four real actor links resolve and execute repeat pass");
        for(i=0;i<4;i++) {
            int repeated=frame>=20 && (frame-20)%5==0;
            s32 countdown=frame<20 ? 20-(s32)frame : 5-(s32)((frame-20)%5);
            check(inputs[i].f1C4==(repeated?(inputs[i].f1C0 & 0x400):0),
                  "initial delay20 then cadence5 applies only selected mask bits");
            check(host.countdown_0021F588[i]==countdown,"per-player repetition countdown");
        }
    }
    inputs[0].f1C0=0x800; inputs[0].f1C4=0xA55;
    check(dreadzone_input_repeat(&host)==0 && host.countdown_0021F588[0]==20 && inputs[0].f1C4==0xA55,
          "changed mask restarts initial delay and preserves already decoded press");
    inputs[0].f1C0=0;
    for(frame=0;frame<25;frame++)
        check(dreadzone_input_repeat(&host)==0 && host.countdown_0021F588[0]==20,
              "zero held mask resets delay on every pass");
    inputs[0].f1C0=0x400; host.previous_0021F598[0]=0x400;
    host.countdown_0021F588[0]=(s32)0x80000000U;
    before=inputs[0];
    check(dreadzone_input_repeat(&host)==0 && host.countdown_0021F588[0]==0x7FFFFFFF,
          "real addiu decrement wraps signed INT_MIN to positive INT_MAX");
    check(memcmp(&before,&inputs[0],sizeof(before))==0,"countdown-only pass preserves entire input record");
    bind_actor(&actors[1],0); before=inputs[1];
    host.countdown_0021F588[1]=-77; host.previous_0021F598[1]=0xABC;
    check(dreadzone_input_repeat(&host)==0 && host.countdown_0021F588[1]==-77 &&
          host.previous_0021F598[1]==0xABC && memcmp(&before,&inputs[1],sizeof(before))==0,
          "null actor input link skips all state for that player");
    check(dreadzone_repeat_actor_at(&host,3)==&actors[3] && dreadzone_repeat_actor_at(&host,4)==NULL,
          "real 31F0 selector stride retains host pointer width and bounds");
}
static void test_mapping(void) {
    DreadzoneActorInit31F0 actors[2];
    InputRecord5C0 inputs[2];
    DreadzoneRepeatInputView views[2];
    DreadzoneRepeatContext host,before;
    memset(&host,0,sizeof(host)); memset(actors,0,sizeof(actors)); memset(inputs,0,sizeof(inputs));
    host.player_count=2; host.actors=actors; host.actor_count=2;
    host.inputs=views; host.input_count=2;
    views[0].guest_address=0x1EE600; views[0].storage=&inputs[0];
    views[1].guest_address=0x1EFCC0; views[1].storage=&inputs[1];
    bind_actor(&actors[0],views[0].guest_address); bind_actor(&actors[1],0x123456);
    before=host;
    check(dreadzone_input_repeat(&host)==DREADZONE_REPEAT_NEEDS_INPUT && memcmp(&before,&host,sizeof(host))==0,
          "missing later actor mapping is refused before earlier-player mutation");
    bind_actor(&actors[1],0); views[1]=views[0];
    check(dreadzone_input_repeat(&host)==DREADZONE_REPEAT_INVALID,"duplicate guest mapping is ambiguous");
    host.input_count=1; views[0].guest_address=0xFFFFFFF0; bind_actor(&actors[0],views[0].guest_address);
    check(dreadzone_input_repeat(&host)==DREADZONE_REPEAT_INVALID,"complete guest input span cannot wrap");
    host.player_count=5;
    check(dreadzone_input_repeat(&host)==DREADZONE_REPEAT_INVALID,"four retail countdown slots bound the host domain");
    host.player_count=0; host.actors=NULL; host.inputs=NULL;
    check(dreadzone_input_repeat(&host)==0,"zero players takes real no-op branch without invented storage");
    host.player_count=-1;
    check(dreadzone_input_repeat(&host)==0,"negative count takes the actual signed no-op branch");
}
int main(void) {
    test_sequence(); test_mapping();
    printf("Dreadzone recovered button repetition: %u checks passed.\n",checks);
    return 0;
}
