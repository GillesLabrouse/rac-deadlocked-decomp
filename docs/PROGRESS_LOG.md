# Local development progress

## Objective and validation

Work toward a native PC application and the beginning of the campaign, initially
on Windows x64. Retain the PS2 matching build to validate recovered source. Keep all
changes local until the user approves a push. Source and documentation are in
English. Follow CONTRIBUTING.md and LEGAL.md: retail inputs, generated assembly,
SDK binaries, and reference material stay in ignored local directories.

Distinguish these milestones: function compilation, masked comparison, exact
comparison after relocation, PS2 image reconstruction, native function tests,
native scene rendering, and campaign play. A passing earlier milestone does not
prove a later one. Emulator tests are optional reference checks.

## Current verified checkpoint

- 274 matching functions, 0.532% of counted code, including open-source libraries.
- 223 game functions pass exact linking: 146 resident and 77 overlay functions.
- Resident reconstruction replaces 10,280 bytes with exact compiled C. The whole
  section stream still equals retail. Remaining code and assets use local fallback.
- Thirty-four Python regression tests and all six native CTest checks pass.
- Fourteen matching functions compile and execute in Windows x64 test targets.
  A separate target also executes the experimental nonmatching request handler
  together with the actual queue. Fixtures capture unrecovered command handlers
  and map byte filling to CRT memset. No native
  game update loop, game boot, or campaign play has been verified.
- Dreadzone Station's two terrain chunks and 1,856 placed static objects load and
  render natively, using 86 shared object models and 440 textures. Hardware and
  WARP captures completed; the room capture was inspected. The scene backend does
  not yet render characters or execute the recovered startup/update chain.
- Work remains on the local branch; nothing has been pushed.

## 2026-10-10 — Environment and priorities

- Created the user's fork: https://github.com/silverspy/rac-deadlocked-decomp.
- Local branch: `work/bootable-rebuild`; upstream snapshot: see local Git history.
- Read contribution, legal, research and overlay instructions. Reviewed rac1's
  build workflow: address-preserving reconstruction and exact linked comparison
  are the immediate gap in Deadlocked, which currently builds objects only.
- Located the supplied NTSC-U ISO in the workspace. Added a dependency-free
  executable extraction tool that verifies the supported SHA-1 before writing.
- Added native Windows compiler invocation and virtual-environment detection
  while retaining the Wine workflow for Linux/macOS. The Windows setup, game
  compilation, libgcc and libm compilation have run successfully. The pinned
  newlib snapshot required a complete refetch after lazy blob fetches failed.

## 2026-10-10 — Exact comparisons and correctness fixes

- Verified the supplied executable's expected SHA-1 and generated all resident
  assembly. Extracted all 47 level overlays with official Wrench v0.6.
- Baseline: 243 functions, 0.513% of counted code. The broad old mask hid six
  resident pointer offsets and twelve overlay constants/strides. Corrected
  all eighteen functions; no assembly instructions were manually patched.
- Replaced broad masking in the C audit, word diff and automatic draft check
  with masking of actual object relocation fields only. Empty or failed audits
  now return a failure status. Overlay grouping retains its distinct broad mask.
- Added exact linking with the real legacy linker, original function addresses
  and original `$gp`. All 192 existing game functions passed unmasked checks,
  including 115 resident functions and 77 overlay functions. Library functions
  remain independently audited with relocations masked, not linked by this tool.
- Shared overlay aliases are resolved independently inside the current level.
  An ambiguous alias in `func_L15_00500430` was replaced by the explicit local
  target `func_L15_005004C8`, derived from the retail call.
- Adopted four new resident functions: `func_0011A180`, `func_0011A1A0`,
  `func_0011A1B0`, `func_0011A1C8`. Each passed both audits. Updated report:
  247 functions, 0.515% of counted code. Percentages do not include retail fallback.
- The startup network-image wrapper `func_001579A0` is a near-match draft under
  `nonmatching/`; return-address save scheduling still differs. Its object is
  excluded from the game build. The startup path enters at `0x1574E8`, calls
  `0x157C58`, then uses `0x1579A0` to load the network image.

## 2026-10-10 — Reconstruction and verification

- Reconstructed the resident section stream using 115 exact linked C functions
  (8,204 bytes) and local retail fallback. All 17 sections equal retail; the ELF
  SHA-1 is `5619a8de0247e093ab672fca30dbf59bc16fe333`.
- Produced `build/rebuilt/deadlocked.iso` as a verified byte-identical test disc.
  Matching reconstruction reuses the original compressed representation. This
  is a build provenance milestone, not additional decompilation or a PC port.
- Wrench full asset repacking encountered metadata/dependency errors. Packing
  its boot ELF separately worked, but its compressor produced a larger loader.
  The retained packaging workflow preserves original compression and sectors.
- Added build input manifests; exact linking and reconstruction reject stale
  sources, headers, pipeline inputs, objects or compiler assembly.
- Nineteen synthetic-input regression tests pass; CI now runs them. No retail
  bytes, executable, assets, assembly, toolchain or BIOS are tracked.
- Clean rebuild complete: 194 game translation units, 247 audited functions,
  196 exact linked game functions, and zero failures. Resident reconstruction
  now contains 119 C functions (8,296 bytes). The regenerated ELF still has
  SHA-1 `5619a8de0247e093ab672fca30dbf59bc16fe333`.
- The regenerated test ISO is byte-identical to the supplied disc, SHA-256
  `dda48512e9b7609f69a49146ab239c4ebda7f13cf3d06397f19f6f3ce155a42b`.
- Pinned objdiff CLI v3.8.1 successfully validated the updated progress report,
  alongside `gen_progress_report.py --check`, synthetic tests and Python syntax
  compilation. Small local commits preserve the fixes and one progress snapshot
  per newly adopted function. No branch or commit has been pushed.
- Emulator/BIOS paths were requested; runtime remains unverified.

## 2026-10-10 — Resident command helpers

- Added fourteen exact resident functions: result polling at `00158198`, result
  buffer setup at `00158240`, and twelve command wrappers from `00158598` through
  `001589A8`. These cover single values, a value pair, optional 24-byte payloads,
  six-value payloads, and callback/context forwarding. Numeric command identities
  remain address-derived or unnamed until their purpose is established.
- Every new function passes the actual-relocation audit and unmasked comparison
  after linking. Each has a small local commit and a corresponding progress
  snapshot. The resident reconstruction now contains 133 functions, up from 119.
- Fixed `try_variants.py` and `compare_variants.py` to use actual relocations,
  rather than ignoring ordinary immediate constants. A deliberately wrong
  command code now reports 11/12 matching words and is rejected. The correct
  wrapper reports 12/12. `try_variants.py` now uses the shared compile pipeline
  on Windows and checks compiler failure before reading any previous object.
- Kept section-loader and startup-parameter formatter drafts outside `src`.
  Compiler scheduling, packed-field stores, and register allocation still differ;
  they do not contribute to coverage. The optional m2c helper was cloned locally
  at `708d2d2cb2698f091a92492b328f73b24209f72d`.
- Found resident boundary issues near startup: some splat functions include a
  later real entry point, and some start in padding made of stack adjustments.
  Investigate those boundaries before adopting the affected functions; do not
  reproduce the padding as if it were a C function.

## Next milestones

1. Map startup and first-scene platform dependencies for the native target.
2. Extend host asset loading beyond static objects to dynamic objects/characters,
   then implement the recovered scene/update path; use explicit guest-address conversion.
3. Continue recovering startup and first-level code with exact linking as the
   matching acceptance check. Keep failed matching drafts outside src.

## 2026-10-10 — Startup boundaries and native target clarification

- Pinned seven genuine resident entry points that splat had grouped into earlier
  functions. Generated metadata changes are confined to those startup ranges.
  Added six wrappers after exact object and linked comparisons: `00158568`,
  `001586F0`, `00158780`, `001587E0`, `00158838`, and `00158988`. Each has its own
  local commit and progress snapshot. The changed function denominator reflects
  corrected boundaries, not additional recovered code.
- Kept the separated startup loop at `00157C58` as a nonmatching draft. Its
  global address materialization and compiler scheduling still differ.
- The user clarified that the intended outcome is native execution. Earlier
  PS2 reconstruction work remains useful validation but is not that deliverable.
  PCSX2 v2.8.2 was downloaded locally and its configuration check passed; no BIOS
  is present and no game boot was tested. Emulator preparation is now deferred.
- Added a separate CMake native target, a fixed-width host type header, and an
  ABI/command forwarding test using seven recovered source functions unchanged.
  MSVC 19.38 compiled and executed it on Windows x64. The test exercises 64-bit
  callback values, signed payload boundaries, record sizes and optional payload
  handling; it has no game, BIOS, emulator or original-toolchain dependency.
- Documented the remaining native startup, asset, overlay and platform work in
  `docs/NATIVE_PORT.md`. The test executable is not a game runtime.

## 2026-10-10 — Native terrain backend

- Inspected the local Wrench v0.6 extraction. Dreadzone Station chunk zero has
  terrain geometry in COLLADA, PNG textures, and a color accessor not connected
  by its vertex inputs. Added an explicit validated recovery option for that
  aligned color source, rather than applying it to arbitrary COLLADA files.
- Added a dependency-free local asset bridge and a versioned host scene reader.
  The bridge checks geometry, attributes, material paths and supported transforms;
  the reader rejects truncated packages, non-finite fields and invalid draw ranges.
  All extracted data and generated packages remain ignored. No asset bytes were
  added to the repository.
- Built a native Windows Direct3D 11 renderer with texture decoding, depth testing,
  a free camera and offscreen PNG capture. Its C++ backend is written for the host;
  it does not run the PS2 executable or the recovered game update loop.
- Prepared and rendered 42,930 vertices / 14,310 triangles and 55 textures. Inspected
  the room view at developer camera `(600, 480, 203)`, yaw `-1.5707963`, pitch `-0.05`.
  The hardware capture contains 921,600 non-background pixels at 1280 x 720; this
  full-frame count is not proof of gameplay. A bounds-framed WARP capture also passed.
- Local scene package SHA-256:
  `8f154a144afce71f6bc946e02252af91d79fcb76c2f6a2bd92cffc6b831769e6`.
  Inspected room PNG SHA-256:
  `5cc7cfc656875ad93fff83a76405ae0049aa0ef801312c1223a811761e7288c3`.
- Fixed the capture encoder to use supported BGRA PNG output with explicit channel
  conversion. The render self-test verifies red triangle pixels, not merely a
  changed background. All three native CTest checks pass with MSVC 19.38; all
  twenty-six Python tests pass. Matching progress remains 267 functions / 0.539%.
- Remaining work includes scene-object placement, characters, collision, original
  camera/player behavior, audio, missions, and recovered startup/update code.
  Rendering fidelity is unverified. This is the first terrain backend milestone,
  not a playable beginning. Changes are local; nothing has been pushed.

## 2026-10-10 — Static objects and recovered boot parameters

- Added an authored parser for the structured local export text, static class
  resolution, and a version-two host package with shared mesh geometry and affine
  instance records. The renderer uses separate per-vertex/per-instance buffers.
  Missing classes, escaping mesh paths, invalid matrices/ranges/bounds and excessive
  draw work fail explicitly. The original terrain format still loads.
- Prepared both Dreadzone Station terrain chunks and all 1,856 exported static
  object placements, from 86 models. Final package: 197,412 shared vertices,
  1,858 instances including terrain, 440 textures and 461 material draw batches;
  672,516 submitted triangles before GPU clipping. The second chunk resolves its
  textures through the explicitly supplied chunk-zero material directory.
- Hardware and WARP room captures completed. Inspected the added wall ornaments,
  support structures and other static geometry. Instance lighting remains an
  unimplemented part of rendering fidelity; static objects currently use white
  tint. Dynamic objects, vegetation, characters, collision, sky and game behavior
  remain excluded. All packages and images stay local and ignored.
- Final package SHA-256:
  `889c4d466f4725a38fc8702e8ebcfe35bdda0bb9aa1fd285a7e5d0e854fd79b5`.
  Hardware room capture SHA-256:
  `2e8b938885782f5898acea5f55c1fa348ccc6ac1bdd17f65a3a35c4954a5a23e`.
  Version-one room regression PNG remains byte-identical to its earlier capture.
- Recovered `func_0045CB88`, the startup configuration copier, as exact C: nine
  values and two bytes initialize eight slots, followed by four `-1` markers.
  All 40 words match and its linked bytes equal retail with no masking. Native
  tests execute the same source with signed boundary values and byte-layout checks.
- Splitting around the verified `0045CB88..0045CC28` routine also makes an existing
  handwritten span's following labels available as 265 new function-table entries.
  They are newly indexed, not newly decompiled. The denominator is now 15,328;
  coverage changes from 0.539% to 0.536% despite the additional matched routine.
  The earlier `0045CB78` entry ends before the real routine instead of overlapping it.
- Startup loop `00157C58` remains a draft. Compiler flag and loop-form variants
  did not match. Its observed argument strings are `BOPT=`, `gooey` and `multi`;
  further recovery is needed before using this path in the native runtime.
- All 268 audited functions and 217 exact linked game functions pass. The resident
  reconstruction contains 140 compiled functions / 9,688 bytes and still equals
  retail. All thirty-four Python tests and four native CTest checks pass. The
  instancing self-test verifies distinct red/green placements and orientation,
  including PNG decoding and texture channel order with an authored texture.
  Work remains local; nothing has been pushed.

## 2026-10-10 — Startup entry boundaries and recovered request queue

- Verified the resident startup's real initial entry at `00437EB0`, after an
  eight-byte fall-through prelude. Its 2,004-byte extent contains all 33 direct
  branches and all 18 targets in the state table. The body has 63 distinct direct
  callees across initialization, frame work and cleanup; it is not yet recovered C.
- Corrected two false splits caused by early returns inside queue routines.
  The split configuration now exposes another 438 resident function-table entries.
  These are newly indexed functions, not newly decompiled code. The denominator
  is 15,766; counted coverage changes from 0.536% to 0.532% with five new matches.
- Recovered exact queue initialization, enqueue, dequeue, reset and dispatch in
  `src/game/0045C768.c` and `src/game/0045C8C8.c`: five routines, 584 linked bytes.
  Enqueue classifies values at the boundary 100. Dispatch derives its mode from
  the observed state, calls the handler with five arguments including a null
  optional pointer, and resets the queue after a negative handler result.
- Native tests execute all five routines with FIFO, capacity, wraparound, lock,
  empty-output, signed-state, handler-argument and error-reset cases. State
  subtraction is unsigned before decrement, preserving defined behavior at
  `INT32_MIN`. The same expression still produces exact PS2 code.
- Native fixtures replace vectorized memset with CRT byte filling and capture
  `0047F568` without implementing its game effects. These tests do not execute the
  menu loop or add gameplay to the scenery renderer.
- Identified the Dreadzone overlay ELF entry `00528098`, grouped by the existing
  normalizer under common function `func_L00_00594458`. Its 4,584-byte extent and
  normalized grouping were checked against both overlay ELFs. This grouping is
  not proof of equivalent global/callee addresses or a verified host port.
- Documented the entry chain, state branch groups and missing host boundaries in
  `docs/STARTUP_PATH.md`. The campaign entry and frame/update dependencies remain
  the next recovery targets.
- Global audit: 273 exact functions. Exact linking: 222 game functions, including
  145 resident and 77 overlay functions. Resident reconstruction replaces 10,272
  bytes and still equals retail; ELF SHA-1 remains
  `5619a8de0247e093ab672fca30dbf59bc16fe333`. All five native CTest checks pass.
  Source and documentation remain local; nothing has been pushed.

## 2026-10-10 — Local native prototype launcher

- Added `run-native.cmd` for double-click testing of the existing Windows Release
  scenery viewer and the locally prepared Dreadzone package. It resolves paths
  relative to the launcher and opens the verified room camera position.
- Verified the launcher with a hardware Direct3D 11 capture and inspected the
  resulting 1280-by-720 image. It renders both terrain chunks and 1,856 static
  object placements without an emulator or BIOS. Generated assets remain local.
- This is a runnable native development prototype, not a playable game. The
  recovered startup/update chain, Ratchet, missions, collision and audio remain
  unimplemented. Documented the controls and capture command in `NATIVE_PORT.md`.

## 2026-10-10 — Compilable Dreadzone entry draft

- Recovered a typed C draft of Dreadzone's actual `00528098` entry in
  `nonmatching/L01_00528098.c`. Verified its local branch/table extent and
  independently resolved 150 direct call sites, 105 distinct callees, and 250
  constant-address reference sites. There are 87 data bindings, of which 32
  change from the L00 overlay. Positional agreement does not establish callee
  semantics or native compatibility.
- Corrected decompiler errors in byte strides, argument-register aliases, mixed
  integer/float arguments and a 32-byte stack payload. Verified guest record
  sizes and relevant field offsets during compilation. Preserved the payload's
  unspecified bytes rather than inventing initialization.
- The saved draft compiles with SN GCC, but produces 4,452 bytes versus 4,584 in
  retail and fails relocation-aware comparison. It remains excluded from
  accepted sources, progress coverage, image reconstruction and native builds.
  No campaign boot or playable behavior was added by this draft.
- Fixed overlay disassembly regeneration on Windows by using the running Python
  executable, recording splat output and stopping on subprocess failure. Verified
  L00 regeneration: 2,949 canonical functions retained. Generated assembly and
  experimental analysis remain ignored/local.
- The matching checkpoint remains 273 audited functions / 222 exactly linked
  game functions. Work stays on the local fork; nothing has been pushed.

## 2026-10-10 — Request preflight on the native host

- Recovered the `0047F568` request-handler algorithm as a typed draft, including
  four ordered rejection codes, object gating, eight-slot snapshots and payload
  assignment. The fifth argument is an optional completion-byte pointer. Its
  508-byte PS2 output still differs from the 512-byte original, so it stays in
  `nonmatching/` and outside accepted matching coverage.
- Recovered its only callee, `00465C08`, as exact C. It reads the scalar at
  `0021DAE0` without arguments. Its eight bytes pass exact linking.
- Added a separate native target for the experimental handler. It executes real
  queue dispatch and the matched state reader, rather than a request-handler
  stub. Tests pass for success/refusal precedence, history capacity, unchanged
  rejected payloads, host pointer width, float zero/NaN object gates, scan bounds
  and queue reset after a pending request rejects a later dispatch. Native alias
  pairs share one scalar storage location. The target substitutes only CRT byte
  filling and does not execute the request consumer or claim a campaign boot.
- Re-expressed Dreadzone's observed scalar fields through their shared base at
  `00172160`, with compile-time field-offset checks. Its draft now uses the same
  112-byte stack frame as retail, but produces 4,424 versus 4,584 bytes and still
  fails comparison. Source-form and mixed-address experiments remain local.
- All six native tests pass. The global audit has 274 exact functions, and 223
  game functions pass exact linking with zero mismatches or object failures.
  Resident reconstruction inserts 146 functions / 10,280 bytes and equals retail.
  Source and documentation remain local; nothing has been pushed.
