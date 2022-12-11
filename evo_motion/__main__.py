from random import random

from glm import normalize, vec3, vec4
from OpenGL import GL

from .core import Agent, ContinuousRandomAgent
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

    env: Environment = cartpole.CartPole(
        "/home/samuel/PycharmProjects/EvoMotion/resources/"
    )

    agent: Agent = ContinuousRandomAgent(env.state_space, env.action_space)

    assert agent.is_continuous == env.is_continuous

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

    step = env.reset()

    while not r.is_close():

        step = env.step(agent.act(step.state))

        r.draw({name: item.model_matrix() for name, item in env.items.items()})

        i += 1

        if i % 1000 == 0:
            step = env.reset()


if __name__ == "__main__":
    main()
