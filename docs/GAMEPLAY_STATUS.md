# Native playable-opening checkpoint

Current checkpoint: 2026-10-11. The Windows executable displays Dreadzone scenery
and a separate prepared animation diagnostic. It does not run a controllable
Ratchet or a complete gameplay frame. Matching coverage is 312 functions of
15,766 (0.558% code); this percentage does not measure playable readiness.

| Required gate | Verified pieces | Remaining integration |
| --- | --- | --- |
| Authentic scene loading | Original ISO gameplay-core provenance, terrain/static geometry and collision data; actual entry-to-loader/pool call edges. | Execute the allocation/layout and parser chain with its real cursor, globals, selected records and resources. |
| Actor and player construction | Full initial pool memory effects, reset, inventory, parameters, class publication/relocation and model allocation components. | Load the placed actors through `0057B788`; close model-aware class initialization, `00668070` and the actual child/backlink producers. |
| Repeated player update | Controller decoding and selected direction, walking, position and collision paths; timer, record and queue helpers. | Complete the reached frame paths, including `00684EF0`, callback `0066ACA0`, secondary movement and their real input/history/state producers. |
| Live visual output | Native terrain renderer, original body topology and validated prepared animation samples. | Bind the initialized actor's changing position, live animation and camera to the renderer across successive frames. |

The first playable milestone requires an authentically initialized player in
the opening scene, directional input, movement and floor collision, a following
camera, and stable repeated updates. A camera-only viewer, a prepared animated
mesh or isolated diagnostic RAM comparisons do not satisfy that milestone.
There is no measured schedule for closing the remaining dependencies yet.

## Latest startup component

`native/dreadzone_initial_pool.c` implements complete memory effects of actual
L01 `0057B6B8` with the real `0055D2A8` filler and `0057B6B0` identity-return
callee. GP, header and pointer-to-allocation-cursor are explicit inputs. It
publishes the pool base and counts, clears the reserved storage and two fixed
tables, and preserves ordered aliases. Even zero/negative filler sizes write
one word in the original; signed arithmetic traps and missing mappings are
atomic host refusals, whereas original faults may retain earlier writes.

The final central-library proof has 182 complete RAM matches, 12 original-fault
refusals, 1,423,530 original instructions and 763,363,328 compared RAM bytes.
An independent review adds eight successful alias cases and verifies signed
trap boundaries. All 91 native tests pass. The original gameplay core has a
nonzero `+30` record offset `1FE730`, whose first words are 401 and 400. Using
these two fields in a fixture does not establish the runtime allocation cursor
or sizing globals, nor does clearing the pool construct its actors.

The three original-compiler variants of this initializer remain nonmatching
and private. This native component adds zero accepted matching functions.
The latest two bounded exact-recovery lots accepted zero functions in 537
combined worker seconds; those drafts remain excluded from coverage. Parallel
work now prioritizes startup provenance and independent validation, with root
owning integration and all central builds.

## Run the currently available programs

From the repository root, `run-native.cmd` opens the scenery viewer;
`run-model-animation.cmd` opens the prepared body animation diagnostic.
Their camera controls and limitations are documented in `NATIVE_PORT.md` and
`NATIVE_MODELS.md`. Run `tools/check-native.ps1` for the native validation suite.
All disc-derived packages and private proof artifacts stay local and ignored.
