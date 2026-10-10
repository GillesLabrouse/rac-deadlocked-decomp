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

## Verified first step

`native/CMakeLists.txt` builds a native static library from seventeen recovered game
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
coverage while recovery continues. All six native CTest checks currently pass.

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

Keep matching progress and native functionality separate. A function compiling
on PC does not prove its behavior or make its PS2 dependencies portable. Every
native milestone must describe executed code and observed behavior. A retail
fallback reconstruction must never be counted as a native game build.
