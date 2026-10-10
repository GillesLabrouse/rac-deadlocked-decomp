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

`run-model-preview.cmd` opens a separately prepared armor editor-mesh package.
See [native model inspection](NATIVE_MODELS.md) for preparation, verified captures
and the missing parts/animation. It is separate from initialized actor rendering.

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

Keep matching progress and native functionality separate. A function compiling
on PC does not prove its behavior or make its PS2 dependencies portable. Every
native milestone must describe executed code and observed behavior. A retail
fallback reconstruction must never be counted as a native game build.
