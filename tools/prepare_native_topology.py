#!/usr/bin/env python3
"""Render genuine native packet selection, blend and topology stages.

Requires a local unrelocated class container, explicit joint matrices (or an
explicit identity diagnostic), and local PNG textures. No editor GLB, inferred
joint association, animation or game frame is used. Actor mask and LOD are
explicit inputs; omitting the mask retains the primary-only comparison.
The scene bridge remaps raw (x,y,z) to display (y,x,z), applies class scale/1024
and uses flat, double-sided shading. This is a diagnostic presentation; the
original actor/world/view pipeline and physical PS2 arithmetic remain absent.
"""
import argparse
import ctypes as C
import hashlib
import json
import math
import struct
from collections import defaultdict
from pathlib import Path

from prepare_native_level import IDENTITY, write_package

U8 = C.c_uint8
U32 = C.c_uint32


class Vertex(C.Structure):
    _fields_ = [("matrix", C.c_float * 16), ("position", C.c_float * 4),
                ("packet", U32), ("index", C.c_uint16), ("row", C.c_uint16),
                ("duplicate", U8), ("reserved", U8 * 3), ("original", U8 * 16)]


class State(C.Structure):
    _fields_ = [("matrix", U8 * 4096), ("defined", C.c_uint64),
                ("vertices", Vertex * 512), ("vertex_defined", U8 * 512)]


class Packet(C.Structure):
    _fields_ = [("storage", C.POINTER(U8)), ("bytes", U32)]


class Blend(C.Structure):
    _fields_ = [("packets", C.POINTER(Packet)), ("packet_count", U32),
                ("joints", C.POINTER(U8)), ("joint_count", U32),
                ("state", C.POINTER(State)), ("output", C.POINTER(Vertex)),
                ("capacity", U32), ("count", C.POINTER(U32)),
                ("workspace", C.c_void_p), ("workspace_bytes", U32)]


class TopologyPacket(C.Structure):
    _fields_ = [("vif", C.POINTER(U8)), ("bytes", U32),
                ("first", U32), ("count", U32)]


class Texture(C.Structure):
    _fields_ = [("index", C.c_int32), ("defined", U32)]


class Triangle(C.Structure):
    _fields_ = [("output", U32 * 3), ("packet", U32),
                ("ids", C.c_uint16 * 3), ("st", (C.c_int16 * 2) * 3),
                ("texture", C.c_int32)]


class Topology(C.Structure):
    _fields_ = [("packets", C.POINTER(TopologyPacket)), ("packet_count", U32),
                ("vertices", C.POINTER(Vertex)), ("vertex_count", U32),
                ("texture", C.POINTER(Texture)), ("triangles", C.POINTER(Triangle)),
                ("capacity", U32), ("count", C.POINTER(U32))]


class BangleRange(C.Structure):
    _fields_ = [("first", U32), ("count", U32), ("bit", C.c_int32)]


class BanglePlan(C.Structure):
    _fields_ = [("ranges", BangleRange * 16), ("count", U32), ("packets", U32)]


class BangleRequest(C.Structure):
    _fields_ = [("raw", C.POINTER(U8)), ("bytes", U32),
                ("mask", C.c_uint16), ("low", U8), ("reserved", U8)]


def span(raw, offset, length):
    if length <= 0 or offset < 0 or offset + length > len(raw):
        raise ValueError("Model packet exceeds the local class container")
    return (U8 * length).from_buffer_copy(raw[offset:offset + length])


def prepare(container, matrices, identity, textures, library, output, actor_mask=None, low_detail=False):
    paths = [Path(container).resolve(), Path(library).resolve()]
    raw = paths[0].read_bytes()
    if not 0x30 <= len(raw) <= 32 * 1024 * 1024:
        raise ValueError("Unsupported class container size")
    table = struct.unpack_from("<I", raw)[0]
    packet_count, joint_count = raw[4], raw[8]
    scale = struct.unpack_from("<f", raw, 0x24)[0] / 1024
    if (not 1 <= packet_count <= 256 or not 1 <= joint_count <= 127 or
            table % 16 or table + packet_count * 16 > len(raw) or
            not math.isfinite(scale) or scale <= 0):
        raise ValueError("Invalid unrelocated primary class header")
    dll = C.CDLL(str(paths[1]))
    ranges = [[0, packet_count, -1]]
    if actor_mask is not None:
        if not 0 <= actor_mask <= 0x7FFF:
            raise ValueError("Actor mask must fit the supported positive halfword domain")
        dll.dreadzone_bangle_select.argtypes = [C.POINTER(BangleRequest), C.POINTER(BanglePlan)]
        dll.dreadzone_bangle_select.restype = C.c_int
        raw_storage = (U8 * len(raw)).from_buffer_copy(raw)
        request, plan = BangleRequest(raw_storage, len(raw), actor_mask, low_detail, 0), BanglePlan()
        status = dll.dreadzone_bangle_select(C.byref(request), C.byref(plan))
        if status != 1:
            raise ValueError(f"Native packet selection refused: {status}")
        ranges = [[row.first, row.count, row.bit] for row in plan.ranges[:plan.count]]
    elif low_detail:
        raise ValueError("An explicit LOD choice requires an explicit actor mask")
    selection = [number for first, count, _ in ranges for number in range(first, first + count)]
    packet_count = len(selection)
    if not packet_count:
        raise ValueError("No candidate packets were selected")
    if identity:
        joint_data = struct.pack("<16f", *IDENTITY) * joint_count
    else:
        paths.append(Path(matrices).resolve())
        joint_data = paths[-1].read_bytes()
    if len(joint_data) != joint_count * 64:
        raise ValueError("Explicit joint matrix count differs from the class")
    vertex_storage, vif_storage, blend_packets, topology_packets = [], [], [], []
    total = 0
    for number in selection:
        row = table + number * 16
        vif, qwc, _, vertex_offset = struct.unpack_from("<IHHI", raw, row)
        vertex_bytes, count = raw[row + 12] * 16, raw[row + 15]
        if not 1 <= count <= 127 or vertex_offset % 16 or vif % 4:
            raise ValueError("Invalid selected packet descriptor")
        vertex_storage.append(span(raw, vertex_offset, vertex_bytes))
        vif_storage.append(span(raw, vif, qwc * 16))
        blend_packets.append(Packet(vertex_storage[-1], vertex_bytes))
        topology_packets.append(TopologyPacket(vif_storage[-1], qwc * 16, total, count))
        total += count
    dll.dreadzone_vertex_blend_prepare.argtypes = [C.POINTER(Blend)]
    dll.dreadzone_vertex_blend_prepare.restype = C.c_int
    dll.dreadzone_topology_prepare.argtypes = [C.POINTER(Topology)]
    dll.dreadzone_topology_prepare.restype = C.c_int
    if C.sizeof(Vertex) != 108 or C.sizeof(Triangle) != 40:
        raise ValueError("Unexpected host diagnostic ABI")
    descriptors = (Packet * packet_count)(*blend_packets)
    vif_descriptors = (TopologyPacket * packet_count)(*topology_packets)
    joints = (U8 * len(joint_data)).from_buffer_copy(joint_data)
    state, vertices, count = State(), (Vertex * total)(), U32()
    workspace = (U8 * (C.sizeof(state) + C.sizeof(vertices) + 15))()
    blend = Blend(descriptors, packet_count, joints, joint_count, C.pointer(state),
                  vertices, total, C.pointer(count), C.addressof(workspace), len(workspace))
    status = dll.dreadzone_vertex_blend_prepare(C.byref(blend))
    if status != 1 or count.value != total:
        raise ValueError(f"Native local vertex stage refused: {status}")
    triangles, triangle_count, texture = (Triangle * (packet_count * 1016))(), U32(), Texture()
    topology = Topology(vif_descriptors, packet_count, vertices, total, C.pointer(texture),
                        triangles, len(triangles), C.pointer(triangle_count))
    status = dll.dreadzone_topology_prepare(C.byref(topology))
    if status != 1 or not triangle_count.value:
        raise ValueError(f"Native topology diagnostic refused: {status}")
    textures = [Path(p).resolve() for p in textures]
    groups = defaultdict(list)
    for triangle in triangles[:triangle_count.value]:
        if triangle.texture < 0 or triangle.texture >= len(textures):
            raise ValueError("Native texture identity has no supplied PNG")
        for index, st in zip(triangle.output, triangle.st):
            position = vertices[index].position
            display = tuple(position[i] * scale for i in (1, 0, 2))
            if not all(math.isfinite(v) for v in display):
                raise ValueError("Nonfinite diagnostic position")
            groups[textures[triangle.texture]].append((*display, st[0] / 4096, st[1] / 4096, 1, 1, 1, 1))
    report = write_package([(groups, [IDENTITY], "Original selected native topology diagnostic")], output, paths)
    report.update(kind="native-model-topology-diagnostic", native_blend_status="APPROXIMATE",
                  native_topology_status="DIAGNOSTIC", joint_count=joint_count,
                  packet_count=packet_count, blended_vertices=total,
                  explicit_identity_joints=identity, actor_frame_running=False,
                  game_logic_running=False, active_bangles_selected=actor_mask is not None,
                  explicit_actor_mask=actor_mask, explicit_low_detail=low_detail,
                  candidate_ranges=ranges, original_packet_indices=selection,
                  render_queue_budget_executed=False, lod_distance_executed=False,
                  original_lighting=False, original_world_view=False,
                  presentation_axes=[1, 0, 2], presentation_scale=scale,
                  double_sided_required=True,
                  native_vertices_sha256=hashlib.sha256(bytes(vertices)).hexdigest(),
                  native_triangles_sha256=hashlib.sha256(C.string_at(
                      C.addressof(triangles), triangle_count.value * C.sizeof(Triangle))).hexdigest(),
                  excluded_systems=["animation", "live actor mask", "LOD distance", "render queue budget", "actor/world/view", "clipping",
                                    "original lighting", "texture upload", "DMA", "collision", "gameplay"])
    Path(output).with_suffix(".json").write_text(json.dumps(report, indent=2) + "\n")
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("container", type=Path)
    joints = parser.add_mutually_exclusive_group(required=True)
    joints.add_argument("--joint-matrices", type=Path)
    joints.add_argument("--identity-joints", action="store_true", help="Explicit comparison fixture only")
    parser.add_argument("--texture", type=Path, action="append", required=True, help="PNG by original texture index")
    parser.add_argument("--actor-mask", type=lambda value: int(value, 0), help="Explicit actor half74, e.g. 0x1002 at the fresh reset/OR boundary")
    parser.add_argument("--low-detail", action="store_true", help="Explicit low LOD candidate selection, without distance evaluation")
    parser.add_argument("--library", type=Path, default=Path(__file__).resolve().parent.parent /
                        "build/native/Release/deadlocked_native_model_diagnostic.dll")
    parser.add_argument("-o", "--output", type=Path, required=True)
    args = parser.parse_args()
    try:
        result = prepare(args.container, args.joint_matrices, args.identity_joints,
                         args.texture, args.library, args.output, args.actor_mask, args.low_detail)
    except (ValueError, OSError) as error:
        parser.exit(1, f"Native topology diagnostic failed: {error}\n")
    print(f"Native diagnostic: {result['blended_vertices']} computed vertices, "
          f"{result['prototype_triangles']} triangles; no actor/game frame.")


if __name__ == "__main__":
    main()
