#!/usr/bin/env python3
"""Link one explicitly addressed overlay draft without changing accepted reports.

Derive splitter extents from the supplied local retail overlay. Only actual
local-address names are supported; normalized cross-overlay identities must be
resolved independently before using this diagnostic. All relocation fields are
resolved by the real linker and compared without masking.
Use --span for a separately verified complete range when early returns make
the retail splitter truncate a function. This never changes accepted metadata.
Use --entry only for an independently established pointer-only entry omitted
by the splitter; supplying an entry does not verify that boundary automatically.
"""
import argparse
import io
import json
import os
import re
import struct
from pathlib import Path

from elftools.elf.elffile import ELFFile
import link_objects
import retail
from split_overlays import functions as overlay_functions


def draft_sizes(reference, context):
    if not re.fullmatch(r"L\d{2}", context):
        raise ValueError("context must be an explicit level such as L01")
    text = [section for section in reference if section[2] == ".text"]
    if len(text) != 1:
        raise ValueError("overlay must have exactly one text section")
    address, _, _, data = text[0]
    return {"func_%s_%08X" % (context, target): (target, size)
            for target, size, _ in overlay_functions(data, address)}


def check_local_symbols(elf, sizes, context):
    names = link_objects.functions(elf)
    if not names:
        raise ValueError("draft object has no game functions")
    for name in names:
        if name not in sizes:
            raise ValueError("function is not a known local overlay boundary: " + name)
    for symbol in elf.get_section_by_name(".symtab").iter_symbols():
        match = re.match(r"(?:func|D)_(L\d\d)_", symbol.name)
        if match and match[1] != context:
            raise ValueError("draft needs an independently resolved local symbol: " + symbol.name)


def verified_spans(sizes, reference, context, spans, new_entries=False):
    result = dict(sizes)
    seen = set()
    for span in spans:
        if not re.fullmatch(r"(?:0x)?[0-9A-Fa-f]+:(?:0x)?[0-9A-Fa-f]+", span):
            raise ValueError("span must be HEX_ADDRESS:HEX_SIZE")
        address, size = (int(value, 16) for value in span.split(":"))
        name = "func_%s_%08X" % (context, address)
        if address in seen or (name in sizes if new_entries else name not in sizes):
            requirement = "unlisted" if new_entries else "known"
            raise ValueError("span needs a unique " + requirement + " entry: " + span)
        if size < 8 or size % 4 or address % 4:
            raise ValueError("span needs a positive instruction-aligned extent: " + span)
        if not any(section_name == ".text" and start <= address and
                   address + size <= start + len(data)
                   for start, _, section_name, data in reference):
            raise ValueError("span extends outside overlay text: " + span)
        result[name] = address, size
        seen.add(address)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("context", help="actual overlay context, e.g. L01")
    parser.add_argument("object", type=Path, help="one object below build/drafts")
    parser.add_argument("--span", action="append", default=[], metavar="HEX_ADDRESS:HEX_SIZE",
                        help="independently verified complete range; never updates accepted metadata")
    parser.add_argument("--entry", action="append", default=[], metavar="HEX_ADDRESS:HEX_SIZE",
                        help="independently verified entry omitted by the splitter; draft-only")
    args = parser.parse_args()
    root = retail.ROOT
    path = args.object.resolve()
    output = root / "build/drafts/linked" / path.stem
    local_input = False
    try:
        path.relative_to(root / "build/drafts")
        local_input = True
        directories = {}
        for line in (root / "config/overlays.tsv").read_text().splitlines():
            if line and not line.startswith("#"):
                level, directory, _, _ = line.split("\t")
                directories[level] = directory
        overlay_root = Path(os.environ.get("OVERLAYS") or root / "private/overlays")
        with (overlay_root / "levels" / directories[args.context] / "overlay.elf").open("rb") as stream:
            reference = retail.sections(ELFFile(stream))
        sizes = verified_spans(draft_sizes(reference, args.context), reference,
                               args.context, args.span)
        sizes = verified_spans(sizes, reference, args.context, args.entry, new_entries=True)
        check_local_symbols(ELFFile(io.BytesIO(path.read_bytes())), sizes, args.context)
        gp = struct.unpack_from("<I", (root / "baserom/sections/00_reginfo.bin").read_bytes(), 20)[0]
        output.mkdir(parents=True, exist_ok=True)
        records = link_objects.link_object(path, sizes, reference, gp, output, context=args.context)
    except (OSError, ValueError, KeyError) as error:
        if local_input:
            output.mkdir(parents=True, exist_ok=True)
            (output / "report.json").write_text(json.dumps({
                "kind": "overlay-draft", "status": "failed", "context": args.context,
                "functions": [], "failure": str(error)
            }, indent=2) + "\n", encoding="utf-8")
        print("Draft check failed: %s" % error)
        return 1
    report = {"kind": "overlay-draft", "status": "compared", "context": args.context,
              "verified_spans": args.span, "verified_entries": args.entry,
              "functions": records, "gp": gp}
    report_path = output / "report.json"
    report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    for record in records:
        print("%s: %s (%d retail bytes)" %
              (record["name"], "exact" if record["exact"] else "NONMATCHING", record["size"]))
    print("Draft-only report: " + report_path.relative_to(root).as_posix())
    return 0 if records and all(record["exact"] for record in records) else 1


if __name__ == "__main__":
    raise SystemExit(main())
