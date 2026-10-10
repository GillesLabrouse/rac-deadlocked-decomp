#!/usr/bin/env python3
"""Package an exact hybrid rebuild without moving sector-addressed disc data.

Matching C plus retail fallback reconstructs an unchanged section stream. Reuse
the verified retail loader's compressed representation of that exact stream:
recompression is unnecessary and can overflow the boot file's sector extent.
The output ISO is byte-identical to the supplied disc. This is a packaging and
build-provenance milestone, not a new port or a fully decompiled game.
"""
import argparse
import hashlib
import json
from pathlib import Path

from extract_baserom import extract
from split_image import build_elf, parse
from unpack_wad import decompress, find_wad
from record_build import validate

ROOT = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("iso", type=Path, help="the original NTSC-U v1.00 disc image")
    args = parser.parse_args()
    original = args.iso.resolve()
    output = ROOT / "build/rebuilt"
    target = output / "deadlocked.iso"
    if original == target.resolve():
        raise ValueError("input ISO must not be the output ISO")
    loader = extract(original)
    image = (output / "SCUS_974.65.unpacked").read_bytes()
    decoded = decompress(loader, find_wad(loader))
    if decoded != image:
        raise ValueError("rebuilt section stream differs: cannot reuse the original packed representation")
    sections = parse(image)
    if (output / "SCUS_974.65.elf").read_bytes() != build_elf(sections, sections[0]["entry"]):
        raise ValueError("rebuilt ELF and section stream disagree")
    resident = json.loads((output / "report.json").read_text())
    link_report = (ROOT / "build/linked/report.json").read_bytes()
    if hashlib.sha256(link_report).hexdigest() != resident["link_report_sha256"]:
        raise ValueError("reconstruction is stale: rerun tools/rebuild_image.py")
    for record in json.loads(link_report)["functions"]:
        inputs = validate(ROOT / record["object"])
        if inputs["object_sha256"] != record["object_sha256"]:
            raise ValueError("link report is stale: rerun tools/link_objects.py")
    if not resident["linked_c_functions"] or not resident["section_stream_matches_retail"]:
        raise ValueError("reconstruct exact linked C functions first")
    temporary = output / "deadlocked.iso.partial"
    digest = hashlib.sha256()
    with original.open("rb") as source, temporary.open("wb") as destination:
        while chunk := source.read(8 * 1024 * 1024):
            destination.write(chunk)
            digest.update(chunk)
    output_digest = hashlib.sha256()
    with temporary.open("rb") as stream:
        while chunk := stream.read(8 * 1024 * 1024):
            output_digest.update(chunk)
    if output_digest.digest() != digest.digest() or extract(temporary) != loader:
        raise ValueError("packaged ISO verification failed")
    temporary.replace(target)
    summary = {"kind": "exact-hybrid-iso", "iso": target.relative_to(ROOT).as_posix(),
               "size": target.stat().st_size, "sha256": digest.hexdigest(),
               "byte_identical_to_source_disc": True,
               "loader_sections_match_reconstruction": True,
               "linked_resident_c_functions": resident["linked_c_functions"],
               "runtime_tested": False,
               "packing": "Reuse the verified original compressed representation of the exact reconstructed section stream",
               "fallback": "All undecompiled code, libraries, overlays and game assets from the user's disc"}
    (output / "iso-report.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    print("Verified test ISO: " + str(target))
    print("Byte-identical to the source disc; %d resident C functions validated after linking" % resident["linked_c_functions"])
    print("Runtime boot and campaign play remain unverified")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, KeyError) as error:
        raise SystemExit(str(error))
