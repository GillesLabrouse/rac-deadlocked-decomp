#include "dreadzone_actor_input_bridge.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

typedef struct Fixture {
    u8 guard0[16],record[0x31F0],guard1[16],input[0x5C0],guard2[16];
    u8 modes[2],guard3[16],state[4],guard4[16],mode[4],guard5[16];
    u8 flag[4],guard6[16],client[0x54],guard7[16],related[0x88],guard8[16];
    DreadzoneActorInputMemory views[7];
    DreadzoneActorInputContext context;
} Fixture;
static Fixture fixture,expected;
static unsigned checks,failures;
u8 D_0021DE31[1];
s32 D_0021DDB4_[1],D_0021E8A0[1];
static u32 random_state=0x6A7D48;
static u32 next(void) {
    random_state^=random_state<<13;random_state^=random_state>>17;random_state^=random_state<<5;return random_state;
}
static void check(int okay,const char *message) {
    ++checks;if(!okay && failures++<12) printf("FAIL: %s\n",message);
}
static void bind(Fixture *f) {
    f->views[0]=(DreadzoneActorInputMemory){f->input,0x1EE600,sizeof(f->input),0};
    f->views[1]=(DreadzoneActorInputMemory){f->modes,0x171DE0,sizeof(f->modes),0};
    f->views[2]=(DreadzoneActorInputMemory){f->state,0x21DDB4,sizeof(f->state),0};
    f->views[3]=(DreadzoneActorInputMemory){f->mode,0x21E824,sizeof(f->mode),0};
    f->views[4]=(DreadzoneActorInputMemory){f->flag,0x21E8F4,sizeof(f->flag),1};
    f->views[5]=(DreadzoneActorInputMemory){f->client,0x230280,sizeof(f->client),0};
    f->views[6]=(DreadzoneActorInputMemory){f->related,0x500000,sizeof(f->related),0};
    f->context=(DreadzoneActorInputContext){f->record,0x368F60,sizeof(f->record),f->views,7};
}
static void store(u8 *p,u32 value) {dreadzone_actor_input_store32(p,value);}
static u32 word(const u8 *p) {return dreadzone_actor_input_word(p);}
static u32 float_bits(float value) {u32 bits;memcpy(&bits,&value,4);return bits;}
static float number(const u8 *p) {u32 bits=word(p);float value;memcpy(&value,&bits,4);return value;}
static void initialize(void) {
    size_t i;
    for(i=0;i<offsetof(Fixture,views);++i) ((u8 *)&fixture)[i]=(u8)next();
    bind(&fixture);fixture.modes[0]=fixture.modes[1]=0;
    store(fixture.state,0);store(fixture.mode,0);store(fixture.flag,0);
    store(fixture.record+0x2F10,0);store(fixture.record+0x2F00,0x1EE600);
    store(fixture.record+0x2EFC,0x230280);store(fixture.record+0x3F4,0);
    fixture.record[0x266D]=1;store(fixture.record+0x25CC,0);
    store(fixture.client+0x50,0x500000);fixture.related[0x86]=1;fixture.related[0x87]=0;
    store(fixture.record+0x128,0);store(fixture.input+0x108,0);store(fixture.input+0x10C,0);
}
static void snapshot(void) {expected=fixture;bind(&expected);}
static void image(void) {check(memcmp(&fixture,&expected,offsetof(Fixture,views))==0,"entire RAM images and backing guards");}
static int predicate(const Fixture *f) {
    u32 index=word(f->record+0x2F10),state=word(f->state),action=word(f->record+0x25CC);
    if(!f->modes[index] || word(f->record+0x3F4) || state==2 || state==6) return 0;
    if(f->record[0x266D] && f->related[0x86]==5 && f->related[0x87]==0) return 0;
    return !(action==0x71 || action==0x31 || action==0x1C || action==0x8E || action==0x8F);
}
static void button_oracle(Fixture *f) {
    s16 timer;u32 mask=word(f->mode)==1?1:3;
    memcpy(&timer,f->record+0x5F4,2);
    if(predicate(f)) {
        if(timer) {timer=(s16)((timer<1?1:timer)-1);memcpy(f->record+0x5F4,&timer,2);}
    } else {
        timer=(s16)((word(f->input+0x1C0)&mask)!=0);
        memcpy(f->record+0x5F4,&timer,2);
        if(word(f->mode)==1 && word(f->record+0x5F0)) {f->record[0x5F4]=1;f->record[0x5F5]=0;}
    }
}
static void numeric_cases(void) {
    u32 seed,index,axis;float x,y,z;double length,angle;
    for(seed=0;seed<2048;++seed) {
        initialize();index=seed&1;store(fixture.record+0x2F10,index);
        x=(float)((s32)(next()%33)-16)/16.f;
        y=(float)((s32)(next()%33)-16)/16.f;
        z=(float)((s32)(next()%33)-16)/16.f;
        if(seed%8==0) {x=0.f;y=0.f;z=0.f;}
        if(seed%8==1) {x=-0.f;y=-0.f;z=-0.f;}
        if(seed%8==2) {x=1.f;y=1.f;z=1.f;}
        if(seed%8==3) {x=0.125f;y=0.25f;z=0.5f;}
        store(fixture.input+0x108,float_bits(x));store(fixture.input+0x10C,float_bits(y));
        store(fixture.record+0x128,float_bits(z));
        store(fixture.mode,seed%3);store(fixture.input+0x1C0,next());
        if(seed&8) {fixture.modes[index]=2;store(fixture.state,2);}
        if(seed&16) {fixture.modes[index]=3;store(fixture.state,0);}
        if(seed&32) fixture.related[0x86]=5;
        if(seed&64) store(fixture.record+0x3F4,1);
        if(seed&128) store(fixture.record+0x25CC,0x71);
        snapshot();button_oracle(&expected);
        store(expected.record+0x120,float_bits(x));store(expected.record+0x124,float_bits(y)^0x80000000U);
        check(dreadzone_actor_input_006A7D48(&fixture.context)==1,"active bridge is explicitly approximate VU");
        length=sqrt((double)x*x+(double)y*y+(double)z*z);if(length>1.0) length=1.0;
        angle=x==0.f && y==0.f?0.0:atan2(-(double)y,(double)x);
        check(fabs((double)number(fixture.record+0x2E38)-length)<0.000001,"independent xyz magnitude oracle");
        check(word(fixture.record+0x2E38)==word(fixture.record+0x2E08),"both actual magnitude stores");
        check(fabs((double)number(fixture.record+0x2E0C)-angle)<0.000003,"independent atan2 finite grid oracle");
        check(word(fixture.record+0x128)==float_bits(z),"third component retained including signed zero");
        /* Approximate float outputs are checked numerically above; every other
         * byte must exactly match the independent ordered RAM oracle. */
        for(axis=0;axis<3;++axis) {
            u32 offset=axis==0?0x2E38:axis==1?0x2E08:0x2E0C;
            memcpy(expected.record+offset,fixture.record+offset,4);
        }
        image();
    }
}
static void exact_and_alias_cases(void) {
    u32 seed,index;
    for(seed=0;seed<2048;++seed) {
        initialize();index=seed&1;store(fixture.record+0x2F10,index);
        fixture.modes[index]=(u8)(seed%4);store(fixture.mode,seed%3);store(fixture.state,seed%7);
        if(seed&4) store(fixture.record+0x3F4,1);
        if(seed&8) fixture.related[0x86]=5;
        if(seed&16) fixture.record[0x266D]=0;
        if(seed&32) store(fixture.record+0x25CC,0x31);
        store(fixture.input+0x1C0,next());store(fixture.flag,next()|1);
        snapshot();button_oracle(&expected);store(expected.flag,0);
        check(dreadzone_actor_input_006A7D48(&fixture.context)==0,"one-shot branch executes exact CPU only");image();
    }
    initialize();
    store(fixture.record+0x2F00,fixture.context.record_guest+0x14);
    store(fixture.record+0x11C,float_bits(0.25f));store(fixture.record+0x120,float_bits(0.75f));
    store(fixture.record+0x1D4,0); /* Aliased input1C0 is read before axis stores. */
    snapshot();expected.record[0x5F4]=expected.record[0x5F5]=0;
    check(dreadzone_actor_input_006A7D48(&fixture.context)==1,"ordered input/actor alias succeeds");
    check(word(fixture.record+0x120)==float_bits(0.25f),"first alias source copied");
    check(word(fixture.record+0x124)==float_bits(-0.25f),"second source observes preceding actor120 store");
    store(expected.record+0x120,float_bits(0.25f));store(expected.record+0x124,float_bits(-0.25f));
    memcpy(expected.record+0x2E38,fixture.record+0x2E38,4);
    memcpy(expected.record+0x2E08,fixture.record+0x2E08,4);
    memcpy(expected.record+0x2E0C,fixture.record+0x2E0C,4);image();
    initialize();
    fixture.context.record_guest=0x21E300;
    fixture.views[3].guest_base=0x600000;fixture.views[4].guest_base=0x600100;
    fixture.modes[0]=1;fixture.record[0x266D]=0;
    store(fixture.record+0x524,0);store(fixture.record+0x5F4,2);
    snapshot();store(expected.record+0x5F4,0);
    check(dreadzone_actor_input_006A7D48(&fixture.context)==0,"one-shot aliases actual timer word");
    image();
    initialize();fixture.context.record_guest=0x21E230;
    fixture.views[3].guest_base=0x600000;fixture.views[4].guest_base=0x600100;
    store(fixture.record+0x5F4,1);store(fixture.record+0x6C4,1);
    store(fixture.record+0x5F0,1);store(fixture.input+0x1C0,0);
    snapshot();store(expected.record+0x5F4,0);store(expected.record+0x6C4,0);
    check(dreadzone_actor_input_006A7D48(&fixture.context)==0,"mode word aliases timer and is genuinely reloaded");
    image();
}
static void timers_and_remaps(void) {
    u32 bits,mode,config; s16 timer,before; s32 result,mask;
    for(bits=0;bits<65536;++bits) {
        u16 raw=(u16)bits;memcpy(&timer,&raw,2);before=timer;
        result=func_L01_0055D180(&timer);
        check(result==(before==0?1:before<=1?2:0),"every signed16 timer return");
        check(timer==(before==0?0:before<=1?0:before-1),"every signed16 timer mutation");
    }
    for(mode=0;mode<4;++mode) for(config=0;config<2;++config) for(bits=0;bits<256;++bits) {
        initialize();store(fixture.mode,mode);fixture.modes[1]=(u8)config;
        mask=(s32)bits;
        result=mode!=1 && mask==1?3:config && (mask==4 || mask==8)?1:
               config && mask==16?18:config && mask==64?68:mask;
        check(dreadzone_actor_input_005BE808(&fixture.context,mask,1,&mask)==0,"remapper exact status");
        check(mask==result,"independent complete byte-mask remapping");
    }
}
static void refusal(int expected_status,const char *message) {
    snapshot();check(dreadzone_actor_input_006A7D48(&fixture.context)==expected_status,message);image();
}
static double packet_axis(u32 sample) {
    if(sample>=79 && sample<=175) return 0;
    if(sample<79) return sample<=3?-1:((double)sample-79)/76;
    return sample>=251?1:((double)sample-175)/76;
}
static void decoded_packets(void) {
    InputRecord5C0 input;
    u8 packet[20];u32 sample,axis;
    double x,y,length,angle;
    /* Actual packet decoder and both actual angle helpers feed the bridge.
     * Zeroed input is controlled test RAM, not a captured controller driver.
     * A first sample needs no previous-frame rotation because its history is0. */
    for(sample=0;sample<256;++sample) {
        initialize();memset(&input,0,sizeof(input));memset(packet,0,sizeof(packet));
        packet[1]=0x79;packet[2]=255;packet[3]=254;
        packet[4]=packet[5]=127;packet[6]=(u8)sample;packet[7]=(u8)(255-sample);
        D_0021DE31[0]=0;D_0021DDB4_[0]=0;D_0021E8A0[0]=0;
        func_L01_005BDA40(&input,packet,20);
        memcpy(fixture.input,&input,sizeof(input));snapshot();
        expected.record[0x5F4]=1;expected.record[0x5F5]=0;
        store(expected.record+0x120,word(expected.input+0x108));
        store(expected.record+0x124,word(expected.input+0x10C)^0x80000000U);
        check((input.f1C0&1)!=0,"genuine decoder supplies the held button");
        check(dreadzone_actor_input_006A7D48(&fixture.context)==1,"genuine decoder feeds complete active bridge");
        x=packet_axis(sample);y=-packet_axis(255-sample);
        check(fabs(number(fixture.record+0x120)-x)<0.000001 &&
              fabs(number(fixture.record+0x124)-y)<0.000001,"packet bytes reach actor axes in the real order");
        length=sqrt(x*x+y*y);if(length>1) length=1;
        angle=x==0 && y==0?0:atan2(y,x);
        check(fabs(number(fixture.record+0x2E38)-length)<0.000001 &&
              word(fixture.record+0x2E38)==word(fixture.record+0x2E08),"decoded packet magnitude has both real stores");
        check(fabs(number(fixture.record+0x2E0C)-angle)<0.000003,"decoded packet angle follows the actual polynomial");
        for(axis=0;axis<3;++axis) {
            u32 offset=axis==0?0x2E38:axis==1?0x2E08:0x2E0C;
            memcpy(expected.record+offset,fixture.record+offset,4);
        }
        image();
    }
}
static void boundaries(void) {
    u32 i;static const u32 bad[]={1,0x80000001,0x307FFFFF,0x3F800001,0x7F800000,0xFF800000,0x7FC00000};
    for(i=0;i<sizeof(bad)/sizeof(bad[0]);++i) {
        initialize();store(fixture.input+0x108,bad[i]);refusal(-4,"unsupported first component is atomic");
        initialize();store(fixture.input+0x10C,bad[i]);refusal(-4,"unsupported second component is atomic");
        initialize();store(fixture.record+0x128,bad[i]);refusal(-4,"unsupported retained third component is atomic");
    }
    initialize();store(fixture.input+0x108,0x30800000);store(fixture.input+0x10C,0x30800000);
    check(dreadzone_actor_input_006A7D48(&fixture.context)==1,"finite lower domain endpoint succeeds");
    check(fabs((double)number(fixture.record+0x2E38)-sqrt(2.0)*ldexp(1.0,-30))<1e-15,
          "lower endpoint square/add/sqrt remains normal and finite");
    check(fabs((double)number(fixture.record+0x2E0C)+0.7853981633974483)<0.000003,
          "lower endpoint ratio retains its real quadrant");
    initialize();store(fixture.record+0x2F10,2);refusal(-3,"unsupported predicate index");
    initialize();fixture.context.view_count=1;refusal(-2,"missing mode/config mapping");
    initialize();fixture.views[0].bytes=0x10C;store(fixture.input+0x108,float_bits(0.25f));
    fixture.modes[0]=1;fixture.record[0x266D]=0;
    refusal(-2,"missing later input read discards earlier stores");
    initialize();store(fixture.record+0x2F00,0x1EE601);refusal(-2,"original unaligned LW input pointer");
    initialize();fixture.views[4].writable=0;store(fixture.flag,1);refusal(-2,"one-shot write permission");
    initialize();fixture.modes[0]=1;fixture.views[5].guest_base=0x230800;refusal(-6,"actual reached client read unavailable");
    initialize();fixture.views[6].guest_base=0x500800;fixture.modes[0]=1;refusal(-6,"actual reached related read unavailable");
    initialize();fixture.context.record_guest++;refusal(-1,"unaligned VU record");
    initialize();fixture.views[0].guest_base=0xFFFFFF00;refusal(-1,"guest span wraps");
    initialize();fixture.views[0].storage=(u8 *)(UINTPTR_MAX-1);refusal(-1,"host span wraps before access");
    initialize();fixture.views[0].storage=fixture.record;refusal(-1,"host backing alias");
    initialize();fixture.views[0].guest_base=fixture.context.record_guest;refusal(-1,"ambiguous guest backing");
    initialize();fixture.context.views=(const DreadzoneActorInputMemory *)(UINTPTR_MAX-1);refusal(-1,"descriptor range wraps");
    initialize(); {
        DreadzoneActorInputContext *alias=(DreadzoneActorInputContext *)(fixture.record+0x120);
        memcpy(alias,&fixture.context,sizeof(*alias));snapshot();
        check(dreadzone_actor_input_006A7D48(alias)==-1,"context overlaps writable actor");image();
    }
    check(dreadzone_actor_input_006A7D48((const DreadzoneActorInputContext *)(UINTPTR_MAX-1))==-1,"context wraps");
}
int main(void) {
    numeric_cases();exact_and_alias_cases();timers_and_remaps();boundaries();decoded_packets();
    printf("%u actor input bridge checks, %u failures\n",checks,failures);
    return failures?1:0;
}
