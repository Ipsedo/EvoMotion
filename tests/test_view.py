import pytest

from evo_motion.view import Renderer


def test_open_close_window(renderer: Renderer) -> None:
    try:
        assert renderer.is_close()

        renderer.open_window()
        assert not renderer.is_close()

        renderer.close()
        assert renderer.is_close()
    except Exception as e:
        pytest.fail(str(e))
