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


def unambiguous_mission_moby_cores(level_directory):
    """Find core containers safe to reuse when every mission copy is byte-identical."""
    candidates = defaultdict(dict)
    mission_root = Path(level_directory) / "missions"
    if not mission_root.is_dir():
        return {}
    for directory in sorted(mission_root.glob("*/moby_classes")):
        if not directory.is_dir() or not directory.parent.name.isdecimal():
            continue
        for definition in sorted(directory.rglob("*.asset")):
            for root in asset_text.read(definition):
                if root.kind != "MobyClass" or type(root.attributes.get("id")) is not int:
                    continue
                core_nodes = [node for node in root.children
                              if node.kind == "Binary" and node.name == "core"]
                if len(core_nodes) > 1:
                    raise ValueError("Mission dynamic object has duplicate core binaries")
                if not core_nodes:
                    continue
                core = (definition.parent / core_nodes[0].attributes.get("src", "")).resolve()
                if not core.is_relative_to(directory.resolve()) or not core.is_file():
                    raise ValueError("Mission dynamic object core is missing or escapes its class directory")
                digest = hashlib.sha256(core.read_bytes()).hexdigest()
                candidates[root.attributes["id"]].setdefault(digest, (core, definition.resolve()))
    return {class_id: next(iter(variants.values()))
            for class_id, variants in candidates.items() if len(variants) == 1}


def unambiguous_mission_moby_geometries(level_directory, class_ids):
    """Reuse an untextured geometry only when all exported variant triangles match."""
    if not class_ids:
        return {}
    import prepare_native_moby as moby_mesh_import
    candidates = defaultdict(list)
    invalid = set()
    mission_root = Path(level_directory) / "missions"
    for directory in sorted(mission_root.glob("*/moby_classes")):
        if not directory.is_dir() or not directory.parent.name.isdecimal():
            continue
        for definition in sorted(directory.rglob("*.asset")):
            for root in asset_text.read(definition):
                class_id = root.attributes.get("id")
                if root.kind != "MobyClass" or class_id not in class_ids:
                    continue
                mesh_nodes = []
                pending = list(root.children)
                while pending:
                    node = pending.pop()
                    if node.kind == "Mesh" and isinstance(node.attributes.get("src"), str):
                        mesh_nodes.append(node)
                    pending.extend(node.children)
                if not mesh_nodes:
                    continue
                if len(mesh_nodes) != 1:
                    invalid.add(class_id)
                    continue
                mesh = (definition.parent / mesh_nodes[0].attributes["src"]).resolve()
                if not mesh.is_relative_to(definition.parent.resolve()) or not mesh.is_file():
                    invalid.add(class_id)
                    continue
                try:
                    groups, _ = moby_mesh_import.read_mesh(mesh)
                    triangles = []
                    geometry = defaultdict(list)
                    for vertices in groups.values():
                        if len(vertices) % 3:
                            raise ValueError("Moby geometry contains an incomplete triangle")
                        for offset in range(0, len(vertices), 3):
                            triangle = vertices[offset:offset + 3]
                            signature = tuple(sorted(struct.pack("<3f", *vertex[:3])
                                                     for vertex in triangle))
                            triangles.append(signature)
                            for vertex in triangle:
                                geometry[None].append((*vertex[:5], 0.5, 0.5, 0.5, 1.0))
                    digest = hashlib.sha256(b"".join(sorted(
                        struct.pack("<I", len(part)) + b"".join(part) for part in triangles))).hexdigest()
                    candidates[class_id].append((digest, mesh, definition.resolve(), geometry))
                except (ValueError, OSError, struct.error):
                    invalid.add(class_id)
    result = {}
    for class_id, rows in candidates.items():
        if class_id in invalid or len({hashlib.sha256(row[1].read_bytes()).digest()
                                       for row in rows}) < 2:
            continue
        if len({row[0] for row in rows}) != 1:
            continue
        first = rows[0]
        result[class_id] = (first[3], [row[1] for row in rows],
                            [row[2] for row in rows], first[0])
    return result


def class_meshes(directory):
    return exported_class_meshes(directory, "TieClass", ".dae", "static object")


def native_material_textures(definition):
    """Resolve native material slots in order, retaining explicit untextured slots."""
    definition = Path(definition).resolve()
    roots = [root for root in asset_text.read(definition) if root.kind == "MobyClass"]
    if len(roots) != 1:
        raise ValueError("Native core definition must contain exactly one Moby class")
    materials = [child for child in roots[0].children if child.kind == "materials"]
    if len(materials) > 1:
        raise ValueError("Native core definition has duplicate material tables")
    if not materials:
        return sorted(definition.parent.glob("*.png"),
                      key=lambda path: int(path.stem) if path.stem.isdecimal() else path.name)
    result = []
    for material in materials[0].children:
        if material.kind != "Material":
            continue
        textures = [node for node in material.children
                    if node.kind == "Texture" and node.name == "diffuse"]
        if len(textures) > 1:
            raise ValueError("Native material has duplicate diffuse textures")
        if not textures:
            result.append(None)
            continue
        source = textures[0].attributes.get("src")
        path = (definition.parent / source).resolve() if isinstance(source, str) else None
        if path is None or not path.is_relative_to(definition.parent) or not path.is_file():
            result.append(None)
        else:
            result.append(path)
    if not result:
        result = sorted(definition.parent.glob("*.png"),
                        key=lambda path: int(path.stem) if path.stem.isdecimal() else path.name)
        return result or [None]
    return result


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


def unresolved_marker_geometry():
    """Build a position-only diagnostic marker without implying object shape or rotation."""
    vertices = []
    radius, half_length = 0.055, 0.65
    axes = ((0, 1, 2), (1, 0, 2), (2, 0, 1))
    for axis, side_a, side_b in axes:
        endpoints = []
        for sign in (-1, 1):
            point = [0.0, 0.0, 0.0]
            point[axis] = sign * half_length
            endpoints.append(tuple(point))
        ring = []
        for side, sign in ((side_a, 1), (side_b, 1), (side_a, -1), (side_b, -1)):
            point = [0.0, 0.0, 0.0]
            point[side] = sign * radius
            ring.append(tuple(point))
        for endpoint in endpoints:
            for index in range(4):
                triangle = (endpoint, ring[index], ring[(index + 1) % 4])
                vertices.extend((*point, 0.0, 0.0, 1.0, 0.1, 0.95, 1.0)
                                for point in triangle)
    return {None: vertices}


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


def editor_data_inventory(nodes):
    """Preserve non-mesh editor objects and their source attributes in the scene report."""
    spatial_kinds = {"Cuboid", "Cylinder", "Area", "Path", "GrindPath", "Camera",
                     "DirLight", "PointLight", "Sound", "MobyGroup", "TieGroup",
                     "ShrubGroup"}
    entries = []
    counts = defaultdict(int)
    for node in nodes:
        if node.kind not in spatial_kinds:
            continue
        counts[node.kind] += 1
        entries.append({"kind": node.kind, "id": node.name, "source_attributes": node.attributes})
    return {"kind": "level-editor-data-inventory", "format_version": 1,
            "counts_by_kind": dict(sorted(counts.items())), "entries": entries,
            "source_values_preserved": True,
            "rendered_as_game_objects": False}


def prepare_level(extracted, level, output, include_sky=False, mission_id=None,
                  include_dynamic_previews=False, dynamic_class_id=None,
                  excluded_dynamic_class_ids=None, dynamic_only=False,
                  include_native_bind_previews=False,
                  include_native_subnormal_flush_previews=False,
                  include_unresolved_moby_markers=False,
                  dynamic_instance_id=None,
                  native_bind_class_ids=None):
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
    native_bind_class_ids = set(native_bind_class_ids or [])
    if any(type(class_id) is not int or class_id < 0 for class_id in native_bind_class_ids):
        raise ValueError("Native bind-pose class IDs need nonnegative integers")
    if native_bind_class_ids and not include_native_bind_previews:
        raise ValueError("Native bind-pose class selection requires native bind-pose previews")
    if include_native_bind_previews and (mission_id is None or not include_dynamic_previews):
        raise ValueError("Native bind-pose previews require a selected mission and dynamic previews")
    if include_native_subnormal_flush_previews and not include_native_bind_previews:
        raise ValueError("Subnormal-flush previews require native bind-pose previews")
    if include_unresolved_moby_markers and not include_dynamic_previews:
        raise ValueError("Unresolved Moby markers require dynamic previews")
    if include_unresolved_moby_markers and (dynamic_class_ids is not None or
                                              excluded_dynamic_class_ids or dynamic_only):
        raise ValueError("Unresolved Moby markers require an unfiltered full scene")
    if dynamic_instance_id is not None:
        if type(dynamic_instance_id) is not int or dynamic_instance_id < 0:
            raise ValueError("Dynamic instance filters need a nonnegative instance ID")
        if (mission_id is None or not include_dynamic_previews or dynamic_only is not True or
                dynamic_class_ids is None or len(dynamic_class_ids) != 1 or include_sky):
            raise ValueError("Instance-only previews require one mission, class, and instance filter")
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
    moby_core_sources = {}
    reusable_mission_geometries = {}
    geometry_only_mobys = defaultdict(list)
    for definition in sorted((extracted / "moby_classes").rglob("*.asset")):
        for root in asset_text.read(definition):
            if root.kind != "MobyClass" or type(root.attributes.get("id")) is not int:
                continue
            cores = [node for node in root.children if node.kind == "Binary" and node.name == "core"]
            if len(cores) > 1:
                raise ValueError("Global dynamic object class has duplicate core binaries")
            if cores:
                source = (definition.parent / cores[0].attributes.get("src", "")).resolve()
                if not source.is_relative_to((extracted / "moby_classes").resolve()) or not source.is_file():
                    raise ValueError("Global dynamic object core binary is missing or escapes its class directory")
                moby_core_sources[root.attributes["id"]] = (source, definition.resolve())
    allowed_mobys = set()
    for root in asset_text.read(moby_classes_file):
        for ref in root.children:
            if ref.kind == "Reference":
                allowed_mobys.add(int(ref.name))
    by_class = defaultdict(list)
    shrubs_by_class = defaultdict(list)
    mobys_by_class = defaultdict(list)
    moby_positions_by_class = defaultdict(list)
    omitted_mobys = defaultdict(lambda: {"placements": 0, "reason": ""})
    disabled_dynamic_preview_placements = 0
    ids = set()
    shrub_ids = set()
    moby_ids = set()
    selected_dynamic_instance_count = 0
    selected_dynamic_instance = None
    dynamic_placement_inventory = []
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
        if mission_classes_directory.is_dir():
            for definition in sorted(mission_classes_directory.rglob("*.asset")):
                for root in asset_text.read(definition):
                    if root.kind != "MobyClass" or type(root.attributes.get("id")) is not int:
                        continue
                    cores = [node for node in root.children if node.kind == "Binary" and node.name == "core"]
                    if len(cores) > 1:
                        raise ValueError("Dynamic object class has duplicate core binaries")
                    if cores:
                        source = (definition.parent / cores[0].attributes.get("src", "")).resolve()
                        if not source.is_relative_to(mission_classes_directory.resolve()) or not source.is_file():
                            raise ValueError("Dynamic object core binary is missing or escapes its mission")
                        moby_core_sources[root.attributes["id"]] = (source, definition.resolve())
        for class_id, source in unambiguous_mission_moby_cores(level).items():
            moby_core_sources.setdefault(class_id, source)
        reusable_mission_meshes, mission_moby_variant_counts = unambiguous_mission_moby_meshes(level)
        for class_id, (mesh, definition) in reusable_mission_meshes.items():
            if class_id not in moby_sources:
                moby_sources[class_id] = mesh
                moby_source_definitions.setdefault(class_id, definition)
                reused_cross_mission_mesh_classes.add(class_id)
        unresolved_ambiguous = {class_id for class_id, count in mission_moby_variant_counts.items()
                                if count > 1 and class_id not in moby_sources}
        reusable_mission_geometries = unambiguous_mission_moby_geometries(
            level, unresolved_ambiguous)
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
            selected_by_preview_filters = (
                (dynamic_class_ids is None or class_id in dynamic_class_ids) and
                class_id not in excluded_dynamic_class_ids and
                (dynamic_instance_id is None or node.name == str(dynamic_instance_id)))
            dynamic_placement_inventory.append({
                "mission_id": mission_id,
                "instance_id": node.name,
                "class_id": class_id,
                "position_xyz": node.attributes.get("pos"),
                "rotation_xyz": node.attributes.get("rot"),
                "uniform_scale": node.attributes.get("scale"),
                "pvars": node.attributes.get("pvars"),
                "draw_distance": node.attributes.get("draw_dist"),
                "occlusion": node.attributes.get("occlusion"),
                "uid": node.attributes.get("uid"),
                "mesh_source_available": class_id in moby_sources,
                "geometry_source_available": class_id in reusable_mission_geometries,
                "native_core_source_available": class_id in moby_core_sources,
                "selected_by_preview_filters": selected_by_preview_filters,
            })
            if dynamic_instance_id is not None:
                if class_id not in dynamic_class_ids or node.name != str(dynamic_instance_id):
                    continue
                selected_dynamic_instance_count += 1
                selected_dynamic_instance = {
                    "mission_id": mission_id,
                    "instance_id": node.name,
                    "class_id": class_id,
                    "position_xyz": node.attributes.get("pos"),
                    "rotation_xyz": node.attributes.get("rot"),
                    "uniform_scale": node.attributes.get("scale"),
                    "pvars": node.attributes.get("pvars"),
                    "draw_distance": node.attributes.get("draw_dist"),
                    "occlusion": node.attributes.get("occlusion"),
                    "uid": node.attributes.get("uid"),
                }
            if include_unresolved_moby_markers:
                position = node.attributes.get("pos")
                if (not isinstance(position, list) or len(position) != 3 or
                        not all(type(value) in (int, float) and math.isfinite(value)
                                for value in position)):
                    raise ValueError("Unresolved Moby marker needs three finite position values")
                moby_positions_by_class[class_id].append({
                    "instance_id": node.name,
                    "position_xyz": list(map(float, position)),
                    "rotation_xyz": node.attributes.get("rot"),
                    "uniform_scale": node.attributes.get("scale"),
                    "pvars": node.attributes.get("pvars"),
                    "draw_distance": node.attributes.get("draw_dist"),
                    "occlusion": node.attributes.get("occlusion"),
                    "mission_id": node.attributes.get("mission"),
                    "uid": node.attributes.get("uid"),
                })
            if (class_id not in moby_source_definitions and
                    class_id not in reusable_mission_geometries):
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
                if class_id in reusable_mission_geometries:
                    try:
                        geometry_only_mobys[class_id].append(
                            moby_matrix(node.attributes.get("pos"), node.attributes.get("rot"),
                                        node.attributes.get("scale")))
                    except ValueError as error:
                        raise ValueError("Invalid dynamic object transform: " + str(error)) from error
                    continue
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
    if dynamic_instance_id is not None and selected_dynamic_instance_count != 1:
        raise ValueError("Selected class and instance must match exactly one placed Moby")
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
        if class_id in native_bind_class_ids:
            continue
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
    geometry_only_placements = 0
    geometry_only_classes = []
    for class_id, matrices in sorted(geometry_only_mobys.items()):
        groups, source_meshes, source_definitions, geometry_hash = reusable_mission_geometries[class_id]
        prototypes.append((groups, matrices, {"kind": "dynamic-object-geometry-only-preview",
                                              "class_id": class_id, "placements": len(matrices),
                                              "source_meshes": [str(path) for path in source_meshes],
                                              "source_definitions": [str(path) for path in source_definitions],
                                              "untextured_geometry_sha256": geometry_hash,
                                              "geometry_identical_across_variants": True,
                                              "material_textures_omitted": True,
                                              "class_axes_and_game_visibility_verified": False}))
        inputs.extend(source_meshes)
        inputs.extend(source_definitions)
        geometry_only_placements += len(matrices)
        geometry_only_classes.append(class_id)
    rendered_moby_placements += geometry_only_placements
    rendered_moby_classes += len(geometry_only_classes)
    bind_pose_classes = set()
    bind_pose_report = {}
    if include_native_bind_previews:
        import prepare_native_skin
        native_libraries = (Path(__file__).resolve().parent.parent / "build/native/Release" /
                            "deadlocked_native_animation_diagnostic.dll",
                            Path(__file__).resolve().parent.parent / "build/native/Release" /
                            "deadlocked_native_model_diagnostic.dll")
        for class_id, matrices in sorted(mobys_by_class.items()):
            if class_id in {prototype.get("class_id") for _, _, prototype in prototypes
                            if prototype.get("kind") in ("dynamic-object-preview",
                                                          "dynamic-object-native-bind-pose-preview")}:
                continue
            if class_id not in moby_core_sources or class_id not in moby_source_definitions:
                continue
            core, definition = moby_core_sources[class_id]
            try:
                material_textures = native_material_textures(definition)
                groups, skin_report = prepare_native_skin.read_mesh(
                    core, material_textures, *native_libraries)
            except (ValueError, OSError) as error:
                if (include_native_subnormal_flush_previews and isinstance(error, ValueError) and
                        str(error) in ("Native skeleton initialization refused",
                                       "Native bind-pose matrix preparation refused")):
                    try:
                        groups, skin_report = prepare_native_skin.read_mesh(
                            core, material_textures, *native_libraries, flush_subnormal_translations=True)
                    except (ValueError, OSError) as flushed_error:
                        omitted_mobys[class_id] = {
                            "reason": "native core bind-pose diagnostic failed: " + str(flushed_error),
                            "placements": len(matrices)}
                        continue
                else:
                    omitted_mobys[class_id] = {
                        "reason": "native core bind-pose diagnostic failed: " + str(error),
                        "placements": len(matrices)}
                    continue
            prototypes.append((groups, matrices, {"kind": "dynamic-object-native-bind-pose-preview",
                                                  "class_id": class_id, "placements": len(matrices),
                                                  "core_source": str(core), "definition_source": str(definition),
                                                  "pose": "native core bind pose; no animation/game frame",
                                                  "game_pose_verified": False,
                                                  "subnormal_flush_assumption":
                                                      skin_report["subnormal_flush_assumption_enabled"]}))
            omitted_mobys.pop(class_id, None)
            inputs.extend((core, definition,
                           *(path for path in material_textures if path is not None)))
            bind_pose_classes.add(class_id)
            bind_pose_report[class_id] = skin_report
            rendered_moby_placements += len(matrices)
            rendered_moby_classes += 1
    unresolved_marker_placements = 0
    unresolved_marker_classes = []
    if include_unresolved_moby_markers:
        marker_geometry = unresolved_marker_geometry()
        for class_id, omission in sorted(omitted_mobys.items()):
            placements_for_class = moby_positions_by_class.get(class_id, [])
            if len(placements_for_class) != omission["placements"]:
                raise ValueError("Unresolved Moby marker count differs from omitted placements")
            matrices = [affine_matrix([1, 0, 0, 0, 0, 1, 0, 0,
                                       0, 0, 1, 0, *placement["position_xyz"], 1])
                        for placement in placements_for_class]
            if not matrices:
                continue
            prototypes.append((marker_geometry, matrices, {
                "kind": "diagnostic-unresolved-moby-position-marker",
                "class_id": class_id,
                "placements": len(matrices),
                "instances": placements_for_class,
                "marker": "fixed-size three-axis locator at the source position",
                "diagnostic_only": True,
                "object_shape_rotation_scale_visibility_assumed": False,
                "omission_reason": omission["reason"],
            }))
            unresolved_marker_placements += len(matrices)
            unresolved_marker_classes.append(class_id)
    if include_sky:
        import prepare_native_sky
        sky_prototypes, sky_inputs, sky_report = prepare_native_sky.sky_prototypes(level)
        prototypes.extend(sky_prototypes)
        inputs.extend(sky_inputs)
    if dynamic_only and not any(row[2]["kind"] in ("dynamic-object-preview",
                                                     "dynamic-object-geometry-only-preview",
                                                     "dynamic-object-native-bind-pose-preview")
                                for row in prototypes):
        raise ValueError("Selected dynamic class has no renderable placements in this mission")
    rendered_dynamic_classes = {
        prototype["class_id"] for _, _, prototype in prototypes
        if prototype.get("kind") in ("dynamic-object-preview",
                                      "dynamic-object-geometry-only-preview",
                                      "dynamic-object-native-bind-pose-preview")}
    for placement in dynamic_placement_inventory:
        class_id = placement["class_id"]
        if not placement["selected_by_preview_filters"]:
            placement["preview_status"] = "excluded_by_preview_filter"
        elif class_id in rendered_dynamic_classes:
            placement["preview_status"] = "rendered"
        else:
            placement["preview_status"] = omitted_mobys.get(class_id, {}).get(
                "reason", "no drawable geometry")
    report = write_package(prototypes, output, inputs)
    report["static_object_placements"] = 0 if dynamic_only else len(ids)
    report["vegetation_placements"] = 0 if dynamic_only else len(shrub_ids)
    report["vegetation_classes"] = 0 if dynamic_only else len(shrubs_by_class)
    report["dynamic_object_preview_placements"] = rendered_moby_placements
    report["dynamic_object_preview_classes"] = rendered_moby_classes
    report["native_bind_pose_preview_classes"] = sorted(bind_pose_classes)
    report["native_bind_pose_diagnostics"] = bind_pose_report
    report["geometry_only_preview_classes"] = geometry_only_classes
    report["geometry_only_preview_placements"] = geometry_only_placements
    report["static_joint_attribute_classes"] = static_joint_attribute_classes
    report["reused_cross_mission_mesh_classes"] = sorted({
        prototype["class_id"] for prototype in report["prototypes"]
        if prototype["kind"] == "dynamic-object-preview" and
        prototype.get("reused_cross_mission_mesh")})
    report["omitted_dynamic_objects"] = dict(sorted(omitted_mobys.items()))
    report["omitted_dynamic_object_placements"] = sum(v["placements"] for v in omitted_mobys.values())
    report["editor_data_inventory"] = editor_data_inventory(combined_nodes)
    report["mission_id"] = mission_id
    report["mission_object_placements"] = sum(1 for node in mission_nodes if node.kind == "Moby")
    report["dynamic_preview_class_filter"] = (sorted(dynamic_class_ids)
                                               if dynamic_class_ids is not None and len(dynamic_class_ids) != 1
                                               else next(iter(dynamic_class_ids))
                                               if dynamic_class_ids is not None else None)
    report["dynamic_preview_instance_filter"] = dynamic_instance_id
    report["dynamic_preview_instance"] = selected_dynamic_instance
    report["dynamic_placement_inventory"] = dynamic_placement_inventory
    report["dynamic_preview_excluded_classes"] = sorted(excluded_dynamic_class_ids)
    report["dynamic_preview_transforms_verified"] = False
    report["dynamic_preview_rotation_order_verified"] = True
    report["dynamic_preview_placements_disabled"] = disabled_dynamic_preview_placements
    report["dynamic_only_class_scene"] = dynamic_only
    report["unresolved_moby_markers_enabled"] = include_unresolved_moby_markers
    report["unresolved_moby_marker_placements"] = unresolved_marker_placements
    report["unresolved_moby_marker_classes"] = unresolved_marker_classes
    report["native_bind_pose_previews_enabled"] = include_native_bind_previews
    report["native_bind_pose_preferred_classes"] = sorted(native_bind_class_ids)
    report["native_subnormal_flush_previews_enabled"] = include_native_subnormal_flush_previews
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
    parser.add_argument("--dynamic-instance", type=int,
                        help="Select one instance ID in a single-class --dynamic-only scene")
    parser.add_argument("--exclude-dynamic-class", type=int, action="append",
                        help="Exclude this class from diagnostic Moby previews; may be repeated")
    parser.add_argument("--dynamic-only", action="store_true",
                        help="Build an isolated preview scene for one selected mission class")
    parser.add_argument("--include-native-bind-previews", action="store_true",
                        help="Include recoverable core.bin models in native bind pose")
    parser.add_argument("--native-bind-class", type=int, action="append",
                        help="Prefer native core bind-pose geometry for this Moby class; may be repeated")
    parser.add_argument("--include-native-subnormal-flush-previews", action="store_true",
                        help="Retry refused core poses with subnormal translations set to zero (explicit diagnostic assumption)")
    parser.add_argument("--include-unresolved-moby-markers", action="store_true",
                        help="Add fixed-size position-only markers for omitted Mobys; no shape, rotation, scale or visibility is implied")
    args = parser.parse_args()
    if args.dynamic_class is not None and not args.include_dynamic_previews:
        parser.error("--dynamic-class requires --include-dynamic-previews")
    if args.dynamic_instance is not None and not args.dynamic_only:
        parser.error("--dynamic-instance requires --dynamic-only")
    try:
        report = prepare_level(args.extracted, args.level, args.output, args.include_sky,
                               args.mission, args.include_dynamic_previews,
                               args.dynamic_class[0] if args.dynamic_class and len(args.dynamic_class) == 1
                               else args.dynamic_class,
                               args.exclude_dynamic_class, args.dynamic_only,
                               args.include_native_bind_previews,
                               args.include_native_subnormal_flush_previews,
                               args.include_unresolved_moby_markers,
                               args.dynamic_instance,
                               args.native_bind_class)
    except (ValueError, KeyError, TypeError, OSError, struct.error, ET.ParseError) as error:
        parser.exit(1, str(error)+"\n")
    print("Prepared %d shared vertices, %d instances, %d batches, %d textures" %
          (report["vertices"], report["instances"], report["batches"], report["textures"]))


if __name__ == "__main__":
    main()
