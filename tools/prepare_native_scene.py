#!/usr/bin/env python3
"""Convert local COLLADA terrain exports to the native renderer's scene format.

Only identity-transform Z-up scenes with triangles or convex quads are
supported. This is an asset bridge, not recovered game logic. All generated
assets must stay local. Use --recover-unbound-colors only for Wrench exports
whose single RGBA source is aligned with the position accessor but unbound.
"""

import argparse
import hashlib
import json
import math
import struct
import xml.etree.ElementTree as ET
from collections import defaultdict
from pathlib import Path

NS = {"c": "http://www.collada.org/2005/11/COLLADASchema"}
MAGIC = b"DLHOST01"
HEADER = struct.Struct("<8s4I6f")
VERTEX = struct.Struct("<9f")
BATCH = struct.Struct("<3I")


def local_id(value):
    if not value or not value.startswith("#"):
        raise ValueError("Expected a local COLLADA reference")
    return value[1:]


def required(element, query):
    found = element.find(query, NS)
    if found is None:
        raise ValueError("Missing COLLADA element: " + query)
    return found


def source_records(source):
    array = required(source, "c:float_array")
    values = [float(v) for v in (array.text or "").split()]
    if len(values) != int(array.attrib["count"]) or not all(map(math.isfinite, values)):
        raise ValueError("Invalid float array")
    access = required(source, "c:technique_common/c:accessor")
    if local_id(access.attrib["source"]) != array.attrib["id"]:
        raise ValueError("Accessor uses a different array")
    count = int(access.attrib["count"])
    stride = int(access.attrib.get("stride", "1"))
    offset = int(access.attrib.get("offset", "0"))
    parameters = access.findall("c:param", NS)
    if count < 0 or offset < 0 or stride < len(parameters) or stride < 1:
        raise ValueError("Invalid accessor layout")
    if offset + (count - 1) * stride + len(parameters) > len(values) and count:
        raise ValueError("Accessor exceeds its array")
    records = [tuple(values[offset + i * stride:offset + i * stride + len(parameters)])
               for i in range(count)]
    return records, tuple(p.attrib.get("name", "") for p in parameters)


def texture_bindings(root, directory, texture_directory=None):
    texture_directory = Path(texture_directory) if texture_directory is not None else directory
    images = {i.attrib["id"]: required(i, "c:init_from").text
              for i in root.findall("c:library_images/c:image", NS)}
    effects = {}
    for effect in root.findall("c:library_effects/c:effect", NS):
        profile = required(effect, "c:profile_COMMON")
        if profile.find("c:technique/c:lambert", NS) is None:
            raise ValueError("Only Lambert material exports are currently supported")
        texture = profile.find("c:technique/c:lambert/c:diffuse/c:texture", NS)
        if texture is None:
            effects[effect.attrib["id"]] = None
            continue
        parameters = {p.attrib["sid"]: p for p in profile.findall("c:newparam", NS)}
        sampler = parameters[texture.attrib["texture"]]
        surface_name = required(sampler, "c:sampler2D/c:source").text
        image_name = required(parameters[surface_name], "c:surface/c:init_from").text
        path = (texture_directory / images[image_name]).resolve()
        if not path.is_relative_to(texture_directory.resolve()) or path.suffix.lower() != ".png":
            raise ValueError("Texture must be a PNG inside the mesh directory")
        if not path.is_file():
            raise ValueError("Missing texture: " + str(path))
        effects[effect.attrib["id"]] = path
    return {m.attrib["id"]: effects[local_id(required(m, "c:instance_effect").attrib["url"])]
            for m in root.findall("c:library_materials/c:material", NS)}


def read_mesh(path, recover_unbound_colors=False, texture_directory=None):
    path = Path(path)
    root = ET.parse(path).getroot()
    if root.findtext("c:asset/c:up_axis", "Y_UP", NS) != "Z_UP":
        raise ValueError("Only Z-up exports are currently supported")
    unit = root.find("c:asset/c:unit", NS)
    if unit is not None and float(unit.attrib.get("meter", "1")) != 1:
        raise ValueError("Non-unit coordinate scales are unsupported")
    materials = texture_bindings(root, path.parent, texture_directory)
    geometry = {g.attrib["id"]: g for g in root.findall("c:library_geometries/c:geometry", NS)}
    if len(root.findall("c:library_visual_scenes/c:visual_scene", NS)) != 1:
        raise ValueError("Expected exactly one visual scene")
    groups = defaultdict(list)
    recovered_colors = 0
    for node in root.findall("c:library_visual_scenes/c:visual_scene//c:node", NS):
        if node.find("c:instance_controller", NS) is not None or node.find("c:instance_node", NS) is not None:
            raise ValueError("Controllers and instanced-node references are unsupported")
        if any(node.find("c:" + t, NS) is not None
               for t in ("matrix", "translate", "rotate", "scale", "lookat", "skew")):
            raise ValueError("Node transforms must be baked before conversion")
        for instance in node.findall("c:instance_geometry", NS):
            mesh = required(geometry[local_id(instance.attrib["url"])], "c:mesh")
            sources = {s.attrib["id"]: source_records(s) for s in mesh.findall("c:source", NS)}
            vertices = {v.attrib["id"]: v for v in mesh.findall("c:vertices", NS)}
            symbols = {b.attrib["symbol"]: materials[local_id(b.attrib["target"])]
                       for b in instance.findall("c:bind_material/c:technique_common/c:instance_material", NS)}
            for primitive in mesh:
                kind = primitive.tag.rsplit("}", 1)[-1]
                if kind in ("source", "vertices", "extra"):
                    continue
                if kind not in ("triangles", "polylist"):
                    raise ValueError("Unsupported primitive: " + kind)
                inputs = primitive.findall("c:input", NS)
                if not inputs:
                    raise ValueError("Primitive has no inputs")
                width = max(int(i.attrib["offset"]) for i in inputs) + 1
                attributes = {}
                for inp in inputs:
                    semantic = inp.attrib["semantic"]
                    slot = int(inp.attrib["offset"])
                    if slot < 0 or width > 32:
                        raise ValueError("Invalid primitive index layout")
                    ref = local_id(inp.attrib["source"])
                    if semantic == "VERTEX":
                        for nested in vertices[ref].findall("c:input", NS):
                            attributes[nested.attrib["semantic"]] = (sources[local_id(nested.attrib["source"])][0], slot)
                    elif semantic in ("TEXCOORD", "COLOR"):
                        if int(inp.attrib.get("set", "0")) != 0:
                            raise ValueError("Multiple attribute sets are unsupported")
                        attributes[semantic] = (sources[ref][0], slot)
                if "POSITION" not in attributes:
                    raise ValueError("Primitive has no positions")
                if recover_unbound_colors and "COLOR" not in attributes:
                    candidates = [records for records, names in sources.values()
                                  if names == ("R", "G", "B", "A")]
                    positions, position_slot = attributes["POSITION"]
                    if len(candidates) != 1 or len(candidates[0]) != len(positions):
                        raise ValueError("Cannot recover an unambiguous position-aligned color source")
                    attributes["COLOR"] = candidates[0], position_slot
                    if int(primitive.attrib["count"]) > 0:
                        recovered_colors += 1
                indices = [int(v) for v in primitive.findtext("c:p", "", NS).split()]
                count = int(primitive.attrib["count"])
                sizes = ([3] * count if kind == "triangles" else
                         [int(v) for v in primitive.findtext("c:vcount", "", NS).split()])
                if len(sizes) != count or any(s not in (3, 4) for s in sizes) or len(indices) != sum(sizes) * width:
                    raise ValueError("Primitive counts do not match index data")
                target = symbols.get(primitive.attrib.get("material"))
                if primitive.attrib.get("material") and primitive.attrib["material"] not in symbols:
                    raise ValueError("Unbound primitive material")

                def vertex(cursor):
                    values = {}
                    for semantic, (records, slot) in attributes.items():
                        index = indices[cursor + slot]
                        if index < 0 or index >= len(records):
                            raise ValueError("Vertex index exceeds its accessor")
                        values[semantic] = records[index]
                    pos = values["POSITION"]
                    uv = values.get("TEXCOORD", (0.0, 0.0))
                    color = values.get("COLOR", (1.0, 1.0, 1.0, 1.0))
                    if len(pos) != 3 or len(uv) != 2 or len(color) not in (3, 4):
                        raise ValueError("Unexpected vertex attribute dimensions")
                    if len(color) == 3:
                        color += (1.0,)
                    # COLLADA texture origin is bottom-left; decoded PNG origin is top-left.
                    return pos + (uv[0], 1.0 - uv[1]) + color

                cursor = 0
                for size in sizes:
                    polygon = [vertex(cursor + i * width) for i in range(size)]
                    for i in range(1, size - 1):
                        groups[target].extend((polygon[0], polygon[i], polygon[i + 1]))
                    cursor += size * width
    if not any(groups.values()):
        raise ValueError("Mesh contains no triangles")
    return groups, recovered_colors


def write_scene(meshes, output, recover_unbound_colors=False):
    groups = defaultdict(list)
    rebound = 0
    for path in meshes:
        imported, count = read_mesh(path, recover_unbound_colors)
        rebound += count
        for texture, vertices in imported.items():
            groups[texture].extend(vertices)
    groups = {t: v for t, v in groups.items() if v}
    textures = sorted((t for t in groups if t is not None), key=str)
    texture_ids = {t: i + 1 for i, t in enumerate(textures)}
    vertices = []
    batches = []
    for texture, group in groups.items():
        batches.append((len(vertices), len(group), texture_ids.get(texture, 0)))
        vertices.extend(group)
    bounds = [min(v[i] for v in vertices) for i in range(3)] + [max(v[i] for v in vertices) for i in range(3)]
    payload = bytearray(HEADER.pack(MAGIC, 1, len(vertices), len(batches), len(textures), *bounds))
    for texture in textures:
        png = texture.read_bytes()
        if not png.startswith(b"\x89PNG\r\n\x1a\n"):
            raise ValueError("Texture is not a PNG: " + str(texture))
        payload.extend(struct.pack("<I", len(png)))
        payload.extend(png)
    for vertex in vertices:
        payload.extend(VERTEX.pack(*vertex))
    for batch in batches:
        payload.extend(BATCH.pack(*batch))
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(payload)
    report = {"kind": "native-terrain-assets", "vertices": len(vertices),
              "triangles": len(vertices) // 3, "batches": len(batches),
              "textures": len(textures), "bounds": bounds,
              "unbound_color_primitives_recovered": rebound,
              "sha256": hashlib.sha256(payload).hexdigest(),
              "source_meshes": [{"path": str(Path(p).resolve()),
                                 "sha256": hashlib.sha256(Path(p).read_bytes()).hexdigest()} for p in meshes],
              "source_textures": [{"path": str(t), "sha256": hashlib.sha256(t.read_bytes()).hexdigest()} for t in textures],
              "contains_game_logic": False}
    output.with_suffix(".json").write_text(json.dumps(report, indent=2) + "\n")
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("meshes", nargs="+", type=Path)
    parser.add_argument("-o", "--output", required=True, type=Path)
    parser.add_argument("--recover-unbound-colors", action="store_true")
    args = parser.parse_args()
    try:
        report = write_scene(args.meshes, args.output, args.recover_unbound_colors)
    except (ValueError, KeyError, OSError, ET.ParseError, struct.error) as error:
        parser.exit(1, str(error) + "\n")
    print("Prepared %d triangles, %d batches, %d textures" %
          (report["triangles"], report["batches"], report["textures"]))


if __name__ == "__main__":
    main()
