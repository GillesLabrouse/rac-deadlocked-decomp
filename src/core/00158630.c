#include "common.h"

typedef struct CommandPayload {
    s32 values[6];
} CommandPayload;

extern s32 func_00158C60(s32, s32, void *, void (*)(s32, s64), s64);

s32 func_00158630(s32 value, CommandPayload *payload) {
    struct {
        s32 value;
        CommandPayload payload;
    } parameters;
    parameters.value = value;
    if (payload != NULL) {
        parameters.payload = *payload;
    } else {
        parameters.payload.values[0] = -1;
    }
    return func_00158C60(0x60, sizeof(parameters), &parameters, NULL, 0);
}
