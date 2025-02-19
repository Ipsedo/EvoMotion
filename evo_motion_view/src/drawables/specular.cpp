//
// Created by samuel on 17/12/22.
//

#include "./specular.h"

#include <fstream>
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
          Program::Builder("./specular_vs.glsl", "./specular_fs.glsl")
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
    const std::optional<std::function<std::optional<glm::vec3>()>> &is_focus_function,
    const std::optional<std::function<bool()>> &is_hidden_function)
    : OBjSpecular(vertices, normals, ambient_color, diffuse_color, specular_color, shininess),
      is_focus_function(
          is_focus_function.has_value() ? is_focus_function.value()
                                        : []() { return std::nullopt; }),
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

    if (const auto focus_color = is_focus_function(); focus_color.has_value()) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);

        const glm::vec4 original_amb_color = ambient_color;
        const glm::vec4 original_diff_color = diffuse_color;
        const glm::vec4 original_spec_color = specular_color;

        ambient_color = glm::vec4(focus_color.value(), ambient_color.a);
        diffuse_color = glm::vec4(focus_color.value(), diffuse_color.a);
        specular_color = glm::vec4(focus_color.value(), specular_color.a);

        OBjSpecular::draw(
            projection_matrix, view_matrix, model_matrix, light_pos_from_camera, camera_pos);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        ambient_color = original_amb_color;
        diffuse_color = original_diff_color;
        specular_color = original_spec_color;

        glLineWidth(1.0f);
    }

    glDisable(GL_BLEND);
}

/*
 * MTL
 */

ObjMtlSpecular::ObjMtlSpecular(
    const std::filesystem::path &obj_file_path, const std::filesystem::path &mtl_file_path,
    float ambient_color_factor, float diffuse_color_factor, float specular_color_factor)
    : position_size(3), normal_size(3), color_size(4), shininess_size(1),
      stride((position_size + normal_size + color_size * 3 + shininess_size) * BYTES_PER_FLOAT),
      nb_vertices(0),
      program(Program::Builder("./specular_mtl_vs.glsl", "./specular_mtl_fs.glsl")
                  .add_uniform("u_mvp_matrix")
                  .add_uniform("u_mv_matrix")
                  .add_uniform("u_light_pos")
                  .add_uniform("u_distance_coef")
                  .add_uniform("u_light_coef")
                  .add_uniform("u_cam_pos")
                  .add_uniform("u_ambient_color_factor")
                  .add_uniform("u_diffuse_color_factor")
                  .add_uniform("u_specular_color_factor")
                  .add_buffer("packed_data_buffer", read_packed_data(obj_file_path, mtl_file_path))
                  .add_attribute("a_position")
                  .add_attribute("a_normal")
                  .add_attribute("a_ambient_color")
                  .add_attribute("a_diffuse_color")
                  .add_attribute("a_specular_color")
                  .add_attribute("a_shininess")
                  .build()),
      ambient_color_factor(ambient_color_factor), diffuse_color_factor(diffuse_color_factor),
      specular_color_factor(specular_color_factor) {}

std::vector<std::string> ObjMtlSpecular::split(const std::string &s, const char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) { elems.push_back(item); }
    return elems;
}

#include <iostream>

std::vector<float> ObjMtlSpecular::read_packed_data(
    const std::filesystem::path &obj_file_path, const std::filesystem::path &mtl_file_path) {

    nb_vertices = 0;

    std::string line;

    std::ifstream mtl_file(mtl_file_path);

    std::map<std::string, std::tuple<float, float, float>> mtl_amb_color;
    std::map<std::string, std::tuple<float, float, float>> mtl_diff_color;
    std::map<std::string, std::tuple<float, float, float>> mtl_spec_color;
    std::map<std::string, float> mtl_shininess;

    // MTL
    std::string current_mtl = "";
    while (std::getline(mtl_file, line)) {
        if (line.starts_with("newmtl")) {
            current_mtl = split(line, ' ')[1];//line.split(" ")[1];
        } else if (line.starts_with("Ka")) {
            auto tmp = split(line, ' ');
            mtl_amb_color[current_mtl] = {std::stof(tmp[1]), std::stof(tmp[2]), std::stof(tmp[3])};
        } else if (line.starts_with("Kd")) {
            auto tmp = split(line, ' ');
            mtl_diff_color[current_mtl] = {std::stof(tmp[1]), std::stof(tmp[2]), std::stof(tmp[3])};
        } else if (line.starts_with("Ks")) {
            auto tmp = split(line, ' ');
            mtl_spec_color[current_mtl] = {std::stof(tmp[1]), std::stof(tmp[2]), std::stof(tmp[3])};
        } else if (line.starts_with("Ns")) {
            mtl_shininess[current_mtl] = std::stof(split(line, ' ')[1]);
        }
    }

    // OBJ
    std::vector<std::tuple<float, float, float>> vertices_ref;
    std::vector<std::tuple<float, float, float>> normals_ref;

    std::vector<std::vector<int>> vertices_order;
    std::vector<std::vector<int>> normals_order;
    std::vector<std::string> mtl_to_use;

    std::vector<int> curr_vertices_order;
    std::vector<int> curr_normals_order;

    std::ifstream obj_file(obj_file_path);

    while (std::getline(obj_file, line)) {
        if (std::vector<std::string> split_line = split(line, ' '); split_line[0] == "vn") {
            normals_ref.emplace_back(
                std::stof(split_line[1]), std::stof(split_line[2]), std::stof(split_line[3]));
        } else if (split_line[0] == "v") {
            vertices_ref.emplace_back(
                std::stof(split_line[1]), std::stof(split_line[2]), std::stof(split_line[3]));
        } else if (split_line[0] == "f") {
            curr_vertices_order.push_back(std::stoi(split(split_line[1], '/')[0]));
            curr_vertices_order.push_back(std::stoi(split(split_line[2], '/')[0]));
            curr_vertices_order.push_back(std::stoi(split(split_line[3], '/')[0]));

            curr_normals_order.push_back(std::stoi(split(split_line[1], '/')[2]));
            curr_normals_order.push_back(std::stoi(split(split_line[2], '/')[2]));
            curr_normals_order.push_back(std::stoi(split(split_line[3], '/')[2]));
        } else if (split_line[0] == "usemtl") {
            if (mtl_to_use.size() > 0) {
                vertices_order.push_back(curr_vertices_order);
                normals_order.push_back(curr_normals_order);
            }
            curr_vertices_order = {};
            curr_normals_order = {};
            mtl_to_use.push_back(split_line[1]);
        }
    }
    vertices_order.push_back(curr_vertices_order);
    normals_order.push_back(curr_normals_order);

    std::vector<float> packed_data;

    for (int i = 0; i < vertices_order.size(); i++) {
        const auto curr_v_order = vertices_order[i];
        const auto curr_n_order = normals_order[i];

        const auto mtl = mtl_to_use[i];
        const auto [a_r, a_g, a_b] = mtl_amb_color[mtl];
        const auto [d_r, d_g, d_b] = mtl_diff_color[mtl];
        const auto [s_r, s_g, s_b] = mtl_spec_color[mtl];
        const auto shininess = mtl_shininess[mtl];

        for (int j = 0; j < curr_v_order.size(); j++) {
            const auto [x_p, y_p, z_p] = vertices_ref[curr_v_order[j] - 1];
            const auto [x_n, y_n, z_n] = normals_ref[curr_n_order[j] - 1];

            // position
            packed_data.push_back(x_p);
            packed_data.push_back(y_p);
            packed_data.push_back(z_p);

            // normals
            packed_data.push_back(x_n);
            packed_data.push_back(y_n);
            packed_data.push_back(z_n);

            // colors
            packed_data.push_back(a_r);
            packed_data.push_back(a_g);
            packed_data.push_back(a_b);
            packed_data.push_back(1.f);

            packed_data.push_back(d_r);
            packed_data.push_back(d_g);
            packed_data.push_back(d_b);
            packed_data.push_back(1.f);

            packed_data.push_back(s_r);
            packed_data.push_back(s_g);
            packed_data.push_back(s_b);
            packed_data.push_back(1.f);

            // shininess
            packed_data.push_back(shininess);

            nb_vertices++;
        }
    }

    return packed_data;
}

void ObjMtlSpecular::draw(
    glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix,
    glm::vec3 light_pos_from_camera, glm::vec3 camera_pos) {
    const auto mv_matrix = view_matrix * model_matrix;
    const auto mvp_matrix = projection_matrix * mv_matrix;

    program.use();

    program.attrib("a_position", "packed_data_buffer", position_size, stride, 0);
    program.attrib(
        "a_normal", "packed_data_buffer", normal_size, stride, position_size * BYTES_PER_FLOAT);
    program.attrib(
        "a_ambient_color", "packed_data_buffer", color_size, stride,
        (position_size + normal_size) * BYTES_PER_FLOAT);
    program.attrib(
        "a_diffuse_color", "packed_data_buffer", color_size, stride,
        (position_size + normal_size + color_size) * BYTES_PER_FLOAT);
    program.attrib(
        "a_specular_color", "packed_data_buffer", color_size, stride,
        (position_size + normal_size + color_size * 2) * BYTES_PER_FLOAT);
    program.attrib(
        "a_shininess", "packed_data_buffer", shininess_size, stride,
        (position_size + normal_size + color_size * 3) * BYTES_PER_FLOAT);

    program.uniform_mat4("u_mvp_matrix", mvp_matrix);
    program.uniform_mat4("u_mv_matrix", mv_matrix);

    program.uniform_vec3("u_light_pos", light_pos_from_camera);
    program.uniform_vec3("u_cam_pos", camera_pos);

    program.uniform_float("u_distance_coef", 0.f);
    program.uniform_float("u_light_coef", 1.f);

    program.uniform_float("u_ambient_color_factor", ambient_color_factor);
    program.uniform_float("u_diffuse_color_factor", diffuse_color_factor);
    program.uniform_float("u_specular_color_factor", specular_color_factor);

    Program::draw_arrays(GL_TRIANGLES, 0, nb_vertices);

    program.disable_attrib_array();
}
