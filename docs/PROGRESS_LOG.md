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

## 2026-10-10 — Reusable inventory instruction validation

- Extended the source-only integer oracle with MOVN, XOR/XORI and DSRL for
  the genuine constructor inventory chain. Three new synthetic ISA tests
  cover full low64 operands, preserved upper halves, zero destinations,
  aliased registers, all32 shift amounts and unsigned immediate extension.
- All123 Python tests pass. Inventory work now reuses these tested operations
  rather than adding a separate instruction interpreter. This tool change
  neither supplies retail data nor implements a game or hardware callee.

## 2026-10-10 — Retained actor parameter inputs

- Review found that a caller descriptor inside a parameter prefix could lose
  its stable mode before later prefixes read it. The native entry now retains
  the backing pointer and mode before mutation and rejects numeric host spans
  that wrap before dereferencing them. PS2 code is unchanged.
- The new regression places the mode inside prefix1100; full output and guards
  agree. All3,315,048 focused native checks pass, and a freshly rebuilt DLL
  still agrees across512 original-instruction cases and95,104 instructions.
  Fake wrapping descriptor/backing pointers are refused without access.

## 2026-10-10 — Complete constructor and model-update orchestration

- Reconstructed the complete `00668070` constructor, retaining five inputs,
  virtual reset, all19 real direct targets, collision retries, exact float
  constants, masked vector lanes and actual VU C60/Q/ACC operations. Shared
  class branches merge in C; the source remains nonmatching3212/4104 bytes.
- Root independently compiled, linked and validated the build receipt,
  branch closure, unique original return and direct-target set. Original26
  direct call sites/nine VU sites become20/six compiled sites through common
  branches. This proves structure and bindings, not complete execution.
- Reconstructed `0058A648` with actual scratchpad/model callees and dirty-bit
  update. Root's default-recipe linked comparison is exact over88 bytes,
  including4 padding bytes. It remains a draft pending canonical identity;
  neither accepted source coverage nor a native completed transform is added.
- The remaining constructor work is split among complete inventory grants,
  actor synchronization and actual model transforms. Pure CPU dependencies
  reuse one tested integer oracle; root owns integration and independent review.
  No compiled MIPS instruction fallback or successful captured callee is used.

## 2026-10-10 — Complete constructor random dependency

- Added the real `00163408` seed transition and `0058FF38` range wrapper to
  native CMake. Seed storage is caller-owned, may be unaligned, and its backing
  descriptor is retained before mutation. Output aliases/wrapping spans fail
  atomically. The full signed/wrapped-width domain uses actual DIV semantics.
- The first instruction comparison caught an incorrectly read multiplier:
  the original is3359741 (0x3343FD), not the standard MSVC rand multiplier.
  Corrected source and fixtures before integration. The freshly rebuilt DLL
  agrees across8,192 cases,184,320 original instructions and393,216 bytes.
  All4,096 range cases execute the genuine RNG callee.
- Shared oracle DIV/MFHI now has synthetic signed remainder, zero-divisor,
  INT_MIN/-1, upper-half and operand-alias fixtures. All125 Python tests pass.
  All thirty-five native CTest checks pass together under MSVC Release /W4 /WX.
- Both PS2 drafts compile and strictly link: seed44/56 remains nonmatching;
  wrapper72/72 is exact in its private L01 diagnostic. No accepted source or
  coverage was changed; complete player construction remains outstanding.

## 2026-10-10 — Complete native default inventory setup

- Integrated18 complete PS2 routines totaling2,144 retail bytes into a native
  library linked to the genuine existing reset/clear implementation. Supported
  paths execute mode0 fresh level-zero grants, existing inventory attachment
  and mode1 indexed reset with original ammunition and slot operations.
- All26,608,676 native byte/guard/refusal checks pass. Caller-zero BSS fixtures
  are explicit: reset preserves shared slot counts rather than inventing zeros.
  Caller-owned immutable table/global layouts are validated before mutation;
  positive progression and paid-message calls remain unavailable dependencies.
- Root rebuilt the DLL and repeated2,048 full-memory instruction cases:
  5,618,114 instructions,566 distinct instructions,1,024 fresh loadouts,
  512 existing inventories and512 indexed resets agree. All19 real callees
  execute;512 cases include originally zeroed pool storage. No call is captured.
- Root independently compiled and strictly linked all18 PS2 drafts using
  default SN flags. Only006BE198 is exact24/24 in the draft diagnostic;
  other17 remain nonmatching. No accepted coverage or full actor boot is added.

## 2026-10-10 — Native static model pose with original hierarchy

- Added the normal static model path as a bounded CPU adapter. It reads actual
  skeleton/animation inputs, expands initialized records, copies the static
  stream and prepares both64-byte matrix banks plus the original dirty bit.
  Unsupported animated streams, lists and non-finite/intermediate failures
  leave all outputs unchanged. Inputs are retained before publication.
- Root review added numeric wrapping guards for the request and view-array
  objects before their initial reads. Six new atomic assertions pass;
  the fixture now has164,231 successful checks.
- Root rebuilt the current transform DLL and reran the independent original
  VU-pair executor:129 cases,8,178 joints,237,162 instruction pairs and130,848
  float words agree with zero different bit patterns under host IEEE arithmetic.
  The original107-joint Armor0 inputs are included; no game data is tracked.
- Status remains APPROXIMATE for VU rounding and covers CPU-visible memory
  effects only. DMA/VIF, internal VU state and a complete game update have not
  executed. Viewer skinning additionally requires the actual compact inverse-
  bind composition and joint weights; direct translation of its GLB is invalid.
- All thirty-seven native CTest checks pass together under MSVC Release
  /W4 /WX. Work remains local; the scene viewer is still not a playable game.

## 2026-10-10 — Complete actor synchronization source review

- Retained four complete PS2 drafts covering position/rotation oscillation,
  record-to-actor synchronization and actor transform update. Their genuine
  collision, spatial, animation and model dependencies remain visible; there
  is no native success body that skips those dependencies.
- Root added explicit nonmatching extent notes, rebuilt all four objects with
  default SN flags, and strictly linked the full128/184/416/384-byte spans.
  Compiled extents are124/184/436/428 bytes; none adds accepted coverage.
- The independent full-span audit verifies branch closure, genuine direct-call
  multiplicity and VU B58 selection. Original and freshly linked instruction
  streams agree across256 complete inactive/no-volume returns:58,516 executed
  instructions and3,403,776 compared bytes, with no captured callee.
- Five synthetic checks cover raw COP1 signed-zero/NaN-payload transport,
  unmapped PREF hints and rejection of floating arithmetic. Active oscillator,
  VU, volume and DMA branches are structurally recovered, not replayed here.
- Parallel work has explicit file ownership and dependency-sized lots; root
  independently rebuilds each frozen lot before integrating it. Current next
  dependencies are spatial cells, compact inverse-bind composition and genuine
  equipment configuration. These lots do not yet provide a native game boot.

## 2026-10-10 — Shared oracle for complete CPU return paths

- Added raw LWC1/SWC1 transport and PREF to the shared integer oracle so
  parallel work can reuse one checked implementation. Floating arithmetic,
  VU, cache state/timing and hardware remain outside its supported domain.
  Load/store semantics were checked against the public
  [PCSX2 FPU implementation](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/FPU.cpp)
  and PREF's lack of a modeled memory result against
  [R5900OpcodeImpl.cpp](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/R5900OpcodeImpl.cpp).
  No implementation was copied or vendored.
- Four new synthetic test methods cover all32 raw registers, signed zero,
  NaN/infinity/subnormal payloads, separate integer registers, signed offsets,
  address wrapping, run reset, bad memory/alignment, unmapped hints and explicit
  floating-arithmetic rejection. All129 Python tests pass.
- Root switched its independent actor synchronization replay to this shared
  module and repeated256 cases:58,516 instructions and3,403,776 bytes still
  agree. The private fork is no longer needed for that check. No native source
  or accepted matching coverage changed in this tooling lot.

## 2026-10-10 — Shared packed-integer spatial validation

- Extended the shared oracle for operations genuinely executed by the spatial
  routines: variable scalar shifts, checked signed SUB, TEQ, packed byte/half
  interleave, saturating signed-byte subtraction, signed-word maximum and
  lower/upper doubleword copies. TEQ raises an explicit IntegerTrap while
  retaining preceding effects; exception-handler execution is unsupported.
- The only accepted COP2 word transfers zero to immutable VF0 without an
  interlock. This models no register/memory result under the explicit absence
  of concurrent VU work; other COP2 instructions and VU computation still fail.
  Lane/shift/trap semantics were checked against public
  [MMI.cpp](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/MMI.cpp),
  [R5900OpcodeImpl.cpp](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/R5900OpcodeImpl.cpp)
  and [VU0.cpp](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/VU0.cpp).
  No reference implementation was copied or vendored.
- Seven new synthetic test methods cover all65,536 signed-byte subtraction
  pairs with destination aliases, lane order, signed extremes, scalar shift
  counts and upper halves, overflow with zero destination, retained trap memory
  and rejection of other COP2 words. All136 Python tests pass in under3 seconds.
  The spatial workstream can now reuse this implementation rather than a copy.

## 2026-10-10 — Native compact binding-matrix composition

- Integrated `dreadzone_bind_pose_prepare` into native CMake. The bounded
  CPU adapter executes original pose64 * binding48 composition in A40/AA8
  order, including count-zero identity and pose70000000 scratch bypass.
  It retains copied bindings and scratch tails; unavailable/nonfinite inputs
  fail atomically. APPROXIMATE explicitly covers host binary32 rounding.
- Root rebuilt a fresh /W4 /WX /fp:precise DLL and added six checks for
  view descriptors inside scratch and null/wrapping requests. All75,305 native
  assertions pass. The integrated focused CTest also passes.
- Root independently replayed257 original microprogram cases:16,531 joints,
  214,903 original instruction pairs and264,496 output float words agree with
  zero different bit patterns. Both external-pose and scratch-source paths
  are checked; original107-joint Armor0 inputs are included locally only.
- Root independently compiled and strictly linked the full384-byte PS2 draft:
  compiled extent352 bytes, nonmatching. Real DMA transfers and paired VU
  calls remain in that source; no accepted coverage or hardware equivalence
  is claimed. The native adapter does not execute DMA/VIF or internal VU state.
- Composed Armor0 translations remain broad, so no rest-pose cancellation or
  rendered character is claimed. The next geometry work follows actual vertex
  weights and their rendering consumer rather than applying matrices to an
  editor GLB with inferred joints. No playable native game boot is added.

## 2026-10-10 — Native CPU equipment maintenance

- Integrated seven complete recovered routines for equipment maintenance,
  selection predicates, slot deletion, marker/reset state and the bounded
  setter. They reuse the genuine inventory library, with no captured UI call.
  Public preflight rejects missing mappings, UI paths and unsafe field aliases
  before writes; context/backing/descriptor ranges are disjoint and checked.
- Preserved the ordered signed-category-2 store that clears266D before the
  retail flag read. Real flag1 maintenance with2318=0 is supported; ordinary
  fresh-player flag1 setter UI dispatch remains an unresolved dependency.
- Root independently built a fresh DLL and repeated4,096 full-image cases:
  2,007,381 actual instructions,270 distinct instructions,512 flag1 maintenance
  cases,370 flag0 maintenance setters,127 real store-clears-flag cases and281
  rejected selections agree. All ten executed CPU callees are genuine.
- Native fixtures compare entire buffers with memcmp plus targeted boundary,
  alias and status assertions:16,906 checks pass, avoiding inflated byte-call
  counts. All thirty-nine native CTest checks pass together in3.15 seconds
  under MSVC Release /W4 /WX. This adds CPU behavior, not a playable frame.
- Root independently compiled and strictly linked all seven PS2 drafts:
  only006BE040 is privately exact16/16; the other six remain nonmatching.
  No accepted source coverage changed and no files were pushed.

## 2026-10-10 — Complete compiled spatial opcode inventory

- Added shared LWU zero extension, DSRA32 signed64 shifts and NOR, required
  by the actual spatial instructions and the freshly compiled allocator helpers.
  Synthetic fixtures cover signed extremes, all shift counts, upper halves,
  destination/base aliases, zero destinations and invalid load memory.
  All139 Python tests pass in2.42 seconds.
- The spatial workstream now inventories every original and compiled opcode
  before replay, including ordinary compiler-generated operations. Unknown
  executed instructions still fail explicitly. This avoids discovering one
  unsupported operation per run while retaining the bounded oracle contract.
  No native game or accepted matching source changed in this tooling lot.

## 2026-10-10 — Complete bounded native spatial storage

- Integrated the complete rectangle update and bitmap allocator/releaser CPU
  memory effects. Actual64x64 cells,1,536 blocks and192-byte allocation bitmap
  come from the original initializers; unused bytes and actor indices remain
  unchanged. Growth allocates before release, shrink releases before allocation.
- Root independently rebuilt all three complete SN drafts and strictly linked
  original96/120/736-byte spans. Compiled88/172/1,068-byte symbols remain
  nonmatching. Full branch/call/trap audits and every original/compiled opcode
  pass; compiled execution uses a genuine nonoverlapping private linker layout.
- Root's fresh MSVC /W4 /WX DLL agrees in515 successful cases across original,
  compiled PS2 and native runs:396,609 instructions,67,963,520 compared bytes,
  matching real helper order and zero captured callees. Four separate native
  atomic refusals include actual original traps/exhaustion after earlier writes.
- Parent review added request-wrap checking before descriptor reads, signed
  actor/base SUB-boundary rejection and unsigned release-count word wrapping.
  All177 native assertions pass, including complete output preservation.
- No accepted matching coverage changes, actor VU transform, collision test,
  native game frame or pushed files are claimed. Shared CMake runs this storage
  test with the existing native suite.

## 2026-10-10 — Native original vertex blend and local position stages

- Integrated a bounded source-only adapter of the renderer's actual matrix
  transfer/blend and local position stages. Original byte weights divide
  by256, not255; delayed IDs, terminal halfwords and cross-packet duplicates
  use real packet fields. Both matrix and vertex caches persist explicitly.
- Root independently built a fresh /W4 /WX /fp:precise DLL. All364 fixture
  checks pass. Root strengthened the private original-pair oracle to compare
  the entire retained state, every vertex-cache record and both definition
  sets, verify decoded words against the extracted microprogram bytes, and
  reread the class container from the local ISO to verify its provenance.
- All60 cases agree:120,318 vertices and562,785 original instruction pairs,
  zero differing local stage bytes. Original28 high/low/bangle ranges and32
  arbitrary nonsymmetric107-joint inputs are included; alternatives are
  validated independently, not drawn as one invented character.
- Output/state/count publish only after all packets pass. Missing cache,
  unsupported packet shapes, reserved constant slots62/63, invalid numeric
  inputs and insufficient capacity refuse without publishing; temporary
  workspace is explicitly allowed to change.
- APPROXIMATE describes only host IEEE mathematical stages. No full0053C818,
  lighting, normal decode, VU1 packing, world/view scale, hardware rounding,
  DMA execution or rendered/gameplay frame is claimed. Genuine topology/ST
  and the actual active bangle mask remain the next preview dependencies.
- All forty-one native CTest checks pass together in2.91 seconds, including
  the new vertex and spatial tests, renderer validation and existing CPU lots.

## 2026-10-10 — Shared raw orientation word rotation

- Added PROT3W to the shared integer oracle: raw128-bit permutation
  [word1,word2,word0,word3], alias-safe and retaining the fourth word.
  The selector comes from the actual constructor instructions; lane order was
  independently checked against the primary
  [PCSX2 MMI implementation](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/MMI.cpp).
  No implementation was copied and no floating arithmetic is performed.
- Synthetic tests exercise destination/source aliases, ignored rs, zero source
  and destination, distinct sign/NaN payloads, and three successive rotations
  restoring each of128 individual input bits. All141 Python tests pass in2.38
  seconds. The camera workstream can reuse this operation without a private ISA
  copy; unsupported instruction families remain explicit errors.

## 2026-10-10 — Correct constructor orientation pair and share camera opcodes

- Independent C60 inspection found a real constructor draft defect: PROT3W
  selects original word1, not word2. Corrected the shared constructor macro's
  cosine lane at all three uses. Root rebuilt and strictly linked the complete
  4,104-byte recovered entry; the3,212-byte source remains nonmatching. Full
  branch bounds,19 real direct targets and one virtual dispatch are retained;
  this is structural validation, not constructor execution.
- Shared PEXTLW now interleaves the two low words rt/rs into a full quad;
  POR performs raw128-bit OR. Both operations are present in the actual camera
  initializer. Their lane order was checked against the primary
  [PCSX2 MMI implementation](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/MMI.cpp),
  without copying its implementation. Synthetic tests cover aliases, zero
  registers, discarded upper words and each of128 OR bits. All143 Python
  tests pass in2.55 seconds. No floating arithmetic or VU hardware is added
  to the shared integer oracle.

## 2026-10-10 — Complete bounded native camera initialization

- Recovered the entire688-byte005617A8 span, including all three actual C60
  executions, raw constants, two position reloads and the late global load.
  Root independently compiled and strictly linked its500-byte SN symbol;
  it remains nonmatching with no accepted coverage update.
- Integrated native C60 reflection/ninth-degree polynomial and the complete
  CPU-visible initializer, with separately rounded host binary32 operations
  and `/W4 /WX /fp:strict`. Native domain is finite normal/zero angles in
  [-pi,pi] with supported intermediates and1..8 checked guest/host views.
  Unsupported mappings/arithmetic leave all backing unchanged.
- Root's fresh DLL passes140 native assertions and independently agrees in
  576 full initializer cases against original EE plus the actual C60 resident
  MPG instructions and the genuinely linked SN source. All1,478,912 compared
  bytes agree across170,496 EE instructions and94,206 VU pairs. A separate
  1,030-angle check includes signed zero and interval boundaries; no result
  or callee is captured. Shared packed opcodes execute the actual EE words.
- Position aliases cover all21 output quads; the late global can alias
  output54 and observes that constant's prior store. Fourth quad words,
  output tails, untouched fields and host guards are retained.
- APPROXIMATE explicitly excludes physical VU/EE rounding, flags, register
  effects, stack spills and timing. This supplies one real constructor
  dependency; actor state/collision and the full native game loop remain.

## 2026-10-10 — Genuine indirect integer call validation

- Added JALR to the shared integer oracle for the equipment setter's actual
  virtual getter. The target uses old rs.low32 before the link store, retains
  upper register halves and executes the real delay slot. Actual indirect
  targets join the same call trace as direct calls; no target is captured.
  Behavior was independently checked against the primary
  [PCSX2 interpreter](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/Interpreter.cpp)
  and [link-register definition](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/R5900.h),
  without copying an implementation.
- Synthetic tests cover source/link aliases, zero destination, link-visible
  delay stores, low-word targets,32-bit link wrapping, missing target code
  and invalid nested control transfers. All146 Python tests pass in2.54
  seconds. Missing code or unsupported executed instructions still fail.

## 2026-10-10 — Complete bounded equipment UI CPU chain

- Integrated14 complete recovered PS2 routines and the bounded native
  005D7558/005E93D8 entry points. They retain the actual chosen-item store,
  recent-queue insertion/rotation, actor selector and dirty/summary flags.
  The common chosen-item store still executes when query bytes suppress
  shuffling. Rotation uses the real wrap-index helper, not a captured RNG.
- Root independently rebuilt all14 SN objects and compared full spans:
  1,720 original/1,724 compiled bytes. Only005D7558(32),005D8A08(48) and
  00652FD0(88) are privately exact;11 remain nonmatching. Splitter extents
  for005EA230/005EA270 omit early-return tails and are overridden with the
  independently recovered64/80-byte complete ranges. Accepted coverage is
  unchanged.
- Consolidated the identical ABI/declaration blocks into one authored internal
  header. Native public headers load from their C translation units before
  that shared header, avoiding MSVC's ancestor-directory quoted-include
  resolution selecting PS2 scalar types. Both native and SN builds were
  rebuilt after this correction; source/header inputs remain recorded.
- Root's fresh /W4 /WX DLL agrees across4,096 complete original-instruction
  cases:791,780 instructions,333 distinct instructions,13 real CPU callees,
  1,024 zero-queue item2 cases,1,035 rotations and319 rejected selections.
  Whole actor/pool/table/query/global/queue/state/summary bytes agree.
- Public preflight checks selected-path reads/writes before stores, including
  the real unused paired-copy lookahead word, aliases and wraparound. The
  supported domain is force1/category0/definition28nonzero/initial state24
  and28zero. Secondary/reentrant setter paths remain explicitly unavailable.
  No renderer/audio call, fresh-player setter binding or native game frame is
  captured or claimed. Fixture callback globals satisfy unused linked storage
  only; no callback is invoked.
- Root's integrated UI fixture passes3,932 checks. All forty-three native
  CTest checks pass together in3.17 seconds under MSVC Release /W4 /WX.

## 2026-10-10 — Complete actor-state dependency sources

- Recovered the complete PS2 routines 00667050, 0067DBE8, 00596390 and
  0067DDD0. Root independently rebuilt and strictly linked all four full
  spans. The eight-byte backlink is privately exact; vector construction,
  clipping and the main state body remain nonmatching (416/488, 148/224,
  3380/3256 compiled/original bytes). Accepted coverage is unchanged.
- The main state source retains all 25 direct calls to 16 genuine targets,
  branch domains, five CVT.W.S sites, quad-by-value ABI and VU/Q operations.
  Callback-dependent collision/actor/global reloads and the original
  uninitialized side-probe backing are explicit. No collision result or
  native successful main-state return is invented.
- Root's fresh SN vector helper agrees across 512 complete directional=0
  returns with original instructions: 25,088 EE words and 6,815,744 bytes,
  including output/record/same-output aliases and raw W. This limited
  host-f32 VU-macro evidence does not validate the directional Q path,
  full collision, native actor motion or constructor execution.

## 2026-10-10 — Genuine bounded virtual equipment selection

- Connected the actual flag1 category-zero 00688578 setter to its real
  virtual +4C getter and the recovered equipment UI queue/state chain.
  Getters 006A9630/00690EA0 return byte2FEA/-1; root independently rebuilt
  both using default SN flags and strictly matched their eight-byte spans.
- The supported domain requires mapped actor0/1, nonzero byte266D, index0/1
  and fresh UI state24/28. Category/handler/mapping errors discard copied
  RAM images before publication. Original post-store flag/vtable reads and
  writable aliases are preserved. Actual LW alignment is checked; stable
  exclusive RAM access is a caller requirement. Transaction storage is
  bounded to 16 MiB. Read-only item1 preflight validates storage without
  gameplay dispatch; it is not a synthetic game call.
- Root's fresh native DLL agrees across 4,096 whole-RAM cases: 1,033,597
  original instructions, 393 distinct instructions, 256 valid writable
  virtual-entry aliases, 1,024 item2 zero-queue cases and 1,035 rotations.
  Another 512 getter cases execute 1,024 original words. All 7,504 native
  assertions pass. Complete actor construction, secondary/reentrant UI,
  UI rendering, audio and a playable frame remain outside this composition.
- Reset/constructor alone do not establish the final getter index:
  original outer initializer 006A8420 restores byte2FEA after construction.

## 2026-10-10 — Original native model topology and visible diagnostic

- Added bounded original VIF/ST/index/texture decoding, joined directly to
  genuine computed vertex output ordinals. Signed ST and guest IDs are
  retained; ADC advances raw GS FIFO while suppressing drawing. Area-cull
  headers outside the actual FE domain and unsupported layouts refuse
  atomically. This is a double-sided topology diagnostic, not a renderer
  with original clipping, lighting or initialized live GS state.
- Root's fresh integrated DLL agrees with independently replayed original
  VU1 publication: all 37 primary Armor0 packets, 3,478 computed vertices,
  2,714 chronological FIFO triangles, 77,291 instructions and 118 XGKICKs.
  Both explicit identity comparison and the actual 107 pose/binding matrix
  inputs agree for every triangle, signed ST, output index, guest ID and
  texture. Real zero-valued IDs are retained, not guessed from editor data.
- Review independently resolved two separate pointers: packed secret indices
  advance by 16 bytes; texture payloads/index metadata advance by 64 bytes.
  A further original-instruction replay exposed and fixed the missing FIFO
  reset on inserted texture PRE/PRIM. Two/three-group fixtures agree across
  307/345 instructions and 5/3 ordered triangles with final texture state.
  All 211 native topology assertions pass.
- Audited simultaneous upper/[I] timing in older transform, binding and
  vertex microprogram stages. No previously tested stage has an immediate
  consuming upper paired with an I update. Root's vertex oracle now uses
  old I explicitly; all 60 cases/120,318 vertices/562,785 pairs still agree.
  Camera C60 already snapshots old I. VU1 micro0398 specifically requires
  old3071 before publishing254 to retain the real packed ADC bit.
- Added a buildable diagnostic DLL, reproducible local preparation bridge
  and run-model-topology.cmd. The scene uses actual native vertex/topology
  calculations without a GLB. Presentation remaps (y,z,x), applies class
  scale/1024 and uses flat double-sided shading; it does not claim actor
  basis/world/view or original texture upload. The new WARP capture was
  visually inspected: textured torso and arms, head and legs absent,
  89,853 non-background pixels. Assets, microbytes and images stay ignored.
- All forty-five native CTest checks pass together in 3.17 seconds under
  MSVC Release /W4 /WX. All 146 Python tests pass in 2.35 seconds. Local
  progress adds no retail assets, accepted matching coverage or playable
  game frame; nothing is pushed.

## 2026-10-10 — Actor orientation, collision leaf lookup and selected body

- Added the genuine B58 actor basis with C40 polynomial rotation and B10
  ordered matrix products. Root's fresh build agrees across 1,039 cases,
  62,627 original VU pairs and 66,496 output bytes. Signed-zero subtraction
  preserves the original +pi result. All 202 native assertions pass under
  /fp:strict. Physical VU arithmetic, flags and timing remain approximate.
- Added the complete integer 0053DBD0 Y/X/Z collision-tree lookup. Root's
  independent fresh build agrees across 1,027 fixtures, 19,059 original
  instructions and 8,413,184 table bytes, with 106 nonzero leaves and 27
  signed-overflow refusals. All 25 native assertions pass. This retrieves
  leaf data; it does not calculate a ground contact or move an actor.
- Added bounded native primary/bangle candidate selection with explicit
  actor mask and LOD inputs. Actual missing-primary-low and descriptor-low
  fallbacks, descending mask-bit order and raw packet bounds are retained.
  Unsupported sign-bit/material domains refuse atomically. All 311,314
  native assertions pass. Root reruns the original instruction comparison
  against the integrated DLL: 1,100 cases, 195,414 instructions and five
  actual constructor read/OR boundaries; no full constructor is claimed.
- The explicit fresh-boundary mask1002 selects primary0..36, bangle12
  packets101..107 and bangle1 packets59..60. The integrated preparation
  computes 4,238 vertices and 3,326 triangles. Their complete hashes agree
  with original VU1 topology replay across 94,336 pairs and 144 XGKICKs,
  under the documented finite non-clipping view fixture. The new WARP
  capture uses corrected (y,x,z) presentation axes so original Z remains
  vertical in the viewer. Additional body parts are
  visible: head, torso, arms and legs, with 142,531 non-background pixels.
  The pose has outstretched arms and is not animated gameplay.
- Extended the reproducible preparation CLI and launcher. Assets, matrices,
  original microbytes and capture images remain local and ignored. All
  48 native CTest checks pass together in 7.55 seconds. No accepted matching
  coverage, live actor frame, native game boot or remote push is added.

## 2026-10-10 — Controller-to-actor bridge and literal publication proof

- Added five complete PS2 drafts: 006A7D48, 0067BD58, 0055D180, 005BE808
  and 0067C570. Root's fresh original-compiler objects strictly link and
  compare as nonmatching: compiled/retail sizes144/144,116/136,60/48,
  116/144 and64/80. The first four use baseline flags; magnitude uses
  -ffast-math. No accepted matching coverage is changed.
- Added the genuine bounded native bridge to the existing snapshot/predicate
  and input libraries. Actual button/timer work precedes the one-shot read;
  mode and input-axis aliases observe ordered stores/reloads. The one-shot
  path genuinely clears the flag and skips magnitude/angle work. Active
  input copies axes and computes direction fields, retaining actor128 as
  the third magnitude component. Transactions preserve supplied RAM on
  refusal and publish writable regions only after the selected path succeeds.
- Root's fresh /W4 /WX /fp:precise DLL agrees across4,096 full one-shot
  bridges,4,096 button calls,65,536 signed timers and4,096 mask remaps.
  The original CPU executes1,328,033 instructions at142 distinct sites,
  including256 global/timer aliases; every supplied memory image agrees.
  No floating/VU callee is captured as completed. Active numerical behavior
  is separately approximate, with2,048 independent finite fixtures and256
  actual decoded packet inputs. All153,170 native assertions pass; all49
  CTest checks pass together in3.51 seconds. No position integration,
  collision, animation, controller transport or playable frame is claimed.
- Independently traced the model-selection literal from ISO TOCsector1001
  through level1 header40C and primary96DC7000 to sectionstream96DC78C0.
  Actual literal bytes02 10 are at96DC9E00. Root independently executes
  all00157738 original words with the shared integer CPU oracle:3,782,341
  instructions, zero callees, all destination bytes and guards agree.
  The actual caller limit1E9A000 skips net.text, publishes half1002 at
  2206B0 and returns528098. This proves the supplied-data copy boundary,
  not the disk driver or complete native level load. All ISO/CPU bytes
  and generated receipts remain ignored. Nothing is pushed.

## 2026-10-10 — Repair zero-distance WAD marker decoding

- Collision recovery exposed a one-byte shift in the shared asset decoder.
  The tool incorrectly skipped a legitimate EE match opcode after ordinary
  no-op110000, first differing at decompressed level1 asset offset1B5DF0.
- Recovered the real distinction from original loader00800330..00800384:
  length tag1 selects the no-op/tiny-literal path; other zero-distance tags
  select the actual DMA block boundary without a literal. Root executes
  28 isolated original classification cases, including all tiny-literal
  low bits, with504 instructions and no substituted DMA/callee execution.
  The independent Wrench v0.6 decoder documents the same distinction.
- Fixed classification rather than adding a level/address-specific exception.
  Four synthetic regression tests cover a following EE opcode, block-start
  no-op, padding with all low-bit values and an embedded WAD/tiny literal.
  All150 Python tests pass. No original asset bytes are tracked.
- Root's full level1 asset output now equals independent Wrench extraction:
  14,214,848 bytes, SHA256c953c0d97031da97493e9ecf83e3025de4c150370dd77dea6ac469d994a79331.
  The resident output remains byte-for-byte unchanged at5,157,636 bytes,
  SHA25665e358a40e07c9bd8e7f07d4c8c0f1ce6ebf8578a4db61f96c25ab240aaf871b.
  This restores reliable collision/animation data extraction; it does not
  establish native terrain contacts or a running game frame. Nothing is pushed.

## 2026-10-10 — Real terrain tree and triangle contact

- Root independently rereads the original compressed asset stream directly
  from ISO, decodes with the corrected shared tool and validates every8,249
  nonempty Dreadzone tree cell against original0053DBD0 and the fresh native
  DLL:313,462 instructions, unchanged table backing/guards. The677,568-byte
  tree contains37,227 primary triangle records and23,660 quad continuation
  entries. This full tree lookup check is not a whole contact probe.
- Added the complete single-triangle iteration0053E1E8..0053E384: common
  clip/surface/orientation filters, plane crossing and Q, three edge tests,
  strictly nearer fraction and actual XYZ contact data. Success also retains
  original s5/source_guest and s6/surface+1000. Output is atomic, including
  misses/refusals; request/output overlap is supported through snapshot.
- Root's independent /W4 /WX /fp:strict DLL agrees with raw-word original
  macro-VU/Q and integer/delay-slot replay:3,327 fixtures,3,072 real cases
  from611 authentic triangles,2,552 contacts,767 misses,8 numeric refusals,
  128 partial aliases,329,995 original instructions and183,744 output bytes.
  Root additionally verifies the quantized relative endpoints remain in
  their actual source cell. That domain establishes the real fractions0/1
  clip producer; it does not establish general multi-cell clipping.
- All39 focused assertions and all50 native CTest checks pass; the complete
  suite takes3.49 seconds. Arithmetic remains explicitly APPROXIMATE_CONTACT,
  with no physical VU saturation/flags/Q timing claim. Zero-denominator
  grazing cases refuse and must be recovered for full-probe composition.
  Grid traversal, packed face/quad iteration, cache, dynamic actors, final
  units, DMA and a player frame remain outside this adapter. No accepted
  matching coverage changes, retail bytes or remote push are introduced.

## 2026-10-11 — Extend instruction validation for real grid traversal

- Added scoped PSUBW, PPACH and PMINW execution to the shared integer oracle.
  Word subtraction wraps independently; packing retains the low halfwords of
  each word in rt-then-rs order; minimum compares signed words. All operations
  snapshot both 128-bit sources before writing an aliased destination.
- Checked semantics against the primary PCSX2 MMI implementation. Synthetic
  cases exercise signed boundaries, wrap without lane borrow, ties, all128
  single-bit packing positions, zero registers and aliased destinations.
  All153 Python tests pass. This enables the original collision-grid replay;
  it is validation tooling, not native collision or a playable frame.

## 2026-10-11 — Preserve real animation definitions in native armor data

- Recovered four complete PS2 functions005756D0/00575A18/00575B88/00575BD0.
  The loader retains genuine disc/GPU dependencies; the slot caller uses
  actual001DFC60+20 and publication uses001DFC84. Root's fresh SN builds
  strictly link all four drafts; publication is privately exact16/16 and
  the remaining functions are nonmatching. Accepted reports stay unchanged.
- Added the complete bounded native memory effects of00575A18: metadata and
  animation pointer transfer into retained geometry, preserved destination
  byte47 bit8 and reconciliation of actual actor.half36 bit8. A write journal
  preserves ordered aliases, including the source count reload, and preflights
  every access before publication. Original actor-pool bounds are explicitly
  immutable; aliases writing their global words are refused.
- Root's independent shared raw-word CPU compares the original function,
  fresh SN build and fresh /W4 /WX DLL in513 cases:581,137 original and655,806
  compiled instructions,7,309,952 bytes and zero differences. Fresh ISO reads
  establish the real class23F7 source:107 joints and112 animation pointers.
  The fixture transfers those pointers and metadata into authentic Armor0
  data. All3,145,994 focused assertions pass.
  All51 registered native tests pass in3.55 seconds after source freeze.
- This transfers animation definitions; it does not decode poses, animate the
  viewer, execute the disk/GPU loader or establish a playable frame. Original
  assets and differential receipts remain ignored. Nothing is pushed.

## 2026-10-11 — Traverse real collision cells and retain zero-Q behavior

- Added the actual axis-list producer and stable grid merge0053DDE0..0053DF84.
  Preserved X/Y/Z tie priority, signed fraction clamps, packed cell stepping,
  initial record and sentinel. Identical truncated integer endpoints take
  the original early exit before sentinel publication. The bounded domain
  permits at most79 crossings per axis, explicitly excluding the original
  cap path that counts an unwritten80th fraction.
- Root's fresh /W4 /WX /fp:strict DLL agrees with genuine original words and
  the shared integer/MMI oracle in529 cases:4,810 records,38,480 bytes,
  174,958 original instructions and zero captured callees.382 cases exercise
  zero-axis reciprocal saturation. All30 focused assertions pass, including
  capacity and output-alias refusal without partial writes.
- Extended the triangle kernel with a separate zero-denominator Q API;
  the strict API keeps its refusal contract. Signed saturation and numerical
  I/D condition are supplied explicitly while all real plane/edge branches
  still execute. Both outputs remain unchanged on unsupported arithmetic.
  Root's fresh original-word comparison covers3,331 fixtures:2,552 contacts,
  775 misses,4 refusals,8 saturated divisions,32 request/status aliases and
  330,287 instructions. All56 focused assertions pass.
  All52 registered native tests pass in3.77 seconds after source freeze.
- This validates traversal and a complete triangle iteration, not the full
  probe. Packed block/quad iteration, prefetch order, hit stopping condition,
  final units, dynamic actors, cache, DMA and player motion still require
  integration. Physical VU flags/timing remain unclaimed. Nothing is pushed.

## 2026-10-11 — Recover native player direction and turning

- Added selected complete0067C5F8 direction and0067CFD0 rotation paths with
  genuine projection, normalization, history, getter and scalar turn callees.
  The stronger original-word comparison corrected classification to use the
  preserved unnegated input axes at67C8B4/BC. Facing-angle preparation keeps
  its real negation. No position integration or invented body/camera basis
  is introduced; nonzero actor2659 dependencies refuse atomically.
- Root rebuilds an isolated /W4 /WX /fp:precise DLL and replays actual words:
  4,096 direction cases, 1,024 turns, 128 ordered aliases and 512 scalar cases,
  including 256 angle/speed pointer aliases. All ten selected callees execute,
  with 1,583,894 original instructions and 697 covered sites, no captures.
  Full supplied RAM agrees except explicitly tolerated approximate floating
  results; maximum observed absolute difference is 1.1920929e-7. Physical
  PS2 ACC precision, rounding, flags and scheduling are excluded. All 11,453
  native assertions and 58 independent diagnostic assertions pass.
  All 53 registered native tests pass in 3.74 seconds after integration.
- Root freshly compiles and strictly links all ten complete PS2 drafts.
  Eight float sources use explicit -ffast-math; history/getter use baseline
  flags. All remain nonmatching. The authored normalization VU block keeps
  its explicit branch/delay ordering without forced registers or a whole
  assembly-function substitution. Accepted matching reports stay unchanged.
- Located the actual gameplay core at primary header+60, ISO97BC8900;
  level header+20 is a sound bank. Root's fresh decoded core is 2,390,596
  bytes, SHA256c9697eba9d7f5eb46bf196d6c67af489b10a188a1dd5a97e34fa4e9f913adfbd.
  Settings90+2C (BC) equals zero. Root executes the original nine-instruction
  source-read/clear/branch producer57B0E8..57B108 against that data, proving
  global0021E878=0 with unchanged input and all global guards. This proves
  the selected configuration boundary, not the entire parser or startup.
- True position production, ground/wall caller integration, animation and
  a playable frame remain in progress. No retail data or push is introduced.

## 2026-10-11 — Decode genuine variable animation poses natively

- Added the bounded CPU-visible normal5894F0 decoder effects, including real
  VU160/250 arithmetic order. Fixed/current/next banks and initialized joint
  records are explicit scratchpad inputs. The inherited 4 KiB VU data image
  is also explicit; metadata lanes are not silently initialized to zero.
  Gathers from unwritten high cache entries refuse atomically.
- Preserved packed opcode0/24/48 handling, signed-byte expansion and halfword
  wrapping, low-half index injection, interpolation, alternating output banks
  and all three priming groups. The authentic F99/V5 animation0 has 30 frames
  and consumes 54 quaternion pairs, one scale pair and seven translation pairs.
  Resulting values remain encoded local state, not glTF rotations or matrices.
- Root's fresh /W4 /WX /fp:precise DLL uses independently reread ISO class and
  Armor0 slices. All 210 genuine cases (30 frames at seven fractions, including
  wrap29-to0 and varied explicit inherited VU metadata) agree with original
  EE/VU replay and separate packing derivation: 776,160 EE instructions,
  418,740 VU pairs, 3,440,640 scratch bytes and zero differences. Actual
  resident MPG1170E0 words are checked through328. All 20,507 focused native
  assertions pass, including rejection and unchanged input guards.
  All 54 registered native tests pass in 3.75 seconds after integration.
- This adapter excludes DMA/waits, blend330/620, subsequent matrices738/858,
  physical VU state publication, animation update and renderer. Host binary32
  arithmetic remains explicitly approximate. The complete PS2 C draft5894F0
  is still absent. No animated or playable frame is claimed; nothing is pushed.

## 2026-10-11 — Enable full terrain-stage instruction replay

- Extended the shared bounded integer oracle with PPACB, PADDW, PSUBH,
  PCPYH, PSLLW, PSRLW, PSRAW and checked signed DSUB. Packed operations
  snapshot complete sources, preserve actual lane packing and wrap each lane
  independently. DSUB refuses signed64 overflow after preserving preceding
  effects, including when its destination is the zero register.
- Root reviewed semantics against primary PCSX2 MMI and scalar instruction
  sources. Seven synthetic test methods exercise literal lane order, aliases,
  zero registers, wrap, all 32 shifts and signed64 boundaries/overflow.
  Root's full 160-test Python suite passes in 2.447 seconds. All 58 independent
  float-diagnostic checks still pass; shared class/run hooks remain unchanged.
- This enables validation of the original packed terrain stage. It does not
  implement a DMA backend, complete contact probe or playable frame. The
  terrain composition remains in progress. Nothing is pushed.

## 2026-10-11 — Display the original animation through native stages

- Integrated genuine normal 589950/VU 738/858 matrix effects and 589368 local
  skeleton initialization. Root's fresh /W4 /WX /fp:precise DLL agrees with
  original complete EE/VU replay on 297 matrix cases (570,810 EE instructions,
  637,815 VU pairs, 4,866,048 scratch bytes) plus 129 initializations (83,998 EE
  instructions, 2,113,536 bytes). All 26,077 focused native assertions pass.
  Three complete PS2 drafts 589368/58A258/589950 compile and strictly link as
  NONMATCHING against 88/224/528-byte retail spans; they stay outside `src`.
- Added local native preparation for original animation0's 30 frames and their
  half-frame samples. Actual skeleton, fixed/current/next streams, normal
  matrices, bind 48 records, selected vertices and original FIFO topology
  produce 107 joints, 4,238 computed vertices and 3,326 triangles. No editor
  skeleton, host quaternion interpolation or invented joint mapping is used.
  An authored dependency proof finds 16 inherited unknown output words, all
  ignored scale.w/translation.w, with no influence on visible matrices.
- Original EE 589368→5894F0→589950→589B60 and verified original microprograms
  agree exactly with all 60 prepared matrix hashes: 497,400 EE instructions,
  394,440 VU pairs, 410,880 matrix bytes. Actual bind DMA registers supply a
  bounded memory-copy model, not hardware timing or captured callee success.
- Added a bounded sampled-position file linked to ordered base vertices,
  explicit presentation-rate playback, deterministic sample captures and
  mutable Direct3D geometry. 168 loader/time checks and an actual WARP test
  moving the same geometry between two captures pass. All 57 native tests pass
  in 3.63 seconds; 166 Python tests pass in 2.323 seconds, including five new
  packaging/dependency tests and the independently reviewed PEXTUW addition.
- `run-model-animation.cmd` opens the new articulated, animated body. Software
  samples 0/30 show 123,204/122,549 non-background pixels and were visually
  inspected; all 60 native captures form a local GIF. Code/docs remain English,
  generated disc assets stay ignored and nothing is pushed. Playback rate is
  explicitly diagnostic; no actor clock/state label, live player movement,
  terrain response, campaign frame or playable game is claimed. Terrain/contact
  and genuine velocity integration remain the next work.

## 2026-10-11 — Complete upper-word interleave for velocity validation

- Added PEXTUW to the shared bounded integer oracle. It snapshots full sources,
  interleaves rt.word2/rs.word2/rt.word3/rs.word3, transports raw bits and retains
  the immutable zero register. Root reviewed the primary PCSX2 MMI semantics;
  no third-party implementation or retail instruction list is copied.
- A literal synthetic fixture covers 24 source/destination combinations,
  including zero sources, inverted/same sources and destination aliases.
  The complete current Python suite passes all 166 tests. This enables genuine
  velocity helper replay; it does not add movement, position or a game frame.
  Frozen terrain and velocity lots await parent integration. Nothing is pushed.

## 2026-10-11 — Integrate terrain contacts and genuine velocity production

- Integrated bounded 0053DC70 static-terrain CPU effects, including original
  grid/tree ordering, physical axis mapping, clipped packed-cell decoding,
  primary/quad selection and closest-contact termination. Contact units remain
  1024-scaled; root provenance is not a final game object pointer. The adapter
  excludes full probe publication, other actors and physical DMA/cache timing.
- Root rebuilt a separate /W4 /WX /fp:strict DLL and replayed 10,998 fixtures
  covering all 8,249 nonempty terrain cells: 10,774,074 original EE instructions,
  17,954 true tree calls, 14,040 memory-copy effect models, zero captured callees
  and 189,072 exact contact bytes. The 2,626 hits, 8,358 misses, 14 atomic IEEE
  refusals and work counters agree. Source/oracle hashes stay unchanged across
  the full run; 984 cases select the quad mutation result.
- Integrated speed progression, real velocity production, normalization and
  planar projection in actor2659=0. Actual C60 evaluates pitch before heading;
  mapped RAM aliases and unsupported-domain atomic refusals remain explicit.
  Eight complete PS2 drafts compile and strictly link as NONMATCHING, outside
  accepted sources. They do not increase matching progress.
- Root rebuilt and checked the integrated CMake velocity diagnostic DLL against
  actual and separately compiled words: 3,072 cases, 212,019 EE instructions,
  77,952 C60 pairs and maximum absolute float difference 2.3841858e-7 within
  declared approximate binary32 tolerances. Other supplied RAM agrees exactly.
  The 1,552 velocity and 27 terrain checks pass; all 59 registered native tests
  pass in 4.25 seconds. No full position update or playable frame is claimed.
- Ordinary walking also tests twelve static collision elements in the station.
  Capsule/static-body response, genuine target-speed/turn callers and actor
  animation-clock integration remain in progress. The existing animated model
  preview remains available through `run-model-animation.cmd`. No assets or
  original instructions are tracked; nothing is pushed.

## 2026-10-11 — Validate packed multiplication for static-body replay

- Added PABSW and PMULTH to the bounded integer oracle after reviewing primary
  PCSX2 instruction semantics. PABSW saturates the signed minimum; PMULTH
  snapshots complete sources, writes all eight signed halfword products into
  128-bit HI/LO and writes even products to the destination. A zero destination
  still changes HI/LO. Subsequent scalar MULT/DIV retain upper accumulator
  halves, matching the original low-half-only writes.
- Three synthetic test methods cover literal lane order, signed boundaries,
  aliased/zero sources and destinations, accumulator replacement and scalar
  upper-half preservation. All 169 Python tests pass in 2.402 seconds; all 58
  independent float-diagnostic checks also pass. No original instruction list
  or third-party implementation is tracked.
- This enables independent static-body instruction replay. It does not itself
  integrate the body's collision response or a playable frame. The autonomous
  work continues locally; nothing is pushed.

## 2026-10-11 — Integrate genuine movement callers and animation clocks

- Integrated complete CAF8/D100 native target-speed/turn callers using real
  direction/rotation gateways, mapped time/coefficient globals and actor state.
  Root independently compiles and strictly links both PS2 bodies as NONMATCHING
  (192/200 and 572/592), then checks the integrated CMake diagnostic on 6,272
  actual/rebuilt cases: 4,009,074 instructions, 1,024 ordered coefficient aliases
  and 128 real input-bridge/target/turn sequences. The 6,261 native completions,
  11 atomic refusals and full RAM images agree outside stated float tolerances;
  maximum absolute difference 4.7683716e-7. Small real target angles still expose
  an inherited read-domain limit in eight sequences; none are clamped to zero.
- Added complete 6834C8 accumulated-motion effects and a public transactional
  vertical projection entry reusing the existing68ABA8 recovery. Root's fresh
  helper replay covers 2,048 cases/361,216 EE instructions, all PS2 modes and 512
  native mode0 cases, with the three actual callees and zero captures. Raw W,
  signed reductions, threshold clears and atomic refusal are preserved. The
  new PS2 draft remains NONMATCHING 196/184; velocity checks increase to 1,594.
  These helpers update vectors, not positionA0 or collision response.
- Integrated primary 589CE0 temporal effects with genuine frame durations and
  explicit selected definition/rate. Root's fresh SN body remains NONMATCHING
  848/696. Its integrated native diagnostic agrees on 911 original fixtures and
  893 extra fixtures with mixed non-dyadic durations/rates: 144,110 original EE
  instructions, 181,831 SN instructions, 461,824 actor bytes and zero differences.
  Linked layers and negative fractional elapsed cases refuse atomically; no
  unsupported reverse sign is treated as an acceptable arithmetic difference.
  All 16,335 focused checks pass. No clip selection or update frequency is added.
- Integrated complete 58ACB8 list cleanup through the recovered release helpers.
  Its true span is 88 bytes including JR/delay; the splitter's 80 truncated return.
  Root's fresh 80-byte PS2 draft strictly links as NONMATCHING. Actual original,
  rebuilt and native execution agree on 1,024 cases/3,448,832 bytes, including 310
  cycles, 1,534 metadata-to-node aliases and 994 occupancy aliases. Next is read
  before clear; all accesses are preflighted before publication. There are
  731,870 original and 730,910 SN instructions, with every callee executed.
- All 61 registered native tests pass in 4.28 seconds after integration. The
  existing sampled animated-model launcher remains available. Full layer-weight
  clock composition, small-angle movement domain and static-body/capsule
  collision response continue in parallel. No complete position/player/frame
  loop or new matching acceptance is claimed; nothing is pushed.

## 2026-10-11 — Enable rebuilt static-body logical-shift replay

- Added scalar SRLV to the bounded integer oracle after reviewing the primary
  PCSX2 R5900 instruction source. It shifts the low32 source word logically
  by the low five count bits, sign-extends the result into low64 and preserves
  the destination's upper64. Sources are captured before destination writes;
  the zero register stays immutable.
- One authored test method checks all 64 count encodings, zero/same/independent
  sources, four destination choices, aliases and nonzero upper register halves.
  All 170 Python tests pass in 2.393 seconds; all 58 independent COP1/VU diagnostic
  checks also pass. No original instruction list or third-party code is tracked.
- This allows the complete newly compiled static-body body to execute through
  the independent oracle with its real helpers. That collision lot remains in
  final validation; SRLV itself adds no native movement or playable frame.
  Nothing is pushed.

## 2026-10-11 — Integrate height offset, static body and animation layers

- Recovered complete 0068A178 (456 bytes including padding), keeping modes 1/2
  normalization and projection dependencies. The fresh SN entry is 252 bytes
  plus a local helper and remains NONMATCHING. Native mode 0 subtracts the real
  offset from source Z and preserves destination X/Y/W, with ordered aliases
  and atomic refusal. Root replays 2,048 original/SN cases and 512 native cases:
  363,008 instructions, 293 aliases, 128 exact signed-zero cases, zero observed
  difference. Velocity's 1,639 focused checks pass.
- Integrated the complete 00542280 static-body adapter and reverse contact
  collector, with 45 assertions. The fresh SN object strictly links NONMATCHING
  against the full 1416-byte span. Root's integrated DLL agrees with original
  instructions on 3,009 fixtures: 857 contacts, 2,151 misses and one actual
  signed-SUB trap refused atomically; 5,532,024,832 backing bytes, 4,794,058 EE
  and 132,374 genuine microbank instructions. Fresh complete SN execution also
  agrees on 600 cases spanning all 250 authentic faces and nine edge cases:
  7,139,384 EE and 26,774 microbank instructions, no captured helper results.
  The primary capsule and player motion
  frame remain in progress; this entry is supplementary static correction.
- Integrated complete 00589CE0 layer clock/weight/unlink effects using genuine
  cleanup/release components. Root's integrated DLL agrees with actual original
  and SN instructions on 460 supported cases, including 128 original clip
  metadata fixtures, 108 pre-cleanups and 52 metadata-to-decay aliases. There are
  145,507 original and 168,443 SN instructions, 1,718,088 mapped bytes and zero
  differences. 180 unsupported negative fractional cases refuse atomically.
  All 4,595 focused checks pass. No event scheduler or cadence is invented.
- All 63 registered native tests pass in 3.96 seconds; all 170 Python tests pass
  in 2.456 seconds. The animated native preview remains available but Ratchet
  is not controllable. Original-clock preview integration and the primary
  capsule continue in parallel. No matching acceptance or playable frame is
  claimed; generated assets and private proofs remain ignored. Nothing pushed.

## 2026-10-11 — Validate packed AND for capsule replay

- Added PAND to the scoped integer oracle after reviewing the primary PCSX2
  R5900 implementation. Both full 128-bit sources are captured before the
  destination write; the zero register and HI/LO remain unchanged.
- Authored cases check each bit from 0 through 127 against full/complement/self
  masks, literal mixed-lane expectations, zero sources and all destination
  aliases. All 171 Python tests pass in 2.337 seconds. This enables the actual
  capsule/tree path to execute in validation; it adds no native player behavior.
- The preceding static-body integration proof finished before this oracle
  change and retains its recorded input hashes. The new capsule replay uses
  the new frozen oracle. Nothing is pushed.

## 2026-10-11 — Show recovered primary clock in the native preview

- Added a bounded C++ clock attachment and local `.dlclock` packaging with
  original clip0 durations/metadata, exact pose labels, explicit flags and
  fingerprints of scene and complete pose data. Each presentation tick calls
  the recovered primary 589CE0 component and requires an exact pose result;
  unsupported states refuse atomically without interpolation or nearest labels.
- Root rebuilt the production libraries and regenerated the direct-ISO package.
  Their isolated validation bridge agrees with original instructions on 120
  calls/121 states: 9,364 instructions, 10,406 actor bytes and zero differences.
  All 60 pose labels are reached; 394 authored and 122 actual-package attachment
  checks pass.
- The main launcher is switched only after build, package, original replay and
  actual WARP captures succeed. Tick 0/tick 16 captures contain 123,204/122,623
  non-background pixels and are inspected. Presentation tick rate remains an
  explicit diagnostic setting; no PS2 update frequency is inferred. Poses stay
  precomputed; player controls, secondary blending, events and gameplay are absent.
- Recorded a new local GIF from 60 distinct actual integrated-clock captures.
  Its 60 encoded frames retain a one-second presentation loop using the GIF
  format's 10ms timestamp units. The prior sampled GIF remains available.
- All 64 registered native tests pass in 3.93 seconds; all 171 Python tests pass
  in 2.359 seconds. The playable objective remains active; nothing is pushed.

## 2026-10-11 — Preserve genuine small-angle movement inputs

- Selected turn/facing/pitch inputs and movement components in normalization,
  planar projection and accumulation now have separate normal-input floors of
  2^-40. Scalar, rate, axis and height-offset contracts remain unchanged. The
  original ordered C60 polynomial flushes separately rounded subnormal results
  to their signed zero in an explicitly approximate host model.
- Root rebuilt the production DLL, freshly compiled and strictly linked twenty
  complete SN drafts, and guarded source, oracle, library, object and original
  input hashes before/after all replays. All drafts remain NONMATCHING; matching
  acceptance is not increased.
- Actual original/SN/native execution agrees on 9,216 rotation/input-sequence
  cases and 5,120 velocity cases. The C60 sensitivity audit completes all 2,300
  small inputs with identical final words under separate rounding and flushing.
  Camera agrees on 576 complete cases and 1,030 direct C60 cases; orientation
  agrees bitwise with actual B58/C40/B10 on 2,105 cases and 160,039 pairs.
- Component validation covers 4,096 cases/1,366 aliases, including 512 real
  velocity-to-accumulation sequences: 480 complete; 32 below-floor products
  refuse atomically. Small addends differ by at most one observed ULP; one
  cancellation residue differs by 1.1641532e-10 and is reported explicitly.
  Signed-zero checks cover 21,052 words, including 8,898 negative zeros.
- All 65 native tests pass in 3.97 seconds; the new integration fixture adds 82
  checks. The native clock animation remains testable through the launcher.
  Active walking still needs the genuine wall probe, position/ground update and
  actor-frame composition. Capsule, walk parameters and animation-wrapper work
  continue in parallel. Nothing is pushed.

## 2026-10-11 — Integrate walk parameters and reached animation update

- Added the full selected state-2 mode-0 00681B30 native parameter update with
  real history/getter/wrap/progression calls and ordered global reloads. Full
  copied writable views commit only after a supported completion. Four complete
  PS2 drafts retain other states/projection and stay NONMATCHING.
- Root freshly compiled/strictly linked the four drafts and linked an independent
  bridge against the central production libraries. Guarded original/SN/native
  comparison passes 4,608 state-2 cases/512 aliases/20 atomic numeric refusals:
  1,242,567 EE instructions and zero observed difference. The wider PS2 proof
  covers 5,632 cases; adjacent expanded C78/C88 leaves are replayed separately.
  Seven real bridge/dispatcher snapshots agree; four active original frames
  still stop at genuine wall VU E90. All 90 focused checks pass.
- Added reached 00589F98 CPU effects with true layers, phase, scheduler, query
  and release. Event/allocation 626D50 and reached mode-1 predicate 665A28 refuse
  atomically. The wrapper does not clear events or skip reached calls to succeed.
  Its native fixture passes 87,899 checks.
- Root freshly compiled the five complete wrapper/scheduler drafts and compared
  current central-production libraries on 1,536 original/SN cases: 305,374
  original/287,623 rebuilt instructions, 17,836,032 bytes and zero differences.
  There are 1,293 schedulers, 2,997 queries, 688 releases, 255 layer cases and 94
  event-count aliases. Another 96 atomic refusals reach the real unavailable
  dependency in original execution. Strict linking finds 6268A8 exact48/48;
  local drafts are not promoted to accepted normalized overlay coverage.
- All 67 registered native tests pass in 4.10 seconds. Hash-guarded production
  receipts preserve source/oracle/library/original inputs throughout execution.
  No new visible player controls or playable frame are claimed. The next lots
  target actual wall contact, position/radius helpers and walking clip selection.
  No original assets or instruction bytes are tracked. Nothing is pushed.

## 2026-10-11 — Integrate vertical capsule terrain contact

- Added the genuine 0053FAA8 terrain/quad contact and dynamic-filter native
  transaction. Real generation/visitation effects survive misses; eligible
  unrecovered dynamic geometry refuses atomically. Radius/height and incoming
  unsaved VF W lanes remain explicit inputs. The full 3976-byte PS2 draft
  preserves model/primitive, cache, DMA, VU and custom-ABI tree branches.
- Root freshly compiled and strictly linked the complete NONMATCHING draft,
  then compared the central production libraries over all 8,249 actual
  nonempty cells: 3,107 contacts, 5,142 misses, 13,993,800 EE instructions,
  362,956 micro instructions and 21,807,452,352 RAM bytes without differences.
- Separate flags-zero and mask-two original/SN/native samples each cover 250
  cells, 65 contacts, 185 misses, 5,200,745 combined EE and 22,000 micro
  instructions. Thirteen dynamic-filter/alias fixtures pass; twelve genuine
  dynamic geometry contacts validate the complete PS2 draft separately.
- Source, original inputs, private oracles, fresh objects, linked code and
  production libraries remain hash-bound and unchanged throughout the replay.
  All 49 focused checks and all 68 central native tests pass (3.80 seconds).
- Host binary32/saturated-Q arithmetic remains approximate. Reset-empty actor
  grids are declared fixtures, not initialized gameplay. Wall/ground response
  and full player-frame composition remain active work. The animated native
  viewer is unchanged. No retail bytes/assets are tracked; nothing is pushed.

## 2026-10-11 — Extend the bounded wall-validation integer oracle

- Added the actual PSRAH instruction needed by original wall-grid indexing,
  plus MTLO/MADD emitted by the fresh complete wall-probe SN recovery.
  PSRAH snapshots all eight signed halfwords and masks its immediate to four
  bits. MTLO preserves upper accumulator bits and copies low64 without word
  extension. MADD wraps the assembled HI.low32/LO.low32 plus signed32 product,
  sign-extends the two result words and preserves each upper64 half.
- Independently checked these instruction facts against primary PCSX2
  [MMI.cpp](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/MMI.cpp) and
  [R5900OpcodeImpl.cpp](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/R5900OpcodeImpl.cpp).
  No implementation was copied. Synthetic tests cover all 32 encoded PSRAH
  counts, signed lanes, zero/aliased destinations, HI/LO preservation, MTLO
  raw64 transport and MADD carry/wrap/signed products/zero-source/rd aliases.
- All 174 Python tests pass in 2.432 seconds. This changes validation tooling,
  not the native game loop. The capsule replay completed under its explicitly
  hash-bound prior oracle before this extension. Nothing is pushed.

## 2026-10-11 — Integrate position helpers and genuine clip selection

- Added complete selected mode-zero projection/offset/heading/settling,
  scalar/vector progression and metadata-driven radius effects. Full 128-bit
  heading arguments, raw W transport, actual C60, ordered aliases and real
  progression remain intact. Seven complete PS2 drafts retain all modes and
  stay NONMATCHING; the larger rebuilt heading leaf is replayed in isolation.
- Root freshly rebuilt/strictly linked these drafts and compared current
  central production libraries on 7,168 original/SN cases (1,236,192 EE and
  257,880 actual C60 pairs), plus 3,584 native cases, 512 aliases, 128 edges
  and eight atomic numeric refusals. Native maximum observed absolute float
  difference is 2.3841858e-7; all other RAM and 2,878 signed-zero comparisons
  agree. The native fixture passes 136 checks.
- Added actual 00688C08 selection through true allocation, loop, weapon gates
  and cleanup. Reached unsupported callbacks/releases refuse atomically;
  metadata, pointers and selector/event counts are not changed to avoid them.
  The fixture passes 10,306 checks.
- Root's fresh central-production replay covers 662 completed original/SN/
  native cases, all 112 original clips, 90 metadata aliases, 587 allocations,
  18 actual exhausted pools and 58 separately checked dependency refusals.
  All 10,222,804 RAM bytes agree. Another 128 real state-two selection blocks
  reach true health/clip selection: clips 5/frame16 or 6/frame15 and blends
  8/12; 2,036,736 bytes agree without capturing any helper return. These are
  explicit caller/block fixtures, not full entry or initialized gameplay.
- All source/oracle/asset/object/linked-code/production-library hashes remain
  unchanged throughout the root replay. Allocator 58A850 and loop 688E60
  compare exact locally; accepted matching progress remains 280 functions.
  All 70 native tests pass in 4.92 seconds. Physical PS2 arithmetic, full
  constructor/state entry, ground/position frame and live pose/blend rendering
  remain unverified. The animated viewer is unchanged. Nothing is pushed.

## 2026-10-11 — Recover complete walking update and native direction history

- Retained the complete 00695CB8 state-two update outside accepted source.
  Its true history, turn, parameter, acceleration/velocity, wall, optional
  terrain/slope branches remain intact. Fresh SN/strict linking is NONMATCHING:
  2468 rebuilt bytes versus the genuine 2404-byte span. The expanded draft is
  executed in isolation from the adjacent entry, with default arithmetic;
  fast-math reassociation was corrected before this validated checkpoint.
- Full original/SN replay passes 672 cases: 76 complete and 596 explicitly
  stopped at the genuine wall-probe entry, without replacing any callee.
  This includes 160 actual projected contributions and 32 actor/global aliases,
  746,576 original/760,304 rebuilt EE instructions and zero RAM differences.
  Freshly promoted source/code and unchanged private inputs are hash-bound.
- Added complete native 005BD900 signed angle-history reading: null selects
  actual mapped001EE600; head/count, wrapped32 arithmetic and signed remainder
  retain their real behavior. Raw returned float words are never evaluated or
  normalized. Missing/misaligned reached reads refuse before publication.
- Central-production original/SN/native replay passes 1,920 cases, four caller
  output aliases and four real reached-memory refusals. It compares 8,847,360
  RAM bytes and executes 32,640 original/32,000 rebuilt instructions without
  differences. The focused fixture passes 3,851 checks; all 71 central native
  tests pass in 4.94 seconds. Matching progress remains 280 functions.
- Optimized private replay by validating immutable complete instruction maps
  once, retaining fresh per-case RAM and reset architecture/trace state. All
  672 state-two per-case results and resolved code hashes agree with the prior
  replay; observed local duration drops from 362.66 to 7.34 seconds (~49x).
  The redundant slow history replay was stopped only after an independent
  complete original/SN/native proof passed. Agent jobs were left running.
- Actual wall/ground response, complete state entry/frame composition and
  live rendered pose/blend remain active work. No playable frame is claimed;
  the visible animation preview is unchanged. Nothing is pushed.

## 2026-10-11 — Integrate capsule settlement and genuine position retries

- Added the whole selected 0067EF00 contact settlement and 0067F518 retry/
  history effects, calling the production capsule and supplementary static
  probes. True radius/height, dot/state selection, item suspension, packet
  aliases, contact limits and VF25..29 W lifetimes are retained. Previous17
  genuinely selects the unavailable sphere immediately and refuses atomically.
- Root verified the frozen five-file lot, freshly compiled/strictly linked
  both complete PS2 drafts and built an independent central-production bridge.
  Full guest-RAM comparison passes 128 actual cell cases/30 contact cases/
  245 contact iterations: 2,239,640 EE and 78,848 bank microinstructions.
- Another 57 whole-call cases exercise nonempty static geometry, ordered
  item/global and packet/actor aliases, real state flags, signed-zero inputs,
  genuine EF00 -1 returns and sixteen F518 retries/history counts1/2/32.
  They execute 981,394 EE and 42,592 microinstructions with no RAM differences.
  Seven real unavailable-callee snapshots agree on RAM/ABI and stop explicitly;
  no captured return or successful replacement call is used.
- All source/oracle/asset/object/linked-code/production-library hashes remain
  unchanged throughout the 54.47-second integration replay. EF00 compiles to
  1304 entry bytes plus 160 local helper bytes within the original1560; F518
  is212 versus204 and replayed in isolation. Both remain NONMATCHING.
- All 94 focused checks and all 72 central native tests pass (9.52 seconds).
  Matching progress stays280 functions. Approximate scalar/VU/probe-Q arithmetic
  does not establish physical flags/DMA/timing. Genuine wall/ground response,
  outer position and whole player frames remain active work. No new visible
  controls or playable frame are claimed; nothing is pushed.

## 2026-10-11 — Unblock complete ground-vector replay

- Extended the bounded integer diagnostic with MTSAB byte-position selection
  and QFSRV full128-bit source windows. Source snapshots retain destination
  aliases, zero-register behavior and HI/LO; every run resets SA.
- Added independent byte-window expectations, exhaustive shift/source/target
  combinations, low-nibble selector cases and initialized-accumulator/reset
  checks. All178 Python tests pass (3.05 seconds). MTSAH remains unsupported.
- Instruction semantics were cross-checked against the primary PCSX2
  R5900OpcodeImpl.cpp and MMI.cpp descriptions; no implementation was copied.
  This diagnostic does not establish physical PS2 timing or hardware identity.
- The new shared CPU SHA256 is
  bd0797db5a6f2da3b5e648a8d368a347a59f1bcbb230e580ec48cb13175bc723.
  Completed earlier receipts retain their original CPU identities; their
  frozen sources are not silently relabeled. The live historical wall replay
  uses its separate immutable private CPU snapshot.
- Genuine596390 execution is now available to the ongoing ground-step lot.
  This infrastructure change does not add visible controls or a player frame.

## 2026-10-11 — Recover complete native slope response

- Added complete NONMATCHING0067D4E8 PS2 source and a transactional native
  response preserving the true contact gates, angle polynomial, planar
  normalization, raw velocity W and ordered scale/time reads between stores.
- Genuine terrain normals are unnormalized cross products. The native domain
  supports their component magnitudes through2^30 without replacing them
  with unit vectors. Original coefficient tables are checked when reached;
  changed tables and unsupported arithmetic refuse atomically.
- Independent original/SN/central-production replay passes487 whole calls,
  including146 real terrain-contact normals and133 corrections. All selected
  non-floating bytes agree; maximum velocity error is2.384185791015625e-7.
  The run compares6,305,628 RAM bytes, executes67,535 original/71,972 SN EE
  and retains363 exact signed-zero components. No calls are captured.
- The66-check focused fixture covers ordered time aliases, lazy gates,
  missing memory, changed tables, raw W, numerical bounds and atomic refusals.
  SN has408 entry bytes plus208 local helper bytes versus original508;
  replay is isolated rather than claiming an in-place replacement.
- Matching progress remains280 functions. No new rendered controls, ground
  query, full frame or playable character is claimed; nothing is pushed.

## 2026-10-11 — Integrate real initial/idle/walking state entry

- Added the frozen native699C70 selected states0/1/2 with common prologue
  and suffix, genuine histories/input/animation ownership and whole-memory
  atomic refusal. It uses the real entry ABI, not an internal dispatch label.
- Root independently rebuilt ten complete PS2 prerequisites, linked a DLL to
  central production libraries and repeated complete original/native and
  fresh original/SN proofs. All bound inputs remained unchanged for11.99s.
- Complete replay passes1,536 transitions,128 real retail gate rejections and
  160 actual reached-dependency refusals, including768 ordered client fields
  aliasing history/handles. All1,444,634,880 compared bytes agree over1,225,684
  original instructions using genuine112-clip Ratchet metadata; zero captures.
- Fresh prerequisites pass8,256 original/SN cases. Two compare locally exact,
  but accepted matching coverage remains280. All16,764 focused native checks
  and all74 central tests pass (4.67 seconds for the latter).
- Prioritized genuine00690F18 scheduling and the ordinary selected frame
  closure over further all154-state dispatcher expansion. Full construction,
  wall/ground closure, outer position and live pose/blend remain work; the
  existing preview remains camera-controlled. Nothing is pushed.
