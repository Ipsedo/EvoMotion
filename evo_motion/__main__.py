from random import random

import torch as th
from glm import normalize, vec3, vec4
from OpenGL import GL

from .envs import cartpole
from .view import Renderer, SpecularObj, StaticCamera


def main() -> None:
    GL.errcheck = True

    camera = StaticCamera(
        vec3(0, 1, -1),
        normalize(vec3(0, -0.1, 1)),
        vec3(0, 1, 0),
    )

    r = Renderer("evo_motion", 1600, 900, camera)
    r.open_window()

    env = cartpole.CartPole(
        "/home/samuel/PycharmProjects/EvoMotion/resources/"
    )

    for name in env.items:
        item = env.items[name]
        s = SpecularObj(
            item.shape.get_vertices(),
            item.shape.get_normals(),
            vec4(random(), random(), random(), 1.0),
            vec4(random(), random(), random(), 1.0),
            vec4(random(), random(), random(), 1.0),
            300.0,
        )

        r.add_drawable(item.name, s)

    i = 0
    while not r.is_close():

        if i % 1000 == 0:
            env.reset()

        env.step(th.rand(*env.action_space) * 40 - 20)

        r.draw({name: item.model_matrix() for name, item in env.items.items()})

        i += 1


if __name__ == "__main__":
    main()
