//
// Created by samuel on 17/12/22.
//

#include "constants.h"
#include "specular.h"

OBjSpecular::OBjSpecular(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals,
    glm::vec4 ambient_color,
    glm::vec4 diffuse_color,
    glm::vec4 specular_color, float shininess)
    : ambient_color(ambient_color),
      diffuse_color(diffuse_color),
      specular_color(specular_color),
      nb_vertices(int(vertices.size())),
      shininess(shininess) {

    std::vector<float> vbo_data;
    for (int i = 0; i < vertices.size(); i++) {
        vbo_data.push_back(std::get<0>(vertices[i]));
        vbo_data.push_back(std::get<1>(vertices[i]));
        vbo_data.push_back(std::get<2>(vertices[i]));

        vbo_data.push_back(std::get<0>(normals[i]));
        vbo_data.push_back(std::get<1>(normals[i]));
        vbo_data.push_back(std::get<2>(normals[i]));
    }

    program = Program::Builder(
        "./shaders/specular_vs.glsl",
        "./shaders/specular_fs.glsl"
    ).add_uniform("u_mvp_matrix")
        .add_uniform("u_mv_matrix")
        .add_uniform("u_ambient_color")
        .add_uniform("u_diffuse_color")
        .add_uniform("u_specular_color")
        .add_uniform("u_light_pos")
        .add_uniform("u_distance_coef")
        .add_uniform("u_light_coef")
        .add_uniform("u_shininess")
        .add_uniform("u_cam_pos")
        .add_buffer("vertices_normals_buffer", vbo_data)
        .add_attribute("a_position")
        .add_attribute("a_normal")
        .build();
}

void
OBjSpecular::draw(glm::mat4 mvp_matrix, glm::mat4 mv_matrix,
                  glm::vec3 light_pos_from_camera, glm::vec3 camera_pos) {
    program.use();

    program.attrib(
        "a_position",
        "vertices_normals_buffer",
        POSITION_SIZE,
        STRIDE,
        0
    );

    program.attrib(
        "a_normal",
        "vertices_normals_buffer",
        NORMAL_SIZE,
        STRIDE,
        POSITION_SIZE * BYTES_PER_FLOAT
    );

    program.uniform_mat4("u_mvp_matrix", mvp_matrix);
    program.uniform_mat4("u_mv_matrix", mv_matrix);

    program.uniform_vec3("u_light_pos", light_pos_from_camera);
    program.uniform_vec3("u_cam_pos", camera_pos);

    program.uniform_vec4("u_ambient_color", ambient_color);
    program.uniform_vec4("u_diffuse_color", diffuse_color);
    program.uniform_vec4("u_specular_color", specular_color);

    program.uniform_float("u_distance_coef", 0.f);
    program.uniform_float("u_light_coef", 1.f);
    program.uniform_float("u_shininess", shininess);

    Program::draw_arrays(GL_TRIANGLES, 0, nb_vertices);

    program.disable_attrib_array();
}

OBjSpecular::~OBjSpecular() {
    program.kill();
}
