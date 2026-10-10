"""Reproducible source-only native class/geometry differential validation.

From the repository root, with a locally extracted L01 overlay and MSVC/CMake:
  python tools/check_native_class_differential.py --overlay PATH/overlay.elf
The default builds only the six recovered C sources and a small source bridge
inside build/drafts/native-class-differential. --dll PATH reuses that bridge DLL
only with its adjacent validated build-input manifest. Missing or stale receipts
force a clean automatic rebuild. CMake is discovered from PATH or local vswhere;
--cmake PATH overrides discovery. Every run checks 515 synthetic fixtures
against actual local instruction words, plus seven atomic host-domain refusals.
Missing input, unsupported executed instructions, budget exhaustion, malformed
memory, native rejection or a single differing byte causes a nonzero exit.

No retail data, instruction list, disassembly, binary or third-party source is
embedded here. Span metadata is independently verified control-flow extent;
00576650 includes its internal branches beyond the upstream splitter boundary.
This is a bounded integer oracle, not a test of a complete loader or game.
"""
import argparse
import ctypes
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import struct
import subprocess
import sys

import retail
from retail_integer_cpu import IntegerCPUError, IntegerMemory, RetailIntegerCPU

ROOT = Path(__file__).resolve().parent.parent
SPANS = ((0x576EF8, 0x128), (0x576BC0, 0x338), (0x5B5598, 0xD0),
         (0x576AA8, 0x118), (0x576348, 0x308), (0x576650, 0x168))
SOURCE_FILES = tuple("nonmatching/L01_%08X.c" % address for address, _ in SPANS)
BRIDGE = "native/tests/dreadzone_class_differential_bridge.c"
HEADERS = ("native/include/common.h", "native/dreadzone_class_tables.h", "native/dreadzone_geometry.h")
DATA, REMAP, TEXTURES, CLASS = 0x400000, 0x602000, 0x600100, 9
STACK, STACK_SIZE = 0x10000000, 0x1000


class DifferentialError(RuntimeError):
    pass


class HostView(ctypes.Structure):
    _fields_ = [("guest_base", ctypes.c_uint32), ("bytes", ctypes.c_uint32),
                ("storage", ctypes.POINTER(ctypes.c_uint8))]


def read_words(overlay):
    if not overlay.is_file():
        raise DifferentialError(f"Missing locally extracted overlay: {overlay}")
    try:
        from elftools.elf.elffile import ELFFile
        with overlay.open("rb") as stream:
            sections = retail.sections(ELFFile(stream))
        words = {}
        for start, size in SPANS:
            raw = retail.read(sections, start, size)
            if raw is None or len(raw) != size:
                raise DifferentialError(f"Overlay has no complete span {start:08X}:{size:X}")
            words.update(zip(range(start, start + size, 4), struct.unpack("<%dI" % (size // 4), raw)))
        return words
    except DifferentialError:
        raise
    except Exception as error:
        raise DifferentialError(f"Cannot read local overlay ELF: {error}") from error


def cmake_quote(path):
    return '"' + path.as_posix().replace('$', '\\$').replace('"', '\\"') + '"'


def build_inputs():
    return {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest()
            for name in (*SOURCE_FILES, BRIDGE, *HEADERS)}


def build_configuration(generator):
    return {"compiler": "MSVC", "generator": generator, "architecture": "x64",
            "configuration": "Release", "definitions": ["DEADLOCKED_NATIVE"],
            "compile_options": ["/W4", "/WX"]}


def bridge_recipe():
    sources = "\n ".join(cmake_quote(ROOT / name) for name in (*SOURCE_FILES, BRIDGE))
    return (
        "cmake_minimum_required(VERSION 3.16)\nproject(native_class_differential C)\n"
        f"add_library(native_class_differential SHARED\n {sources})\n"
        "if(NOT MSVC)\n message(FATAL_ERROR \"Local MSVC is required\")\nendif()\n"
        f"target_include_directories(native_class_differential PRIVATE {cmake_quote(ROOT / 'native/include')} {cmake_quote(ROOT / 'native')})\n"
        "target_compile_definitions(native_class_differential PRIVATE DEADLOCKED_NATIVE)\n"
        "target_compile_options(native_class_differential PRIVATE /W4 /WX)\n")


def build_manifest_path(dll):
    return dll.with_name(dll.name + ".inputs.json")


def validate_build_manifest(dll, inputs, configuration, recipe_hash):
    if not dll.is_file():
        raise DifferentialError(f"Missing source bridge DLL: {dll}")
    path = build_manifest_path(dll)
    if not path.is_file():
        raise DifferentialError(f"Missing build-input manifest: {path}; rebuild the source bridge")
    try:
        manifest = json.loads(path.read_text(encoding="utf-8"))
    except (ValueError, OSError) as error:
        raise DifferentialError(f"Cannot read build-input manifest: {path}") from error
    if not isinstance(manifest, dict) or manifest.get("kind") != "native-class-differential-build" or manifest.get("version") != 1:
        raise DifferentialError(f"Unsupported build-input manifest: {path}")
    if manifest.get("inputs") != inputs:
        raise DifferentialError("Bridge build inputs differ from current source/header/bridge hashes; rebuild")
    if manifest.get("configuration") != configuration or manifest.get("recipe_sha256") != recipe_hash:
        raise DifferentialError("Bridge build configuration or recipe differs; rebuild")
    if manifest.get("dll_sha256") != hashlib.sha256(dll.read_bytes()).hexdigest():
        raise DifferentialError("Bridge DLL hash differs from its build-input manifest; rebuild")
    return manifest


def write_build_manifest(dll, inputs, configuration, recipe_hash, clean):
    manifest = {"kind": "native-class-differential-build", "version": 1,
                "inputs": inputs, "configuration": configuration,
                "recipe_sha256": recipe_hash, "clean_rebuild": clean,
                "dll_sha256": hashlib.sha256(dll.read_bytes()).hexdigest()}
    path = build_manifest_path(dll)
    temporary = path.with_name(path.name + f".{os.getpid()}.tmp")
    temporary.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    temporary.replace(path)
    return manifest


def find_cmake(override):
    if override:
        return override
    cmake = shutil.which("cmake")
    if cmake:
        return cmake
    installer = Path(os.environ.get("ProgramFiles(x86)", "C:/Program Files (x86)")) / "Microsoft Visual Studio/Installer/vswhere.exe"
    if not installer.is_file():
        raise DifferentialError("CMake/vswhere was not found; supply --cmake PATH or install Visual Studio C++ Build Tools with CMake")
    result = subprocess.run([str(installer), "-latest", "-version", "[17.0,18.0)", "-products", "*",
                             "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                             "-property", "installationPath"], capture_output=True, text=True, check=True)
    installation = result.stdout.strip()
    if not installation:
        raise DifferentialError("Visual Studio 2022 C++ Build Tools were not found by vswhere")
    bundled = Path(installation) / "Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
    if not bundled.is_file():
        raise DifferentialError("Visual Studio's bundled CMake was not found; supply --cmake PATH")
    return str(bundled)


def build_bridge(args, inputs, configuration, recipe):
    if os.name != "nt":
        raise DifferentialError("Automatic bridge compilation requires local Windows MSVC; use --dll for an existing bridge")
    cmake = find_cmake(args.cmake)
    if not args.generator.startswith("Visual Studio "):
        raise DifferentialError("Automatic compilation requires a Visual Studio MSVC generator")
    build = args.build_dir.resolve()
    recipe_dir, output = build / "recipe", build / "build"
    recipe_dir.mkdir(parents=True, exist_ok=True)
    (recipe_dir / "CMakeLists.txt").write_bytes(recipe.encode("utf-8"))
    recipe_hash = hashlib.sha256(recipe.encode("utf-8")).hexdigest()
    dll = output / "Release/native_class_differential.dll"
    try:
        validate_build_manifest(dll, inputs, configuration, recipe_hash)
        clean = False
    except DifferentialError:
        clean = True
    # Invalidate the previous receipt before any compiler action; a failed
    # build must never leave a receipt for partially rebuilt output.
    build_manifest_path(dll).unlink(missing_ok=True)
    compile_command = [str(cmake), "--build", str(output), "--config", "Release"]
    if clean:
        compile_command.append("--clean-first")
    compile_command += ["--", "/nodeReuse:false"]
    for command in ([str(cmake), "-S", str(recipe_dir), "-B", str(output), "-G", args.generator, "-A", "x64"],
                    compile_command):
        subprocess.run(command, check=True, stdout=sys.stderr, stderr=sys.stderr)
    if not dll.is_file():
        raise DifferentialError(f"Compiler did not produce the expected bridge: {dll}")
    if build_inputs() != inputs:
        raise DifferentialError("Source inputs changed during bridge compilation; rerun with stable inputs")
    write_build_manifest(dll, inputs, configuration, recipe_hash, clean)
    return dll


def load_bridge(path):
    if not path.is_file():
        raise DifferentialError(f"Missing source bridge DLL: {path}")
    dll = ctypes.CDLL(str(path.resolve()))
    try:
        abi = dll.dreadzone_class_differential_abi
        abi.argtypes, abi.restype = [], ctypes.c_uint32
        if abi() != 1:
            raise DifferentialError("Unsupported native differential bridge ABI")
        publish = dll.dreadzone_class_differential_publish
        publish.argtypes = [ctypes.POINTER(HostView), ctypes.c_uint32, ctypes.c_uint32,
                            ctypes.c_uint32, ctypes.c_uint32, ctypes.c_int32]
        publish.restype = ctypes.c_int32
        return dll, publish
    except AttributeError as error:
        raise DifferentialError("DLL lacks the source differential bridge exports") from error


def put32(data, offset, value):
    struct.pack_into("<I", data, offset, value & 0xFFFFFFFF)


def fixture(seed, kind):
    """Synthetic serialized pointers and packets; never copied asset content."""
    rng = random.Random(0xC1A55500 + seed)
    table, glob = bytearray(rng.randbytes(0x10000)), bytearray(rng.randbytes(0x200))
    dispatch, data = bytearray(0x1000), bytearray(rng.randbytes(0x10000))
    remap, textures = bytearray(rng.randbytes(256)), bytearray(rng.randbytes(0x1200))
    info = bytearray(rng.randbytes(0x200))
    put32(glob, 0x118, rng.randrange(240)); put32(glob, 0x11C, rng.randrange(128))
    struct.pack_into("<III", dispatch, 0, CLASS, 0x600100, 0x3C6CAC + rng.randrange(-2, 32) * 32)
    data[:0x100] = bytes(0x100)
    data[0x2D], data[0x47], data[8] = rng.randrange(256), 255, rng.randrange(3)
    textured = bool(seed % 2) and kind not in ("headers", "counter")
    headers = 0x800
    count = 1 if kind in ("count1", "counter") else 2 if kind in ("shared", "headers") else rng.randrange(1, 5)
    data[4] = count; put32(data, 0, headers)
    put32(data, 0x1C, 0xA0); put32(data, 0xA0, 2)
    put32(data, 0xA4, 0x10); put32(data, 0xA8, 0x20)
    data[0xC] = 2
    for index in range(2):
        aux = 0x300 + index * 0x40
        put32(data, 0x48 + index * 4, aux)
        data[aux:aux + 0x40] = bytes(0x40)
        data[aux + 0x10] = 3
        put32(data, aux + 0x18, 0x100)
        put32(data, aux + 0x100, 0x20); put32(data, aux + 0x104, 0x30)
    for index in range(0x1200 // 16):
        struct.pack_into("<hhh", textures, index * 16 + 4,
                         rng.choice([0, -1, -32768, 32767, 1, 64, 128, 1024]),
                         rng.choice([0, -1, 1, 2, 256, 32767]), rng.choice([0, 1, 2, -1, 32767]))
    for index in range(count):
        high = 4 if kind in ("shared", "headers", "counter") else rng.choice([0, 1, 3, 4, 7, 8])
        low = high + rng.randrange(5)
        output = DATA + 0xA00 if kind == "shared" else DATA + 0xA00 + index * 0x400
        if kind == "headers":
            output = DATA + 0x7F0 if index == 0 else DATA + 0xB00
            low = high = 4
        if kind == "counter":
            output, low, high = 0x222CF8, 4, 4
        put32(data, headers + index * 16, output - DATA - (low - high) * 16)
        put32(data, headers + index * 16 + 4, (high << 16) | low)
        put32(data, headers + index * 16 + 8, 0x900 + index * 0x10)
        if DATA <= output < DATA + len(data):
            for group in range(0, high, 4):
                put32(data, output - DATA + group * 16 + 0x20, rng.choice([-1, -2, -3, -7, rng.randrange(256)]))
    if kind == "shared":
        put32(data, 0xA20, -2); put32(table, 0x9B20, 0xFFFFFFFF)
    if kind == "headers":
        # The first packet's q2 replaces the next header pointer and count.
        put32(data, 0x810, 0xB00); put32(data, 0x814, 0x00040004); put32(data, 0x818, 0x900)
        remap[0] = 3; put32(data, 0xB20, 0); put32(data, 0xA20, -1)
        struct.pack_into("<Q", table, 0x99A0 + 3 * 24, 0x0004000400400A00)
    if kind == "counter":
        # The real dispatcher and publication increment must reload counter3.
        for index in range(256):
            struct.pack_into("<Q", table, 0x99A0 + index * 24, 3)
    segments = [(0x260000, table), (0x222C00, glob), (0x3C5E00, dispatch), (DATA, data),
                (REMAP, remap), (0x600000, textures), (0x1DFC00, info)]
    if kind == "headers":
        segments.append((REMAP + DATA + 0xB00, bytearray([3])))
    return segments, [DATA, TEXTURES if textured else 0, REMAP, CLASS], textured


def reload_fixture(kind):
    segments, arguments, textured = fixture(1, "count1")
    data = segments[3][1]
    if kind == "aux_count_reload":
        data[0xC], data[8] = 8, 1
        put32(data, 0x48, -8)
    elif kind == "aux_pointer_reload":
        data[0xC], data[8] = 1, 1
        put32(data, 0x318, 0x18); put32(data, 0x31C, 0x20)
        segments.append((0x800618, bytearray(struct.pack("<II", 0, 0x30))))
    elif kind == "list_pointer_reload":
        put32(data, 0x18, -DATA + 2); put32(data, 0x1C, 0x18)
        segments.append((0x800018, bytearray(struct.pack("<III", 0, 0, 0x30))))
        segments.append((0x800000, bytearray(bytes([255]) + bytes(11) + struct.pack("<I", 0x80000000))))
    else:
        raise DifferentialError(f"Unknown reload fixture: {kind}")
    return segments, arguments, textured


def host_backing(segments):
    arrays = [(ctypes.c_uint8 * (len(data) + 2)).from_buffer_copy(b"\xD3" + data + b"\x7C") for _, data in segments]
    views = (HostView * len(segments))(*[
        HostView(base, len(data), ctypes.cast(ctypes.byref(array, 1), ctypes.POINTER(ctypes.c_uint8)))
        for (base, data), array in zip(segments, arrays)])
    return arrays, views


def compare_backing(segments, arrays, memory, label):
    compared = 0
    for (base, data), array in zip(segments, arrays):
        actual = bytes(array)
        if actual[0] != 0xD3 or actual[-1] != 0x7C:
            raise DifferentialError(f"{label}: host guard modified for guest range {base:08X}")
        expected, offset = memory.region(base, len(data))
        wanted = bytes(expected[offset:offset + len(data)])
        if actual[1:-1] != wanted:
            position = next(index for index, (a, b) in enumerate(zip(actual[1:-1], wanted)) if a != b)
            raise DifferentialError(f"{label}: mismatch at guest {base + position:08X}, native={actual[position + 1]:02X}, retail={wanted[position]:02X}")
        compared += len(data)
    return compared


def check_case(words, publish, segments, arguments, label, budget):
    memory = IntegerMemory([*segments, (STACK, bytes(STACK_SIZE))])
    cpu = RetailIntegerCPU(words, memory)
    result = cpu.run(0x576EF8, arguments, gp=0x22597F, sp=STACK + STACK_SIZE, budget=budget)
    arrays, views = host_backing(segments)
    native = publish(views, len(views), *arguments)
    if native != 1:
        raise DifferentialError(f"{label}: native rejected a mapped retail fixture (result {native})")
    compared = compare_backing(segments, arrays, memory, label)
    if 0x576AA8 not in result.calls:
        raise DifferentialError(f"{label}: geometry dispatch was not actually executed")
    return result.steps, compared


def check_refusals(publish):
    modes = ("missing_packet", "missing_table", "missing_config", "bad_alignment", "overlap_guest", "overlap_host", "write_limit")
    for mode in modes:
        segments, arguments, _ = fixture(1, "count1")
        data = segments[3][1]
        if mode == "missing_packet":
            put32(data, 0x800, 0x800000)
        elif mode == "missing_table":
            arguments[1] = 0
            put32(data, 0x804, 0x00040004); put32(data, 0x800, 0xA00); put32(data, 0xA20, 0)
            segments[4][1][0] = 0
            table = segments[0][1]
            segments[0] = (0x260000, table[:0x9900])
            segments.append((0x26A000, table[0xA000:]))
        elif mode == "bad_alignment":
            put32(data, 0x800, 0xA01)
        elif mode == "missing_config":
            arguments[1] = TEXTURES
        elif mode == "write_limit":
            data.extend(bytes(0x40000)); put32(data, 0x804, (12240 << 16) | 12240)
            for offset in range(0, 12240, 4):
                put32(data, 0xA00 + offset * 16 + 0x20, -1)
        arrays, views = host_backing(segments)
        if mode == "missing_config":
            views[6].bytes = 0x20
        elif mode == "overlap_guest":
            views[5].guest_base = views[3].guest_base
        elif mode == "overlap_host":
            views[5].storage = views[3].storage
        before = [bytes(array) for array in arrays]
        native = publish(views, len(views), *arguments)
        if native == 1 or before != [bytes(array) for array in arrays]:
            raise DifferentialError(f"Host-domain refusal {mode}: completed or mutated backing")
    return len(modes)


def verify(args):
    words = read_words(args.overlay)
    inputs, configuration, recipe = build_inputs(), build_configuration(args.generator), bridge_recipe()
    recipe_hash = hashlib.sha256(recipe.encode("utf-8")).hexdigest()
    dll_path = args.dll.resolve() if args.dll else build_bridge(args, inputs, configuration, recipe)
    manifest = validate_build_manifest(dll_path, inputs, configuration, recipe_hash)
    dll, publish = load_bridge(dll_path)  # Keep the loaded library alive during calls.
    stats = {"status": "passed", "cases": 0, "instructions": 0, "bytes_compared": 0,
             "textured": 0, "table": 0, "shared_packet": 0, "packet_headers_alias": 0,
             "counter_reload": 0, "count1_aux2": 0, "reload_regressions": 0}
    categories = ("ordinary", "count1", "shared", "headers", "counter")
    for seed in range(512):
        kind = categories[seed % len(categories)]
        segments, arguments, textured = fixture(seed, kind)
        steps, compared = check_case(words, publish, segments, arguments, f"case {seed} ({kind})", args.budget)
        stats["cases"] += 1; stats["instructions"] += steps; stats["bytes_compared"] += compared
        stats["textured" if textured else "table"] += 1
        key = {"shared": "shared_packet", "headers": "packet_headers_alias", "counter": "counter_reload", "count1": "count1_aux2"}.get(kind)
        if key:
            stats[key] += 1
    for kind in ("aux_count_reload", "aux_pointer_reload", "list_pointer_reload"):
        segments, arguments, textured = reload_fixture(kind)
        steps, compared = check_case(words, publish, segments, arguments, kind, args.budget)
        stats["cases"] += 1; stats["instructions"] += steps; stats["bytes_compared"] += compared
        stats["textured" if textured else "table"] += 1; stats["reload_regressions"] += 1
    stats["atomic_refusals"] = check_refusals(publish)
    # Revalidate after execution so concurrent source/output changes cannot be
    # reported as provenance for the library that was originally loaded.
    validate_build_manifest(dll_path, build_inputs(), configuration, recipe_hash)
    stats["comparison"] = "Every byte of mapped backing and host guards, executing all six genuine retail functions without captured callees"
    stats["host_refusal_domain"] = "Invalid host domains checked separately for refusal without mutation; not successful retail behavior"
    stats["overlay_sha256"] = hashlib.sha256(args.overlay.read_bytes()).hexdigest()
    stats["dll_sha256"] = hashlib.sha256(dll_path.read_bytes()).hexdigest()
    stats["bridge_mode"] = "provided DLL" if args.dll else "compiled locally from sources"
    stats["build_input_manifest"] = manifest
    stats["source_sha256"] = {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest()
                              for name in (*SOURCE_FILES, BRIDGE, *HEADERS, "tools/retail.py",
                                           "tools/retail_integer_cpu.py", "tools/check_native_class_differential.py")}
    return stats


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--overlay", required=True, type=Path, help="Locally extracted NTSC-U L01 overlay.elf")
    parser.add_argument("--dll", type=Path, help="Reuse a source-only bridge DLL instead of compiling")
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build/drafts/native-class-differential")
    parser.add_argument("--cmake", help="Local CMake executable")
    parser.add_argument("--generator", default="Visual Studio 17 2022")
    parser.add_argument("--budget", type=int, default=10000, help="Maximum executed instructions per fixture")
    parser.add_argument("--report", type=Path, help="Optional metadata-only JSON report, overwritten on failure too")
    args = parser.parse_args(argv)
    try:
        if args.budget <= 0:
            raise DifferentialError("Instruction budget must be positive")
        stats = verify(args)
    except (OSError, ValueError, IntegerCPUError, DifferentialError, subprocess.SubprocessError) as error:
        stats = {"status": "failed", "error": str(error)}
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(json.dumps(stats, indent=2) + "\n", encoding="utf-8")
        print(f"Class differential failed: {error}", file=sys.stderr)
        return 1
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(stats, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(stats, indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
