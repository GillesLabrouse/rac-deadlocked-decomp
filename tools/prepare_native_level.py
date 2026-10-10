#!/usr/bin/env python3
"""Prepare extracted terrain and static-object placements for native rendering.

This local asset bridge preserves prototype sharing. It does not implement game
logic, object behavior, mission selection, occlusion or original instance lighting.
"""

import argparse
import hashlib
import json
import math
import struct
import xml.etree.ElementTree as ET
from collections import defaultdict
from pathlib import Path

import asset_text
import prepare_native_scene as mesh_import

HEADER = struct.Struct("<8s5I6f")
INSTANCE = struct.Struct("<20f")
BATCH = struct.Struct("<5I")
IDENTITY = (1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1)


def affine_matrix(values):
    if not isinstance(values, list) or len(values) != 16 or not all(math.isfinite(v) for v in values):
        raise ValueError("Object transform requires sixteen finite values")
    if any(abs(values[i]) > 1e-6 for i in (3,7,11)) or abs(values[15]-1) > 1e-6:
        raise ValueError("Object transform must be affine")
    return tuple(float(v) for v in values)


def transform_point(point, matrix):
    return tuple(sum(point[i]*matrix[4*i+axis] for i in range(3)) + matrix[12+axis]
                 for axis in range(3))


def bounds_corners(vertices):
    low = tuple(min(v[i] for v in vertices) for i in range(3))
    high = tuple(max(v[i] for v in vertices) for i in range(3))
    return [(x,y,z) for x in (low[0],high[0]) for y in (low[1],high[1]) for z in (low[2],high[2])]


def class_meshes(directory):
    classes = {}
    inputs = {}
    for path in sorted(Path(directory).rglob("*.asset")):
        for root in asset_text.read(path):
            if root.kind != "TieClass":
                continue
            class_id = root.attributes.get("id")
            if type(class_id) is not int or class_id < 0 or class_id in classes:
                raise ValueError("Invalid or duplicate static object class")
            candidates = [node for node in root.children if node.kind == "Mesh" and "src" in node.attributes]
            if len(candidates) != 1:
                raise ValueError("Static object class needs one exported mesh")
            mesh = (path.parent / candidates[0].attributes["src"]).resolve()
            if not mesh.is_relative_to(path.parent.resolve()) or mesh.suffix.lower() != ".dae" or not mesh.is_file():
                raise ValueError("Invalid or missing static object mesh")
            classes[class_id] = mesh
            inputs[class_id] = path.resolve()
    return classes, inputs


def write_package(prototypes, output, source_inputs):
    """prototypes: (material->vertices, matrices, source description)."""
    textures = sorted({texture for groups, _, _ in prototypes for texture in groups if texture is not None}, key=str)
    texture_ids = {texture: i+1 for i, texture in enumerate(textures)}
    vertices, instances, batches, world_corners = [], [], [], []
    for groups, matrices, description in prototypes:
        matrices = [affine_matrix(list(matrix)) for matrix in matrices]
        first_instance = len(instances)
        for matrix in matrices:
            instances.append(tuple(matrix) + (1,1,1,1))
        geometry = [v for group in groups.values() for v in group]
        if not geometry or not matrices:
            raise ValueError("A scene prototype needs geometry and placements")
        corners = bounds_corners(geometry)
        world_corners.extend(transform_point(p, m) for m in matrices for p in corners)
        for texture, group in groups.items():
            if not group:
                continue
            batches.append((len(vertices), len(group), texture_ids.get(texture, 0), first_instance, len(matrices)))
            vertices.extend(group)
    bounds = [min(p[i] for p in world_corners) for i in range(3)] + [max(p[i] for p in world_corners) for i in range(3)]
    if not all(map(math.isfinite, bounds)):
        raise ValueError("Scene transforms produce non-finite bounds")
    payload = bytearray(HEADER.pack(mesh_import.MAGIC, 2, len(vertices), len(batches), len(textures), len(instances), *bounds))
    for texture in textures:
        png = texture.read_bytes()
        if not png.startswith(b"\x89PNG\r\n\x1a\n"):
            raise ValueError("Texture is not a PNG: " + str(texture))
        payload.extend(struct.pack("<I", len(png)))
        payload.extend(png)
    for vertex in vertices:
        payload.extend(mesh_import.VERTEX.pack(*vertex))
    for instance in instances:
        payload.extend(INSTANCE.pack(*instance))
    for batch in batches:
        payload.extend(BATCH.pack(*batch))
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(payload)
    drawn_triangles = sum(count//3 * copies for _, count, _, _, copies in batches)
    dependencies = sorted(set(map(Path, source_inputs)) | set(textures), key=str)
    report = {"kind": "native-static-scene-assets", "format_version": 2,
              "vertices": len(vertices), "prototype_triangles": len(vertices)//3,
              "submitted_triangles": drawn_triangles, "instances": len(instances),
              "batches": len(batches), "textures": len(textures), "bounds": bounds,
              "sha256": hashlib.sha256(payload).hexdigest(),
              "prototypes": [description for _, _, description in prototypes],
              "source_inputs": [{"path": str(p.resolve()), "sha256": hashlib.sha256(p.read_bytes()).hexdigest()} for p in dependencies],
              "contains_game_logic": False, "original_instance_lighting": False,
              "excluded_systems": ["dynamic objects", "vegetation", "collision", "sky", "missions", "audio"]}
    output.with_suffix(".json").write_text(json.dumps(report, indent=2)+"\n")
    return report


def prepare_level(extracted, level, output):
    extracted, level = Path(extracted).resolve(), Path(level).resolve()
    if not level.is_relative_to(extracted):
        raise ValueError("Level must be inside the extraction directory")
    placements = level / "gameplay.instances"
    nodes = asset_text.read(placements)
    classes, definitions = class_meshes(extracted / "tie_classes")
    allowed = set()
    for root in asset_text.read(level / "tie_classes" / "tie_classes.asset"):
        for ref in root.children:
            if ref.kind == "Reference":
                allowed.add(int(ref.name))
    by_class = defaultdict(list)
    ids = set()
    for node in nodes:
        if node.kind != "Tie":
            continue
        if node.name in ids:
            raise ValueError("Duplicate static object instance")
        ids.add(node.name)
        class_id = node.attributes.get("class")
        if type(class_id) is not int or class_id not in allowed or class_id not in classes:
            raise ValueError("Static object references an unavailable level class")
        by_class[class_id].append(affine_matrix(node.attributes.get("matrix")))
    terrain_meshes = sorted((level / "chunks").glob("*/tfrags/mesh.dae"))
    if not terrain_meshes:
        raise ValueError("Level has no exported terrain")
    shared_textures = level / "chunks" / "0" / "tfrags"
    prototypes = []
    inputs = [placements, level / "tie_classes" / "tie_classes.asset"]
    for terrain in terrain_meshes:
        groups, rebound = mesh_import.read_mesh(terrain, recover_unbound_colors=True,
                                               texture_directory=shared_textures)
        prototypes.append((groups, [IDENTITY], {"kind": "terrain", "chunk": terrain.parent.parent.name,
                                              "placements": 1, "unbound_color_primitives_recovered": rebound}))
        inputs.append(terrain)
    for class_id, matrices in sorted(by_class.items()):
        groups, _ = mesh_import.read_mesh(classes[class_id])
        prototypes.append((groups, matrices, {"kind": "static-object", "class_id": class_id, "placements": len(matrices)}))
        inputs.extend((classes[class_id], definitions[class_id]))
    return write_package(prototypes, output, inputs)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--extracted", required=True, type=Path)
    parser.add_argument("--level", required=True, type=Path)
    parser.add_argument("-o", "--output", required=True, type=Path)
    args = parser.parse_args()
    try:
        report = prepare_level(args.extracted, args.level, args.output)
    except (ValueError, KeyError, TypeError, OSError, struct.error, ET.ParseError) as error:
        parser.exit(1, str(error)+"\n")
    print("Prepared %d shared vertices, %d instances, %d batches, %d textures" %
          (report["vertices"], report["instances"], report["batches"], report["textures"]))


if __name__ == "__main__":
    main()
