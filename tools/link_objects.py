#!/usr/bin/env python3
"""Link game functions at retail addresses and compare unmasked bytes.

Unlike audit_matches.py, calls, global addresses, small-data offsets and all
constant immediates must match. Uses the real GNU linker from the local SN
mirror. Each compiler-emitted function gets its own section without changing
instructions. Reassembly is checked against the original build/obj object
before linking. Unknown symbols and additional data sections fail closed.
Outputs stay in build/linked; this does not change progress/report.json.
"""
import argparse
import io
import json
import os
import re
import struct
import subprocess
from pathlib import Path

from elftools.elf.elffile import ELFFile
import retail
from record_build import validate
from split_overlays import digest, functions as overlay_functions

ROOT = retail.ROOT
BIN = ROOT / "toolchain/sn-prodg-3.01/usr/local/sce/ee/gcc/bin"
FUNCTION = re.compile(r"func_(?:L\d\d_)?[0-9A-F]{8}$")
ADDRESS = re.compile(r"(?:func|D)_(?:L\d\d_)?([0-9A-Fa-f]{6,8})_?$")


def tool(name, *args):
    command = [str(BIN / name), *map(str, args)]
    if os.name != "nt":
        command.insert(0, "wine")
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    if result.returncode:
        raise ValueError((result.stderr or result.stdout).strip())


def function_sections(assembly):
    """Switch sections at .ent; leave all instructions and assembler state alone."""
    lines = []
    for line in assembly.splitlines(keepends=True):
        match = re.fullmatch(r"\s*\.ent\s+(func_(?:L\d\d_)?[0-9A-F]{8})\s*", line)
        if match:
            lines.append('\t.section .text.%s,"ax"\n\t.align 2\n' % match[1])
        lines.append(line)
    return "".join(lines)


def functions(elf):
    return {s.name: s for s in elf.get_section_by_name(".symtab").iter_symbols()
            if s["st_info"]["type"] == "STT_FUNC" and FUNCTION.fullmatch(s.name)}


def symbol_code(elf, symbol):
    return elf.get_section(symbol["st_shndx"]).data()[symbol["st_value"]:symbol["st_value"] + symbol["st_size"]]


def link_object(path, sizes, reference, gp, output, context="resident", resolve_overlay=None):
    inputs = validate(path)
    original = ELFFile(io.BytesIO(path.read_bytes()))
    funcs = functions(original)
    if not funcs:
        raise ValueError("no game functions")
    for section in original.iter_sections():
        if section["sh_flags"] & 2 and section["sh_size"] and section.name not in (".text", ".reginfo"):
            raise ValueError("unsupported allocated section: " + section.name)
    stem = path.stem
    source = ROOT / "build/tmp" / (stem + "_fixed.s")
    assembly = output / (stem + ".s")
    assembly.write_text(function_sections(source.read_text()), encoding="utf-8")
    separated = output / (stem + ".o")
    tool("ee-gcc2953.exe", "-c", assembly.relative_to(ROOT), "-o", separated.relative_to(ROOT))
    split = ELFFile(io.BytesIO(separated.read_bytes()))
    split_funcs = functions(split)
    if set(funcs) != set(split_funcs):
        raise ValueError("reassembly changed the function set")
    for name, symbol in funcs.items():
        if symbol_code(original, symbol) != symbol_code(split, split_funcs[name]):
            raise ValueError("reassembly changed code: " + name)
    definitions = {"_gp": gp}
    for symbol in split.get_section_by_name(".symtab").iter_symbols():
        if symbol["st_shndx"] != "SHN_UNDEF" or not symbol.name:
            continue
        overlay_symbol = re.match(r"func_(L\d\d)_", symbol.name)
        if overlay_symbol and overlay_symbol[1] != context:
            if resolve_overlay is None:
                raise ValueError("cross-overlay symbol needs a context: " + symbol.name)
            definitions[symbol.name] = resolve_overlay(symbol.name, context)
            continue
        match = ADDRESS.fullmatch(symbol.name)
        if not match:
            raise ValueError("unknown symbol: " + symbol.name)
        definitions[symbol.name] = int(match[1], 16)
    script = output / (stem + ".ld")
    lines = ["%s = 0x%X;" % (name, address) for name, address in sorted(definitions.items())]
    lines.append("SECTIONS {")
    for name in sorted(funcs, key=lambda n: sizes[n][0]):
        address, _ = sizes[name]
        lines.append(".text.%s 0x%X : { *(.text.%s) }" % (name, address, name))
    lines.append("/DISCARD/ : { *(.reginfo) *(.mdebug*) *(.pdr) *(.comment) }")
    lines.append("}")
    script.write_text("\n".join(lines) + "\n", encoding="utf-8")
    linked_path = output / (stem + ".elf")
    tool("ee-ld.exe", "-T", script.relative_to(ROOT), separated.relative_to(ROOT), "-o", linked_path.relative_to(ROOT))
    linked = ELFFile(io.BytesIO(linked_path.read_bytes()))
    linked_funcs = functions(linked)
    records = []
    for name in sorted(funcs):
        address, size = sizes[name]
        symbol = linked_funcs[name]
        section = linked.get_section(symbol["st_shndx"])
        offset = symbol["st_value"] - section["sh_addr"]
        code = section.data()[offset:offset + symbol["st_size"]]
        expected = retail.read(reference, address, size)
        if expected is None or symbol["st_value"] != address:
            raise ValueError("invalid placement: " + name)
        padding = size - len(code)
        if 0 <= padding <= 12 and not any(expected[len(code):]):
            code += bytes(padding)
        exact = code == expected
        record = {"name": name, "address": address, "size": size, "exact": exact,
                  "object": path.relative_to(ROOT).as_posix(), "context": context,
                  "source": inputs["source"], "object_sha256": inputs["object_sha256"]}
        if exact:
            binary = output / (name + ".bin")
            binary.write_bytes(code)
            record["binary"] = binary.relative_to(ROOT).as_posix()
        else:
            record["compiled_size"] = len(code)
            record["different_offsets"] = [i for i, (a, b) in enumerate(zip(code, expected)) if a != b][:16]
        records.append(record)
    return records


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--object", help="one build/obj object filename")
    parser.add_argument("--overlays", action="store_true", help="also link overlay objects against their local overlay ELFs")
    args = parser.parse_args()
    sizes = {}
    for line in (ROOT / "config/functions.tsv").read_text().splitlines():
        if line and not line.startswith("#"):
            name, address, size, _ = line.split("\t")
            sizes[name] = int(address, 16), int(size, 16)
    for line in (ROOT / "config/overlay_functions.tsv").read_text().splitlines():
        if line and not line.startswith("#"):
            name, address, size, _, _ = line.split("\t")
            sizes[name] = int(address, 16), int(size, 16)
    overlay_dirs = {}
    for line in (ROOT / "config/overlays.tsv").read_text().splitlines():
        if line and not line.startswith("#"):
            level, directory, _, _ = line.split("\t")
            overlay_dirs[level] = directory
    with (ROOT / "baserom/SCUS_974.65.elf").open("rb") as stream:
        reference = retail.sections(ELFFile(stream))
    gp = struct.unpack_from("<I", (ROOT / "baserom/sections/00_reginfo.bin").read_bytes(), 20)[0]
    output = ROOT / "build/linked"
    output.mkdir(parents=True, exist_ok=True)
    records, failures = [], []
    overlay_reference = {}
    overlay_indices = {}

    def read_overlay(context):
        if context not in overlay_reference:
            overlay_root = Path(os.environ.get("OVERLAYS") or ROOT / "private/overlays")
            with (overlay_root / "levels" / overlay_dirs[context] / "overlay.elf").open("rb") as stream:
                overlay_reference[context] = retail.sections(ELFFile(stream))
        return overlay_reference[context]

    def resolve_overlay(name, context):
        # Canonical common-function names encode their FIRST overlay. Locate
        # that function's grouping key in the CURRENT overlay independently
        # of the caller's expected instructions. Reject ambiguous identities.
        source_context = name.split("_")[1]
        address, size = sizes[name]
        source_code = retail.read(read_overlay(source_context), address, size)
        if source_code is None:
            raise ValueError("canonical overlay function is outside its image: " + name)
        key = digest(source_code)
        if context not in overlay_indices:
            index = {}
            for address, _, section_name, data in read_overlay(context):
                if section_name == ".text":
                    for target, _, code in overlay_functions(data, address):
                        index.setdefault(digest(code), []).append(target)
            overlay_indices[context] = index
        candidates = overlay_indices[context].get(key, [])
        if len(candidates) != 1:
            raise ValueError("canonical overlay symbol has %d local candidates: %s" % (len(candidates), name))
        return candidates[0]
    paths = [ROOT / "build/obj" / args.object] if args.object else sorted((ROOT / "build/obj").glob("*.o"))
    for path in paths:
        overlay = re.match(r"overlays_(L\d\d)_", path.stem)
        if overlay and not args.overlays:
            continue
        try:
            context = overlay[1] if overlay else "resident"
            current_reference = reference
            if overlay:
                current_reference = read_overlay(context)
            records.extend(link_object(path, sizes, current_reference, gp, output, context, resolve_overlay))
        except (OSError, ValueError, KeyError) as error:
            failures.append({"object": path.name, "error": str(error)})
    exact = sum(record["exact"] for record in records)
    report = {"functions": records, "failures": failures, "gp": gp}
    report_path = output / ("report-single.json" if args.object else "report.json")
    report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("%d exact linked functions, %d mismatches, %d object failures" % (exact, len(records) - exact, len(failures)))
    for failure in failures:
        print("%s: %s" % (failure["object"], failure["error"]))
    return 1 if failures or exact != len(records) or not records else 0


if __name__ == "__main__":
    raise SystemExit(main())
