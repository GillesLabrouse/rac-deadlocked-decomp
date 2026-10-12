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


def exported_class_meshes(directory, class_kind, extension, label, allow_missing=False):
    classes = {}
    inputs = {}
    for path in sorted(Path(directory).rglob("*.asset")):
        for root in asset_text.read(path):
            if root.kind != class_kind:
                continue
            class_id = root.attributes.get("id")
            if type(class_id) is not int or class_id < 0 or class_id in classes:
                raise ValueError("Invalid or duplicate " + label + " class")
            candidates = []
            pending = list(root.children)
            while pending:
                node = pending.pop()
                if node.kind == "Mesh" and "src" in node.attributes:
                    candidates.append(node)
                pending.extend(node.children)
            if len(candidates) != 1:
                if allow_missing and not candidates:
                    inputs[class_id] = path.resolve()
                    continue
                raise ValueError(label + " class needs one exported mesh")
            mesh = (path.parent / candidates[0].attributes["src"]).resolve()
            if (not mesh.is_relative_to(path.parent.resolve()) or
                    mesh.suffix.lower() != extension or not mesh.is_file()):
                raise ValueError("Invalid or missing " + label + " mesh")
            classes[class_id] = mesh
            inputs[class_id] = path.resolve()
    return classes, inputs


def unambiguous_mission_moby_meshes(level_directory):
    """Find mission meshes safe to reuse when their exported bytes agree."""
    candidates = defaultdict(dict)
    mission_root = Path(level_directory) / "missions"
    if not mission_root.is_dir():
        return {}
    for directory in sorted(mission_root.glob("*/moby_classes")):
        if not directory.is_dir() or not directory.parent.name.isdecimal():
            continue
        meshes, definitions = exported_class_meshes(
            directory, "MobyClass", ".glb", "mission dynamic object", allow_missing=True)
        for class_id, mesh in meshes.items():
            digest = hashlib.sha256(mesh.read_bytes()).hexdigest()
            candidates[class_id].setdefault(digest, (mesh, definitions[class_id]))
    return ({class_id: next(iter(variants.values()))
             for class_id, variants in candidates.items() if len(variants) == 1},
            {class_id: len(variants) for class_id, variants in candidates.items()})


def class_meshes(directory):
    return exported_class_meshes(directory, "TieClass", ".dae", "static object")


def moby_matrix(position, rotation, scale):
    if (not isinstance(position, list) or len(position) != 3 or
            not isinstance(rotation, list) or len(rotation) != 3 or
            type(scale) not in (int, float)):
        raise ValueError("Dynamic object transform needs position, Euler rotation and scale")
    values = [*position, *rotation, scale]
    if not all(type(value) in (int, float) and math.isfinite(value) for value in values) or scale < 0:
        raise ValueError("Dynamic object transform must contain finite values and nonnegative scale")
    x, y, z = map(float, rotation)
    sx, sy, sz = math.sin(x), math.sin(y), math.sin(z)
    cx, cy, cz = math.cos(x), math.cos(y), math.cos(z)
    s = float(scale)
    # Compose local X, Y, then Z. L01's placed-actor loader at 0057B788 copies
    # each record's angles to actor+F0 and calls 0058DF58, whose recovered basis
    # uses this same ordered composition. Mesh-local axes and visibility remain
    # separate from this verified instance rotation order.
    return affine_matrix([
        cz*cy*s, sz*cy*s, -sy*s, 0,
        (cz*sy*sx - sz*cx)*s, (sz*sy*sx + cz*cx)*s, cy*sx*s, 0,
        (cz*sy*cx + sz*sx)*s, (sz*sy*cx - cz*sx)*s, cy*cx*s, 0,
        *map(float, position), 1,
    ])


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
    report = {"kind": "native-level-scene-assets", "format_version": 2,
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


def prepare_level(extracted, level, output, include_sky=False, mission_id=None,
                  include_dynamic_previews=False, dynamic_class_id=None,
                  excluded_dynamic_class_ids=None, dynamic_only=False):
    import prepare_native_moby as moby_mesh_import

    if dynamic_class_id is not None and not include_dynamic_previews:
        raise ValueError("Dynamic class filters require diagnostic previews")
    dynamic_class_ids = ({dynamic_class_id} if type(dynamic_class_id) is int else
                         set(dynamic_class_id) if dynamic_class_id is not None else None)
    if dynamic_class_ids is not None and any(type(class_id) is not int or class_id < 0
                                             for class_id in dynamic_class_ids):
        raise ValueError("Dynamic class filters need nonnegative integer IDs")
    excluded_dynamic_class_ids = set(excluded_dynamic_class_ids or [])
    if any(type(class_id) is not int or class_id < 0
           for class_id in excluded_dynamic_class_ids):
        raise ValueError("Excluded dynamic classes need nonnegative integer IDs")
    if dynamic_class_ids is not None and dynamic_class_ids & excluded_dynamic_class_ids:
        raise ValueError("A dynamic class cannot be both selected and excluded")
    if dynamic_only and (mission_id is None or not include_dynamic_previews or
                         dynamic_class_ids is None or include_sky):
        raise ValueError("Dynamic-only scenes require a selected mission and class, enabled previews, and no sky")

    extracted, level = Path(extracted).resolve(), Path(level).resolve()
    if not level.is_relative_to(extracted):
        raise ValueError("Level must be inside the extraction directory")
    placements = level / "gameplay.instances"
    nodes = asset_text.read(placements)
    classes, definitions = class_meshes(extracted / "tie_classes")
    allowed = set()
    tie_classes_file = level / "tie_classes" / "tie_classes.asset"
    for root in asset_text.read(tie_classes_file):
        for ref in root.children:
            if ref.kind == "Reference":
                allowed.add(int(ref.name))
    shrub_classes_file = level / "shrub_classes" / "shrub_classes.asset"
    shrub_classes, shrub_definitions = exported_class_meshes(
        extracted / "shrub_classes", "ShrubClass", ".glb", "vegetation")
    allowed_shrubs = set()
    for root in asset_text.read(shrub_classes_file):
        for ref in root.children:
            if ref.kind == "Reference":
                allowed_shrubs.add(int(ref.name))
    moby_classes_file = level / "moby_classes" / "moby_classes.asset"
    moby_classes, moby_definitions = exported_class_meshes(
        extracted / "moby_classes", "MobyClass", ".glb", "dynamic object", allow_missing=True)
    moby_sources = dict(moby_classes)
    moby_source_definitions = dict(moby_definitions)
    allowed_mobys = set()
    for root in asset_text.read(moby_classes_file):
        for ref in root.children:
            if ref.kind == "Reference":
                allowed_mobys.add(int(ref.name))
    by_class = defaultdict(list)
    shrubs_by_class = defaultdict(list)
    mobys_by_class = defaultdict(list)
    omitted_mobys = defaultdict(lambda: {"placements": 0, "reason": ""})
    disabled_dynamic_preview_placements = 0
    ids = set()
    shrub_ids = set()
    moby_ids = set()
    mission_nodes = []
    mission_path = None
    mission_moby_classes = {}
    mission_moby_definitions = {}
    mission_moby_variant_counts = {}
    reused_cross_mission_mesh_classes = set()
    if mission_id is not None:
        if type(mission_id) is not int or mission_id < 0:
            raise ValueError("Mission ID must be a nonnegative integer")
        mission_path = level / "missions" / str(mission_id)
        mission_instances = mission_path / "mission.instances"
        mission_classes_directory = mission_path / "moby_classes"
        if not mission_instances.is_file():
            raise ValueError("Selected mission needs local instances")
        mission_nodes = asset_text.read(mission_instances)
        if mission_classes_directory.is_dir():
            mission_moby_classes, mission_moby_definitions = exported_class_meshes(
                mission_classes_directory, "MobyClass", ".glb", "mission dynamic object",
                allow_missing=True)
        else:
            mission_moby_classes, mission_moby_definitions = {}, {}
        moby_sources.update(mission_moby_classes)
        moby_source_definitions.update(mission_moby_definitions)
        reusable_mission_meshes, mission_moby_variant_counts = unambiguous_mission_moby_meshes(level)
        for class_id, (mesh, definition) in reusable_mission_meshes.items():
            if class_id not in moby_sources:
                moby_sources[class_id] = mesh
                moby_source_definitions.setdefault(class_id, definition)
                reused_cross_mission_mesh_classes.add(class_id)
    combined_nodes = [*nodes, *mission_nodes]
    for node_index, node in enumerate(combined_nodes):
        namespace = 0 if node_index < len(nodes) else 1
        if node.kind == "Tie":
            identity = (namespace, node.name)
            if identity in ids:
                raise ValueError("Duplicate static object instance")
            ids.add(identity)
            class_id = node.attributes.get("class")
            if type(class_id) is not int or class_id not in allowed or class_id not in classes:
                raise ValueError("Static object references an unavailable level class")
            by_class[class_id].append(affine_matrix(node.attributes.get("matrix")))
        elif node.kind == "Shrub":
            identity = (namespace, node.name)
            if identity in shrub_ids:
                raise ValueError("Duplicate vegetation instance")
            shrub_ids.add(identity)
            class_id = node.attributes.get("class")
            if (type(class_id) is not int or class_id not in allowed_shrubs or
                    class_id not in shrub_classes):
                raise ValueError("Vegetation references an unavailable level class")
            shrubs_by_class[class_id].append(affine_matrix(node.attributes.get("matrix")))
        elif node.kind == "Moby":
            identity = (namespace, node.name)
            if identity in moby_ids:
                raise ValueError("Duplicate dynamic object instance")
            moby_ids.add(identity)
            if not include_dynamic_previews:
                disabled_dynamic_preview_placements += 1
                continue
            class_id = node.attributes.get("class")
            if type(class_id) is not int or class_id < 0:
                raise ValueError("Dynamic object references an unavailable level class")
            if class_id not in moby_source_definitions:
                omitted_mobys[class_id]["reason"] = (
                    "ambiguous cross-mission mesh variants"
                    if mission_moby_variant_counts.get(class_id, 0) > 1
                    else "no exported class definition")
                omitted_mobys[class_id]["placements"] += 1
                continue
            if dynamic_class_ids is not None and class_id not in dynamic_class_ids:
                omitted_mobys[class_id]["reason"] = "excluded by diagnostic class filter"
                omitted_mobys[class_id]["placements"] += 1
                continue
            if class_id in excluded_dynamic_class_ids:
                omitted_mobys[class_id]["reason"] = "excluded by explicit diagnostic class exclusion"
                omitted_mobys[class_id]["placements"] += 1
                continue
            if class_id not in moby_sources:
                omitted_mobys[class_id]["reason"] = (
                    "ambiguous cross-mission mesh variants"
                    if mission_moby_variant_counts.get(class_id, 0) > 1
                    else "no exported editor mesh")
                omitted_mobys[class_id]["placements"] += 1
                continue
            try:
                matrix = moby_matrix(node.attributes.get("pos"), node.attributes.get("rot"),
                                     node.attributes.get("scale"))
            except ValueError as error:
                raise ValueError("Invalid dynamic object transform: " + str(error)) from error
            mobys_by_class[class_id].append(matrix)
    prototypes = []
    inputs = [placements, tie_classes_file, shrub_classes_file, moby_classes_file]
    if not dynamic_only:
        terrain_meshes = sorted((level / "chunks").glob("*/tfrags/mesh.dae"))
        if not terrain_meshes:
            raise ValueError("Level has no exported terrain")
        shared_textures = level / "chunks" / "0" / "tfrags"
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
        for class_id, matrices in sorted(shrubs_by_class.items()):
            groups, _ = moby_mesh_import.read_mesh(shrub_classes[class_id])
            prototypes.append((groups, matrices, {"kind": "vegetation", "class_id": class_id,
                                                  "placements": len(matrices)}))
            inputs.extend((shrub_classes[class_id], shrub_definitions[class_id]))
    rendered_moby_placements = 0
    rendered_moby_classes = 0
    static_joint_attribute_classes = 0
    for class_id, matrices in sorted(mobys_by_class.items()):
        try:
            groups, mesh_report = moby_mesh_import.read_mesh(
                moby_sources[class_id], allow_unskinned_static_joints=True)
        except ValueError as error:
            omitted_mobys[class_id]["reason"] = str(error)
            omitted_mobys[class_id]["placements"] += len(matrices)
            continue
        prototypes.append((groups, matrices, {"kind": "dynamic-object-preview", "class_id": class_id,
                                              "placements": len(matrices),
                                              "mesh_source": str(moby_sources[class_id]),
                                              "definition_source": str(moby_source_definitions[class_id]),
                                              "reused_cross_mission_mesh":
                                                  class_id in reused_cross_mission_mesh_classes}))
        inputs.extend((moby_sources[class_id], moby_source_definitions[class_id]))
        rendered_moby_placements += len(matrices)
        rendered_moby_classes += 1
        if mesh_report["ignored_static_joint_attributes"]:
            static_joint_attribute_classes += 1
    if include_sky:
        import prepare_native_sky
        sky_prototypes, sky_inputs, sky_report = prepare_native_sky.sky_prototypes(level)
        prototypes.extend(sky_prototypes)
        inputs.extend(sky_inputs)
    if dynamic_only and not any(row[2]["kind"] == "dynamic-object-preview" for row in prototypes):
        raise ValueError("Selected dynamic class has no renderable placements in this mission")
    report = write_package(prototypes, output, inputs)
    report["static_object_placements"] = 0 if dynamic_only else len(ids)
    report["vegetation_placements"] = 0 if dynamic_only else len(shrub_ids)
    report["vegetation_classes"] = 0 if dynamic_only else len(shrubs_by_class)
    report["dynamic_object_preview_placements"] = rendered_moby_placements
    report["dynamic_object_preview_classes"] = rendered_moby_classes
    report["static_joint_attribute_classes"] = static_joint_attribute_classes
    report["reused_cross_mission_mesh_classes"] = sorted({
        prototype["class_id"] for prototype in report["prototypes"]
        if prototype["kind"] == "dynamic-object-preview" and
        prototype.get("reused_cross_mission_mesh")})
    report["omitted_dynamic_objects"] = dict(sorted(omitted_mobys.items()))
    report["omitted_dynamic_object_placements"] = sum(v["placements"] for v in omitted_mobys.values())
    report["mission_id"] = mission_id
    report["mission_object_placements"] = sum(1 for node in mission_nodes if node.kind == "Moby")
    report["dynamic_preview_class_filter"] = (sorted(dynamic_class_ids)
                                               if dynamic_class_ids is not None and len(dynamic_class_ids) != 1
                                               else next(iter(dynamic_class_ids))
                                               if dynamic_class_ids is not None else None)
    report["dynamic_preview_excluded_classes"] = sorted(excluded_dynamic_class_ids)
    report["dynamic_preview_transforms_verified"] = False
    report["dynamic_preview_rotation_order_verified"] = True
    report["dynamic_preview_placements_disabled"] = disabled_dynamic_preview_placements
    report["dynamic_only_class_scene"] = dynamic_only
    if mission_path is not None:
        inputs = list(inputs) + [mission_path / "mission.instances"]
        report["source_inputs"] = [{"path": str(p.resolve()),
                                    "sha256": hashlib.sha256(p.read_bytes()).hexdigest()}
                                   for p in sorted(set(map(Path, inputs)), key=str)]
    report["excluded_systems"] = ["dynamic object behavior", "character skinning and animation",
                                   "collision", "missions", "audio"]
    if dynamic_only:
        report["excluded_systems"].extend(["terrain", "static objects", "vegetation", "sky"])
    if not include_dynamic_previews:
        report["excluded_systems"].append("dynamic object mesh previews")
    if include_sky:
        report["sky"] = sky_report
        report["excluded_systems"].append("sky animation and alpha blending")
    else:
        report["excluded_systems"].append("sky")
    output = Path(output)
    output.with_suffix(".json").write_text(json.dumps(report, indent=2) + "\n")
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--extracted", required=True, type=Path)
    parser.add_argument("--level", required=True, type=Path)
    parser.add_argument("-o", "--output", required=True, type=Path)
    parser.add_argument("--include-sky", action="store_true",
                        help="Include the four static sky shells; animation and blending remain unsupported")
    parser.add_argument("--mission", type=int,
                        help="Select one mission variant (requires dynamic previews to render its Mobys)")
    parser.add_argument("--include-dynamic-previews", action="store_true",
                        help="Include experimental Moby meshes with unverified local axes and visibility")
    parser.add_argument("--dynamic-class", type=int, action="append",
                        help="Limit diagnostic Moby previews to this class ID; may be repeated")
    parser.add_argument("--exclude-dynamic-class", type=int, action="append",
                        help="Exclude this class from diagnostic Moby previews; may be repeated")
    parser.add_argument("--dynamic-only", action="store_true",
                        help="Build an isolated preview scene for one selected mission class")
    args = parser.parse_args()
    if args.dynamic_class is not None and not args.include_dynamic_previews:
        parser.error("--dynamic-class requires --include-dynamic-previews")
    try:
        report = prepare_level(args.extracted, args.level, args.output, args.include_sky,
                               args.mission, args.include_dynamic_previews,
                               args.dynamic_class[0] if args.dynamic_class and len(args.dynamic_class) == 1
                               else args.dynamic_class,
                               args.exclude_dynamic_class, args.dynamic_only)
    except (ValueError, KeyError, TypeError, OSError, struct.error, ET.ParseError) as error:
        parser.exit(1, str(error)+"\n")
    print("Prepared %d shared vertices, %d instances, %d batches, %d textures" %
          (report["vertices"], report["instances"], report["batches"], report["textures"]))


if __name__ == "__main__":
    main()
