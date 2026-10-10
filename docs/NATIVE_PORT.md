# Native PC development

The local fork's intended playable target is a native PC application, initially
Windows x64. Upstream Deadlocked and rac1 are matching PS2 decompilations. Their
matching builds validate recovered source against the game; they do not produce
a PC executable. Keep that validation while developing the host runtime.

## Verified first step

`native/CMakeLists.txt` builds a native static library from seven recovered game
functions in `src/core`, without retail machine code or PS2 toolchain inputs.
An explicit source list excludes startup, drivers, and address-dependent code
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
and renders extracted terrain through Direct3D 11. It supports an interactive
free camera and an offscreen PNG capture, with hardware rendering or WARP.
This is a host backend written for the port, not recovered game logic. It does
not execute a MIPS binary, and it does not link the isolated command-test transport.

The local Dreadzone Station chunk-zero export successfully converted and rendered:
42,930 vertices, 14,310 triangles, and 55 PNG textures in 55 draw batches. A room
capture was inspected, and both hardware and WARP captures completed. Terrain
is only part of the scene: placed objects, characters, sky, missions, collision,
audio and the recovered frame/update path remain absent. Color modulation,
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
The native CTest suite includes scalar/command ABI checks, malformed scene
rejection, and a WARP draw/readback test that must render a red synthetic triangle.
Python tests use authored synthetic geometry for the asset converter.

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

## Priorities toward the beginning of the game

1. Recover the startup dependency chain and identify its platform interfaces.
   Resident entry `0x1574E8` reaches the startup loop at `0x157C58`; the drafts
   at `0x157738` and `0x1579A0` load executable sections. These routines copy
   code to PS2 addresses and call it. The host needs compiled functions and
   explicit level selection in their place, not calls into copied MIPS bytes.
2. Extend the terrain asset bridge to placed scene objects and characters from
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
