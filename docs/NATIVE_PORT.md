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
validation milestone. It does not initialize graphics, load a level, or run the
game. No game launch or playable scene has been achieved.

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
A future playable native build will read assets extracted from the user's ISO.

## Priorities toward the beginning of the game

1. Recover the startup dependency chain and identify its platform interfaces.
   Resident entry `0x1574E8` reaches the startup loop at `0x157C58`; the drafts
   at `0x157738` and `0x1579A0` load executable sections. These routines copy
   code to PS2 addresses and call it. The host needs compiled functions and
   explicit level selection in their place, not calls into copied MIPS bytes.
2. Build host asset loading from the existing local Wrench extraction and
   document the first-scene formats. Guest pointers, offsets, and overlay-local
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
