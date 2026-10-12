#!/usr/bin/env python3
"""Inventory mission Moby meshes without treating previews as game rendering."""

import argparse
import hashlib
import json
from collections import Counter, defaultdict
from pathlib import Path

import asset_text
import prepare_native_level as level
import prepare_native_moby as moby
import prepare_native_scene as terrain_mesh


def mesh_record(path):
    try:
        groups, _ = moby.read_mesh(path, allow_unskinned_static_joints=True)
    except ValueError as error:
        return {"status": "unsupported", "reason": str(error), "path": str(path.resolve())}
    vertices = [vertex[:3] for group in groups.values() for vertex in group]
    bounds = [min(vertex[axis] for vertex in vertices) for axis in range(3)] + [
        max(vertex[axis] for vertex in vertices) for axis in range(3)]
    return {"status": "mesh_supported_for_static_preview", "path": str(path.resolve()),
            "source_bounds_xyz": bounds,
            "axis_extents_xyz": [bounds[axis + 3] - bounds[axis] for axis in range(3)]}


def terrain_bounds(level_directory):
    meshes = sorted((Path(level_directory) / "chunks").glob("*/tfrags/mesh.dae"))
    if not meshes:
        raise ValueError("Level has no exported terrain for placement bounds")
    vertices = []
    texture_directory = Path(level_directory) / "chunks" / "0" / "tfrags"
    for path in meshes:
        groups, _ = terrain_mesh.read_mesh(path, recover_unbound_colors=True,
                                           texture_directory=texture_directory)
        vertices.extend(vertex[:3] for group in groups.values() for vertex in group)
    return [min(vertex[axis] for vertex in vertices) for axis in range(3)] + [
        max(vertex[axis] for vertex in vertices) for axis in range(3)]


def inside_bounds(position, bounds):
    return (isinstance(position, list) and len(position) == 3 and all(
        isinstance(position[axis], (int, float)) and
        bounds[axis] <= position[axis] <= bounds[axis + 3] for axis in range(3)))


def inspect_missions(extracted, level_directory):
    extracted = Path(extracted).resolve()
    level_directory = Path(level_directory).resolve()
    if not level_directory.is_relative_to(extracted):
        raise ValueError("Level must be inside the extraction directory")
    global_meshes, _ = level.exported_class_meshes(
        extracted / "moby_classes", "MobyClass", ".glb", "dynamic object", allow_missing=True)
    reusable_mission_meshes, _ = level.unambiguous_mission_moby_meshes(level_directory)
    world_bounds = terrain_bounds(level_directory)
    mesh_cache = {}
    missions = []
    mission_paths = [path for path in (level_directory / "missions").iterdir()
                     if path.is_dir() and path.name.isdecimal()]
    for mission_path in sorted(mission_paths, key=lambda path: int(path.name)):
        instances = mission_path / "mission.instances"
        if not instances.is_file():
            continue
        local_meshes, _ = level.exported_class_meshes(
            mission_path / "moby_classes", "MobyClass", ".glb", "mission dynamic object",
            allow_missing=True)
        placements = [node for node in asset_text.read(instances) if node.kind == "Moby"]
        counts = Counter(node.attributes.get("class") for node in placements)
        placements_by_class = defaultdict(list)
        inside_counts = Counter(node.attributes.get("class") for node in placements
                                if inside_bounds(node.attributes.get("pos"), world_bounds))
        for node in placements:
            class_id = node.attributes.get("class")
            placements_by_class[class_id].append({
                "instance_id": node.name,
                "position_xyz": node.attributes.get("pos"),
                "rotation_xyz": node.attributes.get("rot"),
                "uniform_scale": node.attributes.get("scale"),
                "inside_terrain_aabb": inside_bounds(node.attributes.get("pos"), world_bounds),
            })
        classes = []
        totals = Counter()
        for class_id, placement_count in sorted(counts.items()):
            mesh = local_meshes.get(class_id, global_meshes.get(class_id))
            reused_cross_mission_mesh = False
            if mesh is None and class_id in reusable_mission_meshes:
                mesh = reusable_mission_meshes[class_id][0]
                reused_cross_mission_mesh = True
            if mesh is None:
                record = {"status": "missing_mesh", "reason": "no exported editor mesh"}
            else:
                if mesh not in mesh_cache:
                    mesh_cache[mesh] = mesh_record(mesh)
                record = dict(mesh_cache[mesh])
                if reused_cross_mission_mesh:
                    record["reused_cross_mission_mesh"] = True
            totals[record["status"]] += placement_count
            classes.append({"class_id": class_id, "placements": placement_count,
                            "inside_terrain_aabb_placements": inside_counts[class_id],
                            "outside_terrain_aabb_placements": placement_count - inside_counts[class_id],
                            "instances": placements_by_class[class_id],
                            **record})
        missions.append({"mission_id": int(mission_path.name), "placements": len(placements),
                         "mesh_supported_placements": totals["mesh_supported_for_static_preview"],
                         "missing_mesh_placements": totals["missing_mesh"],
                         "unsupported_mesh_placements": totals["unsupported"],
                         "classes": classes,
                         "source_sha256": hashlib.sha256(instances.read_bytes()).hexdigest()})
    return {"kind": "dreadzone-mission-moby-mesh-inventory", "format_version": 1,
            "terrain_bounds_xyz": world_bounds,
            "terrain_bounds_are_only_a_coarse_visibility_hint": True,
            "mesh_axes_verified_against_game": False,
            "visibility_and_lod_verified_against_game": False,
            "mission_variants_are_mutually_exclusive": True,
            "missions": missions}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--extracted", required=True, type=Path)
    parser.add_argument("--level", required=True, type=Path)
    parser.add_argument("-o", "--output", required=True, type=Path)
    args = parser.parse_args()
    try:
        report = inspect_missions(args.extracted, args.level)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report, indent=2) + "\n")
    except (ValueError, KeyError, TypeError, OSError) as error:
        parser.exit(1, str(error) + "\n")
    print("Inventoried %d mission variants" % len(report["missions"]))


if __name__ == "__main__":
    main()
