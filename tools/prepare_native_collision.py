#!/usr/bin/env python3
"""Prepare an isolated diagnostic scene from exported Dreadzone collision meshes."""

import argparse
import json
import math
from pathlib import Path

import prepare_native_level as scene
import prepare_native_scene as mesh_import


CHUNK_COLORS = ((0.20, 0.85, 1.0, 1.0), (1.0, 0.48, 0.12, 1.0),
                (0.35, 1.0, 0.35, 1.0))


def color_collision_geometry(groups, color):
    if len(color) != 4 or not all(math.isfinite(value) and 0 <= value <= 1 for value in color):
        raise ValueError("Collision diagnostic color must contain four finite normalized values")
    result = {}
    for texture, vertices in groups.items():
        if len(vertices) % 3:
            raise ValueError("Collision mesh vertex count is not a triangle multiple")
        result[texture] = [(*vertex[:5], *color) for vertex in vertices]
    if not any(result.values()):
        raise ValueError("Collision mesh has no drawable triangles")
    return result


def prepare(level_directory, output):
    level_directory, output = Path(level_directory).resolve(), Path(output).resolve()
    prototypes, source_inputs, chunks = [], [], []
    for directory in sorted((level_directory / "chunks").iterdir(),
                            key=lambda path: (0, int(path.name)) if path.name.isdecimal()
                            else (1, path.name)):
        if not directory.is_dir() or not directory.name.isdecimal():
            continue
        source = directory / "collision" / "collision.dae"
        if not source.is_file():
            continue
        groups, _ = mesh_import.read_mesh(source)
        color = CHUNK_COLORS[int(directory.name) % len(CHUNK_COLORS)]
        colored = color_collision_geometry(groups, color)
        triangles = sum(len(vertices) for vertices in colored.values()) // 3
        prototypes.append((colored, [scene.IDENTITY], {
            "kind": "diagnostic-collision-mesh",
            "chunk": directory.name,
            "triangles": triangles,
            "diagnostic_color_rgba": color,
            "source_coordinates_preserved": True,
            "game_collision_runtime_verified": False,
        }))
        source_inputs.append(source)
        chunks.append({"chunk": directory.name, "source": str(source), "triangles": triangles,
                       "diagnostic_color_rgba": color})
    if not prototypes:
        raise ValueError("Level has no exported collision meshes")
    report = scene.write_package(prototypes, output, source_inputs)
    report.update(kind="native-level-collision-diagnostic", format_version=1,
                  chunks=chunks, triangle_count=sum(item["triangles"] for item in chunks),
                  collision_material_ids_preserved=False,
                  contains_game_logic=False,
                  excluded_systems=["dynamic objects", "vegetation", "mission selection",
                                    "collision response", "occlusion", "original lighting"])
    output.with_suffix(".json").write_text(json.dumps(report, indent=2) + "\n")
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--level", required=True, type=Path)
    parser.add_argument("-o", "--output", required=True, type=Path)
    args = parser.parse_args()
    try:
        report = prepare(args.level, args.output)
    except (OSError, ValueError) as error:
        parser.exit(1, f"Collision scene preparation failed: {error}\n")
    print(f"Prepared {report['triangle_count']} collision triangles in {len(report['chunks'])} chunks")


if __name__ == "__main__":
    main()
