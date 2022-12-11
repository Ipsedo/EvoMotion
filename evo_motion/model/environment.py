from typing import Callable, List

from pydantic import BaseModel
from torch import Tensor

from evo_motion.controller import Controller

from .engine import Engine
from .item import Item


class Step(BaseModel):
    state: Tensor
    reward: float
    done: bool

    class Config:
        arbitrary_types_allowed = True


class EnvironmentConfig(BaseModel):
    items: List[Item]
    controllers: List[Controller]
    engine: Engine
    compute_step: Callable[[List[Item]], Step]
    state_space: List[int]
    action_space: List[int]
    is_continuous: bool

    class Config:
        arbitrary_types_allowed = True


class Environment:
    def __init__(self, config: EnvironmentConfig):
        self.__engine = config.engine

        self.__items = config.items

        for item in self.__items:
            self.__engine.add_item(item)

        self.__controllers = config.controllers

        self.__state_space = config.state_space
        self.__action_space = config.action_space
        self.__is_continuous = config.is_continuous

        self.__compute_step = config.compute_step

    def step(self, action: Tensor) -> Step:
        for c in self.__controllers:
            c.on_input(action)

        self.__engine.step()

        return self.__compute_step(self.__items)

    def reset(self) -> Step:
        return self.__compute_step(self.__items)
