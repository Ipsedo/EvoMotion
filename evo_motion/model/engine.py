from typing import Dict

import pybullet as pb
from glm import mat4, vec3

from .item import Item


class Engine:
    def __init__(self, time_step: float, gravity: vec3) -> None:
        self.__items: Dict[str, Item] = {}

        self.__world_id = pb.connect(pb.DIRECT)

        pb.setGravity(gravity[0], gravity[1], gravity[2])
        pb.setTimeStep(time_step, self.__world_id)

    def step(self) -> None:
        pb.stepSimulation(self.__world_id)

    def add_item(self, item: Item) -> None:
        self.__items[item.name] = item

    def get_model_matrix(self, item_name: str) -> mat4:
        return self.__items[item_name].model_matrix()

    def reset(self) -> None:
        pb.resetSimulation(self.__world_id)
