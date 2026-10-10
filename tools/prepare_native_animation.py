#!/usr/bin/env python3
"""Prepare an original animation0 diagnostic with recovered native stages.

Inputs are local, unrelocated Armor0 and level Ratchet animation definitions.
The normal single-animation path initializes actual skeleton records, decodes
explicit fixed/current/next banks, generates matrices, applies original binding
records and executes selected vertex/topology stages. No retail instructions,
editor skeleton, host quaternion interpolation or invented joint mapping run.
Samples retain exact frame labels and original duration records for the
recovered primary clock attachment. Presentation tick scheduling is a choice,
not a recovered PS2 frequency. All generated assets must remain local.
This does not execute a player wrapper or gameplay frame.
"""
import argparse
import ctypes as C
import hashlib
import json
import math
import struct
import tempfile
from pathlib import Path

from prepare_native_topology import prepare as prepare_mesh

U8, U32 = C.c_uint8, C.c_uint32
POSE_HEADER = struct.Struct("<8sIIIfQ6fQ")
CLOCK_HEADER = struct.Struct("<8sIIIIfIQQIIII")


class Init(C.Structure):
    _fields_ = [("skeleton", C.POINTER(U8)), ("skeleton_bytes", U32),
                ("scratch", C.POINTER(U8)), ("scratch_bytes", U32), ("joints", U32)]


class Decode(C.Structure):
    _fields_ = [("scratch", C.POINTER(U8)), ("bytes", U32), ("fraction", C.c_float),
                ("vu_data", C.POINTER(U8)), ("vu_data_bytes", U32)]


class General(C.Structure):
    _fields_ = [("scratch", C.POINTER(U8)), ("bytes", U32), ("joints", U32)]


class View(C.Structure):
    _fields_ = [("guest_base", U32), ("bytes", U32), ("storage", C.POINTER(U8))]


class Bind(C.Structure):
    _fields_ = [("views", C.POINTER(View)), ("view_count", U32), ("pose_guest", U32),
                ("bind_guest", U32), ("count", C.c_int32),
                ("scratch", C.POINTER(U8)), ("scratch_bytes", U32)]


def word(raw, offset):
    if offset < 0 or offset + 4 > len(raw):
        raise ValueError("Definition field is outside its container")
    return struct.unpack_from("<I", raw, offset)[0]


def region(raw, offset, size):
    if size <= 0 or offset < 0 or offset + size > len(raw):
        raise ValueError("Animation input span is outside its container")
    return raw[offset:offset + size]


def animation_layout(raw, joints):
    if len(raw) < 0x4C or raw[8] != joints or not raw[12]:
        raise ValueError("Animation class does not match the armor joints")
    definition = word(raw, 0x48)
    frames = region(raw, definition + 0x10, 1)[0]
    stream = definition + word(raw, definition + 0x1C)
    header = region(raw, stream, 16)
    if list(header[:11]) != [2, 99, 5, 62, 62, 1, 0, 107, 54, 1, 7] or joints != 107 or frames != 30:
        raise ValueError("Only the verified normal Ratchet animation0 stream is supported")
    fixed_bytes, variable_bytes = header[1] * 16, header[2] * 16
    payload = region(raw, stream, fixed_bytes + variable_bytes * frames)
    return payload[:fixed_bytes], [payload[fixed_bytes + i * variable_bytes:
                                          fixed_bytes + (i + 1) * variable_bytes] for i in range(frames)]


def prove_matrix_independence(fixed, joints):
    """Track actual cache-write dominance, without choosing inherited VU values.

    Three priming groups feed alternating outputs. Descriptor bytes select
    four independent lanes; the fourth lane also selects each paired joint.
    Unknown words may reach ignored scale.w/translation.w only. Refuse any
    stream that needs inherited quaternion/XYZ/parent data for its matrices.
    """
    header = region(fixed, 0, 16)
    descriptor = 16 + header[3] * 16
    opcode = descriptor + header[4] * 8
    counters = {0: -32, 24: -30, 48: -31}
    defined = [False] * 256
    banks, bank, writes, groups = [None, None], 0, {}, 0

    def group():
        nonlocal descriptor, opcode, bank, groups
        indices = region(fixed, descriptor, 8)
        descriptor += 8
        op = region(fixed, opcode, 1)[0]
        opcode += 1
        if op not in counters:
            raise ValueError("Unsupported animation dependency opcode")
        counters[op] += 32
        for i in range(8):
            index = (counters[op] & 127) + 128 + i * 4
            if index >= 256:
                raise ValueError("Animation cache write exceeds VU memory")
            defined[index] = True
        tags = [None if defined[index] else index for index in indices]
        result = banks[1 - bank]
        banks[bank] = (tags, (64 * indices[3], 64 * indices[7]))
        bank = 1 - bank
        groups += 1
        return result

    def store(address, tags):
        if address < 0 or address + 16 > 0x2000 or address % 16:
            raise ValueError("Animation output exceeds local joint records")
        for lane, tag in enumerate(tags):
            writes[address + lane * 4] = tag

    group(); descriptor -= 8
    group(); descriptor -= 8
    group()
    for i in range(header[8]):
        tags, _ = group()
        store(header[6] * 64 + i * 128, tags[:4])
        store(header[6] * 64 + i * 128 + 64, tags[4:])
    for count, offset in ((header[9], 16), (header[10], 32)):
        for _ in range(count):
            tags, pointers = group()
            store(pointers[0] + offset, tags[:4])
            store(pointers[1] + offset, tags[4:])
    unknown = [address for address, tag in writes.items() if tag is not None]
    if any(address % 64 >= 48 for address in writes):
        raise ValueError("Animation decoder modifies skeleton parent metadata")
    if any(address < joints * 64 and address % 64 not in (28, 44) for address in unknown):
        raise ValueError("Visible matrices depend on unknown inherited VU data")
    return {"groups": groups, "unknown_words": sorted(unknown), "unknown_live_matrix_words": [],
            "inherited_VU_values_assumed": False}


def scene_geometry(data):
    if len(data) < 52 or data[:8] != b"DLHOST01":
        raise ValueError("Invalid generated base scene")
    version, count, batches, textures, instances = struct.unpack_from("<5I", data, 8)
    if version != 2 or not count or instances != 1:
        raise ValueError("Animation diagnostic requires one version2 prototype")
    offset = 52
    for _ in range(textures):
        size = word(data, offset)
        offset += 4
        region(data, offset, size)
        offset += size
    geometry = region(data, offset, count * 36)
    if offset + count * 36 + instances * 80 + batches * 20 != len(data):
        raise ValueError("Invalid generated base scene extents")
    return geometry


def fingerprint(data):
    value = 14695981039346656037
    for byte in data:
        value = ((value ^ byte) * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    return value


def prepare(armor_path, animation_path, textures, animation_library, model_library, output, sample_rate=60,
            clock_tick_rate=60, clock_flags=0):
    if not math.isfinite(sample_rate) or not 1 <= sample_rate <= 240:
        raise ValueError("Presentation sample rate must be between 1 and 240")
    if not math.isfinite(clock_tick_rate) or not 1 <= clock_tick_rate <= 240:
        raise ValueError("Presentation clock tick rate must be between 1 and 240")
    if not isinstance(clock_flags, int) or not 0 <= clock_flags <= 255 or clock_flags & 4:
        raise ValueError("Explicit clock flags must select the supported normal primary pose path")
    output = Path(output).resolve()
    if output.suffix.lower() != ".dlscene":
        raise ValueError("Output must use the .dlscene suffix")
    output.parent.mkdir(parents=True, exist_ok=True)
    armor = Path(armor_path).read_bytes()
    raw = Path(animation_path).read_bytes()
    if not 0x30 <= len(armor) <= 32 * 1024 * 1024 or len(raw) > 32 * 1024 * 1024:
        raise ValueError("Unsupported local class container size")
    joints = armor[8]
    skeleton = region(armor, word(armor, 0x18), joints * 16)
    bind_offset = word(armor, 0x14)
    region(armor, bind_offset, joints * 48)
    fixed, frames = animation_layout(raw, joints)
    definition_offset = word(raw, 0x48)
    clock_metadata = region(raw, definition_offset, 0x20 + len(frames) * 4)
    durations = [struct.unpack_from("<H", clock_metadata, 0x20 + i * 4)[0] for i in range(len(frames))]
    if any(not 1 <= duration <= 32767 for duration in durations):
        raise ValueError("Original clock duration is outside the supported finite domain")
    dependency = prove_matrix_independence(fixed, joints)
    dll = C.CDLL(str(Path(animation_library).resolve()))
    for name, request in (("dreadzone_local_pose_initialize", Init), ("dreadzone_variable_pose_decode", Decode),
                          ("dreadzone_general_matrices_prepare", General), ("dreadzone_bind_pose_prepare", Bind)):
        function = getattr(dll, name)
        function.argtypes = [C.POINTER(request)]
        function.restype = C.c_int
    skeleton_storage = (U8 * len(skeleton)).from_buffer_copy(skeleton)
    armor_storage = (U8 * len(armor)).from_buffer_copy(armor)
    views = (View * 1)(View(0x10000000, len(armor), armor_storage))
    # Arbitrary diagnostic bits; the dependency proof establishes that no
    # unknown inherited lane contributes to any visible matrix. Not PS2 state.
    inherited = (U8 * 4096).from_buffer_copy(b"\xA5" * 4096)
    samples, positions, first_scene, first_geometry, base_report = [], [], None, None, None
    with tempfile.TemporaryDirectory(prefix="animation-", dir=output.parent) as directory:
        work = Path(directory).resolve()
        if not work.is_relative_to(output.parent):
            raise ValueError("Temporary animation workspace escaped output directory")
        matrix_path, scene_path = work / "matrices.bin", work / "sample.dlscene"
        for frame in range(len(frames)):
            for fraction in (0.0, 0.5):
                scratch = (U8 * 0x4000)()
                request = Init(skeleton_storage, len(skeleton), scratch, len(scratch), joints)
                if dll.dreadzone_local_pose_initialize(C.byref(request)) != 1:
                    raise ValueError("Native skeleton initialization refused")
                for offset, data in ((0x2000, fixed), (0x3000, frames[frame]), (0x3800, frames[(frame + 1) % len(frames)])):
                    C.memmove(C.addressof(scratch) + offset, data, len(data))
                for name, request in (("dreadzone_variable_pose_decode", Decode(scratch, len(scratch), fraction, inherited, len(inherited))),
                                      ("dreadzone_general_matrices_prepare", General(scratch, len(scratch), joints)),
                                      ("dreadzone_bind_pose_prepare", Bind(views, 1, 0x70000000, 0x10000000 + bind_offset, joints, scratch, len(scratch)))):
                    status = getattr(dll, name)(C.byref(request))
                    if status != 1:
                        raise ValueError(f"Native {name} refused frame{frame}/{fraction}: {status}")
                matrices = bytes(scratch[:joints * 64])
                matrix_path.write_bytes(matrices)
                mesh = prepare_mesh(armor_path, matrix_path, False, textures, model_library, scene_path, 0x1002, False)
                scene = scene_path.read_bytes()
                geometry = scene_geometry(scene)
                if first_scene is None:
                    first_scene, first_geometry, base_report = scene, geometry, mesh
                elif (len(geometry) != len(first_geometry) or
                      mesh["native_triangles_sha256"] != base_report["native_triangles_sha256"] or
                      any(geometry[i + 12:i + 36] != first_geometry[i + 12:i + 36]
                          for i in range(0, len(geometry), 36))):
                    raise ValueError("Animation changed base topology or vertex attributes")
                sample = b"".join(geometry[i:i + 12] for i in range(0, len(geometry), 36))
                positions.append(sample)
                samples.append({"frame": frame, "fraction": fraction,
                                "matrices_sha256": hashlib.sha256(matrices).hexdigest(),
                                "positions_sha256": hashlib.sha256(sample).hexdigest()})
    points = [point for data in positions for point in struct.iter_unpack("<3f", data)]
    bounds = [min(p[i] for p in points) for i in range(3)] + [max(p[i] for p in points) for i in range(3)]
    count = len(first_geometry) // 36
    pose_path = output.with_suffix(".dlposes")
    payload = POSE_HEADER.pack(b"DLPOSE01", 1, len(samples), count, sample_rate, fingerprint(first_geometry), *bounds, 0)
    payload += b"".join(positions)
    if len(payload) > 256 * 1024 * 1024:
        raise ValueError("Animation diagnostic exceeds the native sequence limit")
    output.write_bytes(first_scene)
    pose_path.write_bytes(payload)
    # Bind all decoded pose dimensions, bounds and position bits, plus the
    # scene's vertex ordering/attributes. This is a consistency fingerprint.
    pose_fingerprint = fingerprint(payload[12:24] + payload[32:56] + payload[64:])
    clock_payload = CLOCK_HEADER.pack(b"DLCLK001", 1, len(samples), len(clock_metadata), clock_flags,
                                      clock_tick_rate, 0x3F800000, fingerprint(first_geometry),
                                      pose_fingerprint, 0, durations[0], 0, 0)
    clock_payload += clock_metadata
    clock_payload += b"".join(struct.pack("<f", sample["frame"] + sample["fraction"]) for sample in samples)
    output.with_suffix(".dlclock").write_bytes(clock_payload)
    dependencies = [Path(armor_path), Path(animation_path), Path(animation_library), Path(model_library), *map(Path, textures)]
    report = {"kind": "native-original-animation0-diagnostic", "animation_index": 0,
              "animation_state_label": None, "original_frames": len(frames), "samples": samples,
              "sample_rate": sample_rate, "sample_rate_is_presentation_only": True,
              "clock_tick_rate": clock_tick_rate, "clock_tick_rate_is_presentation_only": True,
              "clock_effective_rate_per_tick": 1, "clock_original_durations": durations,
              "clock_metadata_offset": definition_offset, "clock_metadata_bytes": len(clock_metadata),
              "clock_metadata_sha256": hashlib.sha256(clock_metadata).hexdigest(),
              "clock_sha256": hashlib.sha256(clock_payload).hexdigest(), "clock_flags_explicit": clock_flags,
              "clock_flags_source": "Explicit diagnostic input; reset688FE0 preserves actor55",
              "clock_initial_selection_source": "Constructor668070 calls reset688FE0(player,0,0); later player state is excluded",
              "clock_lookup": "Exact actor44 binary32 label; no interpolation or nearest pose",
              "clock_stages": ["primary589CE0"], "PS2_clock_frequency_proven": False,
              "joint_count": joints, "vertices": count, "triangles": base_report["prototype_triangles"],
              "packet_count": base_report["packet_count"], "explicit_actor_mask": "0x1002",
              "poses_sha256": hashlib.sha256(payload).hexdigest(), "scene_sha256": hashlib.sha256(first_scene).hexdigest(),
              "matrix_dependency_proof": dependency, "bounds": bounds,
              "native_stages": ["589368", "5894F0+160/250", "589950+738/858", "589B60+A40/AA8",
                                "selected vertex blend", "GS FIFO topology"],
              "source_inputs": [{"path": str(p.resolve()), "sha256": hashlib.sha256(p.read_bytes()).hexdigest()} for p in dependencies],
              "game_logic_running": False, "actor_frame_running": False, "arithmetic": "approximate host binary32",
              "excluded_systems": ["player clock wrapper/events/scheduler", "secondary animation blending", "input", "movement", "collision",
                                   "actor/world/view", "original lighting", "DMA", "VU state publication", "gameplay"]}
    output.with_suffix(".json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("armor", type=Path)
    parser.add_argument("animation_class", type=Path)
    parser.add_argument("--texture", type=Path, action="append", required=True)
    libraries = Path(__file__).resolve().parent.parent / "build/native/Release"
    parser.add_argument("--animation-library", type=Path, default=libraries / "deadlocked_native_animation_diagnostic.dll")
    parser.add_argument("--model-library", type=Path, default=libraries / "deadlocked_native_model_diagnostic.dll")
    parser.add_argument("--sample-rate", type=float, default=60)
    parser.add_argument("--clock-tick-rate", type=float, default=60,
                        help="Presentation calls per second; not an asserted PS2 frequency")
    parser.add_argument("--clock-flags", type=lambda x: int(x, 0), default=0,
                        help="Explicit diagnostic incoming actor55; reset688FE0 does not assign it")
    parser.add_argument("-o", "--output", type=Path, required=True)
    args = parser.parse_args()
    try:
        result = prepare(args.armor, args.animation_class, args.texture, args.animation_library,
                         args.model_library, args.output, args.sample_rate, args.clock_tick_rate, args.clock_flags)
    except (ValueError, OSError) as error:
        parser.exit(1, f"Native animation diagnostic failed: {error}\n")
    print(f"Native animation0: {len(result['samples'])} samples, {result['joint_count']} joints, "
          f"{result['triangles']} triangles. Original primary clock metadata included; "
          "presentation ticks only, no player/game frame.")


if __name__ == "__main__":
    main()
