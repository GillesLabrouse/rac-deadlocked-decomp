/* cflags: -ffast-math -mno-split-addresses */
/* asflags: --float-immediates */
/* Complete 2488-byte actor parameter update; NONMATCHING draft. These scalar
 * fields are upstream state parameters, not proof of a selected animation. */
#include "common.h"
#ifdef DEADLOCKED_NATIVE
#include "../native/dreadzone_actor_walk_parameters.h"
#include <math.h>
#endif

extern volatile f32 D_0021DDEC, D_0021DDF4;
extern s32 func_L01_005BD868(s32,s32,u32,u32);
extern s32 func_L01_006739B0(const u8 *);
extern f32 func_L01_00683580(const u8 *,const void *,s32);
extern f32 func_L01_0055D828(f32,f32);
extern f32 func_L01_0055D870(f32,f32);
extern f32 func_L01_0055D640(f32);
extern f32 func_L01_0055D658(f32);
extern f32 func_L01_00594AE8(f32 *,f32,f32);

#ifdef DEADLOCKED_NATIVE
#define FLOAT(o) dreadzone_walk_parameters_float(run,(o))
#define WORD(o) dreadzone_walk_parameters_word(record+(o))
#define SHORT(o) ((s16)((u16)record[o]|(u16)record[(o)+1]<<8))
#define BYTE(o) record[o]
#define STORE(o,x) dreadzone_walk_parameters_store(run,(o),(x))
#define MIN(a,b) dreadzone_direction_min((a),(b))
#define MAX(a,b) dreadzone_direction_max((a),(b))
#define ABS(a) fabsf(a)
#define TIME_EC dreadzone_walk_parameters_global(run,0x0021DDEC)
#define TIME_F4 dreadzone_walk_parameters_global(run,0x0021DDF4)
#define HISTORY(p) dreadzone_walk_parameters_history(run,(p))
#define DIVIDE(a,b) dreadzone_walk_parameters_divide(run,(a),(b))
#define RETURN return run->status
int dreadzone_walk_parameters_execute(DreadzoneWalkParametersRun *run) {
    u8 *record=run->context->record;
    if(WORD(0x25CC)!=2 || BYTE(0x2659))return DREADZONE_DIRECTION_UNSUPPORTED_MODE;
#else
#define FLOAT(o) (*(volatile f32 *)(record+(o)))
#define WORD(o) (*(volatile s32 *)(record+(o)))
#define SHORT(o) (*(volatile s16 *)(record+(o)))
#define BYTE(o) (*(volatile u8 *)(record+(o)))
#define STORE(o,x) (FLOAT(o)=(x))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define ABS(a) __builtin_fabsf(a)
#define TIME_EC D_0021DDEC
#define TIME_F4 D_0021DDF4
#define HISTORY(p) func_L01_005BD868(0x28,2,0,(p))
#define DIVIDE(a,b) ((a)/(b))
#define RETURN return
void func_L01_00681B30(u8 *record) {
#endif
    f32 angle, amount, scale, a, b, c, absolute, rate;
#ifndef DEADLOCKED_NATIVE
    f32 d,e,reduced;
#endif
    s32 state,selected,special;
#ifndef DEADLOCKED_NATIVE
    s32 left,right;
#endif
    special=SHORT(0x5F4)!=0;
    if(!special) {
        selected=WORD(0x1AE0);
        special=selected==1 && HISTORY((u32)WORD(0x2F00)) &&
                func_L01_006739B0(record)!=selected && func_L01_006739B0(record)!=15;
    }
    if(special) {
            state=WORD(0x25CC);
            if(state!=16 && FLOAT(0x2E08)>.15f && state!=11 && state!=100 &&
               state!=103 && !BYTE(0x5F6)) {
                scale=FLOAT(0x25C4);
                STORE(0x8C4,TIME_EC*.037f*scale);
                STORE(0x8C8,TIME_EC*.27f*scale);
                STORE(0x974,TIME_EC*.025f*scale);
                STORE(0x978,TIME_EC*.27f*scale);
#ifdef DEADLOCKED_NATIVE
                angle=func_L01_0055D870(FLOAT(0x5F8),FLOAT(0xB8));
#else
                angle=BYTE(0x2659)?func_L01_00683580(record,record+0x5E0,0):
                                  func_L01_0055D870(FLOAT(0x5F8),FLOAT(0xB8));
#endif
                STORE(0x888,angle);
                STORE(0x938,-angle);
                RETURN;
            }
    }
    state=WORD(0x25CC);
    if(state==2) {
        STORE(0xA24,.04f); STORE(0xAD4,.02f); STORE(0xAD8,.2f);
        STORE(0x8C4,.04f); STORE(0x8C8,.2f); STORE(0x974,.04f);
        STORE(0x978,.2f); STORE(0xA28,.2f);
        if(WORD(0x25D0)==1) {
            amount=FLOAT(0x17A4)*17.f;
            angle=FLOAT(0x17A8); absolute=ABS(angle);
            amount=MIN(amount,1.4f); amount=MAX(amount,-1.4f);
            amount=MIN(amount,absolute*3.f); amount=MAX(amount,-absolute*3.f);
            if(BYTE(0x1C8F)) {
                rate=TIME_EC; scale=FLOAT(0x25C4);
                b=rate*.3f*scale; a=rate*.055f*scale;
                amount=MIN(amount,1.1f); amount=MAX(amount,-1.1f);
                amount=MIN(amount,absolute); amount=MAX(amount,-absolute);
                STORE(0x978,b); STORE(0x974,a);
                rate=TIME_EC;
                STORE(0xAD8,rate*.3f*scale); STORE(0xAD4,rate*.07f*scale);
            }
            a=-amount*.11f;
            STORE(0x880,a);
            if(BYTE(0x1C8F)) STORE(0x880,a*.25f);
            rate=TIME_F4; scale=FLOAT(0x25C4);
            b=rate*5.7f*scale;
            a=DIVIDE(FLOAT(0x1B4),b); c=FLOAT(0x880);
            STORE(0x938,amount*.31f);
            a=MIN(a,1.f); a=MAX(a,0.f);
            STORE(0x880,c*a);
            if(BYTE(0x1C8F)) STORE(0x938,0.f);
            absolute=ABS(amount);
            STORE(0xA94,absolute*-.15f); STORE(0xA98,amount*.7f);
            STORE(0xA90,absolute*.16f); STORE(0x9E0,amount*-.15f);
            RETURN;
        }
        angle=FLOAT(0x17A8);
        if(FLOAT(0x2E08)<.8f && WORD(0x370)<40) {
            selected=WORD(0x370);
#ifdef DEADLOCKED_NATIVE
            if(selected<0)return DREADZONE_DIRECTION_UNSUPPORTED_MODE;
#endif
            (void)func_L01_00594AE8(&angle,0.f,(f32)(40-selected)*.08726646f);
        }
        amount=angle*.5f;
        amount=MIN(amount,.8f); amount=MAX(amount,-.8f);
        amount=amount*ABS(amount)*1.25f;
        absolute=ABS(amount)*.25f;
        STORE(0x938,amount); STORE(0xA98,amount); STORE(0xA94,absolute);
        RETURN;
    }
#ifndef DEADLOCKED_NATIVE
    if(state==8) {
        amount=FLOAT(0x17A8)*1.6f;
        amount=MIN(amount,1.25f); amount=MAX(amount,-1.25f);
        absolute=ABS(amount);
        a=absolute*.2f; b=-amount*.52f; c=amount*1.2f;
        d=amount*.58f; e=absolute*.16f;
        a=MIN(a,.2f); b=MIN(b,.28f); c=MIN(c,.6f);
        b=MAX(b,-.28f); c=MAX(c,-.6f); a=MAX(a,-.2f);
        STORE(0xA90,e); STORE(0xA24,.015f); STORE(0xA28,.08f);
        STORE(0xAD4,.008f); STORE(0xAD8,.1f);
        STORE(0x880,b); STORE(0x938,d); STORE(0x9E0,amount*-.28f);
        STORE(0xA98,c); STORE(0xA94,a); STORE(0x8C4,.008f);
        STORE(0x8C8,.08f); STORE(0x974,.015f); STORE(0x978,.08f);
        RETURN;
    }
    if(state==132) {
        amount=FLOAT(0x17A8)*.7f;
        amount=MIN(amount,1.2f); amount=MAX(amount,-1.2f);
        absolute=ABS(amount)*.16f;
        a=-amount*.52f; b=amount*1.2f; c=amount*.58f;
        a=MIN(a,.28f); b=MIN(b,.6f); a=MAX(a,-.28f); b=MAX(b,-.6f);
        STORE(0x880,a); STORE(0xA98,b); STORE(0x8C4,.017f);
        STORE(0x8C8,.33f); STORE(0xA24,.015f); STORE(0xAD4,.008f);
        STORE(0xAD8,.25f); STORE(0x938,c); STORE(0x9E0,amount*-.28f);
        STORE(0xA90,absolute); STORE(0x974,.015f);
        STORE(0x978,.25f); STORE(0xA28,.25f);
        STORE(0x884,func_L01_0055D828(FLOAT(0x884),-.12217305f));
        if(FLOAT(0x2E08)>.1f) {
            angle=func_L01_0055D870(FLOAT(0x17A0),FLOAT(0xB8));
            scale=FLOAT(0x2E08)*.61086524f;
            a=-func_L01_0055D658(angle)*scale;
            STORE(0x880,a);
            b=func_L01_0055D640(angle)*scale;
            b=func_L01_0055D828(FLOAT(0x884),b);
            a=FLOAT(0x880);
            b=MIN(b,.47123888f); a=MIN(a,.47123888f);
            b=MAX(b,-.47123888f); a=MAX(a,-.47123888f);
            STORE(0x880,a); STORE(0x884,b);
        }
        left=right=1;
        selected=BYTE(0x2671);
        STORE(0xEF4,.025f); STORE(0xEF8,.37f);
        STORE(0xE44,.025f); STORE(0xE48,.37f);
        if(selected==1) right=0;
        else if(selected==2) left=right=0;
        reduced=func_L01_0055D828(FLOAT(0x158)*5.f,.29670596f);
        if(left) {
            a=-reduced; a=MIN(a,.61086524f); a=MAX(a,-.61086524f);
            STORE(0xE08,-.34906584f); STORE(0xE00,a);
        }
        if(right) {
            a=MIN(reduced,.61086524f); a=MAX(a,-.61086524f);
            STORE(0xEB8,.34906584f); STORE(0xEB0,a);
        }
        a=-FLOAT(0x158)*7.7f; a=MIN(a,.6457718f); a=MAX(a,-.6457718f);
        STORE(0xA94,a);
        RETURN;
    }
    selected=WORD(0x25D4);
    if(selected==2 || selected==4) {
        amount=FLOAT(0x17A8); amount=MIN(amount,1.4f); amount=MAX(amount,-1.4f);
        a=amount*.29f; b=amount*-.27f; c=amount*.87f;
        STORE(0xA24,.04f); STORE(0xA98,c); STORE(0xAD4,.02f);
        STORE(0xAD8,.2f); STORE(0x938,a); STORE(0x9E0,b);
        STORE(0x8C4,.04f); STORE(0x8C8,.2f);
        STORE(0x974,.04f); STORE(0x978,.2f); STORE(0xA28,.2f);
    }
#else
    RETURN;
#endif
}
