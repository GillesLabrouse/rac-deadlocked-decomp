# Exact hybrid reconstruction

The matching workflow now has an optional reconstruction milestone. It links
compiled game functions at their original addresses, checks all instruction
bytes including relocations, and reconstructs the resident executable with
retail fallback for everything that has not been decompiled.

This is a PS2 build, not a native PC port or a build entirely from C. Libraries
and unconverted functions remain retail fallback in the reconstructed image,
even when the separate library audit validates their C. Overlay functions can
be linked and validated but currently remain retail fallback in disc packaging.
Reconstruction does not change decompilation progress counts.

## Prepare local inputs

Follow CONTRIBUTING.md for the compiler mirrors and pinned Python environment.
An ISO can supply the executable without mounting the disc:

```sh
python tools/extract_baserom.py "/path/to/Ratchet - Deadlocked.iso"
bash tools/setup_asm.sh
```

The extractor reads only `SCUS_974.65`, validates its SHA-1, and writes it to
ignored `baserom/`. It rejects other game versions. For overlay validation:

```sh
wrenchbuild unpack "/path/to/Ratchet - Deadlocked.iso" -o private/overlays -g dl -r us
```

## Build and validate on Windows

Run from Git Bash. The compiler runs natively; Wine is not required. Scripts
select `venv/Scripts/python.exe` automatically. Install dependencies with
`venv/Scripts/python.exe -m pip install -r requirements.txt` first.

```sh
bash tools/build.sh
bash tools/build_libgcc.sh
bash tools/get_newlib.sh
bash tools/build_libm.sh
venv/Scripts/python.exe tools/audit_matches.py
venv/Scripts/python.exe tools/link_objects.py --overlays
venv/Scripts/python.exe tools/rebuild_image.py
venv/Scripts/python.exe tools/gen_progress_report.py
venv/Scripts/python.exe tools/gen_progress_report.py --check
python -m unittest discover -s tools -p 'test_*.py'
```

Omit `--overlays` for a resident-only link audit. The full contribution audit
still needs extracted overlays for existing overlay C.

### Check an explicitly addressed overlay draft

Compile a draft into `build/drafts`, keeping it outside accepted objects. Pass
any compiler flags declared in its first-line `cflags` comment explicitly when
invoking `cc.sh` directly. For example, the following L01 initializer uses the
default compiler flags:

```sh
bash tools/cc.sh nonmatching/L01_00628020.c build/drafts/L01-00628020.o
venv/Scripts/python.exe tools/check_overlay_draft.py L01 build/drafts/L01-00628020.o
```

The checker derives actual function extents from the locally extracted overlay
and resolves every relocation through the original linker. It verifies source
provenance and rejects foreign-overlay symbols and unknown entry boundaries.
Reports are written under `build/drafts/linked`, never `build/linked/report.json`
or matching coverage. A nonmatch exits with status one and records the differing
byte offsets; an exact draft still needs canonical source naming and the full
accepted build checks before being added to `src/`. Use explicit local symbols
after independently checking their calls and data, rather than treating an
overlay grouping identity as proof of equivalent bindings.

On Linux/macOS, use the container wrapper for the build scripts and
`python tools/link_objects.py --overlays`; use `venv/bin/python` for the other
Python commands. Those platforms retain Wine. The Windows workflow was
exercised locally; these changes have not yet been run on Linux/macOS.

## What validation proves

- `audit_matches.py` masks only fields with actual ELF relocations. Numeric
  constants and pointer strides stay significant. Mismatches and empty builds
  fail. `retail.normalise` remains broad for grouping overlay copies and must
  not be used as a C correctness check.
- `link_objects.py` separates compiler-emitted functions into independently
  placed sections. Reassembly must preserve the original object's instruction
  bytes. The real linker resolves calls, globals and `$gp` offsets. Each linked
  function must equal retail without masking. Unknown symbols and additional
  allocated data sections are rejected.
- Shared overlay names encode their first level. Resolve them in the current
  level using their independently computed grouping identity. Multiple local
  candidates are rejected: use an explicit local-address name after inspecting
  the original call. The final unmasked comparison still judges the result.
- Object manifests record sources, project headers, pipeline inputs, object
  bytes and compiler assembly. Linking and reconstruction reject stale inputs.
  Recompile after editing a source or build script.
- `rebuild_image.py` inserts only exact linked resident code. Functions may
  not overlap or cross code-section boundaries. The entire reconstructed
  section stream must equal the verified original.

Reports and binaries stay under ignored `build/linked/` and `build/rebuilt/`.
Their binary contents must never be committed.

## Disc packaging and runtime testing

```sh
venv/Scripts/python.exe tools/package_iso.py "/path/to/Ratchet - Deadlocked.iso"
```

Output: `build/rebuilt/deadlocked.iso`. Matching reconstruction reproduces the
original section stream, so packaging reuses its verified original compressed
representation and original disc layout. The output is byte-identical to the
input, checked with SHA-256 and by re-extracting the boot executable. This avoids
moving data read by sector or repacking undecompiled assets. Packaging is a
validation milestone; it does not increase decompilation coverage by itself.

PCSX2 testing requires the user's emulator and console BIOS. Boot the generated
ISO, record the emulator version/settings, confirm the title/menu, start a new
campaign, and record the first playable Dreadzone segment and any failures.
Keep screenshots, logs and BIOS in ignored local directories. Until performed,
record runtime testing as unverified; binary identity is not a gameplay test.
