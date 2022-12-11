from abc import ABC, abstractmethod
from typing import Dict, List

import pybullet as pb
from pydantic import BaseModel
from torch import Tensor

from evo_motion.controller import Controller

from .item import Item


class Step(BaseModel):
    state: Tensor
    reward: float
    done: bool

    class Config:
        arbitrary_types_allowed = True


class Environment(ABC):
    def __init__(
        self,
        state_space: List[int],
        action_space: List[int],
        is_continuous: bool,
    ):
        self.__world_id = pb.connect(pb.DIRECT)

        pb.setGravity(0, -10, 0)
        pb.setTimeStep(1.0 / 60.0, self.__world_id)

        self.__state_space = state_space
        self.__action_space = action_space
        self.__is_continuous = is_continuous

    def step(self, action: Tensor) -> Step:
        for controller in self.controllers:
            controller.on_input(action)

        pb.stepSimulation(self.__world_id)

        return self._compute_step()

    @property
    @abstractmethod
    def items(self) -> Dict[str, Item]:
        pass

    @property
    @abstractmethod
    def controllers(self) -> List[Controller]:
        pass

    def reset(self) -> Step:
        pb.resetSimulation(self.__world_id)
        pb.setGravity(0, -10, 0)
        pb.setTimeStep(1.0 / 60.0, self.__world_id)

        self._reset_engine()

        return self._compute_step()

    @abstractmethod
    def _compute_step(self) -> Step:
        pass

    @abstractmethod
    def _reset_engine(self) -> None:
        pass

    @property
    def state_space(self) -> List[int]:
        return self.__state_space

    @property
    def action_space(self) -> List[int]:
        return self.__action_space

    @property
    def is_continuous(self) -> bool:
        return self.__is_continuous
