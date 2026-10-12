#pragma once

#include <d3d11.h>

namespace dlnative {

// The caller must exclusively own this immediate context for the whole call.
// Complete its previously submitted commands, then clear its pipeline bindings.
// timeout_ms is a polling budget in [0, 30000]; zero refuses without submission.
// Driver calls and OS scheduling are not subject to a hard wall-clock deadline.
// Failure leaves bindings intact; submitted work cannot be rolled back.
// This PC service does not implement PS2 registers, VU reset or display setup.
HRESULT complete_and_reset_graphics(ID3D11DeviceContext* context, DWORD timeout_ms);

}
