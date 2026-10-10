import struct
import unittest

from prepare_native_animation import animation_layout, fingerprint, prove_matrix_independence, scene_geometry


class NativeAnimationPackagingTests(unittest.TestCase):
    def fixed_fixture(self):
        # One quaternion pair followed by scale and translation pairs. Priming
        # repeats the first descriptor, while unknown fourth lanes select joints.
        fixed = bytearray(256)
        fixed[3], fixed[4], fixed[8], fixed[9], fixed[10] = 8, 6, 1, 1, 1
        first = 16 + 8 * 16
        fixed[first:first + 8] = bytes([128] * 8)
        for row in (1, 2, 3):
            fixed[first + row * 8:first + (row + 1) * 8] = bytes([128, 128, 128, 0, 128, 128, 128, 1])
        return fixed, first

    def test_unknown_lanes_are_proved_irrelevant(self):
        fixed, _ = self.fixed_fixture()
        result = prove_matrix_independence(fixed, 2)
        self.assertEqual(result["groups"], 6)
        self.assertEqual(result["unknown_words"], [28, 44, 92, 108])
        self.assertEqual(result["unknown_live_matrix_words"], [])
        self.assertFalse(result["inherited_VU_values_assumed"])

    def test_unknown_quaternion_or_invalid_pair_refuses(self):
        fixed, first = self.fixed_fixture()
        fixed[first] = 7
        with self.assertRaisesRegex(ValueError, "unknown inherited"):
            prove_matrix_independence(fixed, 2)
        fixed, first = self.fixed_fixture()
        fixed[first + 8 + 3] = 130
        with self.assertRaisesRegex(ValueError, "output exceeds"):
            prove_matrix_independence(fixed, 2)

    def test_dependency_truncation_and_unsupported_opcode_refuse(self):
        fixed, first = self.fixed_fixture()
        with self.assertRaises(ValueError):
            prove_matrix_independence(fixed[:first + 2], 2)
        fixed[first + fixed[4] * 8] = 3
        with self.assertRaisesRegex(ValueError, "dependency opcode"):
            prove_matrix_independence(fixed, 2)

    def test_frame_bank_offsets_and_wrap_inputs(self):
        raw = bytearray(5000)
        raw[8], raw[12] = 107, 112
        struct.pack_into("<I", raw, 0x48, 0x50)
        raw[0x60] = 30
        struct.pack_into("<I", raw, 0x6C, 0x40)
        stream, fixed_bytes, variable_bytes = 0x90, 99 * 16, 5 * 16
        raw[stream:stream + 11] = bytes([2, 99, 5, 62, 62, 1, 0, 107, 54, 1, 7])
        for frame in range(30):
            start = stream + fixed_bytes + frame * variable_bytes
            raw[start:start + variable_bytes] = bytes([frame]) * variable_bytes
        fixed, frames = animation_layout(raw, 107)
        self.assertEqual(len(fixed), 1584)
        self.assertEqual(frames[0], b"\0" * 80)
        self.assertEqual(frames[29], bytes([29]) * 80)
        with self.assertRaises(ValueError):
            animation_layout(raw, 106)
        with self.assertRaises(ValueError):
            animation_layout(raw[:stream + fixed_bytes + 29 * variable_bytes], 107)
        raw[stream + 8] = 0
        with self.assertRaisesRegex(ValueError, "Only the verified"):
            animation_layout(raw, 107)

    def test_geometry_fingerprint_and_extents(self):
        geometry = struct.pack("<27f", *range(27))
        header = struct.pack("<8s5I6f", b"DLHOST01", 2, 3, 1, 0, 1, *([0] * 6))
        scene = header + geometry + bytes(80 + 20)
        self.assertEqual(scene_geometry(scene), geometry)
        self.assertEqual(fingerprint(b"hello"), 0xA430D84680AABD0B)
        for malformed in (scene[:-1], scene + b"x", scene[:52]):
            with self.assertRaises(ValueError):
                scene_geometry(malformed)


if __name__ == "__main__":
    unittest.main()
