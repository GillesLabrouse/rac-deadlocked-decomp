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

- 280 matching functions, 0.537% of counted code, including open-source libraries.
- 229 game functions pass exact linking: 151 resident and 78 overlay functions.
- Resident reconstruction replaces 10,448 bytes with exact compiled C. The whole
  section stream still equals retail. Remaining code and assets use local fallback.
- Ninety-eight Python regression tests and all twenty-eight native CTest checks pass.
- Eighteen matching functions compile and execute in Windows x64 test targets.
  Separate targets execute queue/request drafts with captured unrecovered
  transition effects and actual Dreadzone memory, payload and packet algorithms.
  The latter execute their recovered callback, bit writer and button repetition.
  Model allocation/compaction, class publication/relocation and nonempty geometry
  packet packing also execute native C. A Windows XInput diagnostic executes the
  recovered input path; attached-controller hardware remains unverified. Actor
  field mutation and the actual CPU state snapshot are tested, with nonzero VU
  normalization explicitly approximate.
  Guest links retain their 32-bit width. No native game update
  loop, game boot, or campaign play has been verified.
- Dreadzone Station's two terrain chunks and 1,856 placed static objects load and
  render natively, using 86 shared object models and 440 textures. Hardware and
  WARP captures completed; the room capture was inspected. The scene backend does
  not yet render actors in the level or execute the recovered startup/update
  chain. A separate editor-mesh preview renders selected armor geometry, without
  the remaining body parts, animation or game logic.
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

## 2026-10-10 — Parallel transition recovery and incremental validation

- Ran three subagents on disjoint transition, finite-float and campaign-startup
  scopes, with centralized integration and validation. Added exact recovered C
  for `004757A0`, `0047F538` and `004598E0`. The native matching subset now has
  17 functions. Finite progression tests cover both directions, overshoot,
  signed zero, negative steps, decimal convergence and scalar boundaries;
  Windows rounding and PS2 exceptional arithmetic remain separate concerns.
- Recovered resident consumer `0047EFA8` as a typed draft and exercised its two
  17-entry routing tables in an isolated native fixture. The real queue,
  preflight draft and matching helpers now run through successive request
  acceptance and consumption. Tests cover idle/delay behavior, completion timing,
  pending changes during cleanup, multi-call progress, the observed `14C` record
  stride and transfer arguments. Unrecovered transition effects are captured,
  so this fixture is not an initialized game or a playable frame loop.
- Kept compilable transition drafts outside matching coverage: `004D3060`
  (232/232 bytes, five instruction differences), `004E4DF0` (192/200 bytes) and
  `004E5638` (1,484/1,496 bytes). Corrected false decompiler arguments, actor and
  record strides, a post-call field read, the `0.75` float argument and a bounded
  polling loop. Their graphics, loading and callback dependencies are unresolved.
- Audited Dreadzone's actual pre-loop and first-frame dependencies in
  `DREADZONE_DEPENDENCIES.md`. Added initializer draft `L01_00665D58` with verified
  offsets; three linked instruction words differ. Independently bound consumer
  `L01_005A9030`: 35 direct calls, 32 callees, 67 constant-address references,
  and 22 changed bindings. Both consumer drafts produce 1,460 versus 1,416 bytes.
  The L01 record expressions remain unchanged after checking register sums;
  positional binding does not prove all signatures or complete layouts.
- Added explicit opt-in float-immediate expansion, preserving default assembly
  behavior. Corrected move-hazard detection for literal-pool loads and directives
  which emit no instructions. A full recompile preserves every accepted match.
- Added `tools/build.sh --incremental` with content-hash, flag and header-set
  checks, orphan cleanup and guarded output paths. An unchanged build reused all
  221 objects in about 3.1 seconds locally. Removing one build object caused
  exactly one compilation, followed by complete reuse. Planner failure propagates
  through the shell instead of falsely reporting success.
- Validation: 72 Python tests and eight native CTest checks pass. Audit has 277
  exact functions; 226 game functions pass exact linking, with zero mismatches
  or object failures. Resident reconstruction inserts 149 functions / 10,352
  bytes and equals retail. ELF SHA-1 remains
  `5619a8de0247e093ab672fca30dbf59bc16fe333`. The scenery viewer remains the current
  interactive native artifact; no game launch has been achieved. Nothing pushed.

## 2026-10-10 — Actual Dreadzone memory algorithms and draft review workflow

- Recovered L01 `00628020`, `006A9640`, `006A9AE0` and `006AF588` from actual
  campaign bindings. Host fixtures execute their real word-fill and reset
  algorithms without captured subsystem calls. They verify 64 scratch records,
  256 pool records, all 255 generated links, untouched fields, and every byte of
  twenty `340`-byte records plus seven counters. The pool fixture performs 6,830
  checks. Record reset fixtures exercise nonzero and zero initial storage.
- Corrected the free-list interpretation: links are at field `58`, not the
  first word. The last link is preserved, so null termination relies on the
  entry's earlier bulk clear. Tests preserve an arbitrary nonzero final link
  and traverse the complete chain when that field starts at zero. Guest
  addresses remain u32 even on Windows x64.
- L01 `00628020` is exactly 64 bytes after actual-address linking; helper
  `006A9AE0` matches 88 bytes including four trailing zeros. Their independently
  bound L01 sources remain draft-context diagnostics. Added accepted canonical
  L00 `0066E0B8` after checking its own scratch base `0032B0E0`, rather than
  borrowing L01's `0034D320`. The canonical copy passes both accepted audits.
- `006A9640` remains 136 versus 128 bytes. `006AF588` has the correct 232-byte
  extent but differs in scheduling and registers. Neither is counted as
  matching. Native algorithms use explicit guest layouts and bounded known
  storage; these tests do not establish actor creation or a game boot.
- Added `tools/check_overlay_draft.py`: it derives actual local function
  boundaries from the extracted overlay, checks provenance, resolves every
  relocation and rejects foreign-overlay symbols. Draft reports remain under
  `build/drafts/linked`, outside accepted linking and coverage. Six new Python
  tests exercise ownership and boundary failures. Real L01 checks report both
  exact routines and remaining nonmatches correctly.
- Incremental build compiled the one new accepted source and reused all 221
  prior objects. Full audit: 278 exact, zero failures. Full link: 227 exact,
  zero mismatches or object failures. All 78 Python tests and ten native CTest
  checks pass. Reconstructed resident section stream equals retail, still 149
  functions / 10,352 C bytes and ELF SHA-1
  `5619a8de0247e093ab672fca30dbf59bc16fe333`.
- Parallel recovery continues on actual input decoding and math helpers,
  object producers and the pre-loop bulk clear. Those unfinished changes are
  excluded from this checkpoint. The native scenery viewer remains the
  interactive artifact; no playable campaign has been achieved. Nothing pushed.

## 2026-10-10 — Packet execution, payload allocation and native validation workflow

- Recovered L01 packet decoder `005BDA40`, angle helpers `0055D708` / `0055D910`,
  selective/history/delay resets, and callback `006B5CB8` with its actual bit
  writer `006B59C8`. Native fixtures execute 4,309 decoder/math, 195 reset and
  4,250 callback/encoding checks. Four-frame sequences exercise the real
  callback-before-decoder order. The complete input pool's zero-initialized
  core.bss storage is established separately from selective reset behavior.
- Independently recovered polynomial/quadrant constants give maximum circular
  error `3.34129247e-7` radians on the tested finite `33 x 33` grid. Host scalar
  arithmetic does not establish original VU rounding or exceptional behavior.
  Driver/configuration, polling integration and actor input remain unfinished.
- Native bulk clear executes its actual 1,565,248-byte range and quad-fill
  footprint. Its 3,132,632 checks verify every affected byte and both guards.
  Actual state initializer, table helper and selector execute 8,389,683
  byte/field checks, including both 207-record fixed tables, conditionals,
  signed counts, guest wrap, selection and rejected mappings.
- Payload allocation/release execute real packed-descriptor algorithms with
  guest32 links, 64-byte capacity rounding and word-rounded clearing. The native
  fixture covers exhaustion, fragmentation, first-fit reuse, unlinking, empty
  reset and guards. A private instruction interpreter compared 7,200 operations,
  preserving every descriptor and state word. Its scope is these two routines;
  actor class/model initialization remains required.
- Saved actual object-orchestrator, loader and record-initializer drafts with
  independently bound L01 calls and guest layouts. They are not linked into the
  native game runtime. The loader's emitted switch table remains unsupported by
  the strict draft linker; its private comparison is separately documented.
- Extended the draft checker with explicit complete spans, declared pointer-only
  entries and failed-report replacement. Callback entry `006B5CB8` is absent from
  the splitter but independently established by its pointer producer and actual
  800-byte range. Its unmasked comparison remains nonmatching (624 bytes); the
  304-byte bit writer produces 164 bytes. All new recovery remains outside
  accepted coverage. Tool checkpoint: `a841dae`.
- Consolidated real algorithms into reusable native libraries. Added
  `tools/check-native.ps1` to discover the installed CMake, configure/build
  Release and run all fixtures, with optional CTest filtering and failure
  propagation. This command completed successfully: all sixteen tests pass.
  All 86 Python tests pass and the 278-function accepted report is current.
  Accepted game code is unchanged: 227 exact linked functions, 149 resident
  functions / 10,352 C bytes, and the same retail-equal reconstruction.
- No game boot, player movement, actor rendering or campaign play is added by
  this checkpoint. The interactive artifact remains the scenery viewer. Work
  continues on actual class setup and remaining pre-loop/input dependencies.
  Everything remains local; nothing pushed.

## 2026-10-10 — Actor-context producer, class slice and model inspection

- Recovered the actual virtual reset `006A6598`, base constructor `006A93A0`
  and identity helper `006A9638`. Both inspected input-binding paths invoke the
  larger constructor, then this virtual reset, before callback installation.
  The real word clear initializes the full `31F0`-byte actor and its embedded
  `2FEC` context. A 64,509-check fixture verifies the complete image, preserved
  base-constructor bytes and 24 callback/decoder frames at each delay `0..6`.
  Reset/base remain nonmatching; identity helper is exact but draft-only.
- Executed real `00586B10` class branches that require no cleanup or model
  allocation, with genuine environment and packed-field helpers. A private
  differential caught the zero-model-count branch's extra field/flag effects
  before finalization. All 512 cases / 198,060 instructions now agree for every
  actor byte and preserved table data. Model/cleanup branches fail explicitly
  in the host adapter until their real dependencies exist; none are stubbed.
- Recovered actual byte filler `0011A518` and pre-loop `005D6CB0`: the native
  fixture verifies exact byte counts/return values, all sixteen alignments and
  the real 262,144-byte `EF` region. It now performs 11,934,811 checks including
  parity between L01's level decision and the independently recovered matching
  resident `00510EB0`. That new accepted function passes its own 24-byte exact
  linked comparison. The byte filler itself remains nonmatching, 188/184.
- Added a strict GLB editor-mesh asset bridge, twelve authored tests, model
  inspection documentation and `run-model-preview.cmd`. The local primary armor
  preview renders 2,714 triangles/one texture with hardware and WARP, each with
  89,815 non-background pixels; the software image was inspected. Only the
  selected torso/arm mesh is present, without head/legs, animation or gameplay.
  An inventory imports 288 of 380 meshes and explicitly rejects the other
  unsupported joint/weight or empty meshes. Asset tool checkpoint: `b6e64a0`.
- The native validation script now rejects filters matching no tests. The
  deliberate unmatched-filter check returned failure, while all nineteen
  native fixtures pass together. All 98 Python tests pass. Incremental accepted
  build compiled one source and reused 222 objects; audit has 279 exact and no
  failures. Full link has 228 exact, no mismatches or object failures. Resident
  reconstruction inserts 150 functions / 10,376 C bytes and still equals retail;
  ELF SHA-1 remains `5619a8de0247e093ab672fca30dbf59bc16fe333`.
- The model pool and class-table producers are the next actual dependencies:
  the retail matrix capacity is `3094` records / `C2500` bytes, matching the
  pre-loop allocation. Parallel recovery continues on that real allocator and
  compaction, class definition relocation, and the controller configuration/
  packet dispatch path. No playable campaign frame or native game boot is
  claimed. Changes remain local and nothing has been pushed.

## 2026-10-10 — Model pool, class producers and configured input pipeline

- Recovered the six actual model-pool routines for initialization, animation
  free-list setup, allocation, release and compaction. The native adapter binds
  bounded guest32 ranges for the real 795,904-byte matrix pool and 1,024 eight-byte
  descriptors; no game pointers are truncated to host addresses. Its 9,892,653
  checks pass. A private instruction differential executes 20,483 retail calls /
  3,350,848 instructions and compares 166,418,304 bytes, including twelve
  allocations that themselves trigger compaction. VU/DMA transformation remains
  required to complete visual model initialization.
- The L01 reset `0058B050` compares exactly after linking. Independently verified
  the resident equivalent `0046F6B0` and its own `00221454..6C` globals; accepted
  that 72-byte resident function only after full audit, exact linking and retail-
  equal reconstruction. The remaining model routines stay nonmatching drafts.
- Recovered the actual class slot publication, dispatch selection and definition
  relocation, plus the explicit table-write slice of the larger reset. A private
  differential compares all bytes and guards across 640 cases / 166,779 retail
  instructions. Publication uses 10,240 class IDs, 240 normal slots and 128
  alternative slots. Native preflight refuses nonempty geometry requiring the
  still-unrecovered `00576AA8`, before mutating storage. The real class callback
  stored at actor `A8` is selected from the 313-record dispatch table; it is not
  itself a model pointer. All four PS2 drafts remain nonmatching.
- Recovered the full controller polling state machine and its eight-record loop.
  Cross-review caught a mistaken branch target: configured state99 and all
  unsigned out-of-table states must reach packet reading, not the function exit.
  Corrected the draft and added a complete mode7 configuration-to-reading test.
  The real callback, decoder and newly recovered button-repeat pass execute
  together over 28 pool frames, producing initial presses and repeats after
  20 frames then every five frames. All 2,002 polling and 405 repeat checks pass.
  Driver calls remain explicit captures in these tests. No real hardware input,
  controller transport or actor update is claimed by this lot.
- Poll and wrapper remain nonmatching. The strict checker rejects the poll's
  allocated switch table; a private diagnostic places its independently verified
  316-byte table at `003AD890` and resolves every relocation, without changing
  accepted coverage or masking differences. Repeat and selector drafts are
  likewise checked against their actual L01 spans and remain nonmatching.
- All twenty-three native CTest checks pass together. The accepted incremental
  build compiles one source and reuses 223 objects: 280 exact audited functions,
  229 exact linked game functions and no failures. Reconstruction inserts 151
  resident functions / 10,448 C bytes, still equals the whole retail section
  stream and retains ELF SHA-1 `5619a8de0247e093ab672fca30dbf59bc16fe333`.
  Python/tool sources are unchanged since the 98-test passing checkpoint.
- Parallel work now targets real geometry conversion, the preselected core
  startup path and a Windows controller backend. No native game boot or playable
  campaign frame is verified. All work remains local; nothing has been pushed.

## 2026-10-10 — Preselected core startup state

- Saved checkpoint `e77e3dc` for the reviewed model, class and input lot. No push.
- Recovered PS2 drafts of `0015A170`, `0015AA58` and `0015B498`, preserving the
  genuine transport calls and the CP0 Count instruction in the PS2-only branch.
  Exact linked comparisons remain nonmatching: 784/968, 472/472 and 172/172 bytes.
  Equal compiled size alone is not accepted as a match.
- Native execution covers the actual mode1 paths, the signed early exits, and
  the existing-state early return in `0015F4A0`. The real Dreadzone entry publishes
  `00223E40` to `0021DFE8`; its pre-loop caller sets mode258 to1 and calls these
  two initialization phases. The native fixture verifies all accessed block
  bytes, adjacent guards and scalar globals over 480,149 checks. Unsupported
  transport, level-zero initialization and nonzero callbacks return explicit
  dependency statuses rather than successful fake effects. A null callback is
  the fixture's supported domain, not proof of the full startup's callback state.
- All twenty-four native checks pass together. Matching coverage, linked bytes,
  reconstruction and Python tools are unchanged from the preceding checkpoint.
  The native scenery viewer still does not run the game boot/update chain.

## 2026-10-10 — Geometry integration and Windows input diagnostic

- Saved geometry checkpoint `f47f914`: three genuine packing routines now run
  through class relocation's ordered write journal. Preparation reads preceding
  relocation writes and validates every destination before mutation. Only the
  lower 64 bits of each output quad are changed; GPU submission is outstanding.
- Independent packing comparison covers 512 fixtures, 262,300 original
  instructions and 15,241,216 bytes. The combined publication/relocation/geometry
  comparison executes the real callees rather than capturing them: 515 fixtures,
  227,348 instructions, and no differences. Cases include shared packets, aliased
  packet headers, reloaded publication counters and retained geometry counts.
- Cross-review found and fixed three real reload defects: the auxiliary count,
  definition's list pointer and auxiliary's second-field pointer. Native regression
  fixtures preserve all three mapped aliases. Both native and PS2 branches retain
  the relevant memory read order. Seven missing/invalid-input cases refuse the
  whole host transaction atomically.
- PS2 geometry drafts remain nonmatching and are excluded from coverage. The
  table packer's actual complete span is `00576650..005767B8`, including its two
  internal branch blocks; the diagnostic uses that observed span without changing
  accepted metadata. Class relocation compiles to 928 versus 824 retail bytes.
  Default split-address flags avoid GNU as's unsupported symbolic quad store.
- Real asset validation also relocated class9207's 813,184-byte `core.bin` with
  no invented GS values. The standalone class−1 branch makes no remap reads;
  the genuine overlay remap was supplied but unused. Its 883 changed bytes and
  the whole class image agree after 3,822 original instructions. This class has
  no geometry packets. Nonempty class10024 and armor0 still require missing
  runtime graphics inputs and are rejected without mutation.
- Saved Windows input checkpoint `6013235`. The backend uses actual XInput
  state/capability calls and an explicit
  adapter for the recovered packet layout. Its 223 fixture checks cover axis
  boundaries, configuration, press/hold/release and error paths. The executable
  keeps per-device history across samples. Feedback is disabled. The launcher
  ran twice through all four indices, which all returned disconnected error1167;
  no attached-controller behavior is claimed.
- Recovered `0068E3B0` also runs as a separate native library. Mode0 is an exact
  scalar mutation path; nonzero modes return `APPROXIMATE_VU` within a bounded
  finite domain. Full byte/guard fixtures and a double normalization oracle pass.
  A 60-case instruction diagnostic validates branches/stores but models VU floats,
  so it is not evidence of VU precision or scheduling identity. The PS2 draft is
  nonmatching at 496 versus 552 bytes.
- All twenty-seven native CTest checks pass together under MSVC Release. Python
  tools and accepted matching sources are unchanged from the prior checkpoint;
  matching coverage, exact links and reconstructed bytes remain unchanged.
- Parallel work now has explicit file ownership and frozen review boundaries.
  Composed integer-memory paths receive a separate instruction differential;
  discovered aliases become tracked native regression fixtures. Focused native
  builds remain incremental, with a complete integrated run after each new lot.
  A reusable source-only differential runner is being prepared to replace private
  script-prefix transformations. Retail data stays ignored and work stays local.
- Next dependencies are the real graphics-table producers, actor-state snapshot
  and the remaining VU/DMA transformation path. No native game boot, actor update
  or playable campaign frame is verified.

## 2026-10-10 — Complete actor-state snapshot

- Saved checkpoints `6013235` for Windows input and `9e45170` for bounded actor
  mode fields. All changes remain local.
- Recovered the complete CPU snapshot `0066A2A0`, read-only predicate `0068DE60`
  and the genuine player/base virtual getters `006A7D28` and `00690E68`. Native
  dispatch reads the mapped guest vtable; unsupported handler identities are
  rejected. The existing recovered forward byte-copy helper executes the actual
  48-byte copy, with no captured getter or helper effects.
- An independent 2,048-case diagnostic executes 419,656 original instructions
  and compares the complete record, client, getter object, related object,
  vtable and mode storage. All supported cases agree. Native fixtures also check
  untouched bytes, guards, short-circuit mappings and refused dependencies.
- Review identified three late-read alias cases involving the destination
  snapshot: the related-object field, getter-object field and virtual dispatch
  entry. The bounded native interface explicitly rejects these aliases before
  mutation; regression fixtures and original instructions establish why the
  old preflight values would be wrong. This is a documented supported domain,
  rather than a claim of unrestricted alias equivalence.
- All twenty-eight native CTest checks pass together. Three PS2 drafts remain
  nonmatching at 892/608, 180/188 and 28/28 bytes. The base getter's eight bytes
  match its private diagnostic, but it remains a draft and adds no coverage.
  The snapshot copies player state; it does not create a player or run a frame.
- The texture audit located original armor0 PIF bytes on the ISO and verified
  its class identity. Texture loading needs genuine GS indexed-pixel conversion
  and transfer operations. Its descriptors occupy an uncleared stack frame,
  so unused fields cannot be assumed zero for an exact packet comparison.
  Pure CPU upload/readback packet builders are the next isolated dependencies.
- Constructor parameter initialization `00669A20` is a separately verified
  784-byte leaf. An earlier provisional call graph incorrectly included later
  functions and was corrected before source adoption. Recovery now follows that
  actual boundary. No boot/update or campaign play is verified.

## 2026-10-10 — Reproducible native class differential

- Added a source-only bounded integer instruction interpreter and a Windows
  bridge builder for the six complete class/geometry functions. Retail words
  are read only from the user's local L01 overlay, never stored in the repository.
- Independently reran the public command with automatic CMake discovery: all
  515 cases agree after 227,348 original instructions and 72,643,978 compared
  bytes. This includes three ordered-reload alias regressions and seven separate
  atomic host-domain refusals. No captured callee or GPU effects are used.
- DLL reuse requires a receipt covering all C sources, headers, bridge, compile
  recipe/configuration and DLL hash. Missing or stale inputs trigger a clean
  automatic rebuild; explicit reuse fails. Inputs are rechecked after execution.
- All 118 Python tests pass, including twenty synthetic interpreter/provenance
  checks. Matching sources, coverage and reconstruction are unchanged. This
  improves repeatable validation, without claiming a native game launch.

## 2026-10-10 — Complete constructor parameter initializer

- Integrated `00669A20` as a separate native library. Its eighteen parameter
  prefixes retain the real indices, flags, floating-point constants and B0-byte
  stride. The bounded interface requires a mapped actor record and supplies a
  stable mode word; concurrent changes to that global are outside its domain.
- All 3,302,197 native checks pass. The independent diagnostic compares 512
  complete actor images plus guards after 95,104 original instructions, covering
  every one of the 196 instructions. Root independently compiled and linked the
  PS2 source: 732 compiled versus 784 original bytes, still nonmatching.
- This initializes parameters only. Actor construction, update, rendering and
  campaign launch remain incomplete; no matching coverage was added.

## 2026-10-10 — Complete CPU texture transfer packet preparation

- Integrated the numeric upload and readback builders `00138E08` and
  `00138FF0` as a separate native library. They preserve actual signed16
  truncation, format branches, packet words and return values without storing
  extracted tables, instructions or game data in the repository.
- All 11,659 native checks pass. A separate instruction diagnostic compares
  1,594 successful original executions: 144,156 instructions and 219,972 bytes
  agree. Oversized uploads are tested for atomic refusal separately; their PS2
  diagnostic printer is not executed or captured in the host implementation.
- Root independently compiled and linked both PS2 drafts at their actual
  resident addresses and GP. Upload is 416/484 bytes and readback196/320;
  both remain nonmatching, outside accepted coverage.
- All thirty native CTest checks pass together, including both new libraries.
  All 118 Python tests also pass. Pixel conversion, DMA execution, a complete
  actor constructor and the recovered frame loop remain outstanding. The viewer
  is still an interactive scene inspection tool, not a playable game.

## 2026-10-10 — Complete constructor inventory storage reset

- Recovered `006C0D88` and its actual `0055D2F8` clear helper. The host library
  executes both complete algorithms within an explicit aligned, signed-safe
  address domain. It preserves untouched header bytes, initializes both item
  arrays, restores per-entry invalid markers and sets the real initialized byte.
- All 11,259,390 native byte/guard and refusal checks pass. An independent
  1,024-case comparison executes 1,377,286 original instructions and compares
  4,931,584 mapped bytes, including empty aligned clears and host misalignment.
  The actual clear callee executes twice per inventory reset, without captures.
- Both sources compile and link at their real L01 addresses: inventory188/208
  and clear200/192 bytes, still nonmatching and excluded from accepted coverage.
- The bounded instruction oracle now supports signed ADD/ADDI. Overflow is
  rejected before a register write, including zero-register destinations;
  exception handlers remain outside its scope. Two synthetic tests cover that
  behavior and sign extension; all 120 Python tests pass.
- Inventory storage is a constructor dependency. Item grants, full actor
  initialization and a native gameplay frame remain unverified.
- Follow-up review retained the host backing pointer and guest address before
  mutation, allowing a caller descriptor located inside the cleared storage.
  Two such regressions now pass; the fixture has 11,269,089 checks. Rebuilding
  its differential DLL and repeating all 1,024 instruction cases still agrees.

## 2026-10-10 — Native CPU indexed texture byte conversion

- Added a bounded, allocation-free CPU permutation for the successful byte
  effect of the actual PSMT8 upload followed by half-dimension PSMCT32 readback.
  It uses an independently expressed coordinate equation, not copied GS tables
  or Wrench code. Unsupported rectangles and invalid buffers fail atomically.
- The supported page-aligned domain covers both original armor0 PIF images.
  An independent GS-address oracle checks thirty rectangles and 3,702,784
  positions; 11,190,272 native bytes agree, including both real textures.
  Private source assets and reference material remain ignored.
- Root reran that audit and retained the pixel pointer before scratch writes.
  A new regression places the descriptor inside scratch; all 293,019 native
  checks pass. Caller-owned scratch and explicit guest32/host bounds are kept.
- All thirty-two native CTest checks pass together under MSVC Release /W4 /WX.
  This reproduces a documented hardware memory effect within a bounded domain;
  it is not a hardware capture, DMA/synchronization backend, complete texture
  loader or Direct3D upload. No accepted matching coverage was added.

## 2026-10-10 — Complete PS2 class texture producer reconstruction

- Reconstructed all of `0057A658` with its actual five arguments, tracking
  cursor reloads, special TEX0 branches, PIF handling and genuine hardware
  dependencies. There is no native entry or successful captured upload path.
- The descriptor frame retains the real missing initialization. The original
  one-mip path leaves +0/+C/+E unspecified; C stack-layout differences remain
  unresolved. A nonpositive PIF archive still synchronizes the palette cursor;
  the first diagnostic caught and corrected an incorrect branch interpretation.
- Root independently compiles and links the source: 1296/1352 bytes,
  nonmatching. Repeating the private linked-SN versus original-instruction
  comparison passes 256 complete CPU return paths, 272,289 instructions and
  235,930,624 externally mapped bytes, with the same real callee sequence.
  Stack saves/dead locals are excluded from that comparison.
- Two original armor PIF prefixes agree through entry to the first real
  `00575180` conversion. That callee is not executed, skipped or captured;
  positive-count PIF uploads are not fully validated. Source remains outside
  accepted coverage and native CMake. No complete native texture load or
  playable campaign frame is claimed.

## 2026-10-10 — Composed CPU texture layout preparation

- Reconstructed the complete PS2 control flow of `00575180`, retaining all
  eight real packet/syscall/upload/readback/completion calls, ignored format
  argument and signed truncations. Root's strict linked comparison remains
  nonmatching at304/304 bytes; no hardware callee is replaced by a capture.
- Added a separate native adapter composing real packet construction and the
  verified pixel permutation. It copies request descriptors, preflights all
  host/guest spans and aliases, prepares both packets locally, transforms the
  pixel backing, then publishes packet bytes. Failure is atomic.
- All443,998 native checks pass, including request descriptors placed in
  scratch, packet output and pixels. An independent actual-PIF check passes
  eight cases and658,048 bytes: original instructions validate packet bytes;
  a separate public GS-address oracle validates pixels. Its368 instruction
  count covers packet builders only, not the hardware wrapper.
- Result `CPU_EFFECT_PREPARED` is distinct from the PS2 return0. No DMA, FIFO,
  synchronization or Direct3D upload has executed, and no complete texture load
  or playable frame is claimed. All work remains local and coverage unchanged.

## 2026-10-10 — Complete CPU actor animation selection

- Integrated `00688FE0` with its genuine `0058A8D8`, `0058B6B0` and existing
  `0055D2D0` dependencies. The head clear precedes the next-pointer reload;
  only the head is released, including when its old link was cyclic or self.
- All12,596,504 native checks pass, covering four rounding modes, integer
  frame bounds, full storage/guards, wrapping ranges and descriptor aliases.
  Root rebuilt the differential DLL from current sources and repeated4,096
  cases:237,568 original instructions and85 distinct instructions agree.
- Root independently compiled and strictly linked all three PS2 drafts with
  the default SN recipe. Setup176/176 and metadata32/32 remain nonmatching;
  release104/104 is exact in the private diagnostic. Its canonical accepted
  overlay identity has not been resolved, so no matching coverage is added.
  Removed misleading extra-flag comments: the proof uses default flags.
- All thirty-four native CTest checks pass together under MSVC Release
  /W4 /WX. This selects animation data and initializes fields; it does not
  play an animation, transform a model or complete a playable actor. The
  constructor, inventory grants and model transformations remain in progress.
