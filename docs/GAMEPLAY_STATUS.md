# Native playable-opening checkpoint

Current checkpoint: 2026-10-11. The Windows executable displays Dreadzone scenery
and a separate prepared animation diagnostic. It does not run a controllable
Ratchet or a complete gameplay frame. Matching coverage is 313 functions of
15,766 (0.559% code); this percentage does not measure playable readiness.

| Required gate | Verified pieces | Remaining integration |
| --- | --- | --- |
| Authentic scene loading | Original ISO gameplay-core provenance, terrain/static geometry and collision data; actual entry-to-loader/pool call edges. | Execute the allocation/layout and parser chain with its real cursor, globals, selected records and resources. |
| Actor and player construction | Full initial pool memory effects and allocation bounds/table setup, reset, inventory, parameters, class publication/relocation and model allocation components. | Load the placed actors through `0057B788`; establish the first player's source, close model-aware class initialization, `00668070` and the actual child/backlink producers. |
| Repeated player update | Controller decoding and selected direction, walking, position and collision paths; timer, record and queue helpers. | Complete the reached frame paths, including `00684EF0`, callback `0066ACA0`, secondary movement and their real input/history/state producers. |
| Live visual output | Native terrain renderer, original body topology and validated prepared animation samples. | Bind the initialized actor's changing position, live animation and camera to the renderer across successive frames. |

The first playable milestone requires an authentically initialized player in
the opening scene, directional input, movement and floor collision, a following
camera, and stable repeated updates. A camera-only viewer, a prepared animated
mesh or isolated diagnostic RAM comparisons do not satisfy that milestone.
There is no measured schedule for closing the remaining dependencies yet.

The latest model review withdrew `dreadzone_model_transform_static`'s complete
static-pose claim. Zero variable qwords still reach the original compressed
decoder, which the old implementation omitted. The entry now refuses all
nonempty models without writes. Fixed-channel decoding, full class initializer
composition and transaction-safe model-pool state are immediate construction
dependencies. The prepared animation diagnostic uses a separate path.
The fixed-only decoder component now passes 160 complete original EE/VU scratch
comparisons and three atomic refusals against the final native DLL. The full
model update and model-aware class initializer still need composition proof.
All 92 native tests pass after these changes in 5.75 seconds; these checks
include refusal regressions and do not demonstrate a playable frame.

## Latest startup component

Exact leaf `src/overlays/L01/0047FBF0.c` is newly accepted. Its complete 48-byte
reference includes four trailing padding bytes; both fresh and production
objects pass full unmasked resolution at independently located addresses in
all 46 catalogued overlays. It has no relocations. The original CFG returns one
for null and signed classes `1099`/`171C`, zero otherwise; all 65,536 class
halfwords and null are checked. This adds one distinct exact function, not 46,
and performs no actor construction. Full audit passes 313 exact/zero failures.

`native/dreadzone_pool_bounds.c` implements complete L01 `0057BAC0` RAM effects
through genuine `00578928`, `00586F78` and `0055D2A8`. It publishes ordered pool
boundaries and the free marker, advances the supplied cursor, creates and clears
the allocation descriptor table, and preserves all observable reloads/aliases.
The final central-library proof passes 205 whole-RAM comparisons, 15 actual
original-fault atomic refusals, 32,024 original instructions and 859,832,320
compared bytes. No callee is simulated. Its 68 focused checks and all 92 native
tests pass; the full suite takes 6.05 seconds. Original stack/register ABI and
fault partial writes are outside this host transaction contract. It adds zero
matching credit and does not initialize a player or alter the viewer.

An independent source walk finds the actual settings parser writes `0x169` to
`21EAC4` from core offset `3C0`. `21EAD4` starts at zero in L01 and resident
images, but its live value remains unproved. The 401 rows reached through core
`+30` have no `171C`/`23F7` class word. This is a bounded table result, not absence
from other data or later loaders. Constructor `00668070` scans the pool for
these classes when the first player receives a null actor; that actor's producer
still requires tracing, including the later core `+34` loader.

That later `+34` block has now been checked: its 192 entries are all invalid
and skipped by `0057BF78`. This excludes only that block. The entry's literal
loader arguments `(1,0)` select control paths; they do not identify the first
level. Live first-level/resource selection remains a separate prerequisite.
The known constructor's virtual reset clears the record's actor pointer;
it does not retain a previous player's pointer as a fallback.

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
