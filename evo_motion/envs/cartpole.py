from os.path import join

import pybullet as pb
from glm import vec3
from torch import Tensor, tensor

from evo_motion.controller import Controller
from evo_motion.model import Engine, EnvironmentConfig, Item, ObjShape, Step


class Chariot(Item, Controller):
    def on_input(self, action: Tensor) -> None:
        pb.applyExternalForce(
            objectUniqueId=self.body,
            linkIndex=-1,
            forceObj=vec3(action[0].item(), 0, 0),
            posObj=vec3(0),
            flags=pb.LINK_FRAME,
        )


def create_cartpole_env_args(resource_folder: str) -> EnvironmentConfig:
    base_height = 2.0
    base_pos = -4.0

    pendulum_height = 0.7
    pendulum_width = 0.1
    pendulum_offset = pendulum_height / 4.0

    chariot_height = 0.25
    chariot_width = 0.5

    chariot_pos = base_pos + base_height + chariot_height
    pendulum_pos = (
        chariot_pos + chariot_height + pendulum_height - pendulum_offset
    )

    engine = Engine(1.0 / 60.0, vec3(0.0, -10.0, 0.0))

    obj = ObjShape(join(resource_folder, "obj", "cube.obj"))

    base = Item(
        "base",
        obj,
        vec3(0.0, base_pos, 10.0),
        vec3(10.0, base_height, 10.0),
        0.0,
    )
    chariot = Chariot(
        "chariot",
        obj,
        vec3(0.0, chariot_pos, 10.0),
        vec3(chariot_width, chariot_height, chariot_width),
        2.0,
    )
    pendulum = Item(
        "pendulum",
        obj,
        vec3(0.0, pendulum_pos, 10.0),
        vec3(pendulum_width, pendulum_height, pendulum_width),
        0.5,
    )

    return EnvironmentConfig(
        items=[base, chariot, pendulum],
        controllers=[chariot],
        engine=engine,
        compute_step=lambda items: Step(
            state=tensor([1]), reward=1.0, done=False
        ),
        state_space=[1],
        action_space=[1],
        is_continuous=False,
    )
