from abc import ABC, abstractmethod

from glm import mat4

from .obj import ObjShape
from .shapes import Shape


class Item(ABC):
    def __init__(self, name: str):
        self.__name = name

    @property
    def name(self) -> str:
        return self.__name

    @abstractmethod
    def shape(self) -> Shape:
        pass

    @abstractmethod
    def model_matrix(self) -> mat4:
        pass


class CubeItem(Item):
    def __init__(self, name: str):
        super().__init__(name)

        self.__shape = ObjShape("")  # TODO

        # pybullet stuff

    def shape(self) -> Shape:
        return self.__shape

    def model_matrix(self) -> mat4:
        # get from pybullet
        return mat4(1)
