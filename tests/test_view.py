import pytest
from glm import mat4

from evo_motion.view import Renderer, SpecularObj


def test_open_close_window(renderer: Renderer) -> None:
    try:
        assert renderer.is_close()

        renderer.open_window()
        assert not renderer.is_close()

        renderer.close()
        assert renderer.is_close()
    except Exception as e:
        pytest.fail(str(e))


def test_draw_specular(renderer: Renderer, specular_obj: SpecularObj) -> None:
    assert not renderer.is_close()

    drawable_name = "cube"
    renderer.add_drawable(drawable_name, specular_obj)

    try:
        renderer.draw({drawable_name: mat4(1.0)})

        assert not renderer.is_close()
    except Exception as e:
        pytest.fail(str(e))
