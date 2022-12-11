from evo_motion.model import Shape


def test_shape(obj_shape: Shape) -> None:
    assert len(obj_shape.get_normals()) == len(obj_shape.get_vertices())
    # Cube
    assert len(obj_shape.get_vertices()) == 36
