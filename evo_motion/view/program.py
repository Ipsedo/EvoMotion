import ctypes
from typing import Dict, List, Tuple

import numpy as np
from glm import mat4, vec3, vec4
from OpenGL import GL
from OpenGL.constant import IntConstant
from OpenGL.GL import shaders

from .constants import BYTES_PER_FLOAT


class Program:
    def __init__(self, builder: "Program.Builder") -> None:
        self.__program = shaders.compileProgram(
            Program.__load_shader(
                GL.GL_VERTEX_SHADER, builder.vertex_shader_path
            ),
            Program.__load_shader(
                GL.GL_FRAGMENT_SHADER, builder.fragment_shader_path
            ),
        )

        self.__uniform_mat_4fv: Dict[str, int] = {
            m: GL.glGetUniformLocation(self.__program, m)
            for m in builder.uniform_mat_4fv
        }

        self.__uniform_4fv: Dict[str, int] = {
            v: GL.glGetUniformLocation(self.__program, v)
            for v in builder.uniform_4fv
        }

        self.__uniform_3fv: Dict[str, int] = {
            v: GL.glGetUniformLocation(self.__program, v)
            for v in builder.uniform_3fv
        }

        self.__uniform_1f: Dict[str, int] = {
            f: GL.glGetUniformLocation(self.__program, f)
            for f in builder.uniform_1f
        }

        self.__buffers: Dict[str, Tuple[int, Dict[str, int]]] = {
            b: (
                Program.__gen_buffer(builder.buffers[b][1]),
                {
                    a: GL.glGetAttribLocation(self.__program, a)
                    for a in builder.buffers[b][0]
                },
            )
            for b in builder.buffers
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
        buffer_id, attrib_handles = self.__buffers[buffer_name]
        attrib = attrib_handles[name]

        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, buffer_id)

        GL.glEnableVertexAttribArray(attrib)
        GL.glVertexAttribPointer(
            attrib,
            data_size,
            GL.GL_FLOAT,
            GL.GL_FALSE,
            stride,
            ctypes.c_void_p(offset),
        )

        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, 0)

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

    def disable_attribs(self, buffer_name: str) -> None:
        for attrib_name in self.__buffers[buffer_name][1]:
            GL.glDisableVertexAttribArray(
                self.__buffers[buffer_name][1][attrib_name]
            )

    @staticmethod
    def draw_arrays(draw_type: IntConstant, first: int, count: int) -> None:
        GL.glDrawArrays(draw_type, first, count)

    @staticmethod
    def __load_shader(shader_type: IntConstant, glsl_file_path: str) -> int:
        with open(glsl_file_path, mode="r", encoding="utf-8") as glsl_file:
            shader_source = glsl_file.readlines()
            return int(shaders.compileShader(shader_source, shader_type))

    @staticmethod
    def __gen_buffer(data: np.ndarray) -> int:
        buffer: int = GL.glGenBuffers(1)

        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, buffer)
        GL.glBufferData(
            GL.GL_ARRAY_BUFFER,
            data.shape[0] * BYTES_PER_FLOAT,
            data,
            GL.GL_STATIC_DRAW,
        )
        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, 0)

        return buffer

    class Builder:
        def __init__(
            self,
            vertex_shader_path: str,
            fragment_shader_path: str,
        ) -> None:
            self.vertex_shader_path = vertex_shader_path
            self.fragment_shader_path = fragment_shader_path

            self.uniform_mat_4fv: List[str] = []
            self.uniform_4fv: List[str] = []
            self.uniform_3fv: List[str] = []
            self.uniform_1f: List[str] = []

            self.buffers: Dict[str, Tuple[List[str], np.ndarray]] = {}

        def add_uniform_mat_4fv(self, name: str) -> "Program.Builder":
            assert (
                name not in self.uniform_mat_4fv
            ), f"{name} already registered"
            self.uniform_mat_4fv.append(name)
            return self

        def add_uniform_4fv(self, name: str) -> "Program.Builder":
            assert name not in self.uniform_4fv, f"{name} already registered"
            self.uniform_4fv.append(name)
            return self

        def add_uniform_3fv(self, name: str) -> "Program.Builder":
            assert name not in self.uniform_3fv, f"{name} already registered"
            self.uniform_3fv.append(name)
            return self

        def add_uniform_1f(self, name: str) -> "Program.Builder":
            assert name not in self.uniform_1f, f"{name} already registered"
            self.uniform_1f.append(name)
            return self

        def new_buffer(
            self, buffer_name: str, data: np.ndarray
        ) -> "Program._BufferBuilder":
            return Program._BufferBuilder(self, buffer_name, data)

        def build_program(self) -> "Program":
            return Program(self)

    class _BufferBuilder:
        def __init__(
            self,
            program_builder: "Program.Builder",
            buffer_name: str,
            data: np.ndarray,
        ) -> None:
            self.__program_builder = program_builder
            self.buffer_name = buffer_name
            self.attribs: List[str] = []
            self.data = data

        def add_attrib(self, name: str) -> "Program._BufferBuilder":
            assert name not in self.attribs, f"{name} already registered"
            self.attribs.append(name)
            return self

        def build_buffer(self) -> "Program.Builder":
            self.__program_builder.buffers[self.buffer_name] = (
                self.attribs,
                self.data,
            )

            return self.__program_builder
