from abc import ABC, abstractmethod
from typing import List

from torch import Tensor


class Agent(ABC):
    def __init__(self, state_space: List[int], action_space: List[int]):
        self.__state_space = state_space
        self.__action_space = action_space

    @property
    def state_space(self) -> List[int]:
        return self.__state_space

    @property
    def action_space(self) -> List[int]:
        return self.__action_space

    @abstractmethod
    def act(self, state: Tensor) -> Tensor:
        pass

    @abstractmethod
    def is_continuous(self) -> bool:
        pass
