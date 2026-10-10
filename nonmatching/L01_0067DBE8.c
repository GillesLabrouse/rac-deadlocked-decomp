#include "common.h"

/* Complete 0067DBE8..0067DDD0, 0x1E8 bytes. ABI consumes four independent
 * integer arguments and f12/f13. The directional path really calls596390
 * after constructing both vectors. No collision/clip success is substituted.
 *
 * Direction comes from record270 when global21E878 is nonzero, or the actor's
 * E0 quad otherwise; the scalar signs differ. The global and actor pointer
 * are reloaded for the second vector after the first output write.
 * Dot products, ACC, RSQRT/Q, masked W lanes and QMFC2's low64 predicates
 * retain actual VU operations. Notably the first dot register starts with
 * ones: its unchanged y word means the first zero predicate is NOT a scalar
 * norm.x==0 check. The zero branch and Q operation are still preserved.
 * Output aliases see the original ordered stores, including three stores
 * to outputSecond on the nondirectional path. No native entry exists.
 */
#ifndef DEADLOCKED_NATIVE
extern volatile s32 D_0021E878;
void func_L01_00596390(u8 *,u8 *);
typedef union ActorStateFloatBits { f32 value;u32 bits; } ActorStateFloatBits;

void func_L01_0067DBE8(u8 *record,u8 *outputFirst,u8 *outputSecond,
    s32 directional,f32 distance12,f32 distance13)
{
    u8 *vector;
    ActorStateFloatBits scalar;
    u64 predicate;
    if(directional) {
        if(D_0021E878!=0) {
            scalar.value=-distance13;
            vector=record+0x270;
            __asm__ volatile("lqc2 $vf5,0(%1)\n\tvmaxw.xyzw $vf1,$vf0,$vf0w\n\t"
                "vmove.xyzw $vf6,$vf1\n\tvmove.xyzw $vf4,$vf5\n\t"
                "qmtc2.ni %2,$vf3\n\tvmul.xyzw $vf2,$vf4,$vf4\n\t"
                "vadday.x ACC,$vf2,$vf2y\n\tvmaddz.x $vf1,$vf1,$vf2z\n\t"
                "nop\n\tnop\n\tvrsqrt Q,$vf3x,$vf1x\n\tqmfc2.ni %0,$vf1"
                : "=r"(predicate) : "r"(vector),"r"(scalar.bits) : "memory");
        } else {
            scalar.value=distance13;
            vector=(u8 *)*(volatile u32 *)(record+0x2EF0)+0xE0;
            __asm__ volatile("vmaxw.xyzw $vf2,$vf0,$vf0w\n\t"
                "vmove.xyzw $vf6,$vf2\n\tlqc2 $vf5,0(%1)\n\t"
                "qmtc2.ni %2,$vf3\n\tvmove.xyzw $vf4,$vf5\n\t"
                "vmul.xyzw $vf1,$vf4,$vf4\n\tvadday.x ACC,$vf1,$vf1y\n\t"
                "vmaddz.x $vf2,$vf2,$vf1z\n\tnop\n\tnop\n\t"
                "vrsqrt Q,$vf3x,$vf2x\n\tqmfc2.ni %0,$vf2"
                : "=r"(predicate) : "r"(vector),"r"(scalar.bits) : "memory");
        }
        if(predicate!=0) {
            __asm__ volatile("vwaitq\n\tvmove.xyzw $vf4,$vf5\n\t"
                "vmulq.xyz $vf4,$vf4,Q\n\tvmove.xyzw $vf2,$vf4"
                : : : "memory");
        } else {
            __asm__ volatile("vsub.xyz $vf4,$vf0,$vf0\n\tvmove.xyzw $vf2,$vf4"
                : : : "memory");
        }
        __asm__ volatile("lqc2 $vf1,0xA0(%0)\n\tvadd.xyz $vf1,$vf1,$vf2\n\t"
            "sqc2 $vf1,0(%1)" : : "r"(record),"r"(outputSecond) : "memory");
        if(D_0021E878!=0) {
            scalar.value=-distance12;
            vector=record+0x270;
        } else {
            scalar.value=distance12;
            vector=(u8 *)*(volatile u32 *)(record+0x2EF0)+0xE0;
        }
        __asm__ volatile("lqc2 $vf4,0(%1)\n\tqmtc2.ni %2,$vf2\n\t"
            "vmove.xyzw $vf3,$vf4\n\tvmul.xyzw $vf1,$vf3,$vf3\n\t"
            "vadday.x ACC,$vf1,$vf1y\n\tvmaddz.x $vf1,$vf6,$vf1z\n\t"
            "nop\n\tnop\n\tvrsqrt Q,$vf2x,$vf1x\n\tqmfc2.ni %0,$vf1"
            : "=r"(predicate) : "r"(vector),"r"(scalar.bits) : "memory");
        if(predicate!=0) {
            __asm__ volatile("vwaitq\n\tvmove.xyzw $vf3,$vf4\n\t"
                "vmulq.xyz $vf3,$vf3,Q\n\tvmove.xyzw $vf2,$vf3"
                : : : "memory");
        } else {
            __asm__ volatile("vsub.xyz $vf3,$vf0,$vf0\n\tvmove.xyzw $vf2,$vf3"
                : : : "memory");
        }
        __asm__ volatile("lqc2 $vf1,0xA0(%0)\n\tvadd.xyz $vf1,$vf1,$vf2\n\t"
            "sqc2 $vf1,0(%1)" : : "r"(record),"r"(outputFirst) : "memory");
        func_L01_00596390(outputFirst,outputSecond);
    } else {
        scalar.value=distance13;
        __asm__ volatile("lqc2 $vf2,0xA0(%0)\n\tvmove.xyzw $vf1,$vf2\n\t"
            "qmtc2.ni %2,$vf3\n\tsqc2 $vf2,0(%1)\n\t"
            "vaddx.z $vf1,$vf1,$vf3x\n\tsqc2 $vf1,0(%1)\n\t"
            "qmtc2.ni $0,$vf3\n\tvmaxx.z $vf1,$vf1,$vf3x\n\tsqc2 $vf1,0(%1)"
            : : "r"(record),"r"(outputSecond),"r"(scalar.bits) : "memory");
        scalar.value=distance12;
        __asm__ volatile("sqc2 $vf2,0(%0)\n\tqmtc2.ni %1,$vf1\n\t"
            "vaddx.z $vf2,$vf2,$vf1x\n\tsqc2 $vf2,0(%0)"
            : : "r"(outputFirst),"r"(scalar.bits) : "memory");
    }
}
#endif
