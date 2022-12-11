from os.path import abspath, dirname, join

import pytest
from glm import vec3, vec4

from evo_motion.model import ObjShape, Shape
from evo_motion.view import Camera, Renderer, SpecularObj, StaticCamera


@pytest.fixture(name="resources_folder", scope="session")
def get_resources_folder() -> str:
    return abspath(join(dirname(__file__), "..", "resources"))


@pytest.fixture(name="camera", scope="session")
def get_camera() -> Camera:
    return StaticCamera(vec3(0, 0, 0), vec3(0, 0, 1), vec3(0, 1, 0))


@pytest.fixture(name="renderer", scope="session")
def get_renderer(camera: Camera) -> Renderer:
    return Renderer("test", 128, 128, camera)


@pytest.fixture(name="obj_shape", scope="session")
def get_obj_shape(resources_folder: str) -> Shape:
    return ObjShape(join(resources_folder, "obj", "cube.obj"))


@pytest.fixture(name="specular_obj", scope="session")
def get_specular_obj(renderer: Renderer, obj_shape: Shape) -> SpecularObj:
    renderer.open_window()
    assert not renderer.is_close()

    return SpecularObj(
        obj_shape.get_vertices(),
        obj_shape.get_normals(),
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(1.0, 1.0, 1.0, 1.0),
        256,
    )
