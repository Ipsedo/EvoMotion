uniform mat4 u_mvp_matrix;
uniform mat4 u_mv_atrix;

attribute vec4 a_position;
attribute vec3 a_normal;

varying vec3 v_position;
varying vec3 v_normal;

void main(){
    v_position = vec3(u_mv_atrix * a_position);

    v_material_ambient_Color = a_material_ambient_Color;
    v_material_diffuse_Color = a_material_diffuse_Color;
    v_material_specular_Color = a_material_specular_Color;

    v_normal = normalize(vec3(u_mv_atrix * vec4(a_normal, 0.0)));

    v_material_shininess = a_material_shininess;

    gl_Position = u_mvp_matrix * a_position;
}
