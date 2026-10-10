# Local development progress

## Objective and validation

Work toward a launchable PS2 build and the beginning of the campaign. Keep all
changes local until the user approves a push. Source and documentation are in
English. Follow CONTRIBUTING.md and LEGAL.md: retail inputs, generated assembly,
SDK binaries, and reference material stay in ignored local directories.

Distinguish these milestones: function compilation, masked comparison, exact
comparison after relocation, complete image reconstruction, emulator boot,
and campaign play. A passing earlier milestone does not prove a later one.

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

## Next milestones

1. Test boot and the first playable Dreadzone segment when emulator and BIOS
   paths are available; record actual observations rather than inferred success.
2. Continue resident startup/loading code and first-level helpers with exact
   linking as the acceptance check. Keep failed drafts outside src.
3. Extend exact linking to the open-source runtime libraries and ultimately
   rebuild more sections and overlays from source rather than retail fallback.
