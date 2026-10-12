#!/usr/bin/env python3
"""Resolve an actual overlay address to catalog navigation candidates.

Normalized code masks real constants as well as addresses. A candidate is not
proof of equivalent semantics, relocations, matching coverage or true bounds.
Inspect each level's actual calls, globals and complete CFG before recovery.
This tool reads local ELFs and never changes sources, metadata or build outputs.
"""
import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

from elftools.elf.elffile import ELFFile

import check_recovery_candidates as guard
import gen_progress_report as progress
import retail
import split_overlays


def resolve(level, address, size, elf_root):
    directories = dict(progress.overlay_levels())
    cache = {}

    def image(selected):
        if selected not in cache:
            path = elf_root / "levels" / directories[selected] / "overlay.elf"
            with path.open("rb") as stream:
                section = ELFFile(stream).get_section_by_name(".text")
                if section is None:
                    raise ValueError(f"Missing .text: {path}")
                base, data = section["sh_addr"], section.data()
            cache[selected] = (path, base, data,
                               {a: n for a, n, _ in split_overlays.functions(data, base)})
        return cache[selected]

    def body(selected, start, length):
        path, base, data, boundaries = image(selected)
        offset = start - base
        if offset < 0 or offset + length > len(data):
            raise ValueError(f"Range outside {selected} .text: {start:08X}/{length:X}")
        code = data[offset:offset + length]
        calls = [{"pc": f"{start + index * 4:08X}",
                  "target": f"{((start + index * 4 + 4) & 0xF0000000) | ((word & 0x3FFFFFF) << 2):08X}"}
                 for index, (word,) in enumerate(struct.iter_unpack("<I", code))
                 if word >> 26 == 3]
        return code, {"level": selected, "elf": path.relative_to(retail.ROOT).as_posix()
                     if path.is_relative_to(retail.ROOT) else str(path),
                     "address": f"{start:08X}", "size": f"{length:X}",
                     "raw_sha256": hashlib.sha256(code).hexdigest(),
                     "generator_extent_agrees": boundaries.get(start) == length,
                     "direct_calls": calls}

    code, target = body(level, address, size)
    digest = split_overlays.digest(code)
    target["normalized_digest"] = digest
    matches = []
    scanned = 0
    for line in progress.OVERLAY_FUNCS_TSV.read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        name, start, length, kind, levels = line.split("\t")
        if level not in levels.split(",") or int(length, 16) != size:
            continue
        canonical = re.fullmatch(r"func_(L\d{2})_[0-9A-F]{8}", name)
        if canonical is None:
            raise ValueError(f"Malformed canonical name: {name}")
        other, detail = body(canonical.group(1), int(start, 16), size)
        scanned += 1
        if split_overlays.digest(other) != digest:
            continue
        detail.update({"name": name, "class": kind, "levels": levels.split(","),
                       "raw_equal": other == code,
                       "different_word_offsets": [f"{i:X}" for i in range(0, size, 4)
                                                  if other[i:i + 4] != code[i:i + 4]],
                       "guard": guard.inventory([name])[0]})
        matches.append(detail)
    return {"target": target, "catalog_rows_scanned": scanned,
            "navigation_candidates": matches,
            "status": "unique_navigation_candidate" if len(matches) == 1 else
                      "ambiguous_navigation_candidates" if matches else "no_navigation_candidate",
            "scope": "Navigation only. Broad normalization is not an actual-relocation or resolved exact comparison; bounds and each level's calls/globals still require independent verification."}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--level", required=True, choices=[x for x, _ in progress.overlay_levels()])
    parser.add_argument("--address", required=True, type=lambda x: int(x, 0))
    parser.add_argument("--size", required=True, type=lambda x: int(x, 0))
    parser.add_argument("--elf-root", type=Path, default=retail.ROOT / "private" / "overlays")
    args = parser.parse_args()
    if args.size <= 0 or args.size % 4 or args.address % 4:
        parser.error("Address and positive size must be word-aligned.")
    try:
        result = resolve(args.level, args.address, args.size, args.elf_root.resolve())
    except (OSError, ValueError, KeyError) as error:
        parser.exit(2, f"Cannot inspect actual overlay inputs: {error}\n")
    print(json.dumps(result, indent=2))
    return 0 if len(result["navigation_candidates"]) == 1 else 1


if __name__ == "__main__":
    raise SystemExit(main())
