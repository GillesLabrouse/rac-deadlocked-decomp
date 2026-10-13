#!/usr/bin/env python3
"""Prepare extracted Dreadzone collision meshes for the native scene viewer."""

import argparse
import json
from pathlib import Path

import prepare_native_level as level_bridge
import prepare_native_scene as mesh_import


def prepare_collision(level_directory, output_path):
    level_directory = Path(level_directory).resolve()
    meshes = sorted((level_directory / "chunks").glob("*/collision/collision.dae"))
    if not meshes:
        raise ValueError("No extracted chunk collision meshes were found")

    prototypes = []
    for mesh in meshes:
        groups, _ = mesh_import.read_mesh(mesh)
        prototypes.append((groups, [level_bridge.IDENTITY], {
            "kind": "collision",
            "chunk": mesh.parent.parent.name,
            "placements": 1,
        }))

    report = level_bridge.write_package(prototypes, output_path, meshes)
    report["kind"] = "native-level-collision-scene-assets"
    report["collision_chunks"] = [mesh.parent.parent.name for mesh in meshes]
    report["collision_triangle_count"] = report["submitted_triangles"]
    report["excluded_systems"] = [
        "visual terrain", "static object meshes", "vegetation", "dynamic object meshes",
        "dynamic object behavior", "character", "sky", "missions", "audio",
    ]
    Path(output_path).with_suffix(".json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--level", type=Path, required=True)
    parser.add_argument("-o", "--output", type=Path, required=True)
    args = parser.parse_args()
    try:
        report = prepare_collision(args.level, args.output)
    except (ValueError, OSError) as error:
        parser.exit(1, str(error) + "\n")
    print("Prepared %d collision triangles across %d chunks" % (
        report["collision_triangle_count"], len(report["collision_chunks"])
    ))


if __name__ == "__main__":
    main()
