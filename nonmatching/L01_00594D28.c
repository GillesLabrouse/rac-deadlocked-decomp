/* cflags: -ffast-math -mno-split-addresses */
/* asflags: --float-immediates */
/* Complete original scalar settling00594D28..00594DC4 (156 bytes).
 * The damping helper mutates rate before the original value/rate reloads.
 * NONMATCHING draft; no frame or physical FPU equivalence assertion. */
#include "common.h"
f32 func_L01_00594A88(f32 *,f32,f32,f32,f32);
f32 func_L01_00594D28(f32 *value,f32 *rate,f32 target,f32 force,
                    f32 damping,f32 maximum) {
    f32 current,difference,threshold;
    func_L01_00594A88(rate,target-*value,force,damping,maximum);
    current=*value+*rate;
    threshold=maximum*0.01f;
    difference=target-current;
    *value=current;
    if(__builtin_fabsf(difference)<threshold) {
        *value=target;
        *rate=0.f;
        return *rate;
    }
    return difference;
}
