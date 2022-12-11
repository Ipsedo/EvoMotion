import pybullet as pb
from glm import mat4, mat4_cast, quat
from glm import scale as glm_scale
from glm import translate, vec3

from .shapes import Shape


class Item:
    def __init__(
        self,
        name: str,
        shape: Shape,
        position: vec3,
        scale: vec3,
        mass: float,
    ) -> None:

        self.__name = name
        self.__shape = shape

        self.__shape_id: int = pb.createCollisionShape(
            pb.GEOM_MESH, vertices=shape.get_vertices(), meshScale=scale
        )

        self.__body_id: int = pb.createMultiBody(
            baseCollisionShapeIndex=self.__shape_id,
            basePosition=list(position),
            baseMass=mass,
        )

        self.__scale = scale

    @property
    def name(self) -> str:
        return self.__name

    @property
    def body(self) -> int:
        return self.__body_id

    @property
    def collision_shape(self) -> int:
        return self.__shape_id

    def shape(self) -> Shape:
        return self.__shape

    def model_matrix(self) -> mat4:
        position, quaternion = pb.getBasePositionAndOrientation(self.__body_id)

        scale_mat: mat4 = glm_scale(mat4(1), self.__scale)
        rot_matrix: mat4 = mat4_cast(quat(quaternion))
        trans_matrix: mat4 = translate(
            mat4(1), vec3(position[0], position[1], position[2])
        )

        model_matrix: mat4 = trans_matrix * rot_matrix * scale_mat

        return model_matrix
