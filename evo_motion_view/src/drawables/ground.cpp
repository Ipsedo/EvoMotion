//
// Created by samuel on 11/12/24.
//

#include "./ground.h"

#include "./constants.h"

std::vector<float> TileGround::to_vbo_data(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals) {
    std::vector<float> vbo_data;
    for (int i = 0; i < vertices.size(); i++) {
        vbo_data.push_back(std::get<0>(vertices[i]));
        vbo_data.push_back(std::get<1>(vertices[i]));
        vbo_data.push_back(std::get<2>(vertices[i]));

        vbo_data.push_back(std::get<0>(normals[i]));
        vbo_data.push_back(std::get<1>(normals[i]));
        vbo_data.push_back(std::get<2>(normals[i]));
    }
    return vbo_data;
}

TileGround::TileGround(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color_a,
    glm::vec4 diffuse_color_a, glm::vec4 specular_color_a, glm::vec4 ambient_color_b,
    glm::vec4 diffuse_color_b, glm::vec4 specular_color_b, float shininess, float tile_size)
    : position_size(3), normal_size(3), stride((position_size + normal_size) * BYTES_PER_FLOAT),
      ambient_color_a(ambient_color_a), diffuse_color_a(diffuse_color_a),
      specular_color_a(specular_color_a), ambient_color_b(ambient_color_b),
      diffuse_color_b(diffuse_color_b), specular_color_b(specular_color_b), shininess(shininess),
      nb_vertices(static_cast<int>(vertices.size())), tile_size(tile_size),
      program(Program::Builder("./shaders/tile_specular_vs.glsl", "./shaders/tile_specular_fs.glsl")
                  .add_uniform("u_mvp_matrix")
                  .add_uniform("u_mv_matrix")
                  .add_uniform("u_m_matrix")
                  .add_uniform("u_ambient_color_a")
                  .add_uniform("u_diffuse_color_a")
                  .add_uniform("u_specular_color_a")
                  .add_uniform("u_ambient_color_b")
                  .add_uniform("u_diffuse_color_b")
                  .add_uniform("u_specular_color_b")
                  .add_uniform("u_tile_size")
                  .add_uniform("u_light_pos")
                  .add_uniform("u_distance_coef")
                  .add_uniform("u_light_coef")
                  .add_uniform("u_shininess")
                  .add_uniform("u_cam_pos")
                  .add_buffer("vertices_normals_buffer", TileGround::to_vbo_data(vertices, normals))
                  .add_attribute("a_position")
                  .add_attribute("a_normal")
                  .build()) {}

void TileGround::draw(
    const glm::mat4 projection_matrix, const glm::mat4 view_matrix, const glm::mat4 model_matrix,
    const glm::vec3 light_pos_from_camera, const glm::vec3 camera_pos) {
    const auto mv_matrix = view_matrix * model_matrix;
    const auto mvp_matrix = projection_matrix * mv_matrix;

    program.use();

    program.attrib("a_position", "vertices_normals_buffer", position_size, stride, 0);

    program.attrib(
        "a_normal", "vertices_normals_buffer", normal_size, stride,
        position_size * BYTES_PER_FLOAT);

    program.uniform_mat4("u_mvp_matrix", mvp_matrix);
    program.uniform_mat4("u_mv_matrix", mv_matrix);
    program.uniform_mat4("u_m_matrix", model_matrix);

    program.uniform_vec3("u_light_pos", light_pos_from_camera);
    program.uniform_vec3("u_cam_pos", camera_pos);

    program.uniform_vec4("u_ambient_color_a", ambient_color_a);
    program.uniform_vec4("u_diffuse_color_a", diffuse_color_a);
    program.uniform_vec4("u_specular_color_a", specular_color_a);

    program.uniform_vec4("u_ambient_color_b", ambient_color_b);
    program.uniform_vec4("u_diffuse_color_b", diffuse_color_b);
    program.uniform_vec4("u_specular_color_b", specular_color_b);

    program.uniform_float("u_distance_coef", 0.f);
    program.uniform_float("u_light_coef", 1.f);
    program.uniform_float("u_shininess", shininess);
    program.uniform_float("u_tile_size", tile_size);

    Program::draw_arrays(GL_TRIANGLES, 0, nb_vertices);

    program.disable_attrib_array();
}

TileGround::~TileGround() = default;
