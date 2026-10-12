"""Link outputs must preserve validated objects and compiler assembly."""
import os
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import link_objects


class LinkOutputTests(unittest.TestCase):
    def setUp(self):
        parent = link_objects.ROOT / "build/tests"
        parent.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(dir=parent)
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.addCleanup(patch.stopall)
        patch.object(link_objects, "ROOT", self.root).start()
        self.obj = self.root / "candidate.o"
        self.obj.write_bytes(b"validated candidate")
        self.asm = self.root / "candidate_fixed.s"
        self.asm.write_text("validated assembly")

    def test_same_directory_object_collision_rejected(self):
        with self.assertRaisesRegex(ValueError, "aliases an input"):
            link_objects.protect_inputs(self.root, [self.obj], [self.obj, self.asm])
        self.assertEqual(self.obj.read_bytes(), b"validated candidate")

    def test_hardlink_alias_rejected(self):
        alias = self.root / "other.o"
        os.link(self.obj, alias)
        with self.assertRaisesRegex(ValueError, "aliases an input"):
            link_objects.protect_inputs(self.root, [alias], [self.obj, self.asm])
        self.assertEqual(self.obj.read_bytes(), b"validated candidate")

    def test_assembly_alias_rejected(self):
        with self.assertRaisesRegex(ValueError, "aliases an input"):
            link_objects.protect_inputs(self.root, [self.asm], [self.obj, self.asm])

    def test_escaping_output_rejected_before_creation(self):
        target = self.root.parent / "escaping-candidate.o"
        self.assertFalse(target.exists())
        with self.assertRaisesRegex(ValueError, "escapes the repository"):
            link_objects.protect_inputs(self.root.parent, [target], [self.obj, self.asm])
        self.assertFalse(target.exists())

    def test_distinct_job_output_accepted_without_creating_files(self):
        target = self.root / "linked/candidate.o"
        link_objects.protect_inputs(target.parent, [target], [self.obj, self.asm])
        self.assertFalse(target.exists())


if __name__ == "__main__":
    unittest.main()
