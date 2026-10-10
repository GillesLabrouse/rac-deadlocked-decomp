#include "common.h"

/* Complete224-byte animation-bank loader and variable decoder orchestrator.
 * CVT.W.S follows the actual FPU rounding mode, rather than C truncation.
 * All three genuine toSPR transfers and waits remain unresolved real calls.
 * No native fake-success body is provided; the native decoder takes explicit
 * bounded banks, including an explicit inherited VU-data snapshot. */
#ifndef DEADLOCKED_NATIVE
void func_L01_00583580(u8 *source,s32 qwords,u32 scratch_offset);
void func_L01_005835C0(void);
void func_L01_005894F0(float fraction);
void func_L01_0058A258(u8 *definition,float frame) {
    union { float value;s32 word; } rounded;
    s32 index,next,fixed_qwords,variable_qwords;
    u8 *stream,*current,*following;
    float fraction;
    __asm__ volatile("cvt.w.s %0,%1" : "=f"(rounded.value) : "f"(frame));
    index=rounded.word;next=(s32)((u32)index+1);
    stream=(u8 *)*(u32 *)(definition+0x1C);
    fraction=frame-(float)index;
    if(next==definition[0x10]) next=0;
    fixed_qwords=stream[1];variable_qwords=stream[2];
    current=stream+(((u32)fixed_qwords+(u32)index*(u32)variable_qwords)<<4);
    following=stream+(((u32)fixed_qwords+(u32)next*(u32)variable_qwords)<<4);
    if(fixed_qwords!=0) {
        func_L01_00583580(stream,fixed_qwords,0x2000);
        func_L01_005835C0();
    }
    if(variable_qwords!=0) {
        func_L01_00583580(current,variable_qwords,0x3000);
        func_L01_005835C0();
        func_L01_00583580(following,variable_qwords,0x3800);
        func_L01_005835C0();
        func_L01_005894F0(fraction);
    }
}
#endif
