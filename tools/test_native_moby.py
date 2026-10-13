"""Authored GLB fixtures verify topology, attributes and rejected data layouts."""
import json
import struct
import tempfile
import unittest
from pathlib import Path

import prepare_native_moby as moby


def fixture():
    # Four authored corners, interleaved xyz/uv, followed by strip indices.
    rows = [(0,0,0,0,0), (1,0,0,1,0), (0,1,0,0,1), (1,1,0,1,1)]
    binary = b"".join(struct.pack("<5f", *row) for row in rows) + bytes((0,1,2,3))
    document = {"asset": {"version": "2.0"}, "buffers": [{"byteLength": len(binary)}],
        "bufferViews": [{"buffer": 0, "byteOffset": 0, "byteLength": 80, "byteStride": 20},
                        {"buffer": 0, "byteOffset": 80, "byteLength": 4}],
        "accessors": [{"bufferView": 0, "componentType": 5126, "count": 4, "type": "VEC3"},
                      {"bufferView": 0, "byteOffset": 12, "componentType": 5126, "count": 4, "type": "VEC2"},
                      {"bufferView": 1, "componentType": 5121, "count": 4, "type": "SCALAR"}],
        "meshes": [{"name": "authored", "primitives": [{"attributes": {"POSITION": 0, "TEXCOORD_0": 1},
                                                     "indices": 2, "mode": 5, "material": 0}]}],
        "nodes": [{"mesh": 0}], "materials": [{"pbrMetallicRoughness": {"baseColorFactor": [0.5,1,0.25,1]}}]}
    return document, binary


def encode(document, binary):
    text = json.dumps(document).encode()
    text += b" " * (-len(text) % 4)
    binary += b"\0" * (-len(binary) % 4)
    chunks = struct.pack("<II", len(text), 0x4E4F534A) + text + struct.pack("<II", len(binary), 0x004E4942) + binary
    return struct.pack("<4sII", b"glTF", 2, 12 + len(chunks)) + chunks


class NativeMobyTests(unittest.TestCase):
    def setUp(self):
        scratch = Path(__file__).resolve().parent.parent / "build/tests"
        scratch.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(dir=scratch)
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name) / "mesh.glb"
        self.document, self.binary = fixture()

    def read(self):
        self.path.write_bytes(encode(self.document, self.binary))
        return moby.read_mesh(self.path)

    def test_interleaved_strip_coordinates_uvs_color_and_winding(self):
        groups, report = self.read()
        vertices = groups[None]
        self.assertEqual([v[:3] for v in vertices], [(0,0,0),(1,0,0),(0,1,0),(0,1,0),(1,0,0),(1,1,0)])
        self.assertEqual([v[3:5] for v in vertices], [(0,0),(1,0),(0,1),(0,1),(1,0),(1,1)])
        self.assertTrue(all(v[5:] == (0.5,1,0.25,1) for v in vertices))
        self.assertEqual(report["omitted_meshes"], 0)

    def test_wrench_exported_vertices_are_restored_to_game_axes(self):
        self.document["asset"]["generator"] = "Wrench Build Tool v0.6"
        groups, report = self.read()
        vertices = groups[None]
        self.assertEqual([v[:3] for v in vertices],
                         [(0,0,0),(0,1,0),(0,0,1),(0,0,1),(0,1,0),(0,1,1)])
        self.assertTrue(report["wrench_gltf_axes_converted"])

    def test_degenerate_connector_retains_strip_parity(self):
        self.assertEqual(list(moby.triangle_indices([0,1,2,2,3,4], 5)), [(0,1,2),(3,2,4)])

    def test_bad_container_length_and_chunks_fail(self):
        data = encode(self.document, self.binary)
        for invalid in (data[:-1], b"bad", data[:16] + struct.pack("<I", 7) + data[20:]):
            self.path.write_bytes(invalid)
            with self.assertRaises(ValueError):
                moby.read_mesh(self.path)

    def test_accessor_does_not_read_past_its_view(self):
        self.document["accessors"][0]["count"] = 5
        with self.assertRaisesRegex(ValueError, "exceeds"):
            self.read()

    def test_sparse_and_misaligned_layouts_fail(self):
        for update in ({"sparse": {}}, {"byteOffset": 1}, {"componentType": 5123}):
            self.document, self.binary = fixture()
            self.document["accessors"][0].update(update)
            with self.assertRaises(ValueError):
                self.read()

    def test_vertex_indices_and_primitive_modes_fail(self):
        for value in (4, 255):
            self.binary = self.binary[:-1] + bytes([value])
            with self.assertRaises(ValueError):
                self.read()
        self.document, self.binary = fixture()
        self.document["meshes"][0]["primitives"][0]["mode"] = 1
        with self.assertRaisesRegex(ValueError, "triangles"):
            self.read()

    def test_node_transform_skin_or_deformation_is_not_silently_dropped(self):
        for field, value in (("translation", [1,2,3]), ("skin", 0), ("children", [1])):
            self.document, self.binary = fixture()
            self.document["nodes"][0][field] = value
            with self.assertRaisesRegex(ValueError, "standalone"):
                self.read()
        self.document, self.binary = fixture()
        self.document["meshes"][0]["primitives"][0]["targets"] = [{"POSITION": 0}]
        with self.assertRaisesRegex(ValueError, "deformation"):
            self.read()

    def test_texture_path_stays_inside_model_directory(self):
        self.document["materials"][0]["pbrMetallicRoughness"]["baseColorTexture"] = {"index": 0}
        self.document["textures"] = [{"source": 0}]
        self.document["images"] = [{"uri": "../escape.png"}]
        with self.assertRaisesRegex(ValueError, "local PNG"):
            self.read()

    def test_normalized_integer_uvs(self):
        doc, binary = fixture()
        doc["bufferViews"].append({"buffer": 0, "byteOffset": len(binary), "byteLength": 8})
        doc["buffers"][0]["byteLength"] += 8
        doc["accessors"][1] = {"bufferView": 2, "componentType": 5121, "normalized": True, "type": "VEC2", "count": 4}
        self.document, self.binary = doc, binary + bytes((0,0,255,0,0,255,255,255))
        vertices = self.read()[0][None]
        self.assertEqual(vertices[1][3:5], (1,0))
        self.assertEqual(vertices[2][3:5], (0,1))

    def test_nonfinite_positions_fail(self):
        self.binary = struct.pack("<f", float("nan")) + self.binary[4:]
        with self.assertRaisesRegex(ValueError, "Nonfinite"):
            self.read()

    def test_parent_transform_is_not_silently_dropped(self):
        self.document["nodes"].append({"translation": [2,3,4], "children": [0]})
        with self.assertRaisesRegex(ValueError, "hierarchy"):
            self.read()

    def test_alternative_mesh_is_selected_explicitly(self):
        self.document["meshes"].append({"name": "unused_lod", "primitives": []})
        self.document["nodes"].append({"mesh": 1})
        groups, description = self.read()
        self.assertEqual(len(groups[None]), 6)
        self.assertEqual(description["omitted_meshes"], 1)
        with self.assertRaisesRegex(ValueError, "no drawable"):
            moby.read_mesh(self.path, 1)

    def test_blended_alpha_requires_an_explicit_preview_option(self):
        self.document["materials"][0]["alphaMode"] = "BLEND"
        self.path.write_bytes(encode(self.document, self.binary))
        with self.assertRaisesRegex(ValueError, "blended alpha"):
            moby.read_mesh(self.path)
        groups, _ = moby.read_mesh(self.path, allow_blended_alpha=True)
        self.assertEqual(len(groups[None]), 6)

    def test_unskinned_joint_zero_weights_are_only_allowed_for_static_preview(self):
        self.document, self.binary = fixture()
        joint_offset = len(self.binary)
        self.binary += bytes(16)
        weight_offset = len(self.binary)
        self.binary += bytes((255,0,0,0) * 4)
        self.document["buffers"][0]["byteLength"] = len(self.binary)
        self.document["bufferViews"].extend([
            {"buffer":0,"byteOffset":joint_offset,"byteLength":16},
            {"buffer":0,"byteOffset":weight_offset,"byteLength":16},
        ])
        self.document["accessors"].extend([
            {"bufferView":2,"componentType":5121,"count":4,"type":"VEC4"},
            {"bufferView":3,"componentType":5121,"normalized":True,"count":4,"type":"VEC4"},
        ])
        attributes=self.document["meshes"][0]["primitives"][0]["attributes"]
        attributes.update({"JOINTS_0":3,"WEIGHTS_0":4})
        self.path.write_bytes(encode(self.document,self.binary))
        with self.assertRaisesRegex(ValueError,"deformation"):
            moby.read_mesh(self.path)
        groups,report=moby.read_mesh(self.path,allow_unskinned_static_joints=True)
        self.assertEqual(len(groups[None]),6)
        self.assertTrue(report["ignored_static_joint_attributes"])
        self.binary=bytearray(self.binary)
        self.binary[joint_offset]=1
        self.path.write_bytes(encode(self.document,bytes(self.binary)))
        with self.assertRaisesRegex(ValueError,"inert joint-zero"):
            moby.read_mesh(self.path,allow_unskinned_static_joints=True)


if __name__ == "__main__":
    unittest.main()
