#version 330 core

precision mediump float;

uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;

uniform float u_distance_coef;
uniform float u_light_coef;

uniform vec4 u_ambient_color_a;
uniform vec4 u_diffuse_color_a;
uniform vec4 u_specular_color_a;

uniform vec4 u_ambient_color_b;
uniform vec4 u_diffuse_color_b;
uniform vec4 u_specular_color_b;

uniform float u_shininess;

uniform float u_tile_size;

in vec3 v_local_position;
in vec3 v_position;
in vec3 v_normal;

void main(){
    float distance = length(u_light_pos - v_position);
    vec3 light_vector = normalize(u_light_pos - v_position);

    float diffuse_coeff = max(dot(v_normal, light_vector), 0.1) * u_light_coef;
    diffuse_coeff = diffuse_coeff * (1.0 / (1.0 + (u_distance_coef * distance * distance)));

    vec4 diffuse_a = diffuse_coeff * u_diffuse_color_a;
    vec4 diffuse_b = diffuse_coeff * u_diffuse_color_b;

    float specular_coefficient = 0.0;
    if (diffuse_coeff > 0.0) {
        vec3 incidence_vector = -light_vector;
        vec3 reflection_vector = reflect(incidence_vector, v_normal);
        vec3 surface_to_camera = normalize(u_cam_pos - v_position);
        float cos_angle = max(0.0, dot(surface_to_camera, reflection_vector));
        specular_coefficient = pow(cos_angle, u_shininess) * u_light_coef;
    }

    vec4 specular_a = specular_coefficient * u_specular_color_a;
    vec4 specular_b = specular_coefficient * u_specular_color_b;

    vec4 ambient_a = 0.1 * u_ambient_color_a;
    vec4 ambient_b = 0.1 * u_ambient_color_b;

    int delta_x = int(abs(floor(v_local_position.x / u_tile_size)));
    int delta_y = int(abs(floor(v_local_position.y / u_tile_size)));
    int delta_z = int(abs(floor(v_local_position.z / u_tile_size)));

    vec4 ambient = ambient_a;
    vec4 diffuse = diffuse_a;
    vec4 specular = specular_a;

    if ((delta_x + delta_y + delta_z) % 2 == 0) {
        ambient = ambient_b;
        diffuse = diffuse_b;
        specular = specular_b;
    }

    gl_FragColor = ambient + diffuse + specular;
}
