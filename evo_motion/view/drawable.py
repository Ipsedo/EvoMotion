from abc import ABC, abstractmethod

from glm import mat4, vec3


class Drawable(ABC):
    @abstractmethod
    def draw(
        self,
        mvp_matrix: mat4,
        mv_matrix: mat4,
        light_pos_from_camera: vec3,
        camera_pos: vec3,
    ) -> None:
        pass
