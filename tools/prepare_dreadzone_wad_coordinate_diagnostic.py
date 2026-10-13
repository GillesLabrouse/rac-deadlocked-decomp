#!/usr/bin/env python3
"""Build a native point-cloud diagnostic for a bounded scene-WAD resource.

The generated octahedra are display markers, not recovered triangles. The
tool preserves raw source coordinates and makes no claim about their frame,
topology, object identity, or relationship to a Moby placement.
"""

import argparse
import json
import math
import struct
from pathlib import Path

from prepare_native_level import IDENTITY, write_package
from unpack_wad import decompress

SECTOR_SIZE = 2048
ROW = struct.Struct("<4f")


def extract_rows(report, wad_data, scene_index, first_chunk, last_chunk,
                 resource_id, row_cap=49):
    """Extract finite xyz rows bounded by the next resource descriptor."""
    if first_chunk < 0 or last_chunk < first_chunk or row_cap <= 0:
        raise ValueError("Invalid chunk range or row cap")

    inventory = report.get("scene_chunk_inventory", {})
    chunks = inventory.get("chunks", [])
    nested = inventory.get("nested_candidates", [])
    rows = []
    for chunk in chunks:
        chunk_index = chunk.get("chunk_index")
        if (chunk.get("scene_index") != scene_index or
                not first_chunk <= chunk_index <= last_chunk):
            continue
        entries = chunk.get("resource_entries", [])
        matches = [entry for entry in entries
                   if entry.get("resource_id") == resource_id]
        if not matches:
            continue

        decoded = decompress(wad_data, chunk["sector"] * SECTOR_SIZE)
        boundaries = sorted(
            [entry["descriptor_offset"] for entry in entries] +
            [entry["descriptor_offset"] for entry in nested
             if entry.get("scene_index") == scene_index and
             entry.get("chunk_index") == chunk_index]
        )
        for entry in matches:
            start = entry["payload_offset"]
            end = next((offset for offset in boundaries if offset > start),
                       len(decoded))
            count = min(row_cap, max(0, (end - start) // ROW.size))
            for row_index in range(count):
                x, y, z, w = ROW.unpack_from(decoded, start + row_index * ROW.size)
                if w != 0.0 or not all(map(math.isfinite, (x, y, z))):
                    raise ValueError(
                        f"Invalid coordinate row in chunk {chunk_index} "
                        f"at row {row_index}"
                    )
                rows.append((chunk_index, row_index, (x, y, z)))
    return rows


def point_markers(rows, radius, first_chunk=5, last_chunk=32):
    """Return colored octahedron markers while keeping raw xyz unchanged."""
    if not math.isfinite(radius) or radius <= 0:
        raise ValueError("Marker radius must be positive and finite")
    faces = ((0, 2, 4), (2, 1, 4), (1, 3, 4), (3, 0, 4),
             (2, 0, 5), (1, 2, 5), (3, 1, 5), (0, 3, 5))
    vertices = []
    for chunk_index, _row_index, (x, y, z) in rows:
        progress = ((chunk_index - first_chunk) / (last_chunk - first_chunk)
                    if last_chunk > first_chunk else 0.5)
        color = (0.15 + 0.8 * progress, 0.85 - 0.65 * progress,
                 0.95 - 0.45 * progress, 1.0)
        corners = ((x + radius, y, z), (x - radius, y, z),
                   (x, y + radius, z), (x, y - radius, z),
                   (x, y, z + radius), (x, y, z - radius))
        for face in faces:
            for index in face:
                vertices.append((*corners[index], 0.0, 0.0, *color))
    return vertices


def build_diagnostic(report_path, wad_path, output, scene_index=20,
                     first_chunk=5, last_chunk=32, resource_id=9228,
                     marker_radius=0.012):
    report_path, wad_path, output = map(Path, (report_path, wad_path, output))
    report = json.loads(report_path.read_text(encoding="utf-8"))
    rows = extract_rows(report, wad_path.read_bytes(), scene_index,
                        first_chunk, last_chunk, resource_id)
    if not rows:
        raise ValueError("No bounded coordinate rows matched the selection")
    description = {
        "kind": "raw-coordinate-point-diagnostic",
        "candidate_resource_id": resource_id,
        "scene_index": scene_index,
        "chunk_range": [first_chunk, last_chunk],
        "source_rows": len(rows),
        "marker_radius": marker_radius,
        "coordinate_frame_verified": False,
        "source_topology_verified": False,
        "contains_recovered_object_mesh": False,
        "display_note": (
            "Raw xyz rows are shown unchanged in source space with identity "
            "placement. Octahedra are artificial display markers; color "
            "changes by chunk index."
        ),
    }
    vertices = point_markers(rows, marker_radius, first_chunk, last_chunk)
    write_package([({None: vertices}, [IDENTITY], description)], output,
                  [wad_path])

    metadata_path = output.with_suffix(".json")
    metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
    metadata.update({
        "diagnostic_only": True,
        "coordinate_frame_verified": False,
        "source_topology_verified": False,
        "contains_recovered_object_mesh": False,
        "source_coordinate_rows": len(rows),
        "source_chunks": sorted({chunk for chunk, _, _ in rows}),
        "marker_radius_in_source_units": marker_radius,
        "color_encoding": "Hue progresses across chunk indices.",
        "warning": (
            "This file visualizes raw resource coordinates only. Marker "
            "octahedra are artificial; do not merge them into the level scene "
            "or interpret them as recovered object geometry."
        ),
    })
    metadata_path.write_text(json.dumps(metadata, indent=2) + "\n",
                             encoding="utf-8")
    return metadata


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--audit-report", required=True, type=Path)
    parser.add_argument("--wad", type=Path,
                        help="Defaults to the WAD path recorded in the report")
    parser.add_argument("--scene-index", type=int, default=20)
    parser.add_argument("--first-chunk", type=int, default=5)
    parser.add_argument("--last-chunk", type=int, default=32)
    parser.add_argument("--resource-id", type=int, default=9228)
    parser.add_argument("--marker-radius", type=float, default=0.012)
    parser.add_argument("-o", "--output", type=Path, required=True)
    args = parser.parse_args()

    report = json.loads(args.audit_report.read_text(encoding="utf-8"))
    wad_path = args.wad or Path(report["source"])
    metadata = build_diagnostic(
        args.audit_report, wad_path, args.output, args.scene_index,
        args.first_chunk, args.last_chunk, args.resource_id,
        args.marker_radius)
    print(json.dumps({
        "scene": str(args.output.resolve()),
        "report": str(args.output.with_suffix(".json").resolve()),
        "source_rows": metadata["source_coordinate_rows"],
        "vertices": metadata["vertices"],
        "bounds": metadata["bounds"],
    }, indent=2))


if __name__ == "__main__":
    main()
