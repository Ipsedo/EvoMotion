from abc import ABC, abstractmethod

from glm import mat4, vec4
from OpenGL import GL, constant


def _load_shader(shader_type: constant.IntConstant, shader_source: str) -> int:
    shader_ptr: int = GL.glCreateShader(shader_type)
    GL.glShaderSource(shader_ptr, 1, shader_source, None)
    return shader_ptr


class Drawable(ABC):
    @abstractmethod
    def draw(
        self,
        mvp_matrix: mat4,
        mv_matrix: mat4,
        light_pos_from_camera: vec4,
        camera_pos: vec4,
    ) -> None:
        pass


class ObjMtlVBO(Drawable):
    __VERTEX_SHADER = """
    uniform mat4 u_MVPMatrix;
    uniform mat4 u_MVMatrix;

    attribute vec4 a_Position;
    attribute vec3 a_Normal;

    varying vec3 v_Position;
    varying vec3 v_Normal;

    void main(){
        v_Position = vec3(u_MVMatrix * a_Position);
        v_Normal = normalize(vec3(u_MVMatrix * vec4(a_Normal, 0.0)));
        gl_Position = u_MVPMatrix * a_Position;
    }
    """

    __FRAGMENT_SHADER = """
    #version 150

    precision mediump float;

    uniform vec3 u_CameraPosition;
    uniform vec3 u_LightPos;
    uniform float u_distance_coef;
    uniform float u_light_coef;

    uniform vec4 u_material_ambient_Color;
    uniform vec4 u_material_diffuse_Color;
    uniform vec4 u_material_specular_Color;
    uniform float u_material_shininess;

    varying vec3 v_Position;
    varying vec3 v_Normal;

    void main(){
        float distance = length(u_LightPos - v_Position);
        vec3 lightVector = normalize(u_LightPos - v_Position);

        float diffuse_coeff = max(dot(v_Normal, lightVector), 0.1) * u_light_coef;
        diffuse_coeff = diffuse_coeff * (1.0 / (1.0 + (u_distance_coef * distance * distance)));

        vec4 diffuse = diffuse_coeff * u_material_diffuse_Color;

        float specularCoefficient = 0.0;
        if(diffuse_coeff > 0.0){
            vec3 incidenceVector = -lightVector;
            vec3 reflectionVector = reflect(incidenceVector, v_Normal);
            vec3 surfaceToCamera = normalize(u_CameraPosition - v_Position);
            float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
            specularCoefficient = pow(cosAngle, u_material_shininess) * u_light_coef;
        }

        vec4 specular = specularCoefficient * u_material_specular_Color;

        vec4 ambient = 0.1 * u_material_ambient_Color;

        gl_FragColor = ambient + diffuse + specular;
    }
    """

    def __init__(self) -> None:
        # create opengl program
        self.__program = GL.glCreateProgram()

        GL.glAttachShader(
            self.__program,
            _load_shader(GL.GL_VERTEX_SHADER, ObjMtlVBO.__VERTEX_SHADER),
        )
        GL.glAttachShader(
            self.__program,
            _load_shader(GL.GL_FRAGMENT_SHADER, ObjMtlVBO.__FRAGMENT_SHADER),
        )

        GL.glLinkProgram(self.__program)

        # bind program's variables
        self.__mvp_matrix_handle = GL.glGetUniformLocation(
            self.__program, "u_MVPMatrix"
        )
        self.__mv_matrix_handle = GL.glGetUniformLocation(
            self.__program, "u_MVMatrix"
        )
        self.__position_handle = GL.glGetAttribLocation(
            self.__program, "a_Position"
        )
        self.__amb_color_handle = GL.glGetUniformLocation(
            self.__program, "u_material_ambient_Color"
        )
        self.__diff_color_handle = GL.glGetUniformLocation(
            self.__program, "u_material_diffuse_Color"
        )
        self.__spec_color_handle = GL.glGetUniformLocation(
            self.__program, "u_material_specular_Color"
        )
        self.__light_pos_handle = GL.glGetUniformLocation(
            self.__program, "u_LightPos"
        )
        self.__normal_handle = GL.glGetAttribLocation(
            self.__program, "a_Normal"
        )
        self.__distance_coef_handle = GL.glGetUniformLocation(
            self.__program, "u_distance_coef"
        )
        self.__light_coef_handle = GL.glGetUniformLocation(
            self.__program, "u_light_coef"
        )
        self.__camera_pos_handle = GL.glGetUniformLocation(
            self.__program, "u_CameraPosition"
        )
        self.__spec_shininess_handle = GL.glGetUniformLocation(
            self.__program, "u_material_shininess"
        )

    def draw(
        self,
        mvp_matrix: mat4,
        mv_matrix: mat4,
        light_pos_from_camera: vec4,
        camera_pos: vec4,
    ) -> None:
        pass
