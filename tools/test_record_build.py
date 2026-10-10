"""Source provenance must reject objects built from changed inputs."""
import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import record_build


class BuildProvenanceTests(unittest.TestCase):
    def setUp(self):
        parent = record_build.ROOT / "build/tests"
        parent.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(dir=parent)
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.addCleanup(patch.stopall)
        patch.object(record_build, "ROOT", self.root).start()
        (self.root / "build/tmp").mkdir(parents=True)
        self.src = self.root / "source.c"
        self.obj = self.root / "example.o"
        self.asm = self.root / "build/tmp/example_fixed.s"
        self.src.write_text("int example(void) { return 1; }\n")
        self.obj.write_bytes(b"synthetic object")
        self.asm.write_text("synthetic assembly\n")
        self.manifest = {"source": "source.c", "inputs": {"source.c": record_build.sha256(self.src)},
                         "object_sha256": record_build.sha256(self.obj),
                         "assembly_sha256": record_build.sha256(self.asm)}
        record_build.manifest_path(self.obj).write_text(json.dumps(self.manifest))

    def test_unchanged_build_is_accepted(self):
        self.assertEqual(record_build.validate(self.obj), self.manifest)

    def test_changed_source_rejected(self):
        self.src.write_text("int example(void) { return 2; }\n")
        with self.assertRaisesRegex(ValueError, "stale object"):
            record_build.validate(self.obj)

    def test_changed_object_rejected(self):
        self.obj.write_bytes(b"different object")
        with self.assertRaisesRegex(ValueError, "object changed"):
            record_build.validate(self.obj)

    def test_changed_assembly_rejected(self):
        self.asm.write_text("different assembly\n")
        with self.assertRaisesRegex(ValueError, "assembly changed"):
            record_build.validate(self.obj)


if __name__ == "__main__":
    unittest.main()
