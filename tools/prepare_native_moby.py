#!/usr/bin/env python3
"""Import one local Wrench GLB editor mesh for native model inspection.

This is a host asset bridge, not actor rendering, skinning or game behavior.
Select one explicit mesh; do not draw alternative LODs/bangles simultaneously.
Retain exported coordinates and keep generated game assets in ignored storage.
Format reference: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html
"""
import argparse
import json
import math
import struct
from collections import defaultdict
from pathlib import Path

from prepare_native_level import IDENTITY, write_package

COMPONENTS = {5121: ("B", 1), 5123: ("H", 2), 5125: ("I", 4), 5126: ("f", 4)}
WIDTHS = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4}


def index(items, value, description):
    if type(value) is not int or value < 0 or value >= len(items):
        raise ValueError("Invalid " + description + " index")
    return items[value]


def integer(value, description, minimum=0):
    if type(value) is not int or value < minimum:
        raise ValueError("Invalid " + description)
    return value


def read_glb(path):
    data = Path(path).read_bytes()
    if len(data) < 20 or struct.unpack_from("<4sII", data) != (b"glTF", 2, len(data)):
        raise ValueError("Invalid GLB header")
    chunks = []
    offset = 12
    while offset < len(data):
        if offset + 8 > len(data):
            raise ValueError("Truncated GLB chunk header")
        length, kind = struct.unpack_from("<II", data, offset)
        offset += 8
        if length % 4 or offset + length > len(data):
            raise ValueError("Invalid GLB chunk length")
        chunks.append((kind, data[offset:offset + length]))
        offset += length
    if [kind for kind, _ in chunks] != [0x4E4F534A, 0x004E4942]:
        raise ValueError("Expected one JSON chunk followed by one BIN chunk")
    document = json.loads(chunks[0][1])
    buffers = document.get("buffers", [])
    if document.get("asset", {}).get("version") != "2.0" or len(buffers) != 1 or "uri" in buffers[0]:
        raise ValueError("Expected glTF 2.0 with one embedded buffer")
    length = integer(buffers[0].get("byteLength"), "buffer length")
    if not length <= len(chunks[1][1]) <= length + 3:
        raise ValueError("Embedded buffer length differs from GLB chunk")
    if document.get("extensionsRequired") or document.get("skins") or document.get("animations"):
        raise ValueError("Required extensions, skins and animations are unsupported")
    return document, chunks[1][1][:length]


def accessor(document, binary, identifier, expected_type, allowed_components, allow_integer=False):
    record = index(document.get("accessors", []), identifier, "accessor")
    if record.get("type") != expected_type or record.get("componentType") not in allowed_components:
        raise ValueError("Unsupported accessor type/component")
    if record.get("sparse") is not None:
        raise ValueError("Sparse accessors are unsupported")
    view = index(document.get("bufferViews", []), record.get("bufferView"), "buffer view")
    if view.get("buffer") != 0:
        raise ValueError("Accessor must use the embedded buffer")
    code, size = COMPONENTS[record["componentType"]]
    width = WIDTHS[expected_type]
    count = integer(record.get("count"), "accessor count", 1)
    view_offset = integer(view.get("byteOffset", 0), "buffer view offset")
    view_length = integer(view.get("byteLength"), "buffer view length")
    offset = integer(record.get("byteOffset", 0), "accessor offset")
    stride = integer(view.get("byteStride", width * size), "accessor stride", width * size)
    if (stride % size or (view_offset + offset) % size or
            ("byteStride" in view and (stride < 4 or stride > 252 or stride % 4))):
        raise ValueError("Invalid accessor alignment/stride")
    if view_offset + view_length > len(binary) or offset + (count - 1) * stride + width * size > view_length:
        raise ValueError("Accessor exceeds embedded buffer view")
    normalized = record.get("normalized", False)
    if type(normalized) is not bool or (normalized and record["componentType"] not in (5121, 5123)):
        raise ValueError("Invalid normalized accessor")
    if (expected_type != "SCALAR" and record["componentType"] != 5126 and
            not normalized and not allow_integer):
        raise ValueError("Integer vertex attributes must be normalized")
    decoder = struct.Struct("<" + code * width)
    values = [decoder.unpack_from(binary, view_offset + offset + i * stride) for i in range(count)]
    if normalized:
        maximum = (1 << (size * 8)) - 1
        values = [tuple(v / maximum for v in row) for row in values]
    if not all(math.isfinite(v) for row in values for v in row):
        raise ValueError("Nonfinite vertex data")
    return values


def material(document, identifier, directory, allow_blended_alpha=False):
    record = {} if identifier is None else index(document.get("materials", []), identifier, "material")
    alpha_mode = record.get("alphaMode", "OPAQUE")
    if record.get("extensions") or alpha_mode not in (("OPAQUE", "MASK", "BLEND") if allow_blended_alpha else ("OPAQUE", "MASK")):
        raise ValueError("Unsupported material extension or blended alpha")
    pbr = record.get("pbrMetallicRoughness", {})
    color = pbr.get("baseColorFactor", [1, 1, 1, 1])
    if not isinstance(color, list) or len(color) != 4 or not all(math.isfinite(v) and 0 <= v <= 1 for v in color):
        raise ValueError("Invalid material base color")
    texture = None
    if "baseColorTexture" in pbr:
        binding = pbr["baseColorTexture"]
        if binding.get("texCoord", 0) != 0 or binding.get("extensions"):
            raise ValueError("Only untransformed texture coordinate set zero is supported")
        image = index(document.get("textures", []), binding.get("index"), "texture")
        image = index(document.get("images", []), image.get("source"), "image")
        uri = image.get("uri")
        if not isinstance(uri, str):
            raise ValueError("Expected a local PNG image URI")
        texture = (directory / uri).resolve()
        if not texture.is_relative_to(directory.resolve()) or texture.suffix.lower() != ".png" or not texture.is_file():
            raise ValueError("Texture must be a local PNG inside the model directory")
    return texture, tuple(color)


def triangle_indices(indices, mode):
    if mode == 4:
        if len(indices) % 3:
            raise ValueError("Triangle list count is not divisible by three")
        yield from (tuple(indices[i:i + 3]) for i in range(0, len(indices), 3))
    elif mode == 5:
        for i in range(2, len(indices)):
            a, b, c = indices[i - 2:i + 1]
            if i % 2:
                a, b = b, a
            # Degenerate connector indices still advance strip winding parity.
            if len({a, b, c}) == 3:
                yield a, b, c
    else:
        raise ValueError("Only triangles and triangle strips are supported")


def read_mesh(path, mesh_index=0, allow_blended_alpha=False, allow_unskinned_static_joints=False):
    path = Path(path)
    document, binary = read_glb(path)
    mesh = index(document.get("meshes", []), mesh_index, "mesh")
    all_nodes = document.get("nodes", [])
    nodes = [node for node in all_nodes if node.get("mesh") == mesh_index]
    if len(nodes) != 1 or any(key in nodes[0] for key in ("matrix", "translation", "rotation", "scale", "skin", "children")):
        raise ValueError("Selected editor mesh needs one untransformed standalone node")
    selected = next(i for i, node in enumerate(all_nodes) if node is nodes[0])
    if any(selected in node.get("children", []) for node in all_nodes):
        raise ValueError("Parented editor meshes require a hierarchy-aware importer")
    groups = defaultdict(list)
    ignored_static_joint_attributes = False
    for primitive in mesh.get("primitives", []):
        attributes = primitive.get("attributes", {})
        extra = set(attributes) - {"POSITION", "NORMAL", "TEXCOORD_0", "COLOR_0"}
        inert_joint_pair = extra == {"JOINTS_0", "WEIGHTS_0"}
        if (primitive.get("targets") or primitive.get("extensions") or
                (extra and not (allow_unskinned_static_joints and inert_joint_pair))):
            raise ValueError("Unsupported deformation or primitive attribute")
        positions = accessor(document, binary, attributes.get("POSITION"), "VEC3", (5126,))
        if inert_joint_pair:
            joints = accessor(document, binary, attributes["JOINTS_0"], "VEC4", (5121, 5123),
                              allow_integer=True)
            weights = accessor(document, binary, attributes["WEIGHTS_0"], "VEC4", (5126, 5121, 5123))
            if len(joints) != len(positions) or len(weights) != len(positions):
                raise ValueError("Static joint attribute counts differ from positions")
            if any(row != (0, 0, 0, 0) for row in joints) or any(
                    abs(row[0] - 1) > 1e-6 or any(abs(weight) > 1e-6 for weight in row[1:])
                    for row in weights):
                raise ValueError("Static preview only supports inert joint-zero weights")
            ignored_static_joint_attributes = True
        texture, factor = material(document, primitive.get("material"), path.parent, allow_blended_alpha)
        if texture is not None and "TEXCOORD_0" not in attributes:
            raise ValueError("Textured mesh needs UV coordinates")
        uvs = accessor(document, binary, attributes["TEXCOORD_0"], "VEC2", (5126, 5121, 5123)) if "TEXCOORD_0" in attributes else [(0, 0)] * len(positions)
        colors = [(1, 1, 1, 1)] * len(positions)
        if "COLOR_0" in attributes:
            kind = index(document["accessors"], attributes["COLOR_0"], "color accessor")["type"]
            if kind not in ("VEC3", "VEC4"):
                raise ValueError("Colors require three or four components")
            colors = accessor(document, binary, attributes["COLOR_0"], kind, (5126, 5121, 5123))
            colors = [row + (1,) if len(row) == 3 else row for row in colors]
        if len(uvs) != len(positions) or len(colors) != len(positions):
            raise ValueError("Vertex attributes have different counts")
        if "NORMAL" in attributes and len(accessor(document, binary, attributes["NORMAL"], "VEC3", (5126,))) != len(positions):
            raise ValueError("Normal attribute has a different count")
        if "indices" in primitive:
            indices = [row[0] for row in accessor(document, binary, primitive["indices"], "SCALAR", (5121, 5123, 5125))]
            record = document["accessors"][primitive["indices"]]
            if record.get("normalized") or ((1 << (COMPONENTS[record["componentType"]][1] * 8)) - 1) in indices:
                raise ValueError("Normalized or restart indices are invalid")
        else:
            indices = list(range(len(positions)))
        if any(type(i) is not int or i >= len(positions) for i in indices):
            raise ValueError("Index exceeds vertex count")
        for triangle in triangle_indices(indices, primitive.get("mode", 4)):
            for i in triangle:
                color = tuple(min(1, max(0, colors[i][c])) * factor[c] for c in range(4))
                groups[texture].append(positions[i] + uvs[i] + color)
    if not any(groups.values()):
        raise ValueError("Selected mesh contains no drawable triangles")
    return groups, {"kind": "editor-mesh", "mesh_index": mesh_index,
                    "mesh_name": mesh.get("name", ""), "omitted_meshes": len(document["meshes"]) - 1,
                    "placements": 1, "exported_coordinates_preserved": True,
                    "ignored_static_joint_attributes": ignored_static_joint_attributes}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mesh", type=Path)
    parser.add_argument("--mesh-index", type=int, default=0)
    parser.add_argument("-o", "--output", type=Path, required=True)
    args = parser.parse_args()
    groups, description = read_mesh(args.mesh, args.mesh_index)
    report = write_package([(groups, [IDENTITY], description)], args.output, [args.mesh])
    report.update(kind="native-editor-mesh-preview", actor_initialized=False, skinning=False, animations=False)
    args.output.with_suffix(".json").write_text(json.dumps(report, indent=2) + "\n")
    print("Prepared %d triangles, %d textures; editor mesh only, no actor/gameplay." %
          (report["prototype_triangles"], report["textures"]))


if __name__ == "__main__":
    main()
