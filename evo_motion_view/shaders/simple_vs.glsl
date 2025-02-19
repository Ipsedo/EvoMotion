#version 330 core

in vec3 a_position;

uniform mat4 u_p_matrix;
uniform mat4 u_v_matrix;

void main() {
    gl_Position = u_p_matrix * u_v_matrix * vec4(a_position, 1.0);
}
