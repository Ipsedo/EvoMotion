//
// Created by samuel on 11/08/19.
//

#include "normal_map.h"
#include "../utils/res.h"
#include "../utils/image.h"
#include "shader.h"
#include "../utils/string_utils.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

NormalMapObj::NormalMapObj(std::string obj_file, std::string texture_file, std::string normals_file) : textures(
		new GLuint[2]) {
	init_prgm();
	bind();
	init_tex(std::move(texture_file), std::move(normals_file));
	gen_buffer(std::move(obj_file));
}

void NormalMapObj::init_tex(std::string texture_file, std::string normals_file) {

	img_rgb img_1 = load_image(std::move(texture_file));

	img_rgb img_2 = load_image(std::move(normals_file));

	glGenTextures(2, textures);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_1.width, img_1.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_1.colors);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_2.width, img_2.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_2.colors);

	glBindTexture(GL_TEXTURE_2D, 0);

	delete[] img_1.colors;
	delete[] img_2.colors;
}

void NormalMapObj::init_prgm() {
	m_program = glCreateProgram();
	GLuint vs = load_shader(GL_VERTEX_SHADER, exec_root + EVOMOTION_SEP + "shaders" + EVOMOTION_SEP + "normal_map_vs.glsl");
	GLuint fs = load_shader(GL_FRAGMENT_SHADER, exec_root + EVOMOTION_SEP + "shaders" + EVOMOTION_SEP + "normal_map_fs.glsl");
	glAttachShader(m_program, vs);
	glAttachShader(m_program, fs);
	glLinkProgram(m_program);
}

void NormalMapObj::bind() {
	m_mvp_matrix_handle = (GLuint) glGetUniformLocation(m_program, "u_MVPMatrix");
	m_mv_matrix_handle = (GLuint) glGetUniformLocation(m_program, "u_MVMatrix");

	m_position_handle = (GLuint) glGetAttribLocation(m_program, "a_Position");
	m_text_coord_handle = (GLuint) glGetAttribLocation(m_program, "a_TexCoord");
	m_normal_handle = (GLuint) glGetAttribLocation(m_program, "a_Normal");

	m_light_pos_handle = (GLuint) glGetUniformLocation(m_program, "u_LightPos");
	m_cam_pos_handle = (GLuint) glGetUniformLocation(m_program, "u_cam_pos");

	m_distance_coef_handle = (GLuint) glGetUniformLocation(m_program, "u_distance_coef");
	m_light_coef_handle = (GLuint) glGetUniformLocation(m_program, "u_light_coef");

	m_tex_handle = (GLuint) glGetUniformLocation(m_program, "u_tex");
	m_normal_map_handle = (GLuint) glGetUniformLocation(m_program, "u_normalMap");
}

void NormalMapObj::draw(glm::mat4 mvp_matrix, glm::mat4 mv_matrix, glm::vec3 light_pos, glm::vec3 cam_pos) {
	glUseProgram(m_program);

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray(m_position_handle);
	glVertexAttribPointer(m_position_handle, POSITION_SIZE, GL_FLOAT, GL_FALSE,
	                      STRIDE, 0);

	glEnableVertexAttribArray(m_normal_handle);
	glVertexAttribPointer(m_normal_handle, NORMAL_SIZE, GL_FLOAT, GL_FALSE,
	                      STRIDE, (char *) NULL + POSITION_SIZE * BYTES_PER_FLOAT);

	glEnableVertexAttribArray(m_text_coord_handle);
	glVertexAttribPointer(m_text_coord_handle, TEX_COORD_SIZE, GL_FLOAT, GL_FALSE,
	                      STRIDE, (char *) NULL + (POSITION_SIZE + NORMAL_SIZE) * BYTES_PER_FLOAT);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUniformMatrix4fv(m_mv_matrix_handle, 1, GL_FALSE, glm::value_ptr(mv_matrix));

	glUniformMatrix4fv(m_mvp_matrix_handle, 1, GL_FALSE, glm::value_ptr(mvp_matrix));

	glUniform3fv(m_light_pos_handle, 1, glm::value_ptr(light_pos));

	glUniform3fv(m_cam_pos_handle, 1, glm::value_ptr(cam_pos));

	glUniform1f(m_distance_coef_handle, 0.f);

	glUniform1f(m_light_coef_handle, 1.f);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(m_tex_handle, 0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(m_normal_map_handle, 1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	glDrawArrays(GL_TRIANGLES, 0, nb_vertex);

	glDisableVertexAttribArray(m_position_handle);
	glDisableVertexAttribArray(m_text_coord_handle);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void NormalMapObj::gen_buffer(std::string obj_file_name) {
	std::vector<float> packed_data = parse_obj(move(obj_file_name));

	glGenBuffers(1, &buffer);

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, packed_data.size() * BYTES_PER_FLOAT, packed_data.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

std::vector<float> NormalMapObj::parse_obj(std::string obj_file_name) {
	nb_vertex = 0;

	std::ifstream in(obj_file_name);

	if (!in) {
		std::cout << "Error during opening models file" << std::endl;
		exit(0);
	}
	std::string line;

	std::vector<float> vertex_list;
	std::vector<float> normal_list;
	std::vector<float> uv_list;
	std::vector<int> vertex_draw_order;
	std::vector<int> normal_draw_order;
	std::vector<int> uv_draw_order;

	while (getline(in, line)) {

		std::vector<std::string> splitted_line = split(line, ' ');
		if (!splitted_line.empty()) {
			if (splitted_line[0] == "vn") {
				normal_list.push_back(stof(splitted_line[1]));
				normal_list.push_back(stof(splitted_line[2]));
				normal_list.push_back(stof(splitted_line[3]));
			} else if (splitted_line[0] == "vt") {
				uv_list.push_back(stof(splitted_line[1]));
				uv_list.push_back(stof(splitted_line[2]));
			} else if (splitted_line[0] == "v") {
				vertex_list.push_back(stof(splitted_line[1]));
				vertex_list.push_back(stof(splitted_line[2]));
				vertex_list.push_back(stof(splitted_line[3]));
			} else if (splitted_line[0] == "f") {
				std::vector<std::string> v1 = split(splitted_line[1], '/');
				std::vector<std::string> v2 = split(splitted_line[2], '/');
				std::vector<std::string> v3 = split(splitted_line[3], '/');

				vertex_draw_order.push_back(stoi(v1[0]));
				vertex_draw_order.push_back(stoi(v2[0]));
				vertex_draw_order.push_back(stoi(v3[0]));

				uv_draw_order.push_back(stoi(v1[1]));
				uv_draw_order.push_back(stoi(v2[1]));
				uv_draw_order.push_back(stoi(v3[1]));

				normal_draw_order.push_back(stoi(v1[2]));
				normal_draw_order.push_back(stoi(v2[2]));
				normal_draw_order.push_back(stoi(v3[2]));

				v1.clear();
				v2.clear();
				v3.clear();
			}
		}
		splitted_line.clear();
	}

	in.close();

	std::vector<float> packed_data;
	for (int i = 0; i < vertex_draw_order.size(); i++) {
		packed_data.push_back(vertex_list[(vertex_draw_order[i] - 1) * 3]);
		packed_data.push_back(vertex_list[(vertex_draw_order[i] - 1) * 3 + 1]);
		packed_data.push_back(vertex_list[(vertex_draw_order[i] - 1) * 3 + 2]);

		packed_data.push_back(normal_list[(normal_draw_order[i] - 1) * 3]);
		packed_data.push_back(normal_list[(normal_draw_order[i] - 1) * 3 + 1]);
		packed_data.push_back(normal_list[(normal_draw_order[i] - 1) * 3 + 2]);

		packed_data.push_back(uv_list[(uv_draw_order[i] - 1) * 2]);
		packed_data.push_back(uv_list[(uv_draw_order[i] - 1) * 2 + 1]);

		nb_vertex++;
	}

	vertex_list.clear();
	vertex_draw_order.clear();
	normal_list.clear();
	normal_draw_order.clear();
	uv_list.clear();
	uv_draw_order.clear();

	return packed_data;
}

NormalMapObj::~NormalMapObj() {
	glDeleteTextures(2, textures);
	// TODO desaloué images textures
}