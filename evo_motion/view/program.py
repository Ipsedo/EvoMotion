from typing import Dict, List, Tuple

import numpy as np
from glm import mat4, vec3, vec4
from OpenGL import GL
from OpenGL.arrays.vbo import VBO
from OpenGL.constant import IntConstant
from OpenGL.GL import shaders

from .constants import BYTES_PER_FLOAT


class Program:
    def __init__(
        self,
        vertex_shader_path: str,
        fragment_shader_path: str,
        uniform_mat_4fv: List[str],
        uniform_4fv: List[str],
        uniform_3fv: List[str],
        uniform_1f: List[str],
        buffers: Dict[str, Tuple[List[str], np.ndarray]],
    ) -> None:

        self.__program = shaders.compileProgram(
            Program.__load_shader(GL.GL_VERTEX_SHADER, vertex_shader_path),
            Program.__load_shader(GL.GL_FRAGMENT_SHADER, fragment_shader_path),
        )

        self.__uniform_mat_4fv: Dict[str, int] = {
            m: GL.glGetUniformLocation(self.__program, m)
            for m in uniform_mat_4fv
        }

        self.__uniform_4fv: Dict[str, int] = {
            v: GL.glGetUniformLocation(self.__program, v) for v in uniform_4fv
        }

        self.__uniform_3fv: Dict[str, int] = {
            v: GL.glGetUniformLocation(self.__program, v) for v in uniform_3fv
        }

        self.__uniform_1f: Dict[str, int] = {
            f: GL.glGetUniformLocation(self.__program, f) for f in uniform_1f
        }

        self.__buffers: Dict[str, Tuple[VBO, Dict[str, int]]] = {
            b: (
                VBO(
                    buffers[b][1],
                    target=GL.GL_ARRAY_BUFFER,
                    usage=GL.GL_STATIC_DRAW,
                    size=buffers[b][1].shape[0],
                ),
                {
                    a: GL.glGetAttribLocation(self.__program, a)
                    for a in buffers[b][0]
                },
            )
            for b in buffers
        }

    def use(self) -> None:
        GL.glUseProgram(self.__program)

    def attrib(
        self,
        buffer_name: str,
        name: str,
        data_size: int,
        stride: int,
        offset: int,
    ) -> None:
        vbo, attrib_handles = self.__buffers[buffer_name]
        attrib = attrib_handles[name]

        vbo.bind()

        GL.glEnableVertexAttribArray(attrib)
        GL.glVertexAttribPointer(
            attrib,
            data_size,
            GL.GL_FLOAT,
            GL.GL_FALSE,
            stride,
            vbo + offset,
        )

        vbo.unbind()

    def uniform_mat_4fv(self, name: str, mat: mat4) -> None:
        GL.glUniformMatrix4fv(
            self.__uniform_mat_4fv[name], 1, GL.GL_FALSE, mat.to_list()
        )

    def uniform_4fv(self, name: str, vec: vec4) -> None:
        GL.glUniform4fv(self.__uniform_4fv[name], 1, vec.to_list())

    def uniform_3fv(self, name: str, vec: vec3) -> None:
        GL.glUniform3fv(self.__uniform_3fv[name], 1, vec.to_list())

    def uniform_1f(self, name: str, f: float) -> None:
        GL.glUniform1f(self.__uniform_1f[name], f)

    @staticmethod
    def draw_arrays(draw_type: IntConstant, first: int, count: int) -> None:
        GL.glDrawArrays(draw_type, first, count)

    def disable_attribs(self, buffer_name: str) -> None:
        for attrib_name in self.__buffers[buffer_name][1]:
            GL.glDisableVertexAttribArray(
                self.__buffers[buffer_name][1][attrib_name]
            )

    @staticmethod
    def __load_shader(shader_type: IntConstant, glsl_file_path: str) -> int:
        with open(glsl_file_path, mode="r", encoding="utf-8") as glsl_file:
            shader_source = glsl_file.readlines()
            return int(shaders.compileShader(shader_source, shader_type))

    @staticmethod
    def __gen_buffer(data: np.ndarray) -> int:
        buffer_id: int = GL.glGenBuffers(1)

        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, buffer_id)
        GL.glBufferData(
            GL.GL_ARRAY_BUFFER,
            len(data) * BYTES_PER_FLOAT,
            data,
            GL.GL_STATIC_DRAW,
        )
        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, 0)

        return buffer_id


class ProgramBuilder:
    def __init__(
        self,
        vertex_shader_path: str,
        fragment_shader_path: str,
    ) -> None:
        self.__vertex_shader_path = vertex_shader_path
        self.__fragment_shader_path = fragment_shader_path

        self.__uniform_mat_4fv: List[str] = []
        self.__uniform_4fv: List[str] = []
        self.__uniform_3fv: List[str] = []
        self.__uniform_1f: List[str] = []

        self.__buffers: Dict[str, Tuple[List[str], np.ndarray]] = {}

    def uniform_mat_4fv(self, name: str) -> "ProgramBuilder":
        assert name not in self.__uniform_mat_4fv, f"{name} already registered"
        self.__uniform_mat_4fv.append(name)
        return self

    def uniform_4fv(self, name: str) -> "ProgramBuilder":
        assert name not in self.__uniform_4fv, f"{name} already registered"
        self.__uniform_4fv.append(name)
        return self

    def uniform_3fv(self, name: str) -> "ProgramBuilder":
        assert name not in self.__uniform_3fv, f"{name} already registered"
        self.__uniform_3fv.append(name)
        return self

    def uniform_1f(self, name: str) -> "ProgramBuilder":
        assert name not in self.__uniform_1f, f"{name} already registered"
        self.__uniform_1f.append(name)
        return self

    def buffer(self, buffer_name: str, data: np.ndarray) -> "BufferBuilder":
        return BufferBuilder(self, buffer_name, data)

    def add_buffer(
        self, buffer_name: str, attribs: List[str], data: np.ndarray
    ) -> None:
        assert (
            buffer_name not in self.__buffers
        ), f"{buffer_name} already registered"
        self.__buffers[buffer_name] = (attribs, data)

    def get_buffers(self) -> Dict[str, Tuple[List[str], np.ndarray]]:
        return self.__buffers

    def build_program(self) -> "Program":
        return Program(
            self.__vertex_shader_path,
            self.__fragment_shader_path,
            self.__uniform_mat_4fv,
            self.__uniform_4fv,
            self.__uniform_3fv,
            self.__uniform_1f,
            self.__buffers,
        )


class BufferBuilder:
    def __init__(
        self,
        program_builder: ProgramBuilder,
        buffer_name: str,
        data: np.ndarray,
    ) -> None:
        self.__program_builder = program_builder
        self.__buffer_name = buffer_name
        self.__attribs: List[str] = []
        self.__data = data

    def attrib(self, name: str) -> "BufferBuilder":
        assert name not in self.__attribs, f"{name} already registered"
        self.__attribs.append(name)
        return self

    def build_buffer(self) -> ProgramBuilder:
        self.__program_builder.add_buffer(
            self.__buffer_name, self.__attribs, self.__data
        )

        return self.__program_builder
