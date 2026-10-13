"""Tests for isolated native collision scene preparation."""

import unittest

import prepare_native_collision as collision


class CollisionSceneTests(unittest.TestCase):
    def test_collision_recolor_preserves_geometry_and_uvs(self):
        vertices = [(1, 2, 3, 0.25, 0.75, 0.1, 0.2, 0.3, 1.0)] * 3
        result = collision.color_collision_geometry({None: vertices}, (0.2, 0.8, 1.0, 1.0))
        self.assertEqual(result[None][0], (1, 2, 3, 0.25, 0.75, 0.2, 0.8, 1.0, 1.0))
        self.assertEqual(vertices[0], (1, 2, 3, 0.25, 0.75, 0.1, 0.2, 0.3, 1.0))

    def test_collision_recolor_rejects_invalid_input(self):
        with self.assertRaises(ValueError):
            collision.color_collision_geometry({None: [(0, 0, 0, 0, 0, 1, 1, 1, 1)]},
                                               (1, 1, 1, 1))
        with self.assertRaises(ValueError):
            collision.color_collision_geometry({None: [(0, 0, 0, 0, 0, 1, 1, 1, 1)] * 2},
                                               (0, 1, 1, 1))
        with self.assertRaises(ValueError):
            collision.color_collision_geometry({None: [(0, 0, 0, 0, 0, 1, 1, 1, 1)] * 3},
                                               (0, 1, float("nan"), 1))


if __name__ == "__main__":
    unittest.main()
