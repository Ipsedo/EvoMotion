//
// Created by samuel on 11/08/19.
//

#include <map>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "obj_mtl_vbo.h"
#include "shader.h"
#include "../utils/res.h"
#include "../utils/string_utils.h"

void obj_mtl_vbo::init() {
    m_program = glCreateProgram();

    GLuint vertex_shader = load_shader(GL_VERTEX_SHADER, get_shader_folder() + EVOMOTION_SEP + "specular_vs.glsl");
    GLuint fragment_shader = load_shader(GL_FRAGMENT_SHADER, get_shader_folder() + EVOMOTION_SEP + "specular_fs.glsl");

    glAttachShader(m_program, vertex_shader);
    glAttachShader(m_program, fragment_shader);
    glLinkProgram(m_program);
}

void obj_mtl_vbo::bind() {
    m_mvp_matrix_handle = (GLuint) glGetUniformLocation(m_program, "u_MVPMatrix");
    m_mv_matrix_handle = (GLuint) glGetUniformLocation(m_program, "u_MVMatrix");
    m_position_handle = (GLuint) glGetAttribLocation(m_program, "a_Position");
    m_amb_color_handle = (GLuint) glGetAttribLocation(m_program, "a_material_ambient_Color");
    m_diff_color_handle = (GLuint) glGetAttribLocation(m_program, "a_material_diffuse_Color");
    m_spec_color_handle = (GLuint) glGetAttribLocation(m_program, "a_material_specular_Color");
    m_light_pos_handle = (GLuint) glGetUniformLocation(m_program, "u_LightPos");
    m_normal_handle = (GLuint) glGetAttribLocation(m_program, "a_Normal");
    m_distance_coef_handle = (GLuint) glGetUniformLocation(m_program, "u_distance_coef");
    m_light_coef_handle = (GLuint) glGetUniformLocation(m_program, "u_light_coef");
    m_camera_pos_handle = (GLuint) glGetUniformLocation(m_program, "u_CameraPosition");
    m_spec_shininess_handle = (GLuint) glGetAttribLocation(m_program, "a_material_shininess");
}

void obj_mtl_vbo::bind_buffer(std::vector<float> packed_data) {
    glGenBuffers(1, &packed_data_buffer_id);

    glBindBuffer(GL_ARRAY_BUFFER, packed_data_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, packed_data.size() * BYTES_PER_FLOAT, &packed_data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    packed_data.clear();
}

std::vector<float> obj_mtl_vbo::parse_obj(std::string obj_file_name, std::string mtl_file_name) {
    using namespace std;
    vector<float> res;
    std::map<string, glm::vec3> amb_color;
    std::map<string, glm::vec3> diff_color;
    std::map<string, glm::vec3> spec_color;
    std::map<string, float> shin;

    std::ifstream mtl_file(mtl_file_name);
    string str;

    string curr_mtl;
    while (getline(mtl_file, str)) {
        vector<string> splitted_line = split(str, ' ');
        if (!splitted_line.empty()) {
            if (splitted_line[0] == "newmtl") {
                curr_mtl = splitted_line[1];
            } else if (splitted_line[0] == "Ka") {
                pair<string, glm::vec3>
                        tmp(curr_mtl,
                            glm::vec3(strtof(splitted_line[1].c_str(), NULL),
                                      strtof(splitted_line[2].c_str(), NULL),
                                      strtof(splitted_line[3].c_str(), NULL))
                );
                amb_color.insert(tmp);
            } else if (splitted_line[0] == "Kd") {
                pair<string, glm::vec3>
                        tmp(curr_mtl,
                            glm::vec3(strtof(splitted_line[1].c_str(), NULL),
                                      strtof(splitted_line[2].c_str(), NULL),
                                      strtof(splitted_line[3].c_str(), NULL))
                );
                diff_color.insert(tmp);
            } else if (splitted_line[0] == "Ks") {
                pair<string, glm::vec3>
                        tmp(curr_mtl,
                            glm::vec3(strtof(splitted_line[1].c_str(), NULL),
                                      strtof(splitted_line[2].c_str(), NULL),
                                      strtof(splitted_line[3].c_str(), NULL))
                );
                spec_color.insert(tmp);
            } else if (splitted_line[0] == "Ns") {
                pair<string, float> tmp(curr_mtl, strtof(splitted_line[1].c_str(), 0));
                shin.insert(tmp);
            }
        }
    }
    mtl_file.close();

    vector<float> curr_vertex_list;
    vector<float> curr_normal_list;
    vector<int> curr_vertex_draw_order_list;
    vector<int> curr_normal_draw_order_list;
    vector<vector<int>> all_vertex_draw_order_list;
    vector<vector<int>> all_normal_draw_order_list;
    vector<string> mtl_to_use;

    int id_mtl = 0;

    ifstream obj_file(obj_file_name);
    while (getline(obj_file, str)) {
        vector<string> splitted_line = split(str, ' ');

        if (!splitted_line.empty()) {
            if (splitted_line[0] == "usemtl") {
                mtl_to_use.push_back(splitted_line[1]);

                if (id_mtl != 0) {
                    all_vertex_draw_order_list.push_back(curr_vertex_draw_order_list);
                    all_normal_draw_order_list.push_back(curr_normal_draw_order_list);
                }

                curr_vertex_draw_order_list.clear();
                curr_normal_draw_order_list.clear();

                id_mtl++;
            } else if (splitted_line[0] == "vn") {
                curr_normal_list.push_back(strtof(splitted_line[1].c_str(), NULL));
                curr_normal_list.push_back(strtof(splitted_line[2].c_str(), NULL));
                curr_normal_list.push_back(strtof(splitted_line[3].c_str(), NULL));
            } else if (splitted_line[0] == "v") {
                curr_vertex_list.push_back(strtof(splitted_line[1].c_str(), NULL));
                curr_vertex_list.push_back(strtof(splitted_line[2].c_str(), NULL));
                curr_vertex_list.push_back(strtof(splitted_line[3].c_str(), NULL));
            } else if (splitted_line[0] == "f") {
                curr_vertex_draw_order_list.push_back(atoi(split(splitted_line[1], '/')[0].c_str()));
                curr_vertex_draw_order_list.push_back(atoi(split(splitted_line[2], '/')[0].c_str()));
                curr_vertex_draw_order_list.push_back(atoi(split(splitted_line[3], '/')[0].c_str()));

                curr_normal_draw_order_list.push_back(atoi(split(splitted_line[1], '/')[2].c_str()));
                curr_normal_draw_order_list.push_back(atoi(split(splitted_line[2], '/')[2].c_str()));
                curr_normal_draw_order_list.push_back(atoi(split(splitted_line[3], '/')[2].c_str()));
            }
        }
    }
    all_vertex_draw_order_list.push_back(curr_vertex_draw_order_list);
    all_normal_draw_order_list.push_back(curr_normal_draw_order_list);
    obj_file.close();

    nb_vertex = 0;

    for (int i = 0; i < all_vertex_draw_order_list.size(); i++) {
        glm::vec3 amb = glm::vec3(0.f);
        glm::vec3 diff = glm::vec3(0.f);
        glm::vec3 spec = glm::vec3(0.f);

        if (random_color) {
            amb = glm::vec3((float) rand() / RAND_MAX, (float) rand() / RAND_MAX, (float) rand() / RAND_MAX);
            diff = glm::vec3((float) rand() / RAND_MAX, (float) rand() / RAND_MAX, (float) rand() / RAND_MAX);
            spec = glm::vec3((float) rand() / RAND_MAX, (float) rand() / RAND_MAX, (float) rand() / RAND_MAX);
        } else {
            amb = amb_color.find(mtl_to_use[i])->second;
            diff = diff_color.find(mtl_to_use[i])->second;
            spec = spec_color.find(mtl_to_use[i])->second;
        }

        for (int j = 0; j < all_vertex_draw_order_list[i].size(); j++) {
            res.push_back(curr_vertex_list[(all_vertex_draw_order_list[i][j] - 1) * 3]);
            res.push_back(curr_vertex_list[(all_vertex_draw_order_list[i][j] - 1) * 3 + 1]);
            res.push_back(curr_vertex_list[(all_vertex_draw_order_list[i][j] - 1) * 3 + 2]);

            res.push_back(curr_normal_list[(all_normal_draw_order_list[i][j] - 1) * 3]);
            res.push_back(curr_normal_list[(all_normal_draw_order_list[i][j] - 1) * 3 + 1]);
            res.push_back(curr_normal_list[(all_normal_draw_order_list[i][j] - 1) * 3 + 2]);

            res.push_back(amb[0]);
            res.push_back(amb[1]);
            res.push_back(amb[2]);
            res.push_back(1.f);

            res.push_back(diff[0]);
            res.push_back(diff[1]);
            res.push_back(diff[2]);
            res.push_back(1.f);

            res.push_back(spec[0]);
            res.push_back(spec[1]);
            res.push_back(spec[2]);
            res.push_back(1.f);

            res.push_back(shin.find(mtl_to_use[i])->second);

            nb_vertex++;
        }
    }

    return res;
}

obj_mtl_vbo::obj_mtl_vbo(std::string obj_file_name, std::string mtl_file_name, bool will_random_color) {
    this->random_color = will_random_color;

    init();
    bind();
    bind_buffer(parse_obj(std::move(obj_file_name), std::move(mtl_file_name)));

    light_coef = 1;
    distance_coef = 0;
}

void obj_mtl_vbo::draw(glm::mat4 mvp_matrix, glm::mat4 mv_matrix, glm::vec3 ligh_pos_in_eye_space, glm::vec3 camera_pos) {
    glUseProgram(m_program);

    glBindBuffer(GL_ARRAY_BUFFER, packed_data_buffer_id);
    glEnableVertexAttribArray(m_position_handle);
    glVertexAttribPointer(m_position_handle, POSITION_DATA_SIZE, GL_FLOAT, GL_FALSE,
                          STRIDE, 0);

    glEnableVertexAttribArray(m_normal_handle);
    glVertexAttribPointer(m_normal_handle, NORMAL_DATA_SIZE, GL_FLOAT, GL_FALSE,
                          STRIDE, (char *) NULL + POSITION_DATA_SIZE * BYTES_PER_FLOAT);

    glEnableVertexAttribArray(m_amb_color_handle);
    glVertexAttribPointer(m_amb_color_handle, COLOR_DATA_SIZE, GL_FLOAT, GL_FALSE,
                          STRIDE, (char *) NULL + (POSITION_DATA_SIZE + NORMAL_DATA_SIZE) * BYTES_PER_FLOAT);

    glEnableVertexAttribArray(m_diff_color_handle);
    glVertexAttribPointer(m_diff_color_handle, COLOR_DATA_SIZE, GL_FLOAT, GL_FALSE,
                          STRIDE,
                          (char *) NULL + (POSITION_DATA_SIZE + NORMAL_DATA_SIZE + COLOR_DATA_SIZE) * BYTES_PER_FLOAT);

    glEnableVertexAttribArray(m_spec_color_handle);
    glVertexAttribPointer(m_spec_color_handle, COLOR_DATA_SIZE, GL_FLOAT, GL_FALSE,
                          STRIDE, (char *) NULL +
                                  (POSITION_DATA_SIZE + NORMAL_DATA_SIZE + COLOR_DATA_SIZE * 2) * BYTES_PER_FLOAT);

    glEnableVertexAttribArray(m_spec_shininess_handle);
    glVertexAttribPointer(m_spec_shininess_handle, SHININESS_DATA_SIZE, GL_FLOAT, GL_FALSE,
                          STRIDE, (char *) NULL +
                                  (POSITION_DATA_SIZE + NORMAL_DATA_SIZE + COLOR_DATA_SIZE * 3) * BYTES_PER_FLOAT);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUniformMatrix4fv(m_mv_matrix_handle, 1, GL_FALSE, glm::value_ptr(mv_matrix));

    glUniformMatrix4fv(m_mvp_matrix_handle, 1, GL_FALSE, glm::value_ptr(mvp_matrix));

    glUniform3fv(m_light_pos_handle, 1, glm::value_ptr(ligh_pos_in_eye_space));

    glUniform3fv(m_camera_pos_handle, 1, glm::value_ptr(camera_pos));

    glUniform1f(m_distance_coef_handle, distance_coef);

    glUniform1f(m_light_coef_handle, light_coef);

    glDrawArrays(GL_TRIANGLES, 0, nb_vertex);

    glDisableVertexAttribArray(m_position_handle);
    glDisableVertexAttribArray(m_normal_handle);
    glDisableVertexAttribArray(m_amb_color_handle);
    glDisableVertexAttribArray(m_diff_color_handle);
    glDisableVertexAttribArray(m_spec_color_handle);
    glDisableVertexAttribArray(m_spec_shininess_handle);
}

