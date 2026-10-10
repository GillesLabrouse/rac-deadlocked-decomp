"""Exercise the asset bridge with authored synthetic geometry, never retail data."""
import json
import struct
import tempfile
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

import prepare_native_scene as scene


def source(name, params, values):
    count = len(values) // len(params)
    return '<source id="%s"><float_array id="%s_array" count="%s">%s</float_array><technique_common><accessor source="#%s_array" count="%s" stride="%s">%s</accessor></technique_common></source>' % (
        name, name, len(values), " ".join(map(str, values)), name, count, len(params),
        "".join('<param name="%s" type="float"/>' % p for p in params))


def mesh_text(indices="0 0 1 1 2 2 3 3", recover=False):
    pos = source("positions", "XYZ", [0,0,0, 1,0,0, 1,1,0, 0,1,0])
    uv = source("uv", "ST", [0,0, 1,0, 1,1, 0,1])
    color = source("colors", "RGBA", [0.2,0.3,0.4,1] * 4) if recover else ""
    return '''<COLLADA xmlns="http://www.collada.org/2005/11/COLLADASchema">
    <asset><unit meter="1"/><up_axis>Z_UP</up_axis></asset>
    <library_geometries><geometry id="shape"><mesh>%s%s%s
    <vertices id="vertices"><input semantic="POSITION" source="#positions"/></vertices>
    <polylist count="1"><input semantic="VERTEX" source="#vertices" offset="0"/>
    <input semantic="TEXCOORD" source="#uv" offset="1" set="0"/>
    <vcount>4</vcount><p>%s</p></polylist></mesh></geometry></library_geometries>
    <library_visual_scenes><visual_scene><node><instance_geometry url="#shape"/></node></visual_scene></library_visual_scenes>
    </COLLADA>''' % (pos, uv, color, indices)


class SceneBridgeTests(unittest.TestCase):
    def setUp(self):
        scratch = Path(__file__).resolve().parent.parent / "build" / "tests"
        scratch.mkdir(parents=True, exist_ok=True)
        self.directory = tempfile.TemporaryDirectory(dir=scratch)
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.mesh = self.root / "authored.dae"

    def write_mesh(self, text):
        self.mesh.write_text(text)

    def test_independent_uv_indices_and_quad_triangulation(self):
        self.write_mesh(mesh_text("0 3 1 2 2 1 3 0"))
        groups, recovered = scene.read_mesh(self.mesh)
        vertices = groups[None]
        self.assertEqual(len(vertices), 6)
        self.assertEqual(vertices[0][:5], (0,0,0,0,0))
        self.assertEqual(vertices[1][:5], (1,0,0,1,0))
        self.assertEqual(vertices[2][:5], (1,1,0,1,1))
        self.assertEqual(vertices[5][:5], (0,1,0,0,1))
        self.assertEqual(recovered, 0)

    def test_native_package_header_and_vertex_layout(self):
        self.write_mesh(mesh_text())
        output = self.root / "authored.dlscene"
        report = scene.write_scene([self.mesh], output)
        magic, version, vertices, batches, textures, *bounds = scene.HEADER.unpack_from(output.read_bytes())
        self.assertEqual((magic, version, vertices, batches, textures), (scene.MAGIC, 1, 6, 1, 0))
        self.assertEqual(bounds, [0,0,0,1,1,0])
        self.assertEqual(output.stat().st_size, scene.HEADER.size + 6*scene.VERTEX.size + scene.BATCH.size)
        self.assertEqual(report["triangles"], 2)
        self.assertFalse(json.loads(output.with_suffix(".json").read_text())["contains_game_logic"])

    def test_unbound_colors_are_explicit(self):
        self.write_mesh(mesh_text(recover=True))
        plain, _ = scene.read_mesh(self.mesh)
        rebound, count = scene.read_mesh(self.mesh, True)
        self.assertEqual(plain[None][0][5:], (1,1,1,1))
        self.assertEqual(rebound[None][0][5:], (0.2,0.3,0.4,1))
        self.assertEqual(count, 1)

    def test_bad_indices_and_counts_fail(self):
        for indices in ("-1 0 1 1 2 2 3 3", "4 0 1 1 2 2 3 3", "0 0 1 1"):
            with self.subTest(indices=indices):
                self.write_mesh(mesh_text(indices))
                with self.assertRaises(ValueError):
                    scene.read_mesh(self.mesh)

    def test_transforms_scales_and_other_up_axes_fail(self):
        for changed in (mesh_text().replace("Z_UP", "Y_UP"),
                        mesh_text().replace('meter="1"', 'meter="2"'),
                        mesh_text().replace("<node>", "<node><translate>1 2 3</translate>")):
            self.write_mesh(changed)
            with self.assertRaises(ValueError):
                scene.read_mesh(self.mesh)

    def test_nonfinite_coordinates_fail(self):
        self.write_mesh(mesh_text().replace("0 0 0 1 0 0", "nan 0 0 1 0 0"))
        with self.assertRaises(ValueError):
            scene.read_mesh(self.mesh)

    def test_texture_escape_and_missing_texture_fail(self):
        text = '''<COLLADA xmlns="http://www.collada.org/2005/11/COLLADASchema">
        <library_images><image id="image"><init_from>%s</init_from></image></library_images>
        <library_effects><effect id="effect"><profile_COMMON>
        <newparam sid="surface"><surface><init_from>image</init_from></surface></newparam>
        <newparam sid="sampler"><sampler2D><source>surface</source></sampler2D></newparam>
        <technique><lambert><diffuse><texture texture="sampler"/></diffuse></lambert></technique>
        </profile_COMMON></effect></library_effects></COLLADA>'''
        for texture in ("../outside.png", "missing.png"):
            with self.subTest(texture=texture), self.assertRaises(ValueError):
                scene.texture_bindings(ET.fromstring(text % texture), self.root)


if __name__ == "__main__":
    unittest.main()
