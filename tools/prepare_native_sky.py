#!/usr/bin/env python3
"""Collect local GLB sky shells into a diagnostic scene package.

The package preserves each shell's source mesh and recorded angular velocity.
It does not reproduce shell placement, blending, bloom or animation.
"""

import argparse
import json
from pathlib import Path

import asset_text
import prepare_native_level as level
import prepare_native_moby as moby


def shell_records(asset_path):
    roots = asset_text.read(asset_path)
    skies = [node for node in roots if node.kind == "Sky"]
    if len(skies) != 1:
        raise ValueError("Expected one sky definition")
    containers = [node for node in skies[0].children if node.kind == "shells"]
    if len(containers) != 1:
        raise ValueError("Expected one sky shell collection")
    records = []
    for shell in containers[0].children:
        if shell.kind != "SkyShell":
            continue
        meshes = [node for node in shell.children if node.kind == "Mesh"]
        if len(meshes) != 1:
            raise ValueError("Each sky shell must reference one mesh")
        source = meshes[0].attributes.get("src")
        name = meshes[0].attributes.get("name")
        velocity = shell.attributes.get("angular_velocity")
        if (source != "mesh.glb" or not isinstance(name, str) or
                not isinstance(velocity, list) or len(velocity) != 3):
            raise ValueError("Sky shell is missing its mesh name or angular velocity")
        records.append({"name": name, "source": source, "angular_velocity": velocity})
    if not records or len({record["name"] for record in records}) != len(records):
        raise ValueError("Sky shell names must be nonempty and unique")
    return records


def sky_prototypes(level_directory):
    level_directory = Path(level_directory).resolve()
    sky_directory = (level_directory / "sky").resolve()
    asset_path = sky_directory / "sky.asset"
    glb_path = sky_directory / "mesh.glb"
    if not sky_directory.is_relative_to(level_directory) or not asset_path.is_file() or not glb_path.is_file():
        raise ValueError("Level must contain its local sky definition and GLB")
    records = shell_records(asset_path)
    document, _ = moby.read_glb(glb_path)
    nodes = document.get("nodes", [])
    if len(nodes) != len(records) or len(document.get("meshes", [])) != len(records):
        raise ValueError("Sky shell metadata and GLB mesh counts differ")
    shells = []
    prototypes = []
    inputs = [asset_path, glb_path]
    for mesh_index, record in enumerate(records):
        node = nodes[mesh_index]
        if node.get("mesh") != mesh_index or node.get("name") != record["name"]:
            raise ValueError("Sky shell order or mesh identity differs from its GLB node")
        groups, mesh_report = moby.read_mesh(glb_path, mesh_index, allow_blended_alpha=True)
        prototypes.append((groups, [level.IDENTITY], {"kind": "sky-shell-static-preview",
            "mesh_index": mesh_index, "name": record["name"],
            "angular_velocity": record["angular_velocity"], "coordinate_space": "exported GLB local"}))
        inputs.extend(texture for texture in groups if texture is not None)
        shells.append({"name": record["name"], "mesh_index": mesh_index,
                       "triangles": sum(map(len, groups.values())) // 3,
                       "angular_velocity": record["angular_velocity"],
                       "alpha_modes": sorted({document.get("materials", [])[primitive["material"]].get("alphaMode", "OPAQUE")
                                               for primitive in document["meshes"][mesh_index].get("primitives", [])
                                               if primitive.get("material") is not None})})
    summary = {"kind": "native-sky-static-preview", "shells": shells,
               "world_placement_verified": False, "animation_running": False,
               "alpha_blending_rendered": False, "bloom_rendered": False}
    return prototypes, inputs, summary


def prepare_sky(level_directory, output_directory):
    level_directory = Path(level_directory).resolve()
    prototypes, inputs, summary = sky_prototypes(level_directory)
    output_directory = Path(output_directory).resolve()
    output_directory.mkdir(parents=True, exist_ok=True)
    output_path = output_directory / "dreadzone-sky.dlscene"
    package = level.write_package(prototypes, output_path, inputs)
    summary.update({"scene": str(output_path), "prototype_triangles": package["prototype_triangles"],
                    "textures": package["textures"], "batches": package["batches"],
                    "scene_sha256": package["sha256"]})
    (output_directory / "sky-summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    return summary


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--level", required=True, type=Path)
    parser.add_argument("-o", "--output-directory", required=True, type=Path)
    args = parser.parse_args()
    try:
        report = prepare_sky(args.level, args.output_directory)
    except (ValueError, KeyError, TypeError, OSError) as error:
        parser.exit(1, str(error) + "\n")
    print("Prepared %d static sky shells; world placement, blending and animation remain unsupported." %
          len(report["shells"]))


if __name__ == "__main__":
    main()
