"""Synthetic tests for export text parsing and native object placement."""
import struct
import tempfile
import unittest
from pathlib import Path

import asset_text
import prepare_native_level as level
from test_prepare_native_scene import mesh_text


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
        for index in (0,1):
            directory = self.level / "chunks" / str(index) / "tfrags"
            directory.mkdir(parents=True)
            (directory / "mesh.dae").write_text(mesh_text(recover=True))
        (self.level / "tie_classes").mkdir()
        (self.level / "tie_classes" / "tie_classes.asset").write_text('classes { Reference 7 { asset: "authored" } }')
        self.placements = self.level / "gameplay.instances"
        self.placements.write_text('''Tie 0 { class: 7 matrix: [1 0 0 0 0 1 0 0 0 0 1 0 -2 0 0 1] }
        Tie 1 { class: 7 matrix: [1 0 0 0 0 1 0 0 0 0 1 0 2 0 0 1] }''')

    def test_level_preserves_prototype_sharing_and_matrices(self):
        output = Path(self.temp.name) / "authored.dlscene"
        report = level.prepare_level(self.extracted,self.level,output)
        self.assertEqual(report["instances"],4)
        self.assertEqual(report["vertices"],18)
        self.assertEqual(report["prototype_triangles"],6)
        self.assertEqual(report["submitted_triangles"],8)
        self.assertEqual(report["bounds"],[-2,0,0,3,1,0])
        self.assertFalse(report["contains_game_logic"])
        data = output.read_bytes()
        magic,version,vertices,batches,textures,instances,*bounds=level.HEADER.unpack_from(data)
        self.assertEqual((version,vertices,batches,textures,instances),(2,18,3,0,4))
        start=level.HEADER.size+vertices*36
        self.assertEqual(level.INSTANCE.unpack_from(data,start+2*level.INSTANCE.size)[12],-2)
        self.assertEqual(level.INSTANCE.unpack_from(data,start+3*level.INSTANCE.size)[12],2)
        self.assertEqual(level.BATCH.unpack_from(data,len(data)-level.BATCH.size),(12,6,0,2,2))

    def test_column_major_rotation_scale_and_translation(self):
        matrix=level.affine_matrix([0,2,0,0, -3,0,0,0, 0,0,-1,0, 10,20,30,1])
        self.assertEqual(level.transform_point((1,2,3),matrix),(4,22,27))

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
