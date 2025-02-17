#version 330 core

in vec3 a_position;
in vec3 a_normal;

in vec4 a_ambient_color;
in vec4 a_diffuse_color;
in vec4 a_specular_color;
in float a_shininess;

uniform mat4 u_mvp_matrix;
uniform mat4 u_mv_matrix;

out vec3 v_position;
out vec3 v_normal;

out vec4 v_ambient_color;
out vec4 v_diffuse_color;
out vec4 v_specular_color;
out float v_shininess;

void main() {
    v_position = vec3(u_mvp_matrix * vec4(a_position, 1.0));
    v_normal = normalize(vec3(u_mv_matrix * vec4(a_normal, 0.0)));

    v_ambient_color = a_ambient_color;
    v_diffuse_color = a_diffuse_color;
    v_specular_color = a_specular_color;
    v_shininess = a_shininess;

    gl_Position = u_mvp_matrix * vec4(a_position, 1.0);
}
