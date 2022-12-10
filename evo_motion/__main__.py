from glm import mat4, vec4
from OpenGL import GL

from .model import ObjShape
from .view import Renderer, SpecularObj


def main() -> None:
    GL.errcheck = True
    r = Renderer("evo_motion", 256, 256)
    r.open_window()

    obj = ObjShape(
        "/home/samuel/PycharmProjects/EvoMotion/resources/obj/cube.obj"
    )

    s = SpecularObj(
        obj.get_vertices(),
        obj.get_normals(),
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(0.0, 1.0, 0.0, 1.0),
        vec4(1.0, 1.0, 1.0, 1.0),
        300.0,
    )

    r.add_drawable("cube", s)

    model_mat = mat4(1.0)

    while not r.is_close():
        r.draw({"cube": model_mat})


if __name__ == "__main__":
    main()
