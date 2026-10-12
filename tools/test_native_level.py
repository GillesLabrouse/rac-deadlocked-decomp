"""Synthetic tests for export text parsing and native object placement."""
import math
import struct
import tempfile
import unittest
from pathlib import Path

import asset_text
import prepare_native_level as level
import inspect_native_missions
from test_prepare_native_scene import mesh_text
from test_native_moby import encode as encode_glb, fixture as glb_fixture


class AssetTextTests(unittest.TestCase):
    def test_blocks_attributes_and_arrays(self):
        nodes = asset_text.Parser('''// authored fixture
        settings { label: "a // b" enabled: true samples: [0x10 -2 3.5e-2]
          Child "named" { value: false } }
        classes: [7 8]
        ''').parse()
        self.assertEqual(nodes[0].attributes["label"], "a // b")
        self.assertEqual(nodes[0].attributes["samples"], [16,-2,0.035])
        self.assertFalse(nodes[0].children[0].attributes["value"])
        self.assertEqual(nodes[1].attributes["classes"], [7,8])

    def test_malformed_text_fails(self):
        for text in ('A { x: [1 2', 'A { x: 1 x: 2 }', 'x: 1 x: 2',
                     'A { x: 1e999 }', 'A { x: "unfinished }', 'A { ; }'):
            with self.subTest(text=text), self.assertRaises(ValueError):
                asset_text.Parser(text).parse()


class NativeLevelTests(unittest.TestCase):
    def setUp(self):
        scratch = Path(__file__).resolve().parent.parent / "build" / "tests"
        scratch.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(dir=scratch)
        self.addCleanup(self.temp.cleanup)
        self.extracted = Path(self.temp.name) / "extracted"
        self.level = self.extracted / "levels" / "authored"
        self.model = self.extracted / "tie_classes" / "authored"
        self.model.mkdir(parents=True)
        (self.model / "mesh.dae").write_text(mesh_text())
        (self.model / "definition.asset").write_text('TieClass authored { id: 7 Mesh editor { src: "mesh.dae" } }')
        self.shrub_model = self.extracted / "shrub_classes" / "authored"
        self.shrub_model.mkdir(parents=True)
        glb_document, glb_binary = glb_fixture()
        (self.shrub_model / "mesh.glb").write_bytes(encode_glb(glb_document, glb_binary))
        (self.shrub_model / "definition.asset").write_text(
            'ShrubClass authored { id: 9 ShrubClassCore core { Mesh mesh { src: "mesh.glb" } } }')
        self.moby_model = self.extracted / "moby_classes" / "authored"
        self.moby_model.mkdir(parents=True)
        (self.moby_model / "mesh.glb").write_bytes(encode_glb(glb_document, glb_binary))
        (self.moby_model / "definition.asset").write_text(
            'MobyClass authored { id: 11 Mesh editor { src: "mesh.glb" } }')
        missing_moby_model = self.extracted / "moby_classes" / "unavailable"
        missing_moby_model.mkdir(parents=True)
        (missing_moby_model / "definition.asset").write_text('MobyClass unavailable { id: 12 }')
        for index in (0,1):
            directory = self.level / "chunks" / str(index) / "tfrags"
            directory.mkdir(parents=True)
            (directory / "mesh.dae").write_text(mesh_text(recover=True))
        (self.level / "tie_classes").mkdir()
        (self.level / "tie_classes" / "tie_classes.asset").write_text('classes { Reference 7 { asset: "authored" } }')
        (self.level / "shrub_classes").mkdir()
        (self.level / "shrub_classes" / "shrub_classes.asset").write_text(
            'classes { Reference 9 { asset: "authored" } }')
        (self.level / "moby_classes").mkdir()
        (self.level / "moby_classes" / "moby_classes.asset").write_text(
            'classes { Reference 11 { asset: "authored" } Reference 12 { asset: "unavailable" } }')
        self.placements = self.level / "gameplay.instances"
        self.placements.write_text('''Tie 0 { class: 7 matrix: [1 0 0 0 0 1 0 0 0 0 1 0 -2 0 0 1] }
        Tie 1 { class: 7 matrix: [1 0 0 0 0 1 0 0 0 0 1 0 2 0 0 1] }
        Shrub 0 { class: 9 matrix: [1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1] }
        Moby 0 { class: 11 pos: [3 4 5] rot: [0 0 1.57079632679] scale: 2 occlusion: 1 }
        Moby 1 { class: 12 pos: [6 7 8] rot: [0 0 0] scale: 1 }''')

    def test_level_preserves_prototype_sharing_and_matrices(self):
        output = Path(self.temp.name) / "authored.dlscene"
        report = level.prepare_level(self.extracted,self.level,output,
                                    include_dynamic_previews=True)
        self.assertEqual(report["instances"],6)
        self.assertEqual(report["vertices"],30)
        self.assertEqual(report["prototype_triangles"],10)
        self.assertEqual(report["submitted_triangles"],12)
        self.assertEqual(report["static_object_placements"],2)
        self.assertEqual(report["vegetation_placements"],1)
        self.assertEqual(report["vegetation_classes"],1)
        self.assertEqual(report["dynamic_object_preview_placements"],1)
        self.assertEqual(report["dynamic_object_preview_classes"],1)
        self.assertEqual(report["omitted_dynamic_object_placements"],1)
        self.assertEqual(report["omitted_dynamic_objects"][12],
                         {"placements":1,"reason":"no exported editor mesh"})
        for actual,expected in zip(report["bounds"],[-2,0,0,3,6,5]):
            self.assertAlmostEqual(actual,expected,places=5)
        self.assertFalse(report["contains_game_logic"])
        self.assertTrue(report["dynamic_preview_rotation_order_verified"])
        self.assertFalse(report["dynamic_preview_transforms_verified"])
        data = output.read_bytes()
        magic,version,vertices,batches,textures,instances,*bounds=level.HEADER.unpack_from(data)
        self.assertEqual((version,vertices,batches,textures,instances),(2,30,5,0,6))
        start=level.HEADER.size+vertices*36
        self.assertEqual(level.INSTANCE.unpack_from(data,start+2*level.INSTANCE.size)[12],-2)
        self.assertEqual(level.INSTANCE.unpack_from(data,start+3*level.INSTANCE.size)[12],2)
        self.assertEqual(level.INSTANCE.unpack_from(data,start+4*level.INSTANCE.size)[12],0)
        moby = level.INSTANCE.unpack_from(data,start+5*level.INSTANCE.size)
        self.assertAlmostEqual(moby[0],0,places=5)
        self.assertAlmostEqual(moby[1],2,places=5)
        self.assertEqual(moby[12:15],(3,4,5))
        self.assertEqual(level.BATCH.unpack_from(data,len(data)-level.BATCH.size),(24,6,0,5,1))

    def test_column_major_rotation_scale_and_translation(self):
        matrix=level.affine_matrix([0,2,0,0, -3,0,0,0, 0,0,-1,0, 10,20,30,1])
        self.assertEqual(level.transform_point((1,2,3),matrix),(4,22,27))

    def test_moby_euler_rotation_and_uniform_scale(self):
        matrix=level.moby_matrix([3,4,5],[0,0,1.57079632679],2)
        self.assertAlmostEqual(matrix[0],0,places=5)
        self.assertAlmostEqual(matrix[1],2,places=5)
        self.assertEqual(matrix[12:15],(3,4,5))

    def test_native_material_slots_preserve_missing_diffuse_maps(self):
        definition = self.level / "native-materials.asset"
        texture = self.level / "0.png"
        texture.write_bytes(b"fixture")
        definition.write_text('''MobyClass native { id: 17 materials {
          Material 0 { name: "textured" Texture diffuse { src: "0.png" } }
          Material error { name: "error" }
        } }''')
        self.assertEqual(level.native_material_textures(definition), [texture.resolve(), None])

    def test_native_material_table_without_slots_uses_untextured_slot(self):
        definition = self.level / "empty-native-materials.asset"
        definition.write_text('MobyClass native { id: 18 materials { } }')
        self.assertEqual(level.native_material_textures(definition), [None])

    def test_moby_xyz_order_matches_transposed_native_actor_basis(self):
        x, y, z = 0.31, -0.72, 1.07
        sx, sy, sz = math.sin(x), math.sin(y), math.sin(z)
        cx, cy, cz = math.cos(x), math.cos(y), math.cos(z)
        rx = ((1, 0, 0), (0, cx, -sx), (0, sx, cx))
        ry = ((cy, 0, sy), (0, 1, 0), (-sy, 0, cy))
        rz = ((cz, -sz, 0), (sz, cz, 0), (0, 0, 1))

        def multiply(left, right):
            return tuple(tuple(sum(left[row][k] * right[k][column] for k in range(3))
                               for column in range(3)) for row in range(3))

        rotation = multiply(rz, multiply(ry, rx))
        matrix = level.moby_matrix([0, 0, 0], [x, y, z], 1)
        for row in range(3):
            for column in range(3):
                self.assertAlmostEqual(matrix[4 * row + column], rotation[column][row], places=6)
        point = (2.0, -3.0, 5.0)
        actual = level.transform_point(point, matrix)
        expected = tuple(sum(rotation[row][column] * point[column] for column in range(3))
                         for row in range(3))
        for actual_value, expected_value in zip(actual, expected):
            self.assertAlmostEqual(actual_value, expected_value, places=6)

    def test_selected_mission_imports_its_local_moby_meshes(self):
        mission = self.level / "missions" / "0"
        model = mission / "moby_classes" / "23"
        model.mkdir(parents=True)
        document, binary = glb_fixture()
        (model / "mesh.glb").write_bytes(encode_glb(document, binary))
        (model / "moby23.asset").write_text(
            'MobyClass local { id: 23 Mesh mesh { src: "mesh.glb" } }')
        (mission / "mission.instances").write_text(
            'Moby 2 { class: 23 pos: [9 8 7] rot: [0 0 0] scale: 1 }')
        output = Path(self.temp.name) / "mission.dlscene"
        report = level.prepare_level(self.extracted, self.level, output, mission_id=0,
                                     include_dynamic_previews=True)
        self.assertEqual(report["mission_id"], 0)
        self.assertEqual(report["mission_object_placements"], 1)
        self.assertEqual(report["dynamic_object_preview_placements"], 2)
        self.assertEqual(report["omitted_dynamic_object_placements"], 1)
        self.assertTrue(any(row["path"].endswith("mission.instances")
                            for row in report["source_inputs"]))

    def test_mission_can_use_global_moby_class_outside_level_class_list(self):
        model = self.extracted / "moby_classes" / "unlisted-global"
        model.mkdir(parents=True)
        document, binary = glb_fixture()
        (model / "mesh.glb").write_bytes(encode_glb(document, binary))
        (model / "definition.asset").write_text(
            'MobyClass global { id: 13 Mesh editor { src: "mesh.glb" } }')
        mission = self.level / "missions" / "0"
        (mission / "moby_classes").mkdir(parents=True)
        (mission / "mission.instances").write_text(
            'Moby 3 { class: 13 pos: [0 0 0] rot: [0 0 0] scale: 1 }')
        report = level.prepare_level(self.extracted, self.level,
                                     Path(self.temp.name) / "global-mission-class.dlscene",
                                     mission_id=0, include_dynamic_previews=True)
        self.assertEqual(report["mission_object_placements"], 1)
        self.assertEqual(report["dynamic_object_preview_placements"], 2)
        self.assertEqual(report["omitted_dynamic_objects"][12]["reason"],
                         "no exported editor mesh")

    def test_mission_reports_unexported_moby_class_without_failing_scene(self):
        mission = self.level / "missions" / "0"
        (mission / "moby_classes").mkdir(parents=True)
        (mission / "mission.instances").write_text(
            'Moby 3 { class: 404 pos: [0 0 0] rot: [0 0 0] scale: 1 }')
        report = level.prepare_level(self.extracted, self.level,
                                     Path(self.temp.name) / "missing-mission-class.dlscene",
                                     mission_id=0, include_dynamic_previews=True)
        self.assertEqual(report["omitted_dynamic_objects"][404], {
            "placements": 1, "reason": "no exported class definition"})

    def test_mission_without_local_class_directory_uses_global_classes(self):
        mission = self.level / "missions" / "0"
        mission.mkdir(parents=True)
        (mission / "mission.instances").write_text(
            'Moby 3 { class: 11 pos: [0 0 0] rot: [0 0 0] scale: 1 }')
        report = level.prepare_level(self.extracted, self.level,
                                     Path(self.temp.name) / "global-only-mission.dlscene",
                                     mission_id=0, include_dynamic_previews=True)
        self.assertEqual(report["mission_object_placements"], 1)
        self.assertEqual(report["dynamic_object_preview_placements"], 2)

    def test_mission_reuses_identical_mesh_from_another_variant(self):
        mission = self.level / "missions" / "0"
        mission.mkdir(parents=True)
        (mission / "mission.instances").write_text(
            'Moby 0 { class: 404 pos: [1 2 3] rot: [0 0 0] scale: 1 }')
        source = self.level / "missions" / "1" / "moby_classes" / "404"
        source.mkdir(parents=True)
        document, binary = glb_fixture()
        mesh = source / "mesh.glb"
        mesh.write_bytes(encode_glb(document, binary))
        (source / "moby.asset").write_text(
            'MobyClass other_variant { id: 404 Mesh mesh { src: "mesh.glb" } }')
        output = Path(self.temp.name) / "reused-mission-class.dlscene"
        report = level.prepare_level(self.extracted, self.level, output, mission_id=0,
                                     include_dynamic_previews=True)
        self.assertEqual(report["dynamic_object_preview_placements"], 2)
        self.assertNotIn(404, report["omitted_dynamic_objects"])
        self.assertEqual(report["omitted_dynamic_object_placements"], 1)
        self.assertEqual(report["reused_cross_mission_mesh_classes"], [404])
        self.assertTrue(report["dynamic_preview_rotation_order_verified"])
        self.assertFalse(report["dynamic_preview_transforms_verified"])
        class_row = next(row for row in report["prototypes"]
                         if row.get("class_id") == 404)
        self.assertTrue(class_row["reused_cross_mission_mesh"])
        self.assertIn("missions\\1\\", class_row["mesh_source"])
        self.assertTrue(any(row["path"].endswith("mesh.glb") for row in report["source_inputs"]))

    def test_mission_reuses_only_byte_identical_native_cores(self):
        first = self.level / "missions" / "0" / "moby_classes" / "77"
        second = self.level / "missions" / "1" / "moby_classes" / "77"
        first.mkdir(parents=True)
        second.mkdir(parents=True)
        (first / "core.bin").write_bytes(b"same native core")
        (second / "core.bin").write_bytes(b"same native core")
        for directory in (first, second):
            (directory / "moby.asset").write_text(
                'MobyClass reused { id: 77 Binary core { src: "core.bin" } }')
        reusable = level.unambiguous_mission_moby_cores(self.level)
        self.assertEqual(reusable[77][0], (first / "core.bin").resolve())
        (second / "core.bin").write_bytes(b"different native core")
        self.assertNotIn(77, level.unambiguous_mission_moby_cores(self.level))

    def test_mission_does_not_guess_between_different_variant_meshes(self):
        mission = self.level / "missions" / "0"
        mission.mkdir(parents=True)
        (mission / "mission.instances").write_text(
            'Moby 0 { class: 404 pos: [1 2 3] rot: [0 0 0] scale: 1 }')
        for mission_id, tint in ((1, 0.25), (2, 0.75)):
            source = self.level / "missions" / str(mission_id) / "moby_classes" / "404"
            source.mkdir(parents=True)
            document, binary = glb_fixture()
            document["materials"][0]["pbrMetallicRoughness"]["baseColorFactor"][0] = tint
            (source / "mesh.glb").write_bytes(encode_glb(document, binary))
            (source / "moby.asset").write_text(
                'MobyClass other_variant { id: 404 Mesh mesh { src: "mesh.glb" } }')
        report = level.prepare_level(self.extracted, self.level,
                                     Path(self.temp.name) / "ambiguous-mission-class.dlscene",
                                     mission_id=0, include_dynamic_previews=True)
        self.assertEqual(report["omitted_dynamic_objects"][404], {
            "placements": 1, "reason": "ambiguous cross-mission mesh variants"})

    def test_missing_mission_fails(self):
        with self.assertRaises(ValueError):
            level.prepare_level(self.extracted, self.level,
                                Path(self.temp.name) / "missing-mission.dlscene", mission_id=99)

    def test_default_scene_excludes_unverified_dynamic_previews(self):
        output = Path(self.temp.name) / "static-only.dlscene"
        report = level.prepare_level(self.extracted, self.level, output)
        self.assertEqual(report["dynamic_object_preview_placements"], 0)
        self.assertEqual(report["dynamic_preview_placements_disabled"], 2)
        self.assertFalse(report["dynamic_preview_transforms_verified"])
        self.assertIn("dynamic object mesh previews", report["excluded_systems"])

    def test_dynamic_class_filter_keeps_other_preview_classes_out(self):
        output = Path(self.temp.name) / "class-filter.dlscene"
        report = level.prepare_level(self.extracted, self.level, output,
                                     include_dynamic_previews=True, dynamic_class_id=11)
        self.assertEqual(report["dynamic_object_preview_placements"], 1)
        self.assertEqual(report["dynamic_preview_class_filter"], 11)
        self.assertEqual(report["omitted_dynamic_object_placements"], 1)
        self.assertEqual(report["omitted_dynamic_objects"][12],
                         {"placements": 1, "reason": "excluded by diagnostic class filter"})

    def test_dynamic_only_scene_contains_only_the_selected_class(self):
        mission = self.level / "missions" / "0"
        mission.mkdir(parents=True)
        (mission / "mission.instances").write_text(
            'Moby 0 { class: 11 pos: [1 2 3] rot: [0 0 0] scale: 1 }')
        output = Path(self.temp.name) / "dynamic-only.dlscene"
        report = level.prepare_level(self.extracted, self.level, output, mission_id=0,
                                     include_dynamic_previews=True, dynamic_class_id=11,
                                     dynamic_only=True)
        self.assertTrue(report["dynamic_only_class_scene"])
        self.assertEqual(report["static_object_placements"], 0)
        self.assertEqual(report["vegetation_placements"], 0)
        self.assertEqual(report["dynamic_object_preview_placements"], 2)
        self.assertEqual([row["kind"] for row in report["prototypes"]],
                         ["dynamic-object-preview"])
        self.assertEqual(report["instances"], 2)
        self.assertIn("terrain", report["excluded_systems"])

    def test_dynamic_only_scene_requires_a_single_class_filter(self):
        with self.assertRaisesRegex(ValueError, "require a selected mission and class"):
            level.prepare_level(self.extracted, self.level,
                                Path(self.temp.name) / "dynamic-only-invalid.dlscene",
                                mission_id=0, include_dynamic_previews=True,
                                dynamic_only=True)

    def test_non_occluding_flag_does_not_claim_visibility(self):
        original = self.placements.read_text()
        self.placements.write_text(original.replace(
            "Moby 0 { class: 11 pos: [3 4 5] rot: [0 0 1.57079632679] scale: 2 occlusion: 1 }",
            "Moby 0 { class: 11 pos: [3 4 5] rot: [0 0 1.57079632679] scale: 2 occlusion: 0 }"))
        output = Path(self.temp.name) / "occlusion-filter.dlscene"
        report = level.prepare_level(self.extracted, self.level, output,
                                     include_dynamic_previews=True)
        self.assertEqual(report["dynamic_object_preview_placements"], 1)

    def test_class_filter_can_inspect_non_occluding_mobys(self):
        original = self.placements.read_text()
        self.placements.write_text(original.replace(
            "Moby 0 { class: 11 pos: [3 4 5] rot: [0 0 1.57079632679] scale: 2 occlusion: 1 }",
            "Moby 0 { class: 11 pos: [3 4 5] rot: [0 0 1.57079632679] scale: 2 occlusion: 0 }"))
        output = Path(self.temp.name) / "occlusion-class-diagnostic.dlscene"
        report = level.prepare_level(self.extracted, self.level, output,
                                     include_dynamic_previews=True, dynamic_class_id=11)
        self.assertEqual(report["dynamic_object_preview_placements"], 1)
        self.assertEqual(report["dynamic_object_preview_placements"], 1)

    def test_dynamic_class_filter_accepts_multiple_classes(self):
        output = Path(self.temp.name) / "class-filter-many.dlscene"
        report = level.prepare_level(self.extracted, self.level, output,
                                     include_dynamic_previews=True, dynamic_class_id=[11, 12])
        self.assertEqual(report["dynamic_preview_class_filter"], [11, 12])
        self.assertEqual(report["dynamic_object_preview_placements"], 1)
        self.assertEqual(report["omitted_dynamic_object_placements"], 1)

    def test_explicit_dynamic_class_exclusion_is_reported(self):
        output = Path(self.temp.name) / "class-exclusion.dlscene"
        report = level.prepare_level(self.extracted, self.level, output,
                                     include_dynamic_previews=True,
                                     excluded_dynamic_class_ids=[11])
        self.assertEqual(report["dynamic_object_preview_placements"], 0)
        self.assertEqual(report["dynamic_preview_excluded_classes"], [11])
        self.assertEqual(report["omitted_dynamic_objects"][11], {
            "placements": 1, "reason": "excluded by explicit diagnostic class exclusion"})

    def test_dynamic_class_filter_requires_preview_mode_at_cli(self):
        # The parser enforces this at the command boundary; the library accepts a
        # class filter only when diagnostic previews have explicitly been enabled.
        with self.assertRaises(ValueError):
            level.prepare_level(self.extracted, self.level,
                                Path(self.temp.name) / "invalid-filter.dlscene",
                                dynamic_class_id=11)

    def test_mission_inventory_reports_mesh_axes_without_claiming_game_orientation(self):
        mission = self.level / "missions" / "0"
        (mission / "moby_classes").mkdir(parents=True)
        (mission / "mission.instances").write_text(
            'Moby 3 { class: 11 pos: [0 0 0] rot: [0 0 0] scale: 1 } '
            'Moby 4 { class: 11 pos: [100 0 0] rot: [0 0 0] scale: 1 }')
        report = inspect_native_missions.inspect_missions(self.extracted, self.level)
        self.assertFalse(report["mesh_axes_verified_against_game"])
        self.assertFalse(report["visibility_and_lod_verified_against_game"])
        self.assertTrue(report["terrain_bounds_are_only_a_coarse_visibility_hint"])
        self.assertEqual(report["missions"][0]["mesh_supported_placements"], 2)
        self.assertEqual(report["missions"][0]["classes"][0]["class_id"], 11)
        self.assertEqual(len(report["missions"][0]["classes"][0]["axis_extents_xyz"]), 3)
        self.assertEqual(report["missions"][0]["classes"][0]["inside_terrain_aabb_placements"], 1)
        self.assertEqual(report["missions"][0]["classes"][0]["outside_terrain_aabb_placements"], 1)
        instances = report["missions"][0]["classes"][0]["instances"]
        self.assertEqual(instances[0]["instance_id"], "3")
        self.assertEqual(instances[0]["position_xyz"], [0, 0, 0])
        self.assertEqual(instances[0]["rotation_xyz"], [0, 0, 0])
        self.assertTrue(instances[0]["inside_terrain_aabb"])
        self.assertFalse(instances[1]["inside_terrain_aabb"])

    def test_mission_inventory_marks_identical_cross_variant_mesh_reuse(self):
        mission = self.level / "missions" / "0"
        mission.mkdir(parents=True)
        (mission / "mission.instances").write_text(
            'Moby 0 { class: 404 pos: [1 2 3] rot: [0 0 0] scale: 1 }')
        source = self.level / "missions" / "1" / "moby_classes" / "404"
        source.mkdir(parents=True)
        document, binary = glb_fixture()
        (source / "mesh.glb").write_bytes(encode_glb(document, binary))
        (source / "moby.asset").write_text(
            'MobyClass other_variant { id: 404 Mesh mesh { src: "mesh.glb" } }')
        report = inspect_native_missions.inspect_missions(self.extracted, self.level)
        row = next(row for row in report["missions"][0]["classes"] if row["class_id"] == 404)
        self.assertEqual(row["status"], "mesh_supported_for_static_preview")
        self.assertTrue(row["reused_cross_mission_mesh"])

    def test_invalid_transform_fails(self):
        for matrix in ([1,2], list(level.IDENTITY[:3]) + [1] + list(level.IDENTITY[4:]),
                       [float("nan")] + list(level.IDENTITY[1:])):
            with self.subTest(matrix=matrix), self.assertRaises(ValueError):
                level.affine_matrix(matrix)

    def test_missing_class_fails(self):
        self.placements.write_text('Tie 0 { class: 8 matrix: [1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1] }')
        with self.assertRaises(ValueError):
            level.prepare_level(self.extracted,self.level,Path(self.temp.name)/"out.dlscene")

    def test_duplicate_instance_fails(self):
        self.placements.write_text(self.placements.read_text().replace("Tie 1","Tie 0"))
        with self.assertRaises(ValueError):
            level.prepare_level(self.extracted,self.level,Path(self.temp.name)/"out.dlscene")

    def test_class_mesh_escape_fails(self):
        (self.model / "definition.asset").write_text('TieClass authored { id: 7 Mesh editor { src: "../outside.dae" } }')
        with self.assertRaises(ValueError):
            level.class_meshes(self.extracted/"tie_classes")


if __name__ == "__main__":
    unittest.main()
