from random import random

import torch as th
from glm import normalize, vec3, vec4
from OpenGL import GL

from .envs import cartpole
from .model import Environment
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

    env_config = cartpole.create_cartpole_env_args(
        "/home/samuel/PycharmProjects/EvoMotion/resources/"
    )
    env = Environment(env_config)

    for item in env_config.items:
        s = SpecularObj(
            item.shape().get_vertices(),
            item.shape().get_normals(),
            vec4(random(), random(), random(), 1.0),
            vec4(random(), random(), random(), 1.0),
            vec4(random(), random(), random(), 1.0),
            300.0,
        )

        r.add_drawable(item.name, s)

    while not r.is_close():

        env.step(th.rand(*env_config.action_space) * 40 - 20)

        r.draw({item.name: item.model_matrix() for item in env_config.items})


if __name__ == "__main__":
    main()
