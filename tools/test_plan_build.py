"""Incremental planning fixtures use authored bytes, never compiler/retail data."""
from contextlib import redirect_stderr, redirect_stdout
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import plan_build
import record_build


class BuildPlannerTests(unittest.TestCase):
    def setUp(self):
        scratch = record_build.ROOT / "build/tests"
        scratch.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(dir=scratch)
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.addCleanup(patch.stopall)
        patch.object(plan_build, "ROOT", self.root).start()
        patch.object(record_build, "ROOT", self.root).start()
        for name in record_build.PIPELINE:
            self.write(name, "authored pipeline fixture: " + name + "\n")
        self.header = self.write("include/common.h", "typedef int fixture_int;\n")
        self.source = self.write("src/game/00400000.c", "int fixture(void) { return 1; }\n")
        (self.root / "build/obj").mkdir(parents=True)
        (self.root / "build/tmp").mkdir(parents=True)
        self.object = self.root / "build/obj/game_00400000.o"
        self.assembly = self.root / "build/tmp/game_00400000_fixed.s"
        self.make_current()

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
        return path

    def make_current(self, source=None):
        source = source or self.source
        obj = self.root / "build/obj" / plan_build.object_name(source)
        obj.write_bytes(b"authored object fixture")
        assembly = self.root / "build/tmp" / (obj.stem + "_fixed.s")
        assembly.write_text("authored assembly fixture\n", encoding="utf-8")
        inputs = [source, *(self.root / name for name in record_build.PIPELINE),
                  *sorted((self.root / "include").rglob("*.h"))]
        manifest = {
            "source": source.relative_to(self.root).as_posix(),
            "extra_flags": plan_build.source_flags(source),
            "inputs": {path.relative_to(self.root).as_posix(): record_build.sha256(path) for path in inputs},
            "object_sha256": record_build.sha256(obj),
            "assembly_sha256": record_build.sha256(assembly),
        }
        record_build.manifest_path(obj).write_text(json.dumps(manifest), encoding="utf-8")
        return obj

    def edit_manifest(self, mutate):
        path = record_build.manifest_path(self.object)
        manifest = json.loads(path.read_text(encoding="utf-8"))
        mutate(manifest)
        path.write_text(json.dumps(manifest), encoding="utf-8")

    def assert_rebuild(self):
        result = plan_build.plan(incremental=True)
        self.assertEqual(result.sources, ["src/game/00400000.c"])
        self.assertEqual(result.reused, 0)

    def test_current_object_is_reused_with_real_provenance_validation(self):
        with patch.object(record_build, "validate", wraps=record_build.validate) as validate:
            result = plan_build.plan(incremental=True)
        validate.assert_called_once_with(self.object)
        self.assertEqual(result.sources, [])
        self.assertEqual(result.total, 1)
        self.assertEqual(result.reused, 1)

    def test_default_full_build_emits_even_current_sources(self):
        self.assertEqual(plan_build.plan().sources, ["src/game/00400000.c"])

    def test_each_changed_input_requires_rebuild(self):
        for path in (self.source, self.header, self.root / "tools/cc.sh", self.object, self.assembly):
            with self.subTest(path=path.name):
                original = path.read_bytes()
                path.write_bytes(original + b"changed authored fixture")
                self.assert_rebuild()
                path.write_bytes(original)

    def test_missing_object_assembly_or_manifest_requires_rebuild(self):
        for path in (self.object, self.assembly, record_build.manifest_path(self.object)):
            with self.subTest(path=path.name):
                original = path.read_bytes()
                path.unlink()
                self.assert_rebuild()
                path.write_bytes(original)

    def test_added_header_requires_rebuild_despite_old_manifest_hashes(self):
        self.write("include/new_header.h", "#define AUTHORED 1\n")
        # The old validator alone accepts this; the planner must notice the new key.
        record_build.validate(self.object)
        self.assert_rebuild()

    def test_removed_header_requires_rebuild(self):
        self.header.unlink()
        self.assert_rebuild()

    def test_missing_manifest_input_requires_rebuild(self):
        self.edit_manifest(lambda m: m["inputs"].pop("include/common.h"))
        self.assert_rebuild()

    def test_wrong_source_binding_requires_rebuild(self):
        self.edit_manifest(lambda m: m.update(source="src/game/another.c"))
        self.assert_rebuild()

    def test_different_recorded_flags_require_rebuild(self):
        self.edit_manifest(lambda m: m.update(extra_flags=["-mno-split-addresses"]))
        self.assert_rebuild()

    def test_assembler_flags_are_required_even_when_hashes_are_current(self):
        self.source.write_text("/* asflags: --float-constant=v0 */\n", encoding="utf-8")
        self.make_current()
        self.edit_manifest(lambda m: m.update(extra_flags=[]))
        self.assert_rebuild()

    def test_cflags_and_asflags_first_three_lines_are_recorded_in_order(self):
        self.source.write_text("/* cflags: -mno-split-addresses */\n"
                               "/* asflags: --float-constant=v0 */\n"
                               "/* cflags: -fno-inline */\n"
                               "/* cflags: ignored-fourth-line */\n", encoding="utf-8")
        self.assertEqual(plan_build.source_flags(self.source),
                         ["-mno-split-addresses", "-fno-inline", "--float-constant=v0"])
        self.assert_rebuild()
        self.make_current()
        self.assertEqual(plan_build.plan(True).sources, [])

    def test_comment_flags_are_literal_and_never_executed(self):
        self.source.write_text("/* cflags: $(touch) `echo` */\n", encoding="utf-8")
        self.assertEqual(plan_build.source_flags(self.source), ["$(touch)", "`echo`"])

    def test_crlf_annotations_and_trailing_text_match_shell_rules(self):
        self.source.write_bytes(b"/* cflags: -ffast-math */\r\n"
                                b"/* asflags: --float-immediates */\r\n"
                                b"/* cflags: -fno-inline */ ignored text\r\n")
        self.assertEqual(plan_build.source_flags(self.source),
                         ["-ffast-math", "--float-immediates"])

    def test_new_source_only_rebuilds_new_object(self):
        self.write("src/core/00100000.c", "int new_fixture(void) { return 2; }\n")
        result = plan_build.plan(True)
        self.assertEqual(result.sources, ["src/core/00100000.c"])
        self.assertEqual(result.reused, 1)

    def test_library_sources_and_library_output_directories_are_preserved(self):
        self.write("src/libgcc/example.c", "authored library source\n")
        self.write("src/libm/example.c", "authored library source\n")
        library = self.write("build/libgcc/example.o", "authored library object\n")
        result = plan_build.plan(True)
        self.assertEqual(result.total, 1)
        self.assertEqual(result.sources, [])
        self.assertTrue(library.is_file())

    def test_deleted_source_removes_only_its_object_and_manifest(self):
        retained = self.write("src/core/00100000.c", "int retained(void) { return 0; }\n")
        retained_obj = self.make_current(retained)
        self.source.unlink()
        result = plan_build.plan(True)
        self.assertEqual(result.removed_objects, 1)
        self.assertEqual(result.sources, [])
        self.assertFalse(self.object.exists())
        self.assertFalse(record_build.manifest_path(self.object).exists())
        self.assertTrue(retained_obj.exists())
        self.assertTrue(self.assembly.exists())

    def test_orphan_without_manifest_is_removed_from_owned_object_directory(self):
        obj = self.write("build/obj/removed.o", "authored orphan\n")
        result = plan_build.plan(True)
        self.assertEqual(result.removed_objects, 1)
        self.assertFalse(obj.exists())

    def test_object_name_matches_shell_and_detects_collisions(self):
        source = self.write("src/overlays/L01/00600000.c", "authored source\n")
        self.assertEqual(plan_build.object_name(source), "overlays_L01_00600000.o")
        self.write("src/overlays_L01_00600000.c", "authored colliding source\n")
        with self.assertRaisesRegex(ValueError, "same object"):
            plan_build.plan(True)

    def test_malformed_manifest_is_stale(self):
        path = record_build.manifest_path(self.object)
        for value in ("{broken", "[]", "null", '{"inputs": []}'):
            with self.subTest(value=value):
                path.write_text(value, encoding="utf-8")
                self.assert_rebuild()

    def test_escaped_manifest_input_is_rejected_before_external_read(self):
        self.edit_manifest(lambda m: m["inputs"].update({"../outside.h": "untrusted hash"}))
        with patch.object(record_build, "validate") as validate:
            with self.assertRaisesRegex(ValueError, "escapes"):
                plan_build.plan(True)
        validate.assert_not_called()
        self.assertTrue(self.object.exists())

    def test_escaped_manifest_source_is_rejected(self):
        self.edit_manifest(lambda m: m.update(source="../outside.c"))
        with self.assertRaisesRegex(ValueError, "escapes"):
            plan_build.plan(True)

    def test_escaped_orphan_path_prevents_all_deletions(self):
        orphan = self.write("build/obj/removed.o", "authored orphan\n")
        manifest = self.write("build/tmp/removed.inputs.json", "{}")
        resolve = Path.resolve

        def escaped(path, *args, **kwargs):
            if path == manifest:
                return self.root.parent / "outside.inputs.json"
            return resolve(path, *args, **kwargs)

        with patch.object(Path, "resolve", escaped):
            with self.assertRaisesRegex(ValueError, "escapes"):
                plan_build.plan(True)
        self.assertTrue(orphan.exists())
        self.assertTrue(manifest.exists())

    def test_output_directories_cannot_redirect_inside_the_workspace(self):
        protected = self.write("src/protected.o", "authored object outside build/obj\n")
        resolve = Path.resolve
        for name in ("obj", "tmp"):
            with self.subTest(directory=name):
                alias = self.root / "build" / name

                def redirected(path, *args, **kwargs):
                    if path == alias:
                        return self.root / "src"
                    return resolve(path, *args, **kwargs)

                with patch.object(Path, "resolve", redirected):
                    with self.assertRaisesRegex(ValueError, "redirected"):
                        plan_build.plan(True)
                self.assertEqual(protected.read_text(encoding="utf-8"),
                                 "authored object outside build/obj\n")
                self.assertTrue(self.object.exists())

    def test_unreadable_source_walk_fails_before_orphan_deletion(self):
        orphan = self.write("build/obj/removed.o", "authored orphan\n")

        def denied(*args, **kwargs):
            kwargs["onerror"](PermissionError("authored unreadable directory"))
            return iter(())

        with patch.object(plan_build.os, "walk", denied):
            with self.assertRaises(PermissionError):
                plan_build.plan(True)
        self.assertTrue(orphan.exists())

    def test_missing_source_root_fails_before_orphan_deletion(self):
        resolve = Path.is_dir

        def missing(path):
            if path == self.root / "src":
                return False
            return resolve(path)

        with patch.object(Path, "is_dir", missing):
            with self.assertRaisesRegex(ValueError, "missing source directory"):
                plan_build.plan(True)
        self.assertTrue(self.object.exists())

    def test_linked_object_outside_workspace_is_rejected(self):
        outside = self.root.parent / (self.root.name + "-outside.o")
        outside.write_bytes(b"outside authored object")
        self.addCleanup(lambda: outside.unlink(missing_ok=True))
        self.object.unlink()
        try:
            self.object.symlink_to(outside)
        except OSError as exc:
            self.skipTest("host cannot create a symbolic link: " + str(exc))
        with self.assertRaisesRegex(ValueError, "escapes"):
            plan_build.plan(True)
        self.assertEqual(outside.read_bytes(), b"outside authored object")

    def test_main_stdout_contains_only_source_paths(self):
        output = io.StringIO()
        diagnostics = io.StringIO()
        with redirect_stdout(output), redirect_stderr(diagnostics):
            result = plan_build.main([])
        self.assertEqual(result, 0)
        self.assertEqual(output.getvalue(), "src/game/00400000.c\n")
        self.assertIn("1 compile", diagnostics.getvalue())

    def test_main_error_is_nonzero_without_stdout(self):
        self.edit_manifest(lambda m: m.update(source="../outside.c"))
        output = io.StringIO()
        diagnostics = io.StringIO()
        with redirect_stdout(output), redirect_stderr(diagnostics):
            result = plan_build.main(["--incremental"])
        self.assertEqual(result, 2)
        self.assertEqual(output.getvalue(), "")
        self.assertIn("escapes", diagnostics.getvalue())


if __name__ == "__main__":
    unittest.main()
