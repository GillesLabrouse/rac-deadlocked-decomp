#!/usr/bin/env python3
"""
Try several C variants of the same function against retail in one go.

  venv/bin/python tools/try_variants.py FILE.c  name=func_00497438 ...
FILE.c holds functions with distinct names (v1, v2, ...). Each argument maps a
function name in FILE.c to the retail function it should match. The file is
compiled through tools/cc.sh (native Windows or the Wine container), then every
function is compared using actual object relocations like tools/audit_matches.py.
EXTRA supplies additional compiler flags; first-line cflags are also honored.
"""
import os
import re
import shlex
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import retail as mp  # noqa: E402
from reloc_compare import relocated_equal, function_relocations  # noqa: E402
from elftools.elf.elffile import ELFFile  # noqa: E402

ROOT = mp.ROOT


def main() -> int:
    src = Path(sys.argv[1]).resolve().relative_to(ROOT)
    pairs = [a.split("=") for a in sys.argv[2:]]
    out = ROOT / "build" / "try.o"
    first_line = (ROOT / src).read_text().splitlines()[0]
    cflags = re.search(r"/\* cflags: (.*?) \*/", first_line)
    flags = shlex.split(cflags[1] if cflags else "") + shlex.split(os.environ.get("EXTRA", ""))
    command = ["bash", "tools/cc.sh", src.as_posix(), "build/try.o", *flags]
    if os.name == "nt":
        command[0] = str(Path(os.environ.get("ProgramFiles", "C:/Program Files")) / "Git/bin/bash.exe")
    else:
        command = ["bash", "tools/docker/run.sh", *command]
    subprocess.run(command, cwd=ROOT, check=True)
    sizes = {}
    for l in (ROOT / "config" / "functions.tsv").read_text().splitlines()[1:]:
        n, a, s, _ = l.split("\t")
        sizes[n] = (int(a, 16), int(s, 16))
    with open(ROOT / "baserom" / "SCUS_974.65.elf", "rb") as f:
        secs = mp.sections(ELFFile(f))
    elf = ELFFile(open(out, "rb"))
    data = elf.get_section_by_name(".text").data()
    syms = {s.name: s for s in elf.get_section_by_name(".symtab").iter_symbols()
            if s["st_info"]["type"] == "STT_FUNC"}
    for name, target in pairs:
        s = syms.get(name)
        if not s:
            print(name, "missing")
            continue
        code = data[s["st_value"]: s["st_value"] + s["st_size"]]
        addr, size = sizes[target]
        retail = mp.read(secs, addr, size)
        pad = size - len(code)
        if 0 < pad <= 12 and not any(retail[len(code):]):
            retail = retail[:len(code)]
        relocs = function_relocations(elf, s)
        ok = len(code) == len(retail) and relocated_equal(code, retail, relocs)
        eq = sum(relocated_equal(code[offset:offset + 4], retail[offset:offset + 4],
                                 [(0, kind) for pos, kind in relocs if pos == offset])
                 for offset in range(0, min(len(code), len(retail)), 4))
        print("%-10s -> %s  %s  (%d/%d words)" % (name, target, "EXACT" if ok else "no", eq, len(retail) // 4))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
