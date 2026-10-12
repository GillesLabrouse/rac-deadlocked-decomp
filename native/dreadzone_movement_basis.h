#ifndef DEADLOCKED_DREADZONE_MOVEMENT_BASIS_H
#define DEADLOCKED_DREADZONE_MOVEMENT_BASIS_H
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
enum {
    DREADZONE_MOVEMENT_BASIS_APPROXIMATE = 1,
    DREADZONE_MOVEMENT_BASIS_INVALID = -1,
    DREADZONE_MOVEMENT_BASIS_UNSUPPORTED = -2
};

/* Selected RAM effects of actual L01 0x67C20C through 0x67C3B4, including
 * the final transpose store in the JAL delay slot before callee 0x67B9B0.
 * Read angles at record+B0/B4/B8 and publish two complete 4x4 matrices only
 * at record+20..9F. The second matrix is the transpose of the first.
 * Supply exclusive, truthful readable/writable backing of at least BC bytes;
 * byte alignment is sufficient. Inputs are consumed before output publication.
 *
 * Supported angles are finite normal/zero in [-pi,pi], with nonzero magnitude
 * at least 2^-40. Requires FE_TONEAREST and gradual host underflow, including
 * preservation of subnormal operands (no DAZ/FTZ arithmetic). Uses the
 * angle-pair API's explicit intermediate flushing model, then separately
 * rounded binary32 scalar operations whose results must be normal/zero.
 * Every refusal leaves caller memory unchanged. Floating exception flags are
 * outside the contract. APPROXIMATE excludes physical VU/FCSR behavior,
 * original CPU/VU/ACC banks, stack effects, call/return ABI and scheduling.
 * This does not invoke locomotion or perform a complete player update.
 */
int dreadzone_movement_basis_before_locomotion(uint8_t *record, size_t bytes);
#ifdef __cplusplus
}
#endif
#endif
