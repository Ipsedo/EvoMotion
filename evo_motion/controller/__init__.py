from abc import ABC, abstractmethod

from torch import Tensor


class Controller(ABC):
    @abstractmethod
    def on_input(self, action: Tensor) -> None:
        pass
