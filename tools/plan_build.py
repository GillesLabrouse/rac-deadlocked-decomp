"""Plan game C builds using content hashes and source-local compiler flags.

By default, emit every game source; --incremental emits only stale sources.
Stdout is a newline-delimited source list for tools/build.sh. Diagnostics and
the summary go to stderr. Orphan game objects and their provenance manifests
are removed only after validating every candidate path inside the build roots.
The library build directories are never touched.
"""
import argparse
from dataclasses import dataclass
import json
import os
from pathlib import Path
import re
import sys

import record_build

ROOT = record_build.ROOT


@dataclass
class BuildPlan:
    sources: list[str]
    total: int
    removed_objects: int

    @property
    def reused(self):
        return self.total - len(self.sources)


def checked_path(path, parent=None):
    """Reject escaped paths before reading or deleting through them."""
    root = ROOT.resolve()
    resolved = path.resolve()
    try:
        resolved.relative_to(root)
        if parent is not None:
            checked_parent = parent.resolve()
            checked_parent.relative_to(root)
            resolved.relative_to(checked_parent)
    except ValueError as exc:
        raise ValueError("path escapes the intended workspace: " + str(path)) from exc
    return path


def checked_build_directory(name):
    """Build roots must have their canonical locations, including inside ROOT."""
    path = ROOT / "build" / name
    expected = ROOT.resolve() / "build" / name
    checked_path(path)
    if path.resolve() != expected:
        raise ValueError("build directory is redirected: " + str(path))
    return path


def files_under(directory, suffix):
    """Walk local files without following directory links outside the workspace."""
    checked_path(directory)
    if not directory.exists():
        return []
    result = []
    def walk_error(error):
        raise error

    for folder, directories, files in os.walk(directory, followlinks=False, onerror=walk_error):
        for name in directories:
            child = checked_path(Path(folder) / name)
            if child.is_symlink():
                raise ValueError("linked source/include directory is unsupported: " + str(child))
        for name in files:
            if name.endswith(suffix):
                path = checked_path(Path(folder) / name)
                relative = path.relative_to(ROOT).as_posix()
                if "\n" in relative or "\r" in relative:
                    raise ValueError("source list cannot represent newline paths: " + relative)
                result.append(path)
    return sorted(result)


def object_name(source):
    relative = source.relative_to(ROOT / "src")
    return relative.with_suffix(".o").as_posix().replace("/", "_")


def source_flags(source):
    """Mirror build.sh's cflags and cc.sh's asflags in the first three lines.

    Both shells split these annotations on whitespace; quotes are literal flag
    characters rather than executable shell syntax. Never evaluate comments.
    """
    with source.open(encoding="utf-8") as stream:
        lines = [stream.readline() for _ in range(3)]
    compiler = []
    assembler = []
    for line in lines:
        match = re.fullmatch(r"/\* cflags: (.*) \*/", line.rstrip("\r\n"))
        if match:
            compiler.extend(match.group(1).split())
    for line in lines:
        match = re.search(r"/\*\sasflags:\s(.*)\s\*/", line)
        if match:
            assembler = match.group(1).split()
            break
    flags = compiler + assembler
    if any(any(char in flag for char in "*?[") for flag in flags):
        raise ValueError("wildcard source flags are unsupported: " + str(source))
    return flags


def read_manifest(path):
    checked_path(path, ROOT / "build/tmp")
    if path.is_symlink():
        raise ValueError("linked build manifest is unsupported: " + str(path))
    try:
        manifest = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return None
    if not isinstance(manifest, dict):
        return None
    inputs = manifest.get("inputs")
    if isinstance(inputs, dict):
        for name in inputs:
            if not isinstance(name, str):
                return None
            checked_path(ROOT / name)
    source = manifest.get("source")
    if isinstance(source, str):
        checked_path(ROOT / source, ROOT / "src")
    return manifest


def stale(source, obj, shared_inputs, flags):
    checked_path(obj, ROOT / "build/obj")
    if obj.is_symlink():
        raise ValueError("linked build object is unsupported: " + str(obj))
    manifest_path = record_build.manifest_path(obj)
    assembly = ROOT / "build/tmp" / (obj.stem + "_fixed.s")
    checked_path(assembly, ROOT / "build/tmp")
    if assembly.is_symlink():
        raise ValueError("linked build assembly is unsupported: " + str(assembly))
    manifest = read_manifest(manifest_path)
    source_name = source.relative_to(ROOT).as_posix()
    if manifest is None or manifest.get("source") != source_name:
        return True
    expected_inputs = set(shared_inputs) | {source_name}
    inputs = manifest.get("inputs")
    if not isinstance(inputs, dict) or set(inputs) != expected_inputs:
        return True
    if manifest.get("extra_flags") != flags:
        return True
    try:
        record_build.validate(obj)
    except (OSError, ValueError, KeyError, TypeError):
        return True
    return False


def orphan_paths(expected_objects):
    """Preflight all removals before permitting any unlink operation."""
    objects = checked_build_directory("obj")
    manifests = checked_build_directory("tmp")
    result = []
    for obj in sorted(objects.glob("*.o")):
        checked_path(obj, objects)
        if obj.is_symlink():
            raise ValueError("linked build object is unsupported: " + str(obj))
        if not obj.is_file():
            raise ValueError("build object is not a regular file: " + str(obj))
        if obj.name in expected_objects:
            continue
        manifest = manifests / (obj.stem + ".inputs.json")
        checked_path(manifest, manifests)
        if manifest.is_symlink():
            raise ValueError("linked orphan manifest is unsupported: " + str(manifest))
        if manifest.exists() and not manifest.is_file():
            raise ValueError("orphan manifest is not a regular file: " + str(manifest))
        result.append((obj, manifest))
    return result


def plan(incremental=False):
    # Neither output root may redirect into sources or another workspace folder.
    checked_build_directory("obj")
    checked_build_directory("tmp")
    if not checked_path(ROOT / "src").is_dir():
        raise ValueError("missing source directory: " + str(ROOT / "src"))
    sources = [source for source in files_under(ROOT / "src", ".c")
               if source.relative_to(ROOT / "src").parts[0] not in ("libgcc", "libm")]
    objects = {}
    for source in sources:
        name = object_name(source)
        if name in objects:
            raise ValueError("sources map to the same object: " + str(source) + " and " + str(objects[name]))
        objects[name] = source
    shared = []
    for name in record_build.PIPELINE:
        path = checked_path(ROOT / name)
        if not path.is_file():
            raise ValueError("missing compiler pipeline input: " + name)
        shared.append(name)
    shared += [path.relative_to(ROOT).as_posix()
               for path in files_under(ROOT / "include", ".h")]
    selected = []
    for source in sources:
        flags = source_flags(source)
        obj = ROOT / "build/obj" / object_name(source)
        needs_build = stale(source, obj, shared, flags)
        if not incremental or needs_build:
            selected.append(source.relative_to(ROOT).as_posix())
    orphans = orphan_paths(objects)
    for obj, manifest in orphans:
        # Check again immediately before each deletion; never delete recursively.
        checked_path(obj, ROOT / "build/obj")
        checked_path(manifest, ROOT / "build/tmp")
        obj.unlink()
        if manifest.exists():
            manifest.unlink()
    return BuildPlan(selected, len(sources), len(orphans))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--incremental", action="store_true",
                        help="emit only sources with missing or stale build provenance")
    args = parser.parse_args(argv)
    try:
        result = plan(args.incremental)
    except (OSError, ValueError, RuntimeError) as exc:
        print("build planner: " + str(exc), file=sys.stderr)
        return 2
    for source in result.sources:
        print(source)
    print("build planner: %d sources, %d compile, %d reused, %d orphan objects removed" %
          (result.total, len(result.sources), result.reused, result.removed_objects), file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
