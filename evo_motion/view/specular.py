from os.path import join
from typing import List, Tuple

import numpy as np
from glm import mat4, vec3, vec4
from OpenGL.GL import GL_TRIANGLES

from .constants import BYTES_PER_FLOAT, SHADER_ROOT_PATH
from .drawable import Drawable
from .program import Program, ProgramBuilder


class SpecularObj(Drawable):
    def __init__(
        self,
        vertices: List[Tuple[float, float, float]],
        normals: List[Tuple[float, float, float]],
        ambient_color: vec4,
        diffuse_color: vec4,
        specular_color: vec4,
        shininess: float,
    ) -> None:
        assert len(vertices) == len(normals), (
            f"vertices number does not match normals number : "
            f"{len(vertices)} != {len(normals)}"
        )

        vbo_data = np.array(
            [
                d
                for v, n in zip(vertices, normals)
                for d in [v[0], v[1], v[2], n[0], n[1], n[2]]
            ],
            dtype=np.float32,
        )

        self.__nb_vertices = len(vertices)

        self.__stride = BYTES_PER_FLOAT * (3 + 3)

        self.__program: Program = (
            ProgramBuilder(
                join(SHADER_ROOT_PATH, "specular_vs.glsl"),
                join(SHADER_ROOT_PATH, "specular_fs.glsl"),
            )
            .uniform_mat_4fv("u_mvp_matrix")
            .uniform_mat_4fv("u_mv_matrix")
            .uniform_4fv("u_ambient_color")
            .uniform_4fv("u_diffuse_color")
            .uniform_4fv("u_specular_color")
            .uniform_3fv("u_light_pos")
            .uniform_1f("u_distance_coef")
            .uniform_1f("u_light_coef")
            .uniform_1f("u_shininess")
            .uniform_3fv("u_cam_pos")
            .buffer("vertices_normals_buffer", vbo_data)
            .attrib("a_position")
            .attrib("a_normal")
            .build_buffer()
            .build_program()
        )

        self.__ambient_color = ambient_color
        self.__diffuse_color = diffuse_color
        self.__specular_color = specular_color
        self.__shininess = shininess

    def draw(
        self,
        mvp_matrix: mat4,
        mv_matrix: mat4,
        light_pos_from_camera: vec3,
        camera_pos: vec3,
    ) -> None:
        self.__program.use()

        self.__program.attrib(
            "vertices_normals_buffer",
            "a_position",
            3,
            self.__stride,
            0,
        )

        self.__program.attrib(
            "vertices_normals_buffer",
            "a_normal",
            3,
            self.__stride,
            3 * BYTES_PER_FLOAT,
        )

        self.__program.uniform_mat_4fv("u_mvp_matrix", mvp_matrix)
        self.__program.uniform_mat_4fv("u_mv_matrix", mv_matrix)

        self.__program.uniform_3fv("u_light_pos", light_pos_from_camera)
        self.__program.uniform_3fv("u_cam_pos", camera_pos)

        self.__program.uniform_4fv("u_ambient_color", self.__ambient_color)
        self.__program.uniform_4fv("u_diffuse_color", self.__diffuse_color)
        self.__program.uniform_4fv("u_specular_color", self.__specular_color)

        self.__program.uniform_1f("u_distance_coef", 0.0)
        self.__program.uniform_1f("u_light_coef", 1.0)
        self.__program.uniform_1f("u_shininess", self.__shininess)

        Program.draw_arrays(GL_TRIANGLES, 0, self.__nb_vertices)

        self.__program.disable_attribs("vertices_normals_buffer")
