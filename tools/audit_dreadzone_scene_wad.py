#!/usr/bin/env python3
"""Audit the partially decoded Dreadzone LevelSceneWad moby_load streams.

The 16-byte leading records and their first field are reported as raw index
entries; their namespace is not fully documented. This tool does not assign
those entries to Moby classes or infer geometry from texture data.
"""

import argparse
import json
import struct
from pathlib import Path

from unpack_wad import decompress

SECTOR_SIZE = 2048
SCENE_COUNT = 30
SCENE_HEADER_SIZE = 0x38 + 69 * 4
SCENE_HEADER_OFFSET = 8
SCENE_MOBY_LOAD_OFFSET = 0x30
SCENE_WAD_HEADER_SIZE = 8 + SCENE_COUNT * SCENE_HEADER_SIZE
INDEX_ENTRY = struct.Struct("<4I")
PIF_HEADER = struct.Struct("<8I")
UNRESOLVED_CLASS_IDS = {
    3107, 3168, 3174, 3305, 3950, 8226, 8246, 8247, 8321,
    8355, 8520, 9207, 9213, 9271, 9340, 9453, 9918,
}


def parse_index(decoded, scene_index):
    """Read bounded leading 16-byte records through their all-ones sentinel."""
    entries = []
    pos = 0
    while pos + INDEX_ENTRY.size <= len(decoded):
        record_id, start, end, reserved = INDEX_ENTRY.unpack_from(decoded, pos)
        if record_id == 0xFFFFFFFF:
            return entries
        if reserved != 0 or not (0 < start < end <= len(decoded)):
            raise ValueError(
                f"Scene {scene_index} has an invalid index record at 0x{pos:x}"
            )
        entries.append({
            "record_id": record_id,
            "start": start,
            "end": end,
        })
        pos += INDEX_ENTRY.size
    raise ValueError(f"Scene {scene_index} has no complete index sentinel")


def parse_pif_headers(decoded, scene_index):
    """List PIF2 headers without interpreting their texture payloads."""
    result = []
    pos = 0
    while True:
        pos = decoded.find(b"2FIP", pos)
        if pos < 0:
            break
        if pos + PIF_HEADER.size > len(decoded):
            raise ValueError(f"Scene {scene_index} has a truncated PIF2 header")
        magic, file_size, width, height, fmt, clut_fmt, clut_order, mip_levels = (
            PIF_HEADER.unpack_from(decoded, pos)
        )
        if magic != 0x50494632 or not width or not height:
            raise ValueError(f"Scene {scene_index} has an invalid PIF2 header")
        result.append({
            "offset": pos,
            "file_size_field": file_size,
            "width": width,
            "height": height,
            "format": fmt,
            "clut_format": clut_fmt,
            "clut_order": clut_order,
            "mip_levels": mip_levels,
        })
        pos += 4
    return result


def scan_exact_core_headers(core_root, scene_streams):
    """Find exact 64-byte prefixes of exported global cores in decoded streams."""
    core_root = Path(core_root)
    core_files = sorted(core_root.rglob("core.bin")) if core_root.is_dir() else []
    matches = []
    scanned = 0
    for core_path in core_files:
        raw = core_path.read_bytes()
        if len(raw) < 64:
            continue
        scanned += 1
        signature = raw[:64]
        for scene_index, decoded in scene_streams:
            offset = decoded.find(signature)
            if offset >= 0:
                matches.append({
                    "core_path": str(core_path),
                    "scene_index": scene_index,
                    "offset": offset,
                })
    return {
        "global_core_files_scanned": scanned,
        "exact_first_64_byte_core_header_matches": matches,
    }


def audit_scene_wad(path):
    """Return a bounded inventory of Dreadzone's LevelSceneWad streams."""
    path = Path(path)
    data = path.read_bytes()
    if len(data) < SCENE_WAD_HEADER_SIZE:
        raise ValueError("Scene WAD is shorter than its declared header")
    header_size, _sector = struct.unpack_from("<II", data)
    if header_size != SCENE_WAD_HEADER_SIZE:
        raise ValueError(
            f"Unexpected scene WAD header size {header_size}; "
            f"expected {SCENE_WAD_HEADER_SIZE}"
        )

    scenes = []
    decoded_streams = []
    total_index_records = 0
    total_pif_headers = 0
    for scene_index in range(SCENE_COUNT):
        record_offset = SCENE_HEADER_OFFSET + scene_index * SCENE_HEADER_SIZE
        wad_sector, wad_sector_count = struct.unpack_from(
            "<II", data, record_offset + SCENE_MOBY_LOAD_OFFSET
        )
        if not wad_sector and not wad_sector_count:
            continue
        if not wad_sector or not wad_sector_count:
            raise ValueError(f"Scene {scene_index} has a partial moby_load range")

        wad_offset = wad_sector * SECTOR_SIZE
        wad_capacity = wad_sector_count * SECTOR_SIZE
        if wad_offset + wad_capacity > len(data):
            raise ValueError(f"Scene {scene_index} moby_load range exceeds the WAD")
        if data[wad_offset:wad_offset + 3] != b"WAD":
            raise ValueError(f"Scene {scene_index} moby_load does not start with WAD")
        compressed_size = struct.unpack_from("<I", data, wad_offset + 3)[0]
        if not 16 <= compressed_size <= wad_capacity:
            raise ValueError(f"Scene {scene_index} has an invalid compressed WAD size")

        decoded = decompress(data, wad_offset)
        decoded_streams.append((scene_index, decoded))
        entries = parse_index(decoded, scene_index)
        pifs = parse_pif_headers(decoded, scene_index)
        for entry in entries:
            entry["unresolved_id_match"] = (
                entry["record_id"] in UNRESOLVED_CLASS_IDS
            )
        total_index_records += len(entries)
        total_pif_headers += len(pifs)
        scenes.append({
            "scene_index": scene_index,
            "moby_load_sector": wad_sector,
            "moby_load_sector_count": wad_sector_count,
            "compressed_bytes": compressed_size,
            "decoded_bytes": len(decoded),
            "index_records": entries,
            "pif2_headers": pifs,
        })

    core_scan = scan_exact_core_headers(
        path.parent.parent.parent.parent / "moby_classes",
        decoded_streams,
    )
    return {
        "kind": "dreadzone-scene-wad-partial-audit",
        "source": str(path.resolve()),
        "header_size": header_size,
        "scene_count": SCENE_COUNT,
        "nonempty_moby_load_streams": len(scenes),
        "index_record_count": total_index_records,
        "pif2_header_count": total_pif_headers,
        "global_moby_core_scan": core_scan,
        "unresolved_class_ids": sorted(UNRESOLVED_CLASS_IDS),
        "scenes": scenes,
    }


def main():
    root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "scene_wad",
        nargs="?",
        type=Path,
        default=root / "private/overlays/levels/campaign/1_dreadzone_station/scene.wad",
    )
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    report = audit_scene_wad(args.scene_wad)
    output = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(output, encoding="utf-8")
    else:
        print(output, end="")


if __name__ == "__main__":
    main()
