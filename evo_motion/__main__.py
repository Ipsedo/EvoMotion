from random import random

from glm import mat4, normalize, rotate, scale, translate, vec3, vec4
from OpenGL import GL

from .model import ObjShape
from .view import Renderer, SpecularObj, StaticCamera


def main() -> None:
    GL.errcheck = True

    camera = StaticCamera(
        vec3(0, 0, -1),
        vec3(0, 0, 1),
        vec3(0, 1, 0),
    )

    r = Renderer("evo_motion", 1600, 900, camera)
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

    translate_mat = translate(mat4(1), vec3(0.0, 0.0, 1.0))
    scale_mat = scale(mat4(1), vec3(0.25, 0.25, 0.25))

    angle = 0.0
    axis = normalize(
        vec3(random() * 2 - 1, random() * 2 - 1, random() * 2 - 1)
    )

    while not r.is_close():
        rotation_matrix = rotate(mat4(1), angle, axis)
        model_mat = mat4(1) * translate_mat * rotation_matrix * scale_mat
        r.draw({"cube": model_mat})
        angle += 0.01
        angle %= 360.0


if __name__ == "__main__":
    main()
