"""Record source inputs for a compiled object; reject stale objects at link time."""
import hashlib
import json
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PIPELINE = ["tools/cc.sh", "tools/toolchain.sh", "tools/ps2eeas_dli.py",
            "tools/ps2eeas_nops.py", "tools/record_build.py"]


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def scratch_path(directory=None):
    path = (ROOT / (directory or "build/tmp")).resolve()
    if not path.is_relative_to(ROOT):
        raise ValueError("compiler scratch directory must stay inside the repository")
    return path


def manifest_path(obj, scratch_dir=None):
    return scratch_path(scratch_dir) / (obj.stem + ".inputs.json")


def validate(obj, scratch_dir=None):
    manifest = json.loads(manifest_path(obj, scratch_dir).read_text())
    for name, digest in manifest["inputs"].items():
        if sha256(ROOT / name) != digest:
            raise ValueError("stale object; recompile after input changed: " + name)
    if sha256(obj) != manifest["object_sha256"]:
        raise ValueError("compiled object changed; recompile: " + obj.name)
    assembly = scratch_path(scratch_dir) / (obj.stem + "_fixed.s")
    if sha256(assembly) != manifest["assembly_sha256"]:
        raise ValueError("compiler assembly changed; recompile: " + obj.name)
    return manifest


def main():
    src, obj = (ROOT / sys.argv[1]).resolve(), (ROOT / sys.argv[2]).resolve()
    scratch_dir = os.environ.get("DECOMP_TMP_DIR")
    inputs = [src, *(ROOT / name for name in PIPELINE), *sorted((ROOT / "include").rglob("*.h"))]
    manifest = {"source": src.relative_to(ROOT).as_posix(), "extra_flags": sys.argv[3:],
                "inputs": {p.relative_to(ROOT).as_posix(): sha256(p) for p in inputs},
                "object_sha256": sha256(obj),
                "assembly_sha256": sha256(scratch_path(scratch_dir) / (obj.stem + "_fixed.s"))}
    manifest_path(obj, scratch_dir).write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
