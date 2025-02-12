//
// Created by samuel on 17/12/22.
//

#include "./specular.h"

#include <optional>

#include "./constants.h"

std::vector<float> OBjSpecular::to_vbo_data(
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

OBjSpecular::OBjSpecular(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals, const glm::vec4 ambient_color,
    const glm::vec4 diffuse_color, const glm::vec4 specular_color, const float shininess)
    : ambient_color(ambient_color), diffuse_color(diffuse_color), specular_color(specular_color),
      position_size(3), normal_size(3), stride((position_size + normal_size) * BYTES_PER_FLOAT),
      shininess(shininess), nb_vertices(static_cast<int>(vertices.size())),
      program(
          Program::Builder("./shaders/specular_vs.glsl", "./shaders/specular_fs.glsl")
              .add_uniform("u_mvp_matrix")
              .add_uniform("u_mv_matrix")
              .add_uniform("u_ambient_color")
              .add_uniform("u_diffuse_color")
              .add_uniform("u_specular_color")
              .add_uniform("u_light_pos")
              .add_uniform("u_distance_coef")
              .add_uniform("u_light_coef")
              .add_uniform("u_shininess")
              .add_uniform("u_cam_pos")
              .add_buffer("vertices_normals_buffer", OBjSpecular::to_vbo_data(vertices, normals))
              .add_attribute("a_position")
              .add_attribute("a_normal")
              .build()) {}

void OBjSpecular::draw(
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

OBjSpecular::~OBjSpecular() { program.kill(); }

/*
 * Edge
 */

BuilderObjSpecular::BuilderObjSpecular(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals, const glm::vec4 &ambient_color,
    const glm::vec4 &diffuse_color, const glm::vec4 &specular_color, const float shininess,
    const std::optional<std::function<bool()>> &is_focus_function,
    const std::optional<std::function<bool()>> &is_hidden_function)
    : OBjSpecular(vertices, normals, ambient_color, diffuse_color, specular_color, shininess),
      is_focus_function(
          is_focus_function.has_value() ? is_focus_function.value() : []() { return false; }),
      is_hidden_function(
          is_hidden_function.has_value() ? is_hidden_function.value() : []() { return false; }) {}

void BuilderObjSpecular::draw(
    const glm::mat4 projection_matrix, const glm::mat4 view_matrix, const glm::mat4 model_matrix,
    const glm::vec3 light_pos_from_camera, const glm::vec3 camera_pos) {

    if (is_hidden_function()) {
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        ambient_color.z = 0.25f;
        diffuse_color.z = 0.25f;
        specular_color.z = 0.25f;
    } else {
        ambient_color.z = 1.f;
        diffuse_color.z = 1.f;
        specular_color.z = 1.f;
    }

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    OBjSpecular::draw(
        projection_matrix, view_matrix, model_matrix, light_pos_from_camera, camera_pos);

    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    if (is_focus_function()) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);

        const glm::vec4 original_amb_color = ambient_color;
        const glm::vec4 original_diff_color = diffuse_color;
        const glm::vec4 original_spec_color = specular_color;

        ambient_color = glm::vec4(glm::vec3(0.f), 1.f);
        diffuse_color = glm::vec4(glm::vec3(0.f), 1.f);
        specular_color = glm::vec4(glm::vec3(0.f), 1.f);

        OBjSpecular::draw(
            projection_matrix, view_matrix, model_matrix, light_pos_from_camera, camera_pos);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        ambient_color = original_amb_color;
        diffuse_color = original_diff_color;
        specular_color = original_spec_color;
    }
}