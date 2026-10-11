# Native PC development

The local fork's intended playable target is a native PC application, initially
Windows x64. Upstream Deadlocked and rac1 are matching PS2 decompilations. Their
matching builds validate recovered source against the game; they do not produce
a PC executable. Keep that validation while developing the host runtime.

## Try the current local prototype

Once the Release viewer and local Dreadzone package are prepared, double-click
`run-native.cmd` in the repository root. The launcher resolves paths relative to
itself and starts inside the station. No emulator or BIOS is needed.

Use W/A/S/D to move, Q/E to move down/up, arrow keys to look, Shift to move
faster, and Escape to exit. These are letter keys, including on an AZERTY keyboard.
The camera has no collision. This is an interactive native scenery viewer;
Ratchet, gameplay, missions, audio, and the original startup loop are absent.

The launcher also forwards command-line options for repeatable captures:

```powershell
.\run-native.cmd --capture build/native-assets/dreadzone-launcher.png
```

To rebuild and run all native regression fixtures with the installed Visual
Studio 2022 C++ Build Tools, run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/check-native.ps1
```

The script finds CMake on PATH or through Visual Studio's installer, configures
Windows x64 Release, builds incrementally and propagates build/test failures.
Use `-TestRegex dreadzone` for the Dreadzone checks during a focused iteration.
It requires neither the PS2 compiler nor disc assets. This runs algorithm and
renderer fixtures; use `run-native.cmd` for the interactive scene viewer.

`run-model-animation.cmd` opens the latest original animation0 diagnostic:
the complete selected body, articulated by recovered native decoder, matrix,
binding and vertex stages. Its sampled playback has no character controls or
actor/game frame. `run-model-topology.cmd` retains the static native comparison;
`run-model-preview.cmd` retains the older incomplete editor-mesh comparison.
See [native model inspection](NATIVE_MODELS.md) for preparation and validation.

`run-controller-probe.cmd` runs the Windows XInput diagnostic through the
recovered controller poll, callback and decoder. It preserves button history
between samples. See [native controller testing](NATIVE_INPUT.md) for commands,
packet conversion and the attached-hardware validation still required.

## Compare native class loading with original instructions

After extracting the L01 overlay from your own disc, run from the repository root:

```powershell
venv/Scripts/python.exe tools/check_native_class_differential.py --overlay private/overlays/levels/campaign/1_dreadzone_station/overlay.elf --report build/drafts/native-class-differential.json
```

The tool discovers CMake through PATH or Visual Studio's installer and builds a
Windows x64 DLL from the six recovered class/geometry C sources and a source
bridge. It executes their genuine local instruction words with a bounded integer
interpreter and compares every mapped byte and host guard across 515 cases.
Seven invalid host domains are checked separately for atomic refusal. These are
algorithm fixtures, not a complete loader, GPU execution or a playable frame.

A build-input receipt binds the DLL to the current source, headers, bridge,
compile recipe and DLL hash. Stale or missing receipts force a clean rebuild;
explicit `--dll` reuse rejects them. Inputs are checked again after comparison.
An unsupported instruction, unmapped access, exhausted instruction budget,
native rejection or differing byte fails the command. The interpreter includes
no retail instruction list and does not emulate hardware, VU or floating point.
The ordinary Python tool tests require no disc or compiler.

## Verified first step

The complete constructor parameter initializer `00669A20` is also available as
`deadlocked_native_dreadzone_actor_parameters`. It writes eighteen 16-byte
prefixes within a guest-layout actor record and preserves every other byte.
Its host interface supplies one stable mode word for the six original global
reads. This is a complete CPU dependency, not the complete player constructor.
The native fixture checks guards and all actor bytes, and an independent
512-case instruction comparison agrees. The PS2 draft remains nonmatching.

`native/CMakeLists.txt` builds a native static library from eighteen recovered game
functions in `src/core` and `src/game`, without retail machine code or PS2 toolchain inputs.
An explicit source list excludes platform startup, drivers, and address-dependent code
that cannot run unchanged on a host. The original matching header remains intact.

The native `common.h` uses fixed-width integer types. SN's PS2 compiler uses
64-bit `long`; Windows x64 uses 32-bit `long`. Reusing `include/common.h` in a PC
build would truncate `s64` callback contexts. Host pointers also have a different
width from guest addresses; fixing scalar types does not fix serialized pointer
layouts, memory-mapped devices, or code that casts addresses to function pointers.

The test executable uses a capture transport in place of the still-unrecovered
command dispatcher. It checks actual recovered C functions for:

- 32-bit payload layouts and signed boundary values;
- positive and negative callback contexts larger than 32 bits;
- callback forwarding and the no-callback wrapper;
- present and absent optional payloads (absent tail fields remain unspecified);
- the existing guest-address transform as a numeric operation, never as a host pointer.

The library and test passed with MSVC 19.38 on Windows x64. This is a native code
validation milestone, separate from the renderer described below. The recovered
game update loop has not been ported; no game launch or playable scene has been
achieved.

## Recovered Dreadzone memory initialization

Two isolated Windows targets now execute actual L01 pre-loop memory algorithms:
`native_dreadzone_memory` and `native_dreadzone_record_reset`. They reset 64
sixteen-byte scratch records, initialize 256 records of stride `5C`, regenerate
255 free-list links, and reset twenty records of stride `340` plus seven counters.
The tests execute recovered C and the real word-fill algorithm; they require no
captured subsystem calls. Guest addresses remain 32-bit integers on Windows x64.

The pool reset preserves the final record's link. Its null termination depends
on the earlier entry bulk clear, rather than an invented terminating store.
Tests traverse all 256 records and check untouched fields, an arbitrary final
link, full word patterns and every byte in the twenty-record reset. The latter
sets two float fields to 40 and two short fields and one word to invalid values
after filling the entire `4100`-byte region with zero.

These routines are currently exercised in separate host fixtures. They are not
linked into the viewer or sufficient to initialize actors. L01 `00628020` and
helper `006A9AE0` pass actual-address byte comparison; `006A9640` and `006AF588`
remain nonmatching drafts, kept outside accepted coverage. The independent
canonical L00 scratch initializer has its own checked L00 data binding.

The pre-loop bulk clear `00529280` also runs through bounded guest-address
translation in `DreadzoneBulkArena`. It clears the actual half-open range
`00222900..003A0B40`, or 1,565,248 bytes. The caller owns the backing allocation
and binds it before entry execution. The real quad-fill helper and byte fallback
preserve their observed footprints, including a first quad store for an aligned
empty range. The fixture checks all cleared bytes, guards, unaligned host
placement and rejected out-of-range mappings. No PS2 pointer is dereferenced on
Windows. These algorithms share `deadlocked_native_dreadzone_memory` so they
can later be linked to real startup storage without recompiling test copies.

The real state initializer `0055DFE8`, table helper `0055DCF8` and selector
`0055E0A0` also execute through bounded guest storage. The two fixed tables each
contain 207 eight-byte records. The helper conditionally masks a record's first
word after reading its actual object byte, preserves its second word, publishes
three guest32 addresses, and the selector chooses one of three slots. The
native fixture checks all records, signed counts, enable/disable branches,
wrapped guest arithmetic, guards and rejected mappings before slot mutation.
Its 8,389,683 byte/field checks pass; all three PS2 drafts remain nonmatching.

`deadlocked_native_dreadzone_payload` executes the genuine descriptor allocator
and release algorithms, preserving their packed guest32 links and first-fit
search. Allocation rounds capacity to 64 bytes; the actor's clear step writes
four-byte words. The tests cover fragmentation, exhaustion, reuse, clearing and
corrupt chains. The actor initializer and its class/model effects remain separate
dependencies. See `DREADZONE_OBJECT_SETUP.md` for the instruction comparison and
the unfinished full actor constructor.

The pre-loop fill `005D6CB0` executes the actual core `0011A518` byte algorithm,
writing precisely 262,144 bytes of `EF` through the guest buffer pointer. All
sixteen alignment residues, byte counts around block boundaries, signed value
patterns, returned pointers and guard regions are verified. The matching
resident decision `00510EB0` also compiles natively and agrees with L01's real
`00666600` branch across tested levels and signed extremes.

## Recovered Dreadzone packet decoding

`deadlocked_native_dreadzone_input` builds the actual L01 decoder `005BDA40`
and recovered angle helpers `0055D708` and `0055D910`. Its isolated test executes
4,309 checks for buttons, edge masks, every axis byte, dead zones, pressure,
sample delays, ring wraps, mode/flip ordering, history and repeat. This isolated
decoder fixture performs the real caller's four previous-mask operations before
each controlled packet. The additional poll and platform targets described below
exercise configuration and transport separately.

The angle polynomial uses independently recovered coefficients and quadrant
pairs. A double-precision atan2 oracle measures a maximum circular error of
`3.34129247e-7` radians on the tested `33 x 33` finite grid. Scalar host floats
and sqrt do not reproduce VU accumulator precision, rounding or flags, and
threshold fidelity at adjacent float values is unverified. The verified input
domain is bounded finite channels and valid ring indices. These three sources
remain nonmatching and outside the scene viewer; no actor movement or game
frame has been added. See `DREADZONE_INPUT_PATH.md` for actual bindings and
`DREADZONE_OBJECT_SETUP.md` for the record producers still needed.

The library also contains the actual selective/history/delay resets and packet
callback `006B5CB8` with its real MSB-first bit writer. Separate fixtures execute
195 reset checks and 4,250 callback/encoding checks, including callback-before-
decoder order across four frames. These calls retain the original guest record
layout and context. No driver, actor frame or game input transport is supplied
by the fixtures. Their actual local bindings and limits are recorded in
`DREADZONE_INPUT_PATH.md`.

The real virtual reset `006A6598` initializes the complete `31F0`-byte actor
record and its embedded input context before callback execution. The fixture
verifies the entire byte image and 24 frames for each sample delay in `0..6`.
This establishes the context producer; the complete higher-level constructor
and its model/game dependencies remain unfinished.

## Native terrain loading and rendering

The Windows target `deadlocked_native_scene_viewer` loads a host scene package
and renders extracted terrain and static objects through Direct3D 11. It supports an interactive
free camera and an offscreen PNG capture, with hardware rendering or WARP.
This is a host backend written for the port, not recovered game logic. It does
not execute a MIPS binary, and it does not link the isolated command-test transport.

The first terrain-only checkpoint converted and rendered Dreadzone Station chunk zero:
42,930 vertices, 14,310 triangles, and 55 PNG textures in 55 draw batches. A room
capture was inspected, and both hardware and WARP captures completed. This package
contains terrain only; the static-object extension below adds the remaining terrain
and object placements. Characters, sky, missions, collision, audio and the recovered
frame/update path remain absent. Color modulation,
transparency and PS2 rendering fidelity still need verification.

Prepare the local assets after extracting the user's disc with Wrench v0.6:

```powershell
venv/Scripts/python.exe tools/prepare_native_scene.py `
  private/overlays/levels/campaign/1_dreadzone_station/chunks/0/tfrags/mesh.dae `
  --recover-unbound-colors -o build/native-assets/dreadzone-terrain.dlscene
```

The converter implements the relevant subset of
[COLLADA 1.4.1](https://www.khronos.org/files/collada_spec_1_4.pdf): position,
UV and color accessors, local Lambert/PNG material bindings, and triangle/quad
primitives in one identity-transform, unit-scale, Z-up scene. Unknown primitive
types, transforms, invalid index ranges, non-finite coordinates and missing or
escaping texture paths fail instead of silently losing geometry.

Wrench's inspected terrain export contains a position-aligned RGBA source that
is not referenced by the vertex inputs. The explicit `--recover-unbound-colors`
option reconnects it only when there is exactly one RGBA accessor with the same
record count as positions. Conversion records this adaptation and input/output
hashes in a local JSON report. Without that option, unbound colors stay unused.

The package stores versioned counts, bounds, PNG bytes, fixed-width vertex records
and material draw ranges. The native reader validates these before allocation or
draw submission. Texture slot zero supplies white for an untextured material.
Generated `.dlscene` packages and captures are assets and remain ignored/local.

Launch the terrain renderer from the repository root after building:

```powershell
build/native/Release/deadlocked_native_scene_viewer.exe `
  build/native-assets/dreadzone-terrain.dlscene --camera 600 480 203 -1.5707963 -0.05
```

WASD moves the free camera, Q/E moves vertically, arrow keys turn it, Shift moves
faster, and Escape exits. This camera has no collision or player behavior.
The supplied camera position is a developer inspection view, not a recovered
player spawn or original camera setup. The default camera frames the terrain bounds.

For a reproducible capture without opening a window:

```powershell
build/native/Release/deadlocked_native_scene_viewer.exe `
  build/native-assets/dreadzone-terrain.dlscene `
  --capture build/native-assets/dreadzone-room.png `
  --camera 600 480 203 -1.5707963 -0.05
```

Add `--software` to use WARP. Each capture writes a JSON report containing draw
counts, camera parameters, renderer type, and `game_logic_running: false`.
The native CTest suite includes scalar/command ABI checks, recovered startup
parameter and queue checks, finite transition progression, experimental request
consumption, malformed scene rejection, and a WARP draw/readback test. It
decodes an authored PNG and renders two differently placed and tinted triangles.
Python tests use authored synthetic geometry for the asset converter.

## Experimental request consumption

`deadlocked_native_consumer_tests` executes the recovered queue together with
the preflight and consumer drafts in `nonmatching/`. It links the matching state
reader, transition counter setter, status reader and float progression helper.
Its capture fixture observes unresolved transition calls; these captures are
test boundaries, not implementations of loading, rendering or game effects.

Routing cases cover both 17-entry tables from the resident image, signed states
outside those tables, pending-request changes during cleanup, delayed processing,
completion-byte timing, and transitions which retain the pending request across
multiple calls. A fixture checks the observed `14C` record stride and transfer
arguments. A queue-to-consumer sequence accepts successive requests after the
first request completes. This does not establish a frame loop or level launch.

The progression helper is exact in the PS2 build; its Windows branch preserves
finite signed-zero min/max ordering. Native arithmetic still uses host rounding,
and PS2 exceptional floats/FPU flags are not reproduced. Decimal threshold tests
must allow the observed floating-point convergence rather than assume that every
transition completes after a fixed number of frames.

The resident consumer is not automatically Dreadzone's consumer: L01 has a copy
at `005A9030`, with its own call/data bindings. See
[`DREADZONE_DEPENDENCIES.md`](DREADZONE_DEPENDENCIES.md) for the actual pre-loop
initialization chain and host boundaries still required by the campaign entry.

## Build and test

Use CMake 3.20 or newer and a C11 compiler. With Visual Studio 2022 C++ build tools:

```powershell
cmake -S native -B build/native -G "Visual Studio 17 2022" -A x64
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
```

For a single-configuration generator:

```sh
cmake -S native -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native
ctest --test-dir build/native --output-on-failure
```

The test needs no ISO, BIOS, emulator, original compiler, or extracted assets.
A playable native build will read assets extracted from the user's ISO. The
terrain renderer already consumes these assets through the conversion step above.

## Static-object placements and shared geometry

`tools/prepare_native_level.py` reads the local structured export text, resolves
the level's allowed static-object classes, and preserves model sharing in a
version-two host scene. Matrices retain their column-major affine representation;
the renderer applies their basis vectors and translations with `DrawInstanced`.
The version-one terrain format remains readable and renders the same room PNG
as before this extension.

```powershell
venv/Scripts/python.exe tools/prepare_native_level.py --extracted private/overlays `
  --level private/overlays/levels/campaign/1_dreadzone_station `
  -o build/native-assets/dreadzone-static.dlscene
build/native/Release/deadlocked_native_scene_viewer.exe `
  build/native-assets/dreadzone-static.dlscene --camera 600 480 203 -1.5707963 -0.05
```

The current Dreadzone package contains both terrain chunks and 1,856 static object
placements from 86 shared models: 197,412 shared vertices, 1,858 instances including
the terrain, 440 textures, and 461 material draw batches. It submits 672,516
triangles before GPU clipping. The second terrain export reuses the PNG material
set in chunk zero; this shared texture directory is supplied explicitly by the
level converter and recorded in its input hashes.

The text parser rejects malformed blocks and duplicate attributes. Class
resolution rejects unavailable level classes, duplicate instance IDs and mesh
paths escaping their class directory. Package validation includes affine matrices,
instance draw ranges, transformed bounds and bounded draw work. The WARP self-test
checks that two copies of one textured triangle receive distinct transforms and
tints, with the red copy to the left of the green copy. The authored texture also
exercises PNG decoding and texture channel order without game assets.

Static-object instance lighting is not reproduced: their exported ambient bytes
are not yet decoded and their tint is currently white. Dynamic objects, vegetation,
characters, collision, sky, original visibility/LOD rules and game behavior remain
absent. The level converter fails on missing static models rather than dropping
them, but does not claim to implement these excluded systems.

## Recovered startup parameter initialization

`src/game/0045CB88.c` is called from the resident startup loop before entering
level code. It reads nine scalar configuration values and two bytes, copies them
to eight slots, and initializes four state markers to `-1`. The routine's exact
160-byte match is verified after resolving its global addresses, and the same C
source passes a native test with signed boundary values and full-layout checks.
Its eight slots have not been assigned a gameplay interpretation.

The native core library includes this routine, five recovered queue routines,
the original seven isolated helpers, and the matched `00465C08` state reader.
The renderer does not yet invoke the recovered startup/update
chain. Tests of parameter initialization prove this routine's behavior, not boot
of the native game.

## Recovered request queue used by startup

`src/game/0045C768.c` and `src/game/0045C8C8.c` implement a ten-entry FIFO queue,
its reset wrapper, and single-request dispatch. They preserve the scalar layout
and pass both exact PS2 linking and native behavioral tests. The latter check
capacity, wraparound, lock transitions, untouched empty-pop outputs, request
arguments, all signed state boundaries, and reset after handler errors.

The queue-only fixture substitutes CRT `memset` for the unrecovered PS2 vectorized
byte filler. It captures the `0047F568` handler boundary and supplies test results;
it does not implement that handler's game behavior. The queue's request identifiers
and connection to gameplay still need interpretation. Its native test is separate
from the scenery renderer. See [the startup path](STARTUP_PATH.md) for verified
entry points and the remaining dependency chain.

## Experimental request preflight and dispatch

The separate `deadlocked_native_request_tests` target links the recovered queue
to `nonmatching/0047F568.c`, a typed draft of its real handler. It runs the
accepted `00465C08` state reader rather than replacing that call with a fixture.
The matching core and scenery viewer exclude the handler draft.

The test exercises accepted requests, all four rejection codes and their
precedence, eight-slot state history, untouched state after rejection, optional
completion-byte clearing, and preservation of a full-width host pointer. It also
checks zero, negative zero and NaN in the observed object field, the object scan
boundary, and queue reset when a pending request rejects a later dispatch.
Request and state identifiers still have no assigned gameplay meanings.

PS2 GP and absolute aliases refer to the same storage. The host test explicitly
maps each such pair to one scalar; it does not create independent copies.
Fixtures supply valid internal table indices and history counts. The only
substituted boundary in this target is the existing CRT byte filler.

The handler compiles to 508 bytes versus 512 in retail and fails comparison.
These native tests validate observed cases, not full semantic equivalence, game
boot or campaign play. Its algorithm stays outside accepted `src/` and matching
coverage while recovery continues.

## Priorities toward the beginning of the game

1. Recover the startup dependency chain and identify its platform interfaces.
   Resident entry `0x1574E8` reaches the startup loop at `0x157C58`; the drafts
   at `0x157738` and `0x1579A0` load executable sections. These routines copy
   code to PS2 addresses and call it. The host needs compiled functions and
   explicit level selection in their place, not calls into copied MIPS bytes.
2. Extend the static scene asset bridge to dynamic objects and characters from
   the existing local Wrench extraction. Guest pointers, offsets, and overlay-local
   identities need explicit conversion before game structures can be consumed.
3. Recover the menu/first-level update path and its rendering, input, audio,
   timing, and allocation interfaces. Implement host backends incrementally.
   Do not count a placeholder window or test transport as the beginning of the game.
4. Establish a real first-scene milestone: original scene assets rendered by the
   host, with the recovered update path and working input. Record which systems
   work and which remain incomplete before extending campaign play.

The matching pipeline remains available for exact regression checks. PCSX2 is
an optional reference for observing PS2 behavior; it and a BIOS are not native
build dependencies. Its preparation is deferred while native work takes priority.

## Acceptance

The native validation command `powershell -ExecutionPolicy Bypass -File
tools/check-native.ps1` builds and runs the recovered algorithm fixtures.
It currently includes model-pool allocation/release/compaction, class publication
and bounded relocation, and configured controller polling through the real
callback, decoder and button repetition. Platform driver calls remain captures
in those input fixtures; the Windows diagnostic uses real XInput calls with
feedback disabled. Class relocation now invokes recovered geometry packing
through its ordered write journal. It prepares real packet words while preserving
the upper halves of their quads. GPU submission and model transformations remain
outstanding. These libraries are not yet connected to the viewer's
runtime, so running their tests does not launch the game.

The actor field setter `0068E3B0` is also a separate library. Its mode-zero
mutations execute exactly within mapped host storage. Other modes return
`APPROXIMATE_VU`: bounded host float normalization and signed-zero handling are
tested, but VU precision, flags and scheduling are not reproduced. This setter
does not construct or update a playable actor.

The complete CPU state snapshot `0066A2A0` executes with its actual byte copier,
predicate and supported virtual getters. It resolves the mapped guest vtable
instead of taking a test callback. Unknown getters, missing mappings and late
reads aliasing the destination are rejected before mutation. The supported
memory domain agrees across 2,048 original-instruction cases. Snapshot capture
is a preparation dependency; the constructor and frame update remain absent.

The separate `deadlocked_native_texture_transfer_packets` library executes the
complete CPU builders `00138E08` and `00138FF0`. They prepare numeric 96-byte
upload and 112-byte readback packets with the real signed16 argument truncation
and return values. The native fixture checks 11,659 byte/guard and error cases;
1,594 successful original-instruction cases agree. Unsupported host storage and
oversized uploads are refused before mutation; the latter omits the original
PS2 diagnostic printer. The library performs no transfer or pixel conversion.
Both PS2 sources remain nonmatching and add no accepted coverage.

The constructor inventory reset `006C0D88` now executes with its real
`0055D2F8` clear helper. It clears both item arrays, restores invalid entry
markers and publishes the initialized byte while preserving untouched header
fields. Its mapped host domain requires guest16 alignment and signed-safe
addresses; the clear helper's host entry accepts nonnegative lengths divisible
by16. The complete PS2 draft retains prefix/tail stores for other layouts.
All mapped bytes and guards agree across 1,024 original-instruction cases.
This resets storage only; it does not grant items or complete actor creation.

The CPU texture pixel helper transforms the actual indexed byte stream for the
successful `00575180` upload/readback sequence. Page-aligned rectangles with
width divisible by128 and height divisible by64 are supported, including both
original armor0 PIF images. Smaller mip rectangles are refused atomically.
Its independently expressed coordinate relation agrees with a separate VRAM
oracle using public [GS layout facts](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/GS/GSTables.cpp).
No reference implementation or table is vendored. Physical-console capture
remains unverified; this helper performs no DMA, synchronization or Direct3D
texture upload. Converted bytes are a transfer stream, not a linear display
image. The fixture also checks descriptors stored inside scratch.

The complete class texture producer `0057A658` is reconstructed as PS2 source
only, outside native CMake. It retains genuine pixel conversion, upload, syscall,
completion and class-publication calls. Its descriptors occupy an uncleared
stack frame; fields the original never writes are preserved as unspecified
inputs, and the draft's different stack layout remains a validation limit.
Its 256 complete CPU return paths agree with linked SN instructions; two real
PIF prefixes agree up to the first conversion call, which is not executed in
that diagnostic. Positive-count texture loading is still unvalidated end to end.

`texture_layout_prepare_cpu_effect` composes the actual packet builders and
pixel permutation. It exposes both packets and mutates the indexed backing
only after validating all four disjoint host buffers and three guest ranges.
Borrowed request descriptors are copied before mutation, including when stored
inside an output buffer. Its result `CPU_EFFECT_PREPARED` explicitly covers CPU
preparation; it differs from the original `00575180` hardware return value.
The complete PS2 wrapper preserves all eight real calls but remains nonmatching.
The native adapter does not execute those calls or mark a class texture loaded.

The complete CPU animation setup `00688FE0` executes its actual node release
and metadata helpers. The original clears the head before reloading its next
pointer, so only that head is released; old tails and self pointers are not
traversed. The mapped host domain supports animation indices0..255 and integer
frames0..2^24, whose float conversion is exact under all rounding modes.
Descriptors, mapped reads and writes are validated before mutation. All mapped
bytes agree across4,096 original-instruction cases. This selects animation
data and updates fields; transforms, playback and the actor constructor remain
outstanding. All three PS2 sources remain drafts outside accepted coverage.

The whole player constructor `00668070` is reconstructed as PS2 source, including
all five inputs, pool scan/reclassification, real collision calls, VU orientation
and normalization, virtual reset, inventory calls and final field writes. Root
independently compiled and linked its complete4,104-byte original span. Its
19 distinct direct targets and branch boundaries are verified; this is structural
validation, not execution. Even the supplied-actor path still needs VU C60,
collision `0053DC70`, actor synchronization `0066E580`, transform `005617A8`
and complete inventory producers. These dependencies determine the next work.

The full model-update orchestrator `0058A648` compiles to an exact private
L01 linked comparison over88 bytes, including padding. Its two genuine callees
prepare scratchpad transforms and launch fromSPR DMA; the wrapper does not wait
for completion. It has no native body that clears the dirty bit while bypassing
those effects. Its canonical accepted overlay identity remains unresolved, so
this diagnostic adds no accepted matching coverage.

The constructor's complete random dependency `00163408` and range wrapper
`0058FF38` now execute with caller-owned seed storage corresponding to
`0021DC80`. They retain the actual32-bit seed transition and15-bit result,
including signed/wrapped range widths and the R5900 zero-divisor remainder.
All8,192 original-instruction cases agree across seed storage and guards.
This provides deterministic game RNG; it does not initialize the player.

The complete inventory setup `006909C8` now executes18 recovered routines
and the existing real reset/clear helpers. Its supported paths grant the default
level-zero loadout in mode0, attach an existing mode0 inventory, or reset the
indexed mode1 inventory. Tables and globals are immutable caller mappings;
no extracted table is embedded. Full preflight rejects unavailable progression
or notification paths before mutation. All mapped memory agrees across2,048
original-instruction cases with19 genuine callees. This initializes inventory,
not the whole player constructor.

`dreadzone_model_transform_static` prepares the full CPU-visible memory effects
of the normal static-animation model path, including the real107-joint Armor0
skeleton, hierarchy translations, both scratchpad banks and dirty-bit update.
Its identity quaternion/unit-scale specialization follows actual initialized
inputs when the animation stream has zero variable qwords. Compressed animated
streams, blend lists and override lists are refused atomically. An independent
executor of the original VU0738/0858 pairs agrees across129 cases under host
IEEE arithmetic, including original Armor0 inputs. The result remains
`APPROXIMATE`: PS2 rounding, DMA/VIF and VU internal state are not reproduced.
The viewer still needs original joint weights and their actual consumer before
these matrices can transform its mesh.

`dreadzone_bind_pose_prepare` now composes the genuine64-byte pose records
with the supplied48-byte compact binding records in the original A40/AA8
order. It supports count0..127 and the real existing-scratch pose bypass,
copies bindings into scratch, and preserves untouched bytes. All descriptors
are retained before output writes; invalid mappings and unsupported arithmetic
fail atomically. Root's fresh DLL agrees across257 original-microprogram cases
under host IEEE arithmetic, including all107 Armor0 joints. Its result remains
`APPROXIMATE`; it executes no DMA or VU internal state. The resulting translations
do not establish a visually correct rest pose. Original vertex weights, joint
association and the subsequent rendering consumer remain necessary.

The complete CPU equipment maintenance `00690B60`, slot deletion `006C0A58`,
marker clear `00690C88`, reset `005E9258` and bounded setter `00688578` now
execute with their genuine predicates and inventory helpers. Preflight verifies
every selected-path mapping and rejects unresolved UI dispatch before writes.
Maintenance supports the real initialized flag266D=1 when2318 does not match
an active marker. A signed category-2 setter genuinely clears266D through its
preceding word store; that CPU path is retained. Ordinary flag1 setter dispatch
uses the separate bounded selection composition described below. All mapped memory agrees
across4,096 original-instruction cases with ten actual CPU callees. This does
not complete fresh-player construction, UI rendering or a native game frame.

Complete actor synchronization sources `0066A678`, `0066A6F8`, `0066E580`
and `0058DF58` retain the original oscillator, collision-volume, spatial and
model calls. Root independently rebuilt and strictly linked their complete
spans; all four remain nonmatching PS2 drafts with no native entry. Original
and compiled instruction streams agree across256 complete inactive-state,
negative-actor-byte20, absent-volume returns, executing the genuine copy and
adjustment helpers. Raw COP1 word transport is checked without approximating
floating arithmetic. Active oscillation, VU B58, collision and model submission
remain unexecuted dependencies; this evidence does not establish actor motion.

The complete bounded CPU spatial update `0058DC78` now executes the genuine
bitmap helpers `0058DBA0` and `0058DC00`. It preserves cell membership order,
aligned allocation, growth/shrink hysteresis, packed actor.A4 publication and
the original count-byte wrap. Root independently compared515 scenarios across
original instructions, genuinely linked compiled PS2 instructions and a fresh
native DLL:67,963,520 compared bytes agree. All177 native checks pass.
Host descriptors and complete backing spans are preflighted before writes;
invalid state, missing membership and allocation exhaustion leave output
unchanged. Retail can trap or leave its bitmap mapping after earlier writes;
the native refusal is explicit, not an original successful return. This adds
spatial storage, not collision evaluation, VU actor transformation or motion.
All three complete PS2 drafts remain nonmatching outside accepted coverage.

`dreadzone_vertex_blend_prepare` executes the actual local matrix transfer,
two/three-way blend and position stages consumed by `0053C818`. Original
byte weights divide by256. Matrix caches and512 vertex IDs persist between
packets; IDs retain their seven-record delay, epilogue and duplicate behavior.
Root's fresh DLL agrees in60 original-instruction-pair cases covering120,318
vertices, including all28 Armor0 packet ranges and arbitrary joint matrices.
The entire retained state, associations, source bytes and output match under
host IEEE arithmetic;364 native checks also pass. Publication is atomic except
for caller-designated temporary workspace. Status remains `APPROXIMATE`:
lighting, normal decoding, packed VU1 output, world/view scale, physical VU
rounding and DMA synchronization are outside these local mathematical stages.
The viewer needs genuine topology/ST and the actor's active bangle mask; no
weights or associations are inferred from the editor GLB.

`dreadzone_camera_init` executes the complete CPU-visible `005617A8` initializer
under an explicit host IEEE approximation of the actual C60 microprogram.
`dreadzone_angle_pair` retains C60's reflection, polynomial coefficients and
separately rounded multiply/ACC order. No ordinary sin/cos replacement is used.
Two position reloads and the final global0021E754 read observe preceding output
writes when aliased; untouched fields and raw position.w are preserved. Root's
fresh DLL agrees in576 cases against original EE plus actual C60 and compiled
PS2 instructions, as well as1,030 separate angle inputs;140 native checks pass.
Unsupported mappings or arithmetic refuse atomically. Physical VU/EE rounding,
flags, register effects and timing remain outside the `APPROXIMATE` contract.
This supplies one constructor dependency, not a playable camera/game loop.

The equipment UI CPU chain `005D7558` / `005E93D8` now preserves actual item
selection, recent-queue insertion/rotation and dirty/summary flags in a bounded
fresh-selection domain. Its14 complete PS2 routines retain every real callee;
native preflight supports category-zero definitions with nonzero28, index0..1,
force1 and both state24/28 fields initially zero. Secondary/reentrant paths
refuse before writes. Root's fresh DLL agrees across4,096 original-instruction
cases with13 actual CPU callees, including1,024 real item2 zero-queue cases
and1,035 queue rotations. The paired-copy helper's unused lookahead word is
also checked before mutation. This is CPU queue/state behavior; the separate
flag1 setter now has its genuine virtual getter binding below. No UI rendering,
audio, completed actor or playable game frame is added.

`dreadzone_equipment_selection_00688578` now connects the genuine flag1 setter
to its virtual +4C getter and the complete bounded equipment UI chain. Actual
getters006A9630/00690EA0 return byte2FEA/-1; both privately match their eight
original bytes. Constructor reset clears2FEA; outer initializer006A8420 restores
the player index after the constructor returns. This composition does not run
either whole initializer. Category-zero, supported fresh UI state and mapped
actor0/1 are required; other domains refuse atomically. Root's fresh DLL agrees
across4,096 complete RAM cases and512 standalone getter cases, including256
writable virtual-entry aliases and1,024 actual item2 zero-queue selections.
All7,504 native checks pass. Mapped RAM is copied, actual ordered setter/getter/UI
effects execute, then writable images are published. Stable exclusive caller
access is required. No UI rendering or playable actor frame is added.

Complete actor-state PS2 drafts0067DDD0,0067DBE8,00596390 and00667050 now
retain the real VU/Q, ground/surface and side-probe dependencies. The main
3256-byte function preserves25 direct calls to16 genuine targets and all five
CVT.W.S instructions. Only the8-byte backlink is privately exact; the other
three are nonmatching. Root's fresh objects agree across512 whole non-directional
vector-helper returns with original instructions, including ordered aliases
and raw W. No native main-state adapter, directional Q validation or successful
collision substitute is introduced. Collision packet production and the
remaining callbacks are required before this can participate in a native frame.

The [original model diagnostic](NATIVE_MODELS.md) now joins genuine native local
vertex calculations to original VIF/ST/ID topology. Its107 explicit pose/binding
matrices produce3,478 vertices and2,714 raw FIFO triangles from37 primary packets.
The primary-only comparison shows the textured torso and arms. Explicit
fresh-boundary mask1002 now selects primary, bangle12 and bangle1:46 packets,
4,238 computed vertices and3,326 triangles. Native candidates preserve actual
descending-bit ordering and low-to-high fallback, including missing primary
low forcing high for later bangles. They do not execute LOD distance, render
queue budget or complete actor construction. The selected static pose remains
fixed with outstretched arms; animation, actor/world/view, clipping and lighting are absent.

`dreadzone_actor_basis` now computes the complete CPU-visible B58 basis using
the actual C40 polynomial pair and B10 ordered matrix products. Root's fresh
DLL agrees across1,039 cases,62,627 original VU pairs and66,496 output bytes,
including all active-axis combinations and signed-zero endpoints.202 native
assertions pass. The contract remains `APPROXIMATE` host IEEE arithmetic;
physical VU flags/timing and the outer actor render/update are not executed.

`dreadzone_collision_tree` implements the entire integer0053DBD0 hierarchical
Y/X/Z lookup with actual root-relative offsets and delay-slot overflow checks.
Root's fresh DLL agrees across1,027 fixtures,19,059 original EE instructions,
8,413,184 table bytes,106 nonzero leaves and27 overflow refusals.25 native
assertions pass. A returned leaf is collision data, not an established contact
with the ground; the full contact probe and movement caller still need recovery.
Root also rereads the compressed assets directly from ISO and checks every
one of the8,249 actual nonempty terrain cells against original0053DBD0:
313,462 integer instructions, unchanged backing and guards. This verifies
the entire actual tree's selected leaf values, not all collision geometry.

`dreadzone_collision_triangle` now executes one complete original triangle
iteration0053E1E8..0053E384: clip/surface/orientation filters, plane crossing,
Q division, all three edge tests, strictly nearer fraction and full XYZ
publication plus the real source guest address and surface+1000 hit metadata.
Root's fresh /fp:strict DLL agrees in3,327 fixtures, including3,072 cases
from611 authentic terrain triangles, plus128 partial request/output aliases:
329,995 original instructions,2,552 contacts,767 misses,8 numerical refusals
and183,744 published bytes. All39 native assertions pass. Original arithmetic
and delay slots are decoded without captured calls, under explicit ordered
host IEEE approximation; physical VU/Q flags, saturation and timing are absent.

The real fixture segments stay within their source cells after global f32
quantization, so actual traversal fractions0/1 produce the verified clip bounds.
The strict triangle API still refuses zero denominators atomically. Its second
API now follows the same complete calculation with signed FLT_MAX Q saturation
for zero denominators, exposing whether division executed and the numerical
I/D condition. Early filters can skip division; a saturated Q still follows
the original plane and edge tests. Root's fresh DLL agrees across3,331 fixtures:
2,552 contacts,775 misses,4 refusals,8 saturated divisions and32 request/status
aliases, with330,287 original instructions. All56 focused assertions pass.
Physical VU exception registers and timing are outside this numerical contract.

`dreadzone_collision_grid` supplies the actual0053DDE0..0053DF84 axis lists and
stable packed-cell merge, retaining X/Y/Z tie priority, signed-word fraction
clamping, first record and terminal sentinel. Identical truncated integer
endpoints genuinely return only the first record. Each axis is bounded to79
crossings, excluding the original80th-entry path that counts an unwritten
fraction. Zero-axis reciprocals retain explicit Q saturation diagnostics.
Root's fresh DLL agrees in529 original-word cases,4,810 records,38,480 bytes
and174,958 instructions, including382 cases with zero-Q axes. All30 assertions
pass. Outputs are atomic, request/output aliases are supported and output/output
aliases refuse. Arithmetic remains an ordered host-f32 approximation.

Subnormal/nonfinite inputs and unsupported arithmetic refuse atomically.
Packed face/quad iteration, final unit conversion,
dynamic actors, contact cache, DMA and complete player motion remain unexecuted.

`dreadzone_actor_input_bridge` connects actual006A7D48/0067BD58 to the recovered
predicate, signed timer, input-mask remapper, xyz magnitude and angle polynomial.
It preserves button/timer writes before the one-shot flag read and the second
axis reload after the first axis store. Actor128 remains an actual magnitude
component. A nonzero0021E8F4 is genuinely cleared and skips floating work.
Mapped stable RAM is copied transactionally, preserving guest aliases and
publishing writable images only after success. No controller transport or
position integration is added by this API.

Root's fresh DLL agrees across4,096 complete one-shot bridge cases,4,096 button
cases,65,536 signed timer values and4,096 remaps:1,328,033 original integer
instructions,142 distinct sites and256 ordered global/timer alias cases.
Every selected CPU callee runs without captures. The active magnitude/angle
branch is separately labelled `APPROXIMATE_VU`, with2,048 independent finite
numeric fixtures and256 actual decoder-to-bridge packet cases. These do not
establish physical VU equivalence. All153,170 native assertions pass. Root's
fresh SN objects compile and strictly link all five complete PS2 drafts;
all are nonmatching. No playable actor frame is established.

`dreadzone_actor_direction` now executes selected complete direction0067C5F8
and rotation0067CFD0 paths with genuine history, getter, angle-wrap, projection
and scalar turn callees. Input axes produce the real direction vector,
magnitude and facing fields; rotation updates facing and angular-speed state.
The native contract requires actual0021E878=0 and actor2659=0, preserving the
larger nonzero-mode dependencies as atomic refusals. Body and camera matrices,
history and boost globals remain supplied original state, not invented inputs.

Root's fresh DLL agrees with actual-word approximate COP1/VU execution in
4,096 direction cases, 1,024 turns, 128 ordered aliases and 512 scalar cases,
including 256 angle/speed pointer aliases: 1,583,894 instructions at 697 sites,
all ten selected callees executed and no captures. Full supplied RAM agrees
apart from explicitly tolerated floating outputs; the largest observed
absolute difference is 1.1920929e-7. All 11,453 native assertions and 58
independent diagnostic assertions pass. The diagnostic uses approximate
binary32 arithmetic and excludes physical PS2 ACC precision, rounding,
flags and timing. Root's fresh SN builds strictly link all ten complete PS2
drafts; all remain nonmatching.

Root also reads the genuine L01 gameplay core directly from ISO97BC8900,
located by primary header+60 rather than the sound-bank level header+20.
The decoded core has 2,390,596 bytes, settings at90 and settings+2C=0 atBC.
The actual nine-instruction producer57B0E8..57B108 executes its source read,
global clear and taken branch/delay, retaining0021E878=0 with all RAM guards
unchanged. This establishes the real zero-option input boundary, not execution
of the complete parser, disk driver, constructor or playable actor frame.
Position integration and the actual movement caller remain outside this API.

`dreadzone_animation_replacement` executes the full CPU memory effects of
original00575A18, transferring animation pointers and metadata into retained
geometry and reconciling actor.half36 bit8. The source pointer count is reloaded
after ordered writes; even overlapping definitions retain the original effects.
Mapped numeric guest addresses remain32-bit. Stable view/request descriptors
are snapshotted, all writes are preflighted and failures publish nothing.
The explicit pool bounds are immutable; writes aliasing their original global
words are outside this domain.

Root independently rebuilds the host DLL and all four complete PS2 drafts,
then replays513 cases through the shared raw-word integer CPU:581,137 original
instructions,655,806 freshly compiled SN instructions,7,309,952 bytes and zero
differences. The real case uses fresh direct-ISO class23F7 and Armor0 data,
transferring112 authentic animation pointers for107 joints. All3,145,994 focused
assertions pass. The publication helper00575BD0 is privately exact16/16;
005756D0/00575A18/00575B88 remain nonmatching and retain their actual dependencies.
No disk success substitute, GPU upload, pose decode, animated rendering or
complete native actor startup is supplied by this adapter.

`dreadzone_variable_pose` now performs the CPU-visible normal5894F0 decoder
memory effects with actual VU160/250 interpolation order. Explicit inputs are
the initialized 16 KiB scratchpad with fixed/current/next stream banks and
the inherited 4 KiB VU data image. Low VU metadata lanes retain their supplied
values; high-address gathers must have been initialized by the decoder's own
cache writes. Unsupported/truncated streams refuse without publishing writes.
The adapter preserves packed opcode0/24/48 handling, signed byte/halfword wrap,
injected indices, alternating banks and the three real priming groups.

Root's fresh DLL agrees with actual EE/VU replay and a separate packing
derivation across 210 genuine animation0 cases: all 30 frames at seven
fractions, including29-to0 wrapping, with varied explicit inherited VU data.
All 3,440,640 scratchpad bytes agree, including untouched stream banks;
776,160 original EE instructions and 418,740 VU pairs execute. The microprogram
pairs through328 are checked against the actual resident MPG payload1170E0.
All 20,507 focused assertions pass. Arithmetic is explicitly approximate
host binary32. Results remain encoded local pose values, not final joint
matrices. The adapter excludes DMA/waits, blend330/620, matrix generation738/858,
physical VU state publication, an animation update and visible animated rendering.
The complete PS2 function5894F0 has not yet been recovered as a C draft.

`dreadzone_terrain_contact` composes the genuine static-terrain stage of
0053DC70: ordered grid traversal, actual physical tree axes (root Z, row Y,
node X), prefetch/copy effects, clipped packed-cell decode, primary triangles,
quad mutation and closest-contact termination. Endpoints use world units;
contact fields retain the original 1024-scaled units. The recorded root is
diagnostic provenance, not the final game object's pointer. Misses leave the
contact unchanged; unsupported input leaves both contact and work information
unchanged. Original DMA/cache timing, dynamic actors and final probe packet
publication remain outside this contract.

Root's fresh strict-float build agrees with the original terrain stage across
10,998 fixtures covering all 8,249 nonempty original cells and additional
multicell/oblique paths: 10,774,074 EE instructions, 17,954 real tree calls,
14,040 modeled memory-copy effects and no captured callees. All 189,072 hit
contact bytes and work counters agree, including 984 quad-winner cases.
There are 2,626 hits, 8,358 misses and 14 atomic numerical-domain refusals.
The complete replay guards source and oracle hashes before and after execution.
This validates bounded CPU effects with ordered IEEE approximations, rather
than physical PS2 arithmetic or a complete player collision response.

`dreadzone_actor_velocity` executes actual speed progression 0067D350,
velocity production 0067D3A0, normalization 0067D480 and planar projection
0068AC58 for actor byte2659=0. Velocity consumes the real target/speed,
pitch and selected heading, evaluating the recovered C60 pitch/heading
microprogram in its original order. Stable mapped views preserve ordered
guest aliases; all supported writes publish transactionally. The numerical
domain and unsupported modes are explicit in the public header. Inputs,
elapsed-time globals and initial player state are not manufactured here.

Eight complete PS2 drafts compile and strictly link as nonmatching. Root's
fresh integrated diagnostic DLL agrees with original and separately compiled
instruction execution on 3,072 cases: 212,019 EE instructions and 77,952 actual
C60 pairs. All supplied RAM agrees outside declared bounded float tolerances;
the largest observed absolute float difference is 2.3841858e-7. The native
suite includes 1,552 velocity checks and 27 terrain checks; all 59 registered
tests pass. Speed/velocity output does not yet apply the full original position
update, resolve the capsule/static-body contacts or run a playable frame.

`dreadzone_actor_speed_target` now calls the real direction gateway and derives
target1780 from mapped coefficient/time globals and actor state. Its turn entry
derives the real force, damping and maximum arguments before calling 0067CFD0.
No target speed or turn parameters are injected by this API. Root's fresh
integrated DLL agrees with original and separately compiled caller bodies on
6,272 cases, including 1,024 ordered coefficient aliases and 128 actual
input-bridge/target/turn sequences: 4,009,074 instructions and maximum absolute
float difference 4.7683716e-7 within the declared approximate domain. There are
6,261 native completions and 11 atomic refusals. Eight sequence refusals expose
the inherited small-angle read limit; those outputs are preserved rather than
replaced with zero. That milestone's small-angle limitation is addressed by
the bounded extension described below.

The velocity API also exposes the already recovered vertical projection and
complete accumulated-motion helper 006834C8. It reduces actor1A0's planar
length and adds XYZ to actor130, preserving raw W; the original threshold
branch clears all four accumulated words. Signed reductions retain original
behavior. Root compares 2,048 original/rebuilt helper cases, including all four
PS2 modes and 512 supported native cases: 361,216 instructions, all three real
callees executed and no captures. Native velocity now has 1,594 focused checks.
Only the original helper effects are implemented; positionA0 is not written.

`dreadzone_animation_clock` advances the genuine primary temporal state of
00589CE0 using the selected definition, per-frame durations and effective rate
already supplied by its actual caller. It retains forward/reverse wrap/clamp,
the original reverse-clamp duration order, flags and ordered publication.
No clip name or wall-clock frequency is assigned. Linked layers and negative
fractional elapsed values are refused atomically. The latter still depend on
EE adder alignment behavior not established by the finite chopped-float model.
Root's fresh original/SN/native replays compare 911 dyadic cases and 893 extra
mixed-duration cases (including durations3/5/7/31/255/32767) with zero byte
differences. The two runs compare 461,824 actor bytes; unsupported cases are
reported separately. All 16,335 focused checks pass. The visible viewer now
uses this primary clock to select exact prepared pose labels as described below.

`dreadzone_animation_0058ACB8` now cleans the complete secondary-layer list
through real 0058A8D8 release effects. The next link is captured before clearing
each node, retaining original cyclic-list termination and signed metadata
aliases. A byte journal preflights every release before publishing any writes.
Root's fresh integrated DLL agrees with original/SN execution on 1,024 cases:
731,870 original instructions, 730,910 rebuilt instructions, 3,448,832 bytes
and zero differences. This includes 310 cycles and aliases to future node
fields or occupancy bytes. The complete retail body is 88 bytes, rather than
the splitter's truncated 80. Full 589CE0 layer-weight/unlink composition is
provided by the separate adapter described below. At this milestone all 61
registered native tests passed in 4.28 seconds; no complete
player/state/game frame is claimed.

The height-offset entry `dreadzone_actor_height_offset_0068A178` now performs
the original mode 0 source-Z subtraction while preserving destination X/Y/W.
This differs from vertical projection's raw source-lane transport. Its snapshot
retains source/destination aliases and signed-zero subtraction, with atomic
refusal on unsupported modes, floats or memory. The complete PS2 draft includes
modes 1/2 and their actual projection calls; it remains NONMATCHING (252-byte
entry plus a local normalization helper versus the original 456-byte span).
Root compares 2,048 original/SN cases and 512 native cases, including 293 aliases
and 128 exact signed-zero cases: 363,008 instructions and zero observed difference.
Native velocity now has 1,639 focused checks. No position update is added.

`dreadzone_static_body_contact` implements complete 00542280 supplementary static
contact and its reverse 64-contact collection mode. The mapped relocated table,
original source preloads, packed coordinates, contact packet publication and
incoming VU W lanes are explicit. Signed broad-phase overflow and unsupported
numeric/memory cases refuse before publication. The authored native entry uses
ordered host binary32 and saturated-Q diagnostics; it does not claim physical
VU identity. The complete 1416-byte PS2 span recompiles and links NONMATCHING.
Its 45 focused assertions pass. Root's integrated DLL agrees with original
instructions on 3,009 fixtures: 857 contacts, 2,151 misses and one genuine
signed-SUB trap refused atomically. This compares 5,532,024,832 backing bytes
through 4,794,058 EE and 132,374 actual microbank instructions.
Fresh complete SN execution also agrees on 600 cases spanning all 250 authentic
faces and nine edge cases, including preload aliases, 64-contact exhaustion,
degenerate triangles and trapping overflow. These runs execute 7,139,384 EE
and 26,774 actual microbank instructions; no helper results are captured.
This is supplementary body correction; the
primary capsule probe 0053FAA8 and full 67EF00 motion composition remain separate.

`dreadzone_animation_layers_advance` composes complete 00589CE0 CPU effects using
the proved primary temporal block plus actual list cleanup/release. Genuine
guest links remain intact, weights decay in original order, and metadata aliases
can affect later nodes. Caller memory is snapshotted and published only after
supported completion. Negative fractional elapsed time and nonterminating
temporal chains refuse atomically. The integrated DLL agrees with original/SN
on 460 supported scenarios (128 with original class 23F7 clip metadata), including
108 pre-cleanups and 52 metadata-to-decay aliases: 145,507 original instructions,
168,443 SN instructions, 1,718,088 mapped bytes and zero differences. There are
180 separately reported atomic refusals and 4,595 focused checks. Event dispatch,
clip selection, display cadence and DMA remain outside this API.

All 63 registered native tests pass in 3.96 seconds after these integrations.
The visible model remains an animation preview with no player controls.

The preview's optional `pose_clock` attachment invokes the recovered primary
clock for each presentation tick and requires an exact prepared frame label.
Its sidecar binds the original definition/durations and explicit diagnostic
flags to both scene and pose data. Root's integrated production libraries agree
with original 589CE0 on 120 calls/121 states: 9,364 instructions, 10,406 actor bytes
and zero differences. All 394 authored and 122 actual-package attachment checks
pass. The main launcher
now uses this attachment; 60 ticks/second schedules presentation and is not a
claim about PS2 update frequency. No event wrapper, live pose decoding, secondary
blend or player frame is added. All 64 registered native tests pass in 3.93 seconds.

The selected turn/facing/pitch inputs now accept signed zero or normal values
with nonzero magnitude at least 2^-40, retaining their original pi bounds.
The C60 polynomial keeps every original operation and explicitly flushes each
separately rounded subnormal intermediate to signed zero. This is an approximate
host arithmetic contract, not a physical VU claim. Scalars, rates, axes and
height-offset inputs retain their previous bounds. Only movement components
consumed by normalization, planar projection and accumulation receive the
separate 2^-40 floor. Smaller values still refuse atomically.

Root's guarded production-library replay uses twenty freshly compiled and
strictly linked complete SN drafts, all still NONMATCHING. Rotation and real
input/target/turn sequences agree on 9,216 cases and 3,161,510 EE instructions;
velocity agrees on 5,120 cases and 499,712 EE instructions. The C60 sensitivity
audit executes 2,300 inputs under both separate rounding and signed-subnormal
flushing, with identical final output words. Camera agrees on 576 complete
cases plus 1,030 direct C60 cases. Actual resident B58/C40/B10 execution agrees
bitwise on 2,105 orientation cases, including mixed small angles, through
160,039 instruction pairs. No callee results are captured.

The component replay covers 4,096 cases and 1,366 ordered aliases. Of 512 real
velocity-to-accumulation sequences, 480 complete and 32 refuse atomically because
a genuine angle/speed product falls below the retained component floor. Small
projected/normalized/reduced addends differ by at most one observed ULP. One
cancellation residue differs by 1.1641532e-10: each final sum is bitwise the
ordered sum of its own addends, but no ULP bound on that residue is claimed.
All 65 registered native tests pass in 3.97 seconds, including 82 new integration
checks. These changes do not add position updates, wall response or controls
to the animated preview.

`dreadzone_actor_walk_parameters_00681B30` now implements the complete selected
state-2 mode-0 parameter update. It reads the genuine recent-input history,
state getter, angular wrap, progression and mapped coefficient/time globals.
It preserves ordered guest aliases and commits copied writable RAM only after
supported completion. Other states and spherical projection remain in complete
PS2 drafts. These scalar fields do not establish animation selection.
Root's fresh production-library proof agrees with original/SN execution on
4,608 state-2 cases, including 512 aliases and 2,583 exact signed-zero checks;
20 numeric refusals preserve all RAM. It executes 1,242,567 EE instructions with
zero observed float differences in the declared approximate domain. The four
complete drafts remain NONMATCHING. The wider PS2 dependency proof also covers
5,632 cases, retaining original C78/C88 callees in main callers and testing each
expanded rebuilt leaf independently. Seven actual input-bridge/dispatcher call
snapshots agree; four active frames still stop at the genuine wall VU E90.
All 90 focused native checks pass. No player-frame completion is claimed.

`dreadzone_animation_update` now executes the reached CPU closure of 00589F98:
real primary/secondary layers, phase dispatch, scheduler 00586FE0, task query
0055DC00 and releases 006268A8/006268D8. The shadow preserves metadata writes
that change later event-count reads. A first crossed event or eligible task
allocation returns NEEDS_626D50; a reached mode-1 predicate returns NEEDS_665A28.
Each refusal leaves caller RAM unchanged. Neither metadata nor scheduler state
is altered to avoid an unavailable dependency. CPU effects exclude VF register
publication and physical PS2 arithmetic/timing identity.
Root's guarded central-production replay agrees on 1,536 complete original/SN
cases: 305,374 original and 287,623 rebuilt instructions, 17,836,032 RAM bytes
and zero differences. It executes 1,293 schedulers, 2,997 queries and 688 releases,
with 255 secondary-layer cases and 94 event-count aliases. Another 96 native
refusals are checked by running original code up to the actual unavailable call
entry. All 87,899 focused checks pass. Fresh strict linking finds the 48-byte
6268A8 helper exact; the five local sources stay outside accepted coverage.
All 67 registered native tests pass in 4.10 seconds. The animated viewer is
unchanged; first walking clip selection, wall response and position/ground
composition remain active work.

`dreadzone_capsule_contact` now executes the genuine vertical capsule probe
0053FAA8 for terrain/quad contact and actual dynamic-list filtering. Radius and
height remain independent inputs. Generation, visitation, ordered metadata
aliases and contact-packet writes are preserved; a genuine miss still publishes
its original side effects. Eligible dynamic primitive/model geometry refuses
atomically. The complete PS2 draft retains those branches, cache, DMA and VU
calls, including the genuine custom-ABI tree helper; it remains NONMATCHING.

Root freshly compiled/strictly linked the full 3976-byte span and compared an
independent bridge linked to the central production libraries. All 8,249 actual
nonempty terrain cells agree with original execution: 3,107 contacts, 5,142
misses, 13,993,800 EE instructions, 362,956 micro instructions and 21,807,452,352
RAM bytes compared without differences. Two further original/SN/native runs
each sample 250 cells under flags zero and mask two; each executes 5,200,745
combined EE and 22,000 micro instructions with 65 contacts and 185 misses.
Thirteen dynamic-filter/alias fixtures agree, and twelve real-contact dynamic
geometry fixtures validate the complete PS2 recovery separately.

All guarded inputs remain unchanged throughout the central replay. The declared
reset-empty dynamic grid is a fixture, not recovered level initialization.
Ordered host binary32 and saturated Q are numerical approximations; physical
VU arithmetic, DMA synchronization and complete player-frame behavior are not
verified. All 49 focused checks and all 68 central native tests pass. Ground
response, the distinct wall probe and frame composition remain prerequisites
for a controllable character; the visible animation preview is unchanged.

`dreadzone_actor_position_helper` now provides complete selected mode-zero
projection, offset, heading, settling, scalar/vector progression and radius
effects. Raw quad arguments include all 128 bits, ordered aliases can change
the body heading before its C60 call, and radius comes from real actor metadata
and progression. Borrowed guest views are copied and all selected writes commit
only after supported completion. Other modes remain in seven complete PS2
drafts. These helpers do not perform a position frame or ground contact.

Root's fresh complete SN comparison agrees on 7,168 cases, 1,236,192 EE
instructions and 257,880 actual C60 pairs. Central-production native replay
covers 3,584 mode-zero cases, 512 ordered aliases, 128 edge cases and eight
atomic numeric refusals. It executes 608,094 EE instructions; maximum observed
absolute float difference is 2.3841858e-7 in the declared approximate domain.
All other bytes agree; 2,878 exact signed-zero comparisons include 312 negative
zeros. The 100-byte rebuilt heading helper is isolated from the next original
entry because its genuine span is 92 bytes. All 136 focused checks pass.

`dreadzone_actor_animation_select` now implements the real 00688C08 argument
contract, first-free blend allocation 0058A850, loop predicate 00688E60,
auxiliary-weapon gates 00688A60/00688B78 and cleanup 00675430. Real metadata,
event selectors and weapon pointers are preserved. Reached unavailable
auxiliary/effect/release/recursive calls refuse atomically, rather than changing
the selected branches. Arithmetic retains its explicit chopped-binary32 domain.
The adapter supports up to 64 shadowed views and 32 MiB of mapped RAM.

The guarded central-production original/SN/native replay covers 662 complete
cases, all 112 authentic class clips, 90 metadata aliases and 58 genuine reached
dependency refusals. There are 587 allocations, 18 exhausted pools and 662
nonnull-weapon fixtures. All 10,222,804 compared RAM bytes agree; the completed
runs execute 233,236 original and 231,056 rebuilt instructions. The real
688B78 leaf is checked independently on 128 cases. Another 128 actual state-two
selection blocks call the genuine health helper and setter, producing clip five
at frame 16 or clip six at frame 15 with blends eight/twelve; all 2,036,736 bytes
agree. This observes genuine caller outputs without replacing any callee, but
does not execute complete 699C70 entry or fresh construction. The allocation
and loop drafts compare exact locally; they remain outside accepted coverage.
All 10,306 focused checks and all 70 central tests pass (4.92 seconds).
The visible preview still lacks player controls; full state entry/update,
position/ground composition and live rendered pose/blend integration remain.

The complete 00695CB8 state-two update is now retained as a PS2 recovery draft.
It preserves the true history, turning, parameters, acceleration, velocity,
wall, optional terrain and slope calls. The 2404-byte original span rebuilds to
2484 bytes after the probe ABI correction and remains NONMATCHING; the expanded
draft is replayed in isolation
from the adjacent original entry. No extra accumulated-motion call is inserted.
Default arithmetic preserves the original ordered operations; fast-math
reassociation did not agree and is excluded from the validated recipe.

Fresh strict linking and 672 original/SN comparisons agree without captures:
76 calls complete and 596 stop explicitly at the genuine wall-probe entry.
There are 160 projected-contribution cases and 32 actual actor/global aliases,
746,576 original and 760,304 rebuilt EE instructions. These controlled fixtures
do not establish a constructor, physical floating equivalence or active frame
completion. The full native state-two composition remains pending.

The optional `006840B8` call now uses its actual floating ABI: player in A0,
scale/minimum/offset in F12/F13/F14, and an integer return. The earlier integer
declaration was wrong; the previous fixture set had player3D0 zero and did not
exercise this call. Its old source and receipts remain preserved privately.
A fresh672-case replay retains the76 complete/596 actual-wall-boundary result.
A separate672-case set with player3D0 nonzero reaches76 genuine probe entries:
all three floating words agree exactly with the original (5.0,0.2,0.35).
It completes72 calls and stops600 at the actual wall entry, with no replaced
callee results. Both sets agree across full non-stack RAM and call order.
This closes a recovered-source ABI error; it does not establish an active
native walking frame or exact PS2 matching.

`dreadzone_input_angle_history_005BD900` now implements the genuine signed
history query needed by this update. Null input selects mapped 001EE600; signed
head/count, wrapped32 arithmetic and signed remainder can read before the
thirty angle slots. The returned LWC1 word keeps all bits, including signed
zero, subnormal, infinity and NaN patterns. Missing/misaligned reads refuse
without output publication. The native library is compared independently with
original instructions and fresh complete SN code on 1,920 cases, four caller
output aliases and four genuinely reached memory refusals. All 8,847,360 RAM
bytes agree. The fixture passes 3,851 checks; all 71 central tests pass in
4.94 seconds. This query does not supply live controller transport.

Private replay now validates immutable complete instruction maps once and
reuses CPU instances with freshly copied RAM and reset architectural/trace
state per case. The promoted state-two code has the identical resolved
instruction hash and all 672 per-case results match the earlier replay.
Observed local validation time falls from 362.66 to 7.34 seconds; coverage,
true calls and explicit wall boundaries are retained. Original assets/code
maps and diagnostic reports remain private. The visible viewer is unchanged.

`dreadzone_actor_capsule_resolve` now executes the selected whole 0067EF00
contact settlement and 0067F518 retry/history loop with the production capsule
and supplementary static probes. Genuine radius/height, actor states, item
suspension, packet aliases and contact limits select the actual branches.
The real EF00 return -1 is distinct from adapter refusal; F518 can perform
sixteen real retries and history restores. Reached sphere, recovery, transformed
offset and unavailable probe/numeric paths refuse the complete transaction.
Previous state17 selects the sphere on its first iteration and is refused.
Caller VF25..29 W lanes retain their real lifetime through selected capsule
and static publication mode, rather than being reinitialized each iteration.

Root freshly compiled/strictly linked both complete PS2 drafts and linked a
proof DLL to the central production libraries. Guarded full-memory replay agrees
on 128 authentic cell cases, 30 contact cases and 245 contact iterations:
2,239,640 EE and 78,848 actual bank microinstructions. Another 57 cases cover
nonempty static geometry, ordered item/global and packet/actor aliases, true
state masks, signed-zero inputs, genuine -1 returns and retry/history counts.
They execute 981,394 EE and 42,592 microinstructions without RAM differences.
Seven separate original/SN snapshots stop at genuine unavailable callees and
compare actual RAM and ABI; no unavailable call is replaced or completed.
All bound inputs stay unchanged through the 54.47-second central replay.

EF00 rebuilds to 1304 entry bytes plus 160 local helper bytes within its
1560-byte original span. F518 rebuilds to 212 bytes versus 204 and is isolated
from the following entry. Both remain NONMATCHING. All 94 focused checks and
all 72 central native tests pass (9.52 seconds for the latter). Ground DDD0,
the F5E8 outer position update and complete construction/frame composition
remain separate work. Ordered host scalar/VU arithmetic and existing probe-Q
semantics are approximate; physical flags, exceptional corner behavior, DMA
and timing are outside this evidence. The animated viewer remains unchanged.

`dreadzone_actor_slope` executes the complete selected0067D4E8 response:
actual ordered gates, original angle polynomial, planar normalization and
two velocity stores. It caches scale before the first store and reads the
live time word afterward, preserving real record/time aliases. Terrain normals
remain raw cross products, with component magnitudes through2^30; velocity W
is transported unchanged. Missing dependencies or unsupported numeric/table
domains refuse the full transaction. The mapped original angle coefficients
must match the recovered polynomial. Nonzero polynomial ratios below2^-7
refuse to bound subnormal intermediates; physical flush behavior is excluded.

Fresh isolated SN and central-production native replay pass487 whole calls,
including146 genuine terrain-contact normals and133 actual corrections.
The original executes67,535 EE and SN71,972;6,305,628 compared RAM bytes
agree outside a declared host-float tolerance, with maximum velocity error
2.384185791015625e-7 and363 bit-exact signed-zero components. No call result
is captured. The six terrain inclination subnormal examples have negative Z
and genuinely exit this slope entry before the angle call. They are not
successful inclination calculations. The focused native fixture passes66
checks. SN compiles408 entry bytes plus208 helper bytes versus the original
508-byte span, so the whole draft is NONMATCHING and replayed in isolation.
This response does not query ground, integrate position or create a frame.

`dreadzone_player_state_entry` now provides complete selected699C70 states
0/1/2, including the common prologue, real history/input changes, animation
selection, ownership/release paths and suffix. It enters at the actual699C70
ABI rather than its internal69A4F0 label. Actual guard rejection returns0
with unchanged RAM; unsupported reached dependencies refuse the whole RAM/
output transaction. Ordinary state2 genuinely selects clip5/frame16 or
clip6/frame15 after the prologue's real substate clear. Other states remain
outside native support, and the complete154-state PS2 draft is separate.

Root verified the frozen13-file lot, freshly compiled/strictly linked all
ten complete prerequisites and linked an independent DLL to the central
production libraries. Guarded replay passes1,536 original complete entries,
128 genuine guard rejections and160 atomic reached-dependency refusals.
It uses authentic112-clip Ratchet metadata and actual blend/scheduler ownership
with768 ordered client/history/handle aliases. All1,444,634,880 compared
bytes agree across1,225,684 original instructions, without captured returns.
The ten fresh isolated SN prerequisites pass8,256 original/SN cases. Two
compare locally exact; matching coverage is unchanged because acceptance is
separate. Source/asset/oracle/object/library hashes remain unchanged throughout
the11.99-second root integration replay. Earlier284 branch-block checks
corroborate the selected common gates but are not whole-entry evidence.

The focused fixture passes16,764 checks; all74 central tests pass in4.67
seconds. Explicit fixture histories and authentic metadata do not establish
a full constructor or frame. Finite EE/VU arithmetic remains approximate;
reached unrecovered cleanup, metadata, ground and transport dependencies remain
explicit refusals. Actual frame scheduling, idle/walking composition, collision
response, live pose/blend and rendered controls remain active work. The
existing animated viewer is unchanged.

`dreadzone_pose_masks` now executes the complete5893F0 CPU scratch and integer
VU-data mask effects used before secondary pose blending. It consumes the real
halfword offsets and signed joint IDs, preserves gaps in the second list and
the genuine ending sentinel, and honors the null-first-list early return.
Lists can alias scratch metadata: each next ID is loaded before the current
mask store. Both full banks publish only after supported completion.

Independent original/SN/central-production replay passes1,300 whole calls,
including all112 real clip-header ranges,812 combinations of the class's28
authentic masks and128 ordered scratch-list aliases. All687,113,728 compared
scratch/VU/source bytes agree, with1,181,260 original and1,974,930 SN EE
instructions and no captured calls. Three genuinely reached missing-read
cases refuse atomically. All bound inputs remain unchanged for7.32 seconds.
The focused fixture passes84 checks; all75 central tests pass in4.37 seconds.
The complete PS2 draft compiles to388 versus256 original bytes, remains
NONMATCHING and is replayed in isolation.
[VISWR integer store semantics](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/VUops.cpp) were cross-checked
as facts against the primary implementation, without copying code. The adapter
does not publish VU register state or establish physical scheduling/timing.
Full589768 secondary decoding/blend330/620 and live rendering remain work;
this producer alone does not add a visible pose or playable frame.

The central `dreadzone_wall_contact` library now executes the complete bounded
0053DC70 mask-two/null-cache probe: ordered grid/tree lookup, packed triangle
and quad terrain, closest contact, dynamic broadphase/filter/visitation and
packet publication. Genuine caller VF25..27 W words are explicit inputs.
Generation and initial source reads still occur for coincident integer
endpoints; finite signed-zero bounds use the actual VU word-order rule.
Missing or eligible unrecovered model geometry refuses the whole transaction,
preserving RAM/output. A cleared actor grid is an explicit diagnostic fixture,
not proof that a real level was initialized with no eligible actors.

Root independently compiles the full PS2 draft, strict-links its actual L01
span and builds a diagnostic DLL from the central production static libraries.
The56 focused checks and all76 central native tests pass (4.30 seconds).
A guarded fresh original/SN/native proof passes750 sampled authentic terrain
rays (146 hits/604 misses), seven boundaries and12 dynamic/filter/alias cases;
20 additional original/SN cases exercise real primitive/model/cache branches.
All3,965,472,000 sampled RAM bytes agree over12,874,777 EE instructions and
66,000 VU pairs. All39 bound inputs remain unchanged during156.48 seconds.
The complete draft is2696 versus4120 original bytes and remains NONMATCHING.

The historical24,747-ray proof belongs to its saved earlier source/DLL and
private CPU. Its private build selected the PS2 `unsigned long` header, which
is32 bits under Windows MSVC; that receipt does not validate the final
production64-bit bounds implementation. The final agent recipe and root's
production-library replay select `native/include/common.h`, and include high
address/wrap refusal checks. The historical evidence is retained separately.

The vectors entry now captures the two actual caller-local quads before any
persistent writes. A root production-library comparison passes750 authentic
rays and16 output/RAM/input-overlap cases against whole original and SN code
using genuine caller-owned stack quads. All4,050,068,736 RAM bytes agree over
13,055,722 EE instructions and67,408 VU pairs; six explicit invalid or
unsupported inputs preserve RAM/output. Every bound input stays unchanged.

`dreadzone_wall_helpers` supplies complete542FF8 surface classification and
68AFF8 inclination, including all mode bytes and genuine affine input W.
Root freshly compiles the complete NONMATCHING PS2 drafts and links its native
replay DLL to the central production libraries. All408 surface/979 inclination
cases pass, including146 raw terrain normals and2,094 aliases. Twenty-two
original numeric boundaries and one conservative W-subnormal source refusal
remain separately classified. The helpers do not normalize terrain inputs.
The wall fixture passes74 checks, helpers321; all77 native tests pass in4.13s.

The wall entry now also supports the genuine caller masks0x24/4/0x22.
Primitive definitions use the actual nonzero count gate and signed-header
termination order; count is not a header-walk length. Disabled and category
filters execute before the next header. Reached eligible primitive/model
geometry refuses atomically, including a qualifying negative final header.
An explicit1024-header native budget bounds unterminated walks.

Root production-library replay passes618 complete original/SN/native cases:
416 authentic rays,20 explicit diagnostic surface tags,42 dynamic filters,
128 aliases and eight signed-zero cases (218 hits/400 misses). Twenty refusals
retain RAM/output, including16 eligible geometry cases and two each for walk
budget and reached missing headers. Ten eligible geometry refusals correspond
to genuine original/SN contacts. All3,050,876,928 compared bytes agree over
10,887,484 EE instructions/55,704 VU pairs, with no captured calls; every bound
input remains unchanged throughout129.36s. The128 focused checks and all77
central tests pass (4.15s). Genuine audited terrain contains no low-tag-zero
face; the bit0x20 filter is still retained and tested with explicit diagnostic
tags. Full67D6E8 response and ground/frame composition remain work.
Current numeric/VU/Q semantics remain a declared finite host
approximation, with no physical DMA/timing or playable-character claim.

Keep matching progress and native functionality separate. A function compiling
on PC does not prove its behavior or make its PS2 dependencies portable. Every
native milestone must describe executed code and observed behavior. A retail
fallback reconstruction must never be counted as a native game build.

`dreadzone_actor_ground_step` supplies the complete selected mode-zero67DDD0
ground update and its recovered CPU helpers. Genuine scheduling, contact
surface/packet history, gravity direction, counters and required side probes
execute on a transactional shadow of caller RAM. Actual prior side-local W and
five caller VF W words are explicit; the second side iteration retains the
first transform's output. Reached unrecovered modes, objects or overrides
refuse atomically. The runtime does not manufacture radius, state, clock,
empty actors or contacts.

Root independently compiles the full NONMATCHING PS2 main and seven complete
auxiliaries at their genuine L01 addresses. The final central-production
bridge passes169 complete native/original/authored-SN comparisons from189
complete original/SN cases;20 native cases refuse numerically without writes.
This includes ordinary real terrain,204 mandatory native side probes, actual
caller-stack W/affine translation and five explicit contact/tag fixtures.
All897,706,368 compared RAM bytes agree over2,218,935 EE/99,000 VU pairs with
zero captured calls. The101 focused checks and all78 central tests pass.

The final private decoder retains SA across MTSAB/QFSRV; a separately saved
earlier single-word bridge did not, and its low-height diagnostic failed.
Genuine C60 micro instructions retain the full caller VF state. Overflowing
unconsumed lanes are conditioned in the explicit host approximation; the
standard overflow-clamping facts were checked against the primary
[PCSX2 VU implementation](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/VUops.cpp).
This does not establish physical ACC precision, flags or timing. Fixture
constructor/body/clock/grid/prior-stack values do not prove a player frame;
position/state/frame and live rendering composition remain separate work.
The existing animated viewer still provides camera controls.

`dreadzone_actor_wall_response_0067D6E8` supplies the complete selected
mode-zero response. Both genuine mask2 probes execute, preserving the first
surface across the second packet overwrite. The reached inclination and
projection paths retain actual object publication and inward velocity removal.
Caller RAM is transactional; eligible unrecovered geometry, modes, table
changes and unsupported numeric inputs refuse without committing writes.

Independent root replay uses a freshly compiled full NONMATCHING PS2 body and
the central native production archives. All 180 complete comparisons agree,
including 14 actual velocity changes, 308 probes, 20 aliases and explicit five
caller W words. Nine atomic refusals are independently exercised. All
956,136,960 compared native RAM bytes agree, with zero captured dependencies;
the 40 focused checks and all 79 central tests pass. Four original instructions
remain unexecuted by the fixtures. The declared ordered host-f32/VU/Q domain
does not establish hardware precision, flags or timing, full position update,
actor construction or a playable frame.


`dreadzone_actor_walking_00695CB8` composes the complete selected state2/mode0
walking call: actual input history, direction, turning, mapped speed target,
parameter update, velocity, wall response and final height/slope dependency.
The shadow transaction retains ordered reads and aliases; reached event,
special turning, optional6840B8, numeric or missing-memory paths refuse without
publishing RAM or diagnostic output. It neither changes world position nor
manufactures a body, state, clock, radius, grid or contact.

A separate bridge links the central production archives. All384 complete
original/fresh-SN/native comparisons pass:256 ordinary inputs,96 branch and
signed-zero cases, and32 ordered coefficient/input-record aliases. Real L01
references override overlapping resident text. The full genuine C60 producer
clears VF28.W before wall response; other caller W publication remains genuine.
All2,580,824,064 compared native RAM bytes and five outgoing W words agree over
1,858,226 EE instructions and90,656 VU pairs, including656 actual probes and35
surface/inclination paths. Bound inputs stay unchanged, with zero captured
callees, in102.61 seconds. The71 focused checks exercise late atomic rollback,
and all80 central tests pass in5.09 seconds.

The full2404-byte walking PS2 source remains NONMATCHING (2484 compiled bytes)
and adds no matching percentage. Host-f32/VU arithmetic remains a bounded
approximation. Actor/world construction, state/frame dispatch, complete
position integration and live animation/rendering still need composition.
There is no playable Ratchet frame; existing viewer controls move the camera.
