from typing import Dict

import glfw
from glm import frustum, lookAt, mat4, vec3
from OpenGL import GL

from .drawable import Drawable


class Renderer:
    def __init__(self, title: str, width: int, height: int) -> None:
        self.__title = title
        self.__window = None

        self.__drawables: Dict[str, Drawable] = {}

        self.__cam_pos = vec3(0.0, 0.0, -1.0)
        self.__light_pos = vec3(0.0, 0.0, -1.0)

        self.__view_matrix = lookAt(
            vec3(0.0, 0.0, 0.0),
            vec3(0.0, 0.0, 1.0),
            vec3(0, 1, 0),
        )

        self.__proj_matrix = frustum(
            -1.0, 1.0, -height / width, height / width, 1.0, 200.0
        )

        self.__width = width
        self.__height = height

        self.__is_open = False

    def open_window(
        self,
    ) -> None:
        if not glfw.init():
            raise RuntimeError("GLFW init failed")

        self.__window = glfw.create_window(
            self.__width, self.__height, self.__title, None, None
        )

        if not self.__window:
            glfw.terminate()
            raise RuntimeError("GLFW can't be opened")

        glfw.make_context_current(self.__window)

        GL.glViewport(0, 0, self.__width, self.__height)
        GL.glClearColor(0.5, 0.0, 0.0, 1.0)

        GL.glEnable(GL.GL_DEPTH_TEST)
        GL.glEnable(GL.GL_CULL_FACE)
        GL.glEnable(GL.GL_DEBUG_OUTPUT)
        GL.glDepthFunc(GL.GL_LEQUAL)
        GL.glDepthMask(GL.GL_TRUE)

        self.__is_open = True

    def add_drawable(self, name: str, drawable: Drawable) -> None:
        self.__drawables[name] = drawable

    def draw(self, model_matrix: Dict[str, mat4]) -> None:
        if glfw.window_should_close(self.__window):
            pass

        glfw.poll_events()

        GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT)

        for drawable_name in self.__drawables:
            assert drawable_name in model_matrix, (
                f"Can't find model-view-projection and "
                f"model-view matrix for {drawable_name}"
            )

            m_matrix = model_matrix[drawable_name]

            mv_matrix = self.__view_matrix * m_matrix
            mvp_matrix = self.__proj_matrix * mv_matrix

            self.__drawables[drawable_name].draw(
                mvp_matrix, mv_matrix, self.__light_pos, self.__cam_pos
            )

        glfw.swap_buffers(self.__window)

    def is_close(self) -> bool:
        return not self.__is_open
