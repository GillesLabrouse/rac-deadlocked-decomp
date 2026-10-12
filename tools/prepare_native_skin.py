#!/usr/bin/env python3
"""Build a diagnostic static bind-pose mesh from a native core container."""
import ctypes as C
import hashlib
import struct
from collections import defaultdict
from pathlib import Path

from prepare_native_animation import Bind, General, Init, View, region, word
from prepare_native_topology import prepare as prepare_topology

U8 = C.c_uint8
MAX_CORE_BYTES = 32 * 1024 * 1024


def bind_matrices(core_path, animation_library, output):
    core_path = Path(core_path).resolve()
    raw = core_path.read_bytes()
    if not 0x30 <= len(raw) <= MAX_CORE_BYTES:
        raise ValueError("Unsupported native core container size")
    joints = raw[8]
    if not 1 <= joints <= 127:
        raise ValueError("Native core joint count is outside the supported range")
    skeleton = region(raw, word(raw, 0x18), joints * 16)
    bind_offset = word(raw, 0x14)
    region(raw, bind_offset, joints * 48)
    dll = C.CDLL(str(Path(animation_library).resolve()))
    for name, request in (("dreadzone_local_pose_initialize", Init),
                          ("dreadzone_general_matrices_prepare", General),
                          ("dreadzone_bind_pose_prepare", Bind)):
        function = getattr(dll, name)
        function.argtypes = [C.POINTER(request)]
        function.restype = C.c_int
    scratch = (U8 * 0x4000)()
    skeleton_storage = (U8 * len(skeleton)).from_buffer_copy(skeleton)
    core_storage = (U8 * len(raw)).from_buffer_copy(raw)
    views = (View * 1)(View(0x10000000, len(raw), core_storage))
    init = Init(skeleton_storage, len(skeleton), scratch, len(scratch), joints)
    if dll.dreadzone_local_pose_initialize(C.byref(init)) != 1:
        raise ValueError("Native skeleton initialization refused")
    general = General(scratch, len(scratch), joints)
    if dll.dreadzone_general_matrices_prepare(C.byref(general)) != 1:
        raise ValueError("Native bind-pose matrix preparation refused")
    bind = Bind(views, 1, 0x70000000, 0x10000000 + bind_offset, joints, scratch, len(scratch))
    if dll.dreadzone_bind_pose_prepare(C.byref(bind)) != 1:
        raise ValueError("Native bind-pose application refused")
    matrices = bytes(scratch[:joints * 64])
    Path(output).write_bytes(matrices)
    return matrices, joints


def read_mesh(core_path, textures, animation_library, model_library):
    core_path = Path(core_path).resolve()
    textures = [Path(texture).resolve() for texture in textures]
    import tempfile
    with tempfile.TemporaryDirectory(prefix="native-bind-", dir=core_path.parent) as directory:
        work = Path(directory).resolve()
        matrices_path, scene_path = work / "matrices.bin", work / "model.dlscene"
        matrices, joints = bind_matrices(core_path, animation_library, matrices_path)
        report = prepare_topology(core_path, matrices_path, False, textures,
                                  model_library, scene_path)
        groups = defaultdict(list)
        data = scene_path.read_bytes()
        if len(data) < 52 or data[:8] != b"DLHOST01":
            raise ValueError("Native core topology produced an invalid diagnostic scene")
        version, vertex_count, batch_count, texture_count, instance_count = struct.unpack_from("<5I", data, 8)
        if version != 2 or instance_count != 1:
            raise ValueError("Native core topology scene has unsupported structure")
        offset = 52
        scene_textures = []
        for _ in range(texture_count):
            size = struct.unpack_from("<I", data, offset)[0]
            png = data[offset + 4:offset + 4 + size]
            match = next((path for path in textures if path.read_bytes() == png), None)
            if match is None:
                raise ValueError("Native core topology used a texture outside the supplied class materials")
            scene_textures.append(match)
            offset += 4 + size
        vertex_start = offset
        vertex_bytes = vertex_count * 36
        batches_start = vertex_start + vertex_bytes + instance_count * 80
        if batches_start + batch_count * 20 != len(data):
            raise ValueError("Native core topology scene has invalid extents")
        for index in range(batch_count):
            first, count, texture_id, instance_start, instances = struct.unpack_from("<5I", data, batches_start + index * 20)
            if texture_id > texture_count or instance_start != 0 or instances != 1:
                raise ValueError("Native core topology batch has unsupported material or instances")
            if first + count > vertex_count or count % 3:
                raise ValueError("Native core topology batch exceeds its geometry")
            values = struct.iter_unpack("<9f", data[vertex_start + first * 36:vertex_start + (first + count) * 36])
            groups[scene_textures[texture_id - 1] if texture_id else None].extend(values)
        if sum(len(group) for group in groups.values()) != vertex_count:
            raise ValueError("Native core topology batches do not cover every vertex")
    return groups, {"pose": "native bind pose; no animation selected", "game_pose_verified": False,
                    "joints": joints, "triangles": report["prototype_triangles"],
                    "packets": report["packet_count"], "core_sha256": hashlib.sha256(core_path.read_bytes()).hexdigest(),
                    "native_triangles_sha256": report["native_triangles_sha256"],
                    "matrix_sha256": hashlib.sha256(matrices).hexdigest(),
                    "textures": [str(path) for path in textures]}
