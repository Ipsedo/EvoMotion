from abc import ABC, abstractmethod

from glm import vec3


class Camera(ABC):
    @abstractmethod
    def pos(self) -> vec3:
        pass

    @abstractmethod
    def up(self) -> vec3:
        pass

    @abstractmethod
    def look_vec(self) -> vec3:
        pass


class StaticCamera(Camera):
    def __init__(
        self,
        pos: vec3,
        look_vec: vec3,
        up: vec3,
    ):
        self.__pos = pos
        self.__look_vec = look_vec
        self.__up = up

    def pos(self) -> vec3:
        return self.__pos

    def up(self) -> vec3:
        return self.__up

    def look_vec(self) -> vec3:
        return self.__look_vec
