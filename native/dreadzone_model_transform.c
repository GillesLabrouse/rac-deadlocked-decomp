#include "dreadzone_model_transform.h"
#include "dreadzone_general_matrices.h"
#include "dreadzone_variable_pose.h"
#include <stdint.h>
#include <string.h>

static u32 read32(const u8 *p) {
    return (u32)p[0] | (u32)p[1] << 8 | (u32)p[2] << 16 | (u32)p[3] << 24;
}
static int overlap(uint64_t a, uint64_t n, uint64_t b, uint64_t m) {
    return a < b + m && b < a + n;
}
static u8 *resolve(const DreadzoneModelView *views, u32 count, u32 address, u32 bytes) {
    u32 i;
    for (i = 0; i < count; i++) {
        if (address >= views[i].guest_base &&
            (uint64_t)address + bytes <= (uint64_t)views[i].guest_base + views[i].bytes)
            return views[i].storage + (address - views[i].guest_base);
    }
    return NULL;
}

int dreadzone_model_transform_static(const DreadzoneModelTransform *request) {
    DreadzoneModelTransform input;
    DreadzoneModelView views[16];
    u8 *actor;
    u32 i, j;
    if (request == NULL || (uintptr_t)request > UINTPTR_MAX - (sizeof(*request) - 1))
        return DREADZONE_TRANSFORM_INVALID;
    input = *request;
    if (input.actor == 0) return DREADZONE_TRANSFORM_NO_MODEL;
    if (input.views == NULL || input.view_count == 0 || input.view_count > 16 ||
        (uintptr_t)input.views > UINTPTR_MAX - input.view_count * sizeof(*views) ||
        input.scratch == NULL || input.scratch_bytes < DREADZONE_TRANSFORM_SCRATCH_BYTES ||
        (uintptr_t)input.scratch > UINTPTR_MAX - DREADZONE_TRANSFORM_SCRATCH_BYTES)
        return DREADZONE_TRANSFORM_INVALID;
    memcpy(views, input.views, input.view_count * sizeof(*views));
    for (i = 0; i < input.view_count; i++) {
        if (views[i].storage == NULL || views[i].bytes == 0 ||
            (uint64_t)views[i].guest_base + views[i].bytes > UINT64_C(0x100000000) ||
            overlap(views[i].guest_base, views[i].bytes, UINT64_C(0x70000000), DREADZONE_TRANSFORM_SCRATCH_BYTES) ||
            (uintptr_t)views[i].storage > UINTPTR_MAX - views[i].bytes ||
            overlap((uintptr_t)views[i].storage, views[i].bytes,
                (uintptr_t)input.scratch, DREADZONE_TRANSFORM_SCRATCH_BYTES)) return DREADZONE_TRANSFORM_INVALID;
        for (j = 0; j < i; j++)
            if (overlap(views[i].guest_base, views[i].bytes, views[j].guest_base, views[j].bytes) ||
                overlap((uintptr_t)views[i].storage, views[i].bytes,
                    (uintptr_t)views[j].storage, views[j].bytes)) return DREADZONE_TRANSFORM_INVALID;
    }
    actor = resolve(views, input.view_count, input.actor, 0x60);
    if (actor == NULL) return DREADZONE_TRANSFORM_UNMAPPED;
    if (read32(actor + 0x58) == 0) return DREADZONE_TRANSFORM_NO_MODEL;
    /* 0058A2E0's zero-variable branch targets 0058A314, whose JAL
     * still calls 005894F0. Fixed compressed channels must be decoded before
     * generating matrices. The previous identity specialization omitted this
     * call. This legacy request has no inherited VU input; use the explicit
     * fixed entry for its separately verified initial-frame domain. */
    return DREADZONE_TRANSFORM_UNSUPPORTED;
}

/* Bounded initial-frame model memory effects; see the public API contract. */
static int transform_initial(const DreadzoneFixedModelTransform *request, int variable_enabled) {
    DreadzoneFixedModelTransform input;
    DreadzoneModelView views[16];
    DreadzoneLocalPoseInit initialize;
    DreadzoneVariablePose decode;
    DreadzoneGeneralMatrices generate;
    u8 work[0x4000], inherited[0x1000];
    u8 *actor, *definition, *animation, *skeleton, *stream, *matrices;
    u32 i, j, count, fixed_bytes, variable_bytes, stream_bytes, definition_address, animation_address;
    u32 skeleton_address, stream_address, output_address;
    if (!request || (uintptr_t)request > UINTPTR_MAX - (sizeof(*request)-1))
        return DREADZONE_TRANSFORM_INVALID;
    input=*request;
    if (!input.model.actor) return DREADZONE_TRANSFORM_NO_MODEL;
    if (!input.model.views || !input.model.view_count || input.model.view_count>16 ||
        (uintptr_t)input.model.views>UINTPTR_MAX-input.model.view_count*sizeof(*views) ||
        !input.model.scratch || input.model.scratch_bytes<sizeof(work) ||
        (uintptr_t)input.model.scratch>UINTPTR_MAX-sizeof(work) ||
        !input.vu_data || input.vu_data_bytes<sizeof(inherited) ||
        (uintptr_t)input.vu_data>UINTPTR_MAX-sizeof(inherited) ||
        overlap((uintptr_t)input.vu_data,sizeof(inherited),(uintptr_t)input.model.scratch,sizeof(work)))
        return DREADZONE_TRANSFORM_INVALID;
    memcpy(views,input.model.views,input.model.view_count*sizeof(*views));
    for (i=0;i<input.model.view_count;i++) {
        if (!views[i].storage || !views[i].bytes ||
            (uint64_t)views[i].guest_base+views[i].bytes>UINT64_C(0x100000000) ||
            (uintptr_t)views[i].storage>UINTPTR_MAX-views[i].bytes ||
            overlap(views[i].guest_base,views[i].bytes,UINT64_C(0x70000000),sizeof(work)) ||
            overlap((uintptr_t)views[i].storage,views[i].bytes,(uintptr_t)input.model.scratch,sizeof(work)) ||
            overlap((uintptr_t)views[i].storage,views[i].bytes,(uintptr_t)input.vu_data,sizeof(inherited)))
            return DREADZONE_TRANSFORM_INVALID;
        for (j=0;j<i;j++)
            if (overlap(views[i].guest_base,views[i].bytes,views[j].guest_base,views[j].bytes) ||
                overlap((uintptr_t)views[i].storage,views[i].bytes,(uintptr_t)views[j].storage,views[j].bytes))
                return DREADZONE_TRANSFORM_INVALID;
    }
    if (input.model.actor&3) return DREADZONE_TRANSFORM_INVALID;
    actor=resolve(views,input.model.view_count,input.model.actor,0x100);
    if (!actor) return DREADZONE_TRANSFORM_UNMAPPED;
    output_address=read32(actor+0x58);
    if (!output_address) return DREADZONE_TRANSFORM_NO_MODEL;
    if ((actor[0x55]&4) || read32(actor+0x50) || read32(actor+0x5C) || read32(actor+0x44))
        return DREADZONE_TRANSFORM_UNSUPPORTED;
    definition_address=read32(actor+0x24); animation_address=read32(actor+0x40);
    if ((definition_address&3) || (animation_address&3)) return DREADZONE_TRANSFORM_INVALID;
    definition=resolve(views,input.model.view_count,definition_address,0x4C);
    animation=resolve(views,input.model.view_count,animation_address,0x20);
    if (!definition || !animation) return DREADZONE_TRANSFORM_UNMAPPED;
    count=definition[8];
    if (!count || count>127 || actor[0x57]!=count || !animation[0x10])
        return DREADZONE_TRANSFORM_UNSUPPORTED;
    skeleton_address=read32(definition+0x18); stream_address=read32(animation+0x1C);
    stream=resolve(views,input.model.view_count,stream_address,16);
    if (!stream) return DREADZONE_TRANSFORM_UNMAPPED;
    if ((!variable_enabled && stream[2]) || stream[2]>128 || !stream[1] || stream[1]>128) return DREADZONE_TRANSFORM_UNSUPPORTED;
    fixed_bytes=(u32)stream[1]*16;
    variable_bytes=(u32)stream[2]*16;
    stream_bytes=fixed_bytes+variable_bytes*(animation[0x10]==1 ? 1U : 2U);
    stream=resolve(views,input.model.view_count,stream_address,stream_bytes);
    skeleton=resolve(views,input.model.view_count,skeleton_address,count*16);
    matrices=resolve(views,input.model.view_count,output_address,count*64);
    if (!stream || !skeleton || !matrices) return DREADZONE_TRANSFORM_UNMAPPED;
    if ((skeleton_address&15) || (stream_address&15) || (output_address&15) ||
        overlap(output_address,count*64,input.model.actor,0x100) ||
        overlap(output_address,count*64,definition_address,0x4C) ||
        overlap(output_address,count*64,animation_address,0x20) ||
        overlap(output_address,count*64,skeleton_address,count*16) ||
        overlap(output_address,count*64,stream_address,stream_bytes) ||
        overlap(input.model.actor,0x100,definition_address,0x4C) ||
        overlap(input.model.actor,0x100,animation_address,0x20) ||
        overlap(input.model.actor,0x100,skeleton_address,count*16) ||
        overlap(input.model.actor,0x100,stream_address,stream_bytes))
        return DREADZONE_TRANSFORM_INVALID;
    memcpy(work,input.model.scratch,sizeof(work));
    memcpy(inherited,input.vu_data,sizeof(inherited));
    /* Actual 0058A440: skeleton DMA to2000, then00589368. */
    memcpy(work+0x2000,skeleton,count*16);
    initialize.skeleton=work+0x2000;initialize.skeleton_bytes=count*16;
    initialize.scratch=work;initialize.scratch_bytes=sizeof(work);initialize.joints=count;
    if (dreadzone_local_pose_initialize(&initialize)!=DREADZONE_GENERAL_MATRICES_APPROXIMATE)
        return DREADZONE_TRANSFORM_UNSUPPORTED;
    /* Actual0058A258 fixed DMA and unconditional005894F0, even with V==0. */
    memcpy(work+0x2000,stream,fixed_bytes);
    /* Positive-zero frame selects current0 and wraps next1 to0 for one frame. */
    if (variable_bytes) {
        memcpy(work+0x3000,stream+fixed_bytes,variable_bytes);
        memcpy(work+0x3800,stream+fixed_bytes+(animation[0x10]==1 ? 0 : variable_bytes),variable_bytes);
    }
    decode.scratch=work;decode.bytes=sizeof(work);decode.fraction=0.f;
    decode.vu_data=inherited;decode.vu_data_bytes=sizeof(inherited);
    if (dreadzone_variable_pose_decode(&decode)!=DREADZONE_VARIABLE_POSE_APPROXIMATE)
        return DREADZONE_TRANSFORM_UNSUPPORTED;
    generate.scratch=work;generate.bytes=sizeof(work);generate.joints=count;
    if (dreadzone_general_matrices_prepare(&generate)!=DREADZONE_GENERAL_MATRICES_APPROXIMATE)
        return DREADZONE_TRANSFORM_UNSUPPORTED;
    /* CPU-visible result of final fromSPR transfer, followed by actual dirty
     * clear. DMA registers, timing and VU state are outside this contract. */
    memcpy(matrices,work,count*64);
    memcpy(input.model.scratch,work,sizeof(work));
    actor[0x55]&=0xDF;
    return DREADZONE_TRANSFORM_APPROXIMATE;
}

int dreadzone_model_transform_fixed(const DreadzoneFixedModelTransform *request) {
    return transform_initial(request,0);
}
int dreadzone_model_transform_initial(const DreadzoneInitialModelTransform *request) {
    return transform_initial(request,1);
}
