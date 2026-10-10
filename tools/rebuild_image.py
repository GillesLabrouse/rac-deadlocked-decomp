#!/usr/bin/env python3
"""Reconstruct the resident ELF with exact linked C and local retail fallback.

Run tools/build.sh and tools/link_objects.py first. This is a hybrid build,
not a fully decompiled game. All data and undecompiled code come from the
user's verified baserom. A compiled function is inserted only if its complete
linked bytes equal retail. Nothing produced by this tool belongs in Git.
"""
import hashlib
import json
from pathlib import Path

from extract_baserom import EXPECTED_SHA1
from split_image import build_elf, parse
from unpack_wad import decompress, find_wad
from record_build import validate

ROOT = Path(__file__).resolve().parent.parent


def reconstruct(image, records, read_binary):
    sections = parse(image)
    output = bytearray(image)
    inserted, ranges = [], []
    for record in records:
        if record.get("context", "resident") != "resident":
            continue
        if not record["exact"]:
            raise ValueError("unmatched linked function: " + record["name"])
        address, size = record["address"], record["size"]
        if any(address < end and start < address + size for start, end in ranges):
            raise ValueError("overlapping linked functions")
        code = read_binary(record["binary"])
        if len(code) != size:
            raise ValueError("linked binary size changed: " + record["name"])
        for section in sections:
            delta = address - section["dest"]
            if section["name"] not in ("core.text", ".text", "net.text"):
                continue
            if 0 <= delta and delta + size <= section["size"]:
                if code != section["data"][delta:delta + size]:
                    raise ValueError("linked bytes differ from retail: " + record["name"])
                offset = section["file_off"] + delta
                output[offset:offset + size] = code
                ranges.append((address, address + size))
                inserted.append(record["name"])
                break
        else:
            raise ValueError("function is outside resident executable code: " + record["name"])
    if not inserted:
        raise ValueError("no exact linked resident C functions to reconstruct")
    if output != image:
        raise ValueError("reconstructed section stream differs from retail")
    return bytes(output), inserted


def main():
    loader = (ROOT / "baserom/SCUS_974.65").read_bytes()
    if hashlib.sha1(loader).hexdigest() != EXPECTED_SHA1:
        raise ValueError("unsupported baserom SHA-1")
    image = decompress(loader, find_wad(loader))
    report = json.loads((ROOT / "build/linked/report.json").read_text())
    if report["failures"]:
        raise ValueError("link report has failures: resolve them before rebuilding")
    for record in report["functions"]:
        inputs = validate(ROOT / record["object"])
        if inputs["object_sha256"] != record["object_sha256"]:
            raise ValueError("link report is stale: rerun tools/link_objects.py")

    def read_binary(name):
        path = (ROOT / name).resolve()
        if not path.is_relative_to(ROOT / "build/linked"):
            raise ValueError("linked binary must be under build/linked")
        return path.read_bytes()

    reconstructed, inserted = reconstruct(image, report["functions"], read_binary)
    sections = parse(reconstructed)
    output = ROOT / "build/rebuilt"
    output.mkdir(parents=True, exist_ok=True)
    elf = build_elf(sections, sections[0]["entry"])
    (output / "SCUS_974.65.elf").write_bytes(elf)
    (output / "SCUS_974.65.unpacked").write_bytes(reconstructed)
    summary = {"kind": "hybrid-resident", "entry": sections[0]["entry"],
               "link_report_sha256": hashlib.sha256((ROOT / "build/linked/report.json").read_bytes()).hexdigest(),
               "linked_c_functions": len(inserted), "linked_c_bytes": sum(
                   r["size"] for r in report["functions"] if r.get("context", "resident") == "resident"),
               "section_stream_matches_retail": True,
               "elf_sha1": hashlib.sha1(elf).hexdigest(),
               "fallback": "User-supplied retail data and all other functions, including overlays and libraries",
               "runtime_tested": False, "functions": inserted}
    (output / "report.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    print("Rebuilt resident ELF: %d exact linked C functions; section stream equals retail" % len(inserted))
    print("Output: " + str(output / "SCUS_974.65.elf"))
    print("Hybrid build uses retail fallback; emulator boot and campaign play remain unverified")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, KeyError) as error:
        raise SystemExit(str(error))
