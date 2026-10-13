#!/usr/bin/env python3
"""Audit the partially decoded Dreadzone LevelSceneWad moby_load streams.

The 16-byte leading records and their first field are reported as raw index
entries; their namespace is not fully documented. This tool does not assign
those entries to Moby classes or infer geometry from texture data.
"""

import argparse
import bisect
import binascii
import json
import math
import struct
import zlib
from pathlib import Path

from unpack_wad import decompress

SECTOR_SIZE = 2048
SCENE_COUNT = 30
SCENE_HEADER_SIZE = 0x38 + 69 * 4
SCENE_HEADER_OFFSET = 8
SCENE_MOBY_LOAD_OFFSET = 0x30
SCENE_CHUNK_TABLE_OFFSET = 0x38
SCENE_CHUNK_COUNT = 69
CHUNK_RESOURCE_TABLE_OFFSET = 0x50
SCENE_WAD_HEADER_SIZE = 8 + SCENE_COUNT * SCENE_HEADER_SIZE
INDEX_ENTRY = struct.Struct("<4I")
PIF_HEADER = struct.Struct("<8I")
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
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


def decode_pif2_rgba(decoded, offset):
    """Decode the base mip of a supported indexed PIF2 texture to RGBA8."""
    if offset < 0 or offset + PIF_HEADER.size > len(decoded):
        raise ValueError("PIF2 header is outside the decoded stream")
    magic, _file_size, width, height, fmt, _clut_fmt, _clut_order, mip_levels = (
        PIF_HEADER.unpack_from(decoded, offset)
    )
    if magic != 0x50494632 or not 0 < width <= 2048 or not 0 < height <= 2048:
        raise ValueError("PIF2 header has invalid magic or dimensions")
    if fmt == 0x13:
        palette_count = 256
        pixel_bytes = width * height
    elif fmt == 0x94:
        palette_count = 16
        pixel_bytes = (width * height + 1) // 2
    else:
        raise ValueError(f"Unsupported PIF2 pixel format 0x{fmt:x}")
    palette_offset = offset + PIF_HEADER.size
    pixels_offset = palette_offset + palette_count * 4
    if pixels_offset + pixel_bytes > len(decoded):
        raise ValueError("PIF2 palette or base mip exceeds the decoded stream")

    palette = [
        decoded[palette_offset + i * 4:palette_offset + (i + 1) * 4]
        for i in range(palette_count)
    ]
    if fmt == 0x13:
        original_palette = palette
        palette = [
            original_palette[map_palette_index(i)]
            for i in range(palette_count)
        ]
    rgba = bytearray(width * height * 4)
    pixels = decoded[pixels_offset:pixels_offset + pixel_bytes]
    for i in range(width * height):
        if fmt == 0x13:
            index = pixels[i]
        else:
            packed = pixels[i // 2]
            index = packed >> 4 if i % 2 == 0 else packed & 0x0F
        r, g, b, alpha = palette[index]
        alpha = alpha * 2 if alpha < 0x80 else 0xFF
        rgba[i * 4:i * 4 + 4] = bytes((r, g, b, alpha))
    return width, height, bytes(rgba), mip_levels


def map_palette_index(index):
    """Apply the PSMT8 CLUT ordering used by Deadlocked PIF textures."""
    return index ^ 0x18 if (((index & 16) >> 1) != (index & 8)) else index


def encode_rgba_png(width, height, rgba):
    """Encode an unfiltered, non-interlaced 8-bit RGBA PNG."""
    if width <= 0 or height <= 0 or len(rgba) != width * height * 4:
        raise ValueError("RGBA image dimensions do not match its pixel data")

    def chunk(kind, data):
        body = kind + data
        return struct.pack(">I", len(data)) + body + struct.pack(
            ">I", binascii.crc32(body) & 0xFFFFFFFF
        )

    rows = b"".join(
        b"\x00" + rgba[y * width * 4:(y + 1) * width * 4]
        for y in range(height)
    )
    header = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    return (
        PNG_SIGNATURE
        + chunk(b"IHDR", header)
        + chunk(b"IDAT", zlib.compress(rows, 9))
        + chunk(b"IEND", b"")
    )


def parse_chunk_resource_entries(decoded, scene_index, chunk_index):
    """Read candidate resource descriptors from a decoded scene chunk.

    The entry IDs resemble Moby class IDs in several chunks, but their namespace
    and payload format are not fully identified. Preserve raw offsets and do not
    interpret payload bytes as geometry.
    """
    if len(decoded) < CHUNK_RESOURCE_TABLE_OFFSET + 16:
        raise ValueError(
            f"Scene {scene_index} chunk {chunk_index} is shorter than its header"
        )
    offsets = struct.unpack_from("<4I", decoded, CHUNK_RESOURCE_TABLE_OFFSET)
    entries = []
    for descriptor_offset in offsets:
        if descriptor_offset == 0:
            continue
        if descriptor_offset < CHUNK_RESOURCE_TABLE_OFFSET + 16:
            raise ValueError(
                f"Scene {scene_index} chunk {chunk_index} has an invalid "
                f"resource descriptor offset 0x{descriptor_offset:x}"
            )
        if descriptor_offset + 8 > len(decoded):
            raise ValueError(
                f"Scene {scene_index} chunk {chunk_index} has an out-of-range "
                f"resource descriptor offset 0x{descriptor_offset:x}"
            )
        resource_id, payload_offset = struct.unpack_from(
            "<2I", decoded, descriptor_offset
        )
        if not resource_id or not (descriptor_offset + 8 <= payload_offset < len(decoded)):
            raise ValueError(
                f"Scene {scene_index} chunk {chunk_index} has an invalid "
                f"resource entry at 0x{descriptor_offset:x}"
            )
        entries.append({
            "descriptor_offset": descriptor_offset,
            "resource_id": resource_id,
            "payload_offset": payload_offset,
        })
    return entries


def scan_nested_resource_candidates(decoded, scene_index, chunk_index, known_offsets):
    """Find aligned zero-reserved records that may reference nested resources.

    This is a structural heuristic for follow-up analysis, not an asset parser:
    candidate IDs and data pointers are reported without assigning a namespace.
    """
    candidates = []
    for descriptor_offset in range(CHUNK_RESOURCE_TABLE_OFFSET + 16, len(decoded) - 23, 16):
        if descriptor_offset in known_offsets:
            continue
        resource_id, data_offset, reserved_a, reserved_b = struct.unpack_from(
            "<4I", decoded, descriptor_offset
        )
        if (
            0 < resource_id < 0x10000
            and descriptor_offset + 24 <= data_offset < len(decoded)
            and reserved_a == 0
            and reserved_b == 0
        ):
            candidates.append({
                "descriptor_offset": descriptor_offset,
                "resource_id": resource_id,
                "data_offset": data_offset,
            })
    return candidates


def summarize_candidate_payload(decoded, candidate):
    """Summarize a candidate's pointed-to 16-byte rows without decoding them."""
    data_offset = candidate["data_offset"]
    row_count = (len(decoded) - data_offset) // 16
    if row_count <= 0:
        return None
    rows = [decoded[data_offset + i * 16:data_offset + (i + 1) * 16]
            for i in range(row_count)]
    first = rows[0]
    repeated_prefix_rows = 1
    while (repeated_prefix_rows < row_count
           and rows[repeated_prefix_rows] == first):
        repeated_prefix_rows += 1
    return {
        "row_size": 16,
        "available_row_count": row_count,
        "unique_row_count": len(set(rows)),
        "repeated_first_row_count": repeated_prefix_rows,
        "rows_after_repeated_prefix": row_count - repeated_prefix_rows,
        "first_row_hex": first.hex(),
    }


def annotate_next_candidate_descriptors(candidates):
    """Record the nearest candidate descriptor following each data pointer."""
    descriptor_offsets = sorted(
        candidate["descriptor_offset"] for candidate in candidates
    )
    for candidate in candidates:
        position = bisect.bisect_left(
            descriptor_offsets, candidate["data_offset"]
        )
        if position == len(descriptor_offsets):
            candidate["next_candidate_descriptor_offset"] = None
            candidate["rows_before_next_candidate_descriptor"] = None
            continue
        next_offset = descriptor_offsets[position]
        candidate["next_candidate_descriptor_offset"] = next_offset
        gap = next_offset - candidate["data_offset"]
        candidate["rows_before_next_candidate_descriptor"] = (
            gap // 16 if gap % 16 == 0 else None
        )


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


def scan_exact_core_headers_in_chunks(core_roots, chunk_streams):
    """Search decoded scene chunks for exact prefixes of exported Moby cores."""
    core_files = set()
    for root in core_roots:
        root = Path(root)
        if root.is_dir():
            core_files.update(path.resolve() for path in root.rglob("core.bin"))
    matches = []
    scanned = 0
    for core_path in sorted(core_files):
        raw = core_path.read_bytes()
        if len(raw) < 64:
            continue
        scanned += 1
        signature = raw[:64]
        for scene_index, chunk_index, decoded in chunk_streams:
            offset = decoded.find(signature)
            if offset >= 0:
                matches.append({
                    "core_path": str(core_path),
                    "scene_index": scene_index,
                    "chunk_index": chunk_index,
                    "offset": offset,
                })
    return {
        "global_and_mission_core_files_scanned": scanned,
        "exact_first_64_byte_core_header_matches": matches,
    }


def scene_sector_starts(data):
    """Collect the sector starts used by Dreadzone's scene WAD records."""
    starts = set()
    sector_offsets = (
        0x00, 0x04, 0x08, 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30,
    )
    for scene_index in range(SCENE_COUNT):
        record_offset = SCENE_HEADER_OFFSET + scene_index * SCENE_HEADER_SIZE
        for field_offset in sector_offsets:
            sector = struct.unpack_from("<I", data, record_offset + field_offset)[0]
            if sector:
                starts.add(sector)
        chunks = struct.unpack_from(
            f"<{SCENE_CHUNK_COUNT}I", data,
            record_offset + SCENE_CHUNK_TABLE_OFFSET,
        )
        starts.update(sector for sector in chunks if sector)
    starts.add((len(data) + SECTOR_SIZE - 1) // SECTOR_SIZE)
    return sorted(starts)


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
    chunk_references = 0
    empty_chunk_references = 0
    decoded_chunk_count = 0
    chunk_resource_ids = {}
    chunk_inventory = []
    decoded_chunk_streams = []
    nested_resource_ids = {}
    nested_resource_inventory = []
    sector_starts = scene_sector_starts(data)
    for scene_index in range(SCENE_COUNT):
        record_offset = SCENE_HEADER_OFFSET + scene_index * SCENE_HEADER_SIZE
        chunk_table_offset = record_offset + SCENE_CHUNK_TABLE_OFFSET
        chunk_sectors = struct.unpack_from(
            f"<{SCENE_CHUNK_COUNT}I", data, chunk_table_offset
        )
        for chunk_index, chunk_sector in enumerate(chunk_sectors):
            if not chunk_sector:
                continue
            chunk_references += 1
            chunk_offset = chunk_sector * SECTOR_SIZE
            if chunk_offset + 16 > len(data):
                raise ValueError(
                    f"Scene {scene_index} chunk {chunk_index} exceeds the WAD"
                )
            if data[chunk_offset:chunk_offset + 3] != b"WAD":
                if data[chunk_offset:chunk_offset + SECTOR_SIZE] != bytes(SECTOR_SIZE):
                    raise ValueError(
                        f"Scene {scene_index} chunk {chunk_index} is neither WAD nor empty"
                    )
                empty_chunk_references += 1
                continue
            compressed_bytes = struct.unpack_from("<I", data, chunk_offset + 3)[0]
            next_index = bisect.bisect_right(sector_starts, chunk_sector)
            if next_index >= len(sector_starts):
                raise ValueError(
                    f"Scene {scene_index} chunk {chunk_index} has no ending sector"
                )
            next_sector = sector_starts[next_index]
            sector_capacity = (next_sector - chunk_sector) * SECTOR_SIZE
            if not 16 <= compressed_bytes <= sector_capacity:
                raise ValueError(
                    f"Scene {scene_index} chunk {chunk_index} exceeds its sector range"
                )
            decoded_chunk = decompress(data, chunk_offset)
            decoded_chunk_streams.append((scene_index, chunk_index, decoded_chunk))
            resource_entries = parse_chunk_resource_entries(
                decoded_chunk, scene_index, chunk_index
            )
            nested_candidates = scan_nested_resource_candidates(
                decoded_chunk,
                scene_index,
                chunk_index,
                {entry["descriptor_offset"] for entry in resource_entries},
            )
            annotate_next_candidate_descriptors(nested_candidates)
            decoded_chunk_count += 1
            for entry in resource_entries:
                resource_id = entry["resource_id"]
                chunk_resource_ids[resource_id] = (
                    chunk_resource_ids.get(resource_id, 0) + 1
                )
                entry["unresolved_class_id_match"] = (
                    resource_id in UNRESOLVED_CLASS_IDS
                )
            for candidate in nested_candidates:
                resource_id = candidate["resource_id"]
                nested_resource_ids[resource_id] = (
                    nested_resource_ids.get(resource_id, 0) + 1
                )
                candidate["unresolved_class_id_match"] = (
                    resource_id in UNRESOLVED_CLASS_IDS
                )
                candidate["scene_index"] = scene_index
                candidate["chunk_index"] = chunk_index
                candidate["payload_layout"] = summarize_candidate_payload(
                    decoded_chunk, candidate
                )
                nested_resource_inventory.append(candidate)
            chunk_inventory.append({
                "scene_index": scene_index,
                "chunk_index": chunk_index,
                "sector": chunk_sector,
                "compressed_bytes": compressed_bytes,
                "decoded_bytes": len(decoded_chunk),
                "resource_entries": resource_entries,
            })

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
    chunk_core_scan = scan_exact_core_headers_in_chunks(
        [path.parent.parent.parent.parent / "moby_classes", path.parent],
        decoded_chunk_streams,
    )
    return {
        "kind": "dreadzone-scene-wad-partial-audit",
        "source": str(path.resolve()),
        "header_size": header_size,
        "scene_count": SCENE_COUNT,
        "nonempty_moby_load_streams": len(scenes),
        "index_record_count": total_index_records,
        "pif2_header_count": total_pif_headers,
        "scene_chunk_inventory": {
            "chunk_reference_count": chunk_references,
            "empty_chunk_reference_count": empty_chunk_references,
            "decoded_chunk_count": decoded_chunk_count,
            "candidate_resource_id_counts": dict(sorted(chunk_resource_ids.items())),
            "unresolved_class_id_matches": {
                str(resource_id): count
                for resource_id, count in sorted(chunk_resource_ids.items())
                if resource_id in UNRESOLVED_CLASS_IDS
            },
            "nested_candidate_count": len(nested_resource_inventory),
            "nested_candidate_id_counts": dict(sorted(nested_resource_ids.items())),
            "nested_unresolved_class_id_matches": {
                str(resource_id): count
                for resource_id, count in sorted(nested_resource_ids.items())
                if resource_id in UNRESOLVED_CLASS_IDS
            },
            "nested_candidates": nested_resource_inventory,
            "chunks": chunk_inventory,
        },
        "global_moby_core_scan": core_scan,
        "scene_chunk_moby_core_scan": chunk_core_scan,
        "unresolved_class_ids": sorted(UNRESOLVED_CLASS_IDS),
        "scenes": scenes,
    }


def extract_scene_pif_textures(path, report, output_directory):
    """Export supported scene moby_load PIF base mips as PNG diagnostic assets."""
    path = Path(path)
    data = path.read_bytes()
    output_directory = Path(output_directory)
    output_directory.mkdir(parents=True, exist_ok=True)
    output_paths = []
    for scene in report["scenes"]:
        decoded = decompress(data, scene["moby_load_sector"] * SECTOR_SIZE)
        for pif in scene["pif2_headers"]:
            width, height, rgba, _mip_levels = decode_pif2_rgba(
                decoded, pif["offset"]
            )
            output = output_directory / (
                f"scene-{scene['scene_index']:02d}-pif-{pif['offset']:08x}.png"
            )
            output.write_bytes(encode_rgba_png(width, height, rgba))
            output_paths.append(output)
    return output_paths


def correlate_resource_position_rows(path, report, mission_root, tolerance=0.01):
    """Compare bounded 16-byte resource rows with extracted mission Moby positions.

    This reports exact coordinate matches as evidence only. Resource IDs and
    unmatched row semantics remain provisional.
    """
    from asset_text import read as read_asset

    mission_root = Path(mission_root)
    placements = {}
    for instances_path in sorted(mission_root.glob("*/mission.instances")):
        mission_id = instances_path.parent.name
        for node in read_asset(instances_path):
            if node.kind != "Moby":
                continue
            class_id = node.attributes.get("class")
            position = node.attributes.get("pos")
            if (type(class_id) is not int or not isinstance(position, list)
                    or len(position) != 3
                    or not all(type(value) in (int, float) for value in position)):
                continue
            placements.setdefault(class_id, []).append((
                mission_id, node.name, tuple(float(value) for value in position)
            ))

    data = Path(path).read_bytes()
    nested_by_chunk = {}
    for candidate in report["scene_chunk_inventory"]["nested_candidates"]:
        nested_by_chunk.setdefault(
            (candidate["scene_index"], candidate["chunk_index"]), []
        ).append(candidate["descriptor_offset"])

    totals = {}
    for chunk in report["scene_chunk_inventory"]["chunks"]:
        scene_index, chunk_index = chunk["scene_index"], chunk["chunk_index"]
        relevant = [entry for entry in chunk["resource_entries"]
                    if entry["resource_id"] in placements]
        if not relevant:
            continue
        decoded = decompress(data, chunk["sector"] * SECTOR_SIZE)
        descriptor_offsets = sorted(
            [entry["descriptor_offset"] for entry in chunk["resource_entries"]]
            + nested_by_chunk.get((scene_index, chunk_index), [])
        )
        for entry in relevant:
            resource_id = entry["resource_id"]
            start = entry["payload_offset"]
            end = next((offset for offset in descriptor_offsets if offset > start),
                       len(decoded))
            row_count = min(49, max(0, (end - start) // 16))
            summary = totals.setdefault(resource_id, {
                "resource_entry_count": 0,
                "compared_position_row_count": 0,
                "matching_position_row_count": 0,
                "matching_positions": {},
                "matching_placements": set(),
            })
            summary["resource_entry_count"] += 1
            for row_index in range(row_count):
                x, y, z, w = struct.unpack_from("<4f", decoded, start + row_index * 16)
                if (w != 0.0 or not all(math.isfinite(value) for value in (x, y, z))):
                    continue
                summary["compared_position_row_count"] += 1
                position = (x, y, z)
                matches = [placement for placement in placements[resource_id]
                           if math.dist(position, placement[2]) <= tolerance]
                if not matches:
                    continue
                summary["matching_position_row_count"] += 1
                key = ",".join(f"{value:.3f}" for value in position)
                summary["matching_positions"][key] = (
                    summary["matching_positions"].get(key, 0) + 1
                )
                summary["matching_placements"].update(
                    (mission_id, instance_id)
                    for mission_id, instance_id, _placement_position in matches
                )

    return {
        "kind": "provisional-resource-position-correlation",
        "position_match_tolerance": tolerance,
        "maximum_rows_per_resource_entry": 49,
        "resource_ids": {
            str(resource_id): {
                "resource_entry_count": summary["resource_entry_count"],
                "compared_position_row_count": summary["compared_position_row_count"],
                "matching_position_row_count": summary["matching_position_row_count"],
                "unique_matching_positions": len(summary["matching_positions"]),
                "matching_positions": summary["matching_positions"],
                "matching_placement_count": len(summary["matching_placements"]),
            }
            for resource_id, summary in sorted(totals.items())
        },
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
    parser.add_argument(
        "--extract-pif-dir",
        type=Path,
        help="export supported PIF2 base mips as PNG diagnostic textures",
    )
    parser.add_argument(
        "--correlate-mission-root",
        type=Path,
        help="compare bounded resource rows with mission.instances Moby positions",
    )
    args = parser.parse_args()
    report = audit_scene_wad(args.scene_wad)
    if args.correlate_mission_root:
        report["moby_position_correlations"] = correlate_resource_position_rows(
            args.scene_wad, report, args.correlate_mission_root
        )
    if args.extract_pif_dir:
        for output_path in extract_scene_pif_textures(
            args.scene_wad, report, args.extract_pif_dir
        ):
            print(f"Extracted {output_path}")
    output = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(output, encoding="utf-8")
    else:
        print(output, end="")


if __name__ == "__main__":
    main()
