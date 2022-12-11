from os.path import join
from typing import Dict, List

import pybullet as pb
from glm import vec3
from torch import Tensor, tensor

from evo_motion.controller import Controller
from evo_motion.model import Environment, Item, ObjShape, Step


class _Chariot(Item, Controller):
    force = 20.0

    def on_input(self, action: Tensor) -> None:
        pb.applyExternalForce(
            objectUniqueId=self.body,
            linkIndex=-1,
            forceObj=vec3(action[0].item() * _Chariot.force, 0, 0),
            posObj=vec3(0),
            flags=pb.LINK_FRAME,
        )


class CartPole(Environment):
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

    def __init__(self, resource_folder: str):
        super().__init__(
            state_space=[1],
            action_space=[1],
            is_continuous=True,
        )

        self.__obj = ObjShape(join(resource_folder, "obj", "cube.obj"))

        self.__base = self.__get_base()
        self.__chariot = self.__get_chariot()
        self.__pendulum = self.__get_pendulum()

        self.__slider_id = self.__create_constrain()

        self.__ignore_collision()

    def __ignore_collision(self) -> None:
        pb.setCollisionFilterPair(
            bodyUniqueIdA=self.__base.body,
            bodyUniqueIdB=self.__chariot.body,
            linkIndexA=-1,
            linkIndexB=-1,
            enableCollision=0,
        )

    def __get_base(self) -> Item:
        return Item(
            "base",
            self.__obj,
            vec3(0.0, CartPole.base_pos, 10.0),
            vec3(10.0, CartPole.base_height, 10.0),
            0.0,
        )

    def __get_chariot(self) -> _Chariot:
        return _Chariot(
            "chariot",
            self.__obj,
            vec3(0.0, CartPole.chariot_pos, 10.0),
            vec3(
                CartPole.chariot_width,
                CartPole.chariot_height,
                CartPole.chariot_width,
            ),
            2.0,
        )

    def __get_pendulum(self) -> Item:
        return Item(
            "pendulum",
            self.__obj,
            vec3(0.0, CartPole.pendulum_pos, 10.0),
            vec3(
                CartPole.pendulum_width,
                CartPole.pendulum_height,
                CartPole.pendulum_width,
            ),
            0.5,
        )

    def _compute_step(self) -> Step:
        return Step(state=tensor([0]), reward=0.0, done=False)

    def __create_constrain(self) -> int:
        slide_id: int = pb.createConstraint(
            parentBodyUniqueId=self.__base.body,
            parentLinkIndex=-1,
            childBodyUniqueId=self.__chariot.body,
            childLinkIndex=-1,
            jointType=pb.JOINT_PRISMATIC,
            jointAxis=vec3(1, 0, 0),
            parentFramePosition=vec3(0, CartPole.base_height, 0),
            childFramePosition=vec3(0, -CartPole.chariot_height, 0),
        )
        return slide_id

    def _reset_engine(self) -> None:
        self.__base = self.__get_base()
        self.__chariot = self.__get_chariot()
        self.__pendulum = self.__get_pendulum()

        self.__slider_id = self.__create_constrain()

        self.__ignore_collision()

    @property
    def items(self) -> Dict[str, Item]:
        return {
            self.__base.name: self.__base,
            self.__chariot.name: self.__chariot,
            self.__pendulum.name: self.__pendulum,
        }

    @property
    def controllers(self) -> List[Controller]:
        return [self.__chariot]
